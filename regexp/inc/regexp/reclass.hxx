/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* Definitions for data structures and routines for the regular
   expression library, version 0.12.
   Copyright (C) 1985,89,90,91,92,93,95,96,97,98 Free Software Foundation, Inc.

   This file is part of the GNU C Library.  Its master source is NOT part of
   the C library, however.  The master source lives in /gd/gnu/lib.

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

#ifndef INCLUDED_REGEXP_RECLASS_HXX
#define INCLUDED_REGEXP_RECLASS_HXX

#include <i18nutil/unicode.hxx>
#include <com/sun/star/util/SearchFlags.hpp>
#include <com/sun/star/util/SearchOptions.hpp>
#include <sal/types.h>
#include <com/sun/star/i18n/XExtendedTransliteration.hpp>

#if defined REGEXP_DLLIMPLEMENTATION
#define REGEXP_DLLPUBLIC SAL_DLLPUBLIC_EXPORT
#else
#define REGEXP_DLLPUBLIC SAL_DLLPUBLIC_IMPORT
#endif

/* If any error codes are removed, changed, or added, update the
   `re_error_msg' table in regex.c.  */
typedef enum
{
#ifdef _XOPEN_SOURCE
  REG_ENOSYS = -1,  ///< This will never happen for this implementation.
#endif

  REG_NOERROR = 0,  ///< Success.
  REG_NOMATCH,      ///< Didn't find a match (for regexec).

  /* POSIX regcomp return error codes.  (In the order listed in the
     standard.)  */
  REG_BADPAT,       ///< Invalid pattern.
  REG_ECOLLATE,     ///< Not implemented.
  REG_ECTYPE,       ///< Invalid character class name.
  REG_EESCAPE,      ///< Trailing backslash.
  REG_ESUBREG,      ///< Invalid back reference.
  REG_EBRACK,       ///< Unmatched left bracket.
  REG_EPAREN,       ///< Parenthesis imbalance.
  REG_EBRACE,       ///< Unmatched \{.
  REG_BADBR,        ///< Invalid contents of \{\}.
  REG_ERANGE,       ///< Invalid range end.
  REG_ESPACE,       ///< Ran out of memory.
  REG_BADRPT,       ///< No preceding re for repetition op.

  /* Error codes we've added.  */
  REG_EEND,         ///< Premature end.
  REG_ESIZE,        ///< Compiled pattern bigger than 2^16 bytes.
  REG_ERPAREN       ///< Unmatched ) or \); not returned from regcomp.
} reg_errcode_t;


/** This data structure represents a compiled pattern.  Before calling
   the pattern compiler, the fields `buffer', `allocated', `fastmap',
   can be set.  After the pattern has been
   compiled, the `re_nsub' field is available.  All other fields are
   private to the regex routines.  */

struct REGEXP_DLLPUBLIC re_pattern_buffer
{
/* [[[begin pattern_buffer]]] */
    /* Space that holds the compiled pattern.  It is declared as
       `unsigned char *' because its elements are
        sometimes used as array indexes. */
  sal_Unicode *buffer;

  /// Number of bytes to which `buffer' points.
  sal_uInt32 allocated;

  /// Number of bytes actually used in `buffer'.
  sal_uInt32 used;

  /** Pointer to a fastmap, if any, otherwise zero.  re_search uses the fastmap,
      if there is one, to skip over impossible starting points for matches. */
  sal_Unicode *fastmap;

  /// Number of subexpressions found by the compiler.
  size_t re_nsub;

  /** Zero if this pattern cannot match the empty string, one else. Well, in
      truth it's used only in `re_search2', to see whether or not we should use
      the fastmap, so we don't set this absolutely perfectly;
       see `re_compile_fastmap' (the `duplicate' case). */
  unsigned can_be_null : 1;

  /** Set to zero when `regex_compile' compiles a pattern; set to one
      by `re_compile_fastmap' if it updates the fastmap. */
  unsigned fastmap_accurate : 1;

  /** If set, a beginning-of-line anchor doesn't
      match at the beginning of the string. */
  unsigned not_bol : 1;

  /// Similarly for an end-of-line anchor.
  unsigned not_eol : 1;

  /// If true, an anchor at a newline matches.
  unsigned newline_anchor : 1;

/* [[[end pattern_buffer]]] */
};

/* These are the command codes that appear in compiled regular
   expressions.  Some opcodes are followed by argument bytes.  A
   command code can specify any interpretation whatsoever for its
   arguments.  Zero bytes may appear in the compiled regular expression.  */

typedef enum
{
    no_op = 0,

    /// Succeed right away -- no more backtracking.
    succeed,

    /// Followed by one byte giving n, then by n literal bytes.
    exactn,

    /// Matches any (more or less) character.
    anychar,

    /** Matches any one char belonging to specified set. First following byte is
        number of bitmap bytes. Then come bytes for a bitmap saying which chars
        are in. Bits in each byte are ordered low-bit-first.  A character is in
        the set if its bit is 1. A character too large to have a bit in the map
        is automatically not in the set. */
    charset,

    /** Same parameters as charset, but match any character
        that is not one of those specified. */
    charset_not,

    /** Start remembering the text that is matched, for storing in a register.
        Followed by one byte with the register number, in the range 0 to one
        less than the pattern buffer's re_nsub field. Then followed by one byte
        with the number of groups inner to this one. (This last has to be part
        of the start_memory only because we need it in the on_failure_jump of
        re_match2.) */
    start_memory,
    /** Stop remembering the text that is matched and store it in a memory
        register. Followed by one byte with the register number, in the range 0
        to one less than `re_nsub' in the pattern buffer, and one byte with the
        number of inner groups, just like `start_memory'. (We need the number of
        inner groups here because we don't have any easy way of finding the
         corresponding start_memory when we're at a stop_memory.) */
    stop_memory,

    /** Match a duplicate of something remembered. Followed by one
        byte containing the register number. */
    duplicate,

    /// Fail unless at beginning of line.
    begline,

    /// Fail unless at end of line.
    endline,

    /** Succeeds if at beginning of buffer (if emacs) or
        at beginning of string to be matched. */
    begbuf,

    /// Analogously, for end of buffer/string.
    endbuf,

    /// Followed by two byte relative address to which to jump.
    jump,

    /// Same as jump, but marks the end of an alternative.
    jump_past_alt,

    /** Followed by two-byte relative address of place
        to resume at in case of failure. */
    on_failure_jump,

    /** Like on_failure_jump, but pushes a placeholder instead of
        the current string position when executed. */
    on_failure_keep_string_jump,

    /** Throw away latest failure point and then
        jump to following two-byte relative address.
    pop_failure_jump,

    /** Change to pop_failure_jump if know won't have to backtrack to match;
        otherwise change to jump. This is used to jump back to the beginning of
        a repeat. If what follows this jump clearly won't match what the repeat
        does, such that we can be sure that there is no use backtracking out of
        repetitions already matched, then we change it to a pop_failure_jump.
        Followed by two-byte address. */
    maybe_pop_jump,

    /** Jump to following two-byte address, and push a dummy failure point. This
        failure point will be thrown away if an attempt is made to use it for a
        failure. A `+' construct makes this before the first repeat. Also used
        as an intermediary kind of jump when compiling an alternative. */
    dummy_failure_jump,

    /// Push a dummy failure point and continue. Used at the end of alternatives.
    push_dummy_failure,

    /** Followed by two-byte relative address and two-byte number n.
        After matching N times, jump to the address upon failure. */
    succeed_n,

    /** Followed by two-byte relative address, and two-byte number n.
        Jump to the address N times, then fail. */
    jump_n,

    /** Set the following two-byte relative address to the subsequent two-byte
        number. The address *includes* the two bytes of number.  */
    set_number_at,

    wordbeg, ///< Succeeds if at word beginning.
    wordend  ///< Succeeds if at word end.
} re_opcode_t;

typedef struct re_pattern_buffer regex_t;

/// Type for byte offsets within the string. POSIX mandates this.
typedef sal_Int32 regoff_t;

/** This is the structure we store register match data in. See
    regex.texinfo for a full description of what registers match. */
struct REGEXP_DLLPUBLIC re_registers
{
  sal_uInt32 num_regs;
  sal_Int32 *start;
  sal_Int32 *end;
  sal_Int32 num_of_match;
};

typedef struct {
    sal_Int32   begalt_offset;
    sal_Int32   fixup_alt_jump;
    sal_Int32   inner_group_offset;
    sal_Int32   laststart_offset;
    sal_uInt32  regnum;
} compile_stack_elt_t;

typedef struct {
    compile_stack_elt_t *stack;
    sal_uInt32      size;
    sal_uInt32      avail;
} compile_stack_type;

union REGEXP_DLLPUBLIC fail_stack_elt
{
    sal_Unicode *pointer;
    sal_Int32 integer;
};

typedef union fail_stack_elt fail_stack_elt_t;

typedef struct
{
    fail_stack_elt_t *stack;
    sal_uInt32 size;
    sal_uInt32 avail;  ///< Offset of next open position.
} fail_stack_type;

typedef union
{
  fail_stack_elt_t word;
  struct
  {
/* This field is one if this group can match the empty string,
   zero if not.  If not yet determined, `MATCH_NULL_UNSET_VALUE'. */
#define MATCH_NULL_UNSET_VALUE 3
    unsigned match_null_string_p : 2;
    unsigned is_active : 1;
    unsigned matched_something : 1;
    unsigned ever_matched_something : 1;
  } bits;
} register_info_type;


class REGEXP_DLLPUBLIC Regexpr
{
    ::com::sun::star::uno::Reference<
    ::com::sun::star::i18n::XExtendedTransliteration > translit;

    const sal_Unicode *line; ///< line to search in.
    sal_Int32   linelen;     ///< length of search string.
    sal_Unicode *pattern;    ///< RE pattern to match.
    sal_Int32   patsize;     ///< Length of pattern.

    struct re_pattern_buffer *bufp;

    sal_Bool isIgnoreCase;

    /** Either a translate table to apply to all characters before comparing
        them, or zero for no translation. The translation is applied to a
        pattern when it is compiled and to a string when it is matched.  */
    int translate;

    sal_uInt32 failure_id;
    sal_uInt32 nfailure_points_pushed;
    sal_uInt32 nfailure_points_popped;
    sal_uInt32 num_regs_pushed;  ///< Counts the total number of registers pushed.
    sal_uInt32 re_max_failures;
    sal_Unicode reg_unset_dummy; ///< Registers are set to a sentinel when they haven't yet matched.

    // private instance functions
    inline void store_number( sal_Unicode * destination, sal_Int32 number );
    inline void store_number_and_incr( sal_Unicode *& destination, sal_Int32 number );
    inline void extract_number(sal_Int32 & dest, sal_Unicode *source);
    inline void extract_number_and_incr(sal_Int32 & destination, sal_Unicode *& source);

    sal_Bool group_match_null_string_p(sal_Unicode **p, sal_Unicode *end,
                       register_info_type *reg_info);
    sal_Bool alt_match_null_string_p(sal_Unicode *p, sal_Unicode *end,
                     register_info_type *reg_info);

    sal_Bool common_op_match_null_string_p(sal_Unicode **p, sal_Unicode *end,
                       register_info_type *reg_info);
    sal_Int32 bcmp_translate(const sal_Unicode *s1,
                 const sal_Unicode *s2, sal_Int32 len);

    sal_Int32 regcomp(void);
    sal_Int32 regex_compile(void);
    inline void store_op1(re_opcode_t op, sal_Unicode *loc, sal_Int32 arg);
    inline void store_op2(re_opcode_t op, sal_Unicode *loc, sal_Int32 arg1, sal_Int32 arg2);
    void insert_op1(re_opcode_t op, sal_Unicode *loc, sal_Int32 arg,
            sal_Unicode *end);
    void insert_op2(re_opcode_t op, sal_Unicode *loc, sal_Int32 arg1,
            sal_Int32 arg2, sal_Unicode *end);
    sal_Bool at_begline_loc_p(const sal_Unicode *local_pattern,
                  const sal_Unicode *p);
    sal_Bool at_endline_loc_p(const sal_Unicode *p);
    reg_errcode_t compile_range(sal_Unicode range_begin, sal_Unicode range_end, sal_Unicode *b);
    sal_Bool group_in_compile_stack(compile_stack_type compile_stack,
                    sal_uInt32 regnum);
    sal_Int32 re_match2(struct re_registers *regs, sal_Int32 pos, sal_Int32 range);

    sal_Bool iswordbegin(const sal_Unicode *d, sal_Unicode *string, sal_Int32 ssize);
    sal_Bool iswordend(const sal_Unicode *d, sal_Unicode *string, sal_Int32 ssize);
    void set_list_bit(sal_Unicode c, sal_Unicode *b);

public:
    // constructors
    Regexpr( const ::com::sun::star::util::SearchOptions & rOptions,
            ::com::sun::star::uno::Reference<
            ::com::sun::star::i18n::XExtendedTransliteration > XTrans );

    // destructor
    ~Regexpr();

    void set_line( const sal_Unicode *line, sal_Int32 len );

    /// @return pointers to occurrences in regs.
    sal_Int32 re_search(struct re_registers *regs, sal_Int32 pOffset);  // find pattern in line
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
