/*************************************************************************
 *
 *  $RCSfile: tcvtmb.c,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:17:29 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define _RTL_TCVTMB_C

#ifndef _RTL_TENCHELP_H
#include <tenchelp.h>
#endif
#ifndef _RTL_TEXTCVT_H
#include <rtl/textcvt.h>
#endif

/* ======================================================================= */

/* DBCS to Unicode conversion routine use a lead table for the first byte, */
/* where we determine the trail table or for single byte chars the unicode */
/* value. We have for all lead byte a separate table, because we can */
/* then share many tables for diffrent charset encodings. */

/* ======================================================================= */

sal_Size ImplDBCSToUnicode( const ImplTextConverterData* pData, void* pContext,
                            const sal_Char* pSrcBuf, sal_Size nSrcBytes,
                            sal_Unicode* pDestBuf, sal_Size nDestChars,
                            sal_uInt32 nFlags, sal_uInt32* pInfo,
                            sal_Size* pSrcCvtBytes )
{
    sal_uChar                   cLead;
    sal_uChar                   cTrail;
    sal_Unicode                 cConv;
    const ImplDBCSToUniLeadTab* pLeadEntry;
    const ImplDBCSConvertData*  pConvertData = (const ImplDBCSConvertData*)(pData->mpConvertTables);
    const ImplDBCSToUniLeadTab* pLeadTab = pConvertData->mpToUniLeadTab;
    sal_Unicode*                pEndDestBuf;
    const sal_Char*             pEndSrcBuf;

    *pInfo = 0;
    pEndDestBuf = pDestBuf+nDestChars;
    pEndSrcBuf  = pSrcBuf+nSrcBytes;
    while ( pSrcBuf < pEndSrcBuf )
    {
        cLead = (sal_uChar)*pSrcBuf;

        /* get entry for the lead byte */
        pLeadEntry = pLeadTab+cLead;

        /* SingleByte char? */
        if ( !pLeadEntry->mpToUniTrailTab )
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
                    cConv = ImplGetUndefinedUnicodeChar( cLead, nFlags, pData );
            }
        }
        else
        {
            /* Source buffer to small */
            if ( pSrcBuf == pEndSrcBuf )
            {
                *pInfo |= RTL_TEXTTOUNICODE_INFO_ERROR | RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOSMALL;
                break;
            }

            pSrcBuf++;
            cTrail = (sal_uChar)*pSrcBuf;
            if ( (cTrail >= pLeadEntry->mnTrailStart) && (cTrail <= pLeadEntry->mnTrailEnd) )
                cConv = pLeadEntry->mpToUniTrailTab[cTrail-pLeadEntry->mnTrailStart];
            else
                cConv = 0;

            if ( !cConv )
            {
                /* EUC Ranges */
                sal_uInt16              i;
                const ImplDBCSEUCData*  pEUCTab = pConvertData->mpEUCTab;
                for ( i = 0; i < pConvertData->mnEUCCount; i++ )
                {
                    if ( (cLead >= pEUCTab->mnLeadStart) &&
                         (cLead <= pEUCTab->mnLeadEnd) )
                    {
                        sal_uInt16 nTrailCount = 0;
                        if ( (cTrail >= pEUCTab->mnTrail1Start) &&
                             (cTrail <= pEUCTab->mnTrail1End) )
                        {
                            cConv = pEUCTab->mnUniStart+
                                    ((cLead-pEUCTab->mnLeadStart)*pEUCTab->mnTrailRangeCount)+
                                    (cTrail-pEUCTab->mnTrail1Start);
                            break;
                        }
                        else
                        {
                            nTrailCount = pEUCTab->mnTrail1End-pEUCTab->mnTrail1Start+1;
                            if ( (pEUCTab->mnTrailCount >= 2) &&
                                 (cTrail >= pEUCTab->mnTrail2Start) &&
                                 (cTrail <= pEUCTab->mnTrail2End) )
                            {
                                cConv = pEUCTab->mnUniStart+
                                        ((cLead-pEUCTab->mnLeadStart)*pEUCTab->mnTrailRangeCount)+
                                        nTrailCount+
                                        (cTrail-pEUCTab->mnTrail2Start);
                                break;
                            }
                            else
                            {
                                nTrailCount = pEUCTab->mnTrail2End-pEUCTab->mnTrail2Start+1;
                                if ( (pEUCTab->mnTrailCount >= 3) &&
                                     (cTrail >= pEUCTab->mnTrail3Start) &&
                                     (cTrail <= pEUCTab->mnTrail3End) )
                                {
                                    cConv = pEUCTab->mnUniStart+
                                            ((cLead-pEUCTab->mnLeadStart)*pEUCTab->mnTrailRangeCount)+
                                            nTrailCount+
                                            (cTrail-pEUCTab->mnTrail3Start);
                                    break;
                                }
                            }
                        }
                    }

                    pEUCTab++;
                }

                if ( !cConv )
                {
                    /* Wir vergleichen den kompletten Trailbereich den wir */
                    /* definieren, der normalerweise groesser sein kann als */
                    /* der definierte. Dies machen wir, damit Erweiterungen von */
                    /* uns nicht beruecksichtigten Encodings so weit wie */
                    /* moeglich auch richtig zu behandeln, das double byte */
                    /* characters auch als ein einzelner Character behandelt */
                    /* wird. */
                    if ( (cTrail < pConvertData->mnTrailStart) || (cTrail > pConvertData->mnTrailEnd) )
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
                            cConv = RTL_UNICODE_CHAR_DEFAULT;
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
                            cConv = RTL_UNICODE_CHAR_DEFAULT;
                    }
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

sal_Size ImplUnicodeToDBCS( const ImplTextConverterData* pData, void* pContext,
                            const sal_Unicode* pSrcBuf, sal_Size nSrcChars,
                            sal_Char* pDestBuf, sal_Size nDestBytes,
                            sal_uInt32 nFlags, sal_uInt32* pInfo,
                            sal_Size* pSrcCvtChars )
{
    int                         nAction;
    sal_uInt16                  cConv;
    sal_Unicode                 c;
    sal_uChar                   nHighChar;
    sal_uChar                   nLowChar;
    const ImplUniToDBCSHighTab* pHighEntry;
    const ImplDBCSConvertData*  pConvertData = (const ImplDBCSConvertData*)(pData->mpConvertTables);
    const ImplUniToDBCSHighTab* pHighTab = pConvertData->mpToDBCSHighTab;
    sal_Char*                   pEndDestBuf;
    const sal_Unicode*          pEndSrcBuf;

    *pInfo = 0;
    pEndDestBuf = pDestBuf+nDestBytes;
    pEndSrcBuf  = pSrcBuf+nSrcChars;
    while ( pSrcBuf < pEndSrcBuf )
    {
        c = *pSrcBuf;
        nHighChar = (sal_uChar)((c >> 8) & 0xFF);
        nLowChar = (sal_uChar)(c & 0xFF);

        /* get entry for the high byte */
        pHighEntry = pHighTab+nHighChar;

        /* is low byte in the table range */
        if ( (nLowChar >= pHighEntry->mnLowStart) && (nLowChar <= pHighEntry->mnLowEnd) )
            cConv = pHighEntry->mpToUniTrailTab[nLowChar-pHighEntry->mnLowStart];
        else
            cConv = 0;

        if ( !cConv && !c )
        {
            /* EUC Ranges */
            sal_uInt16              i;
            const ImplDBCSEUCData*  pEUCTab = pConvertData->mpEUCTab;
            for ( i = 0; i < pConvertData->mnEUCCount; i++ )
            {
                if ( (c >= pEUCTab->mnUniStart) && (c <= pEUCTab->mnUniEnd) )
                {
                    sal_uInt16 nLead = (c-pEUCTab->mnUniStart) / pEUCTab->mnTrailRangeCount;
                    sal_uInt16 nOff = c-(nLead*pEUCTab->mnTrailRangeCount);
                    cConv = pEUCTab->mnLeadStart+nLead;
                    cConv <<= 8;
                    if ( nOff < (pEUCTab->mnTrail1End-pEUCTab->mnTrail1Start+1) )
                        cConv += pEUCTab->mnTrail1Start+nOff;
                    else
                    {
                        nOff -= pEUCTab->mnTrail1End-pEUCTab->mnTrail1Start+1;
                        if ( nOff < (pEUCTab->mnTrail2End-pEUCTab->mnTrail2Start+1) )
                            cConv += pEUCTab->mnTrail2Start+nOff;
                        else
                        {
                            nOff -= pEUCTab->mnTrail2End-pEUCTab->mnTrail2Start+1;
                            cConv += pEUCTab->mnTrail3Start+nOff;
                        }
                    }

                    break;
                }

                pEUCTab++;
            }

            if ( (c >= 0xF100) && (c <= 0xF1FF) )
            {
                if ( nFlags & RTL_UNICODETOTEXT_FLAGS_PRIVATE_MAPTO0 )
                    cConv = (sal_Char)(sal_uChar)(c & 0xFF);
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
                nAction = ImplHandleUndefinedUnicodeToTextChar( pData,
                                                                &pSrcBuf, pEndSrcBuf,
                                                                &pDestBuf, pEndDestBuf,
                                                                nFlags, pInfo );
                if ( nAction == IMPL_TEXTCVT_BREAK )
                    break;
                else
                    continue;
            }
        }

        /* SingleByte */
        if ( !(cConv & 0xFF00) )
        {
            if ( pDestBuf == pEndDestBuf )
            {
                *pInfo |= RTL_UNICODETOTEXT_INFO_ERROR | RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
                break;
            }

            *pDestBuf = (sal_Char)(sal_uChar)(cConv & 0xFF);
            pDestBuf++;
        }
        else
        {
            if ( pDestBuf+1 >= pEndDestBuf )
            {
                *pInfo |= RTL_UNICODETOTEXT_INFO_ERROR | RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
                break;
            }

            *pDestBuf = (sal_Char)(sal_uChar)((cConv >> 8) & 0xFF);
            pDestBuf++;
            *pDestBuf = (sal_Char)(sal_uChar)(cConv & 0xFF);
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

sal_Size ImplEUCJPToUnicode( const ImplTextConverterData* pData,
                             void* pContext,
                             const sal_Char* pSrcBuf, sal_Size nSrcBytes,
                             sal_Unicode* pDestBuf, sal_Size nDestChars,
                             sal_uInt32 nFlags, sal_uInt32* pInfo,
                             sal_Size* pSrcCvtBytes )
{
    sal_uChar                   c;
    sal_uChar                   cLead;
    sal_uChar                   cTrail;
    sal_Unicode                 cConv;
    const ImplDBCSToUniLeadTab* pLeadEntry;
    const ImplDBCSToUniLeadTab* pLeadTab;
    const ImplEUCJPConvertData* pConvertData = (const ImplEUCJPConvertData*)(pData->mpConvertTables);
    sal_Unicode*                pEndDestBuf;
    const sal_Char*             pEndSrcBuf;

    *pInfo = 0;
    pEndDestBuf = pDestBuf+nDestChars;
    pEndSrcBuf  = pSrcBuf+nSrcBytes;
    while ( pSrcBuf < pEndSrcBuf )
    {
        c = (sal_uChar)*pSrcBuf;

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
                if ( pSrcBuf == pEndSrcBuf )
                {
                    *pInfo |= RTL_TEXTTOUNICODE_INFO_ERROR | RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOSMALL;
                    break;
                }

                pSrcBuf++;
                c = (sal_uChar)*pSrcBuf;
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
                    if ( pSrcBuf+1 == pEndSrcBuf )
                    {
                        *pInfo |= RTL_TEXTTOUNICODE_INFO_ERROR | RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOSMALL;
                        break;
                    }

                    pSrcBuf++;
                    cLead = (sal_uChar)*pSrcBuf;
                    pSrcBuf++;
                    cTrail = (sal_uChar)*pSrcBuf;
                    pLeadTab = pConvertData->mpJIS0212ToUniLeadTab;
                }
                /* CodeSet 2 JIS 0208-1997 */
                /* A1-FE + A1-FE */
                else
                {
                    /* Source buffer to small */
                    if ( pSrcBuf == pEndSrcBuf )
                    {
                        *pInfo |= RTL_TEXTTOUNICODE_INFO_ERROR | RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOSMALL;
                        break;
                    }

                    cLead = c;
                    pSrcBuf++;
                    cTrail = (sal_uChar)*pSrcBuf;
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
                /* Wir vergleichen den kompletten Trailbereich den wir */
                /* definieren, der normalerweise groesser sein kann als */
                /* der definierte. Dies machen wir, damit Erweiterungen von */
                /* uns nicht beruecksichtigten Encodings so weit wie */
                /* moeglich auch richtig zu behandeln, das double byte */
                /* characters auch als ein einzelner Character behandelt */
                /* wird. */
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
                        cConv = RTL_UNICODE_CHAR_DEFAULT;
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
                        cConv = RTL_UNICODE_CHAR_DEFAULT;
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

sal_Size ImplUnicodeToEUCJP( const ImplTextConverterData* pData,
                             void* pContext,
                             const sal_Unicode* pSrcBuf, sal_Size nSrcChars,
                             sal_Char* pDestBuf, sal_Size nDestBytes,
                             sal_uInt32 nFlags, sal_uInt32* pInfo,
                             sal_Size* pSrcCvtChars )
{
    int                         nAction;
    sal_uInt32                  cConv;
    sal_Unicode                 c;
    sal_uChar                   nHighChar;
    sal_uChar                   nLowChar;
    const ImplUniToDBCSHighTab* pHighEntry;
    const ImplUniToDBCSHighTab* pHighTab;
    const ImplEUCJPConvertData* pConvertData = (const ImplEUCJPConvertData*)(pData->mpConvertTables);
    sal_Char*                   pEndDestBuf;
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
            nHighChar = (sal_uChar)((c >> 8) & 0xFF);
            nLowChar = (sal_uChar)(c & 0xFF);

            /* JIS 0208 */
            pHighTab = pConvertData->mpUniToJIS0208HighTab;
            pHighEntry = pHighTab+nHighChar;
            if ( (nLowChar >= pHighEntry->mnLowStart) && (nLowChar <= pHighEntry->mnLowEnd) )
            {
                cConv = pHighEntry->mpToUniTrailTab[nLowChar-pHighEntry->mnLowStart];
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
                    cConv = 0x8F0000+(pHighEntry->mpToUniTrailTab[nLowChar-pHighEntry->mnLowStart]);
                    cConv |= 0x8080;
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
                    nAction = ImplHandleUndefinedUnicodeToTextChar( pData,
                                                                    &pSrcBuf, pEndSrcBuf,
                                                                    &pDestBuf, pEndDestBuf,
                                                                    nFlags, pInfo );
                    if ( nAction == IMPL_TEXTCVT_BREAK )
                        break;
                    else
                        continue;
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

            *pDestBuf = (sal_Char)(sal_uChar)(cConv & 0xFF);
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

            *pDestBuf = (sal_Char)(sal_uChar)((cConv >> 8) & 0xFF);
            pDestBuf++;
            *pDestBuf = (sal_Char)(sal_uChar)(cConv & 0xFF);
            pDestBuf++;
        }
        else
        {
            if ( pDestBuf+2 >= pEndDestBuf )
            {
                *pInfo |= RTL_UNICODETOTEXT_INFO_ERROR | RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
                break;
            }

            *pDestBuf = (sal_Char)(sal_uChar)((cConv >> 16) & 0xFF);
            pDestBuf++;
            *pDestBuf = (sal_Char)(sal_uChar)((cConv >> 8) & 0xFF);
            pDestBuf++;
            *pDestBuf = (sal_Char)(sal_uChar)(cConv & 0xFF);
            pDestBuf++;
        }

        pSrcBuf++;
    }

    *pSrcCvtChars = nSrcChars - (pEndSrcBuf-pSrcBuf);
    return (nDestBytes - (pEndDestBuf-pDestBuf));
}

/* ======================================================================= */

sal_Size ImplUnicodeToJISX0208( const ImplTextConverterData* pData,
                                void* pContext,
                                const sal_Unicode* pSrcBuf, sal_Size nSrcChars,
                                sal_Char* pDestBuf, sal_Size nDestBytes,
                                sal_uInt32 nFlags, sal_uInt32* pInfo,
                                sal_Size* pSrcCvtChars )
{
    sal_Unicode                 c;
    sal_uChar                   nHighChar;
    sal_uChar                   nLowChar;
    const ImplUniToDBCSHighTab* pHighTab;
    const ImplUniToDBCSHighTab* pHighEntry;
    sal_uInt16                  cConv;
    sal_Char*                   pEndDestBuf;
    const sal_Unicode*          pEndSrcBuf;

    pEndDestBuf = pDestBuf+nDestBytes;
    pEndSrcBuf = pSrcBuf+nSrcChars;

    *pInfo = 0;
    pHighTab = (ImplUniToDBCSHighTab*)pData->mpConvertTables;

    while ( pSrcBuf < pEndSrcBuf)
    {
        c = *pSrcBuf;
        nHighChar = (sal_uChar)((c >> 8) & 0xFF);
        nLowChar = (sal_uChar)(c & 0xFF);

        pHighEntry = pHighTab + nHighChar;
        if ( (nLowChar >= pHighEntry->mnLowStart) && (nLowChar <= pHighEntry->mnLowEnd) )
            cConv = pHighEntry->mpToUniTrailTab[nLowChar-pHighEntry->mnLowStart];
        else
            cConv = 0;

        if (cConv != 0)
        {
            if ( pDestBuf+1 >=  pEndDestBuf )
            {
                *pInfo |= RTL_UNICODETOTEXT_INFO_ERROR | RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
                break;
            }

            *pDestBuf = (sal_Char)(sal_uChar)((cConv >> 8) & 0xFF);
            pDestBuf++;
            *pDestBuf = (sal_Char)(sal_uChar)(cConv & 0xFF);
            pDestBuf++;
        }
        pSrcBuf++;
    }

    *pSrcCvtChars = nSrcChars -  (pEndSrcBuf-pSrcBuf);
    return (nDestBytes - (pEndDestBuf-pDestBuf));
}
