/*************************************************************************
 *
 *  $RCSfile: tcvtbyte.c,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:17:28 $
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

#define _RTL_TCVTBYTE_C

#ifndef _RTL_TENCHELP_H
#include <tenchelp.h>
#endif
#ifndef _RTL_TEXTCVT_H
#include <rtl/textcvt.h>
#endif

/* ======================================================================= */

sal_Size ImplSymbolToUnicode( const ImplTextConverterData* pData,
                              void* pContext,
                              const sal_Char* pSrcBuf, sal_Size nSrcBytes,
                              sal_Unicode* pDestBuf, sal_Size nDestChars,
                              sal_uInt32 nFlags, sal_uInt32* pInfo,
                              sal_Size* pSrcCvtBytes )
{
    sal_uChar               c;
    sal_Unicode*            pEndDestBuf;
    const sal_Char*         pEndSrcBuf;

    *pInfo = 0;
    pEndDestBuf = pDestBuf+nDestChars;
    pEndSrcBuf  = pSrcBuf+nSrcBytes;
    while ( pSrcBuf < pEndSrcBuf )
    {
        if ( pDestBuf == pEndDestBuf )
        {
            *pInfo |= RTL_TEXTTOUNICODE_INFO_ERROR | RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOSMALL;
            break;
        }

        /* 0-31 (all Control-Character get the same Unicode value) */
        c = (sal_uChar)*pSrcBuf;
        if ( c <= 0x1F )
            *pDestBuf = (sal_Unicode)c;
        else
            *pDestBuf = ((sal_Unicode)c)+0xF000;
        pDestBuf++;
        pSrcBuf++;
    }

    *pSrcCvtBytes = nSrcBytes - (pEndSrcBuf-pSrcBuf);
    return (nDestChars - (pEndDestBuf-pDestBuf));
}

/* ----------------------------------------------------------------------- */

sal_Size ImplUnicodeToSymbol( const ImplTextConverterData* pData,
                              void* pContext,
                              const sal_Unicode* pSrcBuf, sal_Size nSrcChars,
                              sal_Char* pDestBuf, sal_Size nDestBytes,
                              sal_uInt32 nFlags, sal_uInt32* pInfo,
                              sal_Size* pSrcCvtChars )
{
    int                     nAction;
    sal_Unicode             c;
    sal_Char*               pEndDestBuf;
    const sal_Unicode*      pEndSrcBuf;

    *pInfo = 0;
    pEndDestBuf = pDestBuf+nDestBytes;
    pEndSrcBuf  = pSrcBuf+nSrcChars;
    while ( pSrcBuf < pEndSrcBuf )
    {
        if ( pDestBuf == pEndDestBuf )
        {
            *pInfo |= RTL_UNICODETOTEXT_INFO_ERROR | RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
            break;
        }

        c = *pSrcBuf;
        if ( (c >= 0xF000) && (c <= 0xF0FF) )
        {
            *pDestBuf = (sal_Char)(sal_uChar)(c-0xF000);
            pDestBuf++;
            pSrcBuf++;
        }
        // Normally 0x001F, but in many cases also symbol characters
        // are stored in the first 256 bytes, so that we don't change
        // these values
        else if ( c <= 0x00FF )
        {
            *pDestBuf = (sal_Char)(sal_uChar)c;
            pDestBuf++;
            pSrcBuf++;
        }
        else
        {
            if ( nFlags & RTL_UNICODETOTEXT_FLAGS_UNDEFINED_REPLACE )
            {
                /* !!! */
                /* Only ascii  characters < 0x1F */
            }

            /* Handle undefined and surrogates characters */
            /* (all surrogates characters are undefined) */
            nAction = ImplHandleUndefinedUnicodeToTextChar( pData,
                                                            &pSrcBuf, pEndSrcBuf,
                                                            &pDestBuf, pEndDestBuf,
                                                            nFlags, pInfo );
            if ( nAction == IMPL_TEXTCVT_BREAK )
                break;
        }
    }

    *pSrcCvtChars = nSrcChars - (pEndSrcBuf-pSrcBuf);
    return (nDestBytes - (pEndDestBuf-pDestBuf));
}

/* ======================================================================= */

sal_Size ImplCharToUnicode( const ImplTextConverterData* pData,
                            void* pContext,
                            const sal_Char* pSrcBuf, sal_Size nSrcBytes,
                            sal_Unicode* pDestBuf, sal_Size nDestChars,
                            sal_uInt32 nFlags, sal_uInt32* pInfo,
                            sal_Size* pSrcCvtBytes )
{
    sal_uChar                   c;
    sal_Unicode                 cConv;
    const ImplByteConvertData*  pConvertData = (const ImplByteConvertData*)(pData->mpConvertTables);
    sal_Unicode*                pEndDestBuf;
    const sal_Char*             pEndSrcBuf;

    *pInfo = 0;
    pEndDestBuf = pDestBuf+nDestChars;
    pEndSrcBuf  = pSrcBuf+nSrcBytes;
    while ( pSrcBuf < pEndSrcBuf )
    {
        c = (sal_uChar)*pSrcBuf;
        if ( c < 0x80 )
            cConv = c;
        else
        {
            if ( (c >= pConvertData->mnToUniStart1) && (c <= pConvertData->mnToUniEnd1) )
                cConv = pConvertData->mpToUniTab1[c-pConvertData->mnToUniStart1];
            else if ( (c >= pConvertData->mnToUniStart2) && (c <= pConvertData->mnToUniEnd2) )
                cConv = pConvertData->mpToUniTab2[c-pConvertData->mnToUniStart2];
            else
                cConv = 0;
            if ( !cConv )
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
                    cConv = ImplGetUndefinedUnicodeChar( c, nFlags, pData );
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

static sal_Char ImplConvertUnicodeCharToChar( const ImplByteConvertData* pConvertData,
                                              sal_Unicode c )
{
    sal_Char                    cConv;
    const ImplUniCharTabData*   pToCharExTab;

    if ( c < 0x80 )
        cConv = (sal_Char)c;
    else
    {
        if ( (c >= pConvertData->mnToCharStart1) && (c <= pConvertData->mnToCharEnd1) )
            cConv = (sal_Char)pConvertData->mpToCharTab1[c-pConvertData->mnToCharStart1];
        else if ( (c >= pConvertData->mnToCharStart2) && (c <= pConvertData->mnToCharEnd2) )
            cConv = (sal_Char)pConvertData->mpToCharTab2[c-pConvertData->mnToCharStart2];
        else
        {
            cConv = 0x00;

            pToCharExTab = pConvertData->mpToCharExTab;
            if ( pToCharExTab )
            {
                sal_uInt16                  nLow;
                sal_uInt16                  nHigh;
                sal_uInt16                  nMid;
                sal_uInt16                  nCompareChar;
                const ImplUniCharTabData*   pCharExData;

                nLow = 0;
                nHigh = pConvertData->mnToCharExCount-1;
                do
                {
                    nMid = (nLow+nHigh)/2;
                    pCharExData = pToCharExTab+nMid;
                    nCompareChar = pCharExData->mnUniChar;
                    if ( c < nCompareChar )
                    {
                        if ( !nMid )
                            break;
                        nHigh = nMid-1;
                    }
                    else
                    {
                        if ( c > nCompareChar )
                            nLow = nMid+1;
                        else
                        {
                            cConv = (sal_Char)pCharExData->mnChar;
                            break;
                        }
                    }
                }
                while ( nLow <= nHigh );
            }
        }
    }

    return cConv;
}

/* ----------------------------------------------------------------------- */

sal_Size ImplUnicodeToChar( const ImplTextConverterData* pData,
                            void* pContext,
                            const sal_Unicode* pSrcBuf, sal_Size nSrcChars,
                            sal_Char* pDestBuf, sal_Size nDestBytes,
                            sal_uInt32 nFlags, sal_uInt32* pInfo,
                            sal_Size* pSrcCvtChars )
{
    int                         nAction;
    sal_Unicode                 c;
    sal_Char                    cConv;
    const ImplByteConvertData*  pConvertData = (const ImplByteConvertData*)(pData->mpConvertTables);
    sal_Char*                   pEndDestBuf;
    const sal_Unicode*          pEndSrcBuf;
    int                         i;
    int                         n;
    sal_uInt16                  cTemp;
    sal_Char                    aTempBuf[IMPL_MAX_REPLACECHAR+1];
    const sal_uInt16*           pReplace;

    *pInfo = 0;
    pEndDestBuf = pDestBuf+nDestBytes;
    pEndSrcBuf  = pSrcBuf+nSrcChars;
    while ( pSrcBuf < pEndSrcBuf )
    {
        c = *pSrcBuf;
        if ( c < 0x80 )
            cConv = (sal_Char)c;
        else
        {
            cConv = ImplConvertUnicodeCharToChar( pConvertData, c );

            if ( !cConv )
            {
                if ( nFlags & RTL_UNICODETOTEXT_FLAGS_UNDEFINED_REPLACE )
                {
                    cTemp = ImplGetReplaceChar( c );
                    if ( cTemp )
                        cConv = ImplConvertUnicodeCharToChar( pConvertData, cTemp );
                }

                if ( !cConv )
                {
                    if ( nFlags & RTL_UNICODETOTEXT_FLAGS_UNDEFINED_REPLACESTR )
                    {
                        pReplace = ImplGetReplaceString( c );
                        if ( pReplace )
                        {
                            i = 0;
                            while ( *pReplace && (i < IMPL_MAX_REPLACECHAR) )
                            {
                                cConv = ImplConvertUnicodeCharToChar( pConvertData, *pReplace );
                                if ( !cConv )
                                    break;
                                aTempBuf[i] = cConv;
                                pReplace++;
                                i++;
                            }

                            if ( cConv )
                            {
                                if ( pDestBuf+i >= pEndDestBuf )
                                {
                                    *pInfo |= RTL_UNICODETOTEXT_INFO_ERROR | RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
                                    break;
                                }

                                n = 0;
                                while ( n < i )
                                {
                                    *pDestBuf = aTempBuf[n];
                                    pDestBuf++;
                                    n++;
                                }
                                pSrcBuf++;
                                continue;
                            }
                        }
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

        if ( pDestBuf == pEndDestBuf )
        {
            *pInfo |= RTL_UNICODETOTEXT_INFO_ERROR | RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
            break;
        }

        *pDestBuf = cConv;
        pDestBuf++;
        pSrcBuf++;
    }

    *pSrcCvtChars = nSrcChars - (pEndSrcBuf-pSrcBuf);
    return (nDestBytes - (pEndDestBuf-pDestBuf));
}

