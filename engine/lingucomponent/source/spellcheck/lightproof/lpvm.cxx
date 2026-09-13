/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "lpvm.hxx"

#include <o3tl/safeint.hxx>
#include "lprfile.hxx"

#include <sal/log.hxx>

#include <unicode/locid.h>
#include <unicode/unistr.h>

#include <algorithm>

namespace lightproof
{
Value Value::boolean(bool b)
{
    Value aValue;
    aValue.m_eType = Type::Bool;
    aValue.m_bBool = b;
    return aValue;
}

Value Value::integer(sal_Int32 n)
{
    Value aValue;
    aValue.m_eType = Type::Int;
    aValue.m_nInt = n;
    return aValue;
}

Value Value::string(OUString aString)
{
    Value aValue;
    aValue.m_eType = Type::Str;
    aValue.m_aString = std::move(aString);
    return aValue;
}

Value Value::constant(sal_uInt32 nIndex)
{
    Value aValue;
    aValue.m_eType = Type::Constant;
    aValue.m_nInt = static_cast<sal_Int32>(nIndex);
    return aValue;
}

bool Value::isTrue() const
{
    switch (m_eType)
    {
        case Type::Null:
            return false;
        case Type::Bool:
            return m_bBool;
        case Type::Int:
            return m_nInt != 0;
        case Type::Str:
            return !m_aString.isEmpty();
        case Type::Constant:
            return true;
    }
    return false;
}

namespace
{
// One option lookup. The Python looks the page up as "<lang>_<country>" and
// falls back to the first two characters of that; here the package is already
// chosen by locale, so the option name alone identifies the flag.
bool getOption(const Context& rContext, std::u16string_view rName)
{
    const sal_Int32 nIndex = rContext.rFile.findOption(rName);
    if (nIndex < 0 || o3tl::make_unsigned(nIndex) >= rContext.rOptions.size())
        return false;
    return rContext.rOptions[nIndex];
}

sal_uInt32 readU32(const sal_uInt8* pAt)
{
    return static_cast<sal_uInt32>(pAt[0]) | (static_cast<sal_uInt32>(pAt[1]) << 8)
           | (static_cast<sal_uInt32>(pAt[2]) << 16) | (static_cast<sal_uInt32>(pAt[3]) << 24);
}

sal_Int32 readS32(const sal_uInt8* pAt) { return static_cast<sal_Int32>(readU32(pAt)); }

icu::UnicodeString toIcu(const OUString& rString)
{
    return icu::UnicodeString(false, reinterpret_cast<const UChar*>(rString.getStr()),
                              rString.getLength());
}

OUString fromIcu(const icu::UnicodeString& rString)
{
    return OUString(reinterpret_cast<const sal_Unicode*>(rString.getBuffer()), rString.length());
}

OUString changeCase(const OUString& rString, bool bUpper)
{
    if (rString.isEmpty())
        return rString;
    icu::UnicodeString aText(toIcu(rString));
    // The rules ask for what Python's upper() and lower() do, which is the
    // same everywhere. Naming the root locale keeps the process locale out of
    // it, and with it the dotless i a Turkish one would bring.
    if (bUpper)
        aText.toUpper(icu::Locale::getRoot());
    else
        aText.toLower(icu::Locale::getRoot());
    return fromIcu(aText);
}

// Python str.capitalize(): the first character upper, the rest lower.
OUString capitalize(const OUString& rString)
{
    if (rString.isEmpty())
        return rString;
    const OUString aHead = rString.copy(0, 1);
    const OUString aTail = rString.copy(1);
    return changeCase(aHead, true) + changeCase(aTail, false);
}

// The group a match gave a capture, or a null Value when it took part in no
// match, which is what Python's None reads as here.
Value groupValue(const Context& rContext, sal_Int32 nGroup)
{
    if (rContext.pMatcher == nullptr)
        return Value();
    UErrorCode nStatus = U_ZERO_ERROR;
    const icu::UnicodeString aGroup = rContext.pMatcher->group(nGroup, nStatus);
    if (U_FAILURE(nStatus))
        return Value();
    UErrorCode nStartStatus = U_ZERO_ERROR;
    if (rContext.pMatcher->start(nGroup, nStartStatus) < 0)
        return Value();
    return Value::string(fromIcu(aGroup));
}

// Python "x in y": membership for a set or map constant, substring for a
// string.
bool contains(const Context& rContext, const Value& rNeedle, const Value& rHaystack)
{
    if (rHaystack.getType() == Value::Type::Constant)
        return rContext.rFile.constantContains(rHaystack.getConstant(),
                                               OUStringToOString(rNeedle.getString(),
                                                                 RTL_TEXTENCODING_UTF8));
    if (rHaystack.getType() == Value::Type::Str)
    {
        if (rNeedle.getType() != Value::Type::Str)
            return false;
        return rHaystack.getString().indexOf(rNeedle.getString()) >= 0;
    }
    return false;
}

// Python indexing: a map constant by key, a string by position, counting
// from the end for a negative index. Out of range reads as null rather than
// raising, which stops the rest of the expression without aborting the rule.
Value index(const Context& rContext, const Value& rReceiver, const Value& rKey)
{
    if (rReceiver.getType() == Value::Type::Constant)
    {
        const std::optional<OUString> aFound = rContext.rFile.constantLookup(
            rReceiver.getConstant(),
            OUStringToOString(rKey.getString(), RTL_TEXTENCODING_UTF8));
        return aFound ? Value::string(*aFound) : Value();
    }
    if (rReceiver.getType() != Value::Type::Str || rKey.getType() != Value::Type::Int)
        return Value();
    const OUString& rString = rReceiver.getString();
    sal_Int32 nAt = rKey.getInt();
    if (nAt < 0)
        nAt += rString.getLength();
    if (nAt < 0 || nAt >= rString.getLength())
        return Value();
    return Value::string(OUString(rString[nAt]));
}

// Python slicing, with the compiler's sentinel standing in for a bound the
// rule left out.
Value slice(const Value& rReceiver, sal_Int32 nLower, sal_Int32 nUpper)
{
    if (rReceiver.getType() != Value::Type::Str)
        return Value();
    const OUString& rString = rReceiver.getString();
    const sal_Int32 nLength = rString.getLength();
    const sal_Int32 nOpen = SAL_MIN_INT32;

    sal_Int32 nStart = nLower == nOpen ? 0 : nLower;
    sal_Int32 nEnd = nUpper == nOpen ? nLength : nUpper;
    if (nStart < 0)
        nStart += nLength;
    if (nEnd < 0)
        nEnd += nLength;
    nStart = std::clamp(nStart, sal_Int32(0), nLength);
    nEnd = std::clamp(nEnd, sal_Int32(0), nLength);
    if (nEnd <= nStart)
        return Value::string(OUString());
    return Value::string(rString.copy(nStart, nEnd - nStart));
}

bool runConstantPattern(const Context& rContext, sal_uInt32 nConstant, const OUString& rSubject,
                        bool bAnchored)
{
    icu::RegexMatcher* pMatcher = rContext.rHost.getConstantMatcher(nConstant);
    if (pMatcher == nullptr)
        return false;
    const icu::UnicodeString aSubject = toIcu(rSubject);
    pMatcher->reset(aSubject);
    UErrorCode nStatus = U_ZERO_ERROR;
    const bool bFound = bAnchored ? pMatcher->lookingAt(nStatus) : pMatcher->find(nStatus);
    return U_SUCCESS(nStatus) && bFound;
}
}

Value run(const Context& rContext, const sal_uInt8* pCode)
{
    const sal_uInt8* const pEnd = rContext.rFile.getCodeEnd();
    if (pCode == nullptr || pEnd == nullptr || pCode >= pEnd)
        return Value();

    std::vector<Value> aStack;
    aStack.reserve(8);
    const sal_uInt8* pAt = pCode;
    // The code is read from a file, so it is data like any other: an
    // expression that runs off its end, or jumps out of it, stops here rather
    // than reads on. The whole expression is then worth nothing, which is how
    // a rule that cannot be run is treated everywhere else.
    bool bBroken = false;

    // A malformed expression should never reach here, but a stack underflow
    // must not read past the end of the vector if one does.
    const auto pop = [&aStack]() -> Value {
        if (aStack.empty())
            return Value();
        Value aTop = std::move(aStack.back());
        aStack.pop_back();
        return aTop;
    };

    const auto readByte = [&pAt, pEnd, &bBroken]() -> sal_uInt8 {
        if (pAt >= pEnd)
        {
            bBroken = true;
            return 0;
        }
        return *pAt++;
    };

    const auto readOperand = [&pAt, pEnd, &bBroken]() -> sal_uInt32 {
        if (pEnd - pAt < 4)
        {
            bBroken = true;
            return 0;
        }
        const sal_uInt32 nOperand = readU32(pAt);
        pAt += 4;
        return nOperand;
    };

    // A jump is an offset from the start of the expression, and the code it
    // lands in is the whole blob, so the end of that is the bound.
    const auto jump = [&pAt, pCode, pEnd, &bBroken](sal_uInt32 nTarget) {
        if (nTarget >= o3tl::make_unsigned(pEnd - pCode))
        {
            bBroken = true;
            return;
        }
        pAt = pCode + nTarget;
    };

    while (!bBroken)
    {
        const sal_uInt8 nOpcode = readByte();
        if (bBroken)
            break;

        switch (nOpcode)
        {
            case OP_END:
                return aStack.empty() ? Value() : aStack.back();

            case OP_PUSH_STR:
                aStack.push_back(Value::string(rContext.rFile.getString(readOperand())));
                break;

            case OP_PUSH_INT:
                aStack.push_back(Value::integer(static_cast<sal_Int32>(readOperand())));
                break;

            case OP_PUSH_BOOL:
                aStack.push_back(Value::boolean(readOperand() != 0));
                break;

            case OP_PUSH_NULL:
                aStack.emplace_back();
                break;

            case OP_PUSH_CONST:
                aStack.push_back(Value::constant(readOperand()));
                break;

            case OP_GROUP:
                aStack.push_back(groupValue(rContext, readS32(pAt)));
                pAt += 4;
                break;

            case OP_MSTART:
            case OP_MEND:
            {
                const sal_Int32 nGroup = readS32(pAt);
                pAt += 4;
                sal_Int32 nResult = -1;
                if (rContext.pMatcher)
                {
                    UErrorCode nStatus = U_ZERO_ERROR;
                    nResult = nOpcode == OP_MSTART ? rContext.pMatcher->start(nGroup, nStatus)
                                                   : rContext.pMatcher->end(nGroup, nStatus);
                    if (U_FAILURE(nStatus))
                        nResult = -1;
                }
                aStack.push_back(Value::integer(nResult));
                break;
            }

            case OP_EQ:
            case OP_NE:
            {
                const Value aRight = pop();
                const Value aLeft = pop();
                const bool bEqual = aLeft.getType() == aRight.getType()
                                    && aLeft.getString() == aRight.getString()
                                    && aLeft.getInt() == aRight.getInt();
                aStack.push_back(Value::boolean(nOpcode == OP_EQ ? bEqual : !bEqual));
                break;
            }

            case OP_IN:
            case OP_NOT_IN:
            {
                const Value aHaystack = pop();
                const Value aNeedle = pop();
                const bool bFound = contains(rContext, aNeedle, aHaystack);
                aStack.push_back(Value::boolean(nOpcode == OP_IN ? bFound : !bFound));
                break;
            }

            case OP_CONCAT:
            {
                const Value aRight = pop();
                const Value aLeft = pop();
                aStack.push_back(Value::string(aLeft.getString() + aRight.getString()));
                break;
            }

            case OP_LOWER:
                aStack.push_back(Value::string(changeCase(pop().getString(), false)));
                break;

            case OP_UPPER:
                aStack.push_back(Value::string(changeCase(pop().getString(), true)));
                break;

            case OP_CAPITALIZE:
                aStack.push_back(Value::string(capitalize(pop().getString())));
                break;

            case OP_INDEX:
            {
                const Value aKey = pop();
                const Value aReceiver = pop();
                aStack.push_back(index(rContext, aReceiver, aKey));
                break;
            }

            case OP_SLICE:
            {
                const sal_Int32 nLower = readS32(pAt);
                const sal_Int32 nUpper = readS32(pAt + 4);
                pAt += 8;
                aStack.push_back(slice(pop(), nLower, nUpper));
                break;
            }

            case OP_RE_SEARCH:
            case OP_RE_MATCH:
            {
                const sal_uInt32 nConstant = readOperand();
                aStack.push_back(Value::boolean(runConstantPattern(
                    rContext, nConstant, pop().getString(), nOpcode == OP_RE_MATCH)));
                break;
            }

            case OP_LOAD_TEXT:
            case OP_LOAD_SENTENCE:
                // Lightproof checks a whole paragraph as one unit, so the
                // sentence and the text are the same string.
                aStack.push_back(Value::string(rContext.rText));
                break;

            case OP_LOAD_LOCALE:
                // The locale is only ever passed straight back to a host
                // function, which reads it from the context.
                aStack.emplace_back();
                break;

            case OP_LOCALE_LANG:
                aStack.push_back(Value::string(rContext.rLocale.Language));
                break;

            case OP_LOCALE_COUNTRY:
                aStack.push_back(Value::string(rContext.rLocale.Country));
                break;

            case OP_NOT:
                aStack.push_back(Value::boolean(!pop().isTrue()));
                break;

            case OP_TRUTHY:
                aStack.push_back(Value::boolean(pop().isTrue()));
                break;

            case OP_POP:
                pop();
                break;

            case OP_JMP:
                jump(readOperand());
                break;

            case OP_JMP_IF_FALSE_KEEP:
            {
                const sal_uInt32 nTarget = readOperand();
                if (!aStack.empty() && !aStack.back().isTrue())
                    jump(nTarget);
                break;
            }

            case OP_JMP_IF_TRUE_KEEP:
            {
                const sal_uInt32 nTarget = readOperand();
                if (!aStack.empty() && aStack.back().isTrue())
                    jump(nTarget);
                break;
            }

            case OP_CALL:
            {
                const sal_uInt8 nFunc = readByte();
                const sal_uInt8 nArgc = readByte();
                std::vector<Value> aArgs(nArgc);
                for (sal_uInt8 i = nArgc; i > 0; --i)
                    aArgs[i - 1] = pop();
                const auto arg = [&aArgs](sal_uInt8 nAt) -> OUString {
                    return nAt < aArgs.size() ? aArgs[nAt].getString() : OUString();
                };
                switch (nFunc)
                {
                    case FN_OPTION:
                        aStack.push_back(Value::boolean(
                            nArgc == 2 && getOption(rContext, aArgs[1].getString())));
                        break;

                    case FN_SPELL:
                        aStack.push_back(
                            Value::boolean(rContext.rHost.spell(rContext.rLocale, arg(1))));
                        break;

                    case FN_MORPH:
                    case FN_AFFIX:
                    {
                        // The rules pass "all" positionally when they pass it
                        // at all, and it defaults to true.
                        const bool bAll = aArgs.size() < 4 || aArgs[3].isTrue();
                        const OUString aResult
                            = rContext.rHost.morph(rContext.rLocale, arg(1), arg(2), bAll,
                                                   nFunc == FN_AFFIX);
                        aStack.push_back(aResult.isEmpty() ? Value()
                                                           : Value::string(aResult));
                        break;
                    }

                    case FN_MEASUREMENT:
                        aStack.push_back(Value::string(rContext.rHost.measurement(
                            arg(0), arg(1), arg(2), arg(3), arg(4), arg(5))));
                        break;

                    default:
                        SAL_WARN("lingucomponent.lightproof",
                                 "host function " << int(nFunc) << " is not implemented");
                        aStack.emplace_back();
                        break;
                }
                break;
            }

            default:
                SAL_WARN("lingucomponent.lightproof", "opcode " << int(nOpcode) << " is not implemented");
                return Value();
        }
    }

    return Value();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
