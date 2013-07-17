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

#include "address.hxx"
#include "global.hxx"
#include "compiler.hxx"
#include "document.hxx"
#include "externalrefmgr.hxx"

#include "globstr.hrc"
#include <sal/alloca.h>

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/ExternalLinkInfo.hpp>
#include <com/sun/star/sheet/ExternalLinkType.hpp>
#include <comphelper/string.hxx>
#include <sfx2/objsh.hxx>
#include <tools/urlobj.hxx>

using namespace ::com::sun::star;

const ScAddress::Details ScAddress::detailsOOOa1( formula::FormulaGrammar::CONV_OOO, 0, 0 );

ScAddress::Details::Details ( const ScDocument* pDoc,
                              const ScAddress & rAddr ) :
    eConv( pDoc->GetAddressConvention() ),
    nRow( rAddr.Row() ),
    nCol( rAddr.Col() )
{
}


////////////////////////////////////////////////////////////////////////////

/**
 * Parse from the opening single quote to the closing single quote.  Inside
 * the quotes, a single quote character is encoded by double single-quote
 * characters.
 *
 * @param p pointer to the first character to begin parsing.
 * @param rName (reference) parsed name within the quotes.  If the name is
 *              empty, either the parsing failed or it's an empty quote.
 *
 * @return pointer to the character immediately after the closing single
 *         quote.
 */
static const sal_Unicode* lcl_ParseQuotedName( const sal_Unicode* p, String& rName )
{
    rName.Erase();
    if (*p != '\'')
        return p;

    const sal_Unicode* pStart = p;
    sal_Unicode cPrev = 0;
    for (++p; *p; ++p)
    {
        if (*p == '\'')
        {
            if (cPrev == '\'')
            {
                // double single-quote equals one single quote.
                rName += *p;
                cPrev = 0;
                continue;
            }
        }
        else if (cPrev == '\'')
            // We are past the closing quote.  We're done!
            return p;
        else
            rName += *p;
        cPrev = *p;
    }
    rName.Erase();
    return pStart;
}

static long int
sal_Unicode_strtol ( const sal_Unicode*  p,
                     const sal_Unicode** pEnd )
{
    long int accum = 0, prev = 0;
    bool is_neg = false;

    if( *p == '-' )
    {
        is_neg = true;
        p++;
    }
    else if( *p == '+' )
        p++;

    while (CharClass::isAsciiDigit( *p ))
    {
        accum = accum * 10 + *p - '0';
        if( accum < prev )
        {
            *pEnd = NULL;
            return 0;
        }
        prev = accum;
        p++;
    }

    *pEnd = p;
    return is_neg ? -accum : accum;
}

static const sal_Unicode* lcl_eatWhiteSpace( const sal_Unicode* p )
{
    if ( p )
    {
        while(  *p == ' ' )
            ++p;
    }
    return p;
}

/** Determines the number of sheets an external reference spans and sets
    rRange.aEnd.nTab accordingly. If a sheet is not found, the corresponding
    bits in rFlags are cleared. pExtInfo is filled if it wasn't already. If in
    cached order rStartTabName comes after rEndTabName, pExtInfo->maTabName
    is set to rEndTabName.
    @returns <FALSE/> if pExtInfo is already filled and rExternDocName does not
             result in the identical file ID. Else <TRUE/>.
 */
static bool lcl_ScRange_External_TabSpan(
        ScRange & rRange,
        sal_uInt16 & rFlags,
        ScAddress::ExternalInfo* pExtInfo,
        const String & rExternDocName,
        const String & rStartTabName,
        const String & rEndTabName,
        ScDocument* pDoc )
{
    if (!rExternDocName.Len())
        return !pExtInfo || !pExtInfo->mbExternal;

    ScExternalRefManager* pRefMgr = pDoc->GetExternalRefManager();
    if (pRefMgr->isOwnDocument( rExternDocName))
    {
        // This is an internal document.  Get the sheet positions from the
        // ScDocument instance.
        if (rStartTabName.Len())
        {
            SCTAB nTab;
            if (pDoc->GetTable(rStartTabName, nTab))
                rRange.aStart.SetTab(nTab);
        }

        if (rEndTabName.Len())
        {
            SCTAB nTab;
            if (pDoc->GetTable(rEndTabName, nTab))
                rRange.aEnd.SetTab(nTab);
        }
        return !pExtInfo || !pExtInfo->mbExternal;
    }

    sal_uInt16 nFileId = pRefMgr->getExternalFileId( rExternDocName);

    if (pExtInfo)
    {
        if (pExtInfo->mbExternal)
        {
            if (pExtInfo->mnFileId != nFileId)
                return false;
        }
        else
        {
            pExtInfo->mbExternal = true;
            pExtInfo->maTabName = rStartTabName;
            pExtInfo->mnFileId = nFileId;
        }
    }

    if (!rEndTabName.Len() || rStartTabName == rEndTabName)
    {
        rRange.aEnd.SetTab( rRange.aStart.Tab());
        return true;
    }

    SCsTAB nSpan = pRefMgr->getCachedTabSpan( nFileId, rStartTabName, rEndTabName);
    if (nSpan == -1)
        rFlags &= ~(SCA_VALID_TAB | SCA_VALID_TAB2);
    else if (nSpan == 0)
        rFlags &= ~SCA_VALID_TAB2;
    else if (nSpan >= 1)
        rRange.aEnd.SetTab( rRange.aStart.Tab() + nSpan - 1);
    else // (nSpan < -1)
    {
        rRange.aEnd.SetTab( rRange.aStart.Tab() - nSpan - 1);
        if (pExtInfo)
            pExtInfo->maTabName = rEndTabName;
    }
    return true;
}

/** Returns NULL if the string should be a sheet name, but is invalid.
    Returns a pointer to the first character after the sheet name, if there was
    any, else pointer to start.
    @param pMsoxlQuoteStop
        Starting _within_ a quoted name, but still may be 3D; quoted name stops
        at pMsoxlQuoteStop
 */
static const sal_Unicode *
lcl_XL_ParseSheetRef( const sal_Unicode* start,
                      String& rExternTabName,
                      bool allow_3d,
                      const sal_Unicode* pMsoxlQuoteStop )
{
    String aTabName;
    const sal_Unicode *p = start;

    // XL only seems to use single quotes for sheet names.
    if (pMsoxlQuoteStop)
    {
        const sal_Unicode* pCurrentStart = p;
        while (p < pMsoxlQuoteStop)
        {
            if (*p == '\'')
            {
                // We pre-analyzed the quoting, no checks needed here.
                if (*++p == '\'')
                {
                    aTabName.Append( pCurrentStart,
                            sal::static_int_cast<xub_StrLen>( p - pCurrentStart));
                    pCurrentStart = ++p;
                }
            }
            else if (*p == ':')
            {
                break;  // while
            }
            else
                ++p;
        }
        if (pCurrentStart < p)
            aTabName.Append( pCurrentStart, sal::static_int_cast<xub_StrLen>( p - pCurrentStart));
        if (!aTabName.Len())
            return NULL;
        if (p == pMsoxlQuoteStop)
            ++p;    // position on ! of ...'!...
        if( *p != '!' && ( !allow_3d || *p != ':' ) )
            return (!allow_3d && *p == ':') ? p : start;
    }
    else if( *p == '\'')
    {
        p = lcl_ParseQuotedName(p, aTabName);
        if (!aTabName.Len())
            return NULL;
    }
    else
    {
        bool only_digits = true;

        /*
         * Valid: Normal!a1
         * Valid: x.y!a1
         * Invalid: .y!a1
         *
         * Some names starting with digits are actually valid, but
         * unparse quoted. Things are quite tricky: most sheet names
         * starting with a digit are ok, but not those starting with
         * "[0-9]*\." or "[0-9]+[eE]".
         *
         * Valid: 42!a1
         * Valid: 4x!a1
         * Invalid: 1.!a1
         * Invalid: 1e!a1
         */
        while( 1 )
        {
            const sal_Unicode uc = *p;
            if( CharClass::isAsciiAlpha( uc ) || uc == '_' )
            {
                if( only_digits && p != start &&
                   (uc == 'e' || uc == 'E' ) )
                {
                    p = start;
                    break;
                }
                only_digits = false;
                p++;
            }
            else if( CharClass::isAsciiDigit( uc ))
            {
                p++;
            }
            else if( uc == '.' )
            {
                if( only_digits ) // Valid, except after only digits.
                {
                    p = start;
                    break;
                }
                p++;
            }
            else if (uc > 127)
            {
                // non ASCII character is allowed.
                ++p;
            }
            else
                break;
        }

        if( *p != '!' && ( !allow_3d || *p != ':' ) )
            return (!allow_3d && *p == ':') ? p : start;

        aTabName.Append( start, sal::static_int_cast<xub_StrLen>( p - start ) );
    }

    rExternTabName = aTabName;
    return p;
}


/** Tries to obtain the external document index and replace by actual document
    name.

    @param ppErrRet
           Contains the default pointer the caller would return if this method
           returns FALSE, may be replaced by NULL for type or data errors.

    @returns FALSE only if the input name is numeric and not within the index
    sequence, or the link type cannot be determined or data mismatch. Returns
    TRUE in all other cases, also when there is no index sequence or the input
    name is not numeric.
 */
static bool lcl_XL_getExternalDoc( const sal_Unicode** ppErrRet, String& rExternDocName,
        const uno::Sequence< const sheet::ExternalLinkInfo > * pExternalLinks)
{
    // 1-based, sequence starts with an empty element.
    if (pExternalLinks && pExternalLinks->hasElements())
    {
        // A numeric "document name" is an index into the sequence.
        if (CharClass::isAsciiNumeric( rExternDocName))
        {
            sal_Int32 i = rExternDocName.ToInt32();
            if (i < 0 || i >= pExternalLinks->getLength())
                return false;   // with default *ppErrRet
            const sheet::ExternalLinkInfo & rInfo = (*pExternalLinks)[i];
            switch (rInfo.Type)
            {
                case sheet::ExternalLinkType::DOCUMENT :
                    {
                        OUString aStr;
                        if (!(rInfo.Data >>= aStr))
                        {
                            OSL_TRACE( "ScRange::Parse_XL_Header: Data type mismatch for ExternalLinkInfo %d", i);
                            *ppErrRet = NULL;
                            return false;
                        }
                        rExternDocName = aStr;
                    }
                    break;
                    case sheet::ExternalLinkType::SELF :
                        return false;   // ???
                    case sheet::ExternalLinkType::SPECIAL :
                        // silently return nothing (do not assert), caller has to handle this
                        *ppErrRet = NULL;
                        return false;
                default:
                    OSL_TRACE( "ScRange::Parse_XL_Header: unhandled ExternalLinkType %d for index %d",
                            rInfo.Type, i);
                    *ppErrRet = NULL;
                    return false;
            }
        }
    }
    return true;
}


const sal_Unicode* ScRange::Parse_XL_Header(
        const sal_Unicode* p,
        const ScDocument* pDoc,
        String& rExternDocName,
        String& rStartTabName,
        String& rEndTabName,
        sal_uInt16& nFlags,
        bool bOnlyAcceptSingle,
        const uno::Sequence< const sheet::ExternalLinkInfo > * pExternalLinks )
{
    const sal_Unicode* startTabs, *start = p;
    sal_uInt16 nSaveFlags = nFlags;

    // Is this an external reference ?
    rStartTabName.Erase();
    rEndTabName.Erase();
    rExternDocName.Erase();
    const sal_Unicode* pMsoxlQuoteStop = NULL;
    if (*p == '[')
    {
        ++p;
        // Only single quotes are correct, and a double single quote escapes a
        // single quote text inside the quoted text.
        if (*p == '\'')
        {
            p = lcl_ParseQuotedName(p, rExternDocName);
            if (!*p || *p != ']' || !rExternDocName.Len())
            {
                rExternDocName.Erase();
                return start;
            }
        }
        else
        {
            // non-quoted file name.
            p = ScGlobal::UnicodeStrChr( start+1, ']' );
            if( p == NULL )
                return start;
            rExternDocName.Append( start+1, sal::static_int_cast<xub_StrLen>( p-(start+1) ) );
        }
        ++p;

        const sal_Unicode* pErrRet = start;
        if (!lcl_XL_getExternalDoc( &pErrRet, rExternDocName, pExternalLinks))
            return pErrRet;

        rExternDocName = ScGlobal::GetAbsDocName(rExternDocName, pDoc->GetDocumentShell());
    }
    else if (*p == '\'')
    {
        // Sickness in Excel's ODF msoxl namespace:
        // 'E:\[EXTDATA8.XLS]Sheet1'!$A$7  or
        // 'E:\[EXTDATA12B.XLSB]Sheet1:Sheet3'!$A$11
        // But, 'Sheet1'!B3 would also be a valid!
        // Excel does not allow [ and ] characters in sheet names though.
        // But, more sickness comes with MOOXML as there may be
        // '[1]Sheet 4'!$A$1  where [1] is the external doc's index.
        p = lcl_ParseQuotedName(p, rExternDocName);
        if (!*p || *p != '!')
        {
            rExternDocName.Erase();
            return start;
        }
        if (rExternDocName.Len())
        {
            xub_StrLen nOpen = rExternDocName.Search( '[');
            if (nOpen == STRING_NOTFOUND)
                rExternDocName.Erase();
            else
            {
                xub_StrLen nClose = rExternDocName.Search( ']', nOpen+1);
                if (nClose == STRING_NOTFOUND)
                    rExternDocName.Erase();
                else
                {
                    rExternDocName.Erase( nClose);
                    rExternDocName.Erase( nOpen, 1);
                    pMsoxlQuoteStop = p - 1;    // the ' quote char
                    // There may be embedded escaped quotes, just matching the
                    // doc name's length may not work.
                    for (p = start; *p != '['; ++p)
                        ;
                    for ( ; *p != ']'; ++p)
                        ;
                    ++p;

                    // Handle '[1]Sheet 4'!$A$1
                    if (nOpen == 0)
                    {
                        const sal_Unicode* pErrRet = start;
                        if (!lcl_XL_getExternalDoc( &pErrRet, rExternDocName, pExternalLinks))
                            return pErrRet;
                    }
                }
            }
        }
        if (!rExternDocName.Len())
            p = start;
    }

    startTabs = p;
    p = lcl_XL_ParseSheetRef( p, rStartTabName, !bOnlyAcceptSingle, pMsoxlQuoteStop);
    if( NULL == p )
        return start;       // invalid tab
    if (bOnlyAcceptSingle && *p == ':')
        return NULL;        // 3D
    if( p != startTabs )
    {
        nFlags |= SCA_VALID_TAB | SCA_TAB_3D | SCA_TAB_ABSOLUTE;
        if( *p == ':' ) // 3d ref
        {
            p = lcl_XL_ParseSheetRef( p+1, rEndTabName, false, pMsoxlQuoteStop);
            if( p == NULL )
            {
                nFlags = nSaveFlags;
                return start; // invalid tab
            }
            nFlags |= SCA_VALID_TAB2 | SCA_TAB2_3D | SCA_TAB2_ABSOLUTE;
        }
        else
        {
            // If only one sheet is given, the full reference is still valid,
            // only the second 3D flag is not set.
            nFlags |= SCA_VALID_TAB2 | SCA_TAB2_ABSOLUTE;
            aEnd.SetTab( aStart.Tab() );
        }

        if( *p++ != '!' )
        {
            nFlags = nSaveFlags;
            return start;   // syntax error
        }
        else
            p = lcl_eatWhiteSpace( p );
    }
    else
    {
        nFlags |= SCA_VALID_TAB | SCA_VALID_TAB2;
        // Use the current tab, it needs to be passed in. : aEnd.SetTab( .. );
    }

    if (rExternDocName.Len())
    {
        ScExternalRefManager* pRefMgr = pDoc->GetExternalRefManager();
        OUString aTmp = rExternDocName;
        pRefMgr->convertToAbsName(aTmp);
        rExternDocName = aTmp;
    }
    else
    {
        // Internal reference.
        if (!rStartTabName.Len())
        {
            nFlags = nSaveFlags;
            return start;
        }

        SCTAB nTab;
        if (!pDoc->GetTable(rStartTabName, nTab))
        {
            // invalid table name.
            nFlags &= ~SCA_VALID_TAB;
            nTab = -1;
        }

        aStart.SetTab(nTab);
        aEnd.SetTab(nTab);

        if (rEndTabName.Len())
        {
            if (!pDoc->GetTable(rEndTabName, nTab))
            {
                // invalid table name.
                nFlags &= ~SCA_VALID_TAB2;
                nTab = -1;
            }

            aEnd.SetTab(nTab);
        }
    }
    return p;
}


static const sal_Unicode*
lcl_r1c1_get_col( const sal_Unicode* p,
                  const ScAddress::Details& rDetails,
                  ScAddress* pAddr, sal_uInt16* nFlags )
{
    const sal_Unicode *pEnd;
    long int n;
    bool isRelative;

    if( p[0] == '\0' )
        return NULL;

    p++;
    if( (isRelative = (*p == '[') ) != false )
        p++;
    n = sal_Unicode_strtol( p, &pEnd );
    if( NULL == pEnd )
        return NULL;

    if( p == pEnd ) // C is a relative ref with offset 0
    {
        if( isRelative )
            return NULL;
        n = rDetails.nCol;
    }
    else if( isRelative )
    {
        if( *pEnd != ']' )
            return NULL;
        n += rDetails.nCol;
        pEnd++;
    }
    else
    {
        *nFlags |= SCA_COL_ABSOLUTE;
        n--;
    }

    if( n < 0 || n >= MAXCOLCOUNT )
        return NULL;
    pAddr->SetCol( static_cast<SCCOL>( n ) );
    *nFlags |= SCA_VALID_COL;

    return pEnd;
}
static inline const sal_Unicode*
lcl_r1c1_get_row( const sal_Unicode* p,
                  const ScAddress::Details& rDetails,
                  ScAddress* pAddr, sal_uInt16* nFlags )
{
    const sal_Unicode *pEnd;
    long int n;
    bool isRelative;

    if( p[0] == '\0' )
        return NULL;

    p++;
    if( (isRelative = (*p == '[') ) != false )
        p++;
    n = sal_Unicode_strtol( p, &pEnd );
    if( NULL == pEnd )
        return NULL;

    if( p == pEnd ) // R is a relative ref with offset 0
    {
        if( isRelative )
            return NULL;
        n = rDetails.nRow;
    }
    else if( isRelative )
    {
        if( *pEnd != ']' )
            return NULL;
        n += rDetails.nRow;
        pEnd++;
    }
    else
    {
        *nFlags |= SCA_ROW_ABSOLUTE;
        n--;
    }

    if( n < 0 || n >= MAXROWCOUNT )
        return NULL;
    pAddr->SetRow( static_cast<SCROW>( n ) );
    *nFlags |= SCA_VALID_ROW;

    return pEnd;
}

static sal_uInt16
lcl_ScRange_Parse_XL_R1C1( ScRange& r,
                           const sal_Unicode* p,
                           ScDocument* pDoc,
                           const ScAddress::Details& rDetails,
                           bool bOnlyAcceptSingle,
                           ScAddress::ExternalInfo* pExtInfo )
{
    const sal_Unicode* pTmp = NULL;
    String aExternDocName, aStartTabName, aEndTabName;
    sal_uInt16 nFlags = SCA_VALID | SCA_VALID_TAB;
    // Keep in mind that nFlags2 gets left-shifted by 4 bits before being merged.
    sal_uInt16 nFlags2 = SCA_VALID_TAB;

    p = r.Parse_XL_Header( p, pDoc, aExternDocName, aStartTabName,
            aEndTabName, nFlags, bOnlyAcceptSingle, NULL );

    if (aExternDocName.Len() > 0)
        lcl_ScRange_External_TabSpan( r, nFlags, pExtInfo, aExternDocName,
                aStartTabName, aEndTabName, pDoc);

    if( NULL == p )
        return 0;

    if( *p == 'R' || *p == 'r' )
    {
        if( NULL == (p = lcl_r1c1_get_row( p, rDetails, &r.aStart, &nFlags )) )
            goto failed;

        if( *p != 'C' && *p != 'c' )    // full row R#
        {
            if( p[0] != ':' || (p[1] != 'R' && p[1] != 'r' ) ||
                NULL == (pTmp = lcl_r1c1_get_row( p+1, rDetails, &r.aEnd, &nFlags2 )))
            {
                // Only the initial row number is given, or the second row
                // number is invalid. Fallback to just the initial R
                nFlags |= (nFlags << 4);
                r.aEnd.SetRow( r.aStart.Row() );
            }
            else
            {
                // Full row range successfully parsed.
                nFlags |= (nFlags2 << 4);
                p = pTmp;
            }

            if (p && p[0] != 0)
            {
                // any trailing invalid character must invalidate the whole address.
                nFlags &= ~(SCA_VALID | SCA_VALID_COL | SCA_VALID_ROW | SCA_VALID_TAB |
                            SCA_VALID_COL2 | SCA_VALID_ROW2 | SCA_VALID_TAB2);
                return nFlags;
            }

            nFlags |=
                SCA_VALID_COL | SCA_VALID_COL2 |
                SCA_COL_ABSOLUTE | SCA_COL2_ABSOLUTE;
            r.aStart.SetCol( 0 );
            r.aEnd.SetCol( MAXCOL );

            return bOnlyAcceptSingle ? 0 : nFlags;
        }
        else if( NULL == (p = lcl_r1c1_get_col( p, rDetails, &r.aStart, &nFlags )))
            goto failed;

        if( p[0] != ':' ||
            (p[1] != 'R' && p[1] != 'r') ||
            NULL == (pTmp = lcl_r1c1_get_row( p+1, rDetails, &r.aEnd, &nFlags2 )) ||
            (*pTmp != 'C' && *pTmp != 'c') ||
            NULL == (pTmp = lcl_r1c1_get_col( pTmp, rDetails, &r.aEnd, &nFlags2 )))
        {
            // single cell reference

            if (p && p[0] != 0)
            {
                // any trailing invalid character must invalidate the whole address.
                nFlags &= ~(SCA_VALID | SCA_VALID_COL | SCA_VALID_ROW | SCA_VALID_TAB);
                return nFlags;
            }

            return bOnlyAcceptSingle ? nFlags : 0;
        }
        p = pTmp;

        // double reference

        if (p && p[0] != 0)
        {
            // any trailing invalid character must invalidate the whole range.
            nFlags &= ~(SCA_VALID | SCA_VALID_COL | SCA_VALID_ROW | SCA_VALID_TAB |
                        SCA_VALID_COL2 | SCA_VALID_ROW2 | SCA_VALID_TAB2);
            return nFlags;
        }

        nFlags |= (nFlags2 << 4);
        return bOnlyAcceptSingle ? 0 : nFlags;
    }
    else if( *p == 'C' || *p == 'c' )   // full col C#
    {
        if( NULL == (p = lcl_r1c1_get_col( p, rDetails, &r.aStart, &nFlags )))
            goto failed;

        if( p[0] != ':' || (p[1] != 'C' && p[1] != 'c') ||
            NULL == (pTmp = lcl_r1c1_get_col( p+1, rDetails, &r.aEnd, &nFlags2 )))
        {    // Fallback to just the initial C
            nFlags |= (nFlags << 4);
            r.aEnd.SetCol( r.aStart.Col() );
        }
        else
        {
            nFlags |= (nFlags2 << 4);
            p = pTmp;
        }

        if (p && p[0] != 0)
        {
            // any trailing invalid character must invalidate the whole address.
            nFlags &= ~(SCA_VALID | SCA_VALID_COL | SCA_VALID_ROW | SCA_VALID_TAB |
                        SCA_VALID_COL2 | SCA_VALID_ROW2 | SCA_VALID_TAB2);
            return nFlags;
        }

        nFlags |=
            SCA_VALID_ROW | SCA_VALID_ROW2 |
            SCA_ROW_ABSOLUTE | SCA_ROW2_ABSOLUTE;
        r.aStart.SetRow( 0 );
        r.aEnd.SetRow( MAXROW );

        return bOnlyAcceptSingle ? 0 : nFlags;
    }

failed :
    return 0;
}

static inline const sal_Unicode*
lcl_a1_get_col( const sal_Unicode* p, ScAddress* pAddr, sal_uInt16* nFlags )
{
    SCCOL nCol;

    if( *p == '$' )
        *nFlags |= SCA_COL_ABSOLUTE, p++;

    if( !CharClass::isAsciiAlpha( *p ) )
        return NULL;

    nCol = sal::static_int_cast<SCCOL>( toupper( char(*p++) ) - 'A' );
    while (nCol <= MAXCOL && CharClass::isAsciiAlpha(*p))
        nCol = sal::static_int_cast<SCCOL>( ((nCol + 1) * 26) + toupper( char(*p++) ) - 'A' );
    if( nCol > MAXCOL || CharClass::isAsciiAlpha( *p ) )
        return NULL;

    *nFlags |= SCA_VALID_COL;
    pAddr->SetCol( nCol );

    return p;
}

static inline const sal_Unicode*
lcl_a1_get_row( const sal_Unicode* p, ScAddress* pAddr, sal_uInt16* nFlags )
{
    const sal_Unicode *pEnd;
    long int n;

    if( *p == '$' )
        *nFlags |= SCA_ROW_ABSOLUTE, p++;

    n = sal_Unicode_strtol( p, &pEnd ) - 1;
    if( NULL == pEnd || p == pEnd || n < 0 || n > MAXROW )
        return NULL;

    *nFlags |= SCA_VALID_ROW;
    pAddr->SetRow( static_cast<SCROW>(n) );

    return pEnd;
}

static sal_uInt16
lcl_ScRange_Parse_XL_A1( ScRange& r,
                         const sal_Unicode* p,
                         ScDocument* pDoc,
                         bool bOnlyAcceptSingle,
                         ScAddress::ExternalInfo* pExtInfo,
                         const uno::Sequence< const sheet::ExternalLinkInfo > * pExternalLinks )
{
    const sal_Unicode* tmp1, *tmp2;
    String aExternDocName, aStartTabName, aEndTabName; // for external link table
    sal_uInt16 nFlags = SCA_VALID | SCA_VALID_TAB, nFlags2 = SCA_VALID_TAB;

    p = r.Parse_XL_Header( p, pDoc, aExternDocName, aStartTabName,
            aEndTabName, nFlags, bOnlyAcceptSingle, pExternalLinks );

    if (aExternDocName.Len() > 0)
        lcl_ScRange_External_TabSpan( r, nFlags, pExtInfo, aExternDocName,
                aStartTabName, aEndTabName, pDoc);

    if( NULL == p )
        return 0;

    tmp1 = lcl_a1_get_col( p, &r.aStart, &nFlags );
    if( tmp1 == NULL )          // Is it a row only reference 3:5
    {
        if( bOnlyAcceptSingle ) // by definition full row refs are ranges
            return 0;

        tmp1 = lcl_a1_get_row( p, &r.aStart, &nFlags );

        tmp1 = lcl_eatWhiteSpace( tmp1 );
        if( !tmp1 || *tmp1++ != ':' ) // Even a singleton requires ':' (eg 2:2)
            return 0;

        tmp1 = lcl_eatWhiteSpace( tmp1 );
        tmp2 = lcl_a1_get_row( tmp1, &r.aEnd, &nFlags2 );
        if( !tmp2 )
            return 0;

        r.aStart.SetCol( 0 ); r.aEnd.SetCol( MAXCOL );
        nFlags |=
            SCA_VALID_COL | SCA_VALID_COL2 |
            SCA_COL_ABSOLUTE | SCA_COL2_ABSOLUTE;
        nFlags |= (nFlags2 << 4);
        return nFlags;
    }

    tmp2 = lcl_a1_get_row( tmp1, &r.aStart, &nFlags );
    if( tmp2 == NULL )          // check for col only reference F:H
    {
        if( bOnlyAcceptSingle ) // by definition full col refs are ranges
            return 0;

        tmp1 = lcl_eatWhiteSpace( tmp1 );
        if( *tmp1++ != ':' )    // Even a singleton requires ':' (eg F:F)
            return 0;

        tmp1 = lcl_eatWhiteSpace( tmp1 );
        tmp2 = lcl_a1_get_col( tmp1, &r.aEnd, &nFlags2 );
        if( !tmp2 )
            return 0;

        r.aStart.SetRow( 0 ); r.aEnd.SetRow( MAXROW );
        nFlags |=
            SCA_VALID_ROW | SCA_VALID_ROW2 |
            SCA_ROW_ABSOLUTE | SCA_ROW2_ABSOLUTE;
        nFlags |= (nFlags2 << 4);
        return nFlags;
    }

    // prepare as if it's a singleton, in case we want to fall back */
    r.aEnd.SetCol( r.aStart.Col() );
    r.aEnd.SetRow( r.aStart.Row() );    // don't overwrite sheet number as parsed in Parse_XL_Header()

    if ( bOnlyAcceptSingle )
    {
        if ( *tmp2 == 0 )
            return nFlags;
        else
        {
            // any trailing invalid character must invalidate the address.
            nFlags &= ~(SCA_VALID | SCA_VALID_COL | SCA_VALID_ROW | SCA_VALID_TAB);
            return nFlags;
        }
    }

    tmp2 = lcl_eatWhiteSpace( tmp2 );
    if( *tmp2 != ':' )
    {
        // Sheet1:Sheet2!C4 is a valid range, without a second sheet it is
        // not. Any trailing invalid character invalidates the range.
        if (*tmp2 == 0 && (nFlags & SCA_TAB2_3D))
        {
            if (nFlags & SCA_COL_ABSOLUTE)
                nFlags |= SCA_COL2_ABSOLUTE;
            if (nFlags & SCA_ROW_ABSOLUTE)
                nFlags |= SCA_ROW2_ABSOLUTE;
        }
        else
            nFlags &= ~(SCA_VALID |
                    SCA_VALID_COL | SCA_VALID_ROW | SCA_VALID_TAB |
                    SCA_VALID_COL2 | SCA_VALID_ROW2 | SCA_VALID_TAB2);
        return nFlags;
    }

    p = tmp2;
    p = lcl_eatWhiteSpace( p+1 );
    tmp1 = lcl_a1_get_col( p, &r.aEnd, &nFlags2 );
    if( !tmp1 && !aEndTabName.Len() )     // Probably the aEndTabName was specified after the first range
    {
        p = lcl_XL_ParseSheetRef( p, aEndTabName, false, NULL );
        if( p )
        {
            SCTAB nTab = 0;
            if( aEndTabName.Len() && pDoc->GetTable( aEndTabName, nTab ) )
            {
                r.aEnd.SetTab( nTab );
                nFlags |= SCA_VALID_TAB2 | SCA_TAB2_3D | SCA_TAB2_ABSOLUTE;
            }
            p = lcl_eatWhiteSpace( p+1 );
            tmp1 = lcl_a1_get_col( p, &r.aEnd, &nFlags2 );
        }
    }
    if( !tmp1 ) // strange, but valid singleton
        return nFlags;

    tmp2 = lcl_a1_get_row( tmp1, &r.aEnd, &nFlags2 );
    if( !tmp2 ) // strange, but valid singleton
        return nFlags;

    if ( *tmp2 != 0 )
    {
        // any trailing invalid character must invalidate the range.
        nFlags &= ~(SCA_VALID | SCA_VALID_COL | SCA_VALID_ROW | SCA_VALID_TAB |
                    SCA_VALID_COL2 | SCA_VALID_ROW2 | SCA_VALID_TAB2);
        return nFlags;
    }

    nFlags |= (nFlags2 << 4);
    return nFlags;
}

/**
    @param pRange   pointer to range where rAddr effectively is *pRange->aEnd,
                    used in conjunction with pExtInfo to determine the tab span
                    of a 3D reference.
 */
static sal_uInt16
lcl_ScAddress_Parse_OOo( const sal_Unicode* p, ScDocument* pDoc, ScAddress& rAddr,
                         ScAddress::ExternalInfo* pExtInfo = NULL, ScRange* pRange = NULL )
{
    sal_uInt16  nRes = 0;
    OUString aDocName;       // der pure Dokumentenname
    String  aTab;
    bool    bExtDoc = false;
    bool    bExtDocInherited = false;
    const ScAddress aCurPos(rAddr);

    // Lets see if this is a reference to something in an external file.  A
    // document name is always quoted and has a trailing #.
    if (*p == '\'')
    {
        const sal_Unicode* pStart = p;
        String aTmp;
        p = lcl_ParseQuotedName(p, aTmp);
        aDocName = aTmp;
        if (*p++ == SC_COMPILER_FILE_TAB_SEP)
            bExtDoc = true;
        else
            // This is not a document name.  Perhaps a quoted relative table
            // name.
            p = pStart;
    }
    else if (pExtInfo && pExtInfo->mbExternal)
    {
        // This is an external reference.
        bExtDoc = bExtDocInherited = true;
    }

    SCCOL   nCol = 0;
    SCROW   nRow = 0;
    SCTAB   nTab = 0;
    sal_uInt16  nBits = SCA_VALID_TAB;
    const sal_Unicode* q;
    if ( ScGlobal::FindUnquoted( p, '.') )
    {
        nRes |= SCA_TAB_3D;
        if ( bExtDoc )
            nRes |= SCA_TAB_ABSOLUTE;
        if (*p == '$')
            nRes |= SCA_TAB_ABSOLUTE, p++;

        if (*p == '\'')
        {
            // Tokens that start at ' can have anything in them until a final
            // ' but '' marks an escaped '.  We've earlier guaranteed that a
            // string containing '' will be surrounded by '.
            p = lcl_ParseQuotedName(p, aTab);
        }
        else
        {
            while (*p)
            {
                if( *p == '.')
                    break;

                if( *p == '\'' )
                {
                    p++; break;
                }
                aTab += *p++;
            }
        }
        if( *p++ != '.' )
            nBits = 0;

        if (!bExtDoc && (!pDoc || !pDoc->GetTable( aTab, nTab )))
        {
            // Specified table name is not found in this document.  Assume this is an external document.
            aDocName = aTab;
            xub_StrLen n = aTab.SearchBackward('.');
            if (n != STRING_NOTFOUND && n > 0)
            {
                // Extension found.  Strip it.
                aTab.Erase(n);
                bExtDoc = true;
            }
            else
                // No extension found.  This is probably not an external document.
                nBits = 0;
        }
    }
    else
    {
        if (bExtDoc && !bExtDocInherited)
            return nRes;        // After a document a sheet must follow.
        nTab = rAddr.Tab();
    }
    nRes |= nBits;

    q = p;
    if (*p)
    {
        nBits = SCA_VALID_COL;
        if (*p == '$')
            nBits |= SCA_COL_ABSOLUTE, p++;

        if (CharClass::isAsciiAlpha( *p ))
        {
            nCol = sal::static_int_cast<SCCOL>( toupper( char(*p++) ) - 'A' );
            while (nCol < MAXCOL && CharClass::isAsciiAlpha(*p))
                nCol = sal::static_int_cast<SCCOL>( ((nCol + 1) * 26) + toupper( char(*p++) ) - 'A' );
        }
        else
            nBits = 0;

        if( nCol > MAXCOL || CharClass::isAsciiAlpha( *p ) )
            nBits = 0;
        nRes |= nBits;
        if( !nBits )
            p = q;
    }

    q = p;
    if (*p)
    {
        nBits = SCA_VALID_ROW;
        if (*p == '$')
            nBits |= SCA_ROW_ABSOLUTE, p++;
        if( !CharClass::isAsciiDigit( *p ) )
        {
            nBits = 0;
            nRow = SCROW(-1);
        }
        else
        {
            OUString aTmp( p );
            long n = aTmp.toInt32() - 1;
            while (CharClass::isAsciiDigit( *p ))
                p++;
            if( n < 0 || n > MAXROW )
                nBits = 0;
            nRow = static_cast<SCROW>(n);
        }
        nRes |= nBits;
        if( !nBits )
            p = q;
    }

    rAddr.Set( nCol, nRow, nTab );

    if (!*p && bExtDoc)
    {
        if (!pDoc)
            nRes = 0;
        else
        {
            ScExternalRefManager* pRefMgr = pDoc->GetExternalRefManager();

            // Need document name if inherited.
            if (bExtDocInherited)
            {
                const OUString* pFileName = pRefMgr->getExternalFileName( pExtInfo->mnFileId);
                if (pFileName)
                    aDocName = *pFileName;
                else
                    nRes = 0;
            }
            pRefMgr->convertToAbsName(aDocName);

            if ((!pExtInfo || !pExtInfo->mbExternal) && pRefMgr->isOwnDocument(aDocName))
            {
                if (!pDoc->GetTable( aTab, nTab ))
                    nRes = 0;
                else
                {
                    rAddr.SetTab( nTab);
                    nRes |= SCA_VALID_TAB;
                }
            }
            else
            {
                if (!pExtInfo)
                    nRes = 0;
                else
                {
                    if (!pExtInfo->mbExternal)
                    {
                        sal_uInt16 nFileId = pRefMgr->getExternalFileId(aDocName);

                        pExtInfo->mbExternal = true;
                        pExtInfo->maTabName = aTab;
                        pExtInfo->mnFileId = nFileId;

                        if (pRefMgr->getSingleRefToken(nFileId, aTab,
                                    ScAddress(nCol, nRow, 0), NULL,
                                    &nTab).get())
                        {
                            rAddr.SetTab( nTab);
                            nRes |= SCA_VALID_TAB;
                        }
                        else
                            nRes = 0;
                    }
                    else
                    {
                        // This is a call for the second part of the reference,
                        // we must have the range to adapt tab span.
                        if (!pRange)
                            nRes = 0;
                        else
                        {
                            sal_uInt16 nFlags = nRes | SCA_VALID_TAB2;
                            if (!lcl_ScRange_External_TabSpan( *pRange, nFlags,
                                        pExtInfo, aDocName,
                                        pExtInfo->maTabName, aTab, pDoc))
                                nRes &= ~SCA_VALID_TAB;
                            else
                            {
                                if (nFlags & SCA_VALID_TAB2)
                                {
                                    rAddr.SetTab( pRange->aEnd.Tab());
                                    nRes |= SCA_VALID_TAB;
                                }
                                else
                                    nRes &= ~SCA_VALID_TAB;
                            }
                        }
                    }
                }
            }
        }
    }

    if ( !(nRes & SCA_VALID_ROW) && (nRes & SCA_VALID_COL)
            && !( (nRes & SCA_TAB_3D) && (nRes & SCA_VALID_TAB)) )
    {   // no Row, no Tab, but Col => DM (...), B (...) et al
        nRes = 0;
    }
    if( !*p )
    {
        sal_uInt16 nMask = nRes & ( SCA_VALID_ROW | SCA_VALID_COL | SCA_VALID_TAB );
        if( nMask == ( SCA_VALID_ROW | SCA_VALID_COL | SCA_VALID_TAB ) )
            nRes |= SCA_VALID;
    }
    else
        nRes = 0;
    return nRes;
}

static sal_uInt16
lcl_ScAddress_Parse ( const sal_Unicode* p, ScDocument* pDoc, ScAddress& rAddr,
                      const ScAddress::Details& rDetails,
                      ScAddress::ExternalInfo* pExtInfo = NULL,
                      const uno::Sequence< const sheet::ExternalLinkInfo > * pExternalLinks = NULL )
{
    if( !*p )
        return 0;

    switch (rDetails.eConv)
    {
    default :
    case formula::FormulaGrammar::CONV_OOO:
        {
            return lcl_ScAddress_Parse_OOo( p, pDoc, rAddr, pExtInfo, NULL );
        }

    case formula::FormulaGrammar::CONV_XL_A1:
    case formula::FormulaGrammar::CONV_XL_OOX:
        {
            ScRange r = rAddr;
            sal_uInt16 nFlags = lcl_ScRange_Parse_XL_A1( r, p, pDoc, true, pExtInfo,
                    (rDetails.eConv == formula::FormulaGrammar::CONV_XL_OOX ? pExternalLinks : NULL) );
            rAddr = r.aStart;
            return nFlags;
        }
    case formula::FormulaGrammar::CONV_XL_R1C1:
        {
            ScRange r = rAddr;
            sal_uInt16 nFlags = lcl_ScRange_Parse_XL_R1C1( r, p, pDoc, rDetails, true, pExtInfo );
            rAddr = r.aStart;
            return nFlags;
        }
    }
}


bool ConvertSingleRef( ScDocument* pDoc, const String& rRefString,
                       SCTAB nDefTab, ScRefAddress& rRefAddress,
                       const ScAddress::Details& rDetails,
                       ScAddress::ExternalInfo* pExtInfo /* = NULL */ )
{
    bool bRet = false;
    if (pExtInfo || (ScGlobal::FindUnquoted( rRefString, SC_COMPILER_FILE_TAB_SEP) == STRING_NOTFOUND))
    {
        ScAddress aAddr( 0, 0, nDefTab );
        sal_uInt16 nRes = aAddr.Parse( rRefString, pDoc, rDetails, pExtInfo);
        if ( nRes & SCA_VALID )
        {
            rRefAddress.Set( aAddr,
                    ((nRes & SCA_COL_ABSOLUTE) == 0),
                    ((nRes & SCA_ROW_ABSOLUTE) == 0),
                    ((nRes & SCA_TAB_ABSOLUTE) == 0));
            bRet = true;
        }
    }
    return bRet;
}


bool ConvertDoubleRef( ScDocument* pDoc, const String& rRefString, SCTAB nDefTab,
                       ScRefAddress& rStartRefAddress, ScRefAddress& rEndRefAddress,
                       const ScAddress::Details& rDetails,
                       ScAddress::ExternalInfo* pExtInfo /* = NULL */ )
{
    bool bRet = false;
    if (pExtInfo || (ScGlobal::FindUnquoted( rRefString, SC_COMPILER_FILE_TAB_SEP) == STRING_NOTFOUND))
    {
        ScRange aRange( ScAddress( 0, 0, nDefTab));
        sal_uInt16 nRes = aRange.Parse( rRefString, pDoc, rDetails, pExtInfo);
        if ( nRes & SCA_VALID )
        {
            rStartRefAddress.Set( aRange.aStart,
                    ((nRes & SCA_COL_ABSOLUTE) == 0),
                    ((nRes & SCA_ROW_ABSOLUTE) == 0),
                    ((nRes & SCA_TAB_ABSOLUTE) == 0));
            rEndRefAddress.Set( aRange.aEnd,
                    ((nRes & SCA_COL2_ABSOLUTE) == 0),
                    ((nRes & SCA_ROW2_ABSOLUTE) == 0),
                    ((nRes & SCA_TAB2_ABSOLUTE) == 0));
            bRet = true;
        }
    }
    return bRet;
}


sal_uInt16 ScAddress::Parse( const String& r, ScDocument* pDoc,
                         const Details& rDetails,
                         ExternalInfo* pExtInfo,
                         const uno::Sequence< const sheet::ExternalLinkInfo > * pExternalLinks )
{
    return lcl_ScAddress_Parse( r.GetBuffer(), pDoc, *this, rDetails, pExtInfo, pExternalLinks );
}


bool ScRange::Intersects( const ScRange& r ) const
{
    return !(
        std::min( aEnd.Col(), r.aEnd.Col() ) < std::max( aStart.Col(), r.aStart.Col() )
     || std::min( aEnd.Row(), r.aEnd.Row() ) < std::max( aStart.Row(), r.aStart.Row() )
     || std::min( aEnd.Tab(), r.aEnd.Tab() ) < std::max( aStart.Tab(), r.aStart.Tab() )
        );
}

void ScRange::PutInOrder()
{
    SCCOL nCol1 = aStart.Col(), nCol2 = aEnd.Col();
    SCROW nRow1 = aStart.Row(), nRow2 = aEnd.Row();
    SCTAB nTab1 = aStart.Tab(), nTab2 = aEnd.Tab();

    ::PutInOrder(nCol1, nCol2);
    ::PutInOrder(nRow1, nRow2);
    ::PutInOrder(nTab1, nTab2);

    aStart.SetCol(nCol1);
    aStart.SetRow(nRow1);
    aStart.SetTab(nTab1);

    aEnd.SetCol(nCol2);
    aEnd.SetRow(nRow2);
    aEnd.SetTab(nTab2);
}

void ScRange::Justify()
{
    SCCOL nTempCol;
    if ( aEnd.Col() < (nTempCol = aStart.Col()) )
    {
        aStart.SetCol(aEnd.Col()); aEnd.SetCol(nTempCol);
    }
    SCROW nTempRow;
    if ( aEnd.Row() < (nTempRow = aStart.Row()) )
    {
        aStart.SetRow(aEnd.Row()); aEnd.SetRow(nTempRow);
    }
    SCTAB nTempTab;
    if ( aEnd.Tab() < (nTempTab = aStart.Tab()) )
    {
        aStart.SetTab(aEnd.Tab()); aEnd.SetTab(nTempTab);
    }
}

void ScRange::ExtendTo( const ScRange& rRange )
{
    OSL_ENSURE( rRange.IsValid(), "ScRange::ExtendTo - cannot extend to invalid range" );
    if( IsValid() )
    {
        aStart.SetCol( ::std::min( aStart.Col(), rRange.aStart.Col() ) );
        aStart.SetRow( ::std::min( aStart.Row(), rRange.aStart.Row() ) );
        aStart.SetTab( ::std::min( aStart.Tab(), rRange.aStart.Tab() ) );
        aEnd.SetCol(   ::std::max( aEnd.Col(),   rRange.aEnd.Col() ) );
        aEnd.SetRow(   ::std::max( aEnd.Row(),   rRange.aEnd.Row() ) );
        aEnd.SetTab(   ::std::max( aEnd.Tab(),   rRange.aEnd.Tab() ) );
    }
    else
        *this = rRange;
}

static sal_uInt16
lcl_ScRange_Parse_OOo( ScRange &aRange, const String& r, ScDocument* pDoc, ScAddress::ExternalInfo* pExtInfo = NULL )
{
    sal_uInt16 nRes1 = 0, nRes2 = 0;
    xub_StrLen nPos = ScGlobal::FindUnquoted( r, ':');
    if (nPos != STRING_NOTFOUND)
    {
        String aTmp( r );
        sal_Unicode* p = aTmp.GetBufferAccess();
        p[ nPos ] = 0;
        if( (nRes1 = lcl_ScAddress_Parse_OOo( p, pDoc, aRange.aStart, pExtInfo, NULL ) ) != 0 )
        {
            aRange.aEnd = aRange.aStart;  // sheet must be initialized identical to first sheet
            if ( (nRes2 = lcl_ScAddress_Parse_OOo( p + nPos+ 1, pDoc, aRange.aEnd, pExtInfo, &aRange ) ) != 0 )
            {
                // PutInOrder / Justify
                sal_uInt16 nMask, nBits1, nBits2;
                SCCOL nTempCol;
                if ( aRange.aEnd.Col() < (nTempCol = aRange.aStart.Col()) )
                {
                    aRange.aStart.SetCol(aRange.aEnd.Col()); aRange.aEnd.SetCol(nTempCol);
                    nMask = (SCA_VALID_COL | SCA_COL_ABSOLUTE);
                    nBits1 = nRes1 & nMask;
                    nBits2 = nRes2 & nMask;
                    nRes1 = (nRes1 & ~nMask) | nBits2;
                    nRes2 = (nRes2 & ~nMask) | nBits1;
                }
                SCROW nTempRow;
                if ( aRange.aEnd.Row() < (nTempRow = aRange.aStart.Row()) )
                {
                    aRange.aStart.SetRow(aRange.aEnd.Row()); aRange.aEnd.SetRow(nTempRow);
                    nMask = (SCA_VALID_ROW | SCA_ROW_ABSOLUTE);
                    nBits1 = nRes1 & nMask;
                    nBits2 = nRes2 & nMask;
                    nRes1 = (nRes1 & ~nMask) | nBits2;
                    nRes2 = (nRes2 & ~nMask) | nBits1;
                }
                SCTAB nTempTab;
                if ( aRange.aEnd.Tab() < (nTempTab = aRange.aStart.Tab()) )
                {
                    aRange.aStart.SetTab(aRange.aEnd.Tab()); aRange.aEnd.SetTab(nTempTab);
                    nMask = (SCA_VALID_TAB | SCA_TAB_ABSOLUTE | SCA_TAB_3D);
                    nBits1 = nRes1 & nMask;
                    nBits2 = nRes2 & nMask;
                    nRes1 = (nRes1 & ~nMask) | nBits2;
                    nRes2 = (nRes2 & ~nMask) | nBits1;
                }
                if ( ((nRes1 & ( SCA_TAB_ABSOLUTE | SCA_TAB_3D ))
                        == ( SCA_TAB_ABSOLUTE | SCA_TAB_3D ))
                        && !(nRes2 & SCA_TAB_3D) )
                    nRes2 |= SCA_TAB_ABSOLUTE;
            }
            else
                nRes1 = 0;      // keine Tokens aus halben Sachen
        }
    }
    nRes1 = ( ( nRes1 | nRes2 ) & SCA_VALID )
          | nRes1
          | ( ( nRes2 & SCA_BITS ) << 4 );
    return nRes1;
}

sal_uInt16 ScRange::Parse( const String& r, ScDocument* pDoc,
                       const ScAddress::Details& rDetails,
                       ScAddress::ExternalInfo* pExtInfo,
                       const uno::Sequence< const sheet::ExternalLinkInfo > * pExternalLinks )
{
    if ( r.Len() <= 0 )
        return 0;

    switch (rDetails.eConv)
    {
    default :
    case formula::FormulaGrammar::CONV_OOO:
        return lcl_ScRange_Parse_OOo( *this, r, pDoc, pExtInfo );

    case formula::FormulaGrammar::CONV_XL_A1:
    case formula::FormulaGrammar::CONV_XL_OOX:
        return lcl_ScRange_Parse_XL_A1( *this, r.GetBuffer(), pDoc, false, pExtInfo,
                (rDetails.eConv == formula::FormulaGrammar::CONV_XL_OOX ? pExternalLinks : NULL) );

    case formula::FormulaGrammar::CONV_XL_R1C1:
        return lcl_ScRange_Parse_XL_R1C1( *this, r.GetBuffer(), pDoc, rDetails, false, pExtInfo );
    }
}


// Accept a full range, or an address
sal_uInt16 ScRange::ParseAny( const String& r, ScDocument* pDoc,
                          const ScAddress::Details& rDetails )
{
    sal_uInt16 nRet = Parse( r, pDoc, rDetails );
    const sal_uInt16 nValid = SCA_VALID | SCA_VALID_COL2 | SCA_VALID_ROW2 |
        SCA_VALID_TAB2;

    if ( (nRet & nValid) != nValid )
    {
        ScAddress aAdr(aStart);//initialize with currentPos as fallback for table number
        nRet = aAdr.Parse( r, pDoc, rDetails );
        if ( nRet & SCA_VALID )
            aStart = aEnd = aAdr;
    }
    return nRet;
}

// Parse only full row references
sal_uInt16 ScRange::ParseCols( const String& rStr, ScDocument* pDoc,
                           const ScAddress::Details& rDetails )
{
    const sal_Unicode* p = rStr.GetBuffer();
    sal_uInt16 nRes = 0, ignored = 0;

    if( NULL == p )
        return 0;

    (void)pDoc; // make compiler shutup we may need this later

    switch (rDetails.eConv)
    {
    default :
    case formula::FormulaGrammar::CONV_OOO: // No full col refs in OOO yet, assume XL notation
    case formula::FormulaGrammar::CONV_XL_A1:
    case formula::FormulaGrammar::CONV_XL_OOX:
        if (NULL != (p = lcl_a1_get_col( p, &aStart, &ignored ) ) )
        {
            if( p[0] == ':')
            {
                if( NULL != (p = lcl_a1_get_col( p+1, &aEnd, &ignored )))
                {
                    nRes = SCA_VALID_COL;
                }
            }
            else
            {
                aEnd = aStart;
                nRes = SCA_VALID_COL;
            }
        }
        break;

    case formula::FormulaGrammar::CONV_XL_R1C1:
        if ((p[0] == 'C' || p[0] != 'c') &&
            NULL != (p = lcl_r1c1_get_col( p, rDetails, &aStart, &ignored )))
        {
            if( p[0] == ':')
            {
                if( (p[1] == 'C' || p[1] == 'c') &&
                    NULL != (p = lcl_r1c1_get_col( p+1, rDetails, &aEnd, &ignored )))
                {
                    nRes = SCA_VALID_COL;
                }
            }
            else
            {
                aEnd = aStart;
                nRes = SCA_VALID_COL;
            }
        }
        break;
    }

    return (p != NULL && *p == '\0') ? nRes : 0;
}

// Parse only full row references
sal_uInt16 ScRange::ParseRows( const String& rStr, ScDocument* pDoc,
                           const ScAddress::Details& rDetails )
{
    const sal_Unicode* p = rStr.GetBuffer();
    sal_uInt16 nRes = 0, ignored = 0;

    if( NULL == p )
        return 0;

    (void)pDoc; // make compiler shutup we may need this later

    switch (rDetails.eConv)
    {
    default :
    case formula::FormulaGrammar::CONV_OOO: // No full row refs in OOO yet, assume XL notation
    case formula::FormulaGrammar::CONV_XL_A1:
    case formula::FormulaGrammar::CONV_XL_OOX:
        if (NULL != (p = lcl_a1_get_row( p, &aStart, &ignored ) ) )
        {
            if( p[0] == ':')
            {
                if( NULL != (p = lcl_a1_get_row( p+1, &aEnd, &ignored )))
                {
                    nRes = SCA_VALID_COL;
                }
            }
            else
            {
                aEnd = aStart;
                nRes = SCA_VALID_COL;
            }
        }
        break;

    case formula::FormulaGrammar::CONV_XL_R1C1:
        if ((p[0] == 'R' || p[0] != 'r') &&
            NULL != (p = lcl_r1c1_get_row( p, rDetails, &aStart, &ignored )))
        {
            if( p[0] == ':')
            {
                if( (p[1] == 'R' || p[1] == 'r') &&
                    NULL != (p = lcl_r1c1_get_row( p+1, rDetails, &aEnd, &ignored )))
                {
                    nRes = SCA_VALID_COL;
                }
            }
            else
            {
                aEnd = aStart;
                nRes = SCA_VALID_COL;
            }
        }
        break;
    }

    return (p != NULL && *p == '\0') ? nRes : 0;
}

static inline void
lcl_a1_append_c ( String &r, int nCol, bool bIsAbs )
{
    if( bIsAbs )
        r += '$';
    ScColToAlpha( r, sal::static_int_cast<SCCOL>(nCol) );
}

static inline void
lcl_a1_append_r ( String &r, int nRow, bool bIsAbs )
{
    if ( bIsAbs )
        r += '$';
    r += OUString::number( nRow+1 );
}

static inline void
lcl_r1c1_append_c ( String &r, int nCol, bool bIsAbs,
                    const ScAddress::Details& rDetails )
{
    r += 'C';
    if (bIsAbs)
    {
        r += OUString::number( nCol + 1 );
    }
    else
    {
        nCol -= rDetails.nCol;
        if (nCol != 0) {
            r += "[" + OUString::number( nCol ) + "]";
        }
    }
}
static inline void
lcl_r1c1_append_r ( String &r, int nRow, bool bIsAbs,
                    const ScAddress::Details& rDetails )
{
    r += 'R';
    if (bIsAbs)
    {
        r += OUString::number( nRow + 1 );
    }
    else
    {
        nRow -= rDetails.nRow;
        if (nRow != 0) {
            r += "[" + OUString::number( nRow ) + "]";
        }
    }
}

static String
getFileNameFromDoc( const ScDocument* pDoc )
{
    // TODO : er points at ScGlobal::GetAbsDocName()
    // as a better template.  Look into it
    String sFileName;
    SfxObjectShell* pShell;

    if( NULL != pDoc &&
        NULL != (pShell = pDoc->GetDocumentShell() ) )
    {
        uno::Reference< frame::XModel > xModel( pShell->GetModel(), uno::UNO_QUERY );
        if( xModel.is() )
        {
            if( !xModel->getURL().isEmpty() )
            {
                INetURLObject aURL( xModel->getURL() );
                sFileName = aURL.GetLastName();
            }
            else
                sFileName = pShell->GetTitle();
        }
    }
    return sFileName;
}

void ScAddress::Format( OUString& r, sal_uInt16 nFlags, const ScDocument* pDoc,
                        const Details& rDetails) const
{
    String aStr;
    Format(aStr, nFlags, pDoc, rDetails);
    r = aStr;
}

void ScAddress::Format( String& r, sal_uInt16 nFlags, const ScDocument* pDoc,
                        const Details& rDetails) const
{
    r.Erase();
    if( nFlags & SCA_VALID )
        nFlags |= ( SCA_VALID_ROW | SCA_VALID_COL | SCA_VALID_TAB );
    if( pDoc && (nFlags & SCA_VALID_TAB ) )
    {
        if ( nTab >= pDoc->GetTableCount() )
        {
            r = ScGlobal::GetRscString( STR_NOREF_STR );
            return;
        }
        if( nFlags & SCA_TAB_3D )
        {
            String aTabName, aDocName;
            OUString aTmp;
            pDoc->GetName(nTab, aTmp);
            aTabName = aTmp; // TODO: remove use of String here.
            // External Reference, same as in ScCompiler::MakeTabStr()
            if( aTabName.GetChar(0) == '\'' )
            {   // "'Doc'#Tab"
                xub_StrLen nPos = ScCompiler::GetDocTabPos( aTabName);
                if (nPos != STRING_NOTFOUND)
                {
                    aDocName = aTabName.Copy( 0, nPos + 1 );
                    aTabName.Erase( 0, nPos + 1 );
                }
            }
            else if( nFlags & SCA_FORCE_DOC )
            {
                // VBA has an 'external' flag that forces the addition of the
                // tab name _and_ the doc name.  The VBA code would be
                // needlessly complicated if it constructed an actual external
                // reference so we add this somewhat cheesy kludge to force the
                // addition of the document name even for non-external references
                aDocName = getFileNameFromDoc( pDoc );
            }
            ScCompiler::CheckTabQuotes( aTabName, rDetails.eConv);

            switch( rDetails.eConv )
            {
            default :
            case formula::FormulaGrammar::CONV_OOO:
                r += aDocName;
                if( nFlags & SCA_TAB_ABSOLUTE )
                    r += '$';
                r += aTabName;
                r += '.';
                break;

            case formula::FormulaGrammar::CONV_XL_A1:
            case formula::FormulaGrammar::CONV_XL_R1C1:
            case formula::FormulaGrammar::CONV_XL_OOX:
                if (aDocName.Len() > 0)
                {
                    r += '[';
                    r += aDocName;
                    r += ']';
                }
                r += aTabName;
                r += '!';
                break;
            }
        }
    }
    switch( rDetails.eConv )
    {
    default :
    case formula::FormulaGrammar::CONV_OOO:
    case formula::FormulaGrammar::CONV_XL_A1:
    case formula::FormulaGrammar::CONV_XL_OOX:
        if( nFlags & SCA_VALID_COL )
            lcl_a1_append_c ( r, nCol, nFlags & SCA_COL_ABSOLUTE );
        if( nFlags & SCA_VALID_ROW )
            lcl_a1_append_r ( r, nRow, nFlags & SCA_ROW_ABSOLUTE );
        break;

    case formula::FormulaGrammar::CONV_XL_R1C1:
        if( nFlags & SCA_VALID_ROW )
            lcl_r1c1_append_r ( r, nRow, nFlags & SCA_ROW_ABSOLUTE, rDetails );
        if( nFlags & SCA_VALID_COL )
            lcl_r1c1_append_c ( r, nCol, nFlags & SCA_COL_ABSOLUTE, rDetails );
        break;
    }
}

static void
lcl_Split_DocTab( const ScDocument* pDoc,  SCTAB nTab,
                  const ScAddress::Details& rDetails,
                  sal_uInt16 nFlags,
                  String& rTabName, String& rDocName )
{
    OUString aTmp;
    pDoc->GetName(nTab, aTmp);
    rTabName = aTmp;
    rDocName.Erase();
    // External reference, same as in ScCompiler::MakeTabStr()
    if ( rTabName.GetChar(0) == '\'' )
    {   // "'Doc'#Tab"
        xub_StrLen nPos = ScCompiler::GetDocTabPos( rTabName);
        if (nPos != STRING_NOTFOUND)
        {
            rDocName = rTabName.Copy( 0, nPos + 1 );
            rTabName.Erase( 0, nPos + 1 );
        }
    }
    else if( nFlags & SCA_FORCE_DOC )
    {
        // VBA has an 'external' flag that forces the addition of the
        // tab name _and_ the doc name.  The VBA code would be
        // needlessly complicated if it constructed an actual external
        // reference so we add this somewhat cheesy kludge to force the
        // addition of the document name even for non-external references
        rDocName = getFileNameFromDoc( pDoc );
    }
    ScCompiler::CheckTabQuotes( rTabName, rDetails.eConv);
}

static void
lcl_ScRange_Format_XL_Header( String& r, const ScRange& rRange,
                              sal_uInt16 nFlags, const ScDocument* pDoc,
                              const ScAddress::Details& rDetails )
{
    if( nFlags & SCA_TAB_3D )
    {
        String aTabName, aDocName;
        lcl_Split_DocTab( pDoc, rRange.aStart.Tab(), rDetails, nFlags,
                          aTabName, aDocName );
        if( aDocName.Len() > 0 )
        {
            r += '[';
            r += aDocName;
            r += ']';
        }
        r += aTabName;

        if( nFlags & SCA_TAB2_3D )
        {
            lcl_Split_DocTab( pDoc, rRange.aEnd.Tab(), rDetails, nFlags,
                              aTabName, aDocName );
            r += ':';
            r += aTabName;
        }
        r += '!';
    }
}

void ScRange::Format( String& r, sal_uInt16 nFlags, const ScDocument* pDoc,
                      const ScAddress::Details& rDetails ) const
{
    r.Erase();
    if( !( nFlags & SCA_VALID ) )
    {
        r = ScGlobal::GetRscString( STR_NOREF_STR );
        return;
    }

#define absrel_differ(nFlags, mask) (((nFlags) & (mask)) ^ (((nFlags) >> 4) & (mask)))
    switch( rDetails.eConv ) {
    default :
    case formula::FormulaGrammar::CONV_OOO: {
        bool bOneTab = (aStart.Tab() == aEnd.Tab());
        if ( !bOneTab )
            nFlags |= SCA_TAB_3D;
        aStart.Format( r, nFlags, pDoc, rDetails );
        if( aStart != aEnd ||
            absrel_differ( nFlags, SCA_COL_ABSOLUTE ) ||
            absrel_differ( nFlags, SCA_ROW_ABSOLUTE ))
        {
            String aName;
            nFlags = ( nFlags & SCA_VALID ) | ( ( nFlags >> 4 ) & 0x070F );
            if ( bOneTab )
                pDoc = NULL;
            else
                nFlags |= SCA_TAB_3D;
            aEnd.Format( aName, nFlags, pDoc, rDetails );
            r += ':';
            r += aName;
        }
    }
    break;

    case formula::FormulaGrammar::CONV_XL_A1:
    case formula::FormulaGrammar::CONV_XL_OOX:
        lcl_ScRange_Format_XL_Header( r, *this, nFlags, pDoc, rDetails );
        if( aStart.Col() == 0 && aEnd.Col() >= MAXCOL )
        {
            // Full col refs always require 2 rows (2:2)
            lcl_a1_append_r( r, aStart.Row(), nFlags & SCA_ROW_ABSOLUTE );
            r += ':';
            lcl_a1_append_r( r, aEnd.Row(), nFlags & SCA_ROW2_ABSOLUTE );
        }
        else if( aStart.Row() == 0 && aEnd.Row() >= MAXROW )
        {
            // Full row refs always require 2 cols (A:A)
            lcl_a1_append_c( r, aStart.Col(), nFlags & SCA_COL_ABSOLUTE );
            r += ':';
            lcl_a1_append_c( r, aEnd.Col(), nFlags & SCA_COL2_ABSOLUTE );
        }
        else
        {
            lcl_a1_append_c ( r, aStart.Col(), nFlags & SCA_COL_ABSOLUTE );
            lcl_a1_append_r ( r, aStart.Row(), nFlags & SCA_ROW_ABSOLUTE );
            if( aStart.Col() != aEnd.Col() ||
                absrel_differ( nFlags, SCA_COL_ABSOLUTE ) ||
                aStart.Row() != aEnd.Row() ||
                absrel_differ( nFlags, SCA_ROW_ABSOLUTE )) {
                r += ':';
                lcl_a1_append_c ( r, aEnd.Col(), nFlags & SCA_COL2_ABSOLUTE );
                lcl_a1_append_r ( r, aEnd.Row(), nFlags & SCA_ROW2_ABSOLUTE );
            }
        }
    break;

    case formula::FormulaGrammar::CONV_XL_R1C1:
        lcl_ScRange_Format_XL_Header( r, *this, nFlags, pDoc, rDetails );
        if( aStart.Col() == 0 && aEnd.Col() >= MAXCOL )
        {
            lcl_r1c1_append_r( r, aStart.Row(), nFlags & SCA_ROW_ABSOLUTE, rDetails );
            if( aStart.Row() != aEnd.Row() ||
                absrel_differ( nFlags, SCA_ROW_ABSOLUTE )) {
                r += ':';
                lcl_r1c1_append_r( r, aEnd.Row(), nFlags & SCA_ROW2_ABSOLUTE, rDetails );
            }
        }
        else if( aStart.Row() == 0 && aEnd.Row() >= MAXROW )
        {
            lcl_r1c1_append_c( r, aStart.Col(), nFlags & SCA_COL_ABSOLUTE, rDetails );
            if( aStart.Col() != aEnd.Col() ||
                absrel_differ( nFlags, SCA_COL_ABSOLUTE )) {
                r += ':';
                lcl_r1c1_append_c( r, aEnd.Col(), nFlags & SCA_COL2_ABSOLUTE, rDetails );
            }
        }
        else
        {
            lcl_r1c1_append_r( r, aStart.Row(), nFlags & SCA_ROW_ABSOLUTE, rDetails );
            lcl_r1c1_append_c( r, aStart.Col(), nFlags & SCA_COL_ABSOLUTE, rDetails );
            if( aStart.Col() != aEnd.Col() ||
                absrel_differ( nFlags, SCA_COL_ABSOLUTE ) ||
                aStart.Row() != aEnd.Row() ||
                absrel_differ( nFlags, SCA_ROW_ABSOLUTE )) {
                r += ':';
                lcl_r1c1_append_r( r, aEnd.Row(), nFlags & SCA_ROW2_ABSOLUTE, rDetails );
                lcl_r1c1_append_c( r, aEnd.Col(), nFlags & SCA_COL2_ABSOLUTE, rDetails );
            }
        }
    }
#undef  absrel_differ
}

void ScRange::Format( OUString& r, sal_uInt16 nFlags, const ScDocument* pDoc,
                      const ScAddress::Details& rDetails ) const
{
    String aStr;
    Format(aStr, nFlags, pDoc, rDetails);
    r = aStr;
}

bool ScAddress::Move( SCsCOL dx, SCsROW dy, SCsTAB dz, ScDocument* pDoc )
{
    SCsTAB nMaxTab = pDoc ? pDoc->GetTableCount() : MAXTAB;
    dx = Col() + dx;
    dy = Row() + dy;
    dz = Tab() + dz;
    bool bValid = true;
    if( dx < 0 )
        dx = 0, bValid = false;
    else if( dx > MAXCOL )
        dx = MAXCOL, bValid =false;
    if( dy < 0 )
        dy = 0, bValid = false;
    else if( dy > MAXROW )
        dy = MAXROW, bValid =false;
    if( dz < 0 )
        dz = 0, bValid = false;
    else if( dz > nMaxTab )
        dz = nMaxTab, bValid =false;
    Set( dx, dy, dz );
    return bValid;
}


bool ScRange::Move( SCsCOL dx, SCsROW dy, SCsTAB dz, ScDocument* pDoc )
{
    // single & to process both
    return aStart.Move( dx, dy, dz, pDoc ) & aEnd.Move( dx, dy, dz, pDoc );
}


String ScAddress::GetColRowString( bool bAbsolute,
                                   const Details& rDetails ) const
{
    String aString;

    switch( rDetails.eConv )
    {
    default :
    case formula::FormulaGrammar::CONV_OOO:
    case formula::FormulaGrammar::CONV_XL_A1:
    case formula::FormulaGrammar::CONV_XL_OOX:
    if (bAbsolute)
        aString.Append( '$' );

    ScColToAlpha( aString, nCol);

    if ( bAbsolute )
        aString.Append( '$' );

    aString += OUString::number(nRow+1);
        break;

    case formula::FormulaGrammar::CONV_XL_R1C1:
        lcl_r1c1_append_r ( aString, nRow, bAbsolute, rDetails );
        lcl_r1c1_append_c ( aString, nCol, bAbsolute, rDetails );
        break;
    }

    return aString;
}


String ScRefAddress::GetRefString( ScDocument* pDoc, SCTAB nActTab,
                                   const ScAddress::Details& rDetails ) const
{
    if ( !pDoc )
        return EMPTY_STRING;
    if ( Tab()+1 > pDoc->GetTableCount() )
        return ScGlobal::GetRscString( STR_NOREF_STR );

    String aString;
    sal_uInt16 nFlags = SCA_VALID;
    if ( nActTab != Tab() )
    {
        nFlags |= SCA_TAB_3D;
        if ( !bRelTab )
            nFlags |= SCA_TAB_ABSOLUTE;
    }
    if ( !bRelCol )
        nFlags |= SCA_COL_ABSOLUTE;
    if ( !bRelRow )
        nFlags |= SCA_ROW_ABSOLUTE;

    aAdr.Format( aString, nFlags, pDoc, rDetails );

    return aString;
}

//------------------------------------------------------------------------

void ScColToAlpha( OUStringBuffer& rBuf, SCCOL nCol )
{
    if (nCol < 26*26)
    {
        if (nCol < 26)
            rBuf.append( static_cast<sal_Unicode>( 'A' +
                        static_cast<sal_uInt16>(nCol)));
        else
        {
            rBuf.append( static_cast<sal_Unicode>( 'A' +
                        (static_cast<sal_uInt16>(nCol) / 26) - 1));
            rBuf.append( static_cast<sal_Unicode>( 'A' +
                        (static_cast<sal_uInt16>(nCol) % 26)));
        }
    }
    else
    {
        String aStr;
        while (nCol >= 26)
        {
            SCCOL nC = nCol % 26;
            aStr += static_cast<sal_Unicode>( 'A' +
                    static_cast<sal_uInt16>(nC));
            nCol = sal::static_int_cast<SCCOL>( nCol - nC );
            nCol = nCol / 26 - 1;
        }
        aStr += static_cast<sal_Unicode>( 'A' +
                static_cast<sal_uInt16>(nCol));
        rBuf.append(comphelper::string::reverseString(aStr));
    }
}


bool AlphaToCol( SCCOL& rCol, const String& rStr)
{
    SCCOL nResult = 0;
    xub_StrLen nStop = rStr.Len();
    xub_StrLen nPos = 0;
    sal_Unicode c;
    while (nResult <= MAXCOL && nPos < nStop && (c = rStr.GetChar( nPos)) != 0 &&
            CharClass::isAsciiAlpha(c))
    {
        if (nPos > 0)
            nResult = (nResult + 1) * 26;
        nResult += ScGlobal::ToUpperAlpha(c) - 'A';
        ++nPos;
    }
    bool bOk = (ValidCol(nResult) && nPos > 0);
    if (bOk)
        rCol = nResult;
    return bOk;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
