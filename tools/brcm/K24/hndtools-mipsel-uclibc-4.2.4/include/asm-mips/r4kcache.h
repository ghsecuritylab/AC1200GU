/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Inline assembly cache operations.
 *
 * Copyright (C) 1996 David S. Miller (dm@engr.sgi.com)
 * Copyright (C) 1997 - 2002 Ralf Baechle (ralf@gnu.org)
 * Copyright (C) 2004 Ralf Baechle (ralf@linux-mips.org)
 */
#ifndef __ASM_R4KCACHE_H
#define __ASM_R4KCACHE_H

#include <asm/asm.h>
#include <asm/cacheops.h>

#ifdef CONFIG_BCM4710
#define BCM4710_DUMMY_RREG() (((sbconfig_t *)(KSEG1ADDR(SB_ENUM_BASE + SBCONFIGOFF)))->sbimstate)

#define BCM4710_FILL_TLB(addr) (*(volatile unsigned long *)(addr))
#define BCM4710_PROTECTED_FILL_TLB(addr) ({ unsigned long x; get_dbe(x, (volatile unsigned long *)(addr)); })
#else
#define BCM4710_DUMMY_RREG()

#define BCM4710_FILL_TLB(addr)
#define BCM4710_PROTECTED_FILL_TLB(addr)
#endif

#define cache_op(op,addr)						\
	__asm__ __volatile__(						\
	"	.set	noreorder				\n"	\
	"	.set	mips3\n\t				\n"	\
	"	cache	%0, %1					\n"	\
	"	.set	mips0					\n"	\
	"	.set	reorder"					\
	:								\
	: "i" (op), "m" (*(unsigned char *)(addr)))

static inline void flush_icache_line_indexed(unsigned long addr)
{
	unsigned int way;
	unsigned long ws_inc = 1UL << current_cpu_data.dcache.waybit;
	
	for (way = 0; way < current_cpu_data.dcache.ways; way++) {
		cache_op(Index_Invalidate_I, addr);
		addr += ws_inc;
	}
}

static inline void flush_dcache_line_indexed(unsigned long addr)
{
	unsigned int way;
	unsigned long ws_inc = 1UL << current_cpu_data.dcache.waybit;
	
	for (way = 0; way < current_cpu_data.dcache.ways; way++) {
		BCM4710_DUMMY_RREG();
		cache_op(Index_Writeback_Inv_D, addr);
		addr += ws_inc;
	}
}

static inline void flush_scache_line_indexed(unsigned long addr)
{
	cache_op(Index_Writeback_Inv_SD, addr);
}

static inline void flush_icache_line(unsigned long addr)
{
	cache_op(Hit_Invalidate_I, addr);
}

static inline void flush_dcache_line(unsigned long addr)
{
	BCM4710_DUMMY_RREG();
	cache_op(Hit_Writeback_Inv_D, addr);
}

static inline void invalidate_dcache_line(unsigned long addr)
{
	cache_op(Hit_Invalidate_D, addr);
}

static inline void invalidate_scache_line(unsigned long addr)
{
	cache_op(Hit_Invalidate_SD, addr);
}

static inline void flush_scache_line(unsigned long addr)
{
	cache_op(Hit_Writeback_Inv_SD, addr);
}

/*
 * The next two are for badland addresses like signal trampolines.
 */
static inline void protected_flush_icache_line(unsigned long addr)
{
	__asm__ __volatile__(
		".set noreorder\n\t"
		".set mips3\n"
		"1:\tcache %0,(%1)\n"
		"2:\t.set mips0\n\t"
		".set reorder\n\t"
		".section\t__ex_table,\"a\"\n\t"
		STR(PTR)"\t1b,2b\n\t"
		".previous"
		:
		: "i" (Hit_Invalidate_I), "r" (addr));
}

/*
 * R10000 / R12000 hazard - these processors don't support the Hit_Writeback_D
 * cacheop so we use Hit_Writeback_Inv_D which is supported by all R4000-style
 * caches.  We're talking about one cacheline unnecessarily getting invalidated
 * here so the penaltiy isn't overly hard.
 */
static inline void protected_writeback_dcache_line(unsigned long addr)
{
	BCM4710_DUMMY_RREG();
	__asm__ __volatile__(
		".set noreorder\n\t"
		".set mips3\n"
		"1:\tcache %0,(%1)\n"
		"2:\t.set mips0\n\t"
		".set reorder\n\t"
		".section\t__ex_table,\"a\"\n\t"
		STR(PTR)"\t1b,2b\n\t"
		".previous"
		:
		: "i" (Hit_Writeback_Inv_D), "r" (addr));
}

/*
 * This one is RM7000-specific
 */
static inline void invalidate_tcache_page(unsigned long addr)
{
	cache_op(Page_Invalidate_T, addr);
}

#define cache16_unroll32(base,op)					\
	__asm__ __volatile__(						\
	"	.set noreorder					\n"	\
	"	.set mips3					\n"	\
	"	cache %1, 0x000(%0); cache %1, 0x010(%0)	\n"	\
	"	cache %1, 0x020(%0); cache %1, 0x030(%0)	\n"	\
	"	cache %1, 0x040(%0); cache %1, 0x050(%0)	\n"	\
	"	cache %1, 0x060(%0); cache %1, 0x070(%0)	\n"	\
	"	cache %1, 0x080(%0); cache %1, 0x090(%0)	\n"	\
	"	cache %1, 0x0a0(%0); cache %1, 0x0b0(%0)	\n"	\
	"	cache %1, 0x0c0(%0); cache %1, 0x0d0(%0)	\n"	\
	"	cache %1, 0x0e0(%0); cache %1, 0x0f0(%0)	\n"	\
	"	cache %1, 0x100(%0); cache %1, 0x110(%0)	\n"	\
	"	cache %1, 0x120(%0); cache %1, 0x130(%0)	\n"	\
	"	cache %1, 0x140(%0); cache %1, 0x150(%0)	\n"	\
	"	cache %1, 0x160(%0); cache %1, 0x170(%0)	\n"	\
	"	cache %1, 0x180(%0); cache %1, 0x190(%0)	\n"	\
	"	cache %1, 0x1a0(%0); cache %1, 0x1b0(%0)	\n"	\
	"	cache %1, 0x1c0(%0); cache %1, 0x1d0(%0)	\n"	\
	"	cache %1, 0x1e0(%0); cache %1, 0x1f0(%0)	\n"	\
	"	.set mips0					\n"	\
	"	.set reorder					\n"	\
		:							\
		: "r" (base),						\
		  "i" (op));

#define cache_unroll(base,op)                   \
	__asm__ __volatile__("                  \
		.set noreorder;                 \
		.set mips3;                     \
		cache %1, (%0);                 \
		.set mips0;                     \
		.set reorder"                   \
		:                               \
		: "r" (base),                   \
		  "i" (op));


static inline void blast_dcache(void)
{
	unsigned long start = KSEG0;
	unsigned long dcache_size = current_cpu_data.dcache.waysize * current_cpu_data.dcache.ways;
	unsigned long end = (start + dcache_size);

	while(start < end) {
		BCM4710_DUMMY_RREG();
		cache_unroll(start,Index_Writeback_Inv_D);
		start += current_cpu_data.dcache.linesz;
	}
}

static inline void blast_dcache_page(unsigned long page)
{
	unsigned long start = page;
	unsigned long end = start + PAGE_SIZE;

	BCM4710_FILL_TLB(start);
	do {
		BCM4710_DUMMY_RREG();
		cache_unroll(start,Hit_Writeback_Inv_D);
		start += current_cpu_data.dcache.linesz;
	} while (start < end);
}

static inline void blast_dcache_page_indexed(unsigned long page)
{
	unsigned long start = page;
	unsigned long end = start + PAGE_SIZE;
	unsigned long ws_inc = 1UL << current_cpu_data.dcache.waybit;
	unsigned long ws_end = current_cpu_data.dcache.ways <<
	                       current_cpu_data.dcache.waybit;
	unsigned long ws, addr;

	for (ws = 0; ws < ws_end; ws += ws_inc) {
		start = page + ws;
		for (addr = start; addr < end; addr += current_cpu_data.dcache.linesz) {
			BCM4710_DUMMY_RREG();
			cache_unroll(addr,Index_Writeback_Inv_D);
		}
	}	
}

static inline void blast_dcache16(void)
{
	unsigned long start = KSEG0;
	unsigned long end = start + current_cpu_data.dcache.waysize;
	unsigned long ws_inc = 1UL << current_cpu_data.dcache.waybit;
	unsigned long ws_end = current_cpu_data.dcache.ways << 
	                       current_cpu_data.dcache.waybit;
	unsigned long ws, addr;

	for (ws = 0; ws < ws_end; ws += ws_inc) 
		for (addr = start; addr < end; addr += 0x200) {
			cache16_unroll32(addr|ws,Index_Writeback_Inv_D);
		}
}

static inline void blast_dcache16_page(unsigned long page)
{
	unsigned long start = page;
	unsigned long end = start + PAGE_SIZE;

	do {
		cache16_unroll32(start,Hit_Writeback_Inv_D);
		start += 0x200;
	} while (start < end);
}

static inline void blast_dcache16_page_indexed(unsigned long page)
{
	unsigned long start = page;
	unsigned long end = start + PAGE_SIZE;
	unsigned long ws_inc = 1UL << current_cpu_data.dcache.waybit;
	unsigned long ws_end = current_cpu_data.dcache.ways <<
	                       current_cpu_data.dcache.waybit;
	unsigned long ws, addr;

	for (ws = 0; ws < ws_end; ws += ws_inc) 
		for (addr = start; addr < end; addr += 0x200) {
			cache16_unroll32(addr|ws,Index_Writeback_Inv_D);
		}
}

static inline void blast_icache16(void)
{
	unsigned long start = KSEG0;
	unsigned long end = start + current_cpu_data.icache.waysize;
	unsigned long ws_inc = 1UL << current_cpu_data.icache.waybit;
	unsigned long ws_end = current_cpu_data.icache.ways <<
	                       current_cpu_data.icache.waybit;
	unsigned long ws, addr;

	for (ws = 0; ws < ws_end; ws += ws_inc) 
		for (addr = start; addr < end; addr += 0x200) 
			cache16_unroll32(addr|ws,Index_Invalidate_I);
}

static inline void blast_icache16_page(unsigned long page)
{
	unsigned long start = page;
	unsigned long end = start + PAGE_SIZE;

	BCM4710_FILL_TLB(start);
	do {
		cache16_unroll32(start,Hit_Invalidate_I);
		start += 0x200;
	} while (start < end);
}

static inline void blast_icache16_page_indexed(unsigned long page)
{
	unsigned long start = page;
	unsigned long end = start + PAGE_SIZE;
	unsigned long ws_inc = 1UL << current_cpu_data.icache.waybit;
	unsigned long ws_end = current_cpu_data.icache.ways <<
	                       current_cpu_data.icache.waybit;
	unsigned long ws, addr;

	for (ws = 0; ws < ws_end; ws += ws_inc) 
		for (addr = start; addr < end; addr += 0x200) 
			cache16_unroll32(addr|ws,Index_Invalidate_I);
}

static inline void blast_scache16(void)
{
	unsigned long start = KSEG0;
	unsigned long end = start + current_cpu_data.scache.waysize;
	unsigned long ws_inc = 1UL << current_cpu_data.scache.waybit;
	unsigned long ws_end = current_cpu_data.scache.ways << 
	                       current_cpu_data.scache.waybit;
	unsigned long ws, addr;

	for (ws = 0; ws < ws_end; ws += ws_inc) 
		for (addr = start; addr < end; addr += 0x200)
			cache16_unroll32(addr|ws,Index_Writeback_Inv_SD);
}

static inline void blast_scache16_page(unsigned long page)
{
	unsigned long start = page;
	unsigned long end = page + PAGE_SIZE;

	do {
		cache16_unroll32(start,Hit_Writeback_Inv_SD);
		start += 0x200;
	} while (start < end);
}

static inline void blast_scache16_page_indexed(unsigned long page)
{
	unsigned long start = page;
	unsigned long end = start + PAGE_SIZE;
	unsigned long ws_inc = 1UL << current_cpu_data.scache.waybit;
	unsigned long ws_end = current_cpu_data.scache.ways <<
	                       current_cpu_data.scache.waybit;
	unsigned long ws, addr;

	for (ws = 0; ws < ws_end; ws += ws_inc) 
		for (addr = start; addr < end; addr += 0x200) 
			cache16_unroll32(addr|ws,Index_Writeback_Inv_SD);
}

#define cache32_unroll32(base,op)					\
	__asm__ __volatile__(						\
	"	.set noreorder					\n"	\
	"	.set mips3					\n"	\
	"	cache %1, 0x000(%0); cache %1, 0x020(%0)	\n"	\
	"	cache %1, 0x040(%0); cache %1, 0x060(%0)	\n"	\
	"	cache %1, 0x080(%0); cache %1, 0x0a0(%0)	\n"	\
	"	cache %1, 0x0c0(%0); cache %1, 0x0e0(%0)	\n"	\
	"	cache %1, 0x100(%0); cache %1, 0x120(%0)	\n"	\
	"	cache %1, 0x140(%0); cache %1, 0x160(%0)	\n"	\
	"	cache %1, 0x180(%0); cache %1, 0x1a0(%0)	\n"	\
	"	cache %1, 0x1c0(%0); cache %1, 0x1e0(%0)	\n"	\
	"	cache %1, 0x200(%0); cache %1, 0x220(%0)	\n"	\
	"	cache %1, 0x240(%0); cache %1, 0x260(%0)	\n"	\
	"	cache %1, 0x280(%0); cache %1, 0x2a0(%0)	\n"	\
	"	cache %1, 0x2c0(%0); cache %1, 0x2e0(%0)	\n"	\
	"	cache %1, 0x300(%0); cache %1, 0x320(%0)	\n"	\
	"	cache %1, 0x340(%0); cache %1, 0x360(%0)	\n"	\
	"	cache %1, 0x380(%0); cache %1, 0x3a0(%0)	\n"	\
	"	cache %1, 0x3c0(%0); cache %1, 0x3e0(%0)	\n"	\
	"	.set mips0					\n"	\
	"	.set reorder					\n"	\
		:							\
		: "r" (base),						\
		  "i" (op));


static inline void blast_dcache32(void)
{
	unsigned long start = KSEG0;
	unsigned long end = start + current_cpu_data.dcache.waysize;
	unsigned long ws_inc = 1UL << current_cpu_data.dcache.waybit;
	unsigned long ws_end = current_cpu_data.dcache.ways <<
	                       current_cpu_data.dcache.waybit;
	unsigned long ws, addr;

	for (ws = 0; ws < ws_end; ws += ws_inc) 
		for (addr = start; addr < end; addr += 0x400) {
			cache32_unroll32(addr|ws,Index_Writeback_Inv_D);
		}
}

static inline void blast_dcache32_page(unsigned long page)
{
	unsigned long start = page;
	unsigned long end = start + PAGE_SIZE;

	do {
		cache32_unroll32(start,Hit_Writeback_Inv_D);
		start += 0x400;
	} while (start < end);
}

static inline void blast_dcache32_page_indexed(unsigned long page)
{
	unsigned long start = page;
	unsigned long end = start + PAGE_SIZE;
	unsigned long ws_inc = 1UL << current_cpu_data.dcache.waybit;
	unsigned long ws_end = current_cpu_data.dcache.ways <<
	                       current_cpu_data.dcache.waybit;
	unsigned long ws, addr;

	for (ws = 0; ws < ws_end; ws += ws_inc) 
		for (addr = start; addr < end; addr += 0x400) {
			cache32_unroll32(addr|ws,Index_Writeback_Inv_D);
		}
}

static inline void blast_icache32(void)
{
	unsigned long start = KSEG0;
	unsigned long end = start + current_cpu_data.icache.waysize;
	unsigned long ws_inc = 1UL << current_cpu_data.icache.waybit;
	unsigned long ws_end = current_cpu_data.icache.ways <<
	                       current_cpu_data.icache.waybit;
	unsigned long ws, addr;

	for (ws = 0; ws < ws_end; ws += ws_inc) 
		for (addr = start; addr < end; addr += 0x400) 
			cache32_unroll32(addr|ws,Index_Invalidate_I);
}

static inline void blast_icache32_page(unsigned long page)
{
	unsigned long start = page;
	unsigned long end = start + PAGE_SIZE;

	BCM4710_FILL_TLB(start);
	do {
		cache32_unroll32(start,Hit_Invalidate_I);
		start += 0x400;
	} while (start < end);
}

static inline void blast_icache32_page_indexed(unsigned long page)
{
	unsigned long start = page;
	unsigned long end = start + PAGE_SIZE;
	unsigned long ws_inc = 1UL << current_cpu_data.icache.waybit;
	unsigned long ws_end = current_cpu_data.icache.ways <<
	                       current_cpu_data.icache.waybit;
	unsigned long ws, addr;

	for (ws = 0; ws < ws_end; ws += ws_inc)
		for (addr = start; addr < end; addr += 0x400) 
			cache32_unroll32(addr|ws,Index_Invalidate_I);
}

static inline void blast_scache32(void)
{
	unsigned long start = KSEG0;
	unsigned long end = start + current_cpu_data.scache.waysize;
	unsigned long ws_inc = 1UL << current_cpu_data.scache.waybit;
	unsigned long ws_end = current_cpu_data.scache.ways << 
	                       current_cpu_data.scache.waybit;
	unsigned long ws, addr;

	for (ws = 0; ws < ws_end; ws += ws_inc) 
		for (addr = start; addr < end; addr += 0x400)
			cache32_unroll32(addr|ws,Index_Writeback_Inv_SD);
}

static inline void blast_scache32_page(unsigned long page)
{
	unsigned long start = page;
	unsigned long end = page + PAGE_SIZE;

	do {
		cache32_unroll32(start,Hit_Writeback_Inv_SD);
		start += 0x400;
	} while (start < end);
}

static inline void blast_scache32_page_indexed(unsigned long page)
{
	unsigned long start = page;
	unsigned long end = start + PAGE_SIZE;
	unsigned long ws_inc = 1UL << current_cpu_data.scache.waybit;
	unsigned long ws_end = current_cpu_data.scache.ways <<
	                       current_cpu_data.scache.waybit;
	unsigned long ws, addr;

	for (ws = 0; ws < ws_end; ws += ws_inc) 
		for (addr = start; addr < end; addr += 0x400) 
			cache32_unroll32(addr|ws,Index_Writeback_Inv_SD);
}

#define cache64_unroll32(base,op)					\
	__asm__ __volatile__(						\
	"	.set noreorder					\n"	\
	"	.set mips3					\n"	\
	"	cache %1, 0x000(%0); cache %1, 0x040(%0)	\n"	\
	"	cache %1, 0x080(%0); cache %1, 0x0c0(%0)	\n"	\
	"	cache %1, 0x100(%0); cache %1, 0x140(%0)	\n"	\
	"	cache %1, 0x180(%0); cache %1, 0x1c0(%0)	\n"	\
	"	cache %1, 0x200(%0); cache %1, 0x240(%0)	\n"	\
	"	cache %1, 0x280(%0); cache %1, 0x2c0(%0)	\n"	\
	"	cache %1, 0x300(%0); cache %1, 0x340(%0)	\n"	\
	"	cache %1, 0x380(%0); cache %1, 0x3c0(%0)	\n"	\
	"	cache %1, 0x400(%0); cache %1, 0x440(%0)	\n"	\
	"	cache %1, 0x480(%0); cache %1, 0x4c0(%0)	\n"	\
	"	cache %1, 0x500(%0); cache %1, 0x540(%0)	\n"	\
	"	cache %1, 0x580(%0); cache %1, 0x5c0(%0)	\n"	\
	"	cache %1, 0x600(%0); cache %1, 0x640(%0)	\n"	\
	"	cache %1, 0x680(%0); cache %1, 0x6c0(%0)	\n"	\
	"	cache %1, 0x700(%0); cache %1, 0x740(%0)	\n"	\
	"	cache %1, 0x780(%0); cache %1, 0x7c0(%0)	\n"	\
	"	.set mips0					\n"	\
	"	.set reorder					\n"	\
		:							\
		: "r" (base),						\
		  "i" (op));

static inline void blast_icache64(void)
{
	unsigned long start = KSEG0;
	unsigned long end = start + current_cpu_data.icache.waysize;
	unsigned long ws_inc = 1UL << current_cpu_data.icache.waybit;
	unsigned long ws_end = current_cpu_data.icache.ways <<
	                       current_cpu_data.icache.waybit;
	unsigned long ws, addr;

	for (ws = 0; ws < ws_end; ws += ws_inc) 
		for (addr = start; addr < end; addr += 0x800) 
			cache64_unroll32(addr|ws,Index_Invalidate_I);
}

static inline void blast_icache64_page(unsigned long page)
{
	unsigned long start = page;
	unsigned long end = start + PAGE_SIZE;

	BCM4710_FILL_TLB(start);
	do {
		cache64_unroll32(start,Hit_Invalidate_I);
		start += 0x800;
	} while (start < end);
}

static inline void blast_icache64_page_indexed(unsigned long page)
{
	unsigned long start = page;
	unsigned long end = start + PAGE_SIZE;
	unsigned long ws_inc = 1UL << current_cpu_data.icache.waybit;
	unsigned long ws_end = current_cpu_data.icache.ways <<
	                       current_cpu_data.icache.waybit;
	unsigned long ws, addr;

	for (ws = 0; ws < ws_end; ws += ws_inc)
		for (addr = start; addr < end; addr += 0x800) 
			cache64_unroll32(addr|ws,Index_Invalidate_I);
}

static inline void blast_scache64(void)
{
	unsigned long start = KSEG0;
	unsigned long end = start + current_cpu_data.scache.waysize;
	unsigned long ws_inc = 1UL << current_cpu_data.scache.waybit;
	unsigned long ws_end = current_cpu_data.scache.ways << 
	                       current_cpu_data.scache.waybit;
	unsigned long ws, addr;

	for (ws = 0; ws < ws_end; ws += ws_inc) 
		for (addr = start; addr < end; addr += 0x800)
			cache64_unroll32(addr|ws,Index_Writeback_Inv_SD);
}

static inline void blast_scache64_page(unsigned long page)
{
	unsigned long start = page;
	unsigned long end = page + PAGE_SIZE;

	do {
		cache64_unroll32(start,Hit_Writeback_Inv_SD);
		start += 0x800;
	} while (start < end);
}

static inline void blast_scache64_page_indexed(unsigned long page)
{
	unsigned long start = page;
	unsigned long end = start + PAGE_SIZE;
	unsigned long ws_inc = 1UL << current_cpu_data.scache.waybit;
	unsigned long ws_end = current_cpu_data.scache.ways <<
	                       current_cpu_data.scache.waybit;
	unsigned long ws, addr;

	for (ws = 0; ws < ws_end; ws += ws_inc) 
		for (addr = start; addr < end; addr += 0x800) 
			cache64_unroll32(addr|ws,Index_Writeback_Inv_SD);
}

#define cache128_unroll32(base,op)					\
	__asm__ __volatile__(						\
	"	.set noreorder					\n"	\
	"	.set mips3					\n"	\
	"	cache %1, 0x000(%0); cache %1, 0x080(%0)	\n"	\
	"	cache %1, 0x100(%0); cache %1, 0x180(%0)	\n"	\
	"	cache %1, 0x200(%0); cache %1, 0x280(%0)	\n"	\
	"	cache %1, 0x300(%0); cache %1, 0x380(%0)	\n"	\
	"	cache %1, 0x400(%0); cache %1, 0x480(%0)	\n"	\
	"	cache %1, 0x500(%0); cache %1, 0x580(%0)	\n"	\
	"	cache %1, 0x600(%0); cache %1, 0x680(%0)	\n"	\
	"	cache %1, 0x700(%0); cache %1, 0x780(%0)	\n"	\
	"	cache %1, 0x800(%0); cache %1, 0x880(%0)	\n"	\
	"	cache %1, 0x900(%0); cache %1, 0x980(%0)	\n"	\
	"	cache %1, 0xa00(%0); cache %1, 0xa80(%0)	\n"	\
	"	cache %1, 0xb00(%0); cache %1, 0xb80(%0)	\n"	\
	"	cache %1, 0xc00(%0); cache %1, 0xc80(%0)	\n"	\
	"	cache %1, 0xd00(%0); cache %1, 0xd80(%0)	\n"	\
	"	cache %1, 0xe00(%0); cache %1, 0xe80(%0)	\n"	\
	"	cache %1, 0xf00(%0); cache %1, 0xf80(%0)	\n"	\
	"	.set mips0					\n"	\
	"	.set reorder					\n"	\
		:							\
		: "r" (base),						\
		  "i" (op));

static inline void blast_scache128(void)
{
	unsigned long start = KSEG0;
	unsigned long end = start + current_cpu_data.scache.waysize;
	unsigned long ws_inc = 1UL << current_cpu_data.scache.waybit;
	unsigned long ws_end = current_cpu_data.scache.ways << 
	                       current_cpu_data.scache.waybit;
	unsigned long ws, addr;

	for (ws = 0; ws < ws_end; ws += ws_inc) 
		for (addr = start; addr < end; addr += 0x1000)
			cache128_unroll32(addr|ws,Index_Writeback_Inv_SD);
}

static inline void blast_scache128_page(unsigned long page)
{
	unsigned long start = page;
	unsigned long end = page + PAGE_SIZE;

	do {
		cache128_unroll32(start,Hit_Writeback_Inv_SD);
		start += 0x1000;
	} while (start < end);
}

static inline void blast_scache128_page_indexed(unsigned long page)
{
	unsigned long start = page;
	unsigned long end = start + PAGE_SIZE;
	unsigned long ws_inc = 1UL << current_cpu_data.scache.waybit;
	unsigned long ws_end = current_cpu_data.scache.ways <<
	                       current_cpu_data.scache.waybit;
	unsigned long ws, addr;

	for (ws = 0; ws < ws_end; ws += ws_inc) 
		for (addr = start; addr < end; addr += 0x1000) 
			cache128_unroll32(addr|ws,Index_Writeback_Inv_SD);
}

extern inline void fill_icache_line(unsigned long addr)
{       
	__asm__ __volatile__(
		".set noreorder\n\t"
		".set mips3\n\t"
		"cache %1, (%0)\n\t"
		".set mips0\n\t"
		".set reorder"
		:
		: "r" (addr),
		"i" (Fill));
}      

#endif /* __ASM_R4KCACHE_H */
