/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <filter/msfilter/rtfutil.hxx>
#include <rtl/strbuf.hxx>
#include <svtools/rtfkeywd.hxx>

namespace msfilter {
namespace rtfutil {

OString OutHex(sal_uLong nHex, sal_uInt8 nLen)
{
    sal_Char aNToABuf[] = "0000000000000000";

    OSL_ENSURE( nLen < sizeof(aNToABuf), "nLen is too big" );
    if( nLen >= sizeof(aNToABuf) )
        nLen = (sizeof(aNToABuf)-1);

    // Set pointer to the buffer end
    sal_Char* pStr = aNToABuf + (sizeof(aNToABuf)-1);
    for( sal_uInt8 n = 0; n < nLen; ++n )
    {
        *(--pStr) = (sal_Char)(nHex & 0xf ) + 48;
        if( *pStr > '9' )
            *pStr += 39;
        nHex >>= 4;
    }
    return OString(pStr);
}

OString OutChar(sal_Unicode c, int *pUCMode, rtl_TextEncoding eDestEnc, bool* pSuccess, bool bUnicode)
{
    if (pSuccess)
        *pSuccess = true;
    OStringBuffer aBuf;
    const sal_Char* pStr = 0;
    // 0x0b instead of \n, etc because of the replacements in SwWW8AttrIter::GetSnippet()
    switch (c)
    {
        case 0x0b:
            // hard line break
            pStr = OOO_STRING_SVTOOLS_RTF_LINE;
            break;
        case '\t':
            pStr = OOO_STRING_SVTOOLS_RTF_TAB;
            break;
        case '\\':
        case '}':
        case '{':
            aBuf.append('\\');
            aBuf.append((sal_Char)c);
            break;
        case 0xa0:
            // non-breaking space
            pStr = "\\~";
            break;
        case 0x1e:
            // non-breaking hyphen
            pStr = "\\_";
            break;
        case 0x1f:
            // optional hyphen
            pStr = "\\-";
            break;
        default:
            if (c >= ' ' && c <= '~')
                aBuf.append((sal_Char)c);
            else {
                OUString sBuf(&c, 1);
                OString sConverted;
                if (pSuccess)
                    *pSuccess &= sBuf.convertToString(&sConverted, eDestEnc, RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR | RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR);
                else
                    sBuf.convertToString(&sConverted, eDestEnc, OUSTRING_TO_OSTRING_CVTFLAGS);
                const sal_Int32 nLen = sConverted.getLength();

                if (pUCMode && bUnicode)
                {
                    if (*pUCMode != nLen)
                    {
                        aBuf.append("\\uc");
                        aBuf.append((sal_Int32)nLen);
                        // #i47831# add an additional whitespace, so that "document whitespaces" are not ignored.
                        aBuf.append(' ');
                        *pUCMode = nLen;
                    }
                    aBuf.append("\\u");
                    aBuf.append((sal_Int32)c);
                }

                for (sal_Int32 nI = 0; nI < nLen; ++nI)
                {
                    aBuf.append("\\'");
                    aBuf.append(OutHex(sConverted.getStr()[nI], 2));
                }
            }
    }
    if (pStr) {
        aBuf.append(pStr);
        switch (c)
        {
            case 0xa0:
            case 0x1e:
            case 0x1f:
                break;
            default:
                aBuf.append(' ');
        }
    }
    return aBuf.makeStringAndClear();
}

OString OutString(const OUString &rStr, rtl_TextEncoding eDestEnc, bool bUnicode)
{
    SAL_INFO("filter.ms", OSL_THIS_FUNC << ", rStr = '" << OUString(rStr) << "'");
    OStringBuffer aBuf;
    int nUCMode = 1;
    for (sal_Int32 n = 0; n < rStr.getLength(); ++n)
        aBuf.append(OutChar(rStr[n], &nUCMode, eDestEnc, 0, bUnicode));
    if (nUCMode != 1) {
        aBuf.append(OOO_STRING_SVTOOLS_RTF_UC);
        aBuf.append((sal_Int32)1);
        aBuf.append(" "); // #i47831# add an additional whitespace, so that "document whitespaces" are not ignored.;
    }
    return aBuf.makeStringAndClear();
}

/// Checks if lossless conversion of the string to eDestEnc is possible or not.
static bool TryOutString(const String &rStr, rtl_TextEncoding eDestEnc)
{
    int nUCMode = 1;
    for (xub_StrLen n = 0; n < rStr.Len(); ++n)
    {
        bool bRet;
        OutChar(rStr.GetChar(n), &nUCMode, eDestEnc, &bRet);
        if (!bRet)
            return false;
    }
    return true;
}

OString OutStringUpr(const sal_Char *pToken, const OUString &rStr, rtl_TextEncoding eDestEnc)
{
    if (TryOutString(rStr, eDestEnc))
        return OString("{") + pToken + " " + OutString(rStr, eDestEnc) + "}";

    OStringBuffer aRet;
    aRet.append("{" OOO_STRING_SVTOOLS_RTF_UPR "{");
    aRet.append(pToken);
    aRet.append(" ");
    aRet.append(OutString(rStr, eDestEnc, /*bUnicode =*/ false));
    aRet.append("}{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_UD "{");
    aRet.append(pToken);
    aRet.append(" ");
    aRet.append(OutString(rStr, eDestEnc));
    aRet.append("}}}");
    return aRet.makeStringAndClear();
}
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
