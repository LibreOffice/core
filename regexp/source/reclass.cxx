/* Extended regular expression matching and search library,
   version 0.12.
   (Implements POSIX draft P1003.2/D11.2, except for some of the
   internationalization features.)
   Copyright (C) 1993, 94, 95, 96, 97, 98, 99 Free Software Foundation, Inc.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/*
    Modified for OpenOffice.org to use sal_Unicode and Transliteration service.
 */


#if 0
/* If for any reason (porting, debug) we can't use alloca() use malloc()
   instead.  Use alloca() if possible for performance reasons, this _is_
   significant, with malloc() the re_match2() method makes heavy use of regexps
   through the TextSearch interface up to three times slower.  This is _the_
   bottleneck in some spreadsheet documents.  */
#define REGEX_MALLOC
#endif

/* AIX requires this to be the first thing in the file. */
#if defined _AIX && !defined REGEX_MALLOC
  #pragma alloca
#endif

#include <string.h>
#include <assert.h>

#include <rtl/ustring.hxx>
#include <com/sun/star/i18n/TransliterationModules.hpp>

#include "reclass.hxx"


/* Maximum number of duplicates an interval can allow.  Some systems
   (erroneously) define this in other header files, but we want our
   value, so remove any previous define.  */
#ifdef RE_DUP_MAX
# undef RE_DUP_MAX
#endif
/* If sizeof(int) == 2, then ((1 << 15) - 1) overflows.  */
#define RE_DUP_MAX (0x7fff)


/* If `regs_allocated' is REGS_UNALLOCATED in the pattern buffer,
   `re_match_2' returns information about at least this many registers
   the first time a `regs' structure is passed.  */
#ifndef RE_NREGS
# define RE_NREGS 30
#endif


// Macros
#define INIT_COMPILE_STACK_SIZE     32
#define INIT_BUF_SIZE           ((1 << BYTEWIDTH)/BYTEWIDTH)
#define MAX_BUF_SIZE            65535L
#define NO_HIGHEST_ACTIVE_REG       (1 << BYTEWIDTH)
#define NO_LOWEST_ACTIVE_REG        (NO_HIGHEST_ACTIVE_REG + 1)

/* Since we have one byte reserved for the register number argument to
   {start,stop}_memory, the maximum number of groups we can report
   things about is what fits in that byte.  */
#define MAX_REGNUM 255

#define MIN(x, y) ( (x) < (y) ? (x) : (y) )
#define MAX(x, y) ( (x) > (y) ? (x) : (y) )


// Always. We're not in Emacs and don't use relocating allocators.
#define MATCH_MAY_ALLOCATE

/* Should we use malloc or alloca?  If REGEX_MALLOC is not defined, we
   use `alloca' instead of `malloc'.  This is because malloc is slower and
   causes storage fragmentation.  On the other hand, malloc is more portable,
   and easier to debug.

   Because we sometimes use alloca, some routines have to be macros,
   not functions -- `alloca'-allocated space disappears at the end of the
   function it is called in.  */

#ifdef REGEX_MALLOC

# define REGEX_ALLOCATE malloc
# define REGEX_REALLOCATE(source, osize, nsize) realloc (source, nsize)
# define REGEX_FREE free

#else /* not REGEX_MALLOC  */

/* Emacs already defines alloca, sometimes. So does MSDEV.  */
# ifndef alloca

/* Make alloca work the best possible way.  */
#  ifdef __GNUC__
#   define alloca __builtin_alloca
#  else /* not __GNUC__ */
#   if defined( FREEBSD )
#    include <stdlib.h>
#   elif defined( WNT )
#    include <malloc.h>
#   else
#    include <alloca.h>
#   endif /* HAVE_ALLOCA_H */
#  endif /* not __GNUC__ */

# endif /* not alloca */

# define REGEX_ALLOCATE alloca

/* Assumes a `char *destination' variable.  */
# define REGEX_REALLOCATE(source, osize, nsize)             \
  (destination = (char *) alloca (nsize),               \
   memcpy (destination, source, osize))

/* No need to do anything to free, after alloca.  */
# define REGEX_FREE(arg) ((void)0) /* Do nothing!  But inhibit gcc warning.  */

#endif /* not REGEX_MALLOC */


/* Define how to allocate the failure stack.  */

#ifdef REGEX_MALLOC

# define REGEX_ALLOCATE_STACK malloc
# define REGEX_REALLOCATE_STACK(source, osize, nsize) realloc (source, nsize)
# define REGEX_FREE_STACK free

#else /* not REGEX_MALLOC */

# define REGEX_ALLOCATE_STACK alloca

# define REGEX_REALLOCATE_STACK(source, osize, nsize)           \
   REGEX_REALLOCATE (source, osize, nsize)
/* No need to explicitly free anything.  */
# define REGEX_FREE_STACK(arg)

#endif /* not REGEX_MALLOC */


/* (Re)Allocate N items of type T using malloc, or fail.  */
#define TALLOC(n, t) ((t *) malloc ((n) * sizeof (t)))
#define RETALLOC(addr, n, t) ((addr) = (t *) realloc (addr, (n) * sizeof (t)))
#define RETALLOC_IF(addr, n, t) \
  if (addr) RETALLOC((addr), (n), t); else (addr) = TALLOC ((n), t)
#define REGEX_TALLOC(n, t) ((t *) REGEX_ALLOCATE ((n) * sizeof (t)))

#define BYTEWIDTH 16    /* In bits (assuming sizeof(sal_Unicode)*8) */


#define CHAR_CLASS_MAX_LENGTH 256

/* Fetch the next character in the uncompiled pattern, with no
   translation.  */
#define PATFETCH_RAW(c)                                                 \
    do {                                \
        if (p == pend) return REG_EEND;                         \
        c = (sal_Unicode) *p++;                                 \
    } while (0)

/* Go backwards one character in the pattern.  */
#define PATUNFETCH p--

#define FREE_STACK_RETURN(value)                    \
    return(free(compile_stack.stack), value)

#define GET_BUFFER_SPACE(n)                     \
    while ((sal_uInt32)(b - bufp->buffer + (n)) > bufp->allocated)  \
        EXTEND_BUFFER()

/* Extend the buffer by twice its current size via realloc and
   reset the pointers that pointed into the old block to point to the
   correct places in the new one.  If extending the buffer results in it
   being larger than MAX_BUF_SIZE, then flag memory exhausted.  */
#define EXTEND_BUFFER()                                                 \
  do {                                                                  \
    sal_Unicode *old_buffer = bufp->buffer;                           \
    if (bufp->allocated == MAX_BUF_SIZE)                                \
      return REG_ESIZE;                                                 \
    bufp->allocated <<= 1;                                              \
    if (bufp->allocated > MAX_BUF_SIZE)                                 \
      bufp->allocated = MAX_BUF_SIZE;                                   \
    bufp->buffer = (sal_Unicode *) realloc(bufp->buffer,        \
                       bufp->allocated *        \
                       sizeof(sal_Unicode));    \
    if (bufp->buffer == NULL)                                           \
      return REG_ESPACE;                                                \
    /* If the buffer moved, move all the pointers into it.  */          \
    if (old_buffer != bufp->buffer) {                                   \
        b = (b - old_buffer) + bufp->buffer;                            \
        begalt = (begalt - old_buffer) + bufp->buffer;                  \
        if (fixup_alt_jump)                                             \
          fixup_alt_jump = (fixup_alt_jump - old_buffer) + bufp->buffer;\
        if (laststart)                                                  \
          laststart = (laststart - old_buffer) + bufp->buffer;          \
        if (pending_exact)                                              \
          pending_exact = (pending_exact - old_buffer) + bufp->buffer;  \
      }                                                                 \
  } while (0)

#define BUF_PUSH(c)                         \
    do {                                \
        GET_BUFFER_SPACE(1);                    \
        *b++ = (sal_Unicode)(c);                \
    } while(0)

/* Ensure we have two more bytes of buffer space and then append C1 and C2.  */
#define BUF_PUSH_2(c1, c2)                                              \
  do {                                                                  \
    GET_BUFFER_SPACE(2);                        \
    *b++ = (sal_Unicode) (c1);                                          \
    *b++ = (sal_Unicode) (c2);                                          \
  } while (0)

/* As with BUF_PUSH_2, except for three bytes.  */
#define BUF_PUSH_3(c1, c2, c3)                                          \
  do {                                                                  \
    GET_BUFFER_SPACE(3);                        \
    *b++ = (sal_Unicode) (c1);                                          \
    *b++ = (sal_Unicode) (c2);                                          \
    *b++ = (sal_Unicode) (c3);                                          \
  } while (0)

/* Store a jump with opcode OP at LOC to location TO.  We store a
   relative address offset by the three bytes the jump itself occupies.  */
#define STORE_JUMP(op, loc, to)                     \
    store_op1(op, loc, (int) ((to) - (loc) - 3))

/* Likewise, for a two-argument jump.  */
#define STORE_JUMP2(op, loc, to, arg)                   \
    store_op2(op, loc, (int) ((to) - (loc) - 3), arg)

/* Store NUMBER in two contiguous sal_Unicode starting at DESTINATION.  */

inline
void
Regexpr::store_number( sal_Unicode * destination, sal_Int32 number )
{
  (destination)[0] = sal_Unicode((number) & 0xffff);
  (destination)[1] = sal_Unicode((number) >> 16);
}

/* Same as STORE_NUMBER, except increment DESTINATION to
   the byte after where the number is stored.  Therefore, DESTINATION
   must be an lvalue.  */

inline
void
Regexpr::store_number_and_incr( sal_Unicode *& destination, sal_Int32 number )
{
  store_number( destination, number );
  (destination) += 2;
}

/* Put into DESTINATION a number stored in two contiguous sal_Unicode starting
   at SOURCE.  */

inline void Regexpr::extract_number( sal_Int32 & dest, sal_Unicode *source )
{
  dest = (((sal_Int32) source[1]) << 16) | (source[0] & 0xffff);
}

/* Like `STORE_JUMP', but for inserting.  Assume `b' is the buffer end.  */
#define INSERT_JUMP(op, loc, to)                    \
    insert_op1(op, loc, (sal_Int32) ((to) - (loc) - 3), b)

/* Like `STORE_JUMP2', but for inserting.  Assume `b' is the buffer end.  */
#define INSERT_JUMP2(op, loc, to, arg)                  \
    insert_op2(op, loc, (sal_Int32) ((to) - (loc) - 3), arg, b)

#define STREQ(s1, s2) (rtl_ustr_compare((s1), (s2)) ? (0) : (1))

#define COMPILE_STACK_EMPTY  (compile_stack.avail == 0)
#define COMPILE_STACK_FULL  (compile_stack.avail == compile_stack.size)

/* The next available element.  */
#define COMPILE_STACK_TOP (compile_stack.stack[compile_stack.avail])

/* Get the next unsigned number in the uncompiled pattern.  */
#define GET_UNSIGNED_NUMBER(num) {                                      \
    if (p != pend) {                                                \
        PATFETCH_RAW(c);                    \
        while (c >= (sal_Unicode)'0' && c <= (sal_Unicode)'9') {    \
            if (num < 0)                                    \
                num = 0;                                \
            num = num * 10 + c - (sal_Unicode)'0';      \
            if (p == pend)                  \
                break;                  \
            PATFETCH_RAW(c);                \
        }                                                       \
    }                                                               \
}

/* Get the next hex number in the uncompiled pattern.  */
#define GET_HEX_NUMBER(num) {                                       \
    if (p != pend) {                                                \
        sal_Bool stop = false;                  \
        sal_Int16 hexcnt = 1;                   \
        PATFETCH_RAW(c);                    \
        while ( (c >= (sal_Unicode)'0' && c <= (sal_Unicode)'9') || (c >= (sal_Unicode)'a' && c <= (sal_Unicode)'f') || (c >= (sal_Unicode)'A' && c <= (sal_Unicode)'F') ) {    \
            if (num < 0)                                    \
                num = 0;                                \
            if ( c >= (sal_Unicode)'0' && c <= (sal_Unicode)'9' ) \
                num = num * 16 + c - (sal_Unicode)'0';      \
            else if ( c >= (sal_Unicode)'a' && c <= (sal_Unicode)'f' ) \
                num = num * 16 + (10 + c - (sal_Unicode)'a');       \
            else                        \
                num = num * 16 + (10 + c - (sal_Unicode)'A');       \
            if (p == pend || hexcnt == 4) {         \
                stop = true;                \
                break;                  \
            }                       \
            PATFETCH_RAW(c);                \
            hexcnt++;                   \
        }                                                       \
                                    \
        if ( ! stop ) {                     \
            PATUNFETCH;                 \
            hexcnt--;                   \
        }                           \
        if ( hexcnt > 4 || (num < 0 || num > 0xffff) ) num = -1;\
    }                                                               \
}


/* Number of failure points for which to initially allocate space
   when matching.  If this number is exceeded, we allocate more
   space, so it is not a hard limit.  */
#ifndef INIT_FAILURE_ALLOC
# define INIT_FAILURE_ALLOC 5
#endif

#define INIT_FAIL_STACK()                       \
  do {                                  \
    fail_stack.stack = (fail_stack_elt_t *)             \
      REGEX_ALLOCATE_STACK (INIT_FAILURE_ALLOC * sizeof (fail_stack_elt_t)); \
                                    \
    if (fail_stack.stack == NULL)                   \
      return -2;                            \
                                    \
    fail_stack.size = INIT_FAILURE_ALLOC;               \
    fail_stack.avail = 0;                       \
  } while (0)

#define RESET_FAIL_STACK()  REGEX_FREE_STACK (fail_stack.stack)

/* Double the size of FAIL_STACK, up to approximately `re_max_failures' items.

   Return 1 if succeeds, and 0 if either ran out of memory
   allocating space for it or it was already too large.

   REGEX_REALLOCATE_STACK requires `destination' be declared.   */

#define DOUBLE_FAIL_STACK(fail_stack)                   \
  ((fail_stack).size > (sal_uInt32) (re_max_failures * MAX_FAILURE_ITEMS)   \
   ? 0                                  \
   : ((fail_stack).stack = (fail_stack_elt_t *)             \
        REGEX_REALLOCATE_STACK ((fail_stack).stack,             \
          (fail_stack).size * sizeof (fail_stack_elt_t),        \
          ((fail_stack).size << 1) * sizeof (fail_stack_elt_t)),    \
                                    \
      (fail_stack).stack == NULL                    \
      ? 0                               \
      : ((fail_stack).size <<= 1,                   \
         1)))


#define REG_UNSET_VALUE (&reg_unset_dummy)
#define REG_UNSET(e) ((e) == REG_UNSET_VALUE)

#define REG_MATCH_NULL_STRING_P(R)  ((R).bits.match_null_string_p)
#define IS_ACTIVE(R)  ((R).bits.is_active)
#define MATCHED_SOMETHING(R)  ((R).bits.matched_something)
#define EVER_MATCHED_SOMETHING(R)  ((R).bits.ever_matched_something)

/* Call this when have matched a real character; it sets `matched' flags
   for the subexpressions which we are currently inside.  Also records
   that those subexprs have matched.  */
#define SET_REGS_MATCHED()                                              \
  do {                                  \
      if (!set_regs_matched_done) {                 \
          sal_uInt32 r;                         \
          set_regs_matched_done = 1;                                    \
          for (r = lowest_active_reg; r <= highest_active_reg; r++) {   \
              MATCHED_SOMETHING(reg_info[r])                \
                = EVER_MATCHED_SOMETHING(reg_info[r])           \
                = 1;                                                    \
            }                                                           \
        }                                                               \
    }                                                                   \
  while (0)

#define FAIL_STACK_EMPTY()     (fail_stack.avail == 0)

/* This converts PTR, a pointer into the search string `string2' into an offset from the beginning of that string.  */
#define POINTER_TO_OFFSET(ptr) ((sal_Int32) ((ptr) - string2))

/* This is the number of items that are pushed and popped on the stack
   for each register.  */
#define NUM_REG_ITEMS  3

/* Individual items aside from the registers.  */
# define NUM_NONREG_ITEMS 4

/* We push at most this many items on the stack.  */
/* We used to use (num_regs - 1), which is the number of registers
   this regexp will save; but that was changed to 5
   to avoid stack overflow for a regexp with lots of parens.  */
#define MAX_FAILURE_ITEMS (5 * NUM_REG_ITEMS + NUM_NONREG_ITEMS)

/* We actually push this many items.  */
#define NUM_FAILURE_ITEMS                               \
  (((0                                                  \
     ? 0 : highest_active_reg - lowest_active_reg + 1)  \
    * NUM_REG_ITEMS)                                    \
   + NUM_NONREG_ITEMS)

/* How many items can still be added to the stack without overflowing it.  */
#define REMAINING_AVAIL_SLOTS ((fail_stack).size - (fail_stack).avail)

/* Push a pointer value onto the failure stack.
   Assumes the variable `fail_stack'.  Probably should only
   be called from within `PUSH_FAILURE_POINT'.  */
#define PUSH_FAILURE_POINTER(item)                                      \
  fail_stack.stack[fail_stack.avail++].pointer = (sal_Unicode *) (item)

/* This pushes an integer-valued item onto the failure stack.
   Assumes the variable `fail_stack'.  Probably should only
   be called from within `PUSH_FAILURE_POINT'.  */
#define PUSH_FAILURE_INT(item)                                  \
  fail_stack.stack[fail_stack.avail++].integer = (item)

/* Push a fail_stack_elt_t value onto the failure stack.
   Assumes the variable `fail_stack'.  Probably should only
   be called from within `PUSH_FAILURE_POINT'.  */
#define PUSH_FAILURE_ELT(item)                                  \
  fail_stack.stack[fail_stack.avail++] =  (item)

/* These three POP... operations complement the three PUSH... operations.
   All assume that `fail_stack' is nonempty.  */
#define POP_FAILURE_POINTER() fail_stack.stack[--fail_stack.avail].pointer
#define POP_FAILURE_INT() fail_stack.stack[--fail_stack.avail].integer
#define POP_FAILURE_ELT() fail_stack.stack[--fail_stack.avail]

/* Test if at very beginning or at very end of `string2'. */
#define AT_STRINGS_BEG(d) ((d) == string2 || !size2)
#define AT_STRINGS_END(d) ((d) == end2)

/* Checking for end of string */
#define PREFETCH() \
do { \
    if ( d == end2 ) { \
        goto fail; \
    } \
} while (0)


sal_Bool
Regexpr::iswordbegin(const sal_Unicode *d, sal_Unicode *string, sal_Int32 ssize)
{
   if ( d == string || ! ssize ) return true;

   if ( !unicode::isAlphaDigit(d[-1]) && unicode::isAlphaDigit(d[0])) {
    return true;
   }
   return false;
}

sal_Bool
Regexpr::iswordend(const sal_Unicode *d, sal_Unicode *string, sal_Int32 ssize)
{
   if ( d == (string+ssize) ) return true;

   if ( !unicode::isAlphaDigit(d[0]) && unicode::isAlphaDigit(d[-1])) {
    return true;
   }
   return false;
}

/* Push the information about the state we will need
   if we ever fail back to it.

   Requires variables fail_stack, regstart, regend, and reg_info
   be declared.  DOUBLE_FAIL_STACK requires `destination'
   be declared.

   Does `return FAILURE_CODE' if runs out of memory.  */

#define PUSH_FAILURE_POINT(pattern_place, string_place, failure_code)   \
  do {                                                                  \
    char *destination;                          \
    /* Must be int, so when we don't save any registers, the arithmetic \
       of 0 + -1 isn't done as unsigned.  */                            \
    /* Can't be int, since there is not a shred of a guarantee that int \
       is wide enough to hold a value of something to which pointer can \
       be assigned */                                                   \
    sal_uInt32 this_reg;                                                \
                                                                        \
    /* Ensure we have enough space allocated for what we will push.  */ \
    while (REMAINING_AVAIL_SLOTS < NUM_FAILURE_ITEMS) {                  \
        if (!DOUBLE_FAIL_STACK(fail_stack))                            \
          return failure_code;                                          \
      }                                                                 \
                                                                        \
    /* Push the info, starting with the registers.  */                  \
    if (1)                                                              \
      for (this_reg = lowest_active_reg; this_reg <= highest_active_reg; \
           this_reg++) {                                                 \
          PUSH_FAILURE_POINTER(regstart[this_reg]);                    \
                                                                        \
          PUSH_FAILURE_POINTER (regend[this_reg]);                      \
                                                                        \
          PUSH_FAILURE_ELT(reg_info[this_reg].word);                   \
        }                                                               \
                                                                        \
    PUSH_FAILURE_INT(lowest_active_reg);                               \
                                                                        \
    PUSH_FAILURE_INT(highest_active_reg);                              \
                                                                        \
    PUSH_FAILURE_POINTER(pattern_place);                               \
                                                                        \
    PUSH_FAILURE_POINTER(string_place);                                \
                                                                        \
  } while (0)

/* Pops what PUSH_FAIL_STACK pushes.

   We restore into the parameters, all of which should be lvalues:
     STR -- the saved data position.
     PAT -- the saved pattern position.
     LOW_REG, HIGH_REG -- the highest and lowest active registers.
     REGSTART, REGEND -- arrays of string positions.
     REG_INFO -- array of information about each subexpression.

   Also assumes the variables `fail_stack' and (if debugging), `bufp',
   `pend', `string2', and `size2'.  */

#define POP_FAILURE_POINT(str, pat, low_reg, high_reg, regstart, regend, reg_info) {\
    sal_uInt32 this_reg;                                                \
    sal_Unicode *string_temp;                                     \
                                                                        \
  assert(!FAIL_STACK_EMPTY());                                        \
                                                                        \
  /* Remove failure points and point to how many regs pushed.  */       \
  assert(fail_stack.avail >= NUM_NONREG_ITEMS);                        \
                                                                        \
  /* If the saved string location is NULL, it came from an              \
     on_failure_keep_string_jump opcode, and we want to throw away the  \
     saved NULL, thus retaining our current position in the string.  */ \
  string_temp = POP_FAILURE_POINTER();                                 \
  if (string_temp != NULL)                                              \
    str = (const sal_Unicode *) string_temp;                                   \
                                                                        \
  pat = (sal_Unicode *) POP_FAILURE_POINTER();                       \
                                                                        \
  /* Restore register info.  */                                         \
  high_reg = (sal_uInt32) POP_FAILURE_INT();                         \
                                                                        \
  low_reg = (sal_uInt32) POP_FAILURE_INT();                          \
                                                                        \
  if (1)                                                                \
    for (this_reg = high_reg; this_reg >= low_reg; this_reg--) {         \
                                                                        \
        reg_info[this_reg].word = POP_FAILURE_ELT();                   \
                                                                        \
        regend[this_reg] = (const sal_Unicode *) POP_FAILURE_POINTER();       \
                                                                        \
        regstart[this_reg] = (const sal_Unicode *) POP_FAILURE_POINTER();     \
      } else {                                                          \
      for (this_reg = highest_active_reg; this_reg > high_reg; this_reg--) {\
          reg_info[this_reg].word.integer = 0;                          \
          regend[this_reg] = 0;                                         \
          regstart[this_reg] = 0;                                       \
        }                                                               \
      highest_active_reg = high_reg;                                    \
    }                                                                   \
                                                                        \
  set_regs_matched_done = 0;                                            \
} /* POP_FAILURE_POINT */

inline
void
Regexpr::extract_number_and_incr( sal_Int32 & destination, sal_Unicode *& source )
{
  extract_number(destination, source);
  source += 2;
}


inline
void
Regexpr::store_op1(re_opcode_t op, sal_Unicode *loc, sal_Int32 arg)
{
  *loc = (sal_Unicode) op;
  store_number(loc + 1, arg);
}

/* Like `store_op1', but for two two-byte parameters ARG1 and ARG2.  */

inline
void
Regexpr::store_op2(re_opcode_t op, sal_Unicode *loc, sal_Int32 arg1, sal_Int32 arg2)
{
  *loc = (sal_Unicode) op;
  store_number(loc + 1, arg1);
  store_number(loc + 3, arg2);
}

void
Regexpr::insert_op1(re_opcode_t op, sal_Unicode *loc, sal_Int32 arg, sal_Unicode *end)
{
  register sal_Unicode *pfrom = end;
  register sal_Unicode *pto = end + 3;

  while (pfrom != loc) {
    *--pto = *--pfrom;
  }

  store_op1(op, loc, arg);
}


/* Like `insert_op1', but for two two-byte parameters ARG1 and ARG2.  */

void
Regexpr::insert_op2(re_opcode_t op, sal_Unicode *loc, sal_Int32 arg1, sal_Int32 arg2, sal_Unicode *end)
{
  register sal_Unicode *pfrom = end;
  register sal_Unicode *pto = end + 5;

  while (pfrom != loc)
    *--pto = *--pfrom;

  store_op2 (op, loc, arg1, arg2);
}

/* P points to just after a ^ in PATTERN.  Return true if that ^ comes
   after an alternative or a begin-subexpression.  We assume there is at
   least one character before the ^.  */

sal_Bool
Regexpr::at_begline_loc_p(const sal_Unicode *pattern, const sal_Unicode *p)
{
  const sal_Unicode *prev = p - 2;
  sal_Bool prev_prev_backslash = prev > pattern && prev[-1] == '\\';

  return(
     /* After a subexpression?  */
     (*prev == (sal_Unicode)'(' && prev_prev_backslash)
     /* After an alternative?  */
     || (*prev == (sal_Unicode)'|' && prev_prev_backslash));
}

/* The dual of at_begline_loc_p.  This one is for $.  We assume there is
   at least one character after the $, i.e., `P < PEND'.  */

sal_Bool
Regexpr::at_endline_loc_p(const sal_Unicode *p, const sal_Unicode *pend)
{
  const sal_Unicode *next = p;
  sal_Bool next_backslash = *next == (sal_Unicode)'\\';
  const sal_Unicode *next_next = p + 1 < pend ? p + 1 : 0;

  return(
     /* Before a subexpression?  */
     *next == (sal_Unicode)')'
     // (next_backslash && next_next && *next_next == (sal_Unicode)')')
     /* Before an alternative?  */
     || *next == (sal_Unicode)'|' );
  //    || (next_backslash && next_next && *next_next == (sal_Unicode)'|'));
}

reg_errcode_t
Regexpr::compile_range(sal_Unicode range_start, sal_Unicode range_end, sal_Unicode *b)
{
  sal_uInt32 this_char;

  /* If the start is after the end, the range is empty.  */
  if (range_start > range_end)
    return REG_NOERROR;

  /* Here we see why `this_char' has to be larger than an `sal_Unicode'
     -- the range is inclusive, so if `range_end' == 0xffff
     (assuming 16-bit characters), we would otherwise go into an infinite
     loop, since all characters <= 0xffff.  */
  for (this_char = range_start; this_char <= range_end; this_char++) {
    set_list_bit( sal_Unicode(this_char), b);
  }

  return REG_NOERROR;
}

/* Returns true if REGNUM is in one of COMPILE_STACK's elements and
   false if it's not.  */

sal_Bool
Regexpr::group_in_compile_stack(compile_stack_type compile_stack, sal_Int32 regnum)
{
  sal_Int32 this_element;

  for (this_element = compile_stack.avail - 1;
       this_element >= 0;
       this_element--) {
    if (compile_stack.stack[this_element].regnum == regnum) {
      return true;
    }
  }

  return false;
}


Regexpr::Regexpr( const ::com::sun::star::util::SearchOptions & rOptions,
         ::com::sun::star::uno::Reference<
         ::com::sun::star::i18n::XExtendedTransliteration > XTrans)
{
  bufp = NULL;
  pattern = NULL;

  if ( rOptions.algorithmType != ::com::sun::star::util::SearchAlgorithms_REGEXP ) {
    return;
  }

  if ( rOptions.searchString == NULL ||
       rOptions.searchString.getLength() <= 0) {
    return;
  }

  pattern = (sal_Unicode *)rOptions.searchString.getStr();
  patsize = rOptions.searchString.getLength();

  re_max_failures = 2000;

  translit = XTrans;
  translate = translit.is() ? 1 : 0;

  bufp = NULL;

  isIgnoreCase = ((rOptions.transliterateFlags &
        ::com::sun::star::i18n::TransliterationModules_IGNORE_CASE) != 0);

  // Compile Regular expression pattern
  if ( regcomp() != REG_NOERROR )
    {
      if ( bufp )
    {
      if ( bufp->buffer )
        free(bufp->buffer);
      if( bufp->fastmap )
        free(bufp->fastmap);

      free(bufp);
      bufp = NULL;
        }
    }
}

Regexpr::~Regexpr()
{
  //    translit->remove();
  if( bufp )
    {
      if( bufp->buffer )
    free(bufp->buffer);
      if( bufp->fastmap )
    free(bufp->fastmap);

      free(bufp);
      bufp = NULL;
    }

}

// sets a new line to search in (restore start/end_ptr)
void
Regexpr::set_line(const sal_Unicode *new_line, sal_Int32 len)
{
  line = new_line;
  linelen = len;
}

// main function for searching the pattern
// returns negative or startpos and sets regs
sal_Int32
Regexpr::re_search(struct re_registers *regs, sal_Int32 pOffset)
{
  // Check if pattern buffer is NULL
  if ( bufp == NULL ) {
    return(-3);
  }

  sal_Int32 range;
  sal_Int32 startpos;
  sal_Int32 stoppos;

  startpos = pOffset;
  if ( linelen < 0 ) {
    range = linelen + 1;
    linelen = -(linelen);
    stoppos = pOffset + 1;
  } else {
    range = linelen - 1;
    stoppos = linelen;
  }
  for ( ; ; ) {
    sal_Int32 val = re_match2(regs, startpos, stoppos);

#ifndef REGEX_MALLOC
# ifdef C_ALLOCA
    alloca (0);
# endif
#endif

    // Return success if match found
    if (val == 0) {
      break;
    }

    if (val == -2) {
      return(-2);
    }

    // If match only beginning of string (startpos)
    if (!range) {
      break;
    }

    // If search match from startpos to startpos+range
    else if (range > 0) {   // Forward string search
      range--;
      startpos++;
    } else {        // Reverse string search
      range++;
      startpos--;
    }
  }

  if ( regs->num_of_match > 0 )
    return(0);
  else
    return(-1);
}

sal_Int32
Regexpr::regcomp()
{
  bufp = (struct re_pattern_buffer *)malloc(sizeof(struct re_pattern_buffer));
  if ( bufp == NULL ) {
    return(-1);
  }

  bufp->buffer = 0;
  bufp->allocated = 0;
  bufp->used = 0;

  //bufp->fastmap = (sal_Unicode*) malloc((1 << BYTEWIDTH) * sizeof(sal_Unicode));
  // No fastmap with Unicode
  bufp->fastmap = NULL;

  return(regex_compile());
}

sal_Int32
Regexpr::regex_compile()
{
  register sal_Unicode c, c1;
  const sal_Unicode *p1;
  register sal_Unicode *b;

  /* Keeps track of unclosed groups.  */
  compile_stack_type compile_stack;

  /* Points to the current (ending) position in the pattern.  */
  const sal_Unicode *p = pattern;
  const sal_Unicode *pend = pattern + patsize;

  /* Address of the count-byte of the most recently inserted `exactn'
     command.  This makes it possible to tell if a new exact-match
     character can be added to that command or if the character requires
     a new `exactn' command.  */
  sal_Unicode *pending_exact = 0;

  /* Address of start of the most recently finished expression.
     This tells, e.g., postfix * where to find the start of its
     operand.  Reset at the beginning of groups and alternatives.  */
  sal_Unicode *laststart = 0;

  /* Address of beginning of regexp, or inside of last group.  */
  sal_Unicode *begalt;

  /* Place in the uncompiled pattern (i.e., the {) to
     which to go back if the interval is invalid.  */
  const sal_Unicode *beg_interval;

  /* Address of the place where a forward jump should go to the end of
     the containing expression.  Each alternative of an `or' -- except the
     last -- ends with a forward jump of this sort.  */
  sal_Unicode *fixup_alt_jump = 0;

  /* Counts open-groups as they are encountered.  Remembered for the
     matching close-group on the compile stack, so the same register
     number is put in the stop_memory as the start_memory.  */
  sal_Int32 regnum = 0;

  /* Initialize the compile stack.  */
  compile_stack.stack = (compile_stack_elt_t *)malloc(INIT_COMPILE_STACK_SIZE * sizeof(compile_stack_elt_t));
  if (compile_stack.stack == NULL)
    return(REG_ESPACE);

  compile_stack.size = INIT_COMPILE_STACK_SIZE;
  compile_stack.avail = 0;

  /* Initialize the pattern buffer.  */
  bufp->fastmap_accurate = 0;
  bufp->not_bol = 0;
  bufp->not_eol = 0;
  bufp->newline_anchor = 1;

  /* Set `used' to zero, so that if we return an error, the pattern
     printer (for debugging) will think there's no pattern.  We reset it
     at the end.  */
  bufp->used = 0;

  /* Always count groups. */
  bufp->re_nsub = 0;

  if (bufp->allocated == 0) {
    if (bufp->buffer) {
      /* If zero allocated, but buffer is non-null, try to realloc
     enough space.  This loses if buffer's address is bogus, but
     that is the user's responsibility.  */
      bufp->buffer = (sal_Unicode *)realloc(bufp->buffer, INIT_BUF_SIZE * sizeof(sal_Unicode));
    } else { /* Caller did not allocate a buffer.  Do it for them.  */
      bufp->buffer = (sal_Unicode *)malloc(INIT_BUF_SIZE * sizeof(sal_Unicode));
    }
    if (!bufp->buffer) FREE_STACK_RETURN(REG_ESPACE);

    bufp->allocated = INIT_BUF_SIZE;
  }

  begalt = b = bufp->buffer;

  /* Loop through the uncompiled pattern until we're at the end.  */
  while (p != pend) {
    PATFETCH_RAW(c);

    switch (c) {
    case (sal_Unicode)'^': {
      if (   /* If at start of pattern, it's an operator.  */
      p == pattern + 1
      /* Otherwise, depends on what's come before.  */
      || at_begline_loc_p(pattern, p))
    BUF_PUSH(begline);
      else
    goto normal_char;
    }
    break;

    case (sal_Unicode)'$': {
      if (   /* If at end of pattern, it's an operator.  */
      p == pend
      /* Otherwise, depends on what's next.  */
      || at_endline_loc_p(p, pend)) {
    BUF_PUSH(endline);
      } else {
    goto normal_char;
      }
    }
    break;

    case (sal_Unicode)'+':
    case (sal_Unicode)'?':
    case (sal_Unicode)'*':
      /* If there is no previous pattern... */
      if (!laststart) {
    goto normal_char;
      }

      {
    /* Are we optimizing this jump?  */
    sal_Bool keep_string_p = false;

    /* 1 means zero (many) matches is allowed.  */
    sal_Unicode zero_times_ok = 0, many_times_ok = 0;

    /* If there is a sequence of repetition chars, collapse it
       down to just one (the right one).  We can't combine
       interval operators with these because of, e.g., `a{2}*',
       which should only match an even number of `a's.  */

    for (;;) {
      zero_times_ok |= c != (sal_Unicode)'+';
      many_times_ok |= c != (sal_Unicode)'?';

      if (p == pend)
        break;

      PATFETCH_RAW(c);

      if (c == (sal_Unicode)'*' || (c == (sal_Unicode)'+'
                    || c == (sal_Unicode)'?')) {
      } else {
        PATUNFETCH;
        break;
      }

      /* If we get here, we found another repeat character.  */
    }

    /* Star, etc. applied to an empty pattern is equivalent
       to an empty pattern.  */
    if (!laststart) {
      break;
    }

    /* Now we know whether or not zero matches is allowed
       and also whether or not two or more matches is allowed.  */
    if (many_times_ok) {
      /* More than one repetition is allowed, so put in at the
         end a backward relative jump from `b' to before the next
         jump we're going to put in below (which jumps from
         laststart to after this jump).

         But if we are at the `*' in the exact sequence `.*\n',
         insert an unconditional jump backwards to the .,
         instead of the beginning of the loop.  This way we only
         push a failure point once, instead of every time
         through the loop.  */
      assert(p - 1 > pattern);

      /* Allocate the space for the jump.  */
      GET_BUFFER_SPACE(3);

      /* We know we are not at the first character of the pattern,
         because laststart was nonzero.  And we've already
         incremented `p', by the way, to be the character after
         the `*'.  Do we have to do something analogous here
         for null bytes, because of RE_DOT_NOT_NULL?  */
      if (*(p - 2) == (sal_Unicode)'.'
          && zero_times_ok
          && p < pend && *p == (sal_Unicode)'\n') {
        /* We have .*\n.  */
        STORE_JUMP(jump, b, laststart);
        keep_string_p = true;
      } else {
        /* Anything else.  */
        STORE_JUMP(maybe_pop_jump, b, laststart - 3);
      }

      /* We've added more stuff to the buffer.  */
      b += 3;
    }

    /* On failure, jump from laststart to b + 3, which will be the
       end of the buffer after this jump is inserted.  */
    GET_BUFFER_SPACE(3);
    INSERT_JUMP(keep_string_p ? on_failure_keep_string_jump
            : on_failure_jump,
            laststart, b + 3);
    pending_exact = 0;
    b += 3;

    if (!zero_times_ok) {
      /* At least one repetition is required, so insert a
         `dummy_failure_jump' before the initial
         `on_failure_jump' instruction of the loop. This
         effects a skip over that instruction the first time
         we hit that loop.  */
      GET_BUFFER_SPACE(3);
      INSERT_JUMP(dummy_failure_jump, laststart, laststart + 6);
      b += 3;
    }
      }
      break;

    case (sal_Unicode)'.':
      laststart = b;
      BUF_PUSH(anychar);
      break;


    case (sal_Unicode)'[': {
      sal_Bool have_range = false;
      sal_Unicode last_char = 0xffff;
      sal_Unicode first_range = 0xffff;
      sal_Unicode second_range = 0xffff;
      sal_Int16 bsiz;

      if (p == pend) FREE_STACK_RETURN(REG_EBRACK);

      /* Ensure that we have enough space to push a charset: the
     opcode, the length count, and the bitset;
     1 + 1 + (1 << BYTEWIDTH) / BYTEWIDTH "bytes" in all.  */
      bsiz = 2 + ((1 << BYTEWIDTH) / BYTEWIDTH);
      GET_BUFFER_SPACE(bsiz);

      laststart = b;

      /* We test `*p == '^' twice, instead of using an if
     statement, so we only need one BUF_PUSH.  */
      BUF_PUSH (*p == (sal_Unicode)'^' ? charset_not : charset);
      if (*p == (sal_Unicode)'^')
    p++;

      /* Remember the first position in the bracket expression.  */
      p1 = p;

      /* Push the number of "bytes" in the bitmap.  */
      BUF_PUSH((1 << BYTEWIDTH) / BYTEWIDTH);

      /* Clear the whole map.  */
      memset(b, 0, ((1 << BYTEWIDTH) / BYTEWIDTH) * sizeof(sal_Unicode));

      /* Read in characters and ranges, setting map bits.  */
      for (;;) {
    if (p == pend) FREE_STACK_RETURN(REG_EBRACK);

    PATFETCH_RAW(c);

    if ( c == (sal_Unicode)'\\' ) {

      PATFETCH_RAW(c);

      if ( c == (sal_Unicode)'x' ) {
        sal_Int32 UniChar = -1;

        GET_HEX_NUMBER(UniChar);
        if (UniChar < 0 || UniChar > 0xffff) FREE_STACK_RETURN(REG_BADPAT);
        c = (sal_Unicode) UniChar;
        last_char = c;
        set_list_bit(last_char, b);
      } else {
        last_char = c;
        set_list_bit(last_char, b);
      }
    } else if (c == (sal_Unicode)']') {
      /* Could be the end of the bracket expression.  If it's
         not (i.e., when the bracket expression is `[]' so
         far), the ']' character bit gets set way below.  */
        break;
    } else if ( c == (sal_Unicode)'-' ) {
      if ( !have_range ) {
        if ( last_char != 0xffff ) {
          first_range = last_char;
          have_range = true;
          continue;
        } else {
          last_char = (sal_Unicode)'-';
          set_list_bit(last_char, b);
        }
      }
        }

    /* See if we're at the beginning of a possible character
       class.  */
    else if (c == (sal_Unicode)':' && p[-2] == (sal_Unicode)'[') {
      /* Leave room for the null.  */
      sal_Unicode str[CHAR_CLASS_MAX_LENGTH + 1];

      PATFETCH_RAW(c);
      c1 = 0;

      /* If pattern is `[[:'.  */
      if (p == pend) FREE_STACK_RETURN(REG_EBRACK);

      str[c1++] = c;
      for (;;) {
        PATFETCH_RAW(c);
        if ((c == (sal_Unicode)':' && *p == (sal_Unicode)']') || p == pend)
          break;
        if (c1 < CHAR_CLASS_MAX_LENGTH)
          str[c1++] = c;
        else
                /* This is in any case an invalid class name.  */
          str[0] = (sal_Unicode)'\0';
      }
      str[c1] = (sal_Unicode)'\0';

      /* If isn't a word bracketed by `[:' and `:]':
         undo the ending character, the letters, and leave
         the leading `:' and `[' (but set bits for them).  */
      if (c == (sal_Unicode)':' && *p == (sal_Unicode)']') {
        sal_Int32 ch;
        // no support for GRAPH, PUNCT, or XDIGIT yet
        sal_Bool is_alnum = STREQ(str, ::rtl::OUString::createFromAscii((const sal_Char*)"alnum").getStr());
        sal_Bool is_alpha = STREQ(str, ::rtl::OUString::createFromAscii((const sal_Char*)"alpha").getStr());
        sal_Bool is_cntrl = STREQ(str, ::rtl::OUString::createFromAscii((const sal_Char*)"cntrl").getStr());
        sal_Bool is_digit = STREQ(str, ::rtl::OUString::createFromAscii((const sal_Char*)"digit").getStr());
        sal_Bool is_lower = STREQ(str, ::rtl::OUString::createFromAscii((const sal_Char*)"lower").getStr());
        sal_Bool is_print = STREQ(str, ::rtl::OUString::createFromAscii((const sal_Char*)"print").getStr());
        sal_Bool is_space = STREQ(str, ::rtl::OUString::createFromAscii((const sal_Char*)"space").getStr());
        sal_Bool is_upper = STREQ(str, ::rtl::OUString::createFromAscii((const sal_Char*)"upper").getStr());

        if (!(is_alnum || is_alpha || is_cntrl ||
          is_digit || is_lower || is_print || is_space || is_upper) )
          FREE_STACK_RETURN(REG_ECTYPE);

        /* Throw away the ] at the end of the character
           class.  */
        PATFETCH_RAW(c);

        if (p == pend) FREE_STACK_RETURN(REG_EBRACK);

        for (ch = 0; ch < 1 << BYTEWIDTH; ch++) {
                /* This was split into 3 if's to
                   avoid an arbitrary limit in some compiler.  */
          if (   (is_alnum  && unicode::isAlphaDigit(sal_Unicode(ch))) ||
             (is_alpha  && unicode::isAlpha(sal_Unicode(ch))) ||
             (is_cntrl  && unicode::isControl(sal_Unicode(ch))))
        set_list_bit(sal_Unicode(ch), b);
          if (   (is_digit  && unicode::isDigit(sal_Unicode(ch))) ||
             (is_lower  && unicode::isLower(sal_Unicode(ch))) ||
             (is_print  && unicode::isPrint(sal_Unicode(ch))))
        set_list_bit(sal_Unicode(ch), b);
          if (   (is_space  && unicode::isSpace(sal_Unicode(ch))) ||
             (is_upper  && unicode::isUpper(sal_Unicode(ch))) )
        set_list_bit(sal_Unicode(ch), b);
          if ( isIgnoreCase && (is_upper || is_lower) &&
             (unicode::isUpper(sal_Unicode(ch)) || unicode::isLower(sal_Unicode(ch))))
        set_list_bit(sal_Unicode(ch), b);
        }
        break;
      } else {
        p = p1+1;
        last_char = (sal_Unicode)':';
        set_list_bit(last_char, b);
      }
    } else {
      last_char = c;
      set_list_bit(last_char, b);
    }
    if ( have_range ) {
      if ( last_char != 0xffff ) {
        second_range = last_char;
        have_range = false;
        compile_range(first_range, second_range, b);
      } else FREE_STACK_RETURN(REG_EBRACK);
    } else {
      if ( last_char != 0xffff ) {
        set_list_bit(last_char, b);
      } else FREE_STACK_RETURN(REG_EBRACK);
    }
      }

      /* Discard any (non)matching list bytes that are all 0 at the
     end of the map.  Decrease the map-length byte too.  */
      bsiz = b[-1];
      while ((sal_Int16) bsiz > 0 && b[bsiz - 1] == 0)
    bsiz--;
      b[-1] = (sal_Unicode)bsiz;
      b += bsiz;
    }
    break;

    case (sal_Unicode)'(':
      goto handle_open;

    case (sal_Unicode)')':
      goto handle_close;

    case (sal_Unicode)'\n':
      goto normal_char;

    case (sal_Unicode)'|':
      goto handle_alt;

    case (sal_Unicode)'{':
      goto handle_interval;

    case (sal_Unicode)'\\':
      if (p == pend) FREE_STACK_RETURN(REG_EESCAPE);

      /* Do not translate the character after the \, so that we can
     distinguish, e.g., \B from \b, even if we normally would
     translate, e.g., B to b.  */
      PATFETCH_RAW(c);

      switch (c) {
      case (sal_Unicode)'(':
    goto normal_backslash;

      handle_open:
    bufp->re_nsub++;
    regnum++;

    if (COMPILE_STACK_FULL) {
      compile_stack.stack = (compile_stack_elt_t *)realloc(compile_stack.stack, (compile_stack.size << 1) * sizeof(compile_stack_elt_t));
      if (compile_stack.stack == NULL) return(REG_ESPACE);

      compile_stack.size <<= 1;
    }

    /* These are the values to restore when we hit end of this
       group.  They are all relative offsets, so that if the
       whole pattern moves because of realloc, they will still
       be valid.  */
    COMPILE_STACK_TOP.begalt_offset = begalt - bufp->buffer;
    COMPILE_STACK_TOP.fixup_alt_jump
      = fixup_alt_jump ? fixup_alt_jump - bufp->buffer + 1 : 0;
    COMPILE_STACK_TOP.laststart_offset = b - bufp->buffer;
    COMPILE_STACK_TOP.regnum = regnum;

    /* We will eventually replace the 0 with the number of
       groups inner to this one.  But do not push a
       start_memory for groups beyond the last one we can
       represent in the compiled pattern.  */
    if (regnum <= MAX_REGNUM) {
      COMPILE_STACK_TOP.inner_group_offset = b - bufp->buffer + 2;
      BUF_PUSH_3 (start_memory, regnum, 0);
    }

    compile_stack.avail++;

    fixup_alt_jump = 0;
    laststart = 0;
    begalt = b;
    /* If we've reached MAX_REGNUM groups, then this open
       won't actually generate any code, so we'll have to
       clear pending_exact explicitly.  */
    pending_exact = 0;
    break;


      case (sal_Unicode)')':
    goto normal_backslash;

    if (COMPILE_STACK_EMPTY) {
      FREE_STACK_RETURN(REG_ERPAREN);
    }

      handle_close:
    if (fixup_alt_jump) {
      /* Push a dummy failure point at the end of the
         alternative for a possible future
         `pop_failure_jump' to pop.  See comments at
         `push_dummy_failure' in `re_match2'.  */
      BUF_PUSH(push_dummy_failure);

      /* We allocated space for this jump when we assigned
         to `fixup_alt_jump', in the `handle_alt' case below.  */
      STORE_JUMP(jump_past_alt, fixup_alt_jump, b - 1);
    }

    /* See similar code for backslashed left paren above.  */
    if (COMPILE_STACK_EMPTY) {
      FREE_STACK_RETURN(REG_ERPAREN);
    }

    /* Since we just checked for an empty stack above, this
       ``can't happen''.  */
    assert (compile_stack.avail != 0);

    {
      /* We don't just want to restore into `regnum', because
         later groups should continue to be numbered higher,
         as in `(ab)c(de)' -- the second group is #2.  */
      sal_Int32 this_group_regnum;

      compile_stack.avail--;
      begalt = bufp->buffer + COMPILE_STACK_TOP.begalt_offset;
      fixup_alt_jump
        = COMPILE_STACK_TOP.fixup_alt_jump
        ? bufp->buffer + COMPILE_STACK_TOP.fixup_alt_jump - 1
        : 0;
      laststart = bufp->buffer + COMPILE_STACK_TOP.laststart_offset;
      this_group_regnum = COMPILE_STACK_TOP.regnum;
      /* If we've reached MAX_REGNUM groups, then this open
         won't actually generate any code, so we'll have to
         clear pending_exact explicitly.  */
      pending_exact = 0;

      /* We're at the end of the group, so now we know how many
         groups were inside this one.  */
      if (this_group_regnum <= MAX_REGNUM) {
        sal_Unicode *inner_group_loc
          = bufp->buffer + COMPILE_STACK_TOP.inner_group_offset;

        *inner_group_loc = regnum - this_group_regnum;
        BUF_PUSH_3 (stop_memory, this_group_regnum,
            regnum - this_group_regnum);
      }
    }
    break;


      case (sal_Unicode)'|':            /* `\|'.
                         * */
    goto normal_backslash;
      handle_alt:

    /* Insert before the previous alternative a jump which
       jumps to this alternative if the former fails.  */
    GET_BUFFER_SPACE (3);
    INSERT_JUMP (on_failure_jump, begalt, b + 6);
    pending_exact = 0;
    b += 3;

    /* The alternative before this one has a jump after it
       which gets executed if it gets matched.  Adjust that
       jump so it will jump to this alternative's analogous
       jump (put in below, which in turn will jump to the next
       (if any) alternative's such jump, etc.).  The last such
       jump jumps to the correct final destination.  A picture:
       _____ _____
       |   | |   |
       |   v |   v
       a | b   | c

       If we are at `b', then fixup_alt_jump right now points to a
       three-byte space after `a'.  We'll put in the jump, set
       fixup_alt_jump to right after `b', and leave behind three
       bytes which we'll fill in when we get to after `c'.  */

    if (fixup_alt_jump)
      STORE_JUMP (jump_past_alt, fixup_alt_jump, b);

    /* Mark and leave space for a jump after this alternative,
       to be filled in later either by next alternative or
       when know we're at the end of a series of alternatives.  */
    fixup_alt_jump = b;
    GET_BUFFER_SPACE (3);
    b += 3;

    laststart = 0;
    begalt = b;
    break;


      case (sal_Unicode)'{':
    goto normal_backslash;

      handle_interval:
    {
      /* allows intervals.  */
      /* At least (most) this many matches must be made.  */
      sal_Int32 lower_bound = -1, upper_bound = -1;

      beg_interval = p - 1;

      if (p == pend) {
        goto unfetch_interval;
      }

      GET_UNSIGNED_NUMBER(lower_bound);

      if (c == (sal_Unicode)',') {
        GET_UNSIGNED_NUMBER(upper_bound);
        if (upper_bound < 0) upper_bound = RE_DUP_MAX;
      } else
        /* Interval such as `{1}' => match exactly once. */
        upper_bound = lower_bound;

      if (lower_bound < 0 || upper_bound > RE_DUP_MAX
          || lower_bound > upper_bound) {
        goto unfetch_interval;
      }

      if (c != (sal_Unicode)'}') {
        goto unfetch_interval;
      }

      /* We just parsed a valid interval.  */

      /* If it's invalid to have no preceding re.  */
      if (!laststart) {
        goto unfetch_interval;
      }

      /* If the upper bound is zero, don't want to succeed at
         all; jump from `laststart' to `b + 3', which will be
         the end of the buffer after we insert the jump.  */
      if (upper_bound == 0) {
        GET_BUFFER_SPACE(3);
        INSERT_JUMP(jump, laststart, b + 3);
        b += 3;
      }

      /* Otherwise, we have a nontrivial interval.  When
         we're all done, the pattern will look like:
         set_number_at <jump count> <upper bound>
         set_number_at <succeed_n count> <lower bound>
         succeed_n <after jump addr> <succeed_n count>
         <body of loop>
         jump_n <succeed_n addr> <jump count>
         (The upper bound and `jump_n' are omitted if
         `upper_bound' is 1, though.)  */
      else {
        /* If the upper bound is > 1, we need to insert
           more at the end of the loop.  */
        unsigned nbytes = 10 + (upper_bound > 1) * 10;

        GET_BUFFER_SPACE(nbytes);

        /* Initialize lower bound of the `succeed_n', even
           though it will be set during matching by its
           attendant `set_number_at' (inserted next),
           because `re_compile_fastmap' needs to know.
           Jump to the `jump_n' we might insert below.  */
        INSERT_JUMP2(succeed_n, laststart,
             b + 5 + (upper_bound > 1) * 5,
             lower_bound);
        b += 5;

        /* Code to initialize the lower bound.  Insert
           before the `succeed_n'.  The `5' is the last two
           bytes of this `set_number_at', plus 3 bytes of
           the following `succeed_n'.  */
        insert_op2(set_number_at, laststart, 5, lower_bound, b);
        b += 5;

        if (upper_bound > 1) {
                /* More than one repetition is allowed, so
                   append a backward jump to the `succeed_n'
                   that starts this interval.

                   When we've reached this during matching,
                   we'll have matched the interval once, so
                   jump back only `upper_bound - 1' times.  */
          STORE_JUMP2(jump_n, b, laststart + 5,
              upper_bound - 1);
          b += 5;

                /* The location we want to set is the second
                   parameter of the `jump_n'; that is `b-2' as
                   an absolute address.  `laststart' will be
                   the `set_number_at' we're about to insert;
                   `laststart+3' the number to set, the source
                   for the relative address.  But we are
                   inserting into the middle of the pattern --
                   so everything is getting moved up by 5.
                   Conclusion: (b - 2) - (laststart + 3) + 5,
                   i.e., b - laststart.

                   We insert this at the beginning of the loop
                   so that if we fail during matching, we'll
                   reinitialize the bounds.  */
          insert_op2(set_number_at, laststart, b - laststart,
             upper_bound - 1, b);
          b += 5;
        }
      }
      pending_exact = 0;
      beg_interval = NULL;
    }
    break;

      unfetch_interval:
    /* If an invalid interval, match the characters as literals.  */
    assert (beg_interval);
    p = beg_interval;
    beg_interval = NULL;

    /* normal_char and normal_backslash need `c'.  */
    PATFETCH_RAW(c);

    goto normal_char;

      case (sal_Unicode)'`':
    BUF_PUSH(begbuf);
    break;

      case (sal_Unicode)'\'':
    BUF_PUSH(endbuf);
    break;

      case (sal_Unicode)'1': case (sal_Unicode)'2':
      case (sal_Unicode)'3': case (sal_Unicode)'4':
      case (sal_Unicode)'5': case (sal_Unicode)'6':
      case (sal_Unicode)'7': case (sal_Unicode)'8':
      case (sal_Unicode)'9':
    c1 = c - (sal_Unicode)'0';

    if (c1 > regnum)
      FREE_STACK_RETURN(REG_ESUBREG);

    /* Can't back reference to a subexpression if inside of it.  */
    if (group_in_compile_stack(compile_stack, (sal_Int32) c1)) {
      goto normal_char;
    }

    laststart = b;
    BUF_PUSH_2(duplicate, c1);
    break;


      case (sal_Unicode)'+':
      case (sal_Unicode)'?':
    goto normal_backslash;

      case (sal_Unicode)'x':        // Unicode char
    {
      sal_Int32 UniChar = -1;

      GET_HEX_NUMBER(UniChar);
      if (UniChar < 0 || UniChar > 0xffff) FREE_STACK_RETURN(REG_BADPAT);
      c = (sal_Unicode) UniChar;
      goto normal_char;
    }
    break;

      case (sal_Unicode)'<':        // begin Word boundary
    BUF_PUSH(wordbeg);
    break;

      case (sal_Unicode)'>':        // end Word boundary
    BUF_PUSH(wordend);
    break;

      case (sal_Unicode)'n':
    c = 0x0a;
    goto normal_char;

      case (sal_Unicode)'t':
    c = 0x09;
    goto normal_char;

      default:
      normal_backslash:
    goto normal_char;
      }
      break;

    default:
      /* Expects the character in `c'.  */
    normal_char:
      /* If no exactn currently being built.  */
      if ( pending_exact == NULL

       /* If last exactn not at current position.  */
       || pending_exact + *pending_exact + 1 != b

       /* We have only one sal_Unicode char following the
          exactn for the count.  */
       || *pending_exact == (1 << BYTEWIDTH) - 1

       /* If followed by a repetition operator.  */
       || *p == (sal_Unicode)'*' || *p == (sal_Unicode)'^'
       || *p == (sal_Unicode)'+' || *p == (sal_Unicode)'?'
       || *p == (sal_Unicode) '{' ) {
    /* Start building a new exactn.  */
    laststart = b;
    BUF_PUSH_2(exactn, 0);
    pending_exact = b - 1;
      }

      if ( translate ) {
        try {
            sal_Unicode tmp = translit->transliterateChar2Char(c);
            BUF_PUSH(tmp);
            (*pending_exact)++;
        } catch (::com::sun::star::i18n::MultipleCharsOutputException e) {
            ::rtl::OUString o2( translit->transliterateChar2String( c));
            sal_Int32 len2 = o2.getLength();
            const sal_Unicode * k2 = o2.getStr();
            for (sal_Int32 nmatch = 0; nmatch < len2; nmatch++) {
              BUF_PUSH(k2[nmatch]);
              (*pending_exact)++;
            }
        }
      } else {
    BUF_PUSH(c);
    (*pending_exact)++;
      }
      break;
    } /* switch (c) */
  } /* while p != pend */

  /* Through the pattern now.  */

  if (fixup_alt_jump)
    STORE_JUMP(jump_past_alt, fixup_alt_jump, b);

  if (!COMPILE_STACK_EMPTY)
    FREE_STACK_RETURN(REG_EPAREN);

  // Assumes no backtracking
  BUF_PUSH(succeed);

  if ( compile_stack.stack )
    free(compile_stack.stack);
  compile_stack.stack = NULL;

  /* We have succeeded; set the length of the buffer.  */
  bufp->used = b - bufp->buffer;

  return REG_NOERROR;
} /* regex_compile */

/* Return zero if TRANSLATE[S1] and TRANSLATE[S2] are identical for LEN
   bytes; nonzero otherwise.  */

sal_Int32
Regexpr::bcmp_translate(const sal_Unicode *s1, const sal_Unicode *s2, sal_Int32 len)
{
  for (sal_Int32 nmatch = 0; nmatch < len; nmatch++) {
    if (*s1++ != *s2++) {
      return(1);
    }
  }

  return(0);
}


/* We are passed P pointing to a register number after a start_memory.

   Return true if the pattern up to the corresponding stop_memory can
   match the empty string, and false otherwise.

   If we find the matching stop_memory, sets P to point to one past its number.
   Otherwise, sets P to an undefined byte less than or equal to END.

   We don't handle duplicates properly (yet).  */

sal_Bool
Regexpr::group_match_null_string_p(sal_Unicode **p, sal_Unicode *end, register_info_type *reg_info)
{
  sal_Int32 mcnt;
/* Point to after the args to the start_memory.  */
    sal_Unicode *p1 = *p + 2;

    while (p1 < end) {
    /* Skip over opcodes that can match nothing, and return true or
       false, as appropriate, when we get to one that can't, or to the
                      matching stop_memory.  */

      switch ((re_opcode_t) *p1) {
    /* Could be either a loop or a series of alternatives.  */
      case on_failure_jump:
    p1++;
    extract_number_and_incr(mcnt, p1);

    /* If the next operation is not a jump backwards in the
       pattern.  */

    if (mcnt >= 0) {
      /* Go through the on_failure_jumps of the alternatives,
         seeing if any of the alternatives cannot match nothing.
         The last alternative starts with only a jump,
         whereas the rest start with on_failure_jump and end
         with a jump, e.g., here is the pattern for `a|b|c':

         /on_failure_jump/0/6/exactn/1/a/jump_past_alt/0/6
         /on_failure_jump/0/6/exactn/1/b/jump_past_alt/0/3
         /exactn/1/c

         So, we have to first go through the first (n-1)
         alternatives and then deal with the last one separately.  */


      /* Deal with the first (n-1) alternatives, which start
         with an on_failure_jump (see above) that jumps to right
         past a jump_past_alt.  */

      while ((re_opcode_t) p1[mcnt-3] == jump_past_alt) {
        /* `mcnt' holds how many bytes long the alternative
           is, including the ending `jump_past_alt' and
           its number.  */

        if (!alt_match_null_string_p(p1, p1 + mcnt - 3, reg_info))
          return false;

        /* Move to right after this alternative, including the
           jump_past_alt.  */
        p1 += mcnt;

        /* Break if it's the beginning of an n-th alternative
           that doesn't begin with an on_failure_jump.  */
        if ((re_opcode_t) *p1 != on_failure_jump)
          break;

        /* Still have to check that it's not an n-th
           alternative that starts with an on_failure_jump.  */
        p1++;
        extract_number_and_incr(mcnt, p1);
        if ((re_opcode_t) p1[mcnt-3] != jump_past_alt) {
          /* Get to the beginning of the n-th alternative.  */
          p1 -= 3;
          break;
        }
      }

      /* Deal with the last alternative: go back and get number
         of the `jump_past_alt' just before it.  `mcnt' contains
         the length of the alternative.  */
      extract_number(mcnt, p1 - 2);

      if (!alt_match_null_string_p (p1, p1 + mcnt, reg_info))
        return false;

      p1 += mcnt;       /* Get past the n-th alternative.  */
    } /* if mcnt > 0 */
    break;


      case stop_memory:
    assert (p1[1] == **p);
    *p = p1 + 2;
    return true;


      default:
    if (!common_op_match_null_string_p(&p1, end, reg_info))
      return false;
      }
    } /* while p1 < end */

 return false;
} /* group_match_null_string_p */

/* Similar to group_match_null_string_p, but doesn't deal with alternatives:
   It expects P to be the first byte of a single alternative and END one
   byte past the last. The alternative can contain groups.  */

sal_Bool
Regexpr::alt_match_null_string_p(sal_Unicode *p, sal_Unicode *end, register_info_type *reg_info)
{
  sal_Int32 mcnt;
  sal_Unicode *p1 = p;

  while (p1 < end) {
    /* Skip over opcodes that can match nothing, and break when we get
       to one that can't.  */

    switch ((re_opcode_t) *p1) {
      /* It's a loop.  */
    case on_failure_jump:
      p1++;
      extract_number_and_incr(mcnt, p1);
      p1 += mcnt;
      break;

    default:
      if (!common_op_match_null_string_p(&p1, end, reg_info))
    return false;
    }
  }  /* while p1 < end */

  return true;
} /* alt_match_null_string_p */


/* Deals with the ops common to group_match_null_string_p and
   alt_match_null_string_p.

   Sets P to one after the op and its arguments, if any.  */

sal_Bool
Regexpr::common_op_match_null_string_p(sal_Unicode **p, sal_Unicode *end, register_info_type *reg_info)
{
  sal_Int32 mcnt;
  sal_Bool ret;
  sal_Int32 reg_no;
  sal_Unicode *p1 = *p;

  switch ((re_opcode_t) *p1++) {
  case no_op:
  case begline:
  case endline:
  case begbuf:
  case endbuf:
    break;

  case start_memory:
    reg_no = *p1;
    assert (reg_no > 0 && reg_no <= MAX_REGNUM);
    ret = group_match_null_string_p(&p1, end, reg_info);
    /* Have to set this here in case we're checking a group which
       contains a group and a back reference to it.  */

    if (REG_MATCH_NULL_STRING_P(reg_info[reg_no]) == MATCH_NULL_UNSET_VALUE)
      REG_MATCH_NULL_STRING_P(reg_info[reg_no]) = ret;

    if (!ret)
      return false;
    break;

    /* If this is an optimized succeed_n for zero times, make the jump.  */
  case jump:
    extract_number_and_incr(mcnt, p1);
    if (mcnt >= 0)
      p1 += mcnt;
    else
      return false;
    break;

  case succeed_n:
    /* Get to the number of times to succeed.  */
    p1 += 2;
    extract_number_and_incr(mcnt, p1);

    if (mcnt == 0)
      {
    p1 -= 4;
    extract_number_and_incr(mcnt, p1);
    p1 += mcnt;
      }
    else
      return false;
    break;

  case duplicate:
    if (!REG_MATCH_NULL_STRING_P(reg_info[*p1]))
      return false;
    break;

  case set_number_at:
    p1 += 4;

  default:
    /* All other opcodes mean we cannot match the empty string.  */
    return false;
  }

  *p = p1;
  return true;
} /* common_op_match_null_string_p */



/* Free everything we malloc.  */
#ifdef MATCH_MAY_ALLOCATE
# define FREE_VAR(var) if (var) REGEX_FREE (var); var = NULL
# define FREE_VARIABLES()                       \
  do {                                  \
    REGEX_FREE_STACK (fail_stack.stack);                \
    FREE_VAR (regstart);                        \
    FREE_VAR (regend);                          \
    FREE_VAR (old_regstart);                        \
    FREE_VAR (old_regend);                      \
    FREE_VAR (best_regstart);                       \
    FREE_VAR (best_regend);                     \
    FREE_VAR (reg_info);                        \
    FREE_VAR (reg_dummy);                       \
    FREE_VAR (reg_info_dummy);                      \
  } while (0)
#else
# define FREE_VARIABLES() ((void)0) /* Do nothing!  But inhibit gcc warning. */
#endif /* not MATCH_MAY_ALLOCATE */

/* This is a separate function so that we can force an alloca cleanup
   afterwards.  */
sal_Int32
Regexpr::re_match2(struct re_registers *regs, sal_Int32 pos, sal_Int32 range)
{
  /* General temporaries.  */
  sal_Int32 mcnt;
  sal_Unicode *p1;

  /* Just past the end of the corresponding string.  */
  sal_Unicode *end2;

  /* Pointers into string2, just past the last characters in
       each to consider matching.  */
  sal_Unicode *end_match_2;

  /* Where we are in the data, and the end of the current string.  */
  const sal_Unicode *d, *dend;

  /* Where we are in the compiled pattern, and the end of the compiled
       pattern.  */
  sal_Unicode *p = bufp->buffer;
  register sal_Unicode *pend = p + bufp->used;

    /* Mark the opcode just after a start_memory, so we can test for an
       empty subpattern when we get to the stop_memory.  */
  sal_Unicode *just_past_start_mem = 0;

  /* Failure point stack.  Each place that can handle a failure further
     down the line pushes a failure point on this stack.  It consists of
     restart, regend, and reg_info for all registers corresponding to
     the subexpressions we're currently inside, plus the number of such
     registers, and, finally, two sal_Unicode *'s.  The first
     sal_Unicode * is where to resume scanning the pattern; the second
     one is where to resume scanning the strings.  If the latter is
     zero, the failure point is a ``dummy''; if a failure happens and
     the failure point is a dummy, it gets discarded and the next next
     one is tried.  */
#ifdef MATCH_MAY_ALLOCATE /* otherwise, this is global.  */
  fail_stack_type fail_stack;
#endif

  /* We fill all the registers internally, independent of what we
     return, for use in backreferences.  The number here includes
     an element for register zero.  */
  size_t num_regs = bufp->re_nsub + 1;

  /* The currently active registers.  */
  sal_uInt32 lowest_active_reg = NO_LOWEST_ACTIVE_REG;
  sal_uInt32 highest_active_reg = NO_HIGHEST_ACTIVE_REG;

  /* Information on the contents of registers. These are pointers into
     the input strings; they record just what was matched (on this
     attempt) by a subexpression part of the pattern, that is, the
     regnum-th regstart pointer points to where in the pattern we began
     matching and the regnum-th regend points to right after where we
     stopped matching the regnum-th subexpression.  (The zeroth register
     keeps track of what the whole pattern matches.)  */
#ifdef MATCH_MAY_ALLOCATE /* otherwise, these are global.  */
  const sal_Unicode **regstart, **regend;
#endif

  /* If a group that's operated upon by a repetition operator fails to
     match anything, then the register for its start will need to be
     restored because it will have been set to wherever in the string we
     are when we last see its open-group operator.  Similarly for a
     register's end.  */
#ifdef MATCH_MAY_ALLOCATE /* otherwise, these are global.  */
  const sal_Unicode **old_regstart, **old_regend;
#endif

  /* The is_active field of reg_info helps us keep track of which (possibly
     nested) subexpressions we are currently in. The matched_something
     field of reg_info[reg_num] helps us tell whether or not we have
     matched any of the pattern so far this time through the reg_num-th
     subexpression.  These two fields get reset each time through any
     loop their register is in.  */
#ifdef MATCH_MAY_ALLOCATE /* otherwise, this is global.  */
  register_info_type *reg_info;
#endif

  /* The following record the register info as found in the above
     variables when we find a match better than any we've seen before.
     This happens as we backtrack through the failure points, which in
     turn happens only if we have not yet matched the entire string. */
  unsigned best_regs_set = false;
#ifdef MATCH_MAY_ALLOCATE /* otherwise, these are global.  */
  const sal_Unicode **best_regstart, **best_regend;
#endif

  /* Logically, this is `best_regend[0]'.  But we don't want to have to
     allocate space for that if we're not allocating space for anything
     else (see below).  Also, we never need info about register 0 for
     any of the other register vectors, and it seems rather a kludge to
     treat `best_regend' differently than the rest.  So we keep track of
     the end of the best match so far in a separate variable.  We
     initialize this to NULL so that when we backtrack the first time
     and need to test it, it's not garbage.  */
  const sal_Unicode *match_end = NULL;

  /* This helps SET_REGS_MATCHED avoid doing redundant work.  */
  sal_Int32 set_regs_matched_done = 0;

  /* Used when we pop values we don't care about.  */
#ifdef MATCH_MAY_ALLOCATE /* otherwise, these are global.  */
  const sal_Unicode **reg_dummy;
  register_info_type *reg_info_dummy;
#endif

  INIT_FAIL_STACK();

#ifdef MATCH_MAY_ALLOCATE
  /* Do not bother to initialize all the register variables if there are
     no groups in the pattern, as it takes a fair amount of time.  If
     there are groups, we include space for register 0 (the whole
     pattern), even though we never use it, since it simplifies the
     array indexing.  We should fix this.  */
  if (bufp->re_nsub)
    {
      regstart = REGEX_TALLOC (num_regs, const sal_Unicode *);
      regend = REGEX_TALLOC (num_regs, const sal_Unicode *);
      old_regstart = REGEX_TALLOC (num_regs, const sal_Unicode *);
      old_regend = REGEX_TALLOC (num_regs, const sal_Unicode *);
      best_regstart = REGEX_TALLOC (num_regs, const sal_Unicode *);
      best_regend = REGEX_TALLOC (num_regs, const sal_Unicode *);
      reg_info = REGEX_TALLOC (num_regs, register_info_type);
      reg_dummy = REGEX_TALLOC (num_regs, const sal_Unicode *);
      reg_info_dummy = REGEX_TALLOC (num_regs, register_info_type);

      if (!(regstart && regend && old_regstart && old_regend && reg_info
            && best_regstart && best_regend && reg_dummy && reg_info_dummy))
        {
          FREE_VARIABLES ();
          return -2;
        }
    }
  else
    {
      /* We must initialize all our variables to NULL, so that
         `FREE_VARIABLES' doesn't try to free them.  */
      regstart = regend = old_regstart = old_regend = best_regstart
        = best_regend = reg_dummy = NULL;
      reg_info = reg_info_dummy = (register_info_type *) NULL;
    }
#endif /* MATCH_MAY_ALLOCATE */

  sal_Unicode *string2 = (sal_Unicode *)line;
  sal_Int32 size2 = linelen;
  sal_Int32 stop = range;

  /* The starting position is bogus.  */
  if (pos < 0 || pos >= size2 || linelen <= 0 ) {
      FREE_VARIABLES ();
      return(-1);
  }

  /* Initialize subexpression text positions to -1 to mark ones that no
     start_memory/stop_memory has been seen for. Also initialize the
     register information struct.  */
  for (mcnt = 1; (unsigned) mcnt < num_regs; mcnt++) {
    regstart[mcnt] = regend[mcnt]
      = old_regstart[mcnt] = old_regend[mcnt] = REG_UNSET_VALUE;

    REG_MATCH_NULL_STRING_P (reg_info[mcnt]) = MATCH_NULL_UNSET_VALUE;
    IS_ACTIVE (reg_info[mcnt]) = 0;
    MATCHED_SOMETHING (reg_info[mcnt]) = 0;
    EVER_MATCHED_SOMETHING (reg_info[mcnt]) = 0;
  }

  end2 = (sal_Unicode *)(string2 + size2);

  end_match_2 = (sal_Unicode *)(string2 + stop);

  /* `p' scans through the pattern as `d' scans through the data.
     `dend' is the end of the input string that `d' points within.  `d'
     is advanced into the following input string whenever necessary, but
     this happens before fetching; therefore, at the beginning of the
     loop, `d' can be pointing at the end of a string, but it cannot
     equal `string2'.  */
  d = string2 + pos;
  dend = end_match_2;

    /* This loops over pattern commands.  It exits by returning from the
       function if the match is complete, or it drops through if the match
       fails at this starting point in the input data.  */
  for (;;) {
    if (p == pend) {
      /* End of pattern means we might have succeeded.  */

      /* If we haven't matched the entire string, and we want the
     longest match, try backtracking.  */
      if (d != end_match_2) {
    if (!FAIL_STACK_EMPTY()) {
      goto fail;
    }
      } /* d != end_match_2 */

    succeed_label:

      /* If caller wants register contents data back, do it.  */
      if (regs) {
    /* Have the register data arrays been allocated?  */
    if (regs->num_regs == 0) {
      /* No.  So allocate them with malloc.  We need one
         extra element beyond `num_regs' for the `-1' marker
         GNU code uses.  */
      regs->num_of_match = 0;
      regs->num_regs = MAX(RE_NREGS, num_regs + 1);
      regs->start = (sal_Int32 *) malloc(regs->num_regs * sizeof(sal_Int32));
      regs->end = (sal_Int32 *) malloc(regs->num_regs * sizeof(sal_Int32));
      if (regs->start == NULL || regs->end == NULL) {
        FREE_VARIABLES ();
        return(-2);
      }
    } else if ( regs->num_regs > 0 ) {
      /* Yes.  If we need more elements than were already
         allocated, reallocate them.  If we need fewer, just
         leave it alone.  */
      if (regs->num_regs < num_regs + 1) {
        regs->num_regs = num_regs + 1;
        regs->start = (sal_Int32 *) realloc(regs->start, regs->num_regs * sizeof(sal_Int32));
        regs->end = (sal_Int32 *) realloc(regs->end, regs->num_regs * sizeof(sal_Int32));
        if (regs->start == NULL || regs->end == NULL) {
          FREE_VARIABLES ();
          return(-2);
        }
      }
    } else {    // num_regs is negative
      FREE_VARIABLES ();
      return(-2);
    }

    /* Convert the pointer data in `regstart' and `regend' to
       indices.  Register zero has to be set differently,
       since we haven't kept track of any info for it.  */
    if (regs->num_regs > 0) {
      // Make sure a valid location
      sal_Int32 dpos = d - string2;
      if (pos == dpos || (d - 1) >= dend ) {
        FREE_VARIABLES ();
        return(-1);
      }
      regs->start[regs->num_of_match] = pos;
      regs->end[regs->num_of_match] = ((sal_Int32) (d - string2));
      regs->num_of_match++;
    }

    /* Go through the first `min (num_regs, regs->num_regs)'
       registers, since that is all we initialized.  */
    for (mcnt = regs->num_of_match; (unsigned) mcnt < MIN(num_regs, regs->num_regs);
         mcnt++) {
      if (REG_UNSET(regstart[mcnt]) || REG_UNSET(regend[mcnt]))
        regs->start[mcnt] = regs->end[mcnt] = -1;
      else {
        regs->start[mcnt] = (sal_Int32) POINTER_TO_OFFSET(regstart[mcnt]);
        regs->end[mcnt] = (sal_Int32) POINTER_TO_OFFSET(regend[mcnt]);
      }
    }

    /* If the regs structure we return has more elements than
       were in the pattern, set the extra elements to -1.  If
       we (re)allocated the registers, this is the case,
       because we always allocate enough to have at least one
       -1 at the end.  */
    for (mcnt = regs->num_of_match; (unsigned) mcnt < regs->num_regs; mcnt++)
      regs->start[mcnt] = regs->end[mcnt] = -1;
      } /* regs */

      mcnt = d - pos - string2;

      FREE_VARIABLES ();
      return(0);
    }
    /* Otherwise match next pattern command.  */
    switch ((re_opcode_t) *p++) {
      /* Ignore these.  Used to ignore the n of succeed_n's which
     currently have n == 0.  */
    case no_op:
      break;

    case succeed:
      goto succeed_label;

      /* Match the next n pattern characters exactly.  The following
     byte in the pattern defines n, and the n bytes after that
     are the characters to match.  */
    case exactn:
      mcnt = *p++;

      do {
    PREFETCH();
    if ((sal_Unicode)*d++ != (sal_Unicode) *p++) goto fail;
      } while (--mcnt);
      SET_REGS_MATCHED();
      break;

      /* Match any character except possibly a newline or a null.  */
    case anychar:

      PREFETCH();
      if ( *d == (sal_Unicode)'\n' ||
       *d == (sal_Unicode)'\000' )
    goto fail;

      SET_REGS_MATCHED();
      d++;
      break;

    case charset:
    case charset_not: {
      register sal_Unicode c;
      sal_Bool knot = (re_opcode_t) *(p - 1) == charset_not;

      PREFETCH();
      c = *d; /* The character to match.  */
      /* Cast to `sal_uInt32' instead of `sal_Unicode' in case the
     bit list is a full 32 bytes long.  */
      if ((c < (sal_uInt32) (*p * BYTEWIDTH)) && (p[1 + c / BYTEWIDTH] & (1 << (c % BYTEWIDTH))))
    knot = !knot;

      p += 1 + *p;

      if (!knot) {
    goto fail;
      }

      SET_REGS_MATCHED();
      d++;
      break;
    }

    /* The beginning of a group is represented by start_memory.
       The arguments are the register number in the next byte, and the
       number of groups inner to this one in the next.  The text
       matched within the group is recorded (in the internal
       registers data structure) under the register number.  */
    case start_memory:

      /* Find out if this group can match the empty string.  */
      p1 = p;       /* To send to group_match_null_string_p.  */

      if (REG_MATCH_NULL_STRING_P(reg_info[*p]) == MATCH_NULL_UNSET_VALUE)
    REG_MATCH_NULL_STRING_P(reg_info[*p]) = group_match_null_string_p(&p1, pend, reg_info);

      /* Save the position in the string where we were the last time
     we were at this open-group operator in case the group is
     operated upon by a repetition operator, e.g., with `(a*)*b'
     against `ab'; then we want to ignore where we are now in
     the string in case this attempt to match fails.  */
      old_regstart[*p] = REG_MATCH_NULL_STRING_P(reg_info[*p])
    ? REG_UNSET(regstart[*p]) ? d : regstart[*p]
    : regstart[*p];

      regstart[*p] = d;

      IS_ACTIVE (reg_info[*p]) = 1;
      MATCHED_SOMETHING(reg_info[*p]) = 0;

      /* Clear this whenever we change the register activity status.  */
      set_regs_matched_done = 0;

      /* This is the new highest active register.  */
      highest_active_reg = *p;

      /* If nothing was active before, this is the new lowest active
     register.  */
      if (lowest_active_reg == NO_LOWEST_ACTIVE_REG)
    lowest_active_reg = *p;

      /* Move past the register number and inner group count.  */
      p += 2;
      just_past_start_mem = p;

      break;

      /* The stop_memory opcode represents the end of a group.  Its
     arguments are the same as start_memory's: the register
     number, and the number of inner groups.  */
    case stop_memory:

      /* We need to save the string position the last time we were at
     this close-group operator in case the group is operated
     upon by a repetition operator, e.g., with `((a*)*(b*)*)*'
     against `aba'; then we want to ignore where we are now in
     the string in case this attempt to match fails.  */
      old_regend[*p] = REG_MATCH_NULL_STRING_P (reg_info[*p])
    ? REG_UNSET(regend[*p]) ? d : regend[*p]
    : regend[*p];

      regend[*p] = d;

      /* This register isn't active anymore.  */
      IS_ACTIVE(reg_info[*p]) = 0;

      /* Clear this whenever we change the register activity status.  */
      set_regs_matched_done = 0;

      /* If this was the only register active, nothing is active
     anymore.  */
      if (lowest_active_reg == highest_active_reg) {
    lowest_active_reg = NO_LOWEST_ACTIVE_REG;
    highest_active_reg = NO_HIGHEST_ACTIVE_REG;
      } else { /* We must scan for the new highest active register, since
          it isn't necessarily one less than now: consider
          (a(b)c(d(e)f)g).  When group 3 ends, after the f), the
          new highest active register is 1.  */
    sal_Unicode r = *p - 1;
    while (r > 0 && !IS_ACTIVE (reg_info[r]))
      r--;

    /* If we end up at register zero, that means that we saved
       the registers as the result of an `on_failure_jump', not
       a `start_memory', and we jumped to past the innermost
       `stop_memory'.  For example, in ((.)*) we save
       registers 1 and 2 as a result of the *, but when we pop
       back to the second ), we are at the stop_memory 1.
       Thus, nothing is active.  */
    if (r == 0) {
      lowest_active_reg = NO_LOWEST_ACTIVE_REG;
      highest_active_reg = NO_HIGHEST_ACTIVE_REG;
    } else
      highest_active_reg = r;
      }

      /* If just failed to match something this time around with a
     group that's operated on by a repetition operator, try to
     force exit from the ``loop'', and restore the register
     information for this group that we had before trying this
     last match.  */
      if ((!MATCHED_SOMETHING (reg_info[*p])
       || just_past_start_mem == p - 1)
      && (p + 2) < pend) {
    sal_Bool is_a_jump_n = false;

    p1 = p + 2;
    mcnt = 0;
    switch ((re_opcode_t) *p1++) {
    case jump_n:
      is_a_jump_n = true;
    case pop_failure_jump:
    case maybe_pop_jump:
    case jump:
    case dummy_failure_jump:
      extract_number_and_incr(mcnt, p1);
      if (is_a_jump_n)
        p1 += 2;
      break;

    default:
      /* do nothing */ ;
    }
    p1 += mcnt;

    /* If the next operation is a jump backwards in the pattern
       to an on_failure_jump right before the start_memory
       corresponding to this stop_memory, exit from the loop
       by forcing a failure after pushing on the stack the
       on_failure_jump's jump in the pattern, and d.  */
    if (mcnt < 0 && (re_opcode_t) *p1 == on_failure_jump
        && (re_opcode_t) p1[3] == start_memory && p1[4] == *p) {
      /* If this group ever matched anything, then restore
         what its registers were before trying this last
         failed match, e.g., with `(a*)*b' against `ab' for
         regstart[1], and, e.g., with `((a*)*(b*)*)*'
         against `aba' for regend[3].

         Also restore the registers for inner groups for,
         e.g., `((a*)(b*))*' against `aba' (register 3 would
         otherwise get trashed).  */

      if (EVER_MATCHED_SOMETHING (reg_info[*p])) {
        unsigned r;

        EVER_MATCHED_SOMETHING (reg_info[*p]) = 0;

        /* Restore this and inner groups' (if any) registers.  */
        for (r = *p; r < (unsigned) *p + (unsigned) *(p + 1);
         r++) {
          regstart[r] = old_regstart[r];

                /* xx why this test?  */
          if (old_regend[r] >= regstart[r])
        regend[r] = old_regend[r];
        }
      }
      p1++;
      extract_number_and_incr(mcnt, p1);
      PUSH_FAILURE_POINT(p1 + mcnt, d, -2);

      goto fail;
    }
      }

      /* Move past the register number and the inner group count.  */
      p += 2;
      break;


      /* \<digit> has been turned into a `duplicate' command which is
     followed by the numeric value of <digit> as the register number.  */
    case duplicate:
      {
    register const sal_Unicode *d2, *dend2;
    sal_Unicode regno = *p++;   /* Get which register to match against.  */

    /* Can't back reference a group which we've never matched.  */
    if (REG_UNSET(regstart[regno]) || REG_UNSET(regend[regno])) {
      goto fail;
    }

    /* Where in input to try to start matching.  */
    d2 = regstart[regno];

    /* Where to stop matching; if both the place to start and
       the place to stop matching are in the same string, then
       set to the place to stop, otherwise, for now have to use
       the end of the first string.  */

    dend2 = regend[regno];
    for (;;) {
      /* If necessary, advance to next segment in register
         contents.  */
      while (d2 == dend2) {
        if (dend2 == end_match_2) break;
        if (dend2 == regend[regno]) break;
      }
      /* At end of register contents => success */
      if (d2 == dend2) break;

      PREFETCH();

      /* How many characters left in this segment to match.  */
      mcnt = dend - d;

      /* Want how many consecutive characters we can match in
         one shot, so, if necessary, adjust the count.  */
      if (mcnt > dend2 - d2)
        mcnt = dend2 - d2;

      /* Compare that many; failure if mismatch, else move
         past them.  */
      if (translate
          ? bcmp_translate(d, d2, mcnt)
          : memcmp(d, d2, mcnt * sizeof(sal_Unicode))) {
        goto fail;
      }
      d += mcnt, d2 += mcnt;
      /* Do this because we've match some characters.  */
      SET_REGS_MATCHED();
    }
      }
      break;

      /* begline matches the empty string at the beginning of the string
     (unless `not_bol' is set in `bufp'), and, if
     `newline_anchor' is set, after newlines.  */
    case begline:

      if (AT_STRINGS_BEG (d)) {
    if (!bufp->not_bol) break;
      } else if (d[-1] == '\n' && bufp->newline_anchor) {
    break;
      }
      /* In all other cases, we fail.  */
      goto fail;

      /* endline is the dual of begline.  */
    case endline:

      if (AT_STRINGS_END(d))    {
    if (!bufp->not_eol) break;
      } else if (*d == '\n' && bufp->newline_anchor) {
    break;
      }
      goto fail;

      /* Match at the very beginning of the data.  */
    case begbuf:
      if (AT_STRINGS_BEG (d))
    break;
      goto fail;


      /* Match at the very end of the data.  */
    case endbuf:
      if (AT_STRINGS_END (d))
    break;
      goto fail;


      /* on_failure_keep_string_jump is used to optimize `.*\n'.  It
     pushes NULL as the value for the string on the stack.  Then
     `pop_failure_point' will keep the current value for the
     string, instead of restoring it.  To see why, consider
     matching `foo\nbar' against `.*\n'.  The .* matches the foo;
     then the . fails against the \n.  But the next thing we want
     to do is match the \n against the \n; if we restored the
     string value, we would be back at the foo.

     Because this is used only in specific cases, we don't need to
     check all the things that `on_failure_jump' does, to make
     sure the right things get saved on the stack.  Hence we don't
     share its code.  The only reason to push anything on the
     stack at all is that otherwise we would have to change
     `anychar's code to do something besides goto fail in this
     case; that seems worse than this.  */
    case on_failure_keep_string_jump:

      extract_number_and_incr(mcnt, p);

      PUSH_FAILURE_POINT(p + mcnt, NULL, -2);
      break;


      /* Uses of on_failure_jump:

     Each alternative starts with an on_failure_jump that points
     to the beginning of the next alternative.  Each alternative
     except the last ends with a jump that in effect jumps past
     the rest of the alternatives.  (They really jump to the
     ending jump of the following alternative, because tensioning
     these jumps is a hassle.)

     Repeats start with an on_failure_jump that points past both
     the repetition text and either the following jump or
     pop_failure_jump back to this on_failure_jump.  */
    case on_failure_jump:
    on_failure:

    extract_number_and_incr(mcnt, p);

    /* If this on_failure_jump comes right before a group (i.e.,
       the original * applied to a group), save the information
       for that group and all inner ones, so that if we fail back
       to this point, the group's information will be correct.
       For example, in \(a*\)*\1, we need the preceding group,
       and in \(zz\(a*\)b*\)\2, we need the inner group.  */

    /* We can't use `p' to check ahead because we push
       a failure point to `p + mcnt' after we do this.  */
    p1 = p;

    /* We need to skip no_op's before we look for the
       start_memory in case this on_failure_jump is happening as
       the result of a completed succeed_n, as in \(a\)\{1,3\}b\1
       against aba.  */
    while (p1 < pend && (re_opcode_t) *p1 == no_op)
      p1++;

    if (p1 < pend && (re_opcode_t) *p1 == start_memory) {
      /* We have a new highest active register now.  This will
     get reset at the start_memory we are about to get to,
     but we will have saved all the registers relevant to
     this repetition op, as described above.  */
      highest_active_reg = *(p1 + 1) + *(p1 + 2);
      if (lowest_active_reg == NO_LOWEST_ACTIVE_REG)
    lowest_active_reg = *(p1 + 1);
    }

    PUSH_FAILURE_POINT(p + mcnt, d, -2);
    break;

    /* A smart repeat ends with `maybe_pop_jump'.
       We change it to either `pop_failure_jump' or `jump'.  */
    case maybe_pop_jump:
      extract_number_and_incr(mcnt, p);
      {
    register sal_Unicode *p2 = p;

    /* Compare the beginning of the repeat with what in the
       pattern follows its end. If we can establish that there
       is nothing that they would both match, i.e., that we
       would have to backtrack because of (as in, e.g., `a*a')
       then we can change to pop_failure_jump, because we'll
       never have to backtrack.

       This is not true in the case of alternatives: in
       `(a|ab)*' we do need to backtrack to the `ab' alternative
       (e.g., if the string was `ab').  But instead of trying to
       detect that here, the alternative has put on a dummy
       failure point which is what we will end up popping.  */

    /* Skip over open/close-group commands.
       If what follows this loop is a ...+ construct,
       look at what begins its body, since we will have to
       match at least one of that.  */
    while (1) {
      if (p2 + 2 < pend
          && ((re_opcode_t) *p2 == stop_memory
          || (re_opcode_t) *p2 == start_memory))
        p2 += 3;
      else if (p2 + 6 < pend
           && (re_opcode_t) *p2 == dummy_failure_jump)
        p2 += 6;
      else
        break;
    }

    p1 = p + mcnt;
    /* p1[0] ... p1[2] are the `on_failure_jump' corresponding
       to the `maybe_finalize_jump' of this case.  Examine what
       follows.  */

    /* If we're at the end of the pattern, we can change.  */
    if (p2 == pend) {
                /* Consider what happens when matching ":\(.*\)"
                   against ":/".  I don't really understand this code
                   yet.  */
      p[-3] = (sal_Unicode) pop_failure_jump;
    } else if ((re_opcode_t) *p2 == exactn
           || (bufp->newline_anchor && (re_opcode_t) *p2 == endline)) {
      register sal_Unicode c = *p2 == (sal_Unicode) endline ? (sal_Unicode)'\n' : p2[2];

      if ((re_opcode_t) p1[3] == exactn && p1[5] != c) {
        p[-3] = (sal_Unicode) pop_failure_jump;
      } else if ((re_opcode_t) p1[3] == charset
             || (re_opcode_t) p1[3] == charset_not) {
        sal_Int32 knot = (re_opcode_t) p1[3] == charset_not;

        if (c < (sal_Unicode) (p1[4] * BYTEWIDTH)
        && p1[5 + c / BYTEWIDTH] & (1 << (c % BYTEWIDTH)))
          knot = !knot;

        /* `not' is equal to 1 if c would match, which means
           that we can't change to pop_failure_jump.  */
        if (!knot) {
          p[-3] = (unsigned char) pop_failure_jump;
        }
      }
    } else if ((re_opcode_t) *p2 == charset) {
                /* We win if the first character of the loop is not part
                   of the charset.  */
      if ((re_opcode_t) p1[3] == exactn
          && ! ((int) p2[1] * BYTEWIDTH > (int) p1[5]
            && (p2[2 + p1[5] / BYTEWIDTH]
            & (1 << (p1[5] % BYTEWIDTH))))) {
        p[-3] = (sal_Unicode) pop_failure_jump;
      } else if ((re_opcode_t) p1[3] == charset_not) {
        sal_Int32 idx;
        /* We win if the charset_not inside the loop
           lists every character listed in the charset after.  */
        for (idx = 0; idx < (int) p2[1]; idx++)
          if (! (p2[2 + idx] == 0
             || (idx < (int) p1[4]
             && ((p2[2 + idx] & ~ p1[5 + idx]) == 0))))
        break;

        if (idx == p2[1]) {
          p[-3] = (sal_Unicode) pop_failure_jump;
        }
      } else if ((re_opcode_t) p1[3] == charset) {
        sal_Int32 idx;
        /* We win if the charset inside the loop
           has no overlap with the one after the loop.  */
        for (idx = 0;
         idx < (sal_Int32) p2[1] && idx < (sal_Int32) p1[4];
         idx++)
          if ((p2[2 + idx] & p1[5 + idx]) != 0)
        break;

        if (idx == p2[1] || idx == p1[4]) {
          p[-3] = (sal_Unicode) pop_failure_jump;
        }
      }
    }
      }
      p -= 2;       /* Point at relative address again.  */
      if ((re_opcode_t) p[-1] != pop_failure_jump) {
    p[-1] = (sal_Unicode) jump;
    goto unconditional_jump;
      }
      /* Note fall through.  */


      /* The end of a simple repeat has a pop_failure_jump back to
     its matching on_failure_jump, where the latter will push a
     failure point.  The pop_failure_jump takes off failure
     points put on by this pop_failure_jump's matching
     on_failure_jump; we got through the pattern to here from the
     matching on_failure_jump, so didn't fail.  */
    case pop_failure_jump:
      {
    /* We need to pass separate storage for the lowest and
       highest registers, even though we don't care about the
       actual values.  Otherwise, we will restore only one
       register from the stack, since lowest will == highest in
       `pop_failure_point'.  */
    sal_uInt32 dummy_low_reg, dummy_high_reg;
    sal_Unicode *pdummy = NULL;
    const sal_Unicode *sdummy = NULL;

    POP_FAILURE_POINT(sdummy, pdummy,
              dummy_low_reg, dummy_high_reg,
              reg_dummy, reg_dummy, reg_info_dummy);
      }
      /* Note fall through.  */

    unconditional_jump:
    /* Note fall through.  */

    /* Unconditionally jump (without popping any failure points).  */
    case jump:
      extract_number_and_incr(mcnt, p); /* Get the amount to jump.  */
      p += mcnt;                /* Do the jump.  */
      break;

      /* We need this opcode so we can detect where alternatives end
     in `group_match_null_string_p' et al.  */
    case jump_past_alt:
      goto unconditional_jump;


      /* Normally, the on_failure_jump pushes a failure point, which
     then gets popped at pop_failure_jump.  We will end up at
     pop_failure_jump, also, and with a pattern of, say, `a+', we
     are skipping over the on_failure_jump, so we have to push
     something meaningless for pop_failure_jump to pop.  */
    case dummy_failure_jump:
      /* It doesn't matter what we push for the string here.  What
     the code at `fail' tests is the value for the pattern.  */
      PUSH_FAILURE_POINT(NULL, NULL, -2);
      goto unconditional_jump;


      /* At the end of an alternative, we need to push a dummy failure
     point in case we are followed by a `pop_failure_jump', because
     we don't want the failure point for the alternative to be
     popped.  For example, matching `(a|ab)*' against `aab'
     requires that we match the `ab' alternative.  */
    case push_dummy_failure:
      /* See comments just above at `dummy_failure_jump' about the
     two zeroes.  */
      PUSH_FAILURE_POINT(NULL, NULL, -2);
      break;

      /* Have to succeed matching what follows at least n times.
     After that, handle like `on_failure_jump'.  */
    case succeed_n:
      extract_number(mcnt, p + 2);

      assert (mcnt >= 0);
      /* Originally, this is how many times we HAVE to succeed.  */
      if (mcnt > 0) {
    mcnt--;
    p += 2;
    store_number_and_incr (p, mcnt);
      } else if (mcnt == 0) {
    p[2] = (sal_Unicode) no_op;
    p[3] = (sal_Unicode) no_op;
    goto on_failure;
      }
      break;

    case jump_n:
      extract_number(mcnt, p + 2);

      /* Originally, this is how many times we CAN jump.  */
      if (mcnt) {
    mcnt--;
    store_number (p + 2, mcnt);
    goto unconditional_jump;
      }
      /* If don't have to jump any more, skip over the rest of command.  */
      else
    p += 4;
      break;

    case set_number_at:
      {

    extract_number_and_incr(mcnt, p);
    p1 = p + mcnt;
    extract_number_and_incr(mcnt, p);
    store_number (p1, mcnt);
    break;
      }

    case wordbeg:
      if (iswordbegin(d, string2, size2))
    break;
      goto fail;

    case wordend:
      if (iswordend(d, string2, size2))
    break;
      goto fail;


    default:
      abort();
    }
    continue;  /* Successfully executed one pattern command; keep going.  */

    /* We goto here if a matching operation fails. */
  fail:
    if (!FAIL_STACK_EMPTY()) {
      /* A restart point is known.  Restore to that state.  */
      POP_FAILURE_POINT(d, p,
            lowest_active_reg, highest_active_reg,
            regstart, regend, reg_info);

      /* If this failure point is a dummy, try the next one.  */
      if (!p)
    goto fail;

      /* If we failed to the end of the pattern, don't examine *p.  */
      assert(p <= pend);
      if (p < pend) {
    sal_Bool is_a_jump_n = false;

    /* If failed to a backwards jump that's part of a repetition
       loop, need to pop this failure point and use the next
       one.  */
    switch ((re_opcode_t) *p) {
    case jump_n:
      is_a_jump_n = true;
    case maybe_pop_jump:
    case pop_failure_jump:
    case jump:
      p1 = p + 1;
      extract_number_and_incr(mcnt, p1);
      p1 += mcnt;

      if ((is_a_jump_n && (re_opcode_t) *p1 == succeed_n)
          || (!is_a_jump_n
          && (re_opcode_t) *p1 == on_failure_jump)) {
        goto fail;
      }
      break;
    default:
      /* do nothing */ ;
    }
      }

    } else {
      break;   /* Matching at this starting point really fails.  */
    }
  } /* for (;;) */

  FREE_VARIABLES ();

  return(-1);                   /* Failure to match.  */
} /* re_match2 */

/* Set the bit for character C in a list.  */
void
Regexpr::set_list_bit(sal_Unicode c, sal_Unicode *b)
{
  if ( translate ) {
    try {
        sal_Unicode tmp = translit->transliterateChar2Char(c);
        b[tmp / BYTEWIDTH] |= 1 << (tmp % BYTEWIDTH);
    } catch (::com::sun::star::i18n::MultipleCharsOutputException e) {
        ::rtl::OUString o2( translit->transliterateChar2String( c));
        sal_Int32 len2 = o2.getLength();
        const sal_Unicode * k2 = o2.getStr();
        for (sal_Int32 nmatch = 0; nmatch < len2; nmatch++) {
          b[k2[nmatch] / BYTEWIDTH] |= 1 << (k2[nmatch] % BYTEWIDTH);
        }
    }
  } else {
    b[c / BYTEWIDTH] |= 1 << (c % BYTEWIDTH);
  }
}

/* vim: set ts=8 sw=2 noexpandtab: */
