/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "sal/config.h"

#include "rtl/textcvt.h"

#include "handleundefinedunicodetotextchar.hxx"
#include "tenchelp.hxx"
#include "unichars.hxx"

/* ======================================================================= */

/* DBCS to Unicode conversion routine use a lead table for the first byte, */
/* where we determine the trail table or for single byte chars the unicode */
/* value. We have for all lead byte a separate table, because we can */
/* then share many tables for different charset encodings. */

/* ======================================================================= */

sal_Size ImplDBCSToUnicode( const void* pData, SAL_UNUSED_PARAMETER void*,
                            const char* pSrcBuf, sal_Size nSrcBytes,
                            sal_Unicode* pDestBuf, sal_Size nDestChars,
                            sal_uInt32 nFlags, sal_uInt32* pInfo,
                            sal_Size* pSrcCvtBytes )
{
    unsigned char                   cTrail;
    sal_Unicode                 cConv;
    const ImplDBCSToUniLeadTab* pLeadEntry;
    const ImplDBCSConvertData*  pConvertData = static_cast<const ImplDBCSConvertData*>(pData);
    const ImplDBCSToUniLeadTab* pLeadTab = pConvertData->mpToUniLeadTab;
    sal_Unicode*                pEndDestBuf;
    const char*             pEndSrcBuf;

    *pInfo = 0;
    pEndDestBuf = pDestBuf+nDestChars;
    pEndSrcBuf  = pSrcBuf+nSrcBytes;
    while ( pSrcBuf < pEndSrcBuf )
    {
        unsigned char cLead = (unsigned char)*pSrcBuf;

        /* get entry for the lead byte */
        pLeadEntry = pLeadTab+cLead;

        /* SingleByte char? */
        if (pLeadEntry->mpToUniTrailTab == nullptr
            || cLead < pConvertData->mnLeadStart
            || cLead > pConvertData->mnLeadEnd)
        {
            cConv = pLeadEntry->mnUniChar;
            if ( !cConv && (cLead != 0) )
            {
                *pInfo |= RTL_TEXTTOUNICODE_INFO_UNDEFINED;
                if ( (nFlags & RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_MASK) == RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR )
                {
                    *pInfo |= RTL_TEXTTOUNICODE_INFO_ERROR;
                    break;
                }
                else if ( (nFlags & RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_MASK) == RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_IGNORE )
                {
                    pSrcBuf++;
                    continue;
                }
                else
                    cConv = ImplGetUndefinedUnicodeChar(cLead, nFlags);
            }
        }
        else
        {
            /* Source buffer to small */
            if ( pSrcBuf +1 == pEndSrcBuf )
            {
                if ( (nFlags & RTL_TEXTTOUNICODE_FLAGS_FLUSH) == 0 )
                {
                    *pInfo |= RTL_TEXTTOUNICODE_INFO_ERROR | RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOSMALL;
                    break;
                }
                cConv = 0;
            }
            else
            {
                pSrcBuf++;
                cTrail = (unsigned char)*pSrcBuf;
                if ( (cTrail >= pLeadEntry->mnTrailStart) && (cTrail <= pLeadEntry->mnTrailEnd) )
                    cConv = pLeadEntry->mpToUniTrailTab[cTrail-pLeadEntry->mnTrailStart];
                else
                    cConv = 0;

                if ( !cConv )
                {
                    /* EUDC Ranges */
                    sal_uInt16              i;
                    const ImplDBCSEUDCData* pEUDCTab = pConvertData->mpEUDCTab;
                    for ( i = 0; i < pConvertData->mnEUDCCount; i++ )
                    {
                        if ( (cLead >= pEUDCTab->mnLeadStart) &&
                             (cLead <= pEUDCTab->mnLeadEnd) )
                        {
                            if ( (cTrail >= pEUDCTab->mnTrail1Start) &&
                                 (cTrail <= pEUDCTab->mnTrail1End) )
                            {
                                cConv = pEUDCTab->mnUniStart+
                                    ((cLead-pEUDCTab->mnLeadStart)*pEUDCTab->mnTrailRangeCount)+
                                    (cTrail-pEUDCTab->mnTrail1Start);
                                break;
                            }
                            else
                            {
                                sal_uInt16 nTrailCount = pEUDCTab->mnTrail1End-pEUDCTab->mnTrail1Start+1;
                                if ( (pEUDCTab->mnTrailCount >= 2) &&
                                     (cTrail >= pEUDCTab->mnTrail2Start) &&
                                     (cTrail <= pEUDCTab->mnTrail2End) )
                                {
                                    cConv = pEUDCTab->mnUniStart+
                                        ((cLead-pEUDCTab->mnLeadStart)*pEUDCTab->mnTrailRangeCount)+
                                        nTrailCount+
                                        (cTrail-pEUDCTab->mnTrail2Start);
                                    break;
                                }
                                else
                                {
                                    nTrailCount = pEUDCTab->mnTrail2End-pEUDCTab->mnTrail2Start+1;
                                    if ( (pEUDCTab->mnTrailCount >= 3) &&
                                         (cTrail >= pEUDCTab->mnTrail3Start) &&
                                         (cTrail <= pEUDCTab->mnTrail3End) )
                                    {
                                        cConv = pEUDCTab->mnUniStart+
                                            ((cLead-pEUDCTab->mnLeadStart)*pEUDCTab->mnTrailRangeCount)+
                                            nTrailCount+
                                            (cTrail-pEUDCTab->mnTrail3Start);
                                        break;
                                    }
                                }
                            }
                        }

                        pEUDCTab++;
                    }

                    if ( !cConv )
                    {
                        /* We compare the full range of the trail we defined, */
                        /* which can often be greater than the limit. We do this */
                        /* so that extensions that don't consider encodings */
                        /* correctly treat double-byte characters as a single */
                        /* character as much as possible. */

                        if (cLead < pConvertData->mnLeadStart
                            || cLead > pConvertData->mnLeadEnd
                            || cTrail < pConvertData->mnTrailStart
                            || cTrail > pConvertData->mnTrailEnd)
                        {
                            *pInfo |= RTL_TEXTTOUNICODE_INFO_INVALID;
                            if ( (nFlags & RTL_TEXTTOUNICODE_FLAGS_INVALID_MASK) == RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR )
                            {
                                *pInfo |= RTL_TEXTTOUNICODE_INFO_ERROR;
                                break;
                            }
                            else if ( (nFlags & RTL_TEXTTOUNICODE_FLAGS_INVALID_MASK) == RTL_TEXTTOUNICODE_FLAGS_INVALID_IGNORE )
                            {
                                pSrcBuf++;
                                continue;
                            }
                            else
                                cConv = RTL_TEXTENC_UNICODE_REPLACEMENT_CHARACTER;
                        }
                    }
                }
            }
            if ( !cConv )
            {
                *pInfo |= RTL_TEXTTOUNICODE_INFO_MBUNDEFINED;
                if ( (nFlags & RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_MASK) == RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR )
                {
                    *pInfo |= RTL_TEXTTOUNICODE_INFO_ERROR;
                    break;
                }
                else if ( (nFlags & RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_MASK) == RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_IGNORE )
                {
                    pSrcBuf++;
                    continue;
                }
                else
                    cConv = RTL_TEXTENC_UNICODE_REPLACEMENT_CHARACTER;
            }
        }

        if ( pDestBuf == pEndDestBuf )
        {
            *pInfo |= RTL_TEXTTOUNICODE_INFO_ERROR | RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOSMALL;
            break;
        }

        *pDestBuf = cConv;
        pDestBuf++;
        pSrcBuf++;
    }

    *pSrcCvtBytes = nSrcBytes - (pEndSrcBuf-pSrcBuf);
    return (nDestChars - (pEndDestBuf-pDestBuf));
}

/* ----------------------------------------------------------------------- */

sal_Size ImplUnicodeToDBCS( const void* pData, SAL_UNUSED_PARAMETER void*,
                            const sal_Unicode* pSrcBuf, sal_Size nSrcChars,
                            char* pDestBuf, sal_Size nDestBytes,
                            sal_uInt32 nFlags, sal_uInt32* pInfo,
                            sal_Size* pSrcCvtChars )
{
    sal_uInt16                  cConv;
    sal_Unicode                 c;
    const ImplUniToDBCSHighTab* pHighEntry;
    const ImplDBCSConvertData*  pConvertData = static_cast<const ImplDBCSConvertData*>(pData);
    const ImplUniToDBCSHighTab* pHighTab = pConvertData->mpToDBCSHighTab;
    char*                   pEndDestBuf;
    const sal_Unicode*          pEndSrcBuf;

    bool bCheckRange =
        pConvertData->mnLeadStart != 0 || pConvertData->mnLeadEnd != 0xFF;
        /* this statement has the effect that this extra check is only done for
           EUC-KR, which uses the MS-949 tables, but does not support the full
           range of MS-949 */

    *pInfo = 0;
    pEndDestBuf = pDestBuf+nDestBytes;
    pEndSrcBuf  = pSrcBuf+nSrcChars;
    while ( pSrcBuf < pEndSrcBuf )
    {
        c = *pSrcBuf;
        unsigned char nHighChar = (unsigned char)((c >> 8) & 0xFF);
        unsigned char nLowChar = (unsigned char)(c & 0xFF);

        /* get entry for the high byte */
        pHighEntry = pHighTab+nHighChar;

        /* is low byte in the table range */
        if ( (nLowChar >= pHighEntry->mnLowStart) && (nLowChar <= pHighEntry->mnLowEnd) )
        {
            cConv = pHighEntry->mpToUniTrailTab[nLowChar-pHighEntry->mnLowStart];
            if (bCheckRange && cConv > 0x7F
                && ((cConv >> 8) < pConvertData->mnLeadStart
                    || (cConv >> 8) > pConvertData->mnLeadEnd
                    || (cConv & 0xFF) < pConvertData->mnTrailStart
                    || (cConv & 0xFF) > pConvertData->mnTrailEnd))
                cConv = 0;
        }
        else
            cConv = 0;

        if (cConv == 0 && c != 0)
        {
            /* Map to EUDC ranges: */
            ImplDBCSEUDCData const * pEUDCTab = pConvertData->mpEUDCTab;
            sal_uInt32 i;
            for (i = 0; i < pConvertData->mnEUDCCount; ++i)
            {
                if (c >= pEUDCTab->mnUniStart && c <= pEUDCTab->mnUniEnd)
                {
                    sal_uInt32 nIndex = c - pEUDCTab->mnUniStart;
                    sal_uInt32 nLeadOff
                        = nIndex / pEUDCTab->mnTrailRangeCount;
                    sal_uInt32 nTrailOff
                        = nIndex % pEUDCTab->mnTrailRangeCount;
                    sal_uInt32 nSize;
                    cConv = (sal_uInt16)
                                ((pEUDCTab->mnLeadStart + nLeadOff) << 8);
                    nSize
                        = pEUDCTab->mnTrail1End - pEUDCTab->mnTrail1Start + 1;
                    if (nTrailOff < nSize)
                    {
                        cConv |= pEUDCTab->mnTrail1Start + nTrailOff;
                        break;
                    }
                    nTrailOff -= nSize;
                    nSize
                        = pEUDCTab->mnTrail2End - pEUDCTab->mnTrail2Start + 1;
                    if (nTrailOff < nSize)
                    {
                        cConv |= pEUDCTab->mnTrail2Start + nTrailOff;
                        break;
                    }
                    nTrailOff -= nSize;
                    cConv |= pEUDCTab->mnTrail3Start + nTrailOff;
                    break;
                }
                pEUDCTab++;
            }

            /* FIXME
             * SB: Not sure why this is in here.  Plus, it does not work as
             * intended when (c & 0xFF) == 0, because the next !cConv check
             * will then think c has not yet been converted...
             */
            if (c >= RTL_TEXTCVT_BYTE_PRIVATE_START
                && c <= RTL_TEXTCVT_BYTE_PRIVATE_END)
            {
                if ( nFlags & RTL_UNICODETOTEXT_FLAGS_PRIVATE_MAPTO0 )
                    cConv = static_cast< char >(static_cast< unsigned char >(c & 0xFF));
            }
        }

        if ( !cConv )
        {
            if ( nFlags & RTL_UNICODETOTEXT_FLAGS_UNDEFINED_REPLACE )
            {
                /* !!! */
            }

            if ( nFlags & RTL_UNICODETOTEXT_FLAGS_UNDEFINED_REPLACESTR )
            {
                /* !!! */
            }

            /* Handle undefined and surrogates characters */
            /* (all surrogates characters are undefined) */
            if (sal::detail::textenc::handleUndefinedUnicodeToTextChar(
                    &pSrcBuf, pEndSrcBuf, &pDestBuf, pEndDestBuf, nFlags,
                    pInfo))
                continue;
            else
                break;
        }

        /* SingleByte */
        if ( !(cConv & 0xFF00) )
        {
            if ( pDestBuf == pEndDestBuf )
            {
                *pInfo |= RTL_UNICODETOTEXT_INFO_ERROR | RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
                break;
            }

            *pDestBuf = static_cast< char >(static_cast< unsigned char >(cConv & 0xFF));
            pDestBuf++;
        }
        else
        {
            if ( pDestBuf+1 >= pEndDestBuf )
            {
                *pInfo |= RTL_UNICODETOTEXT_INFO_ERROR | RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
                break;
            }

            *pDestBuf = static_cast< char >(static_cast< unsigned char >((cConv >> 8) & 0xFF));
            pDestBuf++;
            *pDestBuf = static_cast< char >(static_cast< unsigned char >(cConv & 0xFF));
            pDestBuf++;
        }

        pSrcBuf++;
    }

    *pSrcCvtChars = nSrcChars - (pEndSrcBuf-pSrcBuf);
    return (nDestBytes - (pEndDestBuf-pDestBuf));
}

/* ======================================================================= */

#define JIS_EUC_LEAD_OFF                                        0x80
#define JIS_EUC_TRAIL_OFF                                       0x80

/* ----------------------------------------------------------------------- */

sal_Size ImplEUCJPToUnicode( const void* pData,
                             SAL_UNUSED_PARAMETER void*,
                             const char* pSrcBuf, sal_Size nSrcBytes,
                             sal_Unicode* pDestBuf, sal_Size nDestChars,
                             sal_uInt32 nFlags, sal_uInt32* pInfo,
                             sal_Size* pSrcCvtBytes )
{
    unsigned char                   cLead = '\0';
    unsigned char                   cTrail = '\0';
    sal_Unicode                 cConv;
    const ImplDBCSToUniLeadTab* pLeadEntry;
    const ImplDBCSToUniLeadTab* pLeadTab;
    const ImplEUCJPConvertData* pConvertData = static_cast<const ImplEUCJPConvertData*>(pData);
    sal_Unicode*                pEndDestBuf;
    const char*             pEndSrcBuf;

    *pInfo = 0;
    pEndDestBuf = pDestBuf+nDestChars;
    pEndSrcBuf  = pSrcBuf+nSrcBytes;
    while ( pSrcBuf < pEndSrcBuf )
    {
        unsigned char c = (unsigned char)*pSrcBuf;

        /* ASCII */
        if ( c <= 0x7F )
            cConv = c;
        else
        {
            /* SS2 - Half-width katakana */
            /* 8E + A1-DF */
            if ( c == 0x8E )
            {
                /* Source buffer to small */
                if ( pSrcBuf + 1 == pEndSrcBuf )
                {
                    *pInfo |= RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOSMALL;
                    break;
                }

                pSrcBuf++;
                c = (unsigned char)*pSrcBuf;
                if ( (c >= 0xA1) && (c <= 0xDF) )
                    cConv = 0xFF61+(c-0xA1);
                else
                {
                    cConv = 0;
                    cLead = 0x8E;
                    cTrail = c;
                }
            }
            else
            {
                /* SS3 - JIS 0212-1990 */
                /* 8F + A1-FE + A1-FE */
                if ( c == 0x8F )
                {
                    /* Source buffer to small */
                    if (pEndSrcBuf - pSrcBuf < 3)
                    {
                        *pInfo |= RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOSMALL;
                        break;
                    }

                    pSrcBuf++;
                    cLead = (unsigned char)*pSrcBuf;
                    pSrcBuf++;
                    cTrail = (unsigned char)*pSrcBuf;
                    pLeadTab = pConvertData->mpJIS0212ToUniLeadTab;
                }
                /* CodeSet 2 JIS 0208-1997 */
                /* A1-FE + A1-FE */
                else
                {
                    /* Source buffer to small */
                    if ( pSrcBuf + 1 == pEndSrcBuf )
                    {
                        *pInfo |= RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOSMALL;
                        break;
                    }

                    cLead = c;
                    pSrcBuf++;
                    cTrail = (unsigned char)*pSrcBuf;
                    pLeadTab = pConvertData->mpJIS0208ToUniLeadTab;
                }

                /* Undefined Range */
                if ( (cLead < JIS_EUC_LEAD_OFF) || (cTrail < JIS_EUC_TRAIL_OFF) )
                    cConv = 0;
                else
                {
                    cLead   -= JIS_EUC_LEAD_OFF;
                    cTrail  -= JIS_EUC_TRAIL_OFF;
                    pLeadEntry = pLeadTab+cLead;
                    if ( (cTrail >= pLeadEntry->mnTrailStart) && (cTrail <= pLeadEntry->mnTrailEnd) )
                        cConv = pLeadEntry->mpToUniTrailTab[cTrail-pLeadEntry->mnTrailStart];
                    else
                        cConv = 0;
                }
            }

            if ( !cConv )
            {
                /* We compare the full range of the trail we defined, */
                /* which can often be greater than the limit. We do this */
                /* so that extensions that don't consider encodings */
                /* correctly treat double-byte characters as a single */
                /* character as much as possible. */

                if ( (cLead < JIS_EUC_LEAD_OFF) || (cTrail < JIS_EUC_TRAIL_OFF) )
                {
                    *pInfo |= RTL_TEXTTOUNICODE_INFO_INVALID;
                    if ( (nFlags & RTL_TEXTTOUNICODE_FLAGS_INVALID_MASK) == RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR )
                    {
                        *pInfo |= RTL_TEXTTOUNICODE_INFO_ERROR;
                        break;
                    }
                    else if ( (nFlags & RTL_TEXTTOUNICODE_FLAGS_INVALID_MASK) == RTL_TEXTTOUNICODE_FLAGS_INVALID_IGNORE )
                    {
                        pSrcBuf++;
                        continue;
                    }
                    else
                        cConv = RTL_TEXTENC_UNICODE_REPLACEMENT_CHARACTER;
                }
                else
                {
                    *pInfo |= RTL_TEXTTOUNICODE_INFO_MBUNDEFINED;
                    if ( (nFlags & RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_MASK) == RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR )
                    {
                        *pInfo |= RTL_TEXTTOUNICODE_INFO_ERROR;
                        break;
                    }
                    else if ( (nFlags & RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_MASK) == RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_IGNORE )
                    {
                        pSrcBuf++;
                        continue;
                    }
                    else
                        cConv = RTL_TEXTENC_UNICODE_REPLACEMENT_CHARACTER;
                }
            }
        }

        if ( pDestBuf == pEndDestBuf )
        {
            *pInfo |= RTL_TEXTTOUNICODE_INFO_ERROR | RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOSMALL;
            break;
        }

        *pDestBuf = cConv;
        pDestBuf++;
        pSrcBuf++;
    }

    *pSrcCvtBytes = nSrcBytes - (pEndSrcBuf-pSrcBuf);
    return (nDestChars - (pEndDestBuf-pDestBuf));
}

/* ----------------------------------------------------------------------- */

sal_Size ImplUnicodeToEUCJP( const void* pData,
                             SAL_UNUSED_PARAMETER void*,
                             const sal_Unicode* pSrcBuf, sal_Size nSrcChars,
                             char* pDestBuf, sal_Size nDestBytes,
                             sal_uInt32 nFlags, sal_uInt32* pInfo,
                             sal_Size* pSrcCvtChars )
{
    sal_uInt32                  cConv;
    sal_Unicode                 c;
    unsigned char                   nHighChar;
    unsigned char                   nLowChar;
    const ImplUniToDBCSHighTab* pHighEntry;
    const ImplUniToDBCSHighTab* pHighTab;
    const ImplEUCJPConvertData* pConvertData = static_cast<const ImplEUCJPConvertData*>(pData);
    char*                   pEndDestBuf;
    const sal_Unicode*          pEndSrcBuf;

    *pInfo = 0;
    pEndDestBuf = pDestBuf+nDestBytes;
    pEndSrcBuf  = pSrcBuf+nSrcChars;
    while ( pSrcBuf < pEndSrcBuf )
    {
        c = *pSrcBuf;

        /* ASCII */
        if ( c <= 0x7F )
            cConv = c;
        /* Half-width katakana */
        else if ( (c >= 0xFF61) && (c <= 0xFF9F) )
            cConv = 0x8E00+0xA1+(c-0xFF61);
        else
        {
            nHighChar = (unsigned char)((c >> 8) & 0xFF);
            nLowChar = (unsigned char)(c & 0xFF);

            /* JIS 0208 */
            pHighTab = pConvertData->mpUniToJIS0208HighTab;
            pHighEntry = pHighTab+nHighChar;
            if ( (nLowChar >= pHighEntry->mnLowStart) && (nLowChar <= pHighEntry->mnLowEnd) )
            {
                cConv = pHighEntry->mpToUniTrailTab[nLowChar-pHighEntry->mnLowStart];
                if (cConv != 0)
                    cConv |= 0x8080;
            }
            else
                cConv = 0;

            /* JIS 0212 */
            if ( !cConv )
            {
                pHighTab = pConvertData->mpUniToJIS0212HighTab;
                pHighEntry = pHighTab+nHighChar;
                if ( (nLowChar >= pHighEntry->mnLowStart) && (nLowChar <= pHighEntry->mnLowEnd) )
                {
                    cConv = pHighEntry->mpToUniTrailTab[nLowChar-pHighEntry->mnLowStart];
                    if (cConv != 0)
                        cConv |= 0x8F8080;
                }

                if ( !cConv )
                {
                    if ( nFlags & RTL_UNICODETOTEXT_FLAGS_UNDEFINED_REPLACE )
                    {
                        /* !!! */
                    }

                    if ( nFlags & RTL_UNICODETOTEXT_FLAGS_UNDEFINED_REPLACESTR )
                    {
                        /* !!! */
                    }

                    /* Handle undefined and surrogates characters */
                    /* (all surrogates characters are undefined) */
                    if (sal::detail::textenc::handleUndefinedUnicodeToTextChar(
                            &pSrcBuf, pEndSrcBuf, &pDestBuf, pEndDestBuf,
                            nFlags, pInfo))
                        continue;
                    else
                        break;
                }
            }
        }

        /* SingleByte */
        if ( !(cConv & 0xFFFF00) )
        {
            if ( pDestBuf == pEndDestBuf )
            {
                *pInfo |= RTL_UNICODETOTEXT_INFO_ERROR | RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
                break;
            }

            *pDestBuf = static_cast< char >(static_cast< unsigned char >(cConv & 0xFF));
            pDestBuf++;
        }
        /* DoubleByte */
        else if ( !(cConv & 0xFF0000) )
        {
            if ( pDestBuf+1 >= pEndDestBuf )
            {
                *pInfo |= RTL_UNICODETOTEXT_INFO_ERROR | RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
                break;
            }

            *pDestBuf = static_cast< char >(static_cast< unsigned char >((cConv >> 8) & 0xFF));
            pDestBuf++;
            *pDestBuf = static_cast< char >(static_cast< unsigned char >(cConv & 0xFF));
            pDestBuf++;
        }
        else
        {
            if ( pDestBuf+2 >= pEndDestBuf )
            {
                *pInfo |= RTL_UNICODETOTEXT_INFO_ERROR | RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
                break;
            }

            *pDestBuf = static_cast< char >(static_cast< unsigned char >((cConv >> 16) & 0xFF));
            pDestBuf++;
            *pDestBuf = static_cast< char >(static_cast< unsigned char >((cConv >> 8) & 0xFF));
            pDestBuf++;
            *pDestBuf = static_cast< char >(static_cast< unsigned char >(cConv & 0xFF));
            pDestBuf++;
        }

        pSrcBuf++;
    }

    *pSrcCvtChars = nSrcChars - (pEndSrcBuf-pSrcBuf);
    return (nDestBytes - (pEndDestBuf-pDestBuf));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
