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

#include "scitems.hxx"
#include <comphelper/string.hxx>
#include <editeng/eeitem.hxx>

#include <editeng/lrspitem.hxx>
#include <editeng/paperinf.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/boxitem.hxx>
#include <vcl/svapp.hxx>

#include "htmlimp.hxx"
#include "htmlpars.hxx"
#include "filter.hxx"
#include "global.hxx"
#include "document.hxx"
#include "editutil.hxx"
#include "stlpool.hxx"
#include "stlsheet.hxx"
#include "compiler.hxx"
#include "rangenam.hxx"
#include "attrib.hxx"
#include "ftools.hxx"
#include "tokenarray.hxx"

FltError ScFormatFilterPluginImpl::ScImportHTML( SvStream &rStream, const OUString& rBaseURL, ScDocument *pDoc,
        ScRange& rRange, double nOutputFactor, bool bCalcWidthHeight, SvNumberFormatter* pFormatter,
        bool bConvertDate )
{
    ScHTMLImport aImp( pDoc, rBaseURL, rRange, bCalcWidthHeight );
    FltError nErr = (FltError) aImp.Read( rStream, rBaseURL );
    ScRange aR = aImp.GetRange();
    rRange.aEnd = aR.aEnd;
    aImp.WriteToDocument( true, nOutputFactor, pFormatter, bConvertDate );
    return nErr;
}

ScEEAbsImport *ScFormatFilterPluginImpl::CreateHTMLImport( ScDocument* pDocP, const OUString& rBaseURL, const ScRange& rRange )
{
    return new ScHTMLImport( pDocP, rBaseURL, rRange, true/*bCalcWidthHeight*/ );
}

ScHTMLImport::ScHTMLImport( ScDocument* pDocP, const OUString& rBaseURL, const ScRange& rRange, bool bCalcWidthHeight ) :
    ScEEImport( pDocP, rRange )
{
    Size aPageSize;
    OutputDevice* pDefaultDev = Application::GetDefaultDevice();
    const OUString& aPageStyle = mpDoc->GetPageStyle( rRange.aStart.Tab() );
    ScStyleSheet* pStyleSheet = static_cast<ScStyleSheet*>(mpDoc->
        GetStyleSheetPool()->Find( aPageStyle, SfxStyleFamily::Page ));
    if ( pStyleSheet )
    {
        const SfxItemSet& rSet = pStyleSheet->GetItemSet();
        const SvxLRSpaceItem* pLRItem = static_cast<const SvxLRSpaceItem*>( &rSet.Get( ATTR_LRSPACE ) );
        long nLeftMargin   = pLRItem->GetLeft();
        long nRightMargin  = pLRItem->GetRight();
        const SvxULSpaceItem* pULItem = static_cast<const SvxULSpaceItem*>( &rSet.Get( ATTR_ULSPACE ) );
        long nTopMargin    = pULItem->GetUpper();
        long nBottomMargin = pULItem->GetLower();
        aPageSize = static_cast<const SvxSizeItem&>(rSet.Get(ATTR_PAGE_SIZE)).GetSize();
        if ( !aPageSize.Width() || !aPageSize.Height() )
        {
            OSL_FAIL("PageSize Null ?!?!?");
            aPageSize = SvxPaperInfo::GetPaperSize( PAPER_A4 );
        }
        aPageSize.Width() -= nLeftMargin + nRightMargin;
        aPageSize.Height() -= nTopMargin + nBottomMargin;
        aPageSize = pDefaultDev->LogicToPixel( aPageSize, MapMode( MapUnit::MapTwip ) );
    }
    else
    {
        OSL_FAIL("no StyleSheet?!?");
        aPageSize = pDefaultDev->LogicToPixel(
            SvxPaperInfo::GetPaperSize( PAPER_A4 ), MapMode( MapUnit::MapTwip ) );
    }
    if( bCalcWidthHeight )
        mpParser.reset( new ScHTMLLayoutParser( mpEngine.get(), rBaseURL, aPageSize, pDocP ));
    else
        mpParser.reset( new ScHTMLQueryParser( mpEngine.get(), pDocP ));
}

void ScHTMLImport::InsertRangeName( ScDocument* pDoc, const OUString& rName, const ScRange& rRange )
{
    ScComplexRefData aRefData;
    aRefData.InitRange( rRange );
    ScTokenArray aTokArray;
    aTokArray.AddDoubleReference( aRefData );
    ScRangeData* pRangeData = new ScRangeData( pDoc, rName, aTokArray );
    pDoc->GetRangeName()->insert( pRangeData );
}

void ScHTMLImport::WriteToDocument(
    bool bSizeColsRows, double nOutputFactor, SvNumberFormatter* pFormatter, bool bConvertDate )
{
    ScEEImport::WriteToDocument( bSizeColsRows, nOutputFactor, pFormatter, bConvertDate );

    const ScHTMLParser* pParser = static_cast<ScHTMLParser*>(mpParser.get());
    const ScHTMLTable* pGlobTable = pParser->GetGlobalTable();
    if( !pGlobTable )
        return;

    // set cell borders for HTML table cells
    pGlobTable->ApplyCellBorders( mpDoc, maRange.aStart );

    // correct cell borders for merged cells
    for ( size_t i = 0, n = pParser->ListSize(); i < n; ++i )
    {
        const ScEEParseEntry* pEntry = pParser->ListEntry( i );
        if( (pEntry->nColOverlap > 1) || (pEntry->nRowOverlap > 1) )
        {
            SCTAB nTab = maRange.aStart.Tab();
            const ScMergeAttr* pItem = static_cast<const ScMergeAttr*>( mpDoc->GetAttr( pEntry->nCol, pEntry->nRow, nTab, ATTR_MERGE ) );
            if( pItem->IsMerged() )
            {
                SCCOL nColMerge = pItem->GetColMerge();
                SCROW nRowMerge = pItem->GetRowMerge();

                const SvxBoxItem* pToItem = static_cast<const SvxBoxItem*>(
                    mpDoc->GetAttr( pEntry->nCol, pEntry->nRow, nTab, ATTR_BORDER ) );
                SvxBoxItem aNewItem( *pToItem );
                if( nColMerge > 1 )
                {
                    const SvxBoxItem* pFromItem = static_cast<const SvxBoxItem*>(
                        mpDoc->GetAttr( pEntry->nCol + nColMerge - 1, pEntry->nRow, nTab, ATTR_BORDER ) );
                    aNewItem.SetLine( pFromItem->GetLine( SvxBoxItemLine::RIGHT ), SvxBoxItemLine::RIGHT );
                }
                if( nRowMerge > 1 )
                {
                    const SvxBoxItem* pFromItem = static_cast<const SvxBoxItem*>(
                        mpDoc->GetAttr( pEntry->nCol, pEntry->nRow + nRowMerge - 1, nTab, ATTR_BORDER ) );
                    aNewItem.SetLine( pFromItem->GetLine( SvxBoxItemLine::BOTTOM ), SvxBoxItemLine::BOTTOM );
                }
                mpDoc->ApplyAttr( pEntry->nCol, pEntry->nRow, nTab, aNewItem );
            }
        }
    }

    // create ranges for HTML tables
     // 1 - entire document
    ScRange aNewRange( maRange.aStart );
    aNewRange.aEnd.IncCol( static_cast<SCsCOL>(pGlobTable->GetDocSize( tdCol )) - 1 );
    aNewRange.aEnd.IncRow( pGlobTable->GetDocSize( tdRow ) - 1 );
    InsertRangeName( mpDoc, ScfTools::GetHTMLDocName(), aNewRange );

    // 2 - all tables
    InsertRangeName( mpDoc, ScfTools::GetHTMLTablesName(), ScRange( maRange.aStart ) );

    // 3 - single tables
    SCsCOL nColDiff = (SCsCOL)maRange.aStart.Col();
    SCsROW nRowDiff = (SCsROW)maRange.aStart.Row();
    SCsTAB nTabDiff = (SCsTAB)maRange.aStart.Tab();

    ScHTMLTable* pTable = nullptr;
    ScHTMLTableId nTableId = SC_HTML_GLOBAL_TABLE;
    ScRange aErrorRange( ScAddress::UNINITIALIZED );
    while( (pTable = pGlobTable->FindNestedTable( ++nTableId )) != nullptr )
    {
        pTable->GetDocRange( aNewRange );
        if (!aNewRange.Move( nColDiff, nRowDiff, nTabDiff, aErrorRange ))
        {
            assert(!"can't move");
        }
        // insert table number as name
        InsertRangeName( mpDoc, ScfTools::GetNameFromHTMLIndex( nTableId ), aNewRange );
        // insert table id as name
        if (!pTable->GetTableName().isEmpty())
        {
            OUString aName( ScfTools::GetNameFromHTMLName( pTable->GetTableName() ) );
            if (!mpDoc->GetRangeName()->findByUpperName(ScGlobal::pCharClass->uppercase(aName)))
                InsertRangeName( mpDoc, aName, aNewRange );
        }
    }
}

OUString ScFormatFilterPluginImpl::GetHTMLRangeNameList( ScDocument* pDoc, const OUString& rOrigName )
{
    return ScHTMLImport::GetHTMLRangeNameList( pDoc, rOrigName );
}

OUString ScHTMLImport::GetHTMLRangeNameList( ScDocument* pDoc, const OUString& rOrigName )
{
    OSL_ENSURE( pDoc, "ScHTMLImport::GetHTMLRangeNameList - missing document" );

    OUString aNewName;
    ScRangeName* pRangeNames = pDoc->GetRangeName();
    ScRangeList aRangeList;
    sal_Int32 nTokenCnt = comphelper::string::getTokenCount(rOrigName, ';');
    sal_Int32 nStringIx = 0;
    for( sal_Int32 nToken = 0; nToken < nTokenCnt; nToken++ )
    {
        OUString aToken( rOrigName.getToken( 0, ';', nStringIx ) );
        if( pRangeNames && ScfTools::IsHTMLTablesName( aToken ) )
        {   // build list with all HTML tables
            sal_uLong nIndex = 1;
            bool bLoop = true;
            while( bLoop )
            {
                aToken = ScfTools::GetNameFromHTMLIndex( nIndex++ );
                const ScRangeData* pRangeData = pRangeNames->findByUpperName(ScGlobal::pCharClass->uppercase(aToken));
                if (pRangeData)
                {
                    ScRange aRange;
                    if( pRangeData->IsReference( aRange ) && !aRangeList.In( aRange ) )
                    {
                        aNewName = ScGlobal::addToken(aNewName, aToken, ';');
                        aRangeList.Append( aRange );
                    }
                }
                else
                    bLoop = false;
            }
        }
        else
            aNewName = ScGlobal::addToken(aNewName, aToken, ';');
    }
    return aNewName;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
