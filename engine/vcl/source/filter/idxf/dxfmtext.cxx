/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "dxfmtext.hxx"

#include <o3tl/string_view.hxx>
#include <rtl/character.hxx>
#include <rtl/ustrbuf.hxx>

#include <cmath>

namespace
{
// \U+XXXX at nPos, the code point or 0
sal_uInt32 readUnicodeEscape(std::u16string_view aText, size_t nPos)
{
    if (nPos + 7 > aText.size() || aText[nPos] != '\\' || (aText[nPos + 1] != 'U' && aText[nPos + 1] != 'u')
        || aText[nPos + 2] != '+')
        return 0;
    for (size_t i = nPos + 3; i < nPos + 7; ++i)
        if (!rtl::isAsciiHexDigit(aText[i]))
            return 0;
    return o3tl::toUInt32(aText.substr(nPos + 3, 4), 16);
}

// \M+nXXXX at nPos: a double byte character of an East Asian code page
OUString readMultiByteEscape(std::u16string_view aText, size_t nPos)
{
    if (nPos + 8 > aText.size() || aText[nPos] != '\\' || (aText[nPos + 1] != 'M' && aText[nPos + 1] != 'm')
        || aText[nPos + 2] != '+')
        return OUString();
    for (size_t i = nPos + 4; i < nPos + 8; ++i)
        if (!rtl::isAsciiHexDigit(aText[i]))
            return OUString();
    rtl_TextEncoding eEncoding;
    switch (aText[nPos + 3])
    {
        case '1':
            eEncoding = RTL_TEXTENCODING_MS_932;
            break;
        case '2':
            eEncoding = RTL_TEXTENCODING_MS_950;
            break;
        case '3':
            eEncoding = RTL_TEXTENCODING_MS_949;
            break;
        case '4':
            eEncoding = RTL_TEXTENCODING_MS_1361;
            break;
        case '5':
            eEncoding = RTL_TEXTENCODING_MS_936;
            break;
        default:
            return OUString();
    }
    const sal_uInt32 nCode = o3tl::toUInt32(aText.substr(nPos + 4, 4), 16);
    const char aBytes[2] = { static_cast<char>(nCode >> 8), static_cast<char>(nCode & 0xff) };
    return OUString(aBytes, 2, eEncoding);
}

// The parameter of a formatting code up to the next ';', nPos is moved after it
std::u16string_view readParameter(std::u16string_view aText, size_t& nPos)
{
    const size_t nStart = nPos;
    size_t nEnd = aText.find(';', nStart);
    if (nEnd == std::u16string_view::npos)
        nEnd = aText.size();
    nPos = std::min(nEnd + 1, aText.size());
    return aText.substr(nStart, nEnd - nStart);
}

// An absolute value, or one relative to fCurrent with an x suffix
double readScaledValue(std::u16string_view aParameter, double fCurrent)
{
    if (aParameter.empty())
        return fCurrent;
    const bool bRelative = aParameter.back() == 'x' || aParameter.back() == 'X';
    if (bRelative)
        aParameter.remove_suffix(1);
    const double fValue = o3tl::toDouble(aParameter);
    const double fResult = bRelative ? fCurrent * fValue : fValue;
    if (!(fValue > 0) || !std::isfinite(fResult))
        return fCurrent;
    return fResult;
}

// The family name of a font given by name or by file name
OUString fontNameFromSpec(std::u16string_view aSpec)
{
    const size_t nDot = aSpec.rfind('.');
    if (nDot == std::u16string_view::npos)
        return OUString(aSpec);
    // a file name; the SHX fonts are not available
    if (o3tl::equalsIgnoreAsciiCase(aSpec.substr(nDot), u".shx"))
        return OUString();
    return OUString(aSpec.substr(0, nDot));
}

class MTextBuilder
{
public:
    MTextBuilder(const DXFTextFormat& rBase, rtl_TextEncoding eEncoding)
        : maParagraphs(1)
        , maFormat(rBase)
        , meEncoding(eEncoding)
    {
    }

    DXFTextFormat& format()
    {
        flush();
        return maFormat;
    }

    void append(sal_Unicode c) { maText.append(c); }
    void append(std::u16string_view aText) { maText.append(aText); }
    void appendCodePoint(sal_uInt32 nCode) { maText.appendUtf32(nCode); }

    void push()
    {
        flush();
        maStack.push_back(maFormat);
    }

    void pop()
    {
        flush();
        if (!maStack.empty())
        {
            maFormat = maStack.back();
            maStack.pop_back();
        }
    }

    void newParagraph()
    {
        flush();
        maParagraphs.emplace_back();
    }

    std::vector<DXFTextParagraph> finish()
    {
        flush();
        return std::move(maParagraphs);
    }

private:
    void flush()
    {
        if (maText.isEmpty())
            return;
        DXFTextRun aRun;
        aRun.maText = dxf::decodeSpecialCharacters(maText, meEncoding, false);
        maText.setLength(0);
        aRun.maFormat = maFormat;
        if (!aRun.maText.isEmpty())
            maParagraphs.back().maRuns.push_back(aRun);
    }

    std::vector<DXFTextParagraph> maParagraphs;
    std::vector<DXFTextFormat> maStack;
    DXFTextFormat maFormat;
    OUStringBuffer maText;
    rtl_TextEncoding meEncoding;
};
}

namespace dxf
{
std::vector<DXFTextParagraph> parseMText(std::u16string_view aText, const DXFTextFormat& rBase,
                                         rtl_TextEncoding eEncoding)
{
    MTextBuilder aBuilder(rBase, eEncoding);
    size_t i = 0;
    while (i < aText.size())
    {
        const sal_Unicode c = aText[i];
        if (c == '{')
        {
            aBuilder.push();
            ++i;
            continue;
        }
        if (c == '}')
        {
            aBuilder.pop();
            ++i;
            continue;
        }
        if (c == '^' && i + 1 < aText.size())
        {
            // caret notation of control characters
            const sal_Unicode cNext = aText[i + 1];
            // a line feed is a new paragraph, a carriage return before it adds nothing
            if (cNext == 'J')
                aBuilder.newParagraph();
            else if (cNext == 'I')
                aBuilder.append(' ');
            else if (cNext == ' ')
                aBuilder.append('^');
            else if (cNext != 'M')
            {
                aBuilder.append(c);
                aBuilder.append(cNext);
            }
            i += 2;
            continue;
        }
        if (c != '\\' || i + 1 >= aText.size())
        {
            aBuilder.append(c);
            ++i;
            continue;
        }

        const sal_Unicode cCode = aText[i + 1];
        switch (cCode)
        {
            case 'P':
            case 'N':
            case 'X':
                aBuilder.newParagraph();
                i += 2;
                break;
            case '~':
                aBuilder.append(u'\u00a0');
                i += 2;
                break;
            case '\\':
            case '{':
            case '}':
                aBuilder.append(cCode);
                i += 2;
                break;
            case 'L':
            case 'l':
                aBuilder.format().mbUnderline = cCode == 'L';
                i += 2;
                break;
            case 'O':
            case 'o':
                aBuilder.format().mbOverline = cCode == 'O';
                i += 2;
                break;
            case 'K':
            case 'k':
                aBuilder.format().mbStrikeout = cCode == 'K';
                i += 2;
                break;
            case 'H':
            {
                i += 2;
                DXFTextFormat& rFormat = aBuilder.format();
                rFormat.mfHeight = readScaledValue(readParameter(aText, i), rFormat.mfHeight);
                break;
            }
            case 'W':
            {
                i += 2;
                DXFTextFormat& rFormat = aBuilder.format();
                rFormat.mfWidthFactor
                    = readScaledValue(readParameter(aText, i), rFormat.mfWidthFactor);
                break;
            }
            case 'Q':
                i += 2;
                aBuilder.format().mfOblique = o3tl::toDouble(readParameter(aText, i));
                break;
            case 'C':
                i += 2;
                aBuilder.format().mnColorIndex = o3tl::toInt32(readParameter(aText, i));
                aBuilder.format().mnTrueColor = -1;
                break;
            case 'c':
            {
                // the red component is in the low byte
                i += 2;
                const sal_uInt32 nColor = o3tl::toUInt32(readParameter(aText, i));
                aBuilder.format().mnTrueColor
                    = ((nColor & 0xff) << 16) | (nColor & 0xff00) | ((nColor >> 16) & 0xff);
                break;
            }
            case 'f':
            case 'F':
            {
                i += 2;
                std::u16string_view aSpec = readParameter(aText, i);
                DXFTextFormat& rFormat = aBuilder.format();
                sal_Int32 nIndex = 0;
                // \F gives an SHX font, which is not available
                const std::u16string_view aName = o3tl::getToken(aSpec, 0, '|', nIndex);
                rFormat.maFontName = cCode == 'F' ? OUString() : fontNameFromSpec(aName);
                rFormat.mbBold = false;
                rFormat.mbItalic = false;
                while (nIndex >= 0)
                {
                    std::u16string_view aOption = o3tl::getToken(aSpec, 0, '|', nIndex);
                    if (aOption == u"b1")
                        rFormat.mbBold = true;
                    else if (aOption == u"i1")
                        rFormat.mbItalic = true;
                }
                break;
            }
            case 'S':
            {
                // stacked text, drawn in line
                i += 2;
                OUStringBuffer aStack;
                while (i < aText.size() && aText[i] != ';')
                {
                    if (aText[i] == '\\' && i + 1 < aText.size())
                        ++i;
                    aStack.append(aText[i]);
                    ++i;
                }
                ++i;
                OUString aStacked = aStack.makeStringAndClear();
                const sal_Int32 nTolerance = aStacked.indexOf('^');
                if (nTolerance >= 0)
                    aStacked = OUString::Concat(o3tl::trim(aStacked.subView(0, nTolerance))) + " "
                               + o3tl::trim(aStacked.subView(nTolerance + 1));
                aBuilder.append(o3tl::trim(aStacked.replace('#', '/')));
                break;
            }
            case 'A':
            case 'T':
            case 'p':
                // alignment, tracking and paragraph properties
                i += 2;
                readParameter(aText, i);
                break;
            case 'U':
            case 'u':
            {
                const sal_uInt32 nCode = readUnicodeEscape(aText, i);
                if (nCode != 0 && rtl::isUnicodeCodePoint(nCode))
                {
                    aBuilder.appendCodePoint(nCode);
                    i += 7;
                }
                else
                {
                    aBuilder.append(c);
                    ++i;
                }
                break;
            }
            case 'M':
            case 'm':
            {
                const OUString aCharacter = readMultiByteEscape(aText, i);
                if (!aCharacter.isEmpty())
                {
                    aBuilder.append(aCharacter);
                    i += 8;
                }
                else
                {
                    aBuilder.append(c);
                    ++i;
                }
                break;
            }
            default:
                aBuilder.append(c);
                ++i;
                break;
        }
    }
    return aBuilder.finish();
}

OUString decodeSpecialCharacters(std::u16string_view aText, rtl_TextEncoding eEncoding,
                                 bool bEscapes)
{
    OUStringBuffer aResult(static_cast<sal_Int32>(aText.size()));
    size_t i = 0;
    while (i < aText.size())
    {
        const sal_Unicode c = aText[i];
        if (c == '%' && i + 2 < aText.size() && aText[i + 1] == '%')
        {
            const sal_Unicode cCode = rtl::toAsciiLowerCase(aText[i + 2]);
            if (cCode == 'c' || cCode == 'd' || cCode == 'p' || cCode == '%')
            {
                aResult.append(cCode == 'c'   ? u'\u2205'
                               : cCode == 'd' ? u'\u00b0'
                               : cCode == 'p' ? u'\u00b1'
                                              : u'%');
                i += 3;
                continue;
            }
            if (cCode == 'u' || cCode == 'o' || cCode == 'k')
            {
                i += 3;
                continue;
            }
            if (rtl::isAsciiDigit(cCode))
            {
                // %%nnn, a character code of the drawing
                size_t nEnd = i + 2;
                while (nEnd < aText.size() && nEnd < i + 5 && rtl::isAsciiDigit(aText[nEnd]))
                    ++nEnd;
                const sal_uInt32 nCode = o3tl::toUInt32(aText.substr(i + 2, nEnd - i - 2));
                if (nCode > 0 && nCode < 128)
                    aResult.append(static_cast<sal_Unicode>(nCode));
                else if (nCode >= 128 && nCode < 256)
                {
                    const char cByte = static_cast<char>(nCode);
                    aResult.append(OUString(&cByte, 1, eEncoding));
                }
                i = nEnd;
                continue;
            }
        }
        if (bEscapes && c == '\\')
        {
            const sal_uInt32 nCode = readUnicodeEscape(aText, i);
            if (nCode != 0 && rtl::isUnicodeCodePoint(nCode))
            {
                aResult.appendUtf32(nCode);
                i += 7;
                continue;
            }
            const OUString aCharacter = readMultiByteEscape(aText, i);
            if (!aCharacter.isEmpty())
            {
                aResult.append(aCharacter);
                i += 8;
                continue;
            }
        }
        aResult.append(c);
        ++i;
    }
    return aResult.makeStringAndClear();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
