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

#include <sal/config.h>

#include <string_view>

#include <address.hxx>
#include <global.hxx>
#include <compiler.hxx>
#include <document.hxx>
#include <externalrefmgr.hxx>

#include <osl/diagnose.h>
#include <o3tl/underlyingenumvalue.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/sheet/ExternalLinkInfo.hpp>
#include <com/sun/star/sheet/ExternalLinkType.hpp>
#include <sfx2/objsh.hxx>
#include <tools/urlobj.hxx>
#include <sal/log.hxx>
#include <rtl/character.hxx>
#include <unotools/charclass.hxx>

using namespace css;

const ScAddress::Details ScAddress::detailsOOOa1( formula::FormulaGrammar::CONV_OOO, 0, 0 );

ScAddress::Details::Details ( const ScDocument* pDoc,
                              const ScAddress & rAddr ) :
    eConv( pDoc->GetAddressConvention() ),
    nRow( rAddr.Row() ),
    nCol( rAddr.Col() )
{}

namespace {

const sal_Unicode* parseQuotedNameWithBuffer( const sal_Unicode* pStart, const sal_Unicode* p, OUString& rName )
{
    // The current character must be on the 2nd quote.

    // Push all the characters up to the current, but skip the very first
    // character which is the opening quote.
    OUStringBuffer aBuf(OUString(pStart+1, p-pStart-1));

    ++p; // Skip the 2nd quote.
    sal_Unicode cPrev = 0;
    for (; *p; ++p)
    {
        if (*p == '\'')
        {
            if (cPrev == '\'')
            {
                // double single-quote equals one single quote.
                aBuf.append(*p);
                cPrev = 0;
                continue;
            }
        }
        else if (cPrev == '\'')
        {
            // We are past the closing quote.  We're done!
            rName = aBuf.makeStringAndClear();
            return p;
        }
        else
            aBuf.append(*p);
        cPrev = *p;
    }

    return pStart;
}

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
const sal_Unicode* parseQuotedName( const sal_Unicode* p, OUString& rName )
{
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
                return parseQuotedNameWithBuffer(pStart, p, rName);
            }
        }
        else if (cPrev == '\'')
        {
            // We are past the closing quote.  We're done!  Skip the opening
            // and closing quotes.
            rName = OUString(pStart+1, p - pStart-2);
            return p;
        }

        cPrev = *p;
    }

    rName.clear();
    return pStart;
}

}

static long int sal_Unicode_strtol ( const sal_Unicode*  p, const sal_Unicode** pEnd )
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

    while (rtl::isAsciiDigit( *p ))
    {
        accum = accum * 10 + *p - '0';
        if( accum < prev )
        {
            *pEnd = nullptr;
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

// Compare ignore case ASCII.
static bool lcl_isString( const sal_Unicode* p1, const OUString& rStr )
{
    const size_t n = rStr.getLength();
    if (!n)
        return false;
    const sal_Unicode* p2 = rStr.getStr();
    for (size_t i=0; i<n; ++i)
    {
        if (!p1[i])
            return false;
        if (p1[i] != p2[i])
        {
            sal_Unicode c1 = p1[i];
            if ('A' <= c1 && c1 <= 'Z')
                c1 += 0x20;
            if (c1 < 'a' || 'z' < c1)
                return false;   // not a letter

            sal_Unicode c2 = p2[i];
            if ('A' <= c2 && c2 <= 'Z')
                c2 += 0x20;
            if (c2 < 'a' || 'z' < c2)
                return false;   // not a letter to match

            if (c1 != c2)
                return false;   // lower case doesn't match either
        }
    }
    return true;
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
                            ScRefFlags & rFlags,
                            ScAddress::ExternalInfo* pExtInfo,
                            const OUString & rExternDocName,
                            const OUString & rStartTabName,
                            const OUString & rEndTabName,
                            const ScDocument* pDoc )
{
    if (rExternDocName.isEmpty())
        return !pExtInfo || !pExtInfo->mbExternal;

    ScExternalRefManager* pRefMgr = pDoc->GetExternalRefManager();
    if (pRefMgr->isOwnDocument( rExternDocName))
    {
        // This is an internal document.  Get the sheet positions from the
        // ScDocument instance.
        if (!rStartTabName.isEmpty())
        {
            SCTAB nTab;
            if (pDoc->GetTable(rStartTabName, nTab))
                rRange.aStart.SetTab(nTab);
        }

        if (!rEndTabName.isEmpty())
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

    if (rEndTabName.isEmpty() || rStartTabName == rEndTabName)
    {
        rRange.aEnd.SetTab( rRange.aStart.Tab());
        return true;
    }

    SCTAB nSpan = pRefMgr->getCachedTabSpan( nFileId, rStartTabName, rEndTabName);
    if (nSpan == -1)
        rFlags &= ~ScRefFlags(ScRefFlags::TAB_VALID | ScRefFlags::TAB2_VALID);
    else if (nSpan == 0)
        rFlags &= ~ScRefFlags::TAB2_VALID;
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
static const sal_Unicode * lcl_XL_ParseSheetRef( const sal_Unicode* start,
                                                 OUString& rExternTabName,
                                                 bool bAllow3D,
                                                 const sal_Unicode* pMsoxlQuoteStop,
                                                 const OUString* pErrRef )
{
    OUString aTabName;
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
                    aTabName += OUString( pCurrentStart,
                            sal::static_int_cast<sal_Int32>( p - pCurrentStart));
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
            aTabName += OUString( pCurrentStart, sal::static_int_cast<sal_Int32>( p - pCurrentStart));
        if (aTabName.isEmpty())
            return nullptr;
        if (p == pMsoxlQuoteStop)
            ++p;    // position on ! of ...'!...
        if( *p != '!' && ( !bAllow3D || *p != ':' ) )
            return (!bAllow3D && *p == ':') ? p : start;
    }
    else if( *p == '\'')
    {
        p = parseQuotedName(p, aTabName);
        if (aTabName.isEmpty())
            return nullptr;
    }
    else if (pErrRef && lcl_isString( p, *pErrRef) && p[pErrRef->getLength()] == '!')
    {
        p += pErrRef->getLength();  // position after "#REF!" on '!'
        // XXX NOTE: caller has to check the name and that it wasn't quoted.
        aTabName = *pErrRef;
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
        while( true )
        {
            const sal_Unicode uc = *p;
            if( rtl::isAsciiAlpha( uc ) || uc == '_' )
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
            else if( rtl::isAsciiDigit( uc ))
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

        if( *p != '!' && ( !bAllow3D || *p != ':' ) )
            return (!bAllow3D && *p == ':') ? p : start;

        aTabName += OUString( start, sal::static_int_cast<sal_Int32>( p - start ) );
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
static bool lcl_XL_getExternalDoc( const sal_Unicode** ppErrRet, OUString& rExternDocName,
                                   const uno::Sequence<sheet::ExternalLinkInfo>* pExternalLinks )
{
    // 1-based, sequence starts with an empty element.
    if (pExternalLinks && pExternalLinks->hasElements())
    {
        // A numeric "document name" is an index into the sequence.
        if (CharClass::isAsciiNumeric( rExternDocName))
        {
            sal_Int32 i = rExternDocName.toInt32();
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
                            SAL_INFO(
                                "sc.core",
                                "Data type mismatch for ExternalLinkInfo "
                                    << i);
                            *ppErrRet = nullptr;
                            return false;
                        }
                        rExternDocName = aStr;
                    }
                    break;
                    case sheet::ExternalLinkType::SELF :
                        return false;   // ???
                    case sheet::ExternalLinkType::SPECIAL :
                        // silently return nothing (do not assert), caller has to handle this
                        *ppErrRet = nullptr;
                        return false;
                default:
                    SAL_INFO(
                        "sc.core",
                        "unhandled ExternalLinkType " << rInfo.Type
                            << " for index " << i);
                    *ppErrRet = nullptr;
                    return false;
            }
        }
    }
    return true;
}

const sal_Unicode* ScRange::Parse_XL_Header(
                                const sal_Unicode* p,
                                const ScDocument* pDoc,
                                OUString& rExternDocName,
                                OUString& rStartTabName,
                                OUString& rEndTabName,
                                ScRefFlags& nFlags,
                                bool bOnlyAcceptSingle,
                                const uno::Sequence<sheet::ExternalLinkInfo>* pExternalLinks,
                                const OUString* pErrRef )
{
    const sal_Unicode* startTabs, *start = p;
    ScRefFlags nSaveFlags = nFlags;

    // Is this an external reference ?
    rStartTabName.clear();
    rEndTabName.clear();
    rExternDocName.clear();
    const sal_Unicode* pMsoxlQuoteStop = nullptr;
    if (*p == '[')
    {
        ++p;
        // Only single quotes are correct, and a double single quote escapes a
        // single quote text inside the quoted text.
        if (*p == '\'')
        {
            p = parseQuotedName(p, rExternDocName);
            if (*p != ']' || rExternDocName.isEmpty())
            {
                rExternDocName.clear();
                return start;
            }
        }
        else
        {
            // non-quoted file name.
            p = ScGlobal::UnicodeStrChr( start+1, ']' );
            if( p == nullptr )
                return start;
            rExternDocName += OUString( start+1, sal::static_int_cast<sal_Int32>( p-(start+1) ) );
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
        p = parseQuotedName(p, rExternDocName);
        if (*p != '!')
        {
            rExternDocName.clear();
            return start;
        }
        if (!rExternDocName.isEmpty())
        {
            sal_Int32 nOpen = rExternDocName.indexOf( '[');
            if (nOpen == -1)
                rExternDocName.clear();
            else
            {
                sal_Int32 nClose = rExternDocName.indexOf( ']', nOpen+1);
                if (nClose == -1)
                    rExternDocName.clear();
                else
                {
                    rExternDocName = rExternDocName.copy(0, nClose);
                    rExternDocName = rExternDocName.replaceAt( nOpen, 1, "");
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
        if (rExternDocName.isEmpty())
            p = start;
    }

    startTabs = p;
    p = lcl_XL_ParseSheetRef( p, rStartTabName, !bOnlyAcceptSingle, pMsoxlQuoteStop, pErrRef);
    if( nullptr == p )
        return start;       // invalid tab
    if (bOnlyAcceptSingle && *p == ':')
        return nullptr;        // 3D
    const sal_Unicode* startEndTabs = nullptr;
    if( p != startTabs )
    {
        nFlags |= ScRefFlags::TAB_VALID | ScRefFlags::TAB_3D | ScRefFlags::TAB_ABS;
        if( *p == ':' ) // 3d ref
        {
            startEndTabs = p + 1;
            p = lcl_XL_ParseSheetRef( startEndTabs, rEndTabName, false, pMsoxlQuoteStop, pErrRef);
            if( p == nullptr )
            {
                nFlags = nSaveFlags;
                return start; // invalid tab
            }
            nFlags |= ScRefFlags::TAB2_VALID | ScRefFlags::TAB2_3D | ScRefFlags::TAB2_ABS;
        }
        else
        {
            // If only one sheet is given, the full reference is still valid,
            // only the second 3D flag is not set.
            nFlags |= ScRefFlags::TAB2_VALID | ScRefFlags::TAB2_ABS;
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
        nFlags |= ScRefFlags::TAB_VALID | ScRefFlags::TAB2_VALID;
        // Use the current tab, it needs to be passed in. : aEnd.SetTab( .. );
    }

    if (!rExternDocName.isEmpty())
    {
        ScExternalRefManager* pRefMgr = pDoc->GetExternalRefManager();
        pRefMgr->convertToAbsName(rExternDocName);
    }
    else
    {
        // Internal reference.
        if (rStartTabName.isEmpty())
        {
            nFlags = nSaveFlags;
            return start;
        }

        SCTAB nTab;
        if ((pErrRef && *startTabs != '\'' && rStartTabName == *pErrRef) || !pDoc->GetTable(rStartTabName, nTab))
        {
            // invalid table name.
            nFlags &= ~ScRefFlags::TAB_VALID;
            nTab = -1;
        }

        aStart.SetTab(nTab);
        aEnd.SetTab(nTab);

        if (!rEndTabName.isEmpty())
        {
            if ((pErrRef && startEndTabs && *startEndTabs != '\'' && rEndTabName == *pErrRef) ||
                    !pDoc->GetTable(rEndTabName, nTab))
            {
                // invalid table name.
                nFlags &= ~ScRefFlags::TAB2_VALID;
                nTab = -1;
            }

            aEnd.SetTab(nTab);
        }
    }
    return p;
}

static const sal_Unicode* lcl_r1c1_get_col( const sal_Unicode* p,
                                            const ScAddress::Details& rDetails,
                                            ScAddress* pAddr, ScRefFlags* nFlags )
{
    const sal_Unicode *pEnd;
    long int n;
    bool isRelative;

    if( p[0] == '\0' )
        return nullptr;

    p++;
    isRelative = *p == '[';
    if( isRelative )
        p++;
    n = sal_Unicode_strtol( p, &pEnd );
    if( nullptr == pEnd )
        return nullptr;

    if( p == pEnd ) // C is a relative ref with offset 0
    {
        if( isRelative )
            return nullptr;
        n = rDetails.nCol;
    }
    else if( isRelative )
    {
        if( *pEnd != ']' )
            return nullptr;
        n += rDetails.nCol;
        pEnd++;
    }
    else
    {
        *nFlags |= ScRefFlags::COL_ABS;
        n--;
    }

    if( n < 0 || n >= MAXCOLCOUNT )
        return nullptr;
    pAddr->SetCol( static_cast<SCCOL>( n ) );
    *nFlags |= ScRefFlags::COL_VALID;

    return pEnd;
}

static const sal_Unicode* lcl_r1c1_get_row(
                                    const sal_Unicode* p,
                                    const ScAddress::Details& rDetails,
                                    ScAddress* pAddr, ScRefFlags* nFlags )
{
    const sal_Unicode *pEnd;
    long int n;
    bool isRelative;

    if( p[0] == '\0' )
        return nullptr;

    p++;
    isRelative = *p == '[';
    if( isRelative )
        p++;
    n = sal_Unicode_strtol( p, &pEnd );
    if( nullptr == pEnd )
        return nullptr;

    if( p == pEnd ) // R is a relative ref with offset 0
    {
        if( isRelative )
            return nullptr;
        n = rDetails.nRow;
    }
    else if( isRelative )
    {
        if( *pEnd != ']' )
            return nullptr;
        n += rDetails.nRow;
        pEnd++;
    }
    else
    {
        *nFlags |= ScRefFlags::ROW_ABS;
        n--;
    }

    if( n < 0 || n >= MAXROWCOUNT )
        return nullptr;
    pAddr->SetRow( static_cast<SCROW>( n ) );
    *nFlags |= ScRefFlags::ROW_VALID;

    return pEnd;
}

static ScRefFlags lcl_ScRange_Parse_XL_R1C1( ScRange& r,
                                             const sal_Unicode* p,
                                             const ScDocument* pDoc,
                                             const ScAddress::Details& rDetails,
                                             bool bOnlyAcceptSingle,
                                             ScAddress::ExternalInfo* pExtInfo,
                                             sal_Int32* pSheetEndPos )
{
    const sal_Unicode* const pStart = p;
    if (pSheetEndPos)
        *pSheetEndPos = 0;
    const sal_Unicode* pTmp = nullptr;
    OUString aExternDocName, aStartTabName, aEndTabName;
    ScRefFlags nFlags = ScRefFlags::VALID | ScRefFlags::TAB_VALID;
    // Keep in mind that nFlags2 gets left-shifted by 4 bits before being merged.
    ScRefFlags nFlags2 = ScRefFlags::TAB_VALID;

    p = r.Parse_XL_Header( p, pDoc, aExternDocName, aStartTabName,
            aEndTabName, nFlags, bOnlyAcceptSingle );

    ScRefFlags nBailOutFlags = ScRefFlags::ZERO;
    if (pSheetEndPos && pStart < p && (nFlags & ScRefFlags::TAB_VALID) && (nFlags & ScRefFlags::TAB_3D))
    {
        *pSheetEndPos = p - pStart;
        nBailOutFlags = ScRefFlags::TAB_VALID | ScRefFlags::TAB_3D;
    }

    if (!aExternDocName.isEmpty())
        lcl_ScRange_External_TabSpan( r, nFlags, pExtInfo, aExternDocName,
                aStartTabName, aEndTabName, pDoc);

    if( nullptr == p )
        return ScRefFlags::ZERO;

    if( *p == 'R' || *p == 'r' )
    {
        if( nullptr == (p = lcl_r1c1_get_row( p, rDetails, &r.aStart, &nFlags )) )
            return nBailOutFlags;

        if( *p != 'C' && *p != 'c' )    // full row R#
        {
            if( p[0] != ':' || (p[1] != 'R' && p[1] != 'r' ) ||
                nullptr == (pTmp = lcl_r1c1_get_row( p+1, rDetails, &r.aEnd, &nFlags2 )))
            {
                // Only the initial row number is given, or the second row
                // number is invalid. Fallback to just the initial R
                applyStartToEndFlags(nFlags);
                r.aEnd.SetRow( r.aStart.Row() );
            }
            else // pTmp != nullptr
            {
                // Full row range successfully parsed.
                applyStartToEndFlags(nFlags, nFlags2);
                p = pTmp;
            }

            if (p[0] != 0)
            {
                // any trailing invalid character must invalidate the whole address.
                nFlags &= ~ScRefFlags(ScRefFlags::VALID | ScRefFlags::COL_VALID | ScRefFlags::ROW_VALID | ScRefFlags::TAB_VALID |
                            ScRefFlags::COL2_VALID | ScRefFlags::ROW2_VALID | ScRefFlags::TAB2_VALID);
                return nFlags;
            }

            nFlags |=
                ScRefFlags::COL_VALID | ScRefFlags::COL2_VALID |
                ScRefFlags::COL_ABS | ScRefFlags::COL2_ABS;
            r.aStart.SetCol( 0 );
            r.aEnd.SetCol( MAXCOL );

            return bOnlyAcceptSingle ? ScRefFlags::ZERO : nFlags;
        }
        else if( nullptr == (p = lcl_r1c1_get_col( p, rDetails, &r.aStart, &nFlags )))
        {
            return ScRefFlags::ZERO;
        }

        if( p[0] != ':' ||
            (p[1] != 'R' && p[1] != 'r') ||
            nullptr == (pTmp = lcl_r1c1_get_row( p+1, rDetails, &r.aEnd, &nFlags2 )) ||
            (*pTmp != 'C' && *pTmp != 'c') ||
            nullptr == (pTmp = lcl_r1c1_get_col( pTmp, rDetails, &r.aEnd, &nFlags2 )))
        {
            // single cell reference

            if (p[0] != 0)
            {
                // any trailing invalid character must invalidate the whole address.
                nFlags &= ~ScRefFlags(ScRefFlags::VALID | ScRefFlags::COL_VALID | ScRefFlags::ROW_VALID | ScRefFlags::TAB_VALID);
                return nFlags;
            }

            return bOnlyAcceptSingle ? nFlags : ScRefFlags::ZERO;
        }
        assert(pTmp);
        p = pTmp;

        // double reference

        if (p[0] != 0)
        {
            // any trailing invalid character must invalidate the whole range.
            nFlags &= ~ScRefFlags(ScRefFlags::VALID | ScRefFlags::COL_VALID | ScRefFlags::ROW_VALID | ScRefFlags::TAB_VALID |
                        ScRefFlags::COL2_VALID | ScRefFlags::ROW2_VALID | ScRefFlags::TAB2_VALID);
            return nFlags;
        }

        applyStartToEndFlags(nFlags, nFlags2);
        return bOnlyAcceptSingle ? ScRefFlags::ZERO : nFlags;
    }
    else if( *p == 'C' || *p == 'c' )   // full col C#
    {
        if( nullptr == (p = lcl_r1c1_get_col( p, rDetails, &r.aStart, &nFlags )))
            return nBailOutFlags;

        if( p[0] != ':' || (p[1] != 'C' && p[1] != 'c') ||
            nullptr == (pTmp = lcl_r1c1_get_col( p+1, rDetails, &r.aEnd, &nFlags2 )))
        {    // Fallback to just the initial C
            applyStartToEndFlags(nFlags);
            r.aEnd.SetCol( r.aStart.Col() );
        }
        else // pTmp != nullptr
        {
            applyStartToEndFlags(nFlags, nFlags2);
            p = pTmp;
        }

        if (p[0] != 0)
        {
            // any trailing invalid character must invalidate the whole address.
            nFlags &= ~ScRefFlags(ScRefFlags::VALID | ScRefFlags::COL_VALID | ScRefFlags::ROW_VALID | ScRefFlags::TAB_VALID |
                        ScRefFlags::COL2_VALID | ScRefFlags::ROW2_VALID | ScRefFlags::TAB2_VALID);
            return nFlags;
        }

        nFlags |=
            ScRefFlags::ROW_VALID | ScRefFlags::ROW2_VALID |
            ScRefFlags::ROW_ABS | ScRefFlags::ROW2_ABS;
        r.aStart.SetRow( 0 );
        r.aEnd.SetRow( MAXROW );

        return bOnlyAcceptSingle ? ScRefFlags::ZERO : nFlags;
    }

    return nBailOutFlags;
}

static const sal_Unicode* lcl_a1_get_col( const sal_Unicode* p,
                                                 ScAddress* pAddr,
                                                 ScRefFlags* nFlags,
                                                 const OUString* pErrRef )
{
    SCCOL nCol;

    if( *p == '$' )
    {
        *nFlags |= ScRefFlags::COL_ABS;
        p++;
    }

    if (pErrRef && lcl_isString( p, *pErrRef))
    {
        p += pErrRef->getLength();
        *nFlags &= ~ScRefFlags::COL_VALID;
        pAddr->SetCol(-1);
        return p;
    }

    if( !rtl::isAsciiAlpha( *p ) )
        return nullptr;

    nCol = sal::static_int_cast<SCCOL>( rtl::toAsciiUpperCase( *p++ ) - 'A' );
    while (nCol <= MAXCOL && rtl::isAsciiAlpha(*p))
        nCol = sal::static_int_cast<SCCOL>( ((nCol + 1) * 26) + rtl::toAsciiUpperCase( *p++ ) - 'A' );
    if( nCol > MAXCOL || rtl::isAsciiAlpha( *p ) )
        return nullptr;

    *nFlags |= ScRefFlags::COL_VALID;
    pAddr->SetCol( nCol );

    return p;
}

static const sal_Unicode* lcl_a1_get_row( const sal_Unicode* p,
                                                 ScAddress* pAddr,
                                                 ScRefFlags* nFlags,
                                                 const OUString* pErrRef )
{
    const sal_Unicode *pEnd;
    long int n;

    if( *p == '$' )
    {
        *nFlags |= ScRefFlags::ROW_ABS;
        p++;
    }

    if (pErrRef && lcl_isString( p, *pErrRef))
    {
        p += pErrRef->getLength();
        *nFlags &= ~ScRefFlags::ROW_VALID;
        pAddr->SetRow(-1);
        return p;
    }

    n = sal_Unicode_strtol( p, &pEnd ) - 1;
    if( nullptr == pEnd || p == pEnd || n < 0 || n > MAXROW )
        return nullptr;

    *nFlags |= ScRefFlags::ROW_VALID;
    pAddr->SetRow( static_cast<SCROW>(n) );

    return pEnd;
}

/// B:B or 2:2, but not B:2 or 2:B or B2:B or B:B2 or ...
static bool isValidSingleton( ScRefFlags nFlags, ScRefFlags nFlags2 )
{
    bool bCols = (nFlags & ScRefFlags::COL_VALID) && ((nFlags & ScRefFlags::COL2_VALID) || (nFlags2 & ScRefFlags::COL_VALID));
    bool bRows = (nFlags & ScRefFlags::ROW_VALID) && ((nFlags & ScRefFlags::ROW2_VALID) || (nFlags2 & ScRefFlags::ROW_VALID));
    return bCols != bRows;
}

static ScRefFlags lcl_ScRange_Parse_XL_A1( ScRange& r,
                                           const sal_Unicode* p,
                                           const ScDocument* pDoc,
                                           bool bOnlyAcceptSingle,
                                           ScAddress::ExternalInfo* pExtInfo,
                                           const uno::Sequence<sheet::ExternalLinkInfo>* pExternalLinks,
                                           sal_Int32* pSheetEndPos,
                                           const OUString* pErrRef )
{
    const sal_Unicode* const pStart = p;
    if (pSheetEndPos)
        *pSheetEndPos = 0;
    const sal_Unicode* tmp1, *tmp2;
    OUString aExternDocName, aStartTabName, aEndTabName; // for external link table
    ScRefFlags nFlags = ScRefFlags::VALID | ScRefFlags::TAB_VALID, nFlags2 = ScRefFlags::TAB_VALID;

    p = r.Parse_XL_Header( p, pDoc, aExternDocName, aStartTabName,
            aEndTabName, nFlags, bOnlyAcceptSingle, pExternalLinks, pErrRef );

    ScRefFlags nBailOutFlags = ScRefFlags::ZERO;
    if (pSheetEndPos && pStart < p && (nFlags & ScRefFlags::TAB_VALID) && (nFlags & ScRefFlags::TAB_3D))
    {
        *pSheetEndPos = p - pStart;
        nBailOutFlags = ScRefFlags::TAB_VALID | ScRefFlags::TAB_3D;
    }

    if (!aExternDocName.isEmpty())
        lcl_ScRange_External_TabSpan( r, nFlags, pExtInfo, aExternDocName,
                aStartTabName, aEndTabName, pDoc);

    if( nullptr == p )
        return nBailOutFlags;

    tmp1 = lcl_a1_get_col( p, &r.aStart, &nFlags, pErrRef);
    if( tmp1 == nullptr )          // Is it a row only reference 3:5
    {
        if( bOnlyAcceptSingle ) // by definition full row refs are ranges
            return nBailOutFlags;

        tmp1 = lcl_a1_get_row( p, &r.aStart, &nFlags, pErrRef);

        tmp1 = lcl_eatWhiteSpace( tmp1 );
        if( !tmp1 || *tmp1++ != ':' ) // Even a singleton requires ':' (eg 2:2)
            return nBailOutFlags;

        tmp1 = lcl_eatWhiteSpace( tmp1 );
        tmp2 = lcl_a1_get_row( tmp1, &r.aEnd, &nFlags2, pErrRef);
        if( !tmp2 || *tmp2 != 0 )   // Must have fully parsed a singleton.
            return nBailOutFlags;

        r.aStart.SetCol( 0 ); r.aEnd.SetCol( MAXCOL );
        nFlags |=
            ScRefFlags::COL_VALID | ScRefFlags::COL2_VALID |
            ScRefFlags::COL_ABS | ScRefFlags::COL2_ABS;
        applyStartToEndFlags(nFlags, nFlags2);
        return nFlags;
    }

    tmp2 = lcl_a1_get_row( tmp1, &r.aStart, &nFlags, pErrRef);
    if( tmp2 == nullptr )          // check for col only reference F:H
    {
        if( bOnlyAcceptSingle ) // by definition full col refs are ranges
            return nBailOutFlags;

        tmp1 = lcl_eatWhiteSpace( tmp1 );
        if( *tmp1++ != ':' )    // Even a singleton requires ':' (eg F:F)
            return nBailOutFlags;

        tmp1 = lcl_eatWhiteSpace( tmp1 );
        tmp2 = lcl_a1_get_col( tmp1, &r.aEnd, &nFlags2, pErrRef);
        if( !tmp2 || *tmp2 != 0 )   // Must have fully parsed a singleton.
            return nBailOutFlags;

        r.aStart.SetRow( 0 ); r.aEnd.SetRow( MAXROW );
        nFlags |=
            ScRefFlags::ROW_VALID | ScRefFlags::ROW2_VALID |
            ScRefFlags::ROW_ABS | ScRefFlags::ROW2_ABS;
        applyStartToEndFlags(nFlags, nFlags2);
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
            nFlags &= ~ScRefFlags(ScRefFlags::VALID | ScRefFlags::COL_VALID | ScRefFlags::ROW_VALID | ScRefFlags::TAB_VALID);
            return nFlags;
        }
    }

    tmp2 = lcl_eatWhiteSpace( tmp2 );
    if( *tmp2 != ':' )
    {
        // Sheet1:Sheet2!C4 is a valid range, without a second sheet it is
        // not. Any trailing invalid character invalidates the range.
        if (*tmp2 == 0 && (nFlags & ScRefFlags::TAB2_3D))
        {
            if (nFlags & ScRefFlags::COL_ABS)
                nFlags |= ScRefFlags::COL2_ABS;
            if (nFlags & ScRefFlags::ROW_ABS)
                nFlags |= ScRefFlags::ROW2_ABS;
        }
        else
            nFlags &= ~ScRefFlags(ScRefFlags::VALID |
                    ScRefFlags::COL_VALID | ScRefFlags::ROW_VALID | ScRefFlags::TAB_VALID |
                    ScRefFlags::COL2_VALID | ScRefFlags::ROW2_VALID | ScRefFlags::TAB2_VALID);
        return nFlags;
    }

    p = tmp2;
    p = lcl_eatWhiteSpace( p+1 );   // after ':'
    tmp1 = lcl_a1_get_col( p, &r.aEnd, &nFlags2, pErrRef);
    if( !tmp1 && aEndTabName.isEmpty() )     // Probably the aEndTabName was specified after the first range
    {
        p = lcl_XL_ParseSheetRef( p, aEndTabName, false, nullptr, pErrRef);
        if( p )
        {
            SCTAB nTab = 0;
            if( !aEndTabName.isEmpty() && pDoc->GetTable( aEndTabName, nTab ) )
            {
                r.aEnd.SetTab( nTab );
                nFlags |= ScRefFlags::TAB2_VALID | ScRefFlags::TAB2_3D | ScRefFlags::TAB2_ABS;
            }
            if (*p == '!' || *p == ':')
                p = lcl_eatWhiteSpace( p+1 );
            tmp1 = lcl_a1_get_col( p, &r.aEnd, &nFlags2, pErrRef);
        }
    }
    if( !tmp1 ) // strange, but maybe valid singleton
        return isValidSingleton( nFlags, nFlags2) ? nFlags : (nFlags & ~ScRefFlags::VALID);

    tmp2 = lcl_a1_get_row( tmp1, &r.aEnd, &nFlags2, pErrRef);
    if( !tmp2 ) // strange, but maybe valid singleton
        return isValidSingleton( nFlags, nFlags2) ? nFlags : (nFlags & ~ScRefFlags::VALID);

    if ( *tmp2 != 0 )
    {
        // any trailing invalid character must invalidate the range.
        nFlags &= ~ScRefFlags(ScRefFlags::VALID | ScRefFlags::COL_VALID | ScRefFlags::ROW_VALID | ScRefFlags::TAB_VALID |
                    ScRefFlags::COL2_VALID | ScRefFlags::ROW2_VALID | ScRefFlags::TAB2_VALID);
        return nFlags;
    }

    applyStartToEndFlags(nFlags, nFlags2);
    return nFlags;
}

/**
    @param p        pointer to null-terminated sal_Unicode string
    @param rRawRes  returns ScRefFlags::... flags without the final check for full
                    validity that is applied to the return value, with which
                    two addresses that form a column or row singleton range,
                    e.g. A:A or 1:1, can be detected. Used in
                    lcl_ScRange_Parse_OOo().
    @param pRange   pointer to range where rAddr effectively is *pRange->aEnd,
                    used in conjunction with pExtInfo to determine the tab span
                    of a 3D reference.
 */
static ScRefFlags lcl_ScAddress_Parse_OOo( const sal_Unicode* p, const ScDocument* pDoc, ScAddress& rAddr,
                                           ScRefFlags& rRawRes,
                                           ScAddress::ExternalInfo* pExtInfo,
                                           ScRange* pRange,
                                           sal_Int32* pSheetEndPos,
                                           const OUString* pErrRef )
{
    const sal_Unicode* const pStart = p;
    if (pSheetEndPos)
        *pSheetEndPos = 0;
    ScRefFlags  nRes = ScRefFlags::ZERO;
    rRawRes = ScRefFlags::ZERO;
    OUString aDocName;       // the pure Document Name
    OUString aTab;
    bool    bExtDoc = false;
    bool    bExtDocInherited = false;

    // Lets see if this is a reference to something in an external file.  A
    // document name is always quoted and has a trailing #.
    if (*p == '\'')
    {
        OUString aTmp;
        p = parseQuotedName(p, aTmp);
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
    ScRefFlags nBailOutFlags = ScRefFlags::ZERO;
    ScRefFlags nBits = ScRefFlags::TAB_VALID;
    const sal_Unicode* q;
    if ( ScGlobal::FindUnquoted( p, '.') )
    {
        nRes |= ScRefFlags::TAB_3D;
        if ( bExtDoc )
            nRes |= ScRefFlags::TAB_ABS;
        if (*p == '$')
        {
            nRes |= ScRefFlags::TAB_ABS;
            p++;
        }

        if (pErrRef && lcl_isString( p, *pErrRef) && p[pErrRef->getLength()] == '.')
        {
            // #REF! particle of an invalidated reference plus sheet separator.
            p += pErrRef->getLength() + 1;
            nRes &= ~ScRefFlags::TAB_VALID;
            nTab = -1;
        }
        else
        {
            if (*p == '\'')
            {
                // Tokens that start at ' can have anything in them until a final
                // ' but '' marks an escaped '.  We've earlier guaranteed that a
                // string containing '' will be surrounded by '.
                p = parseQuotedName(p, aTab);
            }
            else
            {
                OUStringBuffer aTabAcc;
                while (*p)
                {
                    if( *p == '.')
                        break;

                    if( *p == '\'' )
                    {
                        p++; break;
                    }
                    aTabAcc.append(*p);
                    p++;
                }
                aTab = aTabAcc.makeStringAndClear();
            }
            if( *p++ != '.' )
                nBits = ScRefFlags::ZERO;

            if (!bExtDoc && (!pDoc || !pDoc->GetTable( aTab, nTab )))
            {
                // Specified table name is not found in this document.  Assume this is an external document.
                aDocName = aTab;
                sal_Int32 n = aDocName.lastIndexOf('.');
                if (n > 0)
                {
                    // Extension found.  Strip it.
                    aTab = aTab.replaceAt(n, 1, "");
                    bExtDoc = true;
                }
                else
                    // No extension found.  This is probably not an external document.
                    nBits = ScRefFlags::ZERO;
            }
        }

        if (pSheetEndPos && (nBits & ScRefFlags::TAB_VALID))
        {
            *pSheetEndPos = p - pStart;
            nBailOutFlags = ScRefFlags::TAB_VALID | ScRefFlags::TAB_3D;
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
        nBits = ScRefFlags::COL_VALID;
        if (*p == '$')
        {
            nBits |= ScRefFlags::COL_ABS;
            p++;
        }

        if (pErrRef && lcl_isString( p, *pErrRef))
        {
            // #REF! particle of an invalidated reference.
            p += pErrRef->getLength();
            nBits &= ~ScRefFlags::COL_VALID;
            nCol = -1;
        }
        else
        {
            if (rtl::isAsciiAlpha( *p ))
            {
                nCol = sal::static_int_cast<SCCOL>( rtl::toAsciiUpperCase( *p++ ) - 'A' );
                while (nCol < MAXCOL && rtl::isAsciiAlpha(*p))
                    nCol = sal::static_int_cast<SCCOL>( ((nCol + 1) * 26) + rtl::toAsciiUpperCase( *p++ ) - 'A' );
            }
            else
                nBits = ScRefFlags::ZERO;

            if (nCol > MAXCOL || (*p && *p != '$' && !rtl::isAsciiDigit( *p ) &&
                        (!pErrRef || !lcl_isString( p, *pErrRef))))
                nBits = ScRefFlags::ZERO;
            if( nBits == ScRefFlags::ZERO )
                p = q;
        }
        nRes |= nBits;
    }

    q = p;
    if (*p)
    {
        nBits = ScRefFlags::ROW_VALID;
        if (*p == '$')
        {
            nBits |= ScRefFlags::ROW_ABS;
            p++;
        }

        if (pErrRef && lcl_isString( p, *pErrRef))
        {
            // #REF! particle of an invalidated reference.
            p += pErrRef->getLength();
            // Clearing the ROW_VALID bit here is not possible because of the
            // check at the end whether only a valid column was detected in
            // which case all bits are cleared because it could be any other
            // name. Instead, set to an absolute invalid row value. This will
            // display a $#REF! instead of #REF! if the error value was
            // relative, but live with it.
            nBits |= ScRefFlags::ROW_ABS;
            nRow = -1;
        }
        else
        {
            if( !rtl::isAsciiDigit( *p ) )
            {
                nBits = ScRefFlags::ZERO;
                nRow = SCROW(-1);
            }
            else
            {
                long n = rtl_ustr_toInt32( p, 10 ) - 1;
                while (rtl::isAsciiDigit( *p ))
                    p++;
                if( n < 0 || n > MAXROW )
                    nBits = ScRefFlags::ZERO;
                nRow = static_cast<SCROW>(n);
            }
            if( nBits == ScRefFlags::ZERO )
                p = q;
        }
        nRes |= nBits;
    }

    rAddr.Set( nCol, nRow, nTab );

    if (!*p && bExtDoc)
    {
        if (!pDoc)
            nRes = ScRefFlags::ZERO;
        else
        {
            ScExternalRefManager* pRefMgr = pDoc->GetExternalRefManager();

            // Need document name if inherited.
            if (bExtDocInherited)
            {
                // The FileId was created using the original file name, so
                // obtain that. Otherwise lcl_ScRange_External_TabSpan() would
                // retrieve a FileId for the real name and bail out if that
                // differed from pExtInfo->mnFileId, as is the case when
                // loading documents that refer external files relative to the
                // current own document but were saved from a different path
                // than loaded.
                const OUString* pFileName = pRefMgr->getExternalFileName( pExtInfo->mnFileId, true);
                if (pFileName)
                    aDocName = *pFileName;
                else
                    nRes = ScRefFlags::ZERO;
            }
            pRefMgr->convertToAbsName(aDocName);

            if ((!pExtInfo || !pExtInfo->mbExternal) && pRefMgr->isOwnDocument(aDocName))
            {
                if (!pDoc->GetTable( aTab, nTab ))
                    nRes = ScRefFlags::ZERO;
                else
                {
                    rAddr.SetTab( nTab);
                    nRes |= ScRefFlags::TAB_VALID;
                }
            }
            else
            {
                if (!pExtInfo)
                    nRes = ScRefFlags::ZERO;
                else
                {
                    if (!pExtInfo->mbExternal)
                    {
                        sal_uInt16 nFileId = pRefMgr->getExternalFileId(aDocName);

                        pExtInfo->mbExternal = true;
                        pExtInfo->maTabName = aTab;
                        pExtInfo->mnFileId = nFileId;

                        if (pRefMgr->getSingleRefToken(nFileId, aTab,
                                    ScAddress(nCol, nRow, 0), nullptr,
                                    &nTab).get())
                        {
                            rAddr.SetTab( nTab);
                            nRes |= ScRefFlags::TAB_VALID;
                        }
                        else
                            nRes = ScRefFlags::ZERO;
                    }
                    else
                    {
                        // This is a call for the second part of the reference,
                        // we must have the range to adapt tab span.
                        if (!pRange)
                            nRes = ScRefFlags::ZERO;
                        else
                        {
                            ScRefFlags nFlags = nRes | ScRefFlags::TAB2_VALID;
                            if (!lcl_ScRange_External_TabSpan( *pRange, nFlags,
                                        pExtInfo, aDocName,
                                        pExtInfo->maTabName, aTab, pDoc))
                                nRes &= ~ScRefFlags::TAB_VALID;
                            else
                            {
                                if (nFlags & ScRefFlags::TAB2_VALID)
                                {
                                    rAddr.SetTab( pRange->aEnd.Tab());
                                    nRes |= ScRefFlags::TAB_VALID;
                                }
                                else
                                    nRes &= ~ScRefFlags::TAB_VALID;
                            }
                        }
                    }
                }
            }
        }
    }

    rRawRes |= nRes;

    if ( !(nRes & ScRefFlags::ROW_VALID) && (nRes & ScRefFlags::COL_VALID)
            && !( (nRes & ScRefFlags::TAB_3D) && (nRes & ScRefFlags::TAB_VALID)) )
    {   // no Row, no Tab, but Col => DM (...), B (...) et al
        nRes = ScRefFlags::ZERO;
    }
    if( !*p )
    {
        ScRefFlags nMask = nRes & ( ScRefFlags::ROW_VALID | ScRefFlags::COL_VALID | ScRefFlags::TAB_VALID );
        if( nMask == ( ScRefFlags::ROW_VALID | ScRefFlags::COL_VALID | ScRefFlags::TAB_VALID ) )
            nRes |= ScRefFlags::VALID;
    }
    else
        nRes = rRawRes = nBailOutFlags;
    return nRes;
}

static ScRefFlags lcl_ScAddress_Parse ( const sal_Unicode* p, const ScDocument* pDoc, ScAddress& rAddr,
                                        const ScAddress::Details& rDetails,
                                        ScAddress::ExternalInfo* pExtInfo,
                                        const uno::Sequence<sheet::ExternalLinkInfo>* pExternalLinks,
                                        sal_Int32* pSheetEndPos,
                                        const OUString* pErrRef )
{
    if( !*p )
        return ScRefFlags::ZERO;

    switch (rDetails.eConv)
    {
        case formula::FormulaGrammar::CONV_XL_A1:
        case formula::FormulaGrammar::CONV_XL_OOX:
        {
            ScRange rRange = rAddr;
            ScRefFlags nFlags = lcl_ScRange_Parse_XL_A1(
                    rRange, p, pDoc, true, pExtInfo,
                    (rDetails.eConv == formula::FormulaGrammar::CONV_XL_OOX ? pExternalLinks : nullptr),
                    pSheetEndPos, pErrRef);
            rAddr = rRange.aStart;
            return nFlags;
        }
        case formula::FormulaGrammar::CONV_XL_R1C1:
        {
            ScRange rRange = rAddr;
            ScRefFlags nFlags = lcl_ScRange_Parse_XL_R1C1( rRange, p, pDoc, rDetails, true, pExtInfo, pSheetEndPos);
            rAddr = rRange.aStart;
            return nFlags;
        }
        default :
        case formula::FormulaGrammar::CONV_OOO:
        {
            ScRefFlags nRawRes = ScRefFlags::ZERO;
            return lcl_ScAddress_Parse_OOo( p, pDoc, rAddr, nRawRes, pExtInfo, nullptr, pSheetEndPos, pErrRef);
        }
    }
}

bool ConvertSingleRef( const ScDocument* pDoc, const OUString& rRefString,
                       SCTAB nDefTab, ScRefAddress& rRefAddress,
                       const ScAddress::Details& rDetails,
                       ScAddress::ExternalInfo* pExtInfo /* = NULL */ )
{
    bool bRet = false;
    if (pExtInfo || (ScGlobal::FindUnquoted( rRefString, SC_COMPILER_FILE_TAB_SEP) == -1))
    {
        ScAddress aAddr( 0, 0, nDefTab );
        ScRefFlags nRes = aAddr.Parse( rRefString, pDoc, rDetails, pExtInfo);
        if ( nRes & ScRefFlags::VALID )
        {
            rRefAddress.Set( aAddr,
                    ((nRes & ScRefFlags::COL_ABS) == ScRefFlags::ZERO),
                    ((nRes & ScRefFlags::ROW_ABS) == ScRefFlags::ZERO),
                    ((nRes & ScRefFlags::TAB_ABS) == ScRefFlags::ZERO));
            bRet = true;
        }
    }
    return bRet;
}

bool ConvertDoubleRef( const ScDocument* pDoc, const OUString& rRefString, SCTAB nDefTab,
                       ScRefAddress& rStartRefAddress, ScRefAddress& rEndRefAddress,
                       const ScAddress::Details& rDetails,
                       ScAddress::ExternalInfo* pExtInfo /* = NULL */ )
{
    bool bRet = false;
    if (pExtInfo || (ScGlobal::FindUnquoted( rRefString, SC_COMPILER_FILE_TAB_SEP) == -1))
    {
        ScRange aRange( ScAddress( 0, 0, nDefTab));
        ScRefFlags nRes = aRange.Parse( rRefString, pDoc, rDetails, pExtInfo);
        if ( nRes & ScRefFlags::VALID )
        {
            rStartRefAddress.Set( aRange.aStart,
                    ((nRes & ScRefFlags::COL_ABS) == ScRefFlags::ZERO),
                    ((nRes & ScRefFlags::ROW_ABS) == ScRefFlags::ZERO),
                    ((nRes & ScRefFlags::TAB_ABS) == ScRefFlags::ZERO));
            rEndRefAddress.Set( aRange.aEnd,
                    ((nRes & ScRefFlags::COL2_ABS) == ScRefFlags::ZERO),
                    ((nRes & ScRefFlags::ROW2_ABS) == ScRefFlags::ZERO),
                    ((nRes & ScRefFlags::TAB2_ABS) == ScRefFlags::ZERO));
            bRet = true;
        }
    }
    return bRet;
}

ScRefFlags ScAddress::Parse( const OUString& r, const ScDocument* pDoc,
                             const Details& rDetails,
                             ExternalInfo* pExtInfo,
                             const uno::Sequence<sheet::ExternalLinkInfo>* pExternalLinks,
                             sal_Int32* pSheetEndPos,
                             const OUString* pErrRef )
{
    return lcl_ScAddress_Parse( r.getStr(), pDoc, *this, rDetails, pExtInfo, pExternalLinks, pSheetEndPos, pErrRef);
}

bool ScRange::Intersects( const ScRange& rRange ) const
{
    return !(
        std::min( aEnd.Col(), rRange.aEnd.Col() ) < std::max( aStart.Col(), rRange.aStart.Col() )
     || std::min( aEnd.Row(), rRange.aEnd.Row() ) < std::max( aStart.Row(), rRange.aStart.Row() )
     || std::min( aEnd.Tab(), rRange.aEnd.Tab() ) < std::max( aStart.Tab(), rRange.aStart.Tab() )
        );
}

ScRange ScRange::Intersection( const ScRange& rOther ) const
{
    SCCOL nCol1 = std::max(aStart.Col(), rOther.aStart.Col());
    SCCOL nCol2 = std::min(aEnd.Col(), rOther.aEnd.Col());
    SCROW nRow1 = std::max(aStart.Row(), rOther.aStart.Row());
    SCROW nRow2 = std::min(aEnd.Row(), rOther.aEnd.Row());
    SCTAB nTab1 = std::max(aStart.Tab(), rOther.aStart.Tab());
    SCTAB nTab2 = std::min(aEnd.Tab(), rOther.aEnd.Tab());

    if (nCol1 > nCol2 || nRow1 > nRow2 || nTab1 > nTab2)
        return ScRange(ScAddress::INITIALIZE_INVALID);

    return ScRange(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
}

void ScRange::PutInOrder()
{
    SCCOL nTempCol;
    if ( aEnd.Col() < (nTempCol = aStart.Col()) )
    {
        aStart.SetCol(aEnd.Col());
        aEnd.SetCol(nTempCol);
    }
    SCROW nTempRow;
    if ( aEnd.Row() < (nTempRow = aStart.Row()) )
    {
        aStart.SetRow(aEnd.Row());
        aEnd.SetRow(nTempRow);
    }
    SCTAB nTempTab;
    if ( aEnd.Tab() < (nTempTab = aStart.Tab()) )
    {
        aStart.SetTab(aEnd.Tab());
        aEnd.SetTab(nTempTab);
    }
}

void ScRange::ExtendTo( const ScRange& rRange )
{
    OSL_ENSURE( rRange.IsValid(), "ScRange::ExtendTo - cannot extend to invalid range" );
    if( IsValid() )
    {
        aStart.SetCol( std::min( aStart.Col(), rRange.aStart.Col() ) );
        aStart.SetRow( std::min( aStart.Row(), rRange.aStart.Row() ) );
        aStart.SetTab( std::min( aStart.Tab(), rRange.aStart.Tab() ) );
        aEnd.SetCol(   std::max( aEnd.Col(),   rRange.aEnd.Col() ) );
        aEnd.SetRow(   std::max( aEnd.Row(),   rRange.aEnd.Row() ) );
        aEnd.SetTab(   std::max( aEnd.Tab(),   rRange.aEnd.Tab() ) );
    }
    else
        *this = rRange;
}

static ScRefFlags lcl_ScRange_Parse_OOo( ScRange& rRange,
                                         const OUString& r,
                                         const ScDocument* pDoc,
                                         ScAddress::ExternalInfo* pExtInfo,
                                         const OUString* pErrRef )
{
    ScRefFlags nRes1 = ScRefFlags::ZERO, nRes2 = ScRefFlags::ZERO;
    sal_Int32 nPos = ScGlobal::FindUnquoted( r, ':');
    if (nPos != -1)
    {
        OUStringBuffer aTmp(r);
        aTmp[nPos] = 0;
        const sal_Unicode* p = aTmp.getStr();
        ScRefFlags nRawRes1 = ScRefFlags::ZERO;
        nRes1 = lcl_ScAddress_Parse_OOo( p, pDoc, rRange.aStart, nRawRes1, pExtInfo, nullptr, nullptr, pErrRef);
        if ((nRes1 != ScRefFlags::ZERO) ||
                ((nRawRes1 & (ScRefFlags::COL_VALID | ScRefFlags::ROW_VALID)) &&
                 (nRawRes1 & ScRefFlags::TAB_VALID)))
        {
            rRange.aEnd = rRange.aStart;  // sheet must be initialized identical to first sheet
            ScRefFlags nRawRes2 = ScRefFlags::ZERO;
            nRes2 = lcl_ScAddress_Parse_OOo( p + nPos+ 1, pDoc, rRange.aEnd, nRawRes2,
                    pExtInfo, &rRange, nullptr, pErrRef);
            if (!((nRes1 & ScRefFlags::VALID) && (nRes2 & ScRefFlags::VALID)) &&
                    // If not fully valid addresses, check if both have a valid
                    // column or row, and both have valid (or omitted) sheet references.
                    (nRawRes1 & (ScRefFlags::COL_VALID | ScRefFlags::ROW_VALID)) &&
                    (nRawRes1 & ScRefFlags::TAB_VALID) &&
                    (nRawRes2 & (ScRefFlags::COL_VALID | ScRefFlags::ROW_VALID)) &&
                    (nRawRes2 & ScRefFlags::TAB_VALID) &&
                    // Both must be column XOR row references, A:A or 1:1 but not A:1 or 1:A
                    ((nRawRes1 & (ScRefFlags::COL_VALID | ScRefFlags::ROW_VALID)) ==
                     (nRawRes2 & (ScRefFlags::COL_VALID | ScRefFlags::ROW_VALID))))
            {
                nRes1 = nRawRes1 | ScRefFlags::VALID;
                nRes2 = nRawRes2 | ScRefFlags::VALID;
                if (nRawRes1 & ScRefFlags::COL_VALID)
                {
                    rRange.aStart.SetRow(0);
                    rRange.aEnd.SetRow(MAXROW);
                    nRes1 |= ScRefFlags::ROW_VALID | ScRefFlags::ROW_ABS;
                    nRes2 |= ScRefFlags::ROW_VALID | ScRefFlags::ROW_ABS;
                }
                else
                {
                    rRange.aStart.SetCol(0);
                    rRange.aEnd.SetCol(MAXCOL);
                    nRes1 |= ScRefFlags::COL_VALID | ScRefFlags::COL_ABS;
                    nRes2 |= ScRefFlags::COL_VALID | ScRefFlags::COL_ABS;
                }
            }
            else if ((nRes1 & ScRefFlags::VALID) && (nRes2 & ScRefFlags::VALID))
            {
                // Flag entire column/row references so they can be displayed
                // as such. If the sticky reference parts are not both
                // absolute or relative, assume that the user thought about
                // something we should not touch.
                if (rRange.aStart.Row() == 0 && rRange.aEnd.Row() == MAXROW &&
                        ((nRes1 & ScRefFlags::ROW_ABS) == ScRefFlags::ZERO) &&
                        ((nRes2 & ScRefFlags::ROW_ABS) == ScRefFlags::ZERO))
                {
                    nRes1 |= ScRefFlags::ROW_ABS;
                    nRes2 |= ScRefFlags::ROW_ABS;
                }
                else if (rRange.aStart.Col() == 0 && rRange.aEnd.Col() == MAXCOL &&
                        ((nRes1 & ScRefFlags::COL_ABS) == ScRefFlags::ZERO) && ((nRes2 & ScRefFlags::COL_ABS) == ScRefFlags::ZERO))
                {
                    nRes1 |= ScRefFlags::COL_ABS;
                    nRes2 |= ScRefFlags::COL_ABS;
                }
            }
            if ((nRes1 & ScRefFlags::VALID) && (nRes2 & ScRefFlags::VALID))
            {
                // PutInOrder / Justify
                ScRefFlags nMask, nBits1, nBits2;
                SCCOL nTempCol;
                if ( rRange.aEnd.Col() < (nTempCol = rRange.aStart.Col()) )
                {
                    rRange.aStart.SetCol(rRange.aEnd.Col()); rRange.aEnd.SetCol(nTempCol);
                    nMask = (ScRefFlags::COL_VALID | ScRefFlags::COL_ABS);
                    nBits1 = nRes1 & nMask;
                    nBits2 = nRes2 & nMask;
                    nRes1 = (nRes1 & ~nMask) | nBits2;
                    nRes2 = (nRes2 & ~nMask) | nBits1;
                }
                SCROW nTempRow;
                if ( rRange.aEnd.Row() < (nTempRow = rRange.aStart.Row()) )
                {
                    rRange.aStart.SetRow(rRange.aEnd.Row()); rRange.aEnd.SetRow(nTempRow);
                    nMask = (ScRefFlags::ROW_VALID | ScRefFlags::ROW_ABS);
                    nBits1 = nRes1 & nMask;
                    nBits2 = nRes2 & nMask;
                    nRes1 = (nRes1 & ~nMask) | nBits2;
                    nRes2 = (nRes2 & ~nMask) | nBits1;
                }
                SCTAB nTempTab;
                if ( rRange.aEnd.Tab() < (nTempTab = rRange.aStart.Tab()) )
                {
                    rRange.aStart.SetTab(rRange.aEnd.Tab()); rRange.aEnd.SetTab(nTempTab);
                    nMask = (ScRefFlags::TAB_VALID | ScRefFlags::TAB_ABS | ScRefFlags::TAB_3D);
                    nBits1 = nRes1 & nMask;
                    nBits2 = nRes2 & nMask;
                    nRes1 = (nRes1 & ~nMask) | nBits2;
                    nRes2 = (nRes2 & ~nMask) | nBits1;
                }
                if ( ((nRes1 & ( ScRefFlags::TAB_ABS | ScRefFlags::TAB_3D ))
                            == ( ScRefFlags::TAB_ABS | ScRefFlags::TAB_3D ))
                  && !(nRes2 & ScRefFlags::TAB_3D) )
                    nRes2 |= ScRefFlags::TAB_ABS;
            }
            else
            {
                // Don't leave around valid half references.
                nRes1 = nRes2 = ScRefFlags::ZERO;
            }
        }
    }
    applyStartToEndFlags(nRes1, nRes2 & ScRefFlags::BITS);
    nRes1 |= nRes2 & ScRefFlags::VALID;
    return nRes1;
}

ScRefFlags ScRange::Parse( const OUString& rString, const ScDocument* pDoc,
                           const ScAddress::Details& rDetails,
                           ScAddress::ExternalInfo* pExtInfo,
                           const uno::Sequence<sheet::ExternalLinkInfo>* pExternalLinks,
                           const OUString* pErrRef )
{
    if (rString.isEmpty())
        return ScRefFlags::ZERO;

    switch (rDetails.eConv)
    {
        case formula::FormulaGrammar::CONV_XL_A1:
        case formula::FormulaGrammar::CONV_XL_OOX:
        {
            return lcl_ScRange_Parse_XL_A1( *this, rString.getStr(), pDoc, false, pExtInfo,
                    (rDetails.eConv == formula::FormulaGrammar::CONV_XL_OOX ? pExternalLinks : nullptr),
                    nullptr, pErrRef );
        }

        case formula::FormulaGrammar::CONV_XL_R1C1:
        {
            return lcl_ScRange_Parse_XL_R1C1( *this, rString.getStr(), pDoc, rDetails, false, pExtInfo, nullptr );
        }

        default:
        case formula::FormulaGrammar::CONV_OOO:
        {
            return lcl_ScRange_Parse_OOo( *this, rString, pDoc, pExtInfo, pErrRef );
        }
    }
}

// Accept a full range, or an address
ScRefFlags ScRange::ParseAny( const OUString& rString, const ScDocument* pDoc,
                              const ScAddress::Details& rDetails )
{
    ScRefFlags nRet = Parse( rString, pDoc, rDetails );
    const ScRefFlags nValid = ScRefFlags::VALID | ScRefFlags::COL2_VALID | ScRefFlags::ROW2_VALID | ScRefFlags::TAB2_VALID;

    if ( (nRet & nValid) != nValid )
    {
        ScAddress aAdr(aStart);//initialize with currentPos as fallback for table number
        nRet = aAdr.Parse( rString, pDoc, rDetails );
        if ( nRet & ScRefFlags::VALID )
            aStart = aEnd = aAdr;
    }
    return nRet;
}

// Parse only full row references
ScRefFlags ScRange::ParseCols( const OUString& rStr,
                               const ScAddress::Details& rDetails )
{
    if (rStr.isEmpty())
        return ScRefFlags::ZERO;

    const sal_Unicode* p = rStr.getStr();
    ScRefFlags nRes = ScRefFlags::ZERO;
    ScRefFlags ignored = ScRefFlags::ZERO;

    switch (rDetails.eConv)
    {
    default :
    case formula::FormulaGrammar::CONV_OOO: // No full col refs in OOO yet, assume XL notation
    case formula::FormulaGrammar::CONV_XL_A1:
    case formula::FormulaGrammar::CONV_XL_OOX:
        if (nullptr != (p = lcl_a1_get_col( p, &aStart, &ignored, nullptr) ) )
        {
            if( p[0] == ':')
            {
                if( nullptr != (p = lcl_a1_get_col( p+1, &aEnd, &ignored, nullptr)))
                {
                    nRes = ScRefFlags::COL_VALID;
                }
            }
            else
            {
                aEnd = aStart;
                nRes = ScRefFlags::COL_VALID;
            }
        }
        break;

    case formula::FormulaGrammar::CONV_XL_R1C1:
        if ((p[0] == 'C' || p[0] == 'c') &&
            nullptr != (p = lcl_r1c1_get_col( p, rDetails, &aStart, &ignored )))
        {
            if( p[0] == ':')
            {
                if( (p[1] == 'C' || p[1] == 'c') &&
                    nullptr != (p = lcl_r1c1_get_col( p+1, rDetails, &aEnd, &ignored )))
                {
                    nRes = ScRefFlags::COL_VALID;
                }
            }
            else
            {
                aEnd = aStart;
                nRes = ScRefFlags::COL_VALID;
            }
        }
        break;
    }

    return (p != nullptr && *p == '\0') ? nRes : ScRefFlags::ZERO;
}

// Parse only full row references
void ScRange::ParseRows( const OUString& rStr,
                               const ScAddress::Details& rDetails )
{
    if (rStr.isEmpty())
        return;

    const sal_Unicode* p = rStr.getStr();
    ScRefFlags ignored = ScRefFlags::ZERO;

    switch (rDetails.eConv)
    {
    default :
    case formula::FormulaGrammar::CONV_OOO: // No full row refs in OOO yet, assume XL notation
    case formula::FormulaGrammar::CONV_XL_A1:
    case formula::FormulaGrammar::CONV_XL_OOX:
        if (nullptr != (p = lcl_a1_get_row( p, &aStart, &ignored, nullptr) ) )
        {
            if( p[0] == ':')
            {
                p = lcl_a1_get_row( p+1, &aEnd, &ignored, nullptr);
            }
            else
            {
                aEnd = aStart;
            }
        }
        break;

    case formula::FormulaGrammar::CONV_XL_R1C1:
        if ((p[0] == 'R' || p[0] == 'r') &&
            nullptr != (p = lcl_r1c1_get_row( p, rDetails, &aStart, &ignored )))
        {
            if( p[0] == ':')
            {
                if( p[1] == 'R' || p[1] == 'r' )
                {
                    p = lcl_r1c1_get_row( p+1, rDetails, &aEnd, &ignored );
                }
            }
            else
            {
                aEnd = aStart;
            }
        }
        break;
    }
}

template<typename T > static void lcl_ScColToAlpha( T& rBuf, SCCOL nCol )
{
    if (nCol < 26*26)
    {
        if (nCol < 26)
            rBuf.append( static_cast<char>( 'A' + nCol ));
        else
        {
            rBuf.append( static_cast<char>( 'A' + nCol / 26 - 1 ));
            rBuf.append( static_cast<char>( 'A' + nCol % 26 ));
        }
    }
    else
    {
        sal_Int32 nInsert = rBuf.getLength();
        while (nCol >= 26)
        {
            SCCOL nC = nCol % 26;
            rBuf.insert(nInsert, static_cast<char> ( 'A' + nC ));
            nCol = sal::static_int_cast<SCCOL>( nCol - nC );
            nCol = nCol / 26 - 1;
        }
        rBuf.insert(nInsert, static_cast<char> ( 'A' + nCol ));
    }
}

void ScColToAlpha( OUStringBuffer& rBuf, SCCOL nCol)
{
    lcl_ScColToAlpha(rBuf, nCol);
}

template <typename T > static void lcl_a1_append_c ( T &rString, int nCol, bool bIsAbs )
{
    if( bIsAbs )
        rString.append("$");
    lcl_ScColToAlpha( rString, sal::static_int_cast<SCCOL>(nCol) );
}

template <typename T > static void lcl_a1_append_r ( T &rString, sal_Int32 nRow, bool bIsAbs )
{
    if ( bIsAbs )
        rString.append("$");
    rString.append( nRow + 1 );
}

template <typename T > static void lcl_r1c1_append_c ( T &rString, sal_Int32 nCol, bool bIsAbs,
                                       const ScAddress::Details& rDetails )
{
    rString.append("C");
    if (bIsAbs)
    {
        rString.append( nCol + 1 );
    }
    else
    {
        nCol -= rDetails.nCol;
        if (nCol != 0) {
            rString.append("[").append(nCol).append("]");
        }
    }
}

template <typename T > static void lcl_r1c1_append_r ( T &rString, sal_Int32 nRow, bool bIsAbs,
                                       const ScAddress::Details& rDetails )
{
    rString.append("R");
    if (bIsAbs)
    {
        rString.append( nRow + 1 );
    }
    else
    {
        nRow -= rDetails.nRow;
        if (nRow != 0) {
            rString.append("[").append(nRow).append("]");
        }
    }
}

static OUString getFileNameFromDoc( const ScDocument* pDoc )
{
    // TODO : er points at ScGlobal::GetAbsDocName()
    // as a better template.  Look into it
    OUString sFileName;
    SfxObjectShell* pShell;

    if( nullptr != pDoc &&
        nullptr != (pShell = pDoc->GetDocumentShell() ) )
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


static void lcl_string_append(OUStringBuffer &rString, const OUString &sString)
{
    rString.append(sString);
}

static void lcl_string_append(OStringBuffer &rString, const OUString &sString)
{
    rString.append(OUStringToOString( sString, RTL_TEXTENCODING_UTF8  ));
}

template<typename T > static void lcl_Format( T& r, SCTAB nTab, SCROW nRow, SCCOL nCol, ScRefFlags nFlags,
                                  const ScDocument* pDoc,
                                  const ScAddress::Details& rDetails)
{
    if( nFlags & ScRefFlags::VALID )
        nFlags |= ScRefFlags::ROW_VALID | ScRefFlags::COL_VALID | ScRefFlags::TAB_VALID;
    if( pDoc && (nFlags & ScRefFlags::TAB_VALID ) )
    {
        if ( nTab < 0 || nTab >= pDoc->GetTableCount() )
        {
            lcl_string_append(r, ScCompiler::GetNativeSymbol(ocErrRef));
            return;
        }
        if( nFlags & ScRefFlags::TAB_3D )
        {
            OUString aTabName, aDocName;
            pDoc->GetName(nTab, aTabName);
            assert( !aTabName.isEmpty() && "empty sheet name");
            // External Reference, same as in ScCompiler::MakeTabStr()
            if( aTabName[0] == '\'' )
            {   // "'Doc'#Tab"
                sal_Int32 nPos = ScCompiler::GetDocTabPos( aTabName);
                if (nPos != -1)
                {
                    aDocName = aTabName.copy( 0, nPos + 1 );
                    aTabName = aTabName.copy( nPos + 1 );
                }
            }
            else if( nFlags & ScRefFlags::FORCE_DOC )
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
                lcl_string_append(r, aDocName);
                if( nFlags & ScRefFlags::TAB_ABS )
                    r.append("$");
                lcl_string_append(r, aTabName);
                r.append(".");
                break;

            case formula::FormulaGrammar::CONV_XL_OOX:
                if (!aTabName.isEmpty() && aTabName[0] == '\'')
                {
                    if (!aDocName.isEmpty())
                    {
                        lcl_string_append(r.append("'["), aDocName);
                        r.append("]");
                        lcl_string_append(r, aTabName.copy(1));
                    }
                    else
                    {
                        lcl_string_append(r, aTabName);
                    }
                    r.append("!");
                    break;
                }
                [[fallthrough]];
            case formula::FormulaGrammar::CONV_XL_A1:
            case formula::FormulaGrammar::CONV_XL_R1C1:
                if (!aDocName.isEmpty())
                {
                    lcl_string_append(r.append("["), aDocName);
                    r.append("]");
                }
                lcl_string_append(r, aTabName);
                r.append("!");
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
        if( nFlags & ScRefFlags::COL_VALID )
            lcl_a1_append_c ( r, nCol, (nFlags & ScRefFlags::COL_ABS) != ScRefFlags::ZERO );
        if( nFlags & ScRefFlags::ROW_VALID )
            lcl_a1_append_r ( r, nRow, (nFlags & ScRefFlags::ROW_ABS) != ScRefFlags::ZERO );
        break;

    case formula::FormulaGrammar::CONV_XL_R1C1:
        if( nFlags & ScRefFlags::ROW_VALID )
            lcl_r1c1_append_r ( r, nRow, (nFlags & ScRefFlags::ROW_ABS) != ScRefFlags::ZERO, rDetails );
        if( nFlags & ScRefFlags::COL_VALID )
            lcl_r1c1_append_c ( r, nCol, (nFlags & ScRefFlags::COL_ABS) != ScRefFlags::ZERO, rDetails );
        break;
    }
}

void ScAddress::Format( OStringBuffer& r, ScRefFlags nFlags,
                                  const ScDocument* pDoc,
                                  const Details& rDetails) const
{
    lcl_Format(r, nTab, nRow, nCol, nFlags, pDoc, rDetails);
}

OUString ScAddress::Format(ScRefFlags nFlags, const ScDocument* pDoc,
                           const Details& rDetails) const
{
    OUStringBuffer r;
    lcl_Format(r, nTab, nRow, nCol, nFlags, pDoc, rDetails);
    return r.makeStringAndClear();
}

static void lcl_Split_DocTab( const ScDocument* pDoc,  SCTAB nTab,
                              const ScAddress::Details& rDetails,
                              ScRefFlags nFlags,
                              OUString& rTabName, OUString& rDocName )
{
    pDoc->GetName(nTab, rTabName);
    rDocName.clear();
    // External reference, same as in ScCompiler::MakeTabStr()
    if ( rTabName[0] == '\'' )
    {   // "'Doc'#Tab"
        sal_Int32 nPos = ScCompiler::GetDocTabPos( rTabName);
        if (nPos != -1)
        {
            rDocName = rTabName.copy( 0, nPos + 1 );
            rTabName = rTabName.copy( nPos + 1 );
        }
    }
    else if( nFlags & ScRefFlags::FORCE_DOC )
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

static void lcl_ScRange_Format_XL_Header( OUStringBuffer& rString, const ScRange& rRange,
                                          ScRefFlags nFlags, const ScDocument* pDoc,
                                          const ScAddress::Details& rDetails )
{
    if( nFlags & ScRefFlags::TAB_3D )
    {
        OUString aTabName, aDocName;
        lcl_Split_DocTab( pDoc, rRange.aStart.Tab(), rDetails, nFlags, aTabName, aDocName );
        switch (rDetails.eConv)
        {
            case formula::FormulaGrammar::CONV_XL_OOX:
                if (!aTabName.isEmpty() && aTabName[0] == '\'')
                {
                    if (!aDocName.isEmpty())
                    {
                        rString.append("'[").append(aDocName).append("]").append(std::u16string_view(aTabName).substr(1));
                    }
                    else
                    {
                        rString.append(aTabName);
                    }
                    break;
                }
                [[fallthrough]];
            default:
                if (!aDocName.isEmpty())
                {
                    rString.append("[").append(aDocName).append("]");
                }
                rString.append(aTabName);
            break;
        }
        if( nFlags & ScRefFlags::TAB2_3D )
        {
            lcl_Split_DocTab( pDoc, rRange.aEnd.Tab(), rDetails, nFlags, aTabName, aDocName );
            rString.append(":");
            rString.append(aTabName);
        }
        rString.append("!");
    }
}

// helpers used in ScRange::Format
static bool lcl_ColAbsFlagDiffer(const ScRefFlags nFlags)
{
    return static_cast<bool>(nFlags & ScRefFlags::COL_ABS) != static_cast<bool>(nFlags & ScRefFlags::COL2_ABS);
}
static bool lcl_RowAbsFlagDiffer(const ScRefFlags nFlags)
{
    return static_cast<bool>(nFlags & ScRefFlags::ROW_ABS) != static_cast<bool>(nFlags & ScRefFlags::ROW2_ABS);
}

OUString ScRange::Format( ScRefFlags nFlags, const ScDocument* pDoc,
                          const ScAddress::Details& rDetails, bool bFullAddressNotation ) const
{
    if( !( nFlags & ScRefFlags::VALID ) )
    {
        return ScCompiler::GetNativeSymbol(ocErrRef);
    }

    OUStringBuffer r;
    switch( rDetails.eConv ) {
    default :
    case formula::FormulaGrammar::CONV_OOO: {
        bool bOneTab = (aStart.Tab() == aEnd.Tab());
        if ( !bOneTab )
            nFlags |= ScRefFlags::TAB_3D;
        r = aStart.Format(nFlags, pDoc, rDetails);
        if( aStart != aEnd ||
            lcl_ColAbsFlagDiffer( nFlags ) ||
            lcl_RowAbsFlagDiffer( nFlags ))
        {
            // move flags of end reference to start reference, mask with BITS to exclude FORCE_DOC flag
            nFlags = ScRefFlags::VALID | (ScRefFlags(o3tl::underlyingEnumValue(nFlags) >> 4) & ScRefFlags::BITS);
            if ( bOneTab )
                pDoc = nullptr;
            else
                nFlags |= ScRefFlags::TAB_3D;
            OUString aName(aEnd.Format(nFlags, pDoc, rDetails));
            r.append(":");
            r.append(aName);
        }
    }
    break;

    case formula::FormulaGrammar::CONV_XL_A1:
    case formula::FormulaGrammar::CONV_XL_OOX:
        lcl_ScRange_Format_XL_Header( r, *this, nFlags, pDoc, rDetails );
        if( aStart.Col() == 0 && aEnd.Col() >= MAXCOL && !bFullAddressNotation )
        {
            // Full col refs always require 2 rows (2:2)
            lcl_a1_append_r( r, aStart.Row(), (nFlags & ScRefFlags::ROW_ABS) != ScRefFlags::ZERO );
            r.append(":");
            lcl_a1_append_r( r, aEnd.Row(), (nFlags & ScRefFlags::ROW2_ABS) != ScRefFlags::ZERO );
        }
        else if( aStart.Row() == 0 && aEnd.Row() >= MAXROW && !bFullAddressNotation )
        {
            // Full row refs always require 2 cols (A:A)
            lcl_a1_append_c( r, aStart.Col(), (nFlags & ScRefFlags::COL_ABS) != ScRefFlags::ZERO );
            r.append(":");
            lcl_a1_append_c( r, aEnd.Col(), (nFlags & ScRefFlags::COL2_ABS) != ScRefFlags::ZERO );
        }
        else
        {
            lcl_a1_append_c ( r, aStart.Col(), (nFlags & ScRefFlags::COL_ABS) != ScRefFlags::ZERO );
            lcl_a1_append_r ( r, aStart.Row(), (nFlags & ScRefFlags::ROW_ABS) != ScRefFlags::ZERO );
            if( aStart.Col() != aEnd.Col() ||
                lcl_ColAbsFlagDiffer( nFlags ) ||
                aStart.Row() != aEnd.Row() ||
                lcl_RowAbsFlagDiffer( nFlags ) ) {
                r.append(":");
                lcl_a1_append_c ( r, aEnd.Col(), (nFlags & ScRefFlags::COL2_ABS) != ScRefFlags::ZERO );
                lcl_a1_append_r ( r, aEnd.Row(), (nFlags & ScRefFlags::ROW2_ABS) != ScRefFlags::ZERO );
            }
        }
    break;

    case formula::FormulaGrammar::CONV_XL_R1C1:
        lcl_ScRange_Format_XL_Header( r, *this, nFlags, pDoc, rDetails );
        if( aStart.Col() == 0 && aEnd.Col() >= MAXCOL && !bFullAddressNotation )
        {
            lcl_r1c1_append_r( r, aStart.Row(), (nFlags & ScRefFlags::ROW_ABS) != ScRefFlags::ZERO, rDetails );
            if( aStart.Row() != aEnd.Row() ||
                lcl_RowAbsFlagDiffer( nFlags ) ) {
                r.append(":");
                lcl_r1c1_append_r( r, aEnd.Row(), (nFlags & ScRefFlags::ROW2_ABS) != ScRefFlags::ZERO, rDetails );
            }
        }
        else if( aStart.Row() == 0 && aEnd.Row() >= MAXROW && !bFullAddressNotation )
        {
            lcl_r1c1_append_c( r, aStart.Col(), (nFlags & ScRefFlags::COL_ABS) != ScRefFlags::ZERO, rDetails );
            if( aStart.Col() != aEnd.Col() ||
                lcl_ColAbsFlagDiffer( nFlags )) {
                r.append(":");
                lcl_r1c1_append_c( r, aEnd.Col(), (nFlags & ScRefFlags::COL2_ABS) != ScRefFlags::ZERO, rDetails );
            }
        }
        else
        {
            lcl_r1c1_append_r( r, aStart.Row(), (nFlags & ScRefFlags::ROW_ABS) != ScRefFlags::ZERO, rDetails );
            lcl_r1c1_append_c( r, aStart.Col(), (nFlags & ScRefFlags::COL_ABS) != ScRefFlags::ZERO, rDetails );
            if( aStart.Col() != aEnd.Col() ||
                lcl_ColAbsFlagDiffer( nFlags ) ||
                aStart.Row() != aEnd.Row() ||
                lcl_RowAbsFlagDiffer( nFlags ) ) {
                r.append(":");
                lcl_r1c1_append_r( r, aEnd.Row(), (nFlags & ScRefFlags::ROW2_ABS) != ScRefFlags::ZERO, rDetails );
                lcl_r1c1_append_c( r, aEnd.Col(), (nFlags & ScRefFlags::COL2_ABS) != ScRefFlags::ZERO, rDetails );
            }
        }
    }
    return r.makeStringAndClear();
}

bool ScAddress::Move( SCCOL dx, SCROW dy, SCTAB dz, ScAddress& rErrorPos, const ScDocument* pDoc )
{
    SCTAB nMaxTab = pDoc ? pDoc->GetTableCount() : MAXTAB;
    dx = Col() + dx;
    dy = Row() + dy;
    dz = Tab() + dz;
    bool bValid = true;
    rErrorPos.SetCol(dx);
    if( dx < 0 )
    {
        dx = 0;
        bValid = false;
    }
    else if( dx > MAXCOL )
    {
        dx = MAXCOL;
        bValid =false;
    }
    rErrorPos.SetRow(dy);
    if( dy < 0 )
    {
        dy = 0;
        bValid = false;
    }
    else if( dy > MAXROW )
    {
        dy = MAXROW;
        bValid =false;
    }
    rErrorPos.SetTab(dz);
    if( dz < 0 )
    {
        dz = 0;
        bValid = false;
    }
    else if( dz > nMaxTab )
    {
        // Always set MAXTAB+1 so further checks without ScDocument detect invalid.
        rErrorPos.SetTab(MAXTAB+1);
        dz = nMaxTab;
        bValid =false;
    }
    Set( dx, dy, dz );
    return bValid;
}

bool ScRange::Move( SCCOL dx, SCROW dy, SCTAB dz, ScRange& rErrorRange, const ScDocument* pDoc )
{
    if (dy && aStart.Row() == 0 && aEnd.Row() == MAXROW)
        dy = 0;     // Entire column not to be moved.
    if (dx && aStart.Col() == 0 && aEnd.Col() == MAXCOL)
        dx = 0;     // Entire row not to be moved.
    bool b = aStart.Move( dx, dy, dz, rErrorRange.aStart, pDoc );
    b &= aEnd.Move( dx, dy, dz, rErrorRange.aEnd, pDoc );
    return b;
}

bool ScRange::MoveSticky( SCCOL dx, SCROW dy, SCTAB dz, ScRange& rErrorRange )
{
    bool bColRange = (aStart.Col() < aEnd.Col());
    bool bRowRange = (aStart.Row() < aEnd.Row());
    if (dy && aStart.Row() == 0 && aEnd.Row() == MAXROW)
        dy = 0;     // Entire column not to be moved.
    if (dx && aStart.Col() == 0 && aEnd.Col() == MAXCOL)
        dx = 0;     // Entire row not to be moved.
    bool b1 = aStart.Move( dx, dy, dz, rErrorRange.aStart );
    if (dx && bColRange && aEnd.Col() == MAXCOL)
        dx = 0;     // End column sticky.
    if (dy && bRowRange && aEnd.Row() == MAXROW)
        dy = 0;     // End row sticky.
    SCTAB nOldTab = aEnd.Tab();
    bool b2 = aEnd.Move( dx, dy, dz, rErrorRange.aEnd );
    if (!b2)
    {
        // End column or row of a range may have become sticky.
        bColRange = (!dx || (bColRange && aEnd.Col() == MAXCOL));
        if (dx && bColRange)
            rErrorRange.aEnd.SetCol(MAXCOL);
        bRowRange = (!dy || (bRowRange && aEnd.Row() == MAXROW));
        if (dy && bRowRange)
            rErrorRange.aEnd.SetRow(MAXROW);
        b2 = bColRange && bRowRange && (aEnd.Tab() - nOldTab == dz);
    }
    return b1 && b2;
}

void ScRange::IncColIfNotLessThan(SCCOL nStartCol, SCCOL nOffset)
{
    if (aStart.Col() >= nStartCol)
    {
        aStart.IncCol(nOffset);
        if (aStart.Col() < 0)
            aStart.SetCol(0);
        else if(aStart.Col() > MAXCOL)
            aStart.SetCol(MAXCOL);
    }
    if (aEnd.Col() >= nStartCol)
    {
        aEnd.IncCol(nOffset);
        if (aEnd.Col() < 0)
            aEnd.SetCol(0);
        else if(aEnd.Col() > MAXCOL)
            aEnd.SetCol(MAXCOL);
    }
}

void ScRange::IncRowIfNotLessThan(SCROW nStartRow, SCROW nOffset)
{
    if (aStart.Row() >= nStartRow)
    {
        aStart.IncRow(nOffset);
        if (aStart.Row() < 0)
            aStart.SetRow(0);
        else if(aStart.Row() > MAXROW)
            aStart.SetRow(MAXROW);
    }
    if (aEnd.Row() >= nStartRow)
    {
        aEnd.IncRow(nOffset);
        if (aEnd.Row() < 0)
            aEnd.SetRow(0);
        else if(aEnd.Row() > MAXROW)
            aEnd.SetRow(MAXROW);
    }
}

void ScRange::IncEndColSticky( SCCOL nDelta )
{
    SCCOL nCol = aEnd.Col();
    if (aStart.Col() >= nCol)
    {
        // Less than two columns => not sticky.
        aEnd.IncCol( nDelta);
        return;
    }

    if (nCol == MAXCOL)
        // already sticky
        return;

    if (nCol < MAXCOL)
        aEnd.SetCol( ::std::min( static_cast<SCCOL>(nCol + nDelta), MAXCOL));
    else
        aEnd.IncCol( nDelta);   // was greater than MAXCOL, caller should know..
}

void ScRange::IncEndRowSticky( SCROW nDelta )
{
    SCROW nRow = aEnd.Row();
    if (aStart.Row() >= nRow)
    {
        // Less than two rows => not sticky.
        aEnd.IncRow( nDelta);
        return;
    }

    if (nRow == MAXROW)
        // already sticky
        return;

    if (nRow < MAXROW)
        aEnd.SetRow( ::std::min( static_cast<SCROW>(nRow + nDelta), MAXROW));
    else
        aEnd.IncRow( nDelta);   // was greater than MAXROW, caller should know..
}

OUString ScAddress::GetColRowString() const
{
    OUStringBuffer aString;

    switch( detailsOOOa1.eConv )
    {
    default :
    case formula::FormulaGrammar::CONV_OOO:
    case formula::FormulaGrammar::CONV_XL_A1:
    case formula::FormulaGrammar::CONV_XL_OOX:
        lcl_ScColToAlpha( aString, nCol);
        aString.append(OUString::number(nRow+1));
        break;

    case formula::FormulaGrammar::CONV_XL_R1C1:
        lcl_r1c1_append_r ( aString, nRow, false/*bAbsolute*/, detailsOOOa1 );
        lcl_r1c1_append_c ( aString, nCol, false/*bAbsolute*/, detailsOOOa1 );
        break;
    }

    return aString.makeStringAndClear();
}

OUString ScRefAddress::GetRefString( const ScDocument* pDoc, SCTAB nActTab,
                                   const ScAddress::Details& rDetails ) const
{
    if ( !pDoc )
        return EMPTY_OUSTRING;
    if ( Tab()+1 > pDoc->GetTableCount() )
        return ScCompiler::GetNativeSymbol(ocErrRef);

    ScRefFlags nFlags = ScRefFlags::VALID;
    if ( nActTab != Tab() )
    {
        nFlags |= ScRefFlags::TAB_3D;
        if ( !bRelTab )
            nFlags |= ScRefFlags::TAB_ABS;
    }
    if ( !bRelCol )
        nFlags |= ScRefFlags::COL_ABS;
    if ( !bRelRow )
        nFlags |= ScRefFlags::ROW_ABS;

    return aAdr.Format(nFlags, pDoc, rDetails);
}

bool AlphaToCol( SCCOL& rCol, const OUString& rStr)
{
    SCCOL nResult = 0;
    sal_Int32 nStop = rStr.getLength();
    sal_Int32 nPos = 0;
    sal_Unicode c;
    while (nResult <= MAXCOL && nPos < nStop && (c = rStr[nPos]) != 0 &&
            rtl::isAsciiAlpha(c))
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
