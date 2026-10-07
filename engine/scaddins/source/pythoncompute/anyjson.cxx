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

#include "anyjson.hxx"

#include <cpo/uno/RuntimeException.hpp>
#include <comphelper/json.hxx>
#include <cpo/uno/Sequence.hxx>
#include <cpo/uno/Type.hxx>
#include <o3tl/string_view.hxx>
#include <rtl/math.hxx>
#include <rtl/textcvt.h>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <tools/json_writer.hxx>

#include <algorithm>
#include <cmath>
#include <exception>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

using namespace co::pythoncompute;

cpo::uno::Any co::pythoncompute::makeFormulaErrorAny(FormulaError e)
{
    // Prefer void for #N/A — SetResult maps TypeClass_VOID → NotAvailable.
    if (e == FormulaError::NotAvailable)
        return cpo::uno::Any();
    return cpo::uno::Any(CreateDoubleError(e));
}

namespace
{
/** Detect 1×N or N×1 nested sequences and flatten to a 1D JSON array. */
template <typename T>
bool tryFlattenRowOrCol(const cpo::uno::Sequence<cpo::uno::Sequence<T>>& aGrid,
                        cpo::uno::Sequence<T>& rFlat)
{
    const sal_Int32 nRows = aGrid.getLength();
    if (nRows <= 0)
        return false;
    if (nRows == 1)
    {
        rFlat = aGrid[0];
        return true;
    }
    for (sal_Int32 r = 0; r < nRows; ++r)
    {
        if (aGrid[r].getLength() != 1)
            return false;
    }
    rFlat.realloc(nRows);
    auto* p = rFlat.getArray();
    for (sal_Int32 r = 0; r < nRows; ++r)
        p[r] = aGrid[r][0];
    return true;
}

void writeAnyValue(tools::JsonWriter& rWriter, const cpo::uno::Any& aValue,
                   std::optional<std::string_view> oName = std::nullopt);

template <typename Fn>
void withArray(tools::JsonWriter& rWriter, std::optional<std::string_view> oName, Fn fn)
{
    if (oName)
    {
        auto aArr = rWriter.startArray(*oName);
        fn();
    }
    else
    {
        auto aArr = rWriter.startAnonArray();
        fn();
    }
}

void writeNull(tools::JsonWriter& rWriter, std::optional<std::string_view> oName)
{
    if (oName)
    {
        std::string field = "\"";
        field += *oName;
        field += "\": null";
        rWriter.putRaw(field);
    }
    else
        rWriter.putRaw("null");
}

/** A numeric Any as double. Any's own >>= (double&) does not take the 64-bit integer
 *  types, so they are read as such and converted here. */
double numericAnyToDouble(const cpo::uno::Any& aValue)
{
    switch (aValue.getValueTypeClass())
    {
        case cpo::uno::TypeClass_HYPER:
        {
            sal_Int64 n = 0;
            aValue >>= n;
            return static_cast<double>(n);
        }
        case cpo::uno::TypeClass_UNSIGNED_HYPER:
        {
            sal_uInt64 n = 0;
            aValue >>= n;
            return static_cast<double>(n);
        }
        default:
        {
            double f = 0.0;
            aValue >>= f;
            return f;
        }
    }
}

void writeNumber(tools::JsonWriter& rWriter, std::optional<std::string_view> oName, double f)
{
    if (std::isnan(f) || std::isinf(f))
        writeNull(rWriter, oName);
    else if (oName)
        rWriter.put(*oName, f);
    else
        rWriter.putSimpleValue(f);
}

void writeSequenceElements(tools::JsonWriter& rWriter,
                           const cpo::uno::Sequence<cpo::uno::Any>& aSeq)
{
    for (sal_Int32 i = 0; i < aSeq.getLength(); ++i)
        writeAnyValue(rWriter, aSeq[i]);
}

void writeAnyGrid(tools::JsonWriter& rWriter,
                  const cpo::uno::Sequence<cpo::uno::Sequence<cpo::uno::Any>>& aGrid,
                  std::optional<std::string_view> oName)
{
    cpo::uno::Sequence<cpo::uno::Any> aRowCol;
    if (tryFlattenRowOrCol(aGrid, aRowCol))
    {
        withArray(rWriter, oName, [&]() { writeSequenceElements(rWriter, aRowCol); });
        return;
    }
    withArray(rWriter, oName, [&]() {
        for (sal_Int32 r = 0; r < aGrid.getLength(); ++r)
        {
            auto aInner = rWriter.startAnonArray();
            writeSequenceElements(rWriter, aGrid[r]);
        }
    });
}

void writeDoubleGrid(tools::JsonWriter& rWriter,
                     const cpo::uno::Sequence<cpo::uno::Sequence<double>>& aGrid,
                     std::optional<std::string_view> oName)
{
    cpo::uno::Sequence<double> aRowCol;
    if (tryFlattenRowOrCol(aGrid, aRowCol))
    {
        withArray(rWriter, oName, [&]() {
            for (sal_Int32 i = 0; i < aRowCol.getLength(); ++i)
                writeNumber(rWriter, std::nullopt, aRowCol[i]);
        });
        return;
    }
    withArray(rWriter, oName, [&]() {
        for (sal_Int32 r = 0; r < aGrid.getLength(); ++r)
        {
            auto aInner = rWriter.startAnonArray();
            for (sal_Int32 c = 0; c < aGrid[r].getLength(); ++c)
                writeNumber(rWriter, std::nullopt, aGrid[r][c]);
        }
    });
}

void writeAnyValue(tools::JsonWriter& rWriter, const cpo::uno::Any& aValue,
                   std::optional<std::string_view> oName)
{
    if (!aValue.hasValue())
    {
        writeNull(rWriter, oName);
        return;
    }

    switch (aValue.getValueTypeClass())
    {
        case cpo::uno::TypeClass_BOOLEAN:
        {
            bool b = false;
            aValue >>= b;
            if (oName)
                rWriter.put(*oName, b);
            else
                rWriter.putRaw(b ? "true" : "false");
            return;
        }
        case cpo::uno::TypeClass_BYTE:
        case cpo::uno::TypeClass_SHORT:
        case cpo::uno::TypeClass_UNSIGNED_SHORT:
        case cpo::uno::TypeClass_LONG:
        case cpo::uno::TypeClass_UNSIGNED_LONG:
        case cpo::uno::TypeClass_HYPER:
        case cpo::uno::TypeClass_UNSIGNED_HYPER:
        case cpo::uno::TypeClass_FLOAT:
        case cpo::uno::TypeClass_DOUBLE:
            writeNumber(rWriter, oName, numericAnyToDouble(aValue));
            return;
        case cpo::uno::TypeClass_STRING:
        {
            OUString s;
            aValue >>= s;
            if (oName)
                rWriter.put(*oName, s);
            else
                rWriter.putSimpleValue(s);
            return;
        }
        case cpo::uno::TypeClass_SEQUENCE:
        {
            // Calc ranges arrive as Sequence<Sequence<Any>> — try grids first.
            // Sequence<Any> is a distinct UNO type and must not gate grids.
            cpo::uno::Sequence<cpo::uno::Sequence<cpo::uno::Any>> aGrid;
            if (aValue >>= aGrid)
            {
                writeAnyGrid(rWriter, aGrid, oName);
                return;
            }
            cpo::uno::Sequence<cpo::uno::Sequence<double>> aDGrid;
            if (aValue >>= aDGrid)
            {
                writeDoubleGrid(rWriter, aDGrid, oName);
                return;
            }
            cpo::uno::Sequence<cpo::uno::Any> aFlat;
            if (aValue >>= aFlat)
            {
                withArray(rWriter, oName, [&]() { writeSequenceElements(rWriter, aFlat); });
                return;
            }
            cpo::uno::Sequence<double> aDRow;
            if (aValue >>= aDRow)
            {
                withArray(rWriter, oName, [&]() {
                    for (sal_Int32 i = 0; i < aDRow.getLength(); ++i)
                        writeNumber(rWriter, std::nullopt, aDRow[i]);
                });
                return;
            }
            cpo::uno::Sequence<OUString> aSRow;
            if (aValue >>= aSRow)
            {
                withArray(rWriter, oName, [&]() {
                    for (sal_Int32 i = 0; i < aSRow.getLength(); ++i)
                        rWriter.putSimpleValue(aSRow[i]);
                });
                return;
            }
            break;
        }
        default:
            break;
    }
    SAL_INFO("scaddins.pythoncompute", "anyToJson: unsupported type -> null");
    writeNull(rWriter, oName);
}

// --- Dumb-JSON hand parser: preserve string vs number vs bool vs null. ---

struct JsonCursor
{
    std::string_view s;
    size_t i = 0;

    void skipWs()
    {
        while (i < s.size() && (s[i] == ' ' || s[i] == '\t' || s[i] == '\n' || s[i] == '\r'))
            ++i;
    }

    bool consume(char c)
    {
        skipWs();
        if (i < s.size() && s[i] == c)
        {
            ++i;
            return true;
        }
        return false;
    }

    bool peek(char c)
    {
        skipWs();
        return i < s.size() && s[i] == c;
    }
};

bool skipJsonValueAt(JsonCursor& cur, int nDepth);

/** Nesting limit for service JSON. Bounds the recursion of skipJsonValueAt and of the
 *  comphelper parser, which only ever sees text already checked here. */
constexpr int kMaxJsonDepth = 64;

bool skipJsonValue(JsonCursor& cur) { return skipJsonValueAt(cur, 0); }

bool parseStringToken(JsonCursor& cur, OUString& rOut)
{
    cur.skipWs();
    if (cur.i >= cur.s.size() || cur.s[cur.i] != '"')
        return false;
    ++cur.i;
    OUStringBuffer buf;
    while (cur.i < cur.s.size())
    {
        const char c = cur.s[cur.i++];
        if (c == '"')
        {
            rOut = buf.makeStringAndClear();
            return true;
        }
        if (c == '\\')
        {
            if (cur.i >= cur.s.size())
                return false;
            const char e = cur.s[cur.i++];
            switch (e)
            {
                case '"':
                case '\\':
                case '/':
                    buf.append(sal_Unicode(e));
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
                    auto readCodeUnit = [&cur](sal_uInt32& rCode) {
                        if (cur.i + 4 > cur.s.size())
                            return false;
                        rCode = 0;
                        for (int n = 0; n < 4; ++n)
                        {
                            const char h = cur.s[cur.i++];
                            rCode <<= 4;
                            if (h >= '0' && h <= '9')
                                rCode |= static_cast<sal_uInt32>(h - '0');
                            else if (h >= 'a' && h <= 'f')
                                rCode |= static_cast<sal_uInt32>(h - 'a' + 10);
                            else if (h >= 'A' && h <= 'F')
                                rCode |= static_cast<sal_uInt32>(h - 'A' + 10);
                            else
                                return false;
                        }
                        return true;
                    };

                    sal_uInt32 code = 0;
                    if (!readCodeUnit(code))
                        return false;
                    if (code >= 0xD800 && code <= 0xDBFF)
                    {
                        if (cur.i + 2 > cur.s.size() || cur.s[cur.i] != '\\'
                            || cur.s[cur.i + 1] != 'u')
                            return false;
                        cur.i += 2;
                        sal_uInt32 low = 0;
                        if (!readCodeUnit(low) || low < 0xDC00 || low > 0xDFFF)
                            return false;
                        code = 0x10000 + ((code - 0xD800) << 10) + (low - 0xDC00);
                    }
                    else if (code >= 0xDC00 && code <= 0xDFFF)
                        return false;
                    buf.appendUtf32(code);
                    break;
                }
                default:
                    return false;
            }
            continue;
        }
        if (static_cast<unsigned char>(c) < 0x20)
            return false;
        if (static_cast<unsigned char>(c) < 0x80)
        {
            buf.append(sal_Unicode(c));
            continue;
        }
        // Multi-byte UTF-8: convert the whole run strictly (rejects overlong,
        // surrogate and >U+10FFFF sequences); the input comes from user code.
        const size_t start = cur.i - 1;
        while (cur.i < cur.s.size() && static_cast<unsigned char>(cur.s[cur.i]) >= 0x80)
            ++cur.i;
        OUString piece;
        if (!rtl_convertStringToUString(&piece.pData, cur.s.data() + start, cur.i - start,
                                        RTL_TEXTENCODING_UTF8,
                                        RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR
                                            | RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR
                                            | RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR))
            return false;
        buf.append(piece);
    }
    return false;
}

bool parseNumberToken(JsonCursor& cur, double& rOut)
{
    cur.skipWs();
    if (cur.i >= cur.s.size())
        return false;
    const size_t start = cur.i;
    if (cur.s[cur.i] == '-')
        ++cur.i;
    if (cur.i >= cur.s.size() || !(cur.s[cur.i] >= '0' && cur.s[cur.i] <= '9'))
        return false;
    if (cur.s[cur.i] == '0')
        ++cur.i;
    else
    {
        while (cur.i < cur.s.size() && cur.s[cur.i] >= '0' && cur.s[cur.i] <= '9')
            ++cur.i;
    }
    if (cur.i < cur.s.size() && cur.s[cur.i] == '.')
    {
        ++cur.i;
        if (cur.i >= cur.s.size() || !(cur.s[cur.i] >= '0' && cur.s[cur.i] <= '9'))
            return false;
        while (cur.i < cur.s.size() && cur.s[cur.i] >= '0' && cur.s[cur.i] <= '9')
            ++cur.i;
    }
    if (cur.i < cur.s.size() && (cur.s[cur.i] == 'e' || cur.s[cur.i] == 'E'))
    {
        ++cur.i;
        if (cur.i < cur.s.size() && (cur.s[cur.i] == '+' || cur.s[cur.i] == '-'))
            ++cur.i;
        if (cur.i >= cur.s.size() || !(cur.s[cur.i] >= '0' && cur.s[cur.i] <= '9'))
            return false;
        while (cur.i < cur.s.size() && cur.s[cur.i] >= '0' && cur.s[cur.i] <= '9')
            ++cur.i;
    }
    const std::string_view tok = cur.s.substr(start, cur.i - start);
    rtl_math_ConversionStatus eStatus = rtl_math_ConversionStatus_Ok;
    sal_Int32 nEnd = 0;
    const OUString sData = OUString::fromUtf8(tok);
    rOut = rtl::math::stringToDouble(sData, '.', ',', &eStatus, &nEnd);
    return eStatus == rtl_math_ConversionStatus_Ok && nEnd == sData.getLength();
}

bool consumeLiteral(JsonCursor& cur, std::string_view lit)
{
    cur.skipWs();
    if (cur.i + lit.size() > cur.s.size())
        return false;
    if (cur.s.substr(cur.i, lit.size()) != lit)
        return false;
    cur.i += lit.size();
    return true;
}

bool skipJsonValueAt(JsonCursor& cur, int nDepth)
{
    if (nDepth > kMaxJsonDepth)
        return false;
    cur.skipWs();
    if (cur.i >= cur.s.size())
        return false;
    const char c = cur.s[cur.i];
    if (c == '"')
    {
        OUString ignored;
        return parseStringToken(cur, ignored);
    }
    if (c == '[')
    {
        ++cur.i;
        cur.skipWs();
        if (cur.consume(']'))
            return true;
        for (;;)
        {
            if (!skipJsonValueAt(cur, nDepth + 1))
                return false;
            cur.skipWs();
            if (cur.consume(']'))
                return true;
            if (!cur.consume(','))
                return false;
        }
    }
    if (c == '{')
    {
        ++cur.i;
        cur.skipWs();
        if (cur.consume('}'))
            return true;
        for (;;)
        {
            OUString key;
            if (!parseStringToken(cur, key))
                return false;
            if (!cur.consume(':'))
                return false;
            if (!skipJsonValueAt(cur, nDepth + 1))
                return false;
            cur.skipWs();
            if (cur.consume('}'))
                return true;
            if (!cur.consume(','))
                return false;
        }
    }
    if (c == 't')
        return consumeLiteral(cur, "true");
    if (c == 'f')
        return consumeLiteral(cur, "false");
    if (c == 'n')
        return consumeLiteral(cur, "null");
    if (c == '-' || (c >= '0' && c <= '9'))
    {
        double ignored = 0;
        return parseNumberToken(cur, ignored);
    }
    return false;
}

bool promoteFlatNumeric(const cpo::uno::Sequence<cpo::uno::Any>& aFlat, cpo::uno::Any& rOut)
{
    if (aFlat.getLength() <= 1)
        return false;
    for (sal_Int32 i = 0; i < aFlat.getLength(); ++i)
    {
        const cpo::uno::TypeClass e = aFlat[i].getValueTypeClass();
        if (e != cpo::uno::TypeClass_DOUBLE && e != cpo::uno::TypeClass_LONG
            && e != cpo::uno::TypeClass_HYPER && e != cpo::uno::TypeClass_FLOAT)
            return false;
    }
    cpo::uno::Sequence<cpo::uno::Sequence<double>> row(1);
    row.getArray()[0].realloc(aFlat.getLength());
    auto* pr = row.getArray()[0].getArray();
    for (sal_Int32 i = 0; i < aFlat.getLength(); ++i)
    {
        pr[i] = numericAnyToDouble(aFlat[i]);
    }
    rOut <<= row;
    return true;
}

bool elemsToAny(std::vector<cpo::uno::Any> elems, cpo::uno::Any& rOut)
{
    // [] has no cell to show: map it to an empty cell like JSON null.
    if (elems.empty())
    {
        rOut <<= OUString();
        return true;
    }

    // Nested numeric rows: recursive convert promotes each inner list to a
    // 1×N double matrix; stack those into one R×C sequence<sequence<double>>.
    {
        bool bAll1xN = true;
        sal_Int32 nCols = -1;
        for (const auto& e : elems)
        {
            cpo::uno::Sequence<cpo::uno::Sequence<double>> rowMat;
            if (!(e >>= rowMat) || rowMat.getLength() != 1)
            {
                bAll1xN = false;
                break;
            }
            const sal_Int32 n = rowMat[0].getLength();
            if (nCols < 0)
                nCols = n;
            else if (n != nCols)
            {
                bAll1xN = false;
                break;
            }
        }
        if (bAll1xN && nCols >= 0)
        {
            cpo::uno::Sequence<cpo::uno::Sequence<double>> grid(
                static_cast<sal_Int32>(elems.size()));
            auto* p = grid.getArray();
            for (size_t i = 0; i < elems.size(); ++i)
            {
                cpo::uno::Sequence<cpo::uno::Sequence<double>> rowMat;
                elems[i] >>= rowMat;
                p[static_cast<sal_Int32>(i)] = rowMat[0];
            }
            rOut <<= grid;
            return true;
        }
    }

    // Nested arrays → sequence<sequence<any>>. Accept bare sequence<any> rows
    // or recursive 1×N matrix wraps. Enforce a rectangular shape because Calc
    // matrices cannot represent ragged JSON arrays.
    {
        bool bAllRows = true;
        sal_Int32 nCols = -1;
        std::vector<cpo::uno::Sequence<cpo::uno::Any>> aRows;
        aRows.reserve(elems.size());
        for (const auto& e : elems)
        {
            cpo::uno::Sequence<cpo::uno::Any> aRow;
            cpo::uno::Sequence<cpo::uno::Sequence<cpo::uno::Any>> aMat;
            if ((e >>= aMat) && aMat.getLength() == 1)
                aRow = aMat[0];
            else
            {
                cpo::uno::Sequence<cpo::uno::Sequence<double>> aDoubleMat;
                if ((e >>= aDoubleMat) && aDoubleMat.getLength() == 1)
                {
                    aRow.realloc(aDoubleMat[0].getLength());
                    for (sal_Int32 i = 0; i < aDoubleMat[0].getLength(); ++i)
                        aRow.getArray()[i] <<= aDoubleMat[0][i];
                }
                else if (!(e >>= aRow))
                {
                    bAllRows = false;
                    break;
                }
            }

            if (nCols < 0)
                nCols = aRow.getLength();
            else if (aRow.getLength() != nCols)
            {
                bAllRows = false;
                break;
            }
            aRows.push_back(std::move(aRow));
        }
        if (bAllRows)
        {
            cpo::uno::Sequence<cpo::uno::Sequence<cpo::uno::Any>> grid(
                static_cast<sal_Int32>(aRows.size()));
            auto* p = grid.getArray();
            for (size_t i = 0; i < aRows.size(); ++i)
                p[static_cast<sal_Int32>(i)] = aRows[i];
            rOut <<= grid;
            return true;
        }
    }

    // A JSON array containing array values reached here only when the nested
    // rows were ragged or otherwise not representable as one Calc matrix.
    for (const auto& e : elems)
    {
        if (e.getValueTypeClass() == cpo::uno::TypeClass_SEQUENCE)
            return false;
    }

    cpo::uno::Sequence<cpo::uno::Any> flat(static_cast<sal_Int32>(elems.size()));
    auto* pf = flat.getArray();
    for (size_t i = 0; i < elems.size(); ++i)
        pf[static_cast<sal_Int32>(i)] = elems[i];

    if (promoteFlatNumeric(flat, rOut))
        return true;
    if (flat.getLength() > 1)
    {
        cpo::uno::Sequence<cpo::uno::Sequence<cpo::uno::Any>> row(1);
        row.getArray()[0] = flat;
        rOut <<= row;
        return true;
    }
    // Exactly one element is left here (empty and >1 returned above).
    {
        // Keep a one-element JSON list as a 1×1 matrix for every scalar type.
        const cpo::uno::TypeClass e = flat[0].getValueTypeClass();
        if (e == cpo::uno::TypeClass_DOUBLE || e == cpo::uno::TypeClass_LONG
            || e == cpo::uno::TypeClass_HYPER || e == cpo::uno::TypeClass_FLOAT)
        {
            cpo::uno::Sequence<cpo::uno::Sequence<double>> cell(1);
            cell.getArray()[0].realloc(1);
            cell.getArray()[0].getArray()[0] = numericAnyToDouble(flat[0]);
            rOut <<= cell;
            return true;
        }
        cpo::uno::Sequence<cpo::uno::Sequence<cpo::uno::Any>> cell(1);
        cell.getArray()[0] = flat;
        rOut <<= cell;
        return true;
    }
}

bool utf16HasUnpairedSurrogate(std::u16string_view s)
{
    for (size_t i = 0; i < s.size(); ++i)
    {
        const sal_Unicode c = s[i];
        if (c >= 0xD800 && c <= 0xDBFF)
        {
            if (i + 1 >= s.size())
                return true;
            const sal_Unicode d = s[i + 1];
            if (d < 0xDC00 || d > 0xDFFF)
                return true;
            ++i;
        }
        else if (c >= 0xDC00 && c <= 0xDFFF)
            return true;
    }
    return false;
}

bool anyHasUnpairedSurrogate(const cpo::uno::Any& a)
{
    OUString s;
    if (a >>= s)
        return utf16HasUnpairedSurrogate(
            std::u16string_view(s.getStr(), static_cast<size_t>(s.getLength())));
    cpo::uno::Sequence<cpo::uno::Any> aSeq;
    if (a >>= aSeq)
    {
        for (sal_Int32 i = 0; i < aSeq.getLength(); ++i)
        {
            if (anyHasUnpairedSurrogate(aSeq[i]))
                return true;
        }
    }
    return false;
}

bool promoteInferred(const cpo::uno::Any& aIn, cpo::uno::Any& rOut)
{
    if (anyHasUnpairedSurrogate(aIn))
        return false;
    cpo::uno::Sequence<cpo::uno::Any> aSeq;
    if (aIn >>= aSeq)
    {
        // Zero-column rows ([[]], [[],[]]) are empty like [] and JSON null.
        if (aSeq.hasElements()
            && std::all_of(aSeq.begin(), aSeq.end(), [](const cpo::uno::Any& e) {
                   cpo::uno::Sequence<cpo::uno::Any> aRow;
                   return (e >>= aRow) && !aRow.hasElements();
               }))
        {
            rOut <<= OUString();
            return true;
        }
        std::vector<cpo::uno::Any> elems;
        elems.reserve(static_cast<size_t>(aSeq.getLength()));
        for (sal_Int32 i = 0; i < aSeq.getLength(); ++i)
        {
            cpo::uno::Any aElem;
            if (!promoteInferred(aSeq[i], aElem))
                return false;
            elems.push_back(std::move(aElem));
        }
        return elemsToAny(std::move(elems), rOut);
    }
    if (!aIn.hasValue())
    {
        rOut <<= OUString();
        return true;
    }
    rOut = aIn;
    return true;
}

struct EnvelopeFields
{
    std::optional<OUString> id;
    std::optional<OUString> status;
    std::optional<OUString> error;
    std::optional<OUString> resultJson;
    bool hasImages = false;
    bool transient = false;
};

bool parseEnvelope(std::string_view jsonUtf8, EnvelopeFields& rOut)
{
    JsonCursor cur{ jsonUtf8, 0 };
    if (!cur.consume('{'))
        return false;
    cur.skipWs();
    if (cur.consume('}'))
        return true;
    for (;;)
    {
        OUString key;
        if (!parseStringToken(cur, key))
            return false;
        if (!cur.consume(':'))
            return false;
        const OString aKeyUtf8 = OUStringToOString(key, RTL_TEXTENCODING_UTF8);
        const std::string_view k(aKeyUtf8);

        if (k == "id" || k == "status" || k == "error")
        {
            cur.skipWs();
            if (cur.peek('"'))
            {
                OUString s;
                if (!parseStringToken(cur, s))
                    return false;
                if (k == "id")
                    rOut.id = s;
                else if (k == "status")
                    rOut.status = s;
                else
                    rOut.error = s;
            }
            else if (!skipJsonValue(cur))
                return false;
        }
        else if (k == "result")
        {
            cur.skipWs();
            const size_t nStart = cur.i;
            if (!skipJsonValue(cur))
                return false;
            rOut.resultJson
                = OUString::fromUtf8(o3tl::trim(cur.s.substr(nStart, cur.i - nStart)));
        }
        else if (k == "transient")
        {
            cur.skipWs();
            if (cur.s.substr(cur.i).starts_with("true"))
            {
                rOut.transient = true;
                cur.i += 4;
            }
            else if (!skipJsonValue(cur))
                return false;
        }
        else if (k == "images")
        {
            if (!skipJsonValue(cur))
                return false;
            rOut.hasImages = true;
        }
        else if (!skipJsonValue(cur))
            return false;

        cur.skipWs();
        if (cur.consume('}'))
        {
            cur.skipWs();
            return cur.i == cur.s.size();
        }
        if (!cur.consume(','))
            return false;
    }
}
} // namespace

OUString co::pythoncompute::anyToJsonFragment(const cpo::uno::Any& aValue)
{
    // JsonWriter always builds an object document. Wrap as {"_": VALUE} then strip.
    tools::JsonWriter aWriter;
    writeAnyValue(aWriter, aValue, "_");
    const std::string aDoc = aWriter.finishAndGetAsStdString();
    const std::string_view s(aDoc);
    const size_t nColon = s.find(':');
    if (nColon == std::string_view::npos || s.size() < 3)
        return u"null"_ustr;
    size_t nStart = nColon + 1;
    while (nStart < s.size() && s[nStart] == ' ')
        ++nStart;
    size_t nEnd = s.size();
    if (nEnd > 0 && s[nEnd - 1] == '}')
        --nEnd;
    while (nEnd > nStart && s[nEnd - 1] == ' ')
        --nEnd;
    return OUString::fromUtf8(s.substr(nStart, nEnd - nStart));
}

std::string co::pythoncompute::buildExecuteRequestJson(
    const OUString& sRequestId, const OUString& sCode,
    const cpo::uno::Sequence<cpo::uno::Any>& aData)
{
    tools::JsonWriter aWriter;
    aWriter.put("id", sRequestId);
    aWriter.put("code", sCode);
    aWriter.put("mode", std::string_view("isolated"));
    if (aData.getLength() == 1)
        writeAnyValue(aWriter, aData[0], "data");
    else if (aData.getLength() > 1)
    {
        auto aArr = aWriter.startArray("data");
        writeSequenceElements(aWriter, aData);
    }
    return aWriter.finishAndGetAsStdString();
}

co::pythoncompute::ResultKind co::pythoncompute::jsonResultToAny(std::string_view jsonUtf8,
                                                                  cpo::uno::Any& rOut,
                                                                  OUString& rError)
{
    rError.clear();
    EnvelopeFields env;
    if (!parseEnvelope(jsonUtf8, env))
    {
        rError = u"Python compute result JSON parse failed"_ustr;
        rOut = makeFormulaErrorAny(FormulaError::NoValue);
        return ResultKind::ServiceError;
    }

    if (env.status && *env.status == u"error")
    {
        if (env.error && !env.error->isEmpty())
            rError = *env.error;
        else
            rError = u"Python compute error"_ustr;
        // Admin-off path from coolwsd — show a clear cell marker (like #BUSY!),
        // not opaque #VALUE!, so operators can see the feature flag without logs.
        const bool bDisabled = rError == u"Python compute is disabled";
        if (bDisabled)
            rOut <<= u"#DISABLED"_ustr;
        else
            rOut = makeFormulaErrorAny(FormulaError::NoValue);
        // coolwsd marks its own errors (transport, timeout, too large, disabled)
        // transient; an error raised by the user's code is deterministic.
        return (env.transient || bDisabled) ? ResultKind::TransientError
                                            : ResultKind::ServiceError;
    }

    if (!env.resultJson)
    {
        rError = u"Python compute response missing result"_ustr;
        rOut = makeFormulaErrorAny(FormulaError::NoValue);
        return ResultKind::ServiceError;
    }

    cpo::uno::Any aResult;
    try
    {
        const cpo::uno::Any aInferred = comphelper::parseJsonToInferredAny(*env.resultJson);
        if (!promoteInferred(aInferred, aResult))
        {
            rError = u"Python compute result JSON parse failed"_ustr;
            rOut = makeFormulaErrorAny(FormulaError::NoValue);
            return ResultKind::ServiceError;
        }
    }
    catch (const cpo::uno::RuntimeException&)
    {
        rError = u"Python compute result JSON parse failed"_ustr;
        rOut = makeFormulaErrorAny(FormulaError::NoValue);
        return ResultKind::ServiceError;
    }
    catch (const std::exception&)
    {
        rError = u"Python compute result JSON parse failed"_ustr;
        rOut = makeFormulaErrorAny(FormulaError::NoValue);
        return ResultKind::ServiceError;
    }

    // images[] only — Classic inserts plots; Online v1: short message
    const bool bResultNull
        = !aResult.hasValue()
          || (aResult.getValueTypeClass() == cpo::uno::TypeClass_STRING && [&]() {
                 OUString s;
                 return (aResult >>= s) && s.isEmpty();
             }());
    // JSON null, [] and [[]] map to empty OUString above; treat that as null result.
    if (env.hasImages && bResultNull)
    {
        rOut <<= u"Image generated (plot insert not supported yet)"_ustr;
        return ResultKind::Ok;
    }

    rOut = aResult;
    return ResultKind::Ok;
}

bool co::pythoncompute::extractRequestIdFromJson(std::string_view jsonUtf8,
                                                              OUString& rId)
{
    EnvelopeFields env;
    if (parseEnvelope(jsonUtf8, env))
    {
        if (!env.id || env.id->isEmpty())
            return false;
        rId = *env.id;
        return true;
    }
    // The body is malformed (e.g. nested too deep). Still find a plain top-level "id" so the
    // caller can finish that request as an error instead of leaving it pending until it
    // times out. Iterative scan: no recursion, so depth costs nothing.
    std::string_view sFound;
    int nDepth = 0;
    bool bExpectKey = false;
    size_t i = 0;
    const size_t n = jsonUtf8.size();
    while (i < n)
    {
        const char c = jsonUtf8[i];
        if (c == '"')
        {
            const size_t nStart = ++i;
            bool bEscaped = false;
            while (i < n && jsonUtf8[i] != '"')
            {
                if (jsonUtf8[i] == '\\')
                {
                    bEscaped = true;
                    ++i;
                }
                ++i;
            }
            if (i >= n)
                return false;
            const std::string_view sTok = jsonUtf8.substr(nStart, i - nStart);
            ++i;
            if (nDepth == 1 && bExpectKey && !bEscaped && sTok == "id")
            {
                while (i < n && (jsonUtf8[i] == ' ' || jsonUtf8[i] == '\t' || jsonUtf8[i] == '\n'
                                 || jsonUtf8[i] == '\r' || jsonUtf8[i] == ':'))
                    ++i;
                if (i >= n || jsonUtf8[i] != '"')
                    return false;
                const size_t nV = ++i;
                while (i < n && jsonUtf8[i] != '"' && jsonUtf8[i] != '\\')
                    ++i;
                if (i >= n || jsonUtf8[i] != '"' || !sFound.empty())
                    return false; // escaped, unterminated or repeated id: give up
                sFound = jsonUtf8.substr(nV, i - nV);
                ++i;
            }
            bExpectKey = false;
            continue;
        }
        ++i;
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
            continue; // whitespace keeps the key expectation
        if (c == '{' || c == '[')
            ++nDepth;
        else if (c == '}' || c == ']')
            --nDepth;
        bExpectKey = (nDepth == 1 && (c == '{' || c == ','));
    }
    if (sFound.empty())
        return false;
    rId = OStringToOUString(sFound, RTL_TEXTENCODING_UTF8);
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
