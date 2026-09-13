/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <com/sun/star/lang/Locale.hpp>
#include <rtl/ustring.hxx>

#include <unicode/regex.h>

#include <vector>

namespace lightproof
{
class RuleFile;

// Opcodes. The numbering is part of the .lpr format and is shared with
// lpcompile.py: append, never reorder.
enum Opcode : sal_uInt8
{
    OP_END = 0,
    OP_PUSH_STR = 1,
    OP_PUSH_INT = 2,
    OP_PUSH_BOOL = 3,
    OP_PUSH_NULL = 4,
    OP_PUSH_CONST = 5,
    OP_LOAD_TEXT = 6,
    OP_LOAD_SENTENCE = 7,
    OP_LOAD_LOCALE = 8,
    OP_LOCALE_LANG = 9,
    OP_LOCALE_COUNTRY = 10,
    OP_GROUP = 11,
    OP_MSTART = 12,
    OP_MEND = 13,
    OP_NOT = 14,
    OP_JMP_IF_FALSE_KEEP = 15,
    OP_JMP_IF_TRUE_KEEP = 16,
    OP_JMP = 17,
    OP_POP = 18,
    OP_EQ = 19,
    OP_NE = 20,
    OP_IN = 21,
    OP_NOT_IN = 22,
    OP_CONCAT = 23,
    OP_LOWER = 24,
    OP_UPPER = 25,
    OP_CAPITALIZE = 26,
    OP_REPLACE = 27,
    OP_TRANSLATE = 28,
    OP_SLICE = 29,
    OP_INDEX = 30,
    OP_RE_SEARCH = 31,
    OP_RE_MATCH = 32,
    OP_RE_SUB = 33,
    OP_CALL = 34,
    OP_TRUTHY = 35
};

// Host function ids, also shared with lpcompile.py.
enum HostFunc : sal_uInt8
{
    FN_OPTION = 0,
    FN_SPELL = 1,
    FN_MORPH = 2,
    FN_AFFIX = 3,
    FN_STEM = 4,
    FN_GENERATE = 5,
    FN_SUGGEST = 6,
    FN_WORD = 7,
    FN_WORDMIN = 8,
    FN_CALC = 9,
    FN_MEASUREMENT = 10,
    FN_SUGGEST_FOREIGN = 11
};

// A value on the expression stack. The type tag exists so that Python's
// truthiness rules, which the rules depend on, can be reproduced exactly:
// null, false, zero and the empty string are false, everything else is true.
class Value
{
public:
    enum class Type
    {
        Null,
        Bool,
        Int,
        Str,
        // Several strings, which is what stem() and generate() answer with.
        List,
        // An index into the rule file's constant pool: a word set, a lookup
        // map or one of the module-level patterns.
        Constant
    };

    Value()
        : m_eType(Type::Null)
    {
    }
    static Value boolean(bool b);
    static Value integer(sal_Int32 n);
    static Value string(OUString aValue);
    static Value constant(sal_uInt32 nIndex);
    static Value list(std::vector<OUString> aValues);

    Type getType() const { return m_eType; }
    bool isTrue() const;
    const OUString& getString() const { return m_aString; }
    sal_Int32 getInt() const { return m_nInt; }
    sal_uInt32 getConstant() const { return static_cast<sal_uInt32>(m_nInt); }
    const std::vector<OUString>& getList() const { return m_aList; }

private:
    Type m_eType;
    bool m_bBool = false;
    sal_Int32 m_nInt = 0;
    OUString m_aString;
    std::vector<OUString> m_aList;
};

// The parts of a running check that live outside the expression: the current
// match, the dictionaries, and the unit conversion. Implemented by the
// checker, which owns all three.
class Host
{
public:
    virtual ~Host() = default;

    // The spelling of a word, and the last morphological analysis of it that
    // the pattern matches, which is empty when it matches none.
    virtual bool spell(const css::lang::Locale& rLocale, const OUString& rWord) = 0;
    virtual OUString morph(const css::lang::Locale& rLocale, const OUString& rWord,
                           const OUString& rPattern, bool bAll, bool bOnlyAffix)
        = 0;
    // The four-argument form the Hungarian rules use fixes the decimal mark
    // and drops the fraction handling, so it is told apart here.
    virtual OUString measurement(const OUString& rNumber, const OUString& rFrom,
                                 const OUString& rTo, const OUString& rSuffix,
                                 const OUString& rDecimal, const OUString& rRemove,
                                 bool bLongForm)
        = 0;
    virtual std::vector<OUString> stem(const css::lang::Locale& rLocale, const OUString& rWord) = 0;
    virtual std::vector<OUString> generate(const css::lang::Locale& rLocale, const OUString& rWord,
                                           const OUString& rExample)
        = 0;
    virtual OUString suggest(const css::lang::Locale& rLocale, const OUString& rWord) = 0;
    virtual OUString numberText(const OUString& rNumber, const OUString& rLanguage) = 0;
    // A matcher for one of the rule file's pattern constants, built on first
    // use and owned by the caller's package.
    virtual icu::RegexMatcher* getConstantMatcher(sal_uInt32 nConstantIndex) = 0;
};

// What a running expression can reach outside its own stack. Options are
// indexed in .lpr file order.
struct Context
{
    const RuleFile& rFile;
    const css::lang::Locale& rLocale;
    const OUString& rText;
    const std::vector<bool>& rOptions;
    // The match the expression is being evaluated for, and the group numbers
    // its rule gave its named groups.
    icu::RegexMatcher* pMatcher;
    Host& rHost;
};

// Runs one expression. Returns a null Value if the bytecode is malformed,
// which reads as false and as an empty string.
Value run(const Context& rContext, const sal_uInt8* pCode);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
