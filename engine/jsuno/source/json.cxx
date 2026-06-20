/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <charconv>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <optional>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

#include <cpo/uno/Any.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Type.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/genfunc.hxx>
#include <o3tl/unreachable.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <typelib/typedescription.h>
#include <typelib/typedescription.hxx>
#include <uno/data.h>

#include "json.hxx"

namespace
{
// Append `s` to `buf` as a JSON string literal (UTF-8, RFC 8259 escapes for controls and
// quote/backslash, surrounding quotes):
void appendJsonString(OStringBuffer& buf, std::u16string_view s)
{
    buf.append('"');
    OString utf8 = OUString(s.data(), s.size()).toUtf8();
    for (sal_Int32 i = 0; i != utf8.getLength(); ++i)
    {
        char c = utf8[i];
        switch (c)
        {
            case '"':
                buf.append("\\\"");
                break;
            case '\\':
                buf.append("\\\\");
                break;
            case '\b':
                buf.append("\\b");
                break;
            case '\f':
                buf.append("\\f");
                break;
            case '\n':
                buf.append("\\n");
                break;
            case '\r':
                buf.append("\\r");
                break;
            case '\t':
                buf.append("\\t");
                break;
            default:
                if (static_cast<unsigned char>(c) < 0x20)
                {
                    char tmp[8];
                    std::snprintf(tmp, sizeof(tmp), "\\u%04x", static_cast<unsigned>(c));
                    buf.append(std::string_view(tmp, 6));
                }
                else
                {
                    buf.append(c);
                }
                break;
        }
    }
    buf.append('"');
}

bool isJsonWS(sal_Unicode c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }

// Parse `json` as a number of type T (range-checked, must be a complete in-range token);
// returns std::nullopt otherwise:
template <typename T> std::optional<T> parseJsonNumberAs(OUString const& json)
{
    auto const utf8 = json.toUtf8();
    T value;
    auto const[p, ec] = std::from_chars(utf8.getStr(), utf8.getStr() + utf8.getLength(), value);
    if (ec != std::errc{} || p != utf8.getStr() + utf8.getLength())
    {
        return std::nullopt;
    }
    return value;
}

// Return the position just past the JSON string token that starts at `json[pos]` (which must
// be `"`), or -1 on syntax error or unterminated string:
sal_Int32 findJsonStringEnd(OUString const& json, sal_Int32 pos)
{
    sal_Int32 const len = json.getLength();
    if (pos >= len || json[pos] != '"')
    {
        return -1;
    }
    sal_Int32 i = pos + 1;
    bool esc = false;
    while (i != len)
    {
        sal_Unicode const c = json[i++];
        if (esc)
        {
            esc = false;
            continue;
        }
        if (c == '\\')
        {
            esc = true;
            continue;
        }
        if (c == '"')
        {
            return i;
        }
    }
    return -1;
}

// Decode `json` (which must be exactly one well-formed JSON string token, with no
// leading/trailing junk) to its underlying text, or std::nullopt on syntax error:
std::optional<OUString> parseJsonStringValue(OUString const& json)
{
    if (findJsonStringEnd(json, 0) != json.getLength())
    {
        return std::nullopt;
    }
    sal_Int32 const end = json.getLength() - 1;
    OUStringBuffer buf(end - 1);
    sal_Int32 i = 1;
    while (i != end)
    {
        sal_Unicode const c = json[i++];
        if (c != '\\')
        {
            buf.append(c);
            continue;
        }
        sal_Unicode const e = json[i++];
        switch (e)
        {
            case '"':
            case '\\':
            case '/':
                buf.append(e);
                break;
            case 'b':
                buf.append(u'\b');
                break;
            case 'f':
                buf.append(u'\f');
                break;
            case 'n':
                buf.append(u'\n');
                break;
            case 'r':
                buf.append(u'\r');
                break;
            case 't':
                buf.append(u'\t');
                break;
            case 'u':
            {
                if (end - i < 4)
                {
                    return std::nullopt;
                }
                sal_uInt32 v = 0;
                for (int k = 0; k != 4; ++k)
                {
                    sal_Unicode const h = json[i++];
                    sal_uInt32 d;
                    if (h >= '0' && h <= '9')
                    {
                        d = h - '0';
                    }
                    else if (h >= 'a' && h <= 'f')
                    {
                        d = 10 + (h - 'a');
                    }
                    else if (h >= 'A' && h <= 'F')
                    {
                        d = 10 + (h - 'A');
                    }
                    else
                    {
                        return std::nullopt;
                    }
                    v = (v << 4) | d;
                }
                buf.append(static_cast<sal_Unicode>(v));
                break;
            }
            default:
                return std::nullopt;
        }
    }
    return buf.makeStringAndClear();
}

// Return the position just past one JSON value starting at `json[pos]` (no leading-whitespace
// skipping, bracket-balanced through nested arrays/objects, strings respected), or -1 on
// syntax error or premature end:
sal_Int32 findJsonValueEnd(OUString const& json, sal_Int32 pos)
{
    sal_Int32 const len = json.getLength();
    sal_Int32 i = pos;
    int depth = 0;
    bool inStr = false;
    bool esc = false;
    while (i != len)
    {
        sal_Unicode const c = json[i];
        if (inStr)
        {
            if (esc)
            {
                esc = false;
            }
            else if (c == '\\')
            {
                esc = true;
            }
            else if (c == '"')
            {
                inStr = false;
            }
            ++i;
        }
        else if (c == '"')
        {
            inStr = true;
            ++i;
        }
        else if (c == '[' || c == '{')
        {
            ++depth;
            ++i;
        }
        else if (c == ']' || c == '}')
        {
            if (depth == 0)
            {
                return i;
            }
            --depth;
            ++i;
            if (depth == 0)
            {
                return i;
            }
        }
        else if (c == ',' && depth == 0)
        {
            return i;
        }
        else
        {
            ++i;
        }
    }
    return depth == 0 && !inStr ? i : -1;
}

// Split a JSON array `json` into its (whitespace-trimmed) element substrings into `out`, or
// return false on syntax error:
bool splitJsonArray(OUString const& json, std::vector<OUString>& out)
{
    sal_Int32 const len = json.getLength();
    sal_Int32 i = 0;
    while (i != len && isJsonWS(json[i]))
    {
        ++i;
    }
    if (i == len || json[i] != '[')
    {
        return false;
    }
    ++i;
    while (i != len && isJsonWS(json[i]))
    {
        ++i;
    }
    if (i != len && json[i] == ']')
    {
        return true;
    }
    while (i != len)
    {
        sal_Int32 const start = i;
        sal_Int32 const e = findJsonValueEnd(json, i);
        if (e < 0 || e == len)
        {
            return false;
        }
        sal_Int32 trimmed = e;
        while (trimmed > start && isJsonWS(json[trimmed - 1]))
        {
            --trimmed;
        }
        out.push_back(json.copy(start, trimmed - start));
        i = e;
        if (json[i] == ']')
        {
            return true;
        }
        if (json[i] != ',')
        {
            return false;
        }
        ++i;
        while (i != len && isJsonWS(json[i]))
        {
            ++i;
        }
    }
    return false;
}

// Split a JSON object `json` into a key->(whitespace-trimmed-)value-substring map in `out`
// (duplicate keys: last one wins), or return false on syntax error:
bool splitJsonObject(OUString const& json, std::map<OUString, OUString>& out)
{
    sal_Int32 const len = json.getLength();
    sal_Int32 i = 0;
    while (i != len && isJsonWS(json[i]))
    {
        ++i;
    }
    if (i == len || json[i] != '{')
    {
        return false;
    }
    ++i;
    while (i != len && isJsonWS(json[i]))
    {
        ++i;
    }
    if (i != len && json[i] == '}')
    {
        return true;
    }
    while (i != len)
    {
        if (json[i] != '"')
        {
            return false;
        }
        sal_Int32 const keyStart = i;
        sal_Int32 const keyEnd = findJsonStringEnd(json, i);
        if (keyEnd < 0)
        {
            return false;
        }
        auto const key = parseJsonStringValue(json.copy(keyStart, keyEnd - keyStart));
        if (!key)
        {
            return false;
        }
        i = keyEnd;
        while (i != len && isJsonWS(json[i]))
        {
            ++i;
        }
        if (i == len || json[i] != ':')
        {
            return false;
        }
        ++i;
        while (i != len && isJsonWS(json[i]))
        {
            ++i;
        }
        sal_Int32 const valStart = i;
        sal_Int32 const valEnd = findJsonValueEnd(json, i);
        if (valEnd < 0 || valEnd == len)
        {
            return false;
        }
        sal_Int32 trimmed = valEnd;
        while (trimmed > valStart && isJsonWS(json[trimmed - 1]))
        {
            --trimmed;
        }
        out[*key] = json.copy(valStart, trimmed - valStart);
        i = valEnd;
        if (json[i] == '}')
        {
            return true;
        }
        if (json[i] != ',')
        {
            return false;
        }
        ++i;
        while (i != len && isJsonWS(json[i]))
        {
            ++i;
        }
    }
    return false;
}
}

void appendUnoAsJson(OStringBuffer& buf, css::uno::Type const& type, void const* value)
{
    switch (type.getTypeClass())
    {
        case css::uno::TypeClass_VOID:
            buf.append("null");
            return;
        case css::uno::TypeClass_BOOLEAN:
            buf.append(*static_cast<bool const*>(value) ? "true" : "false");
            return;
        case css::uno::TypeClass_BYTE:
            buf.append(static_cast<sal_Int32>(*static_cast<sal_Int8 const*>(value)));
            return;
        case css::uno::TypeClass_SHORT:
            buf.append(static_cast<sal_Int32>(*static_cast<sal_Int16 const*>(value)));
            return;
        case css::uno::TypeClass_UNSIGNED_SHORT:
            buf.append(static_cast<sal_Int32>(*static_cast<sal_uInt16 const*>(value)));
            return;
        case css::uno::TypeClass_LONG:
            buf.append(*static_cast<sal_Int32 const*>(value));
            return;
        case css::uno::TypeClass_UNSIGNED_LONG:
            buf.append(static_cast<sal_Int64>(*static_cast<sal_uInt32 const*>(value)));
            return;
        case css::uno::TypeClass_HYPER:
            buf.append(*static_cast<sal_Int64 const*>(value));
            return;
        case css::uno::TypeClass_UNSIGNED_HYPER:
            buf.append(OString::number(*static_cast<sal_uInt64 const*>(value)));
            return;
        case css::uno::TypeClass_FLOAT:
        {
            auto const v = *static_cast<float const*>(value);
            if (!std::isfinite(v))
            {
                throw css::uno::RuntimeException(u"appendUnoAsJson: non-finite "_ustr
                                                 + type.getTypeName()
                                                 + u" cannot be converted to JSON"_ustr);
            }
            buf.append(static_cast<double>(v));
            return;
        }
        case css::uno::TypeClass_DOUBLE:
        {
            auto const v = *static_cast<double const*>(value);
            if (!std::isfinite(v))
            {
                throw css::uno::RuntimeException(u"appendUnoAsJson: non-finite "_ustr
                                                 + type.getTypeName()
                                                 + u" cannot be converted to JSON"_ustr);
            }
            buf.append(v);
            return;
        }
        case css::uno::TypeClass_CHAR:
        {
            sal_Unicode c = *static_cast<sal_Unicode const*>(value);
            appendJsonString(buf, std::u16string_view(&c, 1));
            return;
        }
        case css::uno::TypeClass_STRING:
            appendJsonString(buf, OUString::unacquired(static_cast<rtl_uString* const*>(value)));
            return;
        case css::uno::TypeClass_TYPE:
        {
            auto const& ref = *static_cast<typelib_TypeDescriptionReference* const*>(value);
            appendJsonString(buf, OUString::unacquired(&ref->pTypeName));
            return;
        }
        case css::uno::TypeClass_ANY:
        {
            auto const any = static_cast<uno_Any const*>(value);
            appendUnoAsJson(buf, *reinterpret_cast<css::uno::Type const*>(&any->pType), any->pData);
            return;
        }
        case css::uno::TypeClass_SEQUENCE:
        {
            auto const seq = *static_cast<uno_Sequence* const*>(value);
            css::uno::TypeDescription desc(type);
            css::uno::TypeDescription elemDesc(
                reinterpret_cast<typelib_IndirectTypeDescription const*>(desc.get())->pType);
            buf.append('[');
            for (sal_Int32 i = 0; i != seq->nElements; ++i)
            {
                if (i != 0)
                {
                    buf.append(',');
                }
                appendUnoAsJson(buf, elemDesc.get()->pWeakRef,
                                seq->elements + (i * elemDesc.get()->nSize));
            }
            buf.append(']');
            return;
        }
        case css::uno::TypeClass_ENUM:
        {
            auto const val = *static_cast<sal_Int32 const*>(value);
            css::uno::TypeDescription desc(type);
            auto const enumDesc = reinterpret_cast<typelib_EnumTypeDescription const*>(desc.get());
            for (sal_Int32 i = 0; i != enumDesc->nEnumValues; ++i)
            {
                if (enumDesc->pEnumValues[i] == val)
                {
                    appendJsonString(buf, OUString::unacquired(&enumDesc->ppEnumNames[i]));
                    return;
                }
            }
            throw css::uno::RuntimeException(u"appendUnoAsJson: invalid value "_ustr
                                             + OUString::number(val) + u" for UNO enum "_ustr
                                             + type.getTypeName());
        }
        case css::uno::TypeClass_STRUCT:
        case css::uno::TypeClass_EXCEPTION:
        {
            css::uno::TypeDescription desc(type);
            auto compDesc = reinterpret_cast<typelib_CompoundTypeDescription const*>(desc.get());
            buf.append('{');
            bool first = true;
            for (;;)
            {
                for (sal_Int32 i = 0; i != compDesc->nMembers; ++i)
                {
                    if (!first)
                    {
                        buf.append(',');
                    }
                    first = false;
                    appendJsonString(buf, OUString::unacquired(&compDesc->ppMemberNames[i]));
                    buf.append(':');
                    appendUnoAsJson(buf, compDesc->ppTypeRefs[i],
                                    static_cast<std::byte const*>(value)
                                        + compDesc->pMemberOffsets[i]);
                }
                compDesc = compDesc->pBaseTypeDescription;
                if (compDesc == nullptr)
                {
                    break;
                }
            }
            buf.append('}');
            return;
        }
        case css::uno::TypeClass_INTERFACE:
        {
            auto const ptr = *static_cast<css::uno::XInterface* const*>(value);
            if (ptr == nullptr)
            {
                buf.append("null");
            }
            else
            {
                //TODO: Non-null interface references could be round-tripped via JSContext-based
                // proxying once that machinery exists; until then, just always append null:
                buf.append("null");
            }
            return;
        }
        default:
            O3TL_UNREACHABLE;
    }
}

cpo::uno::Any parseJsonToAny(OUString const& json, css::uno::Type const& type)
{
    switch (type.getTypeClass())
    {
        case css::uno::TypeClass_VOID:
            return {};
        case css::uno::TypeClass_BOOLEAN:
        {
            if (json == u"true")
            {
                return cpo::uno::Any(true);
            }
            if (json == u"false")
            {
                return cpo::uno::Any(false);
            }
            throw css::uno::RuntimeException(u"JSON value "_ustr + json
                                             + u" does not parse as UNO type "_ustr
                                             + type.getTypeName());
        }
        case css::uno::TypeClass_BYTE:
        {
            auto const v = parseJsonNumberAs<sal_Int8>(json);
            if (!v)
            {
                throw css::uno::RuntimeException(u"JSON value "_ustr + json
                                                 + u" does not parse as UNO type "_ustr
                                                 + type.getTypeName());
            }
            return cpo::uno::Any(*v);
        }
        case css::uno::TypeClass_SHORT:
        {
            auto const v = parseJsonNumberAs<sal_Int16>(json);
            if (!v)
            {
                throw css::uno::RuntimeException(u"JSON value "_ustr + json
                                                 + u" does not parse as UNO type "_ustr
                                                 + type.getTypeName());
            }
            return cpo::uno::Any(*v);
        }
        case css::uno::TypeClass_UNSIGNED_SHORT:
        {
            auto const v = parseJsonNumberAs<sal_uInt16>(json);
            if (!v)
            {
                throw css::uno::RuntimeException(u"JSON value "_ustr + json
                                                 + u" does not parse as UNO type "_ustr
                                                 + type.getTypeName());
            }
            return cpo::uno::Any(*v);
        }
        case css::uno::TypeClass_LONG:
        {
            auto const v = parseJsonNumberAs<sal_Int32>(json);
            if (!v)
            {
                throw css::uno::RuntimeException(u"JSON value "_ustr + json
                                                 + u" does not parse as UNO type "_ustr
                                                 + type.getTypeName());
            }
            return cpo::uno::Any(*v);
        }
        case css::uno::TypeClass_UNSIGNED_LONG:
        {
            auto const v = parseJsonNumberAs<sal_uInt32>(json);
            if (!v)
            {
                throw css::uno::RuntimeException(u"JSON value "_ustr + json
                                                 + u" does not parse as UNO type "_ustr
                                                 + type.getTypeName());
            }
            return cpo::uno::Any(*v);
        }
        case css::uno::TypeClass_HYPER:
        {
            auto const v = parseJsonNumberAs<sal_Int64>(json);
            if (!v)
            {
                throw css::uno::RuntimeException(u"JSON value "_ustr + json
                                                 + u" does not parse as UNO type "_ustr
                                                 + type.getTypeName());
            }
            return cpo::uno::Any(*v);
        }
        case css::uno::TypeClass_UNSIGNED_HYPER:
        {
            auto const v = parseJsonNumberAs<sal_uInt64>(json);
            if (!v)
            {
                throw css::uno::RuntimeException(u"JSON value "_ustr + json
                                                 + u" does not parse as UNO type "_ustr
                                                 + type.getTypeName());
            }
            return cpo::uno::Any(*v);
        }
        case css::uno::TypeClass_FLOAT:
        {
            auto const v = parseJsonNumberAs<float>(json);
            if (!v)
            {
                throw css::uno::RuntimeException(u"JSON value "_ustr + json
                                                 + u" does not parse as UNO type "_ustr
                                                 + type.getTypeName());
            }
            return cpo::uno::Any(*v);
        }
        case css::uno::TypeClass_DOUBLE:
        {
            auto const v = parseJsonNumberAs<double>(json);
            if (!v)
            {
                throw css::uno::RuntimeException(u"JSON value "_ustr + json
                                                 + u" does not parse as UNO type "_ustr
                                                 + type.getTypeName());
            }
            return cpo::uno::Any(*v);
        }
        case css::uno::TypeClass_CHAR:
        {
            auto const decoded = parseJsonStringValue(json);
            if (!decoded || decoded->getLength() != 1)
            {
                throw css::uno::RuntimeException(u"JSON value "_ustr + json
                                                 + u" does not parse as UNO type "_ustr
                                                 + type.getTypeName());
            }
            return cpo::uno::Any((*decoded)[0]);
        }
        case css::uno::TypeClass_STRING:
        {
            auto const decoded = parseJsonStringValue(json);
            if (!decoded)
            {
                throw css::uno::RuntimeException(u"JSON value "_ustr + json
                                                 + u" does not parse as UNO type "_ustr
                                                 + type.getTypeName());
            }
            return cpo::uno::Any(*decoded);
        }
        case css::uno::TypeClass_TYPE:
        {
            auto const name = parseJsonStringValue(json);
            if (!name)
            {
                throw css::uno::RuntimeException(u"JSON value "_ustr + json
                                                 + u" does not parse as UNO type "_ustr
                                                 + type.getTypeName());
            }
            css::uno::TypeDescription tdesc(*name);
            if (!tdesc.is())
            {
                throw css::uno::RuntimeException(u"JSON value "_ustr + json
                                                 + u" does not parse as UNO type "_ustr
                                                 + type.getTypeName());
            }
            return cpo::uno::Any(css::uno::Type(tdesc.get()->pWeakRef));
        }
        case css::uno::TypeClass_ANY:
        {
            // Encoded as a JSON object {"type": "<idl-name>", "val": <value>}.  This is
            // asymmetric with fromJs's TypeClass_ANY arm, which infers the UNO type from the
            // JS value's runtime type instead.
            //TODO: also accept the {type, val} envelope in fromJs so the two directions agree.
            std::map<OUString, OUString> obj;
            if (!splitJsonObject(json, obj))
            {
                throw css::uno::RuntimeException(u"JSON value "_ustr + json
                                                 + u" does not parse as UNO type "_ustr
                                                 + type.getTypeName());
            }
            auto const itType = obj.find(u"type"_ustr);
            auto const itVal = obj.find(u"val"_ustr);
            if (itType == obj.end() || itVal == obj.end())
            {
                throw css::uno::RuntimeException(u"JSON value "_ustr + json
                                                 + u" does not parse as UNO type "_ustr
                                                 + type.getTypeName());
            }
            auto const tname = parseJsonStringValue(itType->second);
            if (!tname)
            {
                throw css::uno::RuntimeException(u"JSON value "_ustr + json
                                                 + u" does not parse as UNO type "_ustr
                                                 + type.getTypeName());
            }
            css::uno::TypeDescription tdesc(*tname);
            if (!tdesc.is())
            {
                throw css::uno::RuntimeException(u"JSON value "_ustr + json
                                                 + u" does not parse as UNO type "_ustr
                                                 + type.getTypeName());
            }
            return parseJsonToAny(itVal->second, css::uno::Type(tdesc.get()->pWeakRef));
        }
        case css::uno::TypeClass_SEQUENCE:
        {
            std::vector<OUString> elems;
            if (!splitJsonArray(json, elems))
            {
                throw css::uno::RuntimeException(u"JSON value "_ustr + json
                                                 + u" does not parse as UNO type "_ustr
                                                 + type.getTypeName());
            }
            css::uno::TypeDescription desc(type);
            css::uno::TypeDescription elemDesc(
                reinterpret_cast<typelib_IndirectTypeDescription const*>(desc.get())->pType);
            css::uno::Type const elemType(elemDesc.get()->pWeakRef);
            uno_Sequence* seq;
            uno_sequence_construct(&seq, desc.get(), nullptr, static_cast<sal_Int32>(elems.size()),
                                   css::uno::cpp_acquire);
            for (std::size_t i = 0; i != elems.size(); ++i)
            {
                cpo::uno::Any const any = parseJsonToAny(elems[i], elemType);
                uno_copyData(seq->elements + (i * elemDesc.get()->nSize),
                             const_cast<void*>(elemType.getTypeClass() == css::uno::TypeClass_ANY
                                                   ? &any
                                                   : any.getValue()),
                             elemDesc.get(), css::uno::cpp_acquire);
            }
            return { &seq, type };
        }
        case css::uno::TypeClass_ENUM:
        {
            css::uno::TypeDescription desc(type);
            auto const enumDesc = reinterpret_cast<typelib_EnumTypeDescription const*>(desc.get());
            if (auto const name = parseJsonStringValue(json))
            {
                for (sal_Int32 i = 0; i != enumDesc->nEnumValues; ++i)
                {
                    if (OUString::unacquired(&enumDesc->ppEnumNames[i]) == *name)
                    {
                        return cpo::uno::Any(&enumDesc->pEnumValues[i], type);
                    }
                }
            }
            throw css::uno::RuntimeException(u"JSON value "_ustr + json
                                             + u" does not parse as UNO type "_ustr
                                             + type.getTypeName());
        }
        case css::uno::TypeClass_STRUCT:
        case css::uno::TypeClass_EXCEPTION:
        {
            std::map<OUString, OUString> obj;
            if (!splitJsonObject(json, obj))
            {
                throw css::uno::RuntimeException(u"JSON value "_ustr + json
                                                 + u" does not parse as UNO type "_ustr
                                                 + type.getTypeName());
            }
            css::uno::TypeDescription desc(type);
            auto compDesc = reinterpret_cast<typelib_CompoundTypeDescription const*>(desc.get());
            std::vector<cpo::uno::Any> mems;
            for (;;)
            {
                for (sal_Int32 i = 0; i != compDesc->nMembers; ++i)
                {
                    OUString const name = OUString::unacquired(&compDesc->ppMemberNames[i]);
                    auto const it = obj.find(name);
                    if (it == obj.end())
                    {
                        throw css::uno::RuntimeException(
                            u"JSON object "_ustr + json + u" is missing member \""_ustr + name
                            + u"\" of UNO type "_ustr + type.getTypeName());
                    }
                    mems.push_back(
                        parseJsonToAny(it->second, css::uno::Type(compDesc->ppTypeRefs[i])));
                }
                compDesc = compDesc->pBaseTypeDescription;
                if (compDesc == nullptr)
                {
                    break;
                }
            }
            auto const buf = std::malloc(desc.get()->nSize);
            std::size_t mem = 0;
            for (compDesc = reinterpret_cast<typelib_CompoundTypeDescription const*>(desc.get());;)
            {
                for (sal_Int32 i = 0; i != compDesc->nMembers; ++i)
                {
                    css::uno::TypeDescription memDesc(compDesc->ppTypeRefs[i]);
                    uno_copyData(static_cast<std::byte*>(buf) + compDesc->pMemberOffsets[i],
                                 const_cast<void*>(compDesc->ppTypeRefs[i]->eTypeClass
                                                           == typelib_TypeClass_ANY
                                                       ? &mems[mem]
                                                       : mems[mem].getValue()),
                                 memDesc.get(), css::uno::cpp_acquire);
                    ++mem;
                }
                compDesc = compDesc->pBaseTypeDescription;
                if (compDesc == nullptr)
                {
                    break;
                }
            }
            cpo::uno::Any result(buf, type);
            uno_type_destructData(buf, type.getTypeLibType(), nullptr);
            std::free(buf);
            return result;
        }
        case css::uno::TypeClass_INTERFACE:
        {
            if (json == u"null")
            {
                css::uno::XInterface* nullRef = nullptr;
                return cpo::uno::Any(&nullRef, type);
            }
            //TODO: non-null interface references need a JSContext-based path that can
            // round-trip a wrapped UNO object identity from kit→iframe→kit.
            throw css::uno::RuntimeException(u"parseJsonToAny: non-null "_ustr + type.getTypeName()
                                             + u" not yet implemented"_ustr);
        }
        default:
            O3TL_UNREACHABLE;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
