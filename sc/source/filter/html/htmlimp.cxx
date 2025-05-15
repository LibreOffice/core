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

#include <scitems.hxx>
#include <osl/diagnose.h>
#include <unotools/charclass.hxx>

#include <editeng/lrspitem.hxx>
#include <editeng/paperinf.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/boxitem.hxx>
#include <vcl/svapp.hxx>
#include <o3tl/string_view.hxx>

#include <htmlimp.hxx>
#include <htmlpars.hxx>
#include <filter.hxx>
#include <global.hxx>
#include <document.hxx>
#include <editutil.hxx>
#include <stlpool.hxx>
#include <stlsheet.hxx>
#include <refdata.hxx>
#include <rangenam.hxx>
#include <attrib.hxx>
#include <ftools.hxx>
#include <tokenarray.hxx>

ErrCode ScFormatFilterPluginImpl::ScImportHTML( SvStream &rStream, const OUString& rBaseURL, ScDocument& rDoc,
        ScRange& rRange, double nOutputFactor, bool bCalcWidthHeight, SvNumberFormatter* pFormatter,
        bool bConvertDate, bool bConvertScientific )
{
    ScHTMLImport aImp( rDoc, rBaseURL, rRange, bCalcWidthHeight );
    ErrCode nErr = aImp.Read( rStream, rBaseURL );
    ScRange aR = aImp.GetRange();
    rRange.aEnd = aR.aEnd;
    aImp.WriteToDocument( true, nOutputFactor, pFormatter, bConvertDate, bConvertScientific );
    return nErr;
}

std::unique_ptr<ScEEAbsImport> ScFormatFilterPluginImpl::CreateHTMLImport( ScDocument& rDoc, const OUString& rBaseURL, const ScRange& rRange )
{
    return std::make_unique<ScHTMLImport>( rDoc, rBaseURL, rRange, true/*bCalcWidthHeight*/ );
}

ScHTMLImport::ScHTMLImport( ScDocument& rDoc, const OUString& rBaseURL, const ScRange& rRange, bool bCalcWidthHeight ) :
    ScEEImport( rDoc, rRange )
{
    Size aPageSize;
    OutputDevice* pDefaultDev = Application::GetDefaultDevice();
    const OUString aPageStyle = mrDoc.GetPageStyle( rRange.aStart.Tab() );
    ScStyleSheet* pStyleSheet = static_cast<ScStyleSheet*>(mrDoc.
        GetStyleSheetPool()->Find( aPageStyle, SfxStyleFamily::Page ));
    if ( pStyleSheet )
    {
        const SfxItemSet& rSet = pStyleSheet->GetItemSet();
        const SvxLRSpaceItem* pLRItem = &rSet.Get( ATTR_LRSPACE );
        tools::Long nLeftMargin = pLRItem->ResolveLeft({});
        tools::Long nRightMargin = pLRItem->ResolveRight({});
        const SvxULSpaceItem* pULItem = &rSet.Get( ATTR_ULSPACE );
        tools::Long nTopMargin    = pULItem->GetUpper();
        tools::Long nBottomMargin = pULItem->GetLower();
        aPageSize = rSet.Get(ATTR_PAGE_SIZE).GetSize();
        if ( !aPageSize.Width() || !aPageSize.Height() )
        {
            OSL_FAIL("PageSize Null ?!?!?");
            aPageSize = SvxPaperInfo::GetPaperSize( PAPER_A4 );
        }
        aPageSize.AdjustWidth( -(nLeftMargin + nRightMargin) );
        aPageSize.AdjustHeight( -(nTopMargin + nBottomMargin) );
        aPageSize = pDefaultDev->LogicToPixel( aPageSize, MapMode( MapUnit::MapTwip ) );
    }
    else
    {
        OSL_FAIL("no StyleSheet?!?");
        aPageSize = pDefaultDev->LogicToPixel(
            SvxPaperInfo::GetPaperSize( PAPER_A4 ), MapMode( MapUnit::MapTwip ) );
    }
    if( bCalcWidthHeight )
        mpParser.reset( new ScHTMLLayoutParser( mpEngine.get(), rBaseURL, aPageSize, rDoc ));
    else
        mpParser.reset( new ScHTMLQueryParser( mpEngine.get(), rDoc ));
}

void ScHTMLImport::InsertRangeName( ScDocument& rDoc, const OUString& rName, const ScRange& rRange )
{
    ScComplexRefData aRefData;
    aRefData.InitRange( rRange );
    aRefData.Ref1.SetFlag3D( true );
    aRefData.Ref2.SetFlag3D( aRefData.Ref2.Tab() != aRefData.Ref1.Tab() );
    ScTokenArray aTokArray(rDoc);
    aTokArray.AddDoubleReference( aRefData );
    ScRangeData* pRangeData = new ScRangeData( rDoc, rName, aTokArray );
    rDoc.GetRangeName()->insert( pRangeData );
}

void ScHTMLImport::WriteToDocument(
    bool bSizeColsRows, double nOutputFactor, SvNumberFormatter* pFormatter, bool bConvertDate,
    bool bConvertScientific )
{
    ScEEImport::WriteToDocument( bSizeColsRows, nOutputFactor, pFormatter, bConvertDate, bConvertScientific );

    const ScHTMLParser* pParser = static_cast<ScHTMLParser*>(mpParser.get());
    const ScHTMLTable* pGlobTable = pParser->GetGlobalTable();
    if( !pGlobTable )
        return;

    // set cell borders for HTML table cells
    pGlobTable->ApplyCellBorders( mrDoc, maRange.aStart );

    // correct cell borders for merged cells
    for ( size_t i = 0, n = pParser->ListSize(); i < n; ++i )
    {
        const ScEEParseEntry* pEntry = pParser->ListEntry( i );
        if( (pEntry->nColOverlap > 1) || (pEntry->nRowOverlap > 1) )
        {
            SCTAB nTab = maRange.aStart.Tab();
            const ScMergeAttr* pItem = mrDoc.GetAttr( pEntry->nCol, pEntry->nRow, nTab, ATTR_MERGE );
            if( pItem->IsMerged() )
            {
                SCCOL nColMerge = pItem->GetColMerge();
                SCROW nRowMerge = pItem->GetRowMerge();

                const SvxBoxItem* pToItem = mrDoc.GetAttr( pEntry->nCol, pEntry->nRow, nTab, ATTR_BORDER );
                SvxBoxItem aNewItem( *pToItem );
                if( nColMerge > 1 )
                {
                    const SvxBoxItem* pFromItem =
                        mrDoc.GetAttr( pEntry->nCol + nColMerge - 1, pEntry->nRow, nTab, ATTR_BORDER );
                    aNewItem.SetLine( pFromItem->GetLine( SvxBoxItemLine::RIGHT ), SvxBoxItemLine::RIGHT );
                }
                if( nRowMerge > 1 )
                {
                    const SvxBoxItem* pFromItem =
                        mrDoc.GetAttr( pEntry->nCol, pEntry->nRow + nRowMerge - 1, nTab, ATTR_BORDER );
                    aNewItem.SetLine( pFromItem->GetLine( SvxBoxItemLine::BOTTOM ), SvxBoxItemLine::BOTTOM );
                }
                mrDoc.ApplyAttr( pEntry->nCol, pEntry->nRow, nTab, aNewItem );
            }
        }
    }

    // create ranges for HTML tables
     // 1 - entire document
    ScRange aNewRange( maRange.aStart );
    aNewRange.aEnd.IncCol( static_cast<SCCOL>(pGlobTable->GetDocSize( tdCol )) - 1 );
    aNewRange.aEnd.IncRow( pGlobTable->GetDocSize( tdRow ) - 1 );
    InsertRangeName( mrDoc, ScfTools::GetHTMLDocName(), aNewRange );

    // 2 - all tables
    InsertRangeName( mrDoc, ScfTools::GetHTMLTablesName(), ScRange( maRange.aStart ) );

    // 3 - single tables
    SCCOL nColDiff = maRange.aStart.Col();
    SCROW nRowDiff = maRange.aStart.Row();
    SCTAB nTabDiff = maRange.aStart.Tab();

    ScHTMLTable* pTable = nullptr;
    ScHTMLTableId nTableId = SC_HTML_GLOBAL_TABLE;
    ScRange aErrorRange( ScAddress::UNINITIALIZED );
    while( (pTable = pGlobTable->FindNestedTable( ++nTableId )) != nullptr )
    {
        pTable->GetDocRange( aNewRange );
        if (!aNewRange.Move( nColDiff, nRowDiff, nTabDiff, aErrorRange, mrDoc ))
        {
            assert(!"can't move");
        }
        // insert table number as name
        OUStringBuffer aName(ScfTools::GetNameFromHTMLIndex(nTableId));
        // insert table id as name
        if (!pTable->GetTableName().isEmpty())
            aName.append(" - " + pTable->GetTableName());
        // insert table caption as name
        if (!pTable->GetTableCaption().isEmpty())
            aName.append(" - " + pTable->GetTableCaption());
        const OUString sName(aName.makeStringAndClear());
        if (!mrDoc.GetRangeName()->findByUpperName(ScGlobal::getCharClass().uppercase(sName)))
            InsertRangeName(mrDoc, sName, aNewRange);
    }
}

OUString ScFormatFilterPluginImpl::GetHTMLRangeNameList( ScDocument& rDoc, const OUString& rOrigName )
{
    return ScHTMLImport::GetHTMLRangeNameList( rDoc, rOrigName );
}

OUString ScHTMLImport::GetHTMLRangeNameList( const ScDocument& rDoc, std::u16string_view rOrigName )
{
    if (rOrigName.empty())
        return OUString();

    OUString aNewName;
    ScRangeName* pRangeNames = rDoc.GetRangeName();
    ScRangeList aRangeList;
    sal_Int32 nStringIx = 0;
    do
    {
        OUString aToken( o3tl::getToken(rOrigName, 0, ';', nStringIx ) );
        if( pRangeNames && ScfTools::IsHTMLTablesName( aToken ) )
        {   // build list with all HTML tables
            sal_uLong nIndex = 1;
            for(;;)
            {
                aToken = ScfTools::GetNameFromHTMLIndex( nIndex++ );
                const ScRangeData* pRangeData = pRangeNames->findByUpperName(ScGlobal::getCharClass().uppercase(aToken));
                if (!pRangeData)
                    break;
                ScRange aRange;
                if( pRangeData->IsReference( aRange ) && !aRangeList.Contains( aRange ) )
                {
                    aNewName = ScGlobal::addToken(aNewName, aToken, ';');
                    aRangeList.push_back( aRange );
                }
            }
        }
        else
            aNewName = ScGlobal::addToken(aNewName, aToken, ';');
    }
    while (nStringIx>0);
    return aNewName;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
