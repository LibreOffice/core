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

#include <algorithm>

#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/chart2/data/LabelOrigin.hpp>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/sequence.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <svl/zforlist.hxx>

#include <XMLRangeHelper.hxx>
#include <unochart.hxx>
#include <swtable.hxx>
#include <unoprnms.hxx>
#include <unomap.hxx>
#include <unomid.h>
#include <unocrsr.hxx>
#include <unotbl.hxx>
#include <doc.hxx>
#include <IDocumentChartDataProviderAccess.hxx>
#include <frmfmt.hxx>
#include <docsh.hxx>
#include <ndole.hxx>
#include <swtypes.hxx>
#include <unocore.hrc>
#include <docary.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/string.hxx>

using namespace ::com::sun::star;

void SwChartHelper::DoUpdateAllCharts( SwDoc* pDoc )
{
    if (!pDoc)
        return;

    uno::Reference< frame::XModel > xRes;

    SwOLENode *pONd;
    SwStartNode *pStNd;
    SwNodeIndex aIdx( *pDoc->GetNodes().GetEndOfAutotext().StartOfSectionNode(), 1 );
    while( nullptr != (pStNd = aIdx.GetNode().GetStartNode()) )
    {
        ++aIdx;
        if (nullptr != ( pONd = aIdx.GetNode().GetOLENode() ) &&
            pONd->GetOLEObj().GetObject().IsChart() )
        {
            // Load the object and set modified

            uno::Reference < embed::XEmbeddedObject > xIP = pONd->GetOLEObj().GetOleRef();
            if ( svt::EmbeddedObjectRef::TryRunningState( xIP ) )
            {
                try
                {
                    uno::Reference< util::XModifiable > xModif( xIP->getComponent(), uno::UNO_QUERY_THROW );
                    xModif->setModified( sal_True );
                }
                catch ( uno::Exception& )
                {
                }

            }
        }
        aIdx.Assign( *pStNd->EndOfSectionNode(), + 1 );
    }
}

SwChartLockController_Helper::SwChartLockController_Helper( SwDoc *pDocument ) :
    pDoc( pDocument )
    , bIsLocked( false )
{
    aUnlockTimer.SetTimeout( 1500 );
    aUnlockTimer.SetTimeoutHdl( LINK( this, SwChartLockController_Helper, DoUnlockAllCharts ));
}

SwChartLockController_Helper::~SwChartLockController_Helper()
{
    if (pDoc)   // still connected?
        Disconnect();
}

void SwChartLockController_Helper::StartOrContinueLocking()
{
    if (!bIsLocked)
        LockAllCharts();
    aUnlockTimer.Start();   // start or continue time of locking
}

void SwChartLockController_Helper::Disconnect()
{
    aUnlockTimer.Stop();
    UnlockAllCharts();
    pDoc = nullptr;
}

void SwChartLockController_Helper::LockUnlockAllCharts( bool bLock )
{
    if (!pDoc)
        return;

    const SwFrameFormats& rTableFormats = *pDoc->GetTableFrameFormats();
    for( size_t n = 0; n < rTableFormats.size(); ++n )
    {
        SwTable* pTmpTable;
        const SwTableNode* pTableNd;
        const SwFrameFormat* pFormat = rTableFormats[ n ];

        if( nullptr != ( pTmpTable = SwTable::FindTable( pFormat ) ) &&
            nullptr != ( pTableNd = pTmpTable->GetTableNode() ) &&
            pTableNd->GetNodes().IsDocNodes() )
        {
            uno::Reference< frame::XModel > xRes;
            SwOLENode *pONd;
            SwStartNode *pStNd;
            SwNodeIndex aIdx( *pDoc->GetNodes().GetEndOfAutotext().StartOfSectionNode(), 1 );
            while( nullptr != (pStNd = aIdx.GetNode().GetStartNode()) )
            {
                ++aIdx;
                if (nullptr != ( pONd = aIdx.GetNode().GetOLENode() ) &&
                    !pONd->GetChartTableName().isEmpty() /* is chart object? */)
                {
                    uno::Reference < embed::XEmbeddedObject > xIP = pONd->GetOLEObj().GetOleRef();
                    if ( svt::EmbeddedObjectRef::TryRunningState( xIP ) )
                    {
                        xRes.set( xIP->getComponent(), uno::UNO_QUERY );
                        if (xRes.is())
                        {
                            if (bLock)
                                xRes->lockControllers();
                            else
                                xRes->unlockControllers();
                        }
                    }
                }
                aIdx.Assign( *pStNd->EndOfSectionNode(), + 1 );
            }
        }
    }

    bIsLocked = bLock;
}

IMPL_LINK_NOARG_TYPED( SwChartLockController_Helper, DoUnlockAllCharts, Timer *, void )
{
    UnlockAllCharts();
}

static osl::Mutex &    GetChartMutex()
{
    static osl::Mutex   aMutex;
    return aMutex;
}

static void LaunchModifiedEvent(
        ::cppu::OInterfaceContainerHelper &rICH,
        const uno::Reference< uno::XInterface > &rxI )
{
    lang::EventObject aEvtObj( rxI );
    cppu::OInterfaceIteratorHelper aIt( rICH );
    while (aIt.hasMoreElements())
    {
        uno::Reference< util::XModifyListener > xRef( aIt.next(), uno::UNO_QUERY );
        if (xRef.is())
            xRef->modified( aEvtObj );
    }
}

/**
 * rCellRangeName needs to be of one of the following formats:
 * - e.g. "A2:E5" or
 *   - e.g. "Table1.A2:E5"
 */
bool FillRangeDescriptor(
        SwRangeDescriptor &rDesc,
        const OUString &rCellRangeName )
{
    sal_Int32 nToken = -1 == rCellRangeName.indexOf('.') ? 0 : 1;
    OUString aCellRangeNoTableName( rCellRangeName.getToken( nToken, '.' ) );
    OUString aTLName( aCellRangeNoTableName.getToken(0, ':') );  // name of top left cell
    OUString aBRName( aCellRangeNoTableName.getToken(1, ':') );  // name of bottom right cell
    if(aTLName.isEmpty() || aBRName.isEmpty())
        return false;

    rDesc.nTop = rDesc.nLeft = rDesc.nBottom = rDesc.nRight = -1;
    SwXTextTable::GetCellPosition( aTLName, rDesc.nLeft,  rDesc.nTop );
    SwXTextTable::GetCellPosition( aBRName, rDesc.nRight, rDesc.nBottom );
    rDesc.Normalize();
    OSL_ENSURE( rDesc.nTop    != -1 &&
                rDesc.nLeft   != -1 &&
                rDesc.nBottom != -1 &&
                rDesc.nRight  != -1,
            "failed to get range descriptor" );
    OSL_ENSURE( rDesc.nTop <= rDesc.nBottom  &&  rDesc.nLeft <= rDesc.nRight,
            "invalid range descriptor");
    return true;
}

static OUString GetCellRangeName( SwFrameFormat &rTableFormat, SwUnoCursor &rTableCursor )
{
    OUString aRes;

    //!! see also SwXTextTableCursor::getRangeName

    SwUnoTableCursor* pUnoTableCursor = dynamic_cast<SwUnoTableCursor*>(&rTableCursor);
    if (!pUnoTableCursor)
        return OUString();
    pUnoTableCursor->MakeBoxSels();

    const SwStartNode*  pStart;
    const SwTableBox*   pStartBox   = nullptr;
    const SwTableBox*   pEndBox     = nullptr;

    pStart = pUnoTableCursor->GetPoint()->nNode.GetNode().FindTableBoxStartNode();
    if (pStart)
    {
        const SwTable* pTable = SwTable::FindTable( &rTableFormat );
        pEndBox = pTable->GetTableBox( pStart->GetIndex());
        aRes = pEndBox->GetName();

        if(pUnoTableCursor->HasMark())
        {
            pStart = pUnoTableCursor->GetMark()->nNode.GetNode().FindTableBoxStartNode();
            pStartBox = pTable->GetTableBox( pStart->GetIndex());
        }
        OSL_ENSURE( pStartBox, "start box not found" );
        OSL_ENSURE( pEndBox, "end box not found" );

        // need to switch start and end?
        if (*pUnoTableCursor->GetPoint() < *pUnoTableCursor->GetMark())
        {
            const SwTableBox* pTmpBox = pStartBox;
            pStartBox = pEndBox;
            pEndBox = pTmpBox;
        }

        if (!pStartBox)
            return aRes;

        aRes = pStartBox->GetName();
        aRes += ":";
        if (pEndBox)
            aRes += pEndBox->GetName();
        else
            aRes += pStartBox->GetName();
    }

    return aRes;
}

static OUString GetRangeRepFromTableAndCells( const OUString &rTableName,
        const OUString &rStartCell, const OUString &rEndCell,
        bool bForceEndCellName )
{
    OSL_ENSURE( !rTableName.isEmpty(), "table name missing" );
    OSL_ENSURE( !rStartCell.isEmpty(), "cell name missing" );
    OUString aRes( rTableName );
    aRes += ".";
    aRes += rStartCell;

    if (!rEndCell.isEmpty())
    {
        aRes += ":";
        aRes += rEndCell;
    }
    else if (bForceEndCellName)
    {
        aRes += ":";
        aRes += rStartCell;
    }

    return aRes;
}

static bool GetTableAndCellsFromRangeRep(
        const OUString &rRangeRepresentation,
        OUString &rTableName,
        OUString &rStartCell,
        OUString &rEndCell,
        bool bSortStartEndCells = true )
{
    // parse range representation for table name and cell/range names
    // accepted format sth like: "Table1.A2:C5" , "Table2.A2.1:B3.2"
    OUString aTableName;    // table name
    OUString aRange;    // cell range
    OUString aStartCell;  // name of top left cell
    OUString aEndCell;    // name of bottom right cell
    sal_Int32 nIdx = rRangeRepresentation.indexOf( '.' );
    if (nIdx >= 0)
    {
        aTableName = rRangeRepresentation.copy( 0, nIdx );
        aRange = rRangeRepresentation.copy( nIdx + 1 );
        sal_Int32 nPos = aRange.indexOf( ':' );
        if (nPos >= 0) // a cell-range like "Table1.A2:D4"
        {
            aStartCell = aRange.copy( 0, nPos );
            aEndCell   = aRange.copy( nPos + 1 );

            // need to switch start and end cell ?
            // (does not check for normalization here)
            if (bSortStartEndCells && 1 == sw_CompareCellsByColFirst( aStartCell, aEndCell ))
            {
                OUString aTmp( aStartCell );
                aStartCell  = aEndCell;
                aEndCell    = aTmp;
            }
        }
        else    // a single cell like in "Table1.B3"
        {
            aStartCell = aEndCell = aRange;
        }
    }

    bool bSuccess = !aTableName.isEmpty() &&
                        !aStartCell.isEmpty() && !aEndCell.isEmpty();
    if (bSuccess)
    {
        rTableName    = aTableName;
        rStartCell  = aStartCell;
        rEndCell    = aEndCell;
    }
    return bSuccess;
}

static void GetTableByName( const SwDoc &rDoc, const OUString &rTableName,
        SwFrameFormat **ppTableFormat, SwTable **ppTable)
{
    SwFrameFormat *pTableFormat = nullptr;

    // find frame format of table
    //! see SwXTextTables::getByName
    const size_t nCount = rDoc.GetTableFrameFormatCount(true);
    for (size_t i = 0; i < nCount && !pTableFormat; ++i)
    {
        SwFrameFormat& rTableFormat = rDoc.GetTableFrameFormat(i, true);
        if(rTableName == rTableFormat.GetName())
            pTableFormat = &rTableFormat;
    }

    if (ppTableFormat)
        *ppTableFormat = pTableFormat;

    if (ppTable)
        *ppTable = pTableFormat ? SwTable::FindTable( pTableFormat ) : nullptr;
}

static void GetFormatAndCreateCursorFromRangeRep(
        const SwDoc    *pDoc,
        const OUString &rRangeRepresentation,   // must be a single range (i.e. so called sub-range)
        SwFrameFormat    **ppTableFormat,     // will be set to the table format of the table used in the range representation
        std::shared_ptr<SwUnoCursor>&   rpUnoCursor )   // will be set to cursor spanning the cell range (cursor will be created!)
{
    OUString aTableName;    // table name
    OUString aStartCell;  // name of top left cell
    OUString aEndCell;    // name of bottom right cell
    bool bNamesFound = GetTableAndCellsFromRangeRep( rRangeRepresentation,
                                  aTableName, aStartCell, aEndCell );

    if (!bNamesFound)
    {
        if (ppTableFormat)
            *ppTableFormat   = nullptr;
        rpUnoCursor.reset();
    }
    else
    {
        SwFrameFormat *pTableFormat = nullptr;

        // is the correct table format already provided?
        if (*ppTableFormat != nullptr  &&  (*ppTableFormat)->GetName() == aTableName)
            pTableFormat = *ppTableFormat;
        else
            GetTableByName( *pDoc, aTableName, &pTableFormat, nullptr );

        *ppTableFormat = pTableFormat;

        rpUnoCursor.reset();  // default result in case of failure

        SwTable *pTable = pTableFormat ? SwTable::FindTable( pTableFormat ) : nullptr;
        // create new SwUnoCursor spanning the specified range
        //! see also SwXTextTable::GetRangeByName
        // #i80314#
        // perform validation check. Thus, pass <true> as 2nd parameter to <SwTable::GetTableBox(..)>
        const SwTableBox* pTLBox =
                        pTable ? pTable->GetTableBox( aStartCell, true ) : nullptr;
        if(pTLBox)
        {
            const SwStartNode* pSttNd = pTLBox->GetSttNd();
            SwPosition aPos(*pSttNd);

            // set cursor to top left box of range
            auto pUnoCursor = pTableFormat->GetDoc()->CreateUnoCursor(aPos, true);
            pUnoCursor->Move( fnMoveForward, fnGoNode );
            pUnoCursor->SetRemainInSection( false );

            // #i80314#
            // perform validation check. Thus, pass <true> as 2nd parameter to <SwTable::GetTableBox(..)>
            const SwTableBox* pBRBox = pTable->GetTableBox( aEndCell, true );
            if(pBRBox)
            {
                pUnoCursor->SetMark();
                pUnoCursor->GetPoint()->nNode = *pBRBox->GetSttNd();
                pUnoCursor->Move( fnMoveForward, fnGoNode );
                SwUnoTableCursor* pCursor =
                    dynamic_cast<SwUnoTableCursor*>(pUnoCursor.get());
                // HACK: remove pending actions for old style tables
                UnoActionRemoveContext aRemoveContext(*pCursor);
                pCursor->MakeBoxSels();
                rpUnoCursor = pUnoCursor;
            }
        }
    }
}

static bool GetSubranges( const OUString &rRangeRepresentation,
        uno::Sequence< OUString > &rSubRanges, bool bNormalize )
{
    bool bRes = true;
    sal_Int32 nLen = comphelper::string::getTokenCount(rRangeRepresentation, ';');
    uno::Sequence< OUString > aRanges( nLen );

    sal_Int32 nCnt = 0;
    if (nLen != 0)
    {
        OUString *pRanges = aRanges.getArray();
        OUString aFirstTable;
        sal_Int32 nPos = 0;
        for( sal_Int32 i = 0; i < nLen && bRes; ++i )
        {
            const OUString aRange( rRangeRepresentation.getToken( 0, ';', nPos ) );
            if (!aRange.isEmpty())
            {
                pRanges[nCnt] = aRange;

                OUString aTableName, aStartCell, aEndCell;
                if (!GetTableAndCellsFromRangeRep( aRange,
                                                   aTableName, aStartCell, aEndCell ))
                    bRes = false;

                if (bNormalize)
                {
                    sw_NormalizeRange( aStartCell, aEndCell );
                    pRanges[nCnt] = GetRangeRepFromTableAndCells( aTableName,
                                    aStartCell, aEndCell, true );
                }

                // make sure to use only a single table
                if (nCnt == 0)
                    aFirstTable = aTableName;
                else
                    if (aFirstTable != aTableName) bRes = false;

                ++nCnt;
            }
        }
    }
    aRanges.realloc( nCnt );

    rSubRanges = aRanges;
    return bRes;
}

static void SortSubranges( uno::Sequence< OUString > &rSubRanges, bool bCmpByColumn )
{
    sal_Int32 nLen = rSubRanges.getLength();
    OUString *pSubRanges = rSubRanges.getArray();

    OUString aSmallestTableName;
    OUString aSmallestStartCell;
    OUString aSmallestEndCell;

    for (sal_Int32 i = 0;  i < nLen;  ++i)
    {
        sal_Int32 nIdxOfSmallest = i;
        GetTableAndCellsFromRangeRep( pSubRanges[nIdxOfSmallest],
                aSmallestTableName, aSmallestStartCell, aSmallestEndCell );
        if (aSmallestEndCell.isEmpty())
            aSmallestEndCell = aSmallestStartCell;

        for (sal_Int32 k = i+1;  k < nLen;  ++k)
        {
            // get cell names for sub range
            OUString aTableName;
            OUString aStartCell;
            OUString aEndCell;
            GetTableAndCellsFromRangeRep( pSubRanges[k],
                    aTableName, aStartCell, aEndCell );
            if (aEndCell.isEmpty())
                aEndCell = aStartCell;

            // compare cell ranges ( is the new one smaller? )
            if (-1 == sw_CompareCellRanges( aStartCell, aEndCell,
                                aSmallestStartCell, aSmallestEndCell, bCmpByColumn ))
            {
                nIdxOfSmallest = k;
                aSmallestTableName    = aTableName;
                aSmallestStartCell  = aStartCell;
                aSmallestEndCell    = aEndCell;
            }
        }

        // move smallest element to the start of the not sorted area
        const OUString aTmp( pSubRanges[ nIdxOfSmallest ] );
        pSubRanges[ nIdxOfSmallest ] = pSubRanges[ i ];
        pSubRanges[ i ] = aTmp;
    }
}

SwChartDataProvider::SwChartDataProvider( const SwDoc* pSwDoc ) :
    aEvtListeners( GetChartMutex() ),
    pDoc( pSwDoc )
{
    bDisposed = false;
}

SwChartDataProvider::~SwChartDataProvider()
{
}

uno::Reference< chart2::data::XDataSource > SwChartDataProvider::Impl_createDataSource(
        const uno::Sequence< beans::PropertyValue >& rArguments, bool bTestOnly )
    throw (lang::IllegalArgumentException, uno::RuntimeException,
           std::exception)
{
    SolarMutexGuard aGuard;
    if (bDisposed)
        throw lang::DisposedException();

    uno::Reference< chart2::data::XDataSource > xRes;

    if (!pDoc)
        throw uno::RuntimeException();

    // get arguments
    OUString aRangeRepresentation;
    uno::Sequence< sal_Int32 > aSequenceMapping;
    bool bFirstIsLabel      = false;
    bool bDtaSrcIsColumns   = true; // true : DataSource will be sequence of columns
                                    // false: DataSource will be sequence of rows

    OUString aChartOleObjectName; //work around wrong writer ranges ( see Issue 58464 )
    sal_Int32 nArgs = rArguments.getLength();
    OSL_ENSURE( nArgs != 0, "no properties provided" );
    if (nArgs == 0)
        return xRes;
    const beans::PropertyValue *pArg = rArguments.getConstArray();
    for (sal_Int32 i = 0;  i < nArgs;  ++i)
    {
        if ( pArg[i].Name == "DataRowSource" )
        {
            chart::ChartDataRowSource eSource;
            if (!(pArg[i].Value >>= eSource))
            {
                sal_Int32 nTmp = 0;
                if (!(pArg[i].Value >>= nTmp))
                    throw lang::IllegalArgumentException();
                eSource = static_cast< chart::ChartDataRowSource >( nTmp );
            }
            bDtaSrcIsColumns = eSource == chart::ChartDataRowSource_COLUMNS;
        }
        else if ( pArg[i].Name == "FirstCellAsLabel" )
        {
            if (!(pArg[i].Value >>= bFirstIsLabel))
                throw lang::IllegalArgumentException();
        }
        else if ( pArg[i].Name == "CellRangeRepresentation" )
        {
            if (!(pArg[i].Value >>= aRangeRepresentation))
                throw lang::IllegalArgumentException();
        }
        else if ( pArg[i].Name == "SequenceMapping" )
        {
            if (!(pArg[i].Value >>= aSequenceMapping))
                throw lang::IllegalArgumentException();
        }
        else if ( pArg[i].Name == "ChartOleObjectName" )
        {
            if (!(pArg[i].Value >>= aChartOleObjectName))
                throw lang::IllegalArgumentException();
        }
    }

    uno::Sequence< OUString > aSubRanges;
    // get sub-ranges and check that they all are from the very same table
    bool bOk = GetSubranges( aRangeRepresentation, aSubRanges, true );

    if (!bOk && pDoc && !aChartOleObjectName.isEmpty() )
    {
        //try to correct the range here
        //work around wrong writer ranges ( see Issue 58464 )
        OUString aChartTableName;

        const SwNodes& rNodes = pDoc->GetNodes();
        for( sal_uLong nN = rNodes.Count(); nN--; )
        {
            SwNodePtr pNode = rNodes[nN];
            if( !pNode )
                continue;
            const SwOLENode* pOleNode = pNode->GetOLENode();
            if( !pOleNode )
                continue;
            const SwOLEObj& rOObj = pOleNode->GetOLEObj();
            if( aChartOleObjectName.equals( rOObj.GetCurrentPersistName() ) )
            {
                aChartTableName = pOleNode->GetChartTableName();
                break;
            }
        }

        if( !aChartTableName.isEmpty() )
        {
            //the wrong range is still shifted one row down
            //thus the first row is missing and an invalid row at the end is added.
            //Therefore we need to shift the range one row up
            SwRangeDescriptor aDesc;
            if (aRangeRepresentation.isEmpty())
                return xRes;        // we can't handle this thus returning an empty references

            aRangeRepresentation = aRangeRepresentation.copy( 1 ); // get rid of '.' to have only the cell range left
            FillRangeDescriptor( aDesc, aRangeRepresentation );
            aDesc.Normalize();

            if (aDesc.nTop <= 0)    // no chance to shift the range one row up?
                return xRes;        // we can't handle this thus returning an empty references

            aDesc.nTop      -= 1;
            aDesc.nBottom   -= 1;

            OUString aNewStartCell( sw_GetCellName( aDesc.nLeft, aDesc.nTop ) );
            OUString aNewEndCell( sw_GetCellName( aDesc.nRight, aDesc.nBottom ) );
            aRangeRepresentation = GetRangeRepFromTableAndCells(
                        aChartTableName, aNewStartCell, aNewEndCell, true );
            bOk = GetSubranges( aRangeRepresentation, aSubRanges, true );
        }
    }
    if (!bOk) // different tables used, or incorrect range specifiers
        throw lang::IllegalArgumentException();

    SortSubranges( aSubRanges, bDtaSrcIsColumns );
    const OUString *pSubRanges = aSubRanges.getConstArray();
#if OSL_DEBUG_LEVEL > 1
    {
        sal_Int32 nSR = aSubRanges.getLength();
        OUString *pSR = aSubRanges.getArray();
        OUString aRg;
        for (sal_Int32 i = 0;  i < nSR;  ++i)
        {
            aRg = pSR[i];
        }
    }
#endif

    // get table format for that single table from above
    SwFrameFormat    *pTableFormat  = nullptr;      // pointer to table format
    std::shared_ptr<SwUnoCursor> pUnoCursor;      // here required to check if the cells in the range do actually exist
    if (aSubRanges.getLength() > 0)
        GetFormatAndCreateCursorFromRangeRep( pDoc, pSubRanges[0], &pTableFormat, pUnoCursor );

    if (!pTableFormat || !pUnoCursor)
        throw lang::IllegalArgumentException();

    if(pTableFormat)
    {
        SwTable* pTable = SwTable::FindTable( pTableFormat );
        if(pTable->IsTableComplex())
            return xRes;    // we can't handle this thus returning an empty references
        else
        {
            // get a character map in the size of the table to mark
            // all the ranges to use in
            sal_Int32 nRows = pTable->GetTabLines().size();
            sal_Int32 nCols = pTable->GetTabLines().front()->GetTabBoxes().size();
            std::vector< std::vector< sal_Char > > aMap( nRows );
            for (sal_Int32 i = 0;  i < nRows;  ++i)
                aMap[i].resize( nCols );

            // iterate over subranges and mark used cells in above map
            //!! by proceeding this way we automatically get rid of
            //!! multiple listed or overlapping cell ranges which should
            //!! just be ignored silently
            sal_Int32 nSubRanges = aSubRanges.getLength();
            for (sal_Int32 i = 0;  i < nSubRanges;  ++i)
            {
                OUString aTableName, aStartCell, aEndCell;
                bool bOk2 = GetTableAndCellsFromRangeRep(
                                    pSubRanges[i], aTableName, aStartCell, aEndCell );
                (void) bOk2;
                OSL_ENSURE( bOk2, "failed to get table and start/end cells" );

                sal_Int32 nStartRow, nStartCol, nEndRow, nEndCol;
                SwXTextTable::GetCellPosition( aStartCell, nStartCol, nStartRow );
                SwXTextTable::GetCellPosition( aEndCell,   nEndCol,   nEndRow );
                OSL_ENSURE( nStartRow <= nEndRow && nStartCol <= nEndCol,
                        "cell range not normalized");

                // test if the ranges span more than the available cells
                if( nStartRow < 0 || nEndRow >= nRows ||
                    nStartCol < 0 || nEndCol >= nCols )
                {
                    throw lang::IllegalArgumentException();
                }
                for (sal_Int32 k1 = nStartRow;  k1 <= nEndRow;  ++k1)
                {
                    for (sal_Int32 k2 = nStartCol;  k2 <= nEndCol;  ++k2)
                        aMap[k1][k2] = 'x';
                }
            }

            // find label and data sequences to use

            sal_Int32 oi;  // outer index (slower changing index)
            sal_Int32 ii;  // inner index (faster changing index)
            sal_Int32 oiEnd = bDtaSrcIsColumns ? nCols : nRows;
            sal_Int32 iiEnd = bDtaSrcIsColumns ? nRows : nCols;
            std::vector< sal_Int32 > aLabelIdx( oiEnd );
            std::vector< sal_Int32 > aDataStartIdx( oiEnd );
            std::vector< sal_Int32 > aDataLen( oiEnd );
            for (oi = 0;  oi < oiEnd;  ++oi)
            {
                aLabelIdx[oi]       = -1;
                aDataStartIdx[oi]   = -1;
                aDataLen[oi]        = 0;
            }

            for (oi = 0;  oi < oiEnd;  ++oi)
            {
                ii = 0;
                while (ii < iiEnd)
                {
                    sal_Char &rChar = bDtaSrcIsColumns ? aMap[ii][oi] : aMap[oi][ii];

                    // label should be used but is not yet found?
                    if (rChar == 'x' && bFirstIsLabel && aLabelIdx[oi] == -1)
                    {
                        aLabelIdx[oi] = ii;
                        rChar = 'L';    // setting a different char for labels here
                                        // makes the test for the data sequence below
                                        // easier
                    }

                    // find data sequence
                    if (rChar == 'x' && aDataStartIdx[oi] == -1)
                    {
                        aDataStartIdx[oi] = ii;

                        // get length of data sequence
                        sal_Int32 nL = 0;
                        sal_Char c;
                        while (ii< iiEnd && 'x' == (c = bDtaSrcIsColumns ? aMap[ii][oi] : aMap[oi][ii]))
                        {
                            ++nL;   ++ii;
                        }
                        aDataLen[oi] = nL;

                        // check that there is no other separate sequence of data
                        // to be found because that is not supported
                        while (ii < iiEnd)
                        {
                            if ('x' == (c = bDtaSrcIsColumns ? aMap[ii][oi] : aMap[oi][ii]))
                                throw lang::IllegalArgumentException();
                            ++ii;
                        }
                    }
                    else
                        ++ii;
                }
            }

            // make some other consistency checks while calculating
            // the number of XLabeledDataSequence to build:
            // - labels should always be used or not at all
            // - the data sequences should have equal non-zero length
            sal_Int32 nNumLDS = 0;
            if (oiEnd > 0)
            {
                sal_Int32 nFirstSeqLen = 0;
                sal_Int32 nFirstSeqLabelIdx = -1;
                bool bFirstFound = false;
                for (oi = 0;  oi < oiEnd;  ++oi)
                {
                    // row/col used at all?
                    if (aDataStartIdx[oi] != -1 &&
                        (!bFirstIsLabel || aLabelIdx[oi] != -1))
                    {
                        ++nNumLDS;
                        if (!bFirstFound)
                        {
                            nFirstSeqLen        = aDataLen[oi];
                            nFirstSeqLabelIdx   = aLabelIdx[oi];
                            bFirstFound = true;
                        }
                        else
                        {
                            if (nFirstSeqLen != aDataLen[oi] ||
                                nFirstSeqLabelIdx != aLabelIdx[oi])
                                throw lang::IllegalArgumentException();
                        }
                    }
                }
            }
            if (nNumLDS == 0)
                throw lang::IllegalArgumentException();

            // now we should have all necessary data to build a proper DataSource
            // thus if we came this far there should be no further problem
            if (bTestOnly)
                return xRes;    // have createDataSourcePossible return true

            // create data source from found label and data sequences
            uno::Sequence< uno::Reference< chart2::data::XDataSequence > > aLabelSeqs( nNumLDS );
            uno::Reference< chart2::data::XDataSequence > *pLabelSeqs = aLabelSeqs.getArray();
            uno::Sequence< uno::Reference< chart2::data::XDataSequence > > aDataSeqs( nNumLDS );
            uno::Reference< chart2::data::XDataSequence > *pDataSeqs = aDataSeqs.getArray();
            sal_Int32 nSeqsIdx = 0;
            for (oi = 0;  oi < oiEnd;  ++oi)
            {
                // row/col not used? (see if-statement above where nNumLDS was counted)
                if (!(aDataStartIdx[oi] != -1 &&
                        (!bFirstIsLabel || aLabelIdx[oi] != -1)))
                    continue;

                // get cell ranges for label and data

                SwRangeDescriptor aLabelDesc;
                SwRangeDescriptor aDataDesc;
                if (bDtaSrcIsColumns)   // use columns
                {
                    aLabelDesc.nTop     = aLabelIdx[oi];
                    aLabelDesc.nLeft    = oi;
                    aLabelDesc.nBottom  = aLabelDesc.nTop;
                    aLabelDesc.nRight   = oi;

                    aDataDesc.nTop      = aDataStartIdx[oi];
                    aDataDesc.nLeft     = oi;
                    aDataDesc.nBottom   = aDataDesc.nTop + aDataLen[oi] - 1;
                    aDataDesc.nRight    = oi;
                }
                else    // use rows
                {
                    aLabelDesc.nTop     = oi;
                    aLabelDesc.nLeft    = aLabelIdx[oi];
                    aLabelDesc.nBottom  = oi;
                    aLabelDesc.nRight   = aLabelDesc.nLeft;

                    aDataDesc.nTop      = oi;
                    aDataDesc.nLeft     = aDataStartIdx[oi];
                    aDataDesc.nBottom   = oi;
                    aDataDesc.nRight    = aDataDesc.nLeft + aDataLen[oi] - 1;
                }
                const OUString aBaseName =  pTableFormat->GetName() + ".";

                OUString aLabelRange;
                if (aLabelIdx[oi] != -1)
                {
                    aLabelRange = aBaseName
                        + sw_GetCellName( aLabelDesc.nLeft, aLabelDesc.nTop )
                        + ":" + sw_GetCellName( aLabelDesc.nRight, aLabelDesc.nBottom );
                }

                OUString aDataRange;
                if (aDataStartIdx[oi] != -1)
                {
                    aDataRange = aBaseName
                        + sw_GetCellName( aDataDesc.nLeft, aDataDesc.nTop )
                        + ":" + sw_GetCellName( aDataDesc.nRight, aDataDesc.nBottom );
                }

                // get cursors spanning the cell ranges for label and data
                std::shared_ptr<SwUnoCursor> pLabelUnoCursor;
                std::shared_ptr<SwUnoCursor> pDataUnoCursor;
                GetFormatAndCreateCursorFromRangeRep( pDoc, aLabelRange, &pTableFormat, pLabelUnoCursor);
                GetFormatAndCreateCursorFromRangeRep( pDoc, aDataRange,  &pTableFormat, pDataUnoCursor);

                // create XDataSequence's from cursors
                if (pLabelUnoCursor)
                    pLabelSeqs[ nSeqsIdx ] = new SwChartDataSequence( *this, *pTableFormat, pLabelUnoCursor );
                OSL_ENSURE( pDataUnoCursor, "pointer to data sequence missing" );
                if (pDataUnoCursor)
                    pDataSeqs [ nSeqsIdx ] = new SwChartDataSequence( *this, *pTableFormat, pDataUnoCursor );
                if (pLabelUnoCursor || pDataUnoCursor)
                    ++nSeqsIdx;
            }
            OSL_ENSURE( nSeqsIdx == nNumLDS, "mismatch between sequence size and num,ber of entries" );

            // build data source from data and label sequences
            uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > aLDS( nNumLDS );
            uno::Reference< chart2::data::XLabeledDataSequence > *pLDS = aLDS.getArray();
            for (sal_Int32 i = 0;  i < nNumLDS;  ++i)
            {
                SwChartLabeledDataSequence *pLabeledDtaSeq = new SwChartLabeledDataSequence;
                pLabeledDtaSeq->setLabel( pLabelSeqs[i] );
                pLabeledDtaSeq->setValues( pDataSeqs[i] );
                pLDS[i] = pLabeledDtaSeq;
            }

            // apply 'SequenceMapping' if it was provided
            sal_Int32 nSequenceMappingLen = aSequenceMapping.getLength();
            if (nSequenceMappingLen)
            {
                sal_Int32 *pSequenceMapping = aSequenceMapping.getArray();
                uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > aOld_LDS( aLDS );
                uno::Reference< chart2::data::XLabeledDataSequence > *pOld_LDS = aOld_LDS.getArray();

                sal_Int32 nNewCnt = 0;
                for (sal_Int32 i = 0;  i < nSequenceMappingLen;  ++i)
                {
                    // check that index to be used is valid
                    // and has not yet been used
                    sal_Int32 nIdx = pSequenceMapping[i];
                    if (0 <= nIdx && nIdx < nNumLDS && pOld_LDS[nIdx].is())
                    {
                        pLDS[nNewCnt++] = pOld_LDS[nIdx];

                        // mark index as being used already (avoids duplicate entries)
                        pOld_LDS[nIdx].clear();
                    }
                }
                // add not yet used 'old' sequences to new one
                for (sal_Int32 i = 0;  i < nNumLDS;  ++i)
                {
#if OSL_DEBUG_LEVEL > 1
                        if (!pOld_LDS[i].is())
                            i = i;
#endif
                    if (pOld_LDS[i].is())
                        pLDS[nNewCnt++] = pOld_LDS[i];
                }
                OSL_ENSURE( nNewCnt == nNumLDS, "unexpected size of resulting sequence" );
            }

            xRes = new SwChartDataSource( aLDS );
        }
    }

    return xRes;
}

sal_Bool SAL_CALL SwChartDataProvider::createDataSourcePossible(
        const uno::Sequence< beans::PropertyValue >& rArguments )
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    bool bPossible = true;
    try
    {
        Impl_createDataSource( rArguments, true );
    }
    catch (lang::IllegalArgumentException &)
    {
        bPossible = false;
    }

    return bPossible;
}

uno::Reference< chart2::data::XDataSource > SAL_CALL SwChartDataProvider::createDataSource(
        const uno::Sequence< beans::PropertyValue >& rArguments )
    throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return Impl_createDataSource( rArguments );
}

/**
 * Fix for #i79009
 * we need to return a property that has the same value as the property
 * 'CellRangeRepresentation' but for all rows which are increased by one.
 * E.g. Table1.A1:D5 -> Table1,A2:D6
 * Since the problem is only for old charts which did not support multiple
 * we do not need to provide that property/string if the 'CellRangeRepresentation'
 * contains multiple ranges.
 */
OUString SwChartDataProvider::GetBrokenCellRangeForExport(
    const OUString &rCellRangeRepresentation )
{
    // check that we do not have multiple ranges
    if (-1 == rCellRangeRepresentation.indexOf( ';' ))
    {
        // get current cell and table names
        OUString aTableName, aStartCell, aEndCell;
        GetTableAndCellsFromRangeRep( rCellRangeRepresentation,
            aTableName, aStartCell, aEndCell, false );
        sal_Int32 nStartCol = -1, nStartRow = -1, nEndCol = -1, nEndRow = -1;
        SwXTextTable::GetCellPosition( aStartCell, nStartCol, nStartRow );
        SwXTextTable::GetCellPosition( aEndCell, nEndCol, nEndRow );

        // get new cell names
        ++nStartRow;
        ++nEndRow;
        aStartCell = sw_GetCellName( nStartCol, nStartRow );
        aEndCell   = sw_GetCellName( nEndCol, nEndRow );

        return GetRangeRepFromTableAndCells( aTableName,
                aStartCell, aEndCell, false );
    }

    return OUString();
}

uno::Sequence< beans::PropertyValue > SAL_CALL SwChartDataProvider::detectArguments(
        const uno::Reference< chart2::data::XDataSource >& xDataSource )
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (bDisposed)
        throw lang::DisposedException();

    uno::Sequence< beans::PropertyValue > aResult;
    if (!xDataSource.is())
        return aResult;

    const uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > aDS_LDS( xDataSource->getDataSequences() );
    const uno::Reference< chart2::data::XLabeledDataSequence > *pDS_LDS = aDS_LDS.getConstArray();
    sal_Int32 nNumDS_LDS = aDS_LDS.getLength();

    if (nNumDS_LDS == 0)
    {
        OSL_FAIL( "XLabeledDataSequence in data source contains 0 entries" );
        return aResult;
    }

    SwFrameFormat *pTableFormat = nullptr;
    SwTable  *pTable    = nullptr;
    OUString  aTableName;
    sal_Int32 nTableRows = 0;
    sal_Int32 nTableCols = 0;

    // data used to build 'CellRangeRepresentation' from later on
    std::vector< std::vector< sal_Char > > aMap;

    uno::Sequence< sal_Int32 > aSequenceMapping( nNumDS_LDS );
    sal_Int32 *pSequenceMapping = aSequenceMapping.getArray();

    OUString aCellRanges;
    sal_Int16 nDtaSrcIsColumns = -1;// -1: don't know yet, 0: false, 1: true  -2: neither
    sal_Int32 nLabelSeqLen  = -1;   // used to see if labels are always used or not and have
                                    // the expected size of 1 (i.e. if FirstCellAsLabel can
                                    // be determined)
                                    // -1: don't know yet, 0: not used, 1: always a single labe cell, ...
                                    // -2: neither/failed
    for (sal_Int32 nDS1 = 0;  nDS1 < nNumDS_LDS;  ++nDS1)
    {
        uno::Reference< chart2::data::XLabeledDataSequence > xLabeledDataSequence( pDS_LDS[nDS1] );
        if( !xLabeledDataSequence.is() )
        {
            OSL_FAIL("got NULL for XLabeledDataSequence from Data source");
            continue;
        }
        const uno::Reference< chart2::data::XDataSequence > xCurLabel( xLabeledDataSequence->getLabel(), uno::UNO_QUERY );
        const uno::Reference< chart2::data::XDataSequence > xCurValues( xLabeledDataSequence->getValues(), uno::UNO_QUERY );

        // get sequence lengths for label and values.
        // (0 length is Ok)
        sal_Int32 nCurLabelSeqLen   = -1;
        sal_Int32 nCurValuesSeqLen  = -1;
        if (xCurLabel.is())
            nCurLabelSeqLen = xCurLabel->getData().getLength();
        if (xCurValues.is())
            nCurValuesSeqLen = xCurValues->getData().getLength();

        // check for consistent use of 'first cell as label'
        if (nLabelSeqLen == -1)     // set initial value to compare with below further on
            nLabelSeqLen = nCurLabelSeqLen;
        if (nLabelSeqLen != nCurLabelSeqLen)
            nLabelSeqLen = -2;  // failed / no consistent use of label cells

        // get table and cell names for label and values data sequences
        // (start and end cell will be sorted, i.e. start cell <= end cell)
        OUString aLabelTableName, aLabelStartCell, aLabelEndCell;
        OUString aValuesTableName, aValuesStartCell, aValuesEndCell;
        OUString aLabelRange, aValuesRange;
        if (xCurLabel.is())
            aLabelRange = xCurLabel->getSourceRangeRepresentation();
        if (xCurValues.is())
            aValuesRange = xCurValues->getSourceRangeRepresentation();
        if ((!aLabelRange.isEmpty() && !GetTableAndCellsFromRangeRep( aLabelRange,
                aLabelTableName, aLabelStartCell, aLabelEndCell ))  ||
            !GetTableAndCellsFromRangeRep( aValuesRange,
                aValuesTableName, aValuesStartCell, aValuesEndCell ))
        {
            return aResult; // failed -> return empty property sequence
        }

        // make sure all sequences use the same table
        if (aTableName.isEmpty())
            aTableName = aValuesTableName;  // get initial value to compare with
        if (aTableName.isEmpty() ||
             aTableName != aValuesTableName ||
            (!aLabelTableName.isEmpty() && aTableName != aLabelTableName))
        {
            return aResult; // failed -> return empty property sequence
        }

        // try to get 'DataRowSource' value (ROWS or COLUMNS) from inspecting
        // first and last cell used in both sequences

        sal_Int32 nFirstCol = -1, nFirstRow = -1, nLastCol = -1, nLastRow = -1;
        const OUString aCell( !aLabelStartCell.isEmpty() ? aLabelStartCell : aValuesStartCell );
        OSL_ENSURE( !aCell.isEmpty() , "start cell missing?" );
        SwXTextTable::GetCellPosition( aCell, nFirstCol, nFirstRow);
        SwXTextTable::GetCellPosition( aValuesEndCell, nLastCol, nLastRow);

        sal_Int16 nDirection = -1;  // -1: not yet set,  0: columns,  1: rows, -2: failed
        if (nFirstCol == nLastCol && nFirstRow == nLastRow) // a single cell...
        {
            OSL_ENSURE( nCurLabelSeqLen == 0 && nCurValuesSeqLen == 1,
                    "trying to determine 'DataRowSource': something's fishy... should have been a single cell");
            (void)nCurValuesSeqLen;
            nDirection = 0;     // default direction for a single cell should be 'columns'
        }
        else    // more than one cell is available (in values and label together!)
        {
            if (nFirstCol == nLastCol && nFirstRow != nLastRow)
                nDirection = 1;
            else if (nFirstCol != nLastCol && nFirstRow == nLastRow)
                nDirection = 0;
            else
            {
                OSL_FAIL( "trying to determine 'DataRowSource': unexpected case found" );
                nDirection = -2;
            }
        }
        // check for consistent direction of data source
        if (nDtaSrcIsColumns == -1)     // set initial value to compare with below
            nDtaSrcIsColumns = nDirection;
        if (nDtaSrcIsColumns != nDirection)
        {
            nDtaSrcIsColumns = -2;  // failed
        }

        if (nDtaSrcIsColumns == 0 || nDtaSrcIsColumns == 1)
        {
            // build data to obtain 'SequenceMapping' later on

            OSL_ENSURE( nDtaSrcIsColumns == 0  ||   /* rows */
                        nDtaSrcIsColumns == 1,      /* columns */
                    "unexpected value for 'nDtaSrcIsColumns'" );
            pSequenceMapping[nDS1] = nDtaSrcIsColumns ? nFirstCol : nFirstRow;

            // build data used to determine 'CellRangeRepresentation' later on

            GetTableByName( *pDoc, aTableName, &pTableFormat, &pTable );
            if (!pTable || pTable->IsTableComplex())
                return aResult; // failed -> return empty property sequence
            nTableRows = pTable->GetTabLines().size();
            nTableCols = pTable->GetTabLines().front()->GetTabBoxes().size();
            aMap.resize( nTableRows );
            for (sal_Int32 i = 0;  i < nTableRows;  ++i)
                aMap[i].resize( nTableCols );

            if (!aLabelStartCell.isEmpty() && !aLabelEndCell.isEmpty())
            {
                sal_Int32 nStartCol = -1, nStartRow = -1, nEndCol = -1, nEndRow = -1;
                SwXTextTable::GetCellPosition( aLabelStartCell, nStartCol, nStartRow );
                SwXTextTable::GetCellPosition( aLabelEndCell,   nEndCol,   nEndRow );
                if (nStartRow < 0 || nEndRow >= nTableRows ||
                    nStartCol < 0 || nEndCol >= nTableCols)
                {
                    return aResult; // failed -> return empty property sequence
                }
                for (sal_Int32 i = nStartRow;  i <= nEndRow;  ++i)
                {
                    for (sal_Int32 k = nStartCol;  k <= nEndCol;  ++k)
                    {
                        sal_Char &rChar = aMap[i][k];
                        if (rChar == '\0')   // check for overlapping values and/or labels
                            rChar = 'L';
                        else
                            return aResult; // failed -> return empty property sequence
                    }
                }
            }
            if (!aValuesStartCell.isEmpty() && !aValuesEndCell.isEmpty())
            {
                sal_Int32 nStartCol = -1, nStartRow = -1, nEndCol = -1, nEndRow = -1;
                SwXTextTable::GetCellPosition( aValuesStartCell, nStartCol, nStartRow );
                SwXTextTable::GetCellPosition( aValuesEndCell,   nEndCol,   nEndRow );
                if (nStartRow < 0 || nEndRow >= nTableRows ||
                    nStartCol < 0 || nEndCol >= nTableCols)
                {
                    return aResult; // failed -> return empty property sequence
                }
                for (sal_Int32 i = nStartRow;  i <= nEndRow;  ++i)
                {
                    for (sal_Int32 k = nStartCol;  k <= nEndCol;  ++k)
                    {
                        sal_Char &rChar = aMap[i][k];
                        if (rChar == '\0')   // check for overlapping values and/or labels
                            rChar = 'x';
                        else
                            return aResult; // failed -> return empty property sequence
                    }
                }
            }
        }

#if OSL_DEBUG_LEVEL > 0
        // do some extra sanity checking that the length of the sequences
        // matches their range representation
        {
            sal_Int32 nStartRow = -1, nStartCol = -1, nEndRow = -1, nEndCol = -1;
            if (xCurLabel.is())
            {
                SwXTextTable::GetCellPosition( aLabelStartCell, nStartCol, nStartRow);
                SwXTextTable::GetCellPosition( aLabelEndCell,   nEndCol,   nEndRow);
                OSL_ENSURE( (nStartCol == nEndCol && (nEndRow - nStartRow + 1) == xCurLabel->getData().getLength()) ||
                            (nStartRow == nEndRow && (nEndCol - nStartCol + 1) == xCurLabel->getData().getLength()),
                        "label sequence length does not match range representation!" );
            }
            if (xCurValues.is())
            {
                SwXTextTable::GetCellPosition( aValuesStartCell, nStartCol, nStartRow);
                SwXTextTable::GetCellPosition( aValuesEndCell,   nEndCol,   nEndRow);
                OSL_ENSURE( (nStartCol == nEndCol && (nEndRow - nStartRow + 1) == xCurValues->getData().getLength()) ||
                            (nStartRow == nEndRow && (nEndCol - nStartCol + 1) == xCurValues->getData().getLength()),
                        "value sequence length does not match range representation!" );
            }
        }
#endif
    } // for

    // build value for 'CellRangeRepresentation'

    const OUString aCellRangeBase = aTableName + ".";
    OUString aCurRange;
    for (sal_Int32 i = 0;  i < nTableRows;  ++i)
    {
        for (sal_Int32 k = 0;  k < nTableCols;  ++k)
        {
            if (aMap[i][k] != '\0')  // top-left cell of a sub-range found
            {
                // find rectangular sub-range to use
                sal_Int32 nRowIndex1 = i;   // row index
                sal_Int32 nColIndex1 = k;   // column index
                sal_Int32 nRowSubLen = 0;
                sal_Int32 nColSubLen = 0;
                while (nRowIndex1 < nTableRows && aMap[nRowIndex1++][k] != '\0')
                    ++nRowSubLen;
                // be aware of shifted sequences!
                // (according to the checks done prior the length should be ok)
                while (nColIndex1 < nTableCols && aMap[i][nColIndex1] != '\0'
                                       && aMap[i + nRowSubLen-1][nColIndex1] != '\0')
                {
                    ++nColIndex1;
                    ++nColSubLen;
                }
                OUString aStartCell( sw_GetCellName( k, i ) );
                OUString aEndCell( sw_GetCellName( k + nColSubLen - 1, i + nRowSubLen - 1) );
                aCurRange = aCellRangeBase + aStartCell + ":" + aEndCell;
                if (!aCellRanges.isEmpty())
                    aCellRanges += ";";
                aCellRanges += aCurRange;

                // clear already found sub-range from map
                for (sal_Int32 nRowIndex2 = 0;  nRowIndex2 < nRowSubLen;  ++nRowIndex2)
                    for (sal_Int32 nColumnIndex2 = 0;  nColumnIndex2 < nColSubLen;  ++nColumnIndex2)
                        aMap[i + nRowIndex2][k + nColumnIndex2] = '\0';
            }
        }
    }
    // to be nice to the user we now sort the cell ranges according to
    // rows or columns depending on the direction used in the data source
    uno::Sequence< OUString > aSortedRanges;
    GetSubranges( aCellRanges, aSortedRanges, false /*sub ranges should already be normalized*/ );
    SortSubranges( aSortedRanges, (nDtaSrcIsColumns == 1) );
    sal_Int32 nSortedRanges = aSortedRanges.getLength();
    const OUString *pSortedRanges = aSortedRanges.getConstArray();
    OUString aSortedCellRanges;
    for (sal_Int32 i = 0;  i < nSortedRanges;  ++i)
    {
        if (!aSortedCellRanges.isEmpty())
            aSortedCellRanges += ";";
        aSortedCellRanges += pSortedRanges[i];
    }

    // build value for 'SequenceMapping'

    uno::Sequence< sal_Int32 > aSortedMapping( aSequenceMapping );
    sal_Int32 *pSortedMapping = aSortedMapping.getArray();
    std::sort( pSortedMapping, pSortedMapping + aSortedMapping.getLength() );
    OSL_ENSURE( aSortedMapping.getLength() == nNumDS_LDS, "unexpected size of sequence" );
    bool bNeedSequenceMapping = false;
    for (sal_Int32 i = 0;  i < nNumDS_LDS;  ++i)
    {
        sal_Int32 *pIt = std::find( pSortedMapping, pSortedMapping + nNumDS_LDS,
                                    pSequenceMapping[i] );
        OSL_ENSURE( pIt, "index not found" );
        if (!pIt)
            return aResult; // failed -> return empty property sequence
        pSequenceMapping[i] = pIt - pSortedMapping;

        if (i != pSequenceMapping[i])
            bNeedSequenceMapping = true;
    }

    // check if 'SequenceMapping' is actually not required...
    // (don't write unnecessary properties to the XML file)
    if (!bNeedSequenceMapping)
        aSequenceMapping.realloc(0);

    // build resulting properties

    OSL_ENSURE(nLabelSeqLen >= 0 || nLabelSeqLen == -2 /*not used*/,
            "unexpected value for 'nLabelSeqLen'" );
    bool bFirstCellIsLabel = false;     // default value if 'nLabelSeqLen' could not properly determined
    if (nLabelSeqLen > 0) // == 0 means no label sequence in use
        bFirstCellIsLabel = true;

    OSL_ENSURE( !aSortedCellRanges.isEmpty(), "CellRangeRepresentation missing" );
    const OUString aBrokenCellRangeForExport( GetBrokenCellRangeForExport( aSortedCellRanges ) );

    aResult.realloc(5);
    sal_Int32 nProps = 0;
    aResult[nProps  ].Name = "FirstCellAsLabel";
    aResult[nProps++].Value <<= bFirstCellIsLabel;
    aResult[nProps  ].Name = "CellRangeRepresentation";
    aResult[nProps++].Value <<= aSortedCellRanges;
    if (!aBrokenCellRangeForExport.isEmpty())
    {
        aResult[nProps  ].Name = "BrokenCellRangeForExport";
        aResult[nProps++].Value <<= aBrokenCellRangeForExport;
    }
    if (nDtaSrcIsColumns == 0 || nDtaSrcIsColumns == 1)
    {
        chart::ChartDataRowSource eDataRowSource = (nDtaSrcIsColumns == 1) ?
                    chart::ChartDataRowSource_COLUMNS : chart::ChartDataRowSource_ROWS;
        aResult[nProps  ].Name = "DataRowSource";
        aResult[nProps++].Value <<= eDataRowSource;

        if (aSequenceMapping.getLength() != 0)
        {
            aResult[nProps  ].Name = "SequenceMapping";
            aResult[nProps++].Value <<= aSequenceMapping;
        }
    }
    aResult.realloc( nProps );

    return aResult;
}

uno::Reference< chart2::data::XDataSequence > SwChartDataProvider::Impl_createDataSequenceByRangeRepresentation(
        const OUString& rRangeRepresentation, bool bTestOnly )
    throw (lang::IllegalArgumentException, uno::RuntimeException,
           std::exception)
{
    if (bDisposed)
        throw lang::DisposedException();

    SwFrameFormat    *pTableFormat    = nullptr;    // pointer to table format
    std::shared_ptr<SwUnoCursor> pUnoCursor;    // pointer to new created cursor spanning the cell range
    GetFormatAndCreateCursorFromRangeRep( pDoc, rRangeRepresentation,
                                          &pTableFormat, pUnoCursor );
    if (!pTableFormat || !pUnoCursor)
        throw lang::IllegalArgumentException();

    // check that cursors point and mark are in a single row or column.
    OUString aCellRange( GetCellRangeName( *pTableFormat, *pUnoCursor ) );
    SwRangeDescriptor aDesc;
    FillRangeDescriptor( aDesc, aCellRange );
    if (aDesc.nTop != aDesc.nBottom  &&  aDesc.nLeft != aDesc.nRight)
        throw lang::IllegalArgumentException();

    OSL_ENSURE( pTableFormat && pUnoCursor, "table format or cursor missing" );
    uno::Reference< chart2::data::XDataSequence > xDataSeq;
    if (!bTestOnly)
        xDataSeq = new SwChartDataSequence( *this, *pTableFormat, pUnoCursor );

    return xDataSeq;
}

sal_Bool SAL_CALL SwChartDataProvider::createDataSequenceByRangeRepresentationPossible(
        const OUString& rRangeRepresentation )
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    bool bPossible = true;
    try
    {
        Impl_createDataSequenceByRangeRepresentation( rRangeRepresentation, true );
    }
    catch (lang::IllegalArgumentException &)
    {
        bPossible = false;
    }

    return bPossible;
}

uno::Reference< chart2::data::XDataSequence > SAL_CALL SwChartDataProvider::createDataSequenceByRangeRepresentation(
        const OUString& rRangeRepresentation )
    throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return Impl_createDataSequenceByRangeRepresentation( rRangeRepresentation );
}

uno::Reference< sheet::XRangeSelection > SAL_CALL SwChartDataProvider::getRangeSelection(  )
    throw (uno::RuntimeException, std::exception)
{
    // note: it is no error to return nothing here
    return uno::Reference< sheet::XRangeSelection >();
}

uno::Reference<css::chart2::data::XDataSequence> SAL_CALL
    SwChartDataProvider::createDataSequenceByValueArray(
        const OUString& /*aRole*/, const OUString& /*aRangeRepresentation*/ )
            throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception)
{
    return uno::Reference<css::chart2::data::XDataSequence>();
}

void SAL_CALL SwChartDataProvider::dispose(  )
    throw (uno::RuntimeException, std::exception)
{
    bool bMustDispose( false );
    {
        osl::MutexGuard  aGuard( GetChartMutex() );
        bMustDispose = !bDisposed;
        if (!bDisposed)
            bDisposed = true;
    }
    if (bMustDispose)
    {
        // dispose all data-sequences
        Map_Set_DataSequenceRef_t::iterator aIt( aDataSequences.begin() );
        while (aIt != aDataSequences.end())
        {
            DisposeAllDataSequences( (*aIt).first );
            ++aIt;
        }
        // release all references to data-sequences
        aDataSequences.clear();

        // require listeners to release references to this object
        lang::EventObject aEvtObj( dynamic_cast< chart2::data::XDataSequence * >(this) );
        aEvtListeners.disposeAndClear( aEvtObj );
    }
}

void SAL_CALL SwChartDataProvider::addEventListener(
        const uno::Reference< lang::XEventListener >& rxListener )
    throw (uno::RuntimeException, std::exception)
{
    osl::MutexGuard  aGuard( GetChartMutex() );
    if (!bDisposed && rxListener.is())
        aEvtListeners.addInterface( rxListener );
}

void SAL_CALL SwChartDataProvider::removeEventListener(
        const uno::Reference< lang::XEventListener >& rxListener )
    throw (uno::RuntimeException, std::exception)
{
    osl::MutexGuard  aGuard( GetChartMutex() );
    if (!bDisposed && rxListener.is())
        aEvtListeners.removeInterface( rxListener );
}

OUString SAL_CALL SwChartDataProvider::getImplementationName(  )
    throw (uno::RuntimeException, std::exception)
{
    return OUString("SwChartDataProvider");
}

sal_Bool SAL_CALL SwChartDataProvider::supportsService(const OUString& rServiceName )
    throw (uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL SwChartDataProvider::getSupportedServiceNames(  )
    throw (uno::RuntimeException, std::exception)
{
    return { "com.sun.star.chart2.data.DataProvider"};
}

void SwChartDataProvider::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
{
    // actually this function should be superfluous (need to check later)
    ClientModify(this, pOld, pNew );
}

void SwChartDataProvider::AddDataSequence( const SwTable &rTable, uno::Reference< chart2::data::XDataSequence > &rxDataSequence )
{
    aDataSequences[ &rTable ].insert( rxDataSequence );
}

void SwChartDataProvider::RemoveDataSequence( const SwTable &rTable, uno::Reference< chart2::data::XDataSequence > &rxDataSequence )
{
    aDataSequences[ &rTable ].erase( rxDataSequence );
}

void SwChartDataProvider::InvalidateTable( const SwTable *pTable )
{
    OSL_ENSURE( pTable, "table pointer is NULL" );
    if (pTable)
    {
        if (!bDisposed)
           pTable->GetFrameFormat()->GetDoc()->getIDocumentChartDataProviderAccess().GetChartControllerHelper().StartOrContinueLocking();

        const Set_DataSequenceRef_t &rSet = aDataSequences[ pTable ];
        Set_DataSequenceRef_t::const_iterator aIt( rSet.begin() );
        while (aIt != rSet.end())
        {
            uno::Reference< chart2::data::XDataSequence > xTemp(*aIt);  // temporary needed for g++ 3.3.5
            uno::Reference< util::XModifiable > xRef( xTemp, uno::UNO_QUERY );
            if (xRef.is())
            {
                // mark the sequence as 'dirty' and notify listeners
                xRef->setModified( sal_True );
            }
            ++aIt;
        }
    }
}

bool SwChartDataProvider::DeleteBox( const SwTable *pTable, const SwTableBox &rBox )
{
    bool bRes = false;
    OSL_ENSURE( pTable, "table pointer is NULL" );
    if (pTable)
    {
        if (!bDisposed)
            pTable->GetFrameFormat()->GetDoc()->getIDocumentChartDataProviderAccess().GetChartControllerHelper().StartOrContinueLocking();

        Set_DataSequenceRef_t &rSet = aDataSequences[ pTable ];

        // iterate over all data-sequences for that table...
        Set_DataSequenceRef_t::iterator aIt( rSet.begin() );
        Set_DataSequenceRef_t::iterator aEndIt( rSet.end() );
        Set_DataSequenceRef_t::iterator aDelIt;     // iterator used for deletion when appropriate
        while (aIt != aEndIt)
        {
            SwChartDataSequence *pDataSeq = nullptr;
            bool bNowEmpty = false;
            bool bSeqDisposed = false;

            // check if weak reference is still valid...
            uno::Reference< chart2::data::XDataSequence > xTemp(*aIt);  // temporary needed for g++ 3.3.5
            uno::Reference< chart2::data::XDataSequence > xRef( xTemp, uno::UNO_QUERY );
            if (xRef.is())
            {
                // then delete that table box (check if implementation cursor needs to be adjusted)
                pDataSeq = static_cast< SwChartDataSequence * >( xRef.get() );
                if (pDataSeq)
                {
                    try
                    {
                        bNowEmpty = pDataSeq->DeleteBox( rBox );
                    }
                    catch (const lang::DisposedException&)
                    {
                        bNowEmpty = true;
                        bSeqDisposed = true;
                    }

                    if (bNowEmpty)
                        aDelIt = aIt;
                }
            }
            ++aIt;

            if (bNowEmpty)
            {
                rSet.erase( aDelIt );
                if (pDataSeq && !bSeqDisposed)
                    pDataSeq->dispose();    // the current way to tell chart that sth. got removed
            }
        }
    }
    return bRes;
}

void SwChartDataProvider::DisposeAllDataSequences( const SwTable *pTable )
{
    OSL_ENSURE( pTable, "table pointer is NULL" );
    if (pTable)
    {
        if (!bDisposed)
            pTable->GetFrameFormat()->GetDoc()->getIDocumentChartDataProviderAccess().GetChartControllerHelper().StartOrContinueLocking();

        //! make a copy of the STL container!
        //! This is necessary since calling 'dispose' will implicitly remove an element
        //! of the original container, and thus any iterator in the original container
        //! would become invalid.
        const Set_DataSequenceRef_t aSet( aDataSequences[ pTable ] );

        Set_DataSequenceRef_t::const_iterator aIt( aSet.begin() );
        Set_DataSequenceRef_t::const_iterator aEndIt( aSet.end() );
        while (aIt != aEndIt)
        {
            uno::Reference< chart2::data::XDataSequence > xTemp(*aIt);  // temporary needed for g++ 3.3.5
            uno::Reference< lang::XComponent > xRef( xTemp, uno::UNO_QUERY );
            if (xRef.is())
            {
                xRef->dispose();
            }
            ++aIt;
        }
    }
}

/**
 * SwChartDataProvider::AddRowCols tries to notify charts of added columns
 * or rows and extends the value sequence respectively (if possible).
 * If those can be added to the end of existing value data-sequences those
 * sequences get modified accordingly and will send a modification
 * notification (calling 'setModified
 *
 * Since this function is a work-around for non existent Writer core functionality
 * (no arbitrary multi-selection in tables that can be used to define a
 * data-sequence) this function will be somewhat unreliable.
 * For example we will only try to adapt value sequences. For this we assume
 * that a sequence of length 1 is a label sequence and those with length >= 2
 * we presume to be value sequences. Also new cells can only be added in the
 * direction the value sequence is already pointing (rows / cols) and at the
 * start or end of the values data-sequence.
 * Nothing needs to be done if the new cells are in between the table cursors
 * point and mark since data-sequence are considered to consist of all cells
 * between those.
 * New rows/cols need to be added already to the table before calling
 * this function.
 */
void SwChartDataProvider::AddRowCols(
        const SwTable &rTable,
        const SwSelBoxes& rBoxes,
        sal_uInt16 nLines, bool bBehind )
{
    if (rTable.IsTableComplex())
        return;

    const size_t nBoxes = rBoxes.size();
    if (nBoxes < 1 || nLines < 1)
        return;

    SwTableBox* pFirstBox   = rBoxes[0];
    SwTableBox* pLastBox    = rBoxes.back();

    if (pFirstBox && pLastBox)
    {
        sal_Int32 nFirstCol = -1, nFirstRow = -1, nLastCol = -1, nLastRow = -1;
        SwXTextTable::GetCellPosition( pFirstBox->GetName(), nFirstCol, nFirstRow  );
        SwXTextTable::GetCellPosition( pLastBox->GetName(),  nLastCol,  nLastRow );

        bool bAddCols = false;  // default; also to be used if nBoxes == 1 :-/
        if (nFirstCol == nLastCol && nFirstRow != nLastRow)
            bAddCols = true;
        if (nFirstCol == nLastCol || nFirstRow == nLastRow)
        {
            //get range of indices in col/rows for new cells
            sal_Int32 nFirstNewCol = nFirstCol;
            sal_Int32 nFirstNewRow = bBehind ?  nFirstRow + 1 : nFirstRow - nLines;
            if (bAddCols)
            {
                OSL_ENSURE( nFirstCol == nLastCol, "column indices seem broken" );
                nFirstNewCol = bBehind ?  nFirstCol + 1 : nFirstCol - nLines;
                nFirstNewRow = nFirstRow;
            }

            // iterate over all data-sequences for the table
            const Set_DataSequenceRef_t &rSet = aDataSequences[ &rTable ];
            Set_DataSequenceRef_t::const_iterator aIt( rSet.begin() );
            while (aIt != rSet.end())
            {
                uno::Reference< chart2::data::XDataSequence > xTemp(*aIt);  // temporary needed for g++ 3.3.5
                uno::Reference< chart2::data::XTextualDataSequence > xRef( xTemp, uno::UNO_QUERY );
                if (xRef.is())
                {
                    const sal_Int32 nLen = xRef->getTextualData().getLength();
                    if (nLen > 1) // value data-sequence ?
                    {
                        uno::Reference< lang::XUnoTunnel > xTunnel( xRef, uno::UNO_QUERY );
                        if(xTunnel.is())
                        {
                            SwChartDataSequence *pDataSeq = reinterpret_cast< SwChartDataSequence * >(
                                    sal::static_int_cast< sal_IntPtr >( xTunnel->getSomething( SwChartDataSequence::getUnoTunnelId() )));

                            if (pDataSeq)
                            {
                                SwRangeDescriptor aDesc;
                                pDataSeq->FillRangeDesc( aDesc );

                                chart::ChartDataRowSource eDRSource = chart::ChartDataRowSource_COLUMNS;
                                if (aDesc.nTop == aDesc.nBottom && aDesc.nLeft != aDesc.nRight)
                                    eDRSource = chart::ChartDataRowSource_ROWS;

                                if (!bAddCols && eDRSource == chart::ChartDataRowSource_COLUMNS)
                                {
                                    // add rows: extend affected columns by newly added row cells
                                    pDataSeq->ExtendTo( true, nFirstNewRow, nLines );
                                }
                                else if (bAddCols && eDRSource == chart::ChartDataRowSource_ROWS)
                                {
                                    // add cols: extend affected rows by newly added column cells
                                    pDataSeq->ExtendTo( false, nFirstNewCol, nLines );
                                }
                            }
                        }
                    }
                }
                ++aIt;
            }

        }
    }
}

// XRangeXMLConversion
OUString SAL_CALL SwChartDataProvider::convertRangeToXML( const OUString& rRangeRepresentation )
    throw (uno::RuntimeException, lang::IllegalArgumentException,
           std::exception)
{
    SolarMutexGuard aGuard;
    if (bDisposed)
        throw lang::DisposedException();

    OUString aRes;

    // multiple ranges are delimited by a ';' like in
    // "Table1.A1:A4;Table1.C2:C5" the same table must be used in all ranges!
    sal_Int32 nNumRanges = comphelper::string::getTokenCount(rRangeRepresentation, ';');
    SwTable* pFirstFoundTable = nullptr;  // to check that only one table will be used
    sal_Int32 nPos = 0;
    for (sal_Int32 i = 0;  i < nNumRanges;  ++i)
    {
        const OUString aRange( rRangeRepresentation.getToken(0, ';', nPos) );
        SwFrameFormat    *pTableFormat  = nullptr; // pointer to table format
        std::shared_ptr<SwUnoCursor> pCursor;
        GetFormatAndCreateCursorFromRangeRep( pDoc, aRange, &pTableFormat, pCursor );
        if (!pTableFormat)
            throw lang::IllegalArgumentException();
        SwTable* pTable = SwTable::FindTable( pTableFormat );
        if  (pTable->IsTableComplex())
            throw uno::RuntimeException();

        // check that there is only one table used in all ranges
        if (!pFirstFoundTable)
            pFirstFoundTable = pTable;
        if (pTable != pFirstFoundTable)
            throw lang::IllegalArgumentException();

        OUString aTableName;
        OUString aStartCell;
        OUString aEndCell;
        if (!GetTableAndCellsFromRangeRep( aRange, aTableName, aStartCell, aEndCell ))
            throw lang::IllegalArgumentException();

        sal_Int32 nCol, nRow;
        SwXTextTable::GetCellPosition( aStartCell, nCol, nRow );
        if (nCol < 0 || nRow < 0)
            throw uno::RuntimeException();

        //!! following objects/functions are implemented in XMLRangeHelper.?xx
        //!! which is a copy of the respective file from chart2 !!
        XMLRangeHelper::CellRange aCellRange;
        aCellRange.aTableName = aTableName;
        aCellRange.aUpperLeft.nColumn   = nCol;
        aCellRange.aUpperLeft.nRow      = nRow;
        aCellRange.aUpperLeft.bIsEmpty  = false;
        if (aStartCell != aEndCell && !aEndCell.isEmpty())
        {
            SwXTextTable::GetCellPosition( aEndCell, nCol, nRow );
            if (nCol < 0 || nRow < 0)
                throw uno::RuntimeException();

            aCellRange.aLowerRight.nColumn   = nCol;
            aCellRange.aLowerRight.nRow      = nRow;
            aCellRange.aLowerRight.bIsEmpty  = false;
        }
        OUString aTmp( XMLRangeHelper::getXMLStringFromCellRange( aCellRange ) );
        if (!aRes.isEmpty()) // in case of multiple ranges add delimiter
            aRes += " ";
        aRes += aTmp;
    }

    return aRes;
}

OUString SAL_CALL SwChartDataProvider::convertRangeFromXML( const OUString& rXMLRange )
    throw ( uno::RuntimeException, lang::IllegalArgumentException, std::exception )
{
    SolarMutexGuard aGuard;
    if (bDisposed)
        throw lang::DisposedException();

    OUString aRes;

    // multiple ranges are delimited by a ' ' like in
    // "Table1.$A$1:.$A$4 Table1.$C$2:.$C$5" the same table must be used in all ranges!
    sal_Int32 nNumRanges = comphelper::string::getTokenCount(rXMLRange, ' ');
    OUString aFirstFoundTable; // to check that only one table will be used
    sal_Int32 nPos = 0;
    for (sal_Int32 i = 0;  i < nNumRanges;  ++i)
    {
        OUString aRange( rXMLRange.getToken(0, ' ', nPos) );

        //!! following objects and function are implemented in XMLRangeHelper.?xx
        //!! which is a copy of the respective file from chart2 !!
        XMLRangeHelper::CellRange aCellRange( XMLRangeHelper::getCellRangeFromXMLString( aRange ));

        // check that there is only one table used in all ranges
        if (aFirstFoundTable.isEmpty())
            aFirstFoundTable = aCellRange.aTableName;
        if (aCellRange.aTableName != aFirstFoundTable)
            throw lang::IllegalArgumentException();

        OUString aTmp = aCellRange.aTableName + "." +
                        sw_GetCellName( aCellRange.aUpperLeft.nColumn,
                                 aCellRange.aUpperLeft.nRow );
        // does cell range consist of more than a single cell?
        if (!aCellRange.aLowerRight.bIsEmpty)
        {
            aTmp += ":";
            aTmp += sw_GetCellName( aCellRange.aLowerRight.nColumn,
                                     aCellRange.aLowerRight.nRow );
        }

        if (!aRes.isEmpty()) // in case of multiple ranges add delimiter
            aRes += ";";
        aRes += aTmp;
    }

    return aRes;
}

SwChartDataSource::SwChartDataSource(
        const uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > &rLDS ) :
    aLDS( rLDS )
{
}

SwChartDataSource::~SwChartDataSource()
{
}

uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > SAL_CALL SwChartDataSource::getDataSequences(  )
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return aLDS;
}

OUString SAL_CALL SwChartDataSource::getImplementationName(  )
    throw (uno::RuntimeException, std::exception)
{
    return OUString("SwChartDataSource");
}

sal_Bool SAL_CALL SwChartDataSource::supportsService(const OUString& rServiceName )
    throw (uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL SwChartDataSource::getSupportedServiceNames(  )
    throw (uno::RuntimeException, std::exception)
{
    return { "com.sun.star.chart2.data.DataSource" };
}

SwChartDataSequence::SwChartDataSequence(
        SwChartDataProvider &rProvider,
        SwFrameFormat   &rTableFormat,
        std::shared_ptr<SwUnoCursor> pTableCursor ) :
    SwClient( &rTableFormat ),
    m_aEvtListeners( GetChartMutex() ),
    m_aModifyListeners( GetChartMutex() ),
    m_aRowLabelText( SW_RES( STR_CHART2_ROW_LABEL_TEXT ) ),
    m_aColLabelText( SW_RES( STR_CHART2_COL_LABEL_TEXT ) ),
    m_xDataProvider( &rProvider ),
    m_pDataProvider( &rProvider ),
    m_pTableCursor( pTableCursor ),
    m_pPropSet( aSwMapProvider.GetPropertySet( PROPERTY_MAP_CHART2_DATA_SEQUENCE ) )
{
    m_bDisposed = false;

    acquire();
    try
    {
        const SwTable* pTable = SwTable::FindTable( &rTableFormat );
        if (pTable)
        {
            uno::Reference< chart2::data::XDataSequence > xRef( dynamic_cast< chart2::data::XDataSequence * >(this), uno::UNO_QUERY );
            m_pDataProvider->AddDataSequence( *pTable, xRef );
            m_pDataProvider->addEventListener( dynamic_cast< lang::XEventListener * >(this) );
        }
        else {
            OSL_FAIL( "table missing" );
        }
    }
    catch (uno::RuntimeException &)
    {
        // TODO: shouldn't there be a call to release() here?
        throw;
    }
    catch (uno::Exception &)
    {
    }
    release();

#if OSL_DEBUG_LEVEL > 0
    // check if it can properly convert into a SwUnoTableCursor
    // which is required for some functions
    SwUnoTableCursor* pUnoTableCursor = dynamic_cast<SwUnoTableCursor*>(&(*m_pTableCursor));
    OSL_ENSURE(pUnoTableCursor, "SwChartDataSequence: cursor not SwUnoTableCursor");
    (void) pUnoTableCursor;
#endif
}

SwChartDataSequence::SwChartDataSequence( const SwChartDataSequence &rObj ) :
    SwChartDataSequenceBaseClass(),
    SwClient( rObj.GetFrameFormat() ),
    m_aEvtListeners( GetChartMutex() ),
    m_aModifyListeners( GetChartMutex() ),
    m_aRole( rObj.m_aRole ),
    m_aRowLabelText( SW_RES(STR_CHART2_ROW_LABEL_TEXT) ),
    m_aColLabelText( SW_RES(STR_CHART2_COL_LABEL_TEXT) ),
    m_xDataProvider( rObj.m_pDataProvider ),
    m_pDataProvider( rObj.m_pDataProvider ),
    m_pTableCursor( rObj.m_pTableCursor ),
    m_pPropSet( rObj.m_pPropSet )
{
    m_bDisposed = false;

    acquire();
    try
    {
        const SwTable* pTable = SwTable::FindTable( GetFrameFormat() );
        if (pTable)
        {
            uno::Reference< chart2::data::XDataSequence > xRef( dynamic_cast< chart2::data::XDataSequence * >(this), uno::UNO_QUERY );
            m_pDataProvider->AddDataSequence( *pTable, xRef );
            m_pDataProvider->addEventListener( dynamic_cast< lang::XEventListener * >(this) );
        }
        else {
            OSL_FAIL( "table missing" );
        }
    }
    catch (uno::RuntimeException &)
    {
        // TODO: shouldn't there be a call to release() here?
        throw;
    }
    catch (uno::Exception &)
    {
    }
    release();

#if OSL_DEBUG_LEVEL > 0
    // check if it can properly convert into a SwUnoTableCursor
    // which is required for some functions
    SwUnoTableCursor* pUnoTableCursor = dynamic_cast<SwUnoTableCursor*>(&(*m_pTableCursor));
    OSL_ENSURE(pUnoTableCursor, "SwChartDataSequence: cursor not SwUnoTableCursor");
    (void) pUnoTableCursor;
#endif
}

SwChartDataSequence::~SwChartDataSequence()
{
}

namespace
{
    class theSwChartDataSequenceUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSwChartDataSequenceUnoTunnelId > {};
}

const uno::Sequence< sal_Int8 > & SwChartDataSequence::getUnoTunnelId()
{
    return theSwChartDataSequenceUnoTunnelId::get().getSeq();
}

sal_Int64 SAL_CALL SwChartDataSequence::getSomething( const uno::Sequence< sal_Int8 > &rId )
    throw(uno::RuntimeException, std::exception)
{
    if( rId.getLength() == 16
        && 0 == memcmp( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >(this) );
    }
    return 0;
}


OUString SAL_CALL SwChartDataSequence::getSourceRangeRepresentation(  )
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (m_bDisposed)
        throw lang::DisposedException();

    OUString aRes;
    SwFrameFormat* pTableFormat = GetFrameFormat();
    if (pTableFormat)
    {
        const OUString aCellRange( GetCellRangeName( *pTableFormat, *m_pTableCursor ) );
        OSL_ENSURE( !aCellRange.isEmpty(), "failed to get cell range" );
        aRes = pTableFormat->GetName() + "." + aCellRange;
    }
    return aRes;
}

uno::Sequence< OUString > SAL_CALL SwChartDataSequence::generateLabel(
        chart2::data::LabelOrigin eLabelOrigin )
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (m_bDisposed)
        throw lang::DisposedException();

    uno::Sequence< OUString > aLabels;

    {
        SwRangeDescriptor aDesc;
        bool bOk = false;
        SwFrameFormat* pTableFormat = GetFrameFormat();
        SwTable* pTable = pTableFormat ? SwTable::FindTable( pTableFormat ) : nullptr;
        if (!pTableFormat || !pTable || pTable->IsTableComplex())
            throw uno::RuntimeException();
        else
        {
            const OUString aCellRange( GetCellRangeName( *pTableFormat, *m_pTableCursor ) );
            OSL_ENSURE( !aCellRange.isEmpty(), "failed to get cell range" );
            bOk = FillRangeDescriptor( aDesc, aCellRange );
            OSL_ENSURE( bOk, "failed to get SwRangeDescriptor" );
        }
        if (bOk)
        {
            aDesc.Normalize();
            sal_Int32 nColSpan = aDesc.nRight - aDesc.nLeft + 1;
            sal_Int32 nRowSpan = aDesc.nBottom - aDesc.nTop + 1;
            OSL_ENSURE( nColSpan == 1 || nRowSpan == 1,
                    "unexpected range of selected cells" );

            OUString aText; // label text to be returned
            bool bReturnEmptyText = false;
            bool bUseCol = true;
            if (eLabelOrigin == chart2::data::LabelOrigin_COLUMN)
                bUseCol = true;
            else if (eLabelOrigin == chart2::data::LabelOrigin_ROW)
                bUseCol = false;
            else if (eLabelOrigin == chart2::data::LabelOrigin_SHORT_SIDE)
            {
                bUseCol = nColSpan < nRowSpan;
                bReturnEmptyText = nColSpan == nRowSpan;
            }
            else if (eLabelOrigin == chart2::data::LabelOrigin_LONG_SIDE)
            {
                bUseCol = nColSpan > nRowSpan;
                bReturnEmptyText = nColSpan == nRowSpan;
            }
            else {
                OSL_FAIL( "unexpected case" );
            }

            // build label sequence

            sal_Int32 nSeqLen = bUseCol ? nColSpan : nRowSpan;
            aLabels.realloc( nSeqLen );
            OUString *pLabels = aLabels.getArray();
            for (sal_Int32 i = 0;  i < nSeqLen;  ++i)
            {
                if (!bReturnEmptyText)
                {
                    aText = bUseCol ? m_aColLabelText : m_aRowLabelText;
                    sal_Int32 nCol = aDesc.nLeft;
                    sal_Int32 nRow = aDesc.nTop;
                    if (bUseCol)
                        nCol = nCol + i;
                    else
                        nRow = nRow + i;
                    OUString aCellName( sw_GetCellName( nCol, nRow ) );

                    sal_Int32 nLen = aCellName.getLength();
                    if (nLen)
                    {
                        const sal_Unicode *pBuf = aCellName.getStr();
                        const sal_Unicode *pEnd = pBuf + nLen;
                        while (pBuf < pEnd && !('0' <= *pBuf && *pBuf <= '9'))
                            ++pBuf;
                        // start of number found?
                        if (pBuf < pEnd && ('0' <= *pBuf && *pBuf <= '9'))
                        {
                            OUString aRplc;
                            OUString aNew;
                            if (bUseCol)
                            {
                                aRplc = "%COLUMNLETTER";
                                aNew = OUString(aCellName.getStr(), pBuf - aCellName.getStr());
                            }
                            else
                            {
                                aRplc = "%ROWNUMBER";
                                aNew = OUString(pBuf, (aCellName.getStr() + nLen) - pBuf);
                            }
                            aText = aText.replaceFirst( aRplc, aNew );
                        }
                    }
                }
                pLabels[i] = aText;
            }
        }
    }

    return aLabels;
}

::sal_Int32 SAL_CALL SwChartDataSequence::getNumberFormatKeyByIndex(
    ::sal_Int32 /*nIndex*/ )
    throw (lang::IndexOutOfBoundsException,
           uno::RuntimeException, std::exception)
{
    return 0;
}

std::vector< css::uno::Reference< css::table::XCell > > SwChartDataSequence::GetCells()
{
    if (m_bDisposed)
        throw lang::DisposedException();
    auto pTableFormat(GetFrameFormat());
    if(!pTableFormat)
        return std::vector< css::uno::Reference< css::table::XCell > >();
    auto pTable(SwTable::FindTable(pTableFormat));
    if(pTable->IsTableComplex())
        return std::vector< css::uno::Reference< css::table::XCell > >();
    SwRangeDescriptor aDesc;
    if(!FillRangeDescriptor(aDesc, GetCellRangeName(*pTableFormat, *m_pTableCursor)))
        return std::vector< css::uno::Reference< css::table::XCell > >();
    return SwXCellRange::CreateXCellRange(m_pTableCursor, *pTableFormat, aDesc)->GetCells();
}

uno::Sequence< OUString > SAL_CALL SwChartDataSequence::getTextualData()
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    auto vCells(GetCells());
    uno::Sequence< OUString > vTextData(vCells.size());
    std::transform(vCells.begin(),
        vCells.end(),
        vTextData.begin(),
        [] (decltype(vCells)::value_type& xCell)
            { return static_cast<SwXCell*>(xCell.get())->getString(); });
    return vTextData;
}

uno::Sequence< uno::Any > SAL_CALL SwChartDataSequence::getData()
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    auto vCells(GetCells());
    uno::Sequence< uno::Any > vAnyData(vCells.size());
    std::transform(vCells.begin(),
        vCells.end(),
        vAnyData.begin(),
        [] (decltype(vCells)::value_type& xCell)
            { return static_cast<SwXCell*>(xCell.get())->GetAny(); });
    return vAnyData;
}

uno::Sequence< double > SAL_CALL SwChartDataSequence::getNumericalData()
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    auto vCells(GetCells());
    uno::Sequence< double > vNumData(vCells.size());
    std::transform(vCells.begin(),
        vCells.end(),
        vNumData.begin(),
        [] (decltype(vCells)::value_type& xCell)
            { return static_cast<SwXCell*>(xCell.get())->GetForcedNumericalValue(); });
    return vNumData;
}

uno::Reference< util::XCloneable > SAL_CALL SwChartDataSequence::createClone(  )
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (m_bDisposed)
        throw lang::DisposedException();
    return new SwChartDataSequence( *this );
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL SwChartDataSequence::getPropertySetInfo(  )
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (m_bDisposed)
        throw lang::DisposedException();

    static uno::Reference< beans::XPropertySetInfo > xRes = m_pPropSet->getPropertySetInfo();
    return xRes;
}

void SAL_CALL SwChartDataSequence::setPropertyValue(
        const OUString& rPropertyName,
        const uno::Any& rValue )
    throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (m_bDisposed)
        throw lang::DisposedException();

    if (rPropertyName == UNO_NAME_ROLE)
    {
        if ( !(rValue >>= m_aRole) )
            throw lang::IllegalArgumentException();
    }
    else
        throw beans::UnknownPropertyException();
}

uno::Any SAL_CALL SwChartDataSequence::getPropertyValue(
        const OUString& rPropertyName )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (m_bDisposed)
        throw lang::DisposedException();

    uno::Any aRes;
    if (rPropertyName == UNO_NAME_ROLE)
        aRes <<= m_aRole;
    else
        throw beans::UnknownPropertyException();

    return aRes;
}

void SAL_CALL SwChartDataSequence::addPropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/ )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    OSL_FAIL( "not implemented" );
}

void SAL_CALL SwChartDataSequence::removePropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/ )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    OSL_FAIL( "not implemented" );
}

void SAL_CALL SwChartDataSequence::addVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/ )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    OSL_FAIL( "not implemented" );
}

void SAL_CALL SwChartDataSequence::removeVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/ )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    OSL_FAIL( "not implemented" );
}

OUString SAL_CALL SwChartDataSequence::getImplementationName(  )
    throw (uno::RuntimeException, std::exception)
{
    return OUString("SwChartDataSequence");
}

sal_Bool SAL_CALL SwChartDataSequence::supportsService(const OUString& rServiceName )
    throw (uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL SwChartDataSequence::getSupportedServiceNames(  )
    throw (uno::RuntimeException, std::exception)
{
    return { "com.sun.star.chart2.data.DataSequence" };
}

void SwChartDataSequence::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew );

    // table was deleted or cursor was deleted
    if(!GetRegisteredIn() || !m_pTableCursor)
    {
        m_pTableCursor.reset(nullptr);
        dispose();
    }
    else
    {
        setModified( sal_True );
    }
}

sal_Bool SAL_CALL SwChartDataSequence::isModified(  )
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (m_bDisposed)
        throw lang::DisposedException();

    return sal_True;
}

void SAL_CALL SwChartDataSequence::setModified(
        sal_Bool bModified )
    throw (beans::PropertyVetoException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (m_bDisposed)
        throw lang::DisposedException();

    if (bModified)
        LaunchModifiedEvent( m_aModifyListeners, dynamic_cast< XModifyBroadcaster * >(this) );
}

void SAL_CALL SwChartDataSequence::addModifyListener(
        const uno::Reference< util::XModifyListener >& rxListener )
    throw (uno::RuntimeException, std::exception)
{
    osl::MutexGuard  aGuard( GetChartMutex() );
    if (!m_bDisposed && rxListener.is())
        m_aModifyListeners.addInterface( rxListener );
}

void SAL_CALL SwChartDataSequence::removeModifyListener(
        const uno::Reference< util::XModifyListener >& rxListener )
    throw (uno::RuntimeException, std::exception)
{
    osl::MutexGuard  aGuard( GetChartMutex() );
    if (!m_bDisposed && rxListener.is())
        m_aModifyListeners.removeInterface( rxListener );
}

void SAL_CALL SwChartDataSequence::disposing( const lang::EventObject& rSource )
    throw (uno::RuntimeException, std::exception)
{
    if (m_bDisposed)
        throw lang::DisposedException();
    if (rSource.Source == m_xDataProvider)
    {
        m_pDataProvider = nullptr;
        m_xDataProvider.clear();
    }
}

void SAL_CALL SwChartDataSequence::dispose(  )
    throw (uno::RuntimeException, std::exception)
{
    bool bMustDispose( false );
    {
        osl::MutexGuard  aGuard( GetChartMutex() );
        bMustDispose = !m_bDisposed;
        if (!m_bDisposed)
            m_bDisposed = true;
    }
    if (bMustDispose)
    {
        m_bDisposed = true;
        if (m_pDataProvider)
        {
            const SwTable* pTable = SwTable::FindTable( GetFrameFormat() );
            if (pTable)
            {
                uno::Reference< chart2::data::XDataSequence > xRef( dynamic_cast< chart2::data::XDataSequence * >(this), uno::UNO_QUERY );
                m_pDataProvider->RemoveDataSequence( *pTable, xRef );
            }
            else {
                OSL_FAIL( "table missing" );
            }

            //#i119653# The bug is crashed for an exception thrown by
            //SwCharDataSequence::setModified() because
            //the SwCharDataSequence object has been disposed.

            //Actually, the former design of SwClient will disconnect itself
            //from the notification list in its destructor.

            //But the SwCharDataSeqence won't be destructed but disposed in code
            //(the data member SwChartDataSequence::bDisposed will be set to
            //TRUE), the relationship between client and modification is not
            //released.

            //So any notification from modify object will lead to said
            //exception threw out.  Recorrect the logic of code in
            //SwChartDataSequence::Dispose(), release the relationship
            //here...
            SwModify* pLclRegisteredIn = GetRegisteredInNonConst();
            if (pLclRegisteredIn && pLclRegisteredIn->HasWriterListeners())
            {
                pLclRegisteredIn->Remove(this);
                m_pTableCursor.reset(nullptr);
            }
        }

        // require listeners to release references to this object
        lang::EventObject aEvtObj( dynamic_cast< chart2::data::XDataSequence * >(this) );
        m_aModifyListeners.disposeAndClear( aEvtObj );
        m_aEvtListeners.disposeAndClear( aEvtObj );
    }
}

void SAL_CALL SwChartDataSequence::addEventListener(
        const uno::Reference< lang::XEventListener >& rxListener )
    throw (uno::RuntimeException, std::exception)
{
    osl::MutexGuard  aGuard( GetChartMutex() );
    if (!m_bDisposed && rxListener.is())
        m_aEvtListeners.addInterface( rxListener );
}

void SAL_CALL SwChartDataSequence::removeEventListener(
        const uno::Reference< lang::XEventListener >& rxListener )
    throw (uno::RuntimeException, std::exception)
{
    osl::MutexGuard  aGuard( GetChartMutex() );
    if (!m_bDisposed && rxListener.is())
        m_aEvtListeners.removeInterface( rxListener );
}

bool SwChartDataSequence::DeleteBox( const SwTableBox &rBox )
{
    if (m_bDisposed)
        throw lang::DisposedException();

    // to be set if the last box of the data-sequence was removed here
    bool bNowEmpty = false;

    // if the implementation cursor gets affected (i.e. the box where it is located
    // in gets removed) we need to move it before that... (otherwise it does not need to change)

    const SwStartNode* pPointStartNode = m_pTableCursor->GetPoint()->nNode.GetNode().FindTableBoxStartNode();
    const SwStartNode* pMarkStartNode  = m_pTableCursor->GetMark()->nNode.GetNode().FindTableBoxStartNode();

    if (!m_pTableCursor->HasMark() || (pPointStartNode == rBox.GetSttNd()  &&  pMarkStartNode == rBox.GetSttNd()))
    {
        bNowEmpty = true;
    }
    else if (pPointStartNode == rBox.GetSttNd()  ||  pMarkStartNode == rBox.GetSttNd())
    {
        sal_Int32 nPointRow = -1, nPointCol = -1;
        sal_Int32 nMarkRow  = -1, nMarkCol  = -1;
        const SwTable* pTable = SwTable::FindTable( GetFrameFormat() );
        OUString aPointCellName( pTable->GetTableBox( pPointStartNode->GetIndex() )->GetName() );
        OUString aMarkCellName( pTable->GetTableBox( pMarkStartNode->GetIndex() )->GetName() );

        SwXTextTable::GetCellPosition( aPointCellName, nPointCol, nPointRow );
        SwXTextTable::GetCellPosition( aMarkCellName,  nMarkCol,  nMarkRow );
        OSL_ENSURE( nPointRow >= 0 && nPointCol >= 0, "invalid row and col" );
        OSL_ENSURE( nMarkRow >= 0 && nMarkCol >= 0, "invalid row and col" );

        // move vertical or horizontal?
        OSL_ENSURE( nPointRow == nMarkRow || nPointCol == nMarkCol,
                "row/col indices not matching" );
        OSL_ENSURE( nPointRow != nMarkRow || nPointCol != nMarkCol,
                "point and mark are identical" );
        bool bMoveVertical      = (nPointCol == nMarkCol);
        bool bMoveHorizontal    = (nPointRow == nMarkRow);

        // get movement direction
        bool bMoveLeft  = false;    // move left or right?
        bool bMoveUp    = false;    // move up or down?
        if (bMoveVertical)
        {
            if (pPointStartNode == rBox.GetSttNd()) // move point?
                bMoveUp = nPointRow > nMarkRow;
            else    // move mark
                bMoveUp = nMarkRow > nPointRow;
        }
        else if (bMoveHorizontal)
        {
            if (pPointStartNode == rBox.GetSttNd()) // move point?
                bMoveLeft = nPointCol > nMarkCol;
            else    // move mark
                bMoveLeft = nMarkCol > nPointCol;
        }
        else {
            OSL_FAIL( "neither vertical nor horizontal movement" );
        }

        // get new box (position) to use...
        sal_Int32 nRow = (pPointStartNode == rBox.GetSttNd()) ? nPointRow : nMarkRow;
        sal_Int32 nCol = (pPointStartNode == rBox.GetSttNd()) ? nPointCol : nMarkCol;
        if (bMoveVertical)
            nRow += bMoveUp ? -1 : +1;
        if (bMoveHorizontal)
            nCol += bMoveLeft ? -1 : +1;
        const OUString aNewCellName = sw_GetCellName( nCol, nRow );
        SwTableBox* pNewBox = const_cast<SwTableBox*>(pTable->GetTableBox( aNewCellName ));

        if (pNewBox)    // set new position (cell range) to use
        {
            // This is how you get the first content node of a row:
            // First get a SwNodeIndex pointing to the node after SwStartNode of the box...
            SwNodeIndex aIdx( *pNewBox->GetSttNd(), +1 );
            // This can be a SwContentNode, but might also be a table or section node,
            // therefore call GoNext
            SwContentNode *pCNd = aIdx.GetNode().GetContentNode();
            if (!pCNd)
                pCNd = GetFrameFormat()->GetDoc()->GetNodes().GoNext( &aIdx );
            // and then one can e.g. create a SwPosition:
            SwPosition aNewPos( *pCNd );   // new position to be used with cursor

            // if the mark is to be changed, make sure there is one
            if (pMarkStartNode == rBox.GetSttNd() && !m_pTableCursor->HasMark())
                m_pTableCursor->SetMark();

            // set cursor to new position
            SwPosition *pPos = (pPointStartNode == rBox.GetSttNd()) ?
                        m_pTableCursor->GetPoint() : m_pTableCursor->GetMark();
            if (pPos)
            {
                pPos->nNode     = aNewPos.nNode;
                pPos->nContent  = aNewPos.nContent;
            }
            else {
                OSL_FAIL( "neither point nor mark available for change" );
            }
        }
        else {
            OSL_FAIL( "failed to get position" );
        }
    }

    return bNowEmpty;
}

void SwChartDataSequence::FillRangeDesc( SwRangeDescriptor &rRangeDesc ) const
{
    SwFrameFormat* pTableFormat = GetFrameFormat();
    if(pTableFormat)
    {
        SwTable* pTable = SwTable::FindTable( pTableFormat );
        if(!pTable->IsTableComplex())
        {
            FillRangeDescriptor( rRangeDesc, GetCellRangeName( *pTableFormat, *m_pTableCursor ) );
        }
    }
}

/**
 * Extends the data-sequence by new cells added at the end of the direction
 * the data-sequence points to.
 * If the cells are already within the range of the sequence nothing needs
 * to be done.
 * If the cells are beyond the end of the sequence (are not adjacent to the
 * current last cell) nothing can be done. Only if the cells are adjacent to
 * the last cell they can be added.
 *
 * @returns true if the data-sequence was changed.
 * @param   bExtendCols - specifies if columns or rows are to be extended
 * @param   nFirstNew - index of first new row/col to be included in data-sequence
 * @param   nLastNew - index of last new row/col to be included in data-sequence
 */
bool SwChartDataSequence::ExtendTo( bool bExtendCol,
        sal_Int32 nFirstNew, sal_Int32 nCount )
{
    SwUnoTableCursor* pUnoTableCursor = dynamic_cast<SwUnoTableCursor*>(&(*m_pTableCursor));
    if (!pUnoTableCursor)
        return false;

    const SwStartNode *pStartNd  = nullptr;
    const SwTableBox  *pStartBox = nullptr;
    const SwTableBox  *pEndBox   = nullptr;

    const SwTable* pTable = SwTable::FindTable( GetFrameFormat() );
    OSL_ENSURE( !pTable->IsTableComplex(), "table too complex" );
    if (nCount < 1 || nFirstNew < 0 || pTable->IsTableComplex())
        return false;

    // get range descriptor (cell range) for current data-sequence

    pStartNd = pUnoTableCursor->GetPoint()->nNode.GetNode().FindTableBoxStartNode();
    pEndBox = pTable->GetTableBox( pStartNd->GetIndex() );
    const OUString aEndBox( pEndBox->GetName() );

    pStartNd = pUnoTableCursor->GetMark()->nNode.GetNode().FindTableBoxStartNode();
    pStartBox = pTable->GetTableBox( pStartNd->GetIndex() );
    const OUString aStartBox( pStartBox->GetName() );

    SwRangeDescriptor aDesc;
    // note that cell range here takes the newly added rows/cols already into account
    FillRangeDescriptor( aDesc, aStartBox + ":" + aEndBox );

    bool bChanged = false;
    OUString aNewStartCell;
    OUString aNewEndCell;
    if (bExtendCol && aDesc.nBottom + 1 == nFirstNew)
    {
        // new column cells adjacent to the bottom of the
        // current data-sequence to be added...
        OSL_ENSURE( aDesc.nLeft == aDesc.nRight, "data-sequence is not a column" );
        aNewStartCell = sw_GetCellName(aDesc.nLeft,  aDesc.nTop);
        aNewEndCell   = sw_GetCellName(aDesc.nRight, aDesc.nBottom + nCount);
        bChanged = true;
    }
    else if (bExtendCol && aDesc.nTop - nCount == nFirstNew)
    {
        // new column cells adjacent to the top of the
        // current data-sequence to be added...
        OSL_ENSURE( aDesc.nLeft == aDesc.nRight, "data-sequence is not a column" );
        aNewStartCell = sw_GetCellName(aDesc.nLeft,  aDesc.nTop - nCount);
        aNewEndCell   = sw_GetCellName(aDesc.nRight, aDesc.nBottom);
        bChanged = true;
    }
    else if (!bExtendCol && aDesc.nRight + 1 == nFirstNew)
    {
        // new row cells adjacent to the right of the
        // current data-sequence to be added...
        OSL_ENSURE( aDesc.nTop == aDesc.nBottom, "data-sequence is not a row" );
        aNewStartCell = sw_GetCellName(aDesc.nLeft, aDesc.nTop);
        aNewEndCell   = sw_GetCellName(aDesc.nRight + nCount, aDesc.nBottom);
        bChanged = true;
    }
    else if (!bExtendCol && aDesc.nLeft - nCount == nFirstNew)
    {
        // new row cells adjacent to the left of the
        // current data-sequence to be added...
        OSL_ENSURE( aDesc.nTop == aDesc.nBottom, "data-sequence is not a row" );
        aNewStartCell = sw_GetCellName(aDesc.nLeft - nCount, aDesc.nTop);
        aNewEndCell   = sw_GetCellName(aDesc.nRight, aDesc.nBottom);
        bChanged = true;
    }

    if (bChanged)
    {
        // move table cursor to new start and end of data-sequence
        const SwTableBox *pNewStartBox = pTable->GetTableBox( aNewStartCell );
        const SwTableBox *pNewEndBox   = pTable->GetTableBox( aNewEndCell );
        pUnoTableCursor->SetMark();
        pUnoTableCursor->GetPoint()->nNode = *pNewEndBox->GetSttNd();
        pUnoTableCursor->GetMark()->nNode  = *pNewStartBox->GetSttNd();
        pUnoTableCursor->Move( fnMoveForward, fnGoNode );
        pUnoTableCursor->MakeBoxSels();
    }

    return bChanged;
}

SwChartLabeledDataSequence::SwChartLabeledDataSequence() :
    aEvtListeners( GetChartMutex() ),
    aModifyListeners( GetChartMutex() )
{
    bDisposed = false;
}

SwChartLabeledDataSequence::~SwChartLabeledDataSequence()
{
}

uno::Reference< chart2::data::XDataSequence > SAL_CALL SwChartLabeledDataSequence::getValues(  )
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (bDisposed)
        throw lang::DisposedException();
    return xData;
}

void SwChartLabeledDataSequence::SetDataSequence(
        uno::Reference< chart2::data::XDataSequence >& rxDest,
        const uno::Reference< chart2::data::XDataSequence >& rxSource)
{
    uno::Reference< util::XModifyListener >  xML( dynamic_cast< util::XModifyListener* >(this), uno::UNO_QUERY );
    uno::Reference< lang::XEventListener >   xEL( dynamic_cast< lang::XEventListener* >(this), uno::UNO_QUERY );

    // stop listening to old data-sequence
    uno::Reference< util::XModifyBroadcaster > xMB( rxDest, uno::UNO_QUERY );
    if (xMB.is())
        xMB->removeModifyListener( xML );
    uno::Reference< lang::XComponent > xC( rxDest, uno::UNO_QUERY );
    if (xC.is())
        xC->removeEventListener( xEL );

    rxDest = rxSource;

    // start listening to new data-sequence
    xC.set( rxDest, uno::UNO_QUERY );
    if (xC.is())
        xC->addEventListener( xEL );
    xMB.set( rxDest, uno::UNO_QUERY );
    if (xMB.is())
        xMB->addModifyListener( xML );
}

void SAL_CALL SwChartLabeledDataSequence::setValues(
        const uno::Reference< chart2::data::XDataSequence >& rxSequence )
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (bDisposed)
        throw lang::DisposedException();

    if (xData != rxSequence)
    {
        SetDataSequence( xData, rxSequence );
        // inform listeners of changes
        LaunchModifiedEvent( aModifyListeners, dynamic_cast< XModifyBroadcaster * >(this) );
    }
}

uno::Reference< chart2::data::XDataSequence > SAL_CALL SwChartLabeledDataSequence::getLabel(  )
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (bDisposed)
        throw lang::DisposedException();
    return xLabels;
}

void SAL_CALL SwChartLabeledDataSequence::setLabel(
        const uno::Reference< chart2::data::XDataSequence >& rxSequence )
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (bDisposed)
        throw lang::DisposedException();

    if (xLabels != rxSequence)
    {
        SetDataSequence( xLabels, rxSequence );
        // inform listeners of changes
        LaunchModifiedEvent( aModifyListeners, dynamic_cast< XModifyBroadcaster * >(this) );
    }
}

uno::Reference< util::XCloneable > SAL_CALL SwChartLabeledDataSequence::createClone(  )
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (bDisposed)
        throw lang::DisposedException();

    uno::Reference< util::XCloneable > xRes;

    uno::Reference< util::XCloneable > xDataCloneable( xData, uno::UNO_QUERY );
    uno::Reference< util::XCloneable > xLabelsCloneable( xLabels, uno::UNO_QUERY );
    SwChartLabeledDataSequence *pRes = new SwChartLabeledDataSequence();
    if (xDataCloneable.is())
    {
        uno::Reference< chart2::data::XDataSequence > xDataClone( xDataCloneable->createClone(), uno::UNO_QUERY );
        pRes->setValues( xDataClone );
    }

    if (xLabelsCloneable.is())
    {
        uno::Reference< chart2::data::XDataSequence > xLabelsClone( xLabelsCloneable->createClone(), uno::UNO_QUERY );
        pRes->setLabel( xLabelsClone );
    }
    xRes = pRes;
    return xRes;
}

OUString SAL_CALL SwChartLabeledDataSequence::getImplementationName(  )
    throw (uno::RuntimeException, std::exception)
{
    return OUString("SwChartLabeledDataSequence");
}

sal_Bool SAL_CALL SwChartLabeledDataSequence::supportsService(
        const OUString& rServiceName )
    throw (uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL SwChartLabeledDataSequence::getSupportedServiceNames(  )
    throw (uno::RuntimeException, std::exception)
{
    return { "com.sun.star.chart2.data.LabeledDataSequence" };
}

void SAL_CALL SwChartLabeledDataSequence::disposing(
        const lang::EventObject& rSource )
    throw (uno::RuntimeException, std::exception)
{
    osl::MutexGuard  aGuard( GetChartMutex() );
    uno::Reference< uno::XInterface > xRef( rSource.Source );
    if (xRef == xData)
        xData.clear();
    if (xRef == xLabels)
        xLabels.clear();
    if (!xData.is() && !xLabels.is())
        dispose();
}

void SAL_CALL SwChartLabeledDataSequence::modified(
        const lang::EventObject& rEvent )
    throw (uno::RuntimeException, std::exception)
{
    if (rEvent.Source == xData || rEvent.Source == xLabels)
    {
        LaunchModifiedEvent( aModifyListeners, dynamic_cast< XModifyBroadcaster * >(this) );
    }
}

void SAL_CALL SwChartLabeledDataSequence::addModifyListener(
        const uno::Reference< util::XModifyListener >& rxListener )
    throw (uno::RuntimeException, std::exception)
{
    osl::MutexGuard  aGuard( GetChartMutex() );
    if (!bDisposed && rxListener.is())
        aModifyListeners.addInterface( rxListener );
}

void SAL_CALL SwChartLabeledDataSequence::removeModifyListener(
        const uno::Reference< util::XModifyListener >& rxListener )
    throw (uno::RuntimeException, std::exception)
{
    osl::MutexGuard  aGuard( GetChartMutex() );
    if (!bDisposed && rxListener.is())
        aModifyListeners.removeInterface( rxListener );
}

void SAL_CALL SwChartLabeledDataSequence::dispose(  )
    throw (uno::RuntimeException, std::exception)
{
    bool bMustDispose( false );
    {
        osl::MutexGuard  aGuard( GetChartMutex() );
        bMustDispose = !bDisposed;
        if (!bDisposed)
            bDisposed = true;
    }
    if (bMustDispose)
    {
        bDisposed = true;

        // require listeners to release references to this object
        lang::EventObject aEvtObj( dynamic_cast< chart2::data::XLabeledDataSequence * >(this) );
        aModifyListeners.disposeAndClear( aEvtObj );
        aEvtListeners.disposeAndClear( aEvtObj );
    }
}

void SAL_CALL SwChartLabeledDataSequence::addEventListener(
        const uno::Reference< lang::XEventListener >& rxListener )
    throw (uno::RuntimeException, std::exception)
{
    osl::MutexGuard  aGuard( GetChartMutex() );
    if (!bDisposed && rxListener.is())
        aEvtListeners.addInterface( rxListener );
}

void SAL_CALL SwChartLabeledDataSequence::removeEventListener(
        const uno::Reference< lang::XEventListener >& rxListener )
    throw (uno::RuntimeException, std::exception)
{
    osl::MutexGuard  aGuard( GetChartMutex() );
    if (!bDisposed && rxListener.is())
        aEvtListeners.removeInterface( rxListener );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
