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

#include "rangeutl.hxx"
#include "document.hxx"
#include "global.hxx"
#include "dbdata.hxx"
#include "rangenam.hxx"
#include "scresid.hxx"
#include "globstr.hrc"
#include "convuno.hxx"
#include "externalrefmgr.hxx"
#include "compiler.hxx"

using ::formula::FormulaGrammar;
using namespace ::com::sun::star;

sal_Bool ScRangeUtil::MakeArea( const String&   rAreaStr,
                            ScArea&         rArea,
                            ScDocument*     pDoc,
                            SCTAB           nTab,
                            ScAddress::Details const & rDetails ) const
{
    // Input in rAreaStr: "$Tabelle1.$A1:$D17"

    // BROKEN BROKEN BROKEN
    // but it is only used in the consolidate dialog.  Ignore for now.

    sal_Bool        nSuccess    = false;
    sal_uInt16      nPointPos   = rAreaStr.Search('.');
    sal_uInt16      nColonPos   = rAreaStr.Search(':');
    String      aStrArea( rAreaStr );
    ScRefAddress    startPos;
    ScRefAddress    endPos;

    if ( nColonPos == STRING_NOTFOUND )
        if ( nPointPos != STRING_NOTFOUND )
        {
            aStrArea += ':';
            aStrArea += rAreaStr.Copy( nPointPos+1 ); // do not include '.' in copy
        }

    nSuccess = ConvertDoubleRef( pDoc, aStrArea, nTab, startPos, endPos, rDetails );

    if ( nSuccess )
        rArea = ScArea( startPos.Tab(),
                        startPos.Col(), startPos.Row(),
                        endPos.Col(),   endPos.Row() );

    return nSuccess;
}

void ScRangeUtil::CutPosString( const String&   theAreaStr,
                                String&         thePosStr ) const
{
    String  aPosStr;
    // BROKEN BROKEN BROKEN
    // but it is only used in the consolidate dialog.  Ignore for now.

    sal_uInt16  nColonPos = theAreaStr.Search(':');

    if ( nColonPos != STRING_NOTFOUND )
        aPosStr = theAreaStr.Copy( 0, nColonPos ); // do not include ':' in copy
    else
        aPosStr = theAreaStr;

    thePosStr = aPosStr;
}

sal_Bool ScRangeUtil::IsAbsTabArea( const String&   rAreaStr,
                                ScDocument*     pDoc,
                                ScArea***       pppAreas,
                                sal_uInt16*         pAreaCount,
                                sal_Bool            /* bAcceptCellRef */,
                                ScAddress::Details const & rDetails ) const
{
    OSL_ENSURE( pDoc, "No document given!" );
    if ( !pDoc )
        return false;

    // BROKEN BROKEN BROKEN
    // but it is only used in the consolidate dialog.  Ignore for now.

    /*
     * Expects strings like:
     *      "$Tabelle1.$A$1:$Tabelle3.$D$17"
     * If bAcceptCellRef == sal_True then also accept strings like:
     *      "$Tabelle1.$A$1"
     *
     * as result a ScArea-Array is created,
     * which is published via ppAreas and also has to be deleted this route.
     */

    sal_Bool    bStrOk = false;
    String  aTempAreaStr(rAreaStr);
    String  aStartPosStr;
    String  aEndPosStr;

    if ( STRING_NOTFOUND == aTempAreaStr.Search(':') )
    {
        aTempAreaStr.Append(':');
        aTempAreaStr.Append(rAreaStr);
    }

    sal_uInt16   nColonPos = aTempAreaStr.Search(':');

    if (   STRING_NOTFOUND != nColonPos
        && STRING_NOTFOUND != aTempAreaStr.Search('.') )
    {
        ScRefAddress    aStartPos;
        ScRefAddress    aEndPos;

        aStartPosStr = aTempAreaStr.Copy( 0,           nColonPos  );
        aEndPosStr   = aTempAreaStr.Copy( nColonPos+1, STRING_LEN );

        if ( ConvertSingleRef( pDoc, aStartPosStr, 0, aStartPos, rDetails ) )
        {
            if ( ConvertSingleRef( pDoc, aEndPosStr, aStartPos.Tab(), aEndPos, rDetails ) )
            {
                aStartPos.SetRelCol( false );
                aStartPos.SetRelRow( false );
                aStartPos.SetRelTab( false );
                aEndPos.SetRelCol( false );
                aEndPos.SetRelRow( false );
                aEndPos.SetRelTab( false );

                bStrOk = sal_True;

                if ( pppAreas && pAreaCount ) // Array returned ?
                {
                    SCTAB       nStartTab   = aStartPos.Tab();
                    SCTAB       nEndTab     = aEndPos.Tab();
                    sal_uInt16      nTabCount   = static_cast<sal_uInt16>(nEndTab-nStartTab+1);
                    ScArea**    theAreas    = new ScArea*[nTabCount];
                    SCTAB       nTab        = 0;
                    sal_uInt16      i           = 0;
                    ScArea      theArea( 0, aStartPos.Col(), aStartPos.Row(),
                                            aEndPos.Col(), aEndPos.Row() );

                    nTab = nStartTab;
                    for ( i=0; i<nTabCount; i++ )
                    {
                        theAreas[i] = new ScArea( theArea );
                        theAreas[i]->nTab = nTab;
                        nTab++;
                    }
                    *pppAreas   = theAreas;
                    *pAreaCount = nTabCount;
                }
            }
        }
    }

    return bStrOk;
}

sal_Bool ScRangeUtil::IsAbsArea( const String&  rAreaStr,
                             ScDocument*    pDoc,
                             SCTAB          nTab,
                             String*        pCompleteStr,
                             ScRefAddress*  pStartPos,
                             ScRefAddress*  pEndPos,
                             ScAddress::Details const & rDetails ) const
{
    ScRefAddress    startPos;
    ScRefAddress    endPos;

    sal_Bool bIsAbsArea = ConvertDoubleRef( pDoc, rAreaStr, nTab, startPos, endPos, rDetails );

    if ( bIsAbsArea )
    {
        startPos.SetRelCol( false );
        startPos.SetRelRow( false );
        startPos.SetRelTab( false );
        endPos  .SetRelCol( false );
        endPos  .SetRelRow( false );
        endPos  .SetRelTab( false );

        if ( pCompleteStr )
        {
            *pCompleteStr  = startPos.GetRefString( pDoc, MAXTAB+1, rDetails );
            *pCompleteStr += ':';
            *pCompleteStr += endPos  .GetRefString( pDoc, nTab, rDetails );
        }

        if ( pStartPos && pEndPos )
        {
            *pStartPos = startPos;
            *pEndPos   = endPos;
        }
    }

    return bIsAbsArea;
}

sal_Bool ScRangeUtil::IsAbsPos( const String&   rPosStr,
                            ScDocument*     pDoc,
                            SCTAB           nTab,
                            String*         pCompleteStr,
                            ScRefAddress*   pPosTripel,
                            ScAddress::Details const & rDetails ) const
{
    ScRefAddress    thePos;

    sal_Bool bIsAbsPos = ConvertSingleRef( pDoc, rPosStr, nTab, thePos, rDetails );
    thePos.SetRelCol( false );
    thePos.SetRelRow( false );
    thePos.SetRelTab( false );

    if ( bIsAbsPos )
    {
        if ( pPosTripel )
            *pPosTripel = thePos;
        if ( pCompleteStr )
            *pCompleteStr = thePos.GetRefString( pDoc, MAXTAB+1, rDetails );
    }

    return bIsAbsPos;
}

sal_Bool ScRangeUtil::MakeRangeFromName (
    const String&   rName,
    ScDocument*     pDoc,
    SCTAB           nCurTab,
    ScRange&        rRange,
    RutlNameScope   eScope,
    ScAddress::Details const & rDetails ) const
{
    sal_Bool bResult=false;
    ScRangeUtil     aRangeUtil;
    SCTAB nTab = 0;
    SCCOL nColStart = 0;
    SCCOL nColEnd = 0;
    SCROW nRowStart = 0;
    SCROW nRowEnd = 0;

    if( eScope==RUTL_NAMES )
    {
        //first handle ui names like local1 (Sheet1), which point to a local range name
        OUString aName(rName);
        sal_Int32 nEndPos = aName.lastIndexOf(')');
        sal_Int32 nStartPos = aName.lastIndexOfAsciiL(" (",2);
        SCTAB nTable = nCurTab;
        if (nEndPos != -1 && nStartPos != -1)
        {
            OUString aSheetName = aName.copy(nStartPos+2, nEndPos-nStartPos-2);
            if (pDoc->GetTable(aSheetName, nTable))
            {
                aName = aName.copy(0, nStartPos);
            }
            else
                nTable = nCurTab;
        }
        //then check for local range names
        ScRangeName* pRangeNames = pDoc->GetRangeName( nTable );
        ScRangeData* pData = NULL;
        if ( pRangeNames )
            pData = pRangeNames->findByUpperName(ScGlobal::pCharClass->uppercase(aName));
        if (!pData)
            pData = pDoc->GetRangeName()->findByUpperName(ScGlobal::pCharClass->uppercase(aName));
        if (pData)
        {
            String       aStrArea;
            ScRefAddress     aStartPos;
            ScRefAddress     aEndPos;

            pData->GetSymbol( aStrArea );

            if ( IsAbsArea( aStrArea, pDoc, nTable,
                            NULL, &aStartPos, &aEndPos, rDetails ) )
            {
                nTab       = aStartPos.Tab();
                nColStart  = aStartPos.Col();
                nRowStart  = aStartPos.Row();
                nColEnd    = aEndPos.Col();
                nRowEnd    = aEndPos.Row();
                bResult    = sal_True;
            }
            else
            {
                CutPosString( aStrArea, aStrArea );

                if ( IsAbsPos( aStrArea, pDoc, nTable,
                                          NULL, &aStartPos, rDetails ) )
                {
                    nTab       = aStartPos.Tab();
                    nColStart  = nColEnd = aStartPos.Col();
                    nRowStart  = nRowEnd = aStartPos.Row();
                    bResult    = sal_True;
                }
            }
        }
    }
    else if( eScope==RUTL_DBASE )
    {
        ScDBCollection::NamedDBs& rDbNames = pDoc->GetDBCollection()->getNamedDBs();
        ScDBData* pData = rDbNames.findByUpperName(ScGlobal::pCharClass->uppercase(rName));
        if (pData)
        {
            pData->GetArea(nTab, nColStart, nRowStart, nColEnd, nRowEnd);
            bResult = true;
        }
    }
    else
    {
        OSL_FAIL( "ScRangeUtil::MakeRangeFromName" );
    }

    if( bResult )
    {
        rRange = ScRange( nColStart, nRowStart, nTab, nColEnd, nRowEnd, nTab );
    }

    return bResult;
}

void ScRangeStringConverter::AssignString(
        OUString& rString,
        const OUString& rNewStr,
        sal_Bool bAppendStr,
        sal_Unicode cSeparator)
{
    if( bAppendStr )
    {
        if( !rNewStr.isEmpty() )
        {
            if( !rString.isEmpty() )
                rString += OUString(cSeparator);
            rString += rNewStr;
        }
    }
    else
        rString = rNewStr;
}

sal_Int32 ScRangeStringConverter::IndexOf(
        const OUString& rString,
        sal_Unicode cSearchChar,
        sal_Int32 nOffset,
        sal_Unicode cQuote )
{
    sal_Int32       nLength     = rString.getLength();
    sal_Int32       nIndex      = nOffset;
    sal_Bool        bQuoted     = false;
    sal_Bool        bExitLoop   = false;

    while( !bExitLoop && (nIndex >= 0 && nIndex < nLength) )
    {
        sal_Unicode cCode = rString[ nIndex ];
        bExitLoop = (cCode == cSearchChar) && !bQuoted;
        bQuoted = (bQuoted != (cCode == cQuote));
        if( !bExitLoop )
            nIndex++;
    }
    return (nIndex < nLength) ? nIndex : -1;
}

sal_Int32 ScRangeStringConverter::IndexOfDifferent(
        const OUString& rString,
        sal_Unicode cSearchChar,
        sal_Int32 nOffset )
{
    sal_Int32       nLength     = rString.getLength();
    sal_Int32       nIndex      = nOffset;
    sal_Bool        bExitLoop   = false;

    while( !bExitLoop && (nIndex >= 0 && nIndex < nLength) )
    {
        bExitLoop = (rString[ nIndex ] != cSearchChar);
        if( !bExitLoop )
            nIndex++;
    }
    return (nIndex < nLength) ? nIndex : -1;
}

void ScRangeStringConverter::GetTokenByOffset(
        OUString& rToken,
        const OUString& rString,
        sal_Int32& nOffset,
        sal_Unicode cSeparator,
        sal_Unicode cQuote)
{
    sal_Int32 nLength = rString.getLength();
    if( nOffset >= nLength )
    {
        rToken = OUString();
        nOffset = -1;
    }
    else
    {
        sal_Int32 nTokenEnd = IndexOf( rString, cSeparator, nOffset, cQuote );
        if( nTokenEnd < 0 )
            nTokenEnd = nLength;
        rToken = rString.copy( nOffset, nTokenEnd - nOffset );

        sal_Int32 nNextBegin = IndexOfDifferent( rString, cSeparator, nTokenEnd );
        nOffset = (nNextBegin < 0) ? nLength : nNextBegin;
    }
}

void ScRangeStringConverter::AppendTableName(OUStringBuffer& rBuf, const OUString& rTabName, sal_Unicode /* cQuote */)
{
    // quote character is always "'"
    String aQuotedTab(rTabName);
    ScCompiler::CheckTabQuotes(aQuotedTab, ::formula::FormulaGrammar::CONV_OOO);
    rBuf.append(aQuotedTab);
}

sal_Int32 ScRangeStringConverter::GetTokenCount( const OUString& rString, sal_Unicode cSeparator, sal_Unicode cQuote )
{
    OUString    sToken;
    sal_Int32   nCount = 0;
    sal_Int32   nOffset = 0;
    while( nOffset >= 0 )
    {
        GetTokenByOffset( sToken, rString, nOffset, cQuote, cSeparator );
        if( nOffset >= 0 )
            nCount++;
    }
    return nCount;
}

sal_Bool ScRangeStringConverter::GetAddressFromString(
        ScAddress& rAddress,
        const OUString& rAddressStr,
        const ScDocument* pDocument,
        FormulaGrammar::AddressConvention eConv,
        sal_Int32& nOffset,
        sal_Unicode cSeparator,
        sal_Unicode cQuote )
{
    OUString sToken;
    GetTokenByOffset( sToken, rAddressStr, nOffset, cSeparator, cQuote );
    if( nOffset >= 0 )
    {
        if ((rAddress.Parse( sToken, const_cast<ScDocument*>(pDocument), eConv ) & SCA_VALID) == SCA_VALID)
            return true;
        ::formula::FormulaGrammar::AddressConvention eConvUI = pDocument->GetAddressConvention();
        if (eConv != eConvUI)
            return ((rAddress.Parse(sToken, const_cast<ScDocument*>(pDocument), eConvUI) & SCA_VALID) == SCA_VALID);
    }
    return false;
}

sal_Bool ScRangeStringConverter::GetRangeFromString(
        ScRange& rRange,
        const OUString& rRangeStr,
        const ScDocument* pDocument,
        FormulaGrammar::AddressConvention eConv,
        sal_Int32& nOffset,
        sal_Unicode cSeparator,
        sal_Unicode cQuote )
{
    OUString sToken;
    sal_Bool bResult(false);
    GetTokenByOffset( sToken, rRangeStr, nOffset, cSeparator, cQuote );
    if( nOffset >= 0 )
    {
        sal_Int32 nIndex = IndexOf( sToken, ':', 0, cQuote );
        String aUIString(sToken);

        if( nIndex < 0 )
        {
            if ( aUIString.GetChar(0) == (sal_Unicode) '.' )
                aUIString.Erase( 0, 1 );
            bResult = ((rRange.aStart.Parse( aUIString, const_cast<ScDocument*> (pDocument), eConv) & SCA_VALID) == SCA_VALID);
            ::formula::FormulaGrammar::AddressConvention eConvUI = pDocument->GetAddressConvention();
            if (!bResult && eConv != eConvUI)
                bResult = ((rRange.aStart.Parse(
                    aUIString, const_cast<ScDocument*>(pDocument), eConvUI) & SCA_VALID) == SCA_VALID);
            rRange.aEnd = rRange.aStart;
        }
        else
        {
            if ( aUIString.GetChar(0) == (sal_Unicode) '.' )
            {
                aUIString.Erase( 0, 1 );
                --nIndex;
            }

            if ( nIndex < aUIString.Len() - 1 &&
                    aUIString.GetChar((xub_StrLen)nIndex + 1) == (sal_Unicode) '.' )
                aUIString.Erase( (xub_StrLen)nIndex + 1, 1 );

            bResult = ((rRange.Parse(aUIString, const_cast<ScDocument*> (pDocument), eConv) & SCA_VALID) == SCA_VALID);

            // #i77703# chart ranges in the file format contain both sheet names, even for an external reference sheet.
            // This isn't parsed by ScRange, so try to parse the two Addresses then.
            if (!bResult)
            {
                bResult = ((rRange.aStart.Parse( aUIString.Copy(0, (xub_StrLen)nIndex), const_cast<ScDocument*>(pDocument),
                                eConv) & SCA_VALID) == SCA_VALID) &&
                          ((rRange.aEnd.Parse( aUIString.Copy((xub_StrLen)nIndex+1), const_cast<ScDocument*>(pDocument),
                                eConv) & SCA_VALID) == SCA_VALID);

                ::formula::FormulaGrammar::AddressConvention eConvUI = pDocument->GetAddressConvention();
                if (!bResult && eConv != eConvUI)
                {
                    bResult = ((rRange.aStart.Parse( aUIString.Copy(0, (xub_StrLen)nIndex), const_cast<ScDocument*>(pDocument),
                                    eConvUI) & SCA_VALID) == SCA_VALID) &&
                              ((rRange.aEnd.Parse( aUIString.Copy((xub_StrLen)nIndex+1), const_cast<ScDocument*>(pDocument),
                                    eConvUI) & SCA_VALID) == SCA_VALID);
                }
            }
        }
    }
    return bResult;
}

sal_Bool ScRangeStringConverter::GetRangeListFromString(
        ScRangeList& rRangeList,
        const OUString& rRangeListStr,
        const ScDocument* pDocument,
        FormulaGrammar::AddressConvention eConv,
        sal_Unicode cSeparator,
        sal_Unicode cQuote )
{
    sal_Bool bRet = sal_True;
    OSL_ENSURE( !rRangeListStr.isEmpty(), "ScXMLConverter::GetRangeListFromString - empty string!" );
    sal_Int32 nOffset = 0;
    while( nOffset >= 0 )
    {
        ScRange* pRange = new ScRange;
        if (
             GetRangeFromString( *pRange, rRangeListStr, pDocument, eConv, nOffset, cSeparator, cQuote ) &&
             (nOffset >= 0)
           )
        {
            rRangeList.push_back( pRange );
            pRange = NULL;
        }
        else if (nOffset > -1)
            bRet = false;
        //if ownership transferred to rRangeList pRange was NULLed, otherwwise
        //delete it
        delete pRange;
    }
    return bRet;
}

sal_Bool ScRangeStringConverter::GetAreaFromString(
        ScArea& rArea,
        const OUString& rRangeStr,
        const ScDocument* pDocument,
        FormulaGrammar::AddressConvention eConv,
        sal_Int32& nOffset,
        sal_Unicode cSeparator,
        sal_Unicode cQuote )
{
    ScRange aScRange;
    sal_Bool bResult(false);
    if( GetRangeFromString( aScRange, rRangeStr, pDocument, eConv, nOffset, cSeparator, cQuote ) && (nOffset >= 0) )
    {
        rArea.nTab = aScRange.aStart.Tab();
        rArea.nColStart = aScRange.aStart.Col();
        rArea.nRowStart = aScRange.aStart.Row();
        rArea.nColEnd = aScRange.aEnd.Col();
        rArea.nRowEnd = aScRange.aEnd.Row();
        bResult = sal_True;
    }
    return bResult;
}

sal_Bool ScRangeStringConverter::GetAddressFromString(
        table::CellAddress& rAddress,
        const OUString& rAddressStr,
        const ScDocument* pDocument,
        FormulaGrammar::AddressConvention eConv,
        sal_Int32& nOffset,
        sal_Unicode cSeparator,
        sal_Unicode cQuote )
{
    ScAddress aScAddress;
    sal_Bool bResult(false);
    if( GetAddressFromString( aScAddress, rAddressStr, pDocument, eConv, nOffset, cSeparator, cQuote ) && (nOffset >= 0) )
    {
        ScUnoConversion::FillApiAddress( rAddress, aScAddress );
        bResult = sal_True;
    }
    return bResult;
}

sal_Bool ScRangeStringConverter::GetRangeFromString(
        table::CellRangeAddress& rRange,
        const OUString& rRangeStr,
        const ScDocument* pDocument,
        FormulaGrammar::AddressConvention eConv,
        sal_Int32& nOffset,
        sal_Unicode cSeparator,
        sal_Unicode cQuote )
{
    ScRange aScRange;
    sal_Bool bResult(false);
    if( GetRangeFromString( aScRange, rRangeStr, pDocument, eConv, nOffset, cSeparator, cQuote ) && (nOffset >= 0) )
    {
        ScUnoConversion::FillApiRange( rRange, aScRange );
        bResult = sal_True;
    }
    return bResult;
}

sal_Bool ScRangeStringConverter::GetRangeListFromString(
        uno::Sequence< table::CellRangeAddress >& rRangeSeq,
        const OUString& rRangeListStr,
        const ScDocument* pDocument,
        FormulaGrammar::AddressConvention eConv,
        sal_Unicode cSeparator,
        sal_Unicode cQuote )
{
    sal_Bool bRet = sal_True;
    OSL_ENSURE( !rRangeListStr.isEmpty(), "ScXMLConverter::GetRangeListFromString - empty string!" );
    table::CellRangeAddress aRange;
    sal_Int32 nOffset = 0;
    while( nOffset >= 0 )
    {
        if( GetRangeFromString( aRange, rRangeListStr, pDocument, eConv, nOffset, cSeparator, cQuote ) && (nOffset >= 0) )
        {
            rRangeSeq.realloc( rRangeSeq.getLength() + 1 );
            rRangeSeq[ rRangeSeq.getLength() - 1 ] = aRange;
        }
        else
            bRet = false;
    }
    return bRet;
}

void ScRangeStringConverter::GetStringFromAddress(
        OUString& rString,
        const ScAddress& rAddress,
        const ScDocument* pDocument,
        FormulaGrammar::AddressConvention eConv,
        sal_Unicode cSeparator,
        sal_Bool bAppendStr,
        sal_uInt16 nFormatFlags )
{
    if (pDocument && pDocument->HasTable(rAddress.Tab()))
    {
        OUString sAddress(rAddress.Format(nFormatFlags, (ScDocument*) pDocument, eConv));
        AssignString( rString, sAddress, bAppendStr, cSeparator );
    }
}

void ScRangeStringConverter::GetStringFromRange(
        OUString& rString,
        const ScRange& rRange,
        const ScDocument* pDocument,
        FormulaGrammar::AddressConvention eConv,
        sal_Unicode cSeparator,
        sal_Bool bAppendStr,
        sal_uInt16 nFormatFlags )
{
    if (pDocument && pDocument->HasTable(rRange.aStart.Tab()))
    {
        ScAddress aStartAddress( rRange.aStart );
        ScAddress aEndAddress( rRange.aEnd );
        OUString sStartAddress(aStartAddress.Format(nFormatFlags, (ScDocument*) pDocument, eConv));
        OUString sEndAddress(aEndAddress.Format(nFormatFlags, (ScDocument*) pDocument, eConv));
        OUString sOUStartAddress( sStartAddress );
        sOUStartAddress += OUString(':');
        sOUStartAddress += OUString( sEndAddress );
        AssignString( rString, sOUStartAddress, bAppendStr, cSeparator );
    }
}

void ScRangeStringConverter::GetStringFromRangeList(
        OUString& rString,
        const ScRangeList* pRangeList,
        const ScDocument* pDocument,
        FormulaGrammar::AddressConvention eConv,
        sal_Unicode cSeparator,
        sal_uInt16 nFormatFlags )
{
    OUString sRangeListStr;
    if( pRangeList )
    {
        for( size_t nIndex = 0, nCount = pRangeList->size(); nIndex < nCount; nIndex++ )
        {
            const ScRange* pRange = (*pRangeList)[nIndex];
            if( pRange )
                GetStringFromRange( sRangeListStr, *pRange, pDocument, eConv, cSeparator, sal_True, nFormatFlags );
        }
    }
    rString = sRangeListStr;
}

void ScRangeStringConverter::GetStringFromArea(
        OUString& rString,
        const ScArea& rArea,
        const ScDocument* pDocument,
        FormulaGrammar::AddressConvention eConv,
        sal_Unicode cSeparator,
        sal_Bool bAppendStr,
        sal_uInt16 nFormatFlags )
{
    ScRange aRange( rArea.nColStart, rArea.nRowStart, rArea.nTab, rArea.nColEnd, rArea.nRowEnd, rArea.nTab );
    GetStringFromRange( rString, aRange, pDocument, eConv, cSeparator, bAppendStr, nFormatFlags );
}

void ScRangeStringConverter::GetStringFromAddress(
        OUString& rString,
        const table::CellAddress& rAddress,
        const ScDocument* pDocument,
        FormulaGrammar::AddressConvention eConv,
        sal_Unicode cSeparator,
        sal_Bool bAppendStr,
        sal_uInt16 nFormatFlags )
{
    ScAddress aScAddress( static_cast<SCCOL>(rAddress.Column), static_cast<SCROW>(rAddress.Row), rAddress.Sheet );
    GetStringFromAddress( rString, aScAddress, pDocument, eConv, cSeparator, bAppendStr, nFormatFlags );
}

void ScRangeStringConverter::GetStringFromRange(
        OUString& rString,
        const table::CellRangeAddress& rRange,
        const ScDocument* pDocument,
        FormulaGrammar::AddressConvention eConv,
        sal_Unicode cSeparator,
        sal_Bool bAppendStr,
        sal_uInt16 nFormatFlags )
{
    ScRange aScRange( static_cast<SCCOL>(rRange.StartColumn), static_cast<SCROW>(rRange.StartRow), rRange.Sheet,
        static_cast<SCCOL>(rRange.EndColumn), static_cast<SCROW>(rRange.EndRow), rRange.Sheet );
    GetStringFromRange( rString, aScRange, pDocument, eConv, cSeparator, bAppendStr, nFormatFlags );
}

void ScRangeStringConverter::GetStringFromRangeList(
        OUString& rString,
        const uno::Sequence< table::CellRangeAddress >& rRangeSeq,
        const ScDocument* pDocument,
        FormulaGrammar::AddressConvention eConv,
        sal_Unicode cSeparator,
        sal_uInt16 nFormatFlags )
{
    OUString sRangeListStr;
    sal_Int32 nCount = rRangeSeq.getLength();
    for( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++ )
    {
        const table::CellRangeAddress& rRange = rRangeSeq[ nIndex ];
        GetStringFromRange( sRangeListStr, rRange, pDocument, eConv, cSeparator, sal_True, nFormatFlags );
    }
    rString = sRangeListStr;
}

static void lcl_appendCellAddress(
    OUStringBuffer& rBuf, ScDocument* pDoc, const ScAddress& rCell,
    const ScAddress::ExternalInfo& rExtInfo)
{
    if (rExtInfo.mbExternal)
    {
        ScExternalRefManager* pRefMgr = pDoc->GetExternalRefManager();
        const OUString* pFilePath = pRefMgr->getExternalFileName(rExtInfo.mnFileId, true);
        if (!pFilePath)
            return;

        sal_Unicode cQuote = '\'';
        rBuf.append(cQuote);
        rBuf.append(*pFilePath);
        rBuf.append(cQuote);
        rBuf.append(sal_Unicode('#'));
        rBuf.append(sal_Unicode('$'));
        ScRangeStringConverter::AppendTableName(rBuf, rExtInfo.maTabName);
        rBuf.append(sal_Unicode('.'));

        OUString aAddr(rCell.Format(SCA_ABS, NULL, pDoc->GetAddressConvention()));
        rBuf.append(aAddr);
    }
    else
    {
        OUString aAddr(rCell.Format(SCA_ABS_3D, pDoc, pDoc->GetAddressConvention()));
        rBuf.append(aAddr);
    }
}

static void lcl_appendCellRangeAddress(
    OUStringBuffer& rBuf, ScDocument* pDoc, const ScAddress& rCell1, const ScAddress& rCell2,
    const ScAddress::ExternalInfo& rExtInfo1, const ScAddress::ExternalInfo& rExtInfo2)
{
    if (rExtInfo1.mbExternal)
    {
        OSL_ENSURE(rExtInfo2.mbExternal, "2nd address is not external!?");
        OSL_ENSURE(rExtInfo1.mnFileId == rExtInfo2.mnFileId, "File IDs do not match between 1st and 2nd addresses.");

        ScExternalRefManager* pRefMgr = pDoc->GetExternalRefManager();
        const OUString* pFilePath = pRefMgr->getExternalFileName(rExtInfo1.mnFileId, true);
        if (!pFilePath)
            return;

        sal_Unicode cQuote = '\'';
        rBuf.append(cQuote);
        rBuf.append(*pFilePath);
        rBuf.append(cQuote);
        rBuf.append(sal_Unicode('#'));
        rBuf.append(sal_Unicode('$'));
        ScRangeStringConverter::AppendTableName(rBuf, rExtInfo1.maTabName);
        rBuf.append(sal_Unicode('.'));

        OUString aAddr(rCell1.Format(SCA_ABS, NULL, pDoc->GetAddressConvention()));
        rBuf.append(aAddr);

        rBuf.appendAscii(":");

        if (rExtInfo1.maTabName != rExtInfo2.maTabName)
        {
            rBuf.append(sal_Unicode('$'));
            ScRangeStringConverter::AppendTableName(rBuf, rExtInfo2.maTabName);
            rBuf.append(sal_Unicode('.'));
        }

        aAddr = rCell2.Format(SCA_ABS, NULL, pDoc->GetAddressConvention());
        rBuf.append(aAddr);
    }
    else
    {
        ScRange aRange;
        aRange.aStart = rCell1;
        aRange.aEnd   = rCell2;
        OUString aAddr(aRange.Format(SCR_ABS_3D, pDoc, pDoc->GetAddressConvention()));
        rBuf.append(aAddr);
    }
}

void ScRangeStringConverter::GetStringFromXMLRangeString( OUString& rString, const OUString& rXMLRange, ScDocument* pDoc )
{
    FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();
    const sal_Unicode cSep = ' ';
    const sal_Unicode cSepNew = ScCompiler::GetNativeSymbolChar(ocSep);
    const sal_Unicode cQuote = '\'';

    OUStringBuffer aRetStr;
    sal_Int32 nOffset = 0;
    bool bFirst = true;

    while (nOffset >= 0)
    {
        OUString aToken;
        GetTokenByOffset(aToken, rXMLRange, nOffset, cSep, cQuote);
        if (nOffset < 0)
            break;

        sal_Int32 nSepPos = IndexOf(aToken, ':', 0, cQuote);
        if (nSepPos >= 0)
        {
            // Cell range
            OUString aBeginCell = aToken.copy(0, nSepPos);
            OUString aEndCell   = aToken.copy(nSepPos+1);

            if (aBeginCell.isEmpty() || aEndCell.isEmpty())
                // both cell addresses must exist for this to work.
                continue;

            sal_Int32 nEndCellDotPos = aEndCell.indexOf('.');
            if (nEndCellDotPos <= 0)
            {
                // initialize buffer with table name...
                sal_Int32 nDotPos = IndexOf(aBeginCell, sal_Unicode('.'), 0, cQuote);
                OUStringBuffer aBuf = aBeginCell.copy(0, nDotPos);

                if (nEndCellDotPos == 0)
                {
                    // workaround for old syntax (probably pre-chart2 age?)
                    // e.g. Sheet1.A1:.B2
                    aBuf.append(aEndCell);
                }
                else if (nEndCellDotPos < 0)
                {
                    // sheet name in the end cell is omitted (e.g. Sheet2.A1:B2).
                    aBuf.append(sal_Unicode('.'));
                    aBuf.append(aEndCell);
                }
                aEndCell = aBuf.makeStringAndClear();
            }

            ScAddress::ExternalInfo aExtInfo1, aExtInfo2;
            ScAddress aCell1, aCell2;
            sal_uInt16 nRet = aCell1.Parse(aBeginCell, pDoc, FormulaGrammar::CONV_OOO, &aExtInfo1);
            if ((nRet & SCA_VALID) != SCA_VALID)
            {
                // first cell is invalid.
                if (eConv == FormulaGrammar::CONV_OOO)
                    continue;

                nRet = aCell1.Parse(aBeginCell, pDoc, eConv, &aExtInfo1);
                if ((nRet & SCA_VALID) != SCA_VALID)
                    // first cell is really invalid.
                    continue;
            }

            nRet = aCell2.Parse(aEndCell, pDoc, FormulaGrammar::CONV_OOO, &aExtInfo2);
            if ((nRet & SCA_VALID) != SCA_VALID)
            {
                // second cell is invalid.
                if (eConv == FormulaGrammar::CONV_OOO)
                    continue;

                nRet = aCell2.Parse(aEndCell, pDoc, eConv, &aExtInfo2);
                if ((nRet & SCA_VALID) != SCA_VALID)
                    // second cell is really invalid.
                    continue;
            }

            if (aExtInfo1.mnFileId != aExtInfo2.mnFileId || aExtInfo1.mbExternal != aExtInfo2.mbExternal)
                // external info inconsistency.
                continue;

            // All looks good!

            if (bFirst)
                bFirst = false;
            else
                aRetStr.append(cSepNew);

            lcl_appendCellRangeAddress(aRetStr, pDoc, aCell1, aCell2, aExtInfo1, aExtInfo2);
        }
        else
        {
            // Chart always saves ranges using CONV_OOO convention.
            ScAddress::ExternalInfo aExtInfo;
            ScAddress aCell;
            sal_uInt16 nRet = aCell.Parse(aToken, pDoc, ::formula::FormulaGrammar::CONV_OOO, &aExtInfo);
            if ((nRet & SCA_VALID) != SCA_VALID)
            {
                nRet = aCell.Parse(aToken, pDoc, eConv, &aExtInfo);
                if ((nRet & SCA_VALID) != SCA_VALID)
                    continue;
            }

            // Looks good!

            if (bFirst)
                bFirst = false;
            else
                aRetStr.append(cSepNew);

            lcl_appendCellAddress(aRetStr, pDoc, aCell, aExtInfo);
        }
    }

    rString = aRetStr.makeStringAndClear();
}

ScRangeData* ScRangeStringConverter::GetRangeDataFromString(const OUString& rString, const SCTAB nTab, const ScDocument* pDoc)
{
    ScRangeName* pLocalRangeName = pDoc->GetRangeName(nTab);
    ScRangeData* pData = NULL;
    OUString aUpperName = ScGlobal::pCharClass->uppercase(rString);
    if(pLocalRangeName)
    {
        pData = pLocalRangeName->findByUpperName(aUpperName);
    }
    if (!pData)
    {
        ScRangeName* pGlobalRangeName = pDoc->GetRangeName();
        if (pGlobalRangeName)
        {
            pData = pGlobalRangeName->findByUpperName(aUpperName);
        }
    }
    return pData;
}

ScArea::ScArea( SCTAB tab,
                SCCOL colStart, SCROW rowStart,
                SCCOL colEnd,   SCROW rowEnd ) :
        nTab     ( tab ),
        nColStart( colStart ),  nRowStart( rowStart ),
        nColEnd  ( colEnd ),    nRowEnd  ( rowEnd )
{
}

ScArea::ScArea( const ScArea& r ) :
        nTab     ( r.nTab ),
        nColStart( r.nColStart ),   nRowStart( r.nRowStart ),
        nColEnd  ( r.nColEnd ),     nRowEnd  ( r.nRowEnd )
{
}

ScArea& ScArea::operator=( const ScArea& r )
{
    nTab        = r.nTab;
    nColStart   = r.nColStart;
    nRowStart   = r.nRowStart;
    nColEnd     = r.nColEnd;
    nRowEnd     = r.nRowEnd;
    return *this;
}

sal_Bool ScArea::operator==( const ScArea& r ) const
{
    return (   (nTab        == r.nTab)
            && (nColStart   == r.nColStart)
            && (nRowStart   == r.nRowStart)
            && (nColEnd     == r.nColEnd)
            && (nRowEnd     == r.nRowEnd) );
}

ScAreaNameIterator::ScAreaNameIterator( ScDocument* pDoc ) :
    pRangeName(pDoc->GetRangeName()),
    pDBCollection(pDoc->GetDBCollection()),
    bFirstPass(true)
{
    if (pRangeName)
    {
        maRNPos = pRangeName->begin();
        maRNEnd = pRangeName->end();
    }
}

bool ScAreaNameIterator::Next( OUString& rName, ScRange& rRange )
{
    // Just a wrapper for OUString for now.  It should replace the method
    // below eventually.
    String aTmp;
    bool bRet = Next(aTmp, rRange);
    rName = aTmp;
    return bRet;
}

bool ScAreaNameIterator::Next( String& rName, ScRange& rRange )
{
    for (;;)
    {
        if ( bFirstPass )                                   // erst Bereichsnamen
        {
            if ( pRangeName && maRNPos != maRNEnd )
            {
                const ScRangeData& rData = *maRNPos->second;
                ++maRNPos;
                bool bValid = rData.IsValidReference(rRange);
                if (bValid)
                {
                    rName = rData.GetName();
                    return true;                            // found
                }
            }
            else
            {
                bFirstPass = false;
                if (pDBCollection)
                {
                    const ScDBCollection::NamedDBs& rDBs = pDBCollection->getNamedDBs();
                    maDBPos = rDBs.begin();
                    maDBEnd = rDBs.end();
                }
            }
        }

        if ( !bFirstPass )                                  // dann DB-Bereiche
        {
            if (pDBCollection && maDBPos != maDBEnd)
            {
                const ScDBData& rData = *maDBPos;
                ++maDBPos;
                rData.GetArea(rRange);
                rName = rData.GetName();
                return true;                                // found
            }
            else
                return false;                               // nothing left
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
