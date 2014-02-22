/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <memory>
#include <algorithm>

#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/chart2/data/LabelOrigin.hpp>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <svl/zforlist.hxx>
#include <svx/charthelper.hxx>

#include <XMLRangeHelper.hxx>
#include <unochart.hxx>
#include <swtable.hxx>
#include <unoprnms.hxx>
#include <unomap.hxx>
#include <unomid.h>
#include <unocrsr.hxx>
#include <unotbl.hxx>
#include <doc.hxx>
#include <frmfmt.hxx>
#include <docsh.hxx>
#include <ndole.hxx>
#include <swtypes.hxx>
#include <unocore.hrc>
#include <docary.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/string.hxx>

using namespace ::com::sun::star;


extern void sw_GetCellPosition( const OUString &rCellName, sal_Int32 &rColumn, sal_Int32 &rRow);
extern OUString sw_GetCellName( sal_Int32 nColumn, sal_Int32 nRow );
extern int sw_CompareCellsByColFirst( const OUString &rCellName1, const OUString &rCellName2 );
extern int sw_CompareCellsByRowFirst( const OUString &rCellName1, const OUString &rCellName2 );
extern int sw_CompareCellRanges(
        const OUString &rRange1StartCell, const OUString &rRange1EndCell,
        const OUString &rRange2StartCell, const OUString &rRange2EndCell,
        sal_Bool bCmpColsFirst );
extern void sw_NormalizeRange( OUString &rCell1, OUString &rCell2 );


void SwChartHelper::DoUpdateAllCharts( SwDoc* pDoc )
{
    if (!pDoc)
        return;

    uno::Reference< frame::XModel > xRes;

    SwOLENode *pONd;
    SwStartNode *pStNd;
    SwNodeIndex aIdx( *pDoc->GetNodes().GetEndOfAutotext().StartOfSectionNode(), 1 );
    while( 0 != (pStNd = aIdx.GetNode().GetStartNode()) )
    {
        ++aIdx;
        if (0 != ( pONd = aIdx.GetNode().GetOLENode() ) &&
            ChartHelper::IsChart( pONd->GetOLEObj().GetObject() ) )
        {
            

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
    if (pDoc)   
        Disconnect();
}

void SwChartLockController_Helper::StartOrContinueLocking()
{
    if (!bIsLocked)
        LockAllCharts();
    aUnlockTimer.Start();   
}

void SwChartLockController_Helper::Disconnect()
{
    aUnlockTimer.Stop();
    UnlockAllCharts();
    pDoc = 0;
}

void SwChartLockController_Helper::LockUnlockAllCharts( sal_Bool bLock )
{
    if (!pDoc)
        return;

    const SwFrmFmts& rTblFmts = *pDoc->GetTblFrmFmts();
    for( sal_uInt16 n = 0; n < rTblFmts.size(); ++n )
    {
        SwTable* pTmpTbl;
        const SwTableNode* pTblNd;
        SwFrmFmt* pFmt = rTblFmts[ n ];

        if( 0 != ( pTmpTbl = SwTable::FindTable( pFmt ) ) &&
            0 != ( pTblNd = pTmpTbl->GetTableNode() ) &&
            pTblNd->GetNodes().IsDocNodes() )
        {
            uno::Reference< frame::XModel > xRes;
            SwOLENode *pONd;
            SwStartNode *pStNd;
            SwNodeIndex aIdx( *pDoc->GetNodes().GetEndOfAutotext().StartOfSectionNode(), 1 );
            while( 0 != (pStNd = aIdx.GetNode().GetStartNode()) )
            {
                ++aIdx;
                if (0 != ( pONd = aIdx.GetNode().GetOLENode() ) &&
                    !pONd->GetChartTblName().isEmpty() /* is chart object? */)
                {
                    uno::Reference < embed::XEmbeddedObject > xIP = pONd->GetOLEObj().GetOleRef();
                    if ( svt::EmbeddedObjectRef::TryRunningState( xIP ) )
                    {
                        xRes = uno::Reference < frame::XModel >( xIP->getComponent(), uno::UNO_QUERY );
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

IMPL_LINK( SwChartLockController_Helper, DoUnlockAllCharts, Timer *, /*pTimer*/ )
{
    UnlockAllCharts();
    return 0;
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




bool FillRangeDescriptor(
        SwRangeDescriptor &rDesc,
        const OUString &rCellRangeName )
{
    sal_Int32 nToken = -1 == rCellRangeName.indexOf('.') ? 0 : 1;
    OUString aCellRangeNoTableName( rCellRangeName.getToken( nToken, '.' ) );
    OUString aTLName( aCellRangeNoTableName.getToken(0, ':') );  
    OUString aBRName( aCellRangeNoTableName.getToken(1, ':') );  
    if(aTLName.isEmpty() || aBRName.isEmpty())
        return false;

    rDesc.nTop = rDesc.nLeft = rDesc.nBottom = rDesc.nRight = -1;
    sw_GetCellPosition( aTLName, rDesc.nLeft,  rDesc.nTop );
    sw_GetCellPosition( aBRName, rDesc.nRight, rDesc.nBottom );
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

static OUString GetCellRangeName( SwFrmFmt &rTblFmt, SwUnoCrsr &rTblCrsr )
{
    OUString aRes;

    

    SwUnoTableCrsr* pUnoTblCrsr = dynamic_cast<SwUnoTableCrsr*>(&rTblCrsr);
    if (!pUnoTblCrsr)
        return OUString();
    pUnoTblCrsr->MakeBoxSels();

    const SwStartNode*  pStart;
    const SwTableBox*   pStartBox   = 0;
    const SwTableBox*   pEndBox     = 0;

    pStart = pUnoTblCrsr->GetPoint()->nNode.GetNode().FindTableBoxStartNode();
    if (pStart)
    {
        const SwTable* pTable = SwTable::FindTable( &rTblFmt );
        pEndBox = pTable->GetTblBox( pStart->GetIndex());
        aRes = pEndBox->GetName();

        if(pUnoTblCrsr->HasMark())
        {
            pStart = pUnoTblCrsr->GetMark()->nNode.GetNode().FindTableBoxStartNode();
            pStartBox = pTable->GetTblBox( pStart->GetIndex());
        }
        OSL_ENSURE( pStartBox, "start box not found" );
        OSL_ENSURE( pEndBox, "end box not found" );
        
        if (*pUnoTblCrsr->GetPoint() < *pUnoTblCrsr->GetMark())
        {
            const SwTableBox* pTmpBox = pStartBox;
            pStartBox = pEndBox;
            pEndBox = pTmpBox;
        }

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
        sal_Bool bForceEndCellName )
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
        OUString &rTblName,
        OUString &rStartCell,
        OUString &rEndCell,
        bool bSortStartEndCells = true )
{
    
    
    OUString aTblName;    
    OUString aRange;    
    OUString aStartCell;  
    OUString aEndCell;    
    sal_Int32 nIdx = rRangeRepresentation.indexOf( '.' );
    if (nIdx >= 0)
    {
        aTblName = rRangeRepresentation.copy( 0, nIdx );
        aRange = rRangeRepresentation.copy( nIdx + 1 );
        sal_Int32 nPos = aRange.indexOf( ':' );
        if (nPos >= 0) 
        {
            aStartCell = aRange.copy( 0, nPos );
            aEndCell   = aRange.copy( nPos + 1 );

            
            
            if (bSortStartEndCells && 1 == sw_CompareCellsByColFirst( aStartCell, aEndCell ))
            {
                OUString aTmp( aStartCell );
                aStartCell  = aEndCell;
                aEndCell    = aTmp;
            }
        }
        else    
        {
            aStartCell = aEndCell = aRange;
        }
    }

    bool bSuccess = !aTblName.isEmpty() &&
                        !aStartCell.isEmpty() && !aEndCell.isEmpty();
    if (bSuccess)
    {
        rTblName    = aTblName;
        rStartCell  = aStartCell;
        rEndCell    = aEndCell;
    }
    return bSuccess;
}

static void GetTableByName( const SwDoc &rDoc, const OUString &rTableName,
        SwFrmFmt **ppTblFmt, SwTable **ppTable)
{
    SwFrmFmt *pTblFmt = NULL;

    
    
    sal_uInt16 nCount = rDoc.GetTblFrmFmtCount(true);
    for (sal_uInt16 i = 0; i < nCount && !pTblFmt; ++i)
    {
        SwFrmFmt& rTblFmt = rDoc.GetTblFrmFmt(i, true);
        if(rTableName == rTblFmt.GetName())
            pTblFmt = &rTblFmt;
    }

    if (ppTblFmt)
        *ppTblFmt = pTblFmt;

    if (ppTable)
        *ppTable = pTblFmt ? SwTable::FindTable( pTblFmt ) : 0;
}

static void GetFormatAndCreateCursorFromRangeRep(
        const SwDoc    *pDoc,
        const OUString &rRangeRepresentation,   
        SwFrmFmt    **ppTblFmt,     
        SwUnoCrsr   **ppUnoCrsr )   
                                    
{
    OUString aTblName;    
    OUString aStartCell;  
    OUString aEndCell;    
    bool bNamesFound = GetTableAndCellsFromRangeRep( rRangeRepresentation,
                                  aTblName, aStartCell, aEndCell );

    if (!bNamesFound)
    {
        if (ppTblFmt)
            *ppTblFmt   = NULL;
        if (ppUnoCrsr)
            *ppUnoCrsr  = NULL;
    }
    else
    {
        SwFrmFmt *pTblFmt = NULL;

        
        if (*ppTblFmt != NULL  &&  (*ppTblFmt)->GetName() == aTblName)
            pTblFmt = *ppTblFmt;
        else
            GetTableByName( *pDoc, aTblName, &pTblFmt, NULL );

        *ppTblFmt = pTblFmt;

        if (ppUnoCrsr != NULL)
        {
            *ppUnoCrsr = NULL;  

            SwTable *pTable = pTblFmt ? SwTable::FindTable( pTblFmt ) : 0;
            
            
            
            
            const SwTableBox* pTLBox =
                            pTable ? pTable->GetTblBox( aStartCell, true ) : 0;
            if(pTLBox)
            {
                
                UnoActionRemoveContext aRemoveContext(pTblFmt->GetDoc());
                const SwStartNode* pSttNd = pTLBox->GetSttNd();
                SwPosition aPos(*pSttNd);
                
                SwUnoCrsr* pUnoCrsr = pTblFmt->GetDoc()->CreateUnoCrsr(aPos, true);
                pUnoCrsr->Move( fnMoveForward, fnGoNode );
                pUnoCrsr->SetRemainInSection( sal_False );
                
                
                const SwTableBox* pBRBox = pTable->GetTblBox( aEndCell, true );
                if(pBRBox)
                {
                    pUnoCrsr->SetMark();
                    pUnoCrsr->GetPoint()->nNode = *pBRBox->GetSttNd();
                    pUnoCrsr->Move( fnMoveForward, fnGoNode );
                    SwUnoTableCrsr* pCrsr =
                        dynamic_cast<SwUnoTableCrsr*>(pUnoCrsr);
                    pCrsr->MakeBoxSels();

                    if (ppUnoCrsr)
                        *ppUnoCrsr = pCrsr;
                }
                else
                {
                    delete pUnoCrsr;
                }
            }
        }
    }
}

static bool GetSubranges( const OUString &rRangeRepresentation,
        uno::Sequence< OUString > &rSubRanges, bool bNormalize )
{
    bool bRes = true;
    OUString aRangesStr( rRangeRepresentation );
    sal_Int32 nLen = comphelper::string::getTokenCount(aRangesStr, ';');
    uno::Sequence< OUString > aRanges( nLen );

    sal_Int32 nCnt = 0;
    if (nLen != 0)
    {
        OUString *pRanges = aRanges.getArray();
        OUString aFirstTable;
        for( sal_Int32 i = 0; i < nLen && bRes; ++i )
        {
            OUString aRange( aRangesStr.getToken( i, ';' ) );
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
                                    aStartCell, aEndCell, sal_True );
                }

                
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

static void SortSubranges( uno::Sequence< OUString > &rSubRanges, sal_Bool bCmpByColumn )
{
    sal_Int32 nLen = rSubRanges.getLength();
    OUString *pSubRanges = rSubRanges.getArray();

    OUString aSmallestTblName;
    OUString aSmallestStartCell;
    OUString aSmallestEndCell;

    for (sal_Int32 i = 0;  i < nLen;  ++i)
    {
        sal_Int32 nIdxOfSmallest = i;
        GetTableAndCellsFromRangeRep( pSubRanges[nIdxOfSmallest],
                aSmallestTblName, aSmallestStartCell, aSmallestEndCell );
        if (aSmallestEndCell.isEmpty())
            aSmallestEndCell = aSmallestStartCell;

        for (sal_Int32 k = i+1;  k < nLen;  ++k)
        {
            
            OUString aTblName;
            OUString aStartCell;
            OUString aEndCell;
            GetTableAndCellsFromRangeRep( pSubRanges[k],
                    aTblName, aStartCell, aEndCell );
            if (aEndCell.isEmpty())
                aEndCell = aStartCell;

            
            if (-1 == sw_CompareCellRanges( aStartCell, aEndCell,
                                aSmallestStartCell, aSmallestEndCell, bCmpByColumn ))
            {
                nIdxOfSmallest = k;
                aSmallestTblName    = aTblName;
                aSmallestStartCell  = aStartCell;
                aSmallestEndCell    = aEndCell;
            }
        }

        
        OUString aTmp( pSubRanges[ nIdxOfSmallest ] );
        pSubRanges[ nIdxOfSmallest ] = pSubRanges[ i ];
        pSubRanges[ i ] = aTmp;
    }
}

SwChartDataProvider::SwChartDataProvider( const SwDoc* pSwDoc ) :
    aEvtListeners( GetChartMutex() ),
    pDoc( pSwDoc )
{
    bDisposed = sal_False;
}

SwChartDataProvider::~SwChartDataProvider()
{
}

uno::Reference< chart2::data::XDataSource > SwChartDataProvider::Impl_createDataSource(
        const uno::Sequence< beans::PropertyValue >& rArguments, sal_Bool bTestOnly )
    throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (bDisposed)
        throw lang::DisposedException();

    uno::Reference< chart2::data::XDataSource > xRes;

    if (!pDoc)
        throw uno::RuntimeException();

    
    OUString aRangeRepresentation;
    uno::Sequence< sal_Int32 > aSequenceMapping;
    sal_Bool bFirstIsLabel      = sal_False;
    sal_Bool bDtaSrcIsColumns   = sal_True; 
                                            
    OUString aChartOleObjectName;
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
    
    sal_Bool bOk = GetSubranges( aRangeRepresentation, aSubRanges, true );

    if (!bOk && pDoc && !aChartOleObjectName.isEmpty() )
    {
        
        
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
                aChartTableName = pOleNode->GetChartTblName();
                break;
            }
        }

        if( !aChartTableName.isEmpty() )
        {
            
            
            
            SwRangeDescriptor aDesc;
            if (aRangeRepresentation.isEmpty())
                return xRes;        
            aRangeRepresentation = aRangeRepresentation.copy( 1 );    
            FillRangeDescriptor( aDesc, aRangeRepresentation );
            aDesc.Normalize();
            if (aDesc.nTop <= 0)    
                return xRes;        
            aDesc.nTop      -= 1;
            aDesc.nBottom   -= 1;

            OUString aNewStartCell( sw_GetCellName( aDesc.nLeft, aDesc.nTop ) );
            OUString aNewEndCell( sw_GetCellName( aDesc.nRight, aDesc.nBottom ) );
            aRangeRepresentation = GetRangeRepFromTableAndCells(
                        aChartTableName, aNewStartCell, aNewEndCell, sal_True );
            bOk = GetSubranges( aRangeRepresentation, aSubRanges, true );
        }
    }
    if (!bOk)    
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

    
    SwFrmFmt    *pTblFmt  = 0;      
    SwUnoCrsr   *pUnoCrsr = 0;      
    std::auto_ptr< SwUnoCrsr > pAuto( pUnoCrsr );  
    if (aSubRanges.getLength() > 0)
        GetFormatAndCreateCursorFromRangeRep( pDoc, pSubRanges[0], &pTblFmt, &pUnoCrsr );
    if (!pTblFmt || !pUnoCrsr)
        throw lang::IllegalArgumentException();

    if(pTblFmt)
    {
        SwTable* pTable = SwTable::FindTable( pTblFmt );
        if(pTable->IsTblComplex())
            return xRes;    
        else
        {
            
            
            sal_Int32 nRows = pTable->GetTabLines().size();
            sal_Int32 nCols = pTable->GetTabLines().front()->GetTabBoxes().size();
            std::vector< std::vector< sal_Char > > aMap( nRows );
            for (sal_Int32 i = 0;  i < nRows;  ++i)
                aMap[i].resize( nCols );

            
            
            
            
            sal_Int32 nSubRanges = aSubRanges.getLength();
            for (sal_Int32 i = 0;  i < nSubRanges;  ++i)
            {
                OUString aTblName, aStartCell, aEndCell;
                bool bOk2 = GetTableAndCellsFromRangeRep(
                                    pSubRanges[i], aTblName, aStartCell, aEndCell );
                (void) bOk2;
                OSL_ENSURE( bOk2, "failed to get table and start/end cells" );

                sal_Int32 nStartRow, nStartCol, nEndRow, nEndCol;
                sw_GetCellPosition( aStartCell, nStartCol, nStartRow );
                sw_GetCellPosition( aEndCell,   nEndCol,   nEndRow );
                OSL_ENSURE( nStartRow <= nEndRow && nStartCol <= nEndCol,
                        "cell range not normalized");

                
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

            //
            
            //
            sal_Int32 oi;  
            sal_Int32 ii;  
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
            //
            for (oi = 0;  oi < oiEnd;  ++oi)
            {
                ii = 0;
                while (ii < iiEnd)
                {
                    sal_Char &rChar = bDtaSrcIsColumns ? aMap[ii][oi] : aMap[oi][ii];

                    
                    if (rChar == 'x' && bFirstIsLabel && aLabelIdx[oi] == -1)
                    {
                        aLabelIdx[oi] = ii;
                        rChar = 'L';    
                                        
                                        
                    }

                    
                    if (rChar == 'x' && aDataStartIdx[oi] == -1)
                    {
                        aDataStartIdx[oi] = ii;

                        
                        sal_Int32 nL = 0;
                        sal_Char c;
                        while (ii< iiEnd && 'x' == (c = bDtaSrcIsColumns ? aMap[ii][oi] : aMap[oi][ii]))
                        {
                            ++nL;   ++ii;
                        }
                        aDataLen[oi] = nL;

                        
                        
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

            
            
            
            
            sal_Int32 nNumLDS = 0;
            if (oiEnd > 0)
            {
                sal_Int32 nFirstSeqLen = 0;
                sal_Int32 nFirstSeqLabelIdx = -1;
                for (oi = 0;  oi < oiEnd;  ++oi)
                {
                    bool bFirstFound = false;
                    
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

            
            
            if (bTestOnly)
                return xRes;    

            
            uno::Sequence< uno::Reference< chart2::data::XDataSequence > > aLabelSeqs( nNumLDS );
            uno::Reference< chart2::data::XDataSequence > *pLabelSeqs = aLabelSeqs.getArray();
            uno::Sequence< uno::Reference< chart2::data::XDataSequence > > aDataSeqs( nNumLDS );
            uno::Reference< chart2::data::XDataSequence > *pDataSeqs = aDataSeqs.getArray();
            sal_Int32 nSeqsIdx = 0;
            for (oi = 0;  oi < oiEnd;  ++oi)
            {
                
                if (!(aDataStartIdx[oi] != -1 &&
                        (!bFirstIsLabel || aLabelIdx[oi] != -1)))
                    continue;

                
                //
                SwRangeDescriptor aLabelDesc;
                SwRangeDescriptor aDataDesc;
                if (bDtaSrcIsColumns)   
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
                else    
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
                OUString aBaseName =  pTblFmt->GetName() + ".";
                //
                OUString aLabelRange;
                if (aLabelIdx[oi] != -1)
                {
                    aLabelRange += aBaseName;
                    aLabelRange += sw_GetCellName( aLabelDesc.nLeft, aLabelDesc.nTop );
                    aLabelRange += ":";
                    aLabelRange += sw_GetCellName( aLabelDesc.nRight, aLabelDesc.nBottom );
                }
                //
                OUString aDataRange;
                if (aDataStartIdx[oi] != -1)
                {
                    aDataRange += aBaseName;
                    aDataRange += sw_GetCellName( aDataDesc.nLeft, aDataDesc.nTop );
                    aDataRange += ":";
                    aDataRange += sw_GetCellName( aDataDesc.nRight, aDataDesc.nBottom );
                }

                
                SwUnoCrsr   *pLabelUnoCrsr  = 0;
                SwUnoCrsr   *pDataUnoCrsr   = 0;
                GetFormatAndCreateCursorFromRangeRep( pDoc, aLabelRange, &pTblFmt, &pLabelUnoCrsr);
                GetFormatAndCreateCursorFromRangeRep( pDoc, aDataRange,  &pTblFmt, &pDataUnoCrsr);

                
                if (pLabelUnoCrsr)
                    pLabelSeqs[ nSeqsIdx ] = new SwChartDataSequence( *this, *pTblFmt, pLabelUnoCrsr );
                OSL_ENSURE( pDataUnoCrsr, "pointer to data sequence missing" );
                if (pDataUnoCrsr)
                    pDataSeqs [ nSeqsIdx ] = new SwChartDataSequence( *this, *pTblFmt, pDataUnoCrsr );
                if (pLabelUnoCrsr || pDataUnoCrsr)
                    ++nSeqsIdx;
            }
            OSL_ENSURE( nSeqsIdx == nNumLDS,
                    "mismatch between sequence size and num,ber of entries" );

            
            uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > aLDS( nNumLDS );
            uno::Reference< chart2::data::XLabeledDataSequence > *pLDS = aLDS.getArray();
            for (sal_Int32 i = 0;  i < nNumLDS;  ++i)
            {
                SwChartLabeledDataSequence *pLabeledDtaSeq = new SwChartLabeledDataSequence;
                pLabeledDtaSeq->setLabel( pLabelSeqs[i] );
                pLabeledDtaSeq->setValues( pDataSeqs[i] );
                pLDS[i] = pLabeledDtaSeq;
            }

            
            sal_Int32 nSequenceMappingLen = aSequenceMapping.getLength();
            if (nSequenceMappingLen)
            {
                sal_Int32 *pSequenceMapping = aSequenceMapping.getArray();
                uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > aOld_LDS( aLDS );
                uno::Reference< chart2::data::XLabeledDataSequence > *pOld_LDS = aOld_LDS.getArray();

                sal_Int32 nNewCnt = 0;
                for (sal_Int32 i = 0;  i < nSequenceMappingLen;  ++i)
                {
                    
                    
                    sal_Int32 nIdx = pSequenceMapping[i];
                    if (0 <= nIdx && nIdx < nNumLDS && pOld_LDS[nIdx].is())
                    {
                        pLDS[nNewCnt++] = pOld_LDS[nIdx];

                        
                        pOld_LDS[nIdx].clear();
                    }
                }
                
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
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    sal_Bool bPossible = sal_True;
    try
    {
        Impl_createDataSource( rArguments, sal_True );
    }
    catch (lang::IllegalArgumentException &)
    {
        bPossible = sal_False;
    }

    return bPossible;
}

uno::Reference< chart2::data::XDataSource > SAL_CALL SwChartDataProvider::createDataSource(
        const uno::Sequence< beans::PropertyValue >& rArguments )
    throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return Impl_createDataSource( rArguments );
}



//







OUString SwChartDataProvider::GetBrokenCellRangeForExport(
    const OUString &rCellRangeRepresentation )
{
    OUString aRes;

    
    if (-1 == rCellRangeRepresentation.indexOf( ';' ))
    {
        
        OUString aTblName, aStartCell, aEndCell;
        GetTableAndCellsFromRangeRep( rCellRangeRepresentation,
            aTblName, aStartCell, aEndCell, false );
        sal_Int32 nStartCol = -1, nStartRow = -1, nEndCol = -1, nEndRow = -1;
        sw_GetCellPosition( aStartCell, nStartCol, nStartRow );
        sw_GetCellPosition( aEndCell, nEndCol, nEndRow );

        
        ++nStartRow;
        ++nEndRow;
        aStartCell = sw_GetCellName( nStartCol, nStartRow );
        aEndCell   = sw_GetCellName( nEndCol, nEndRow );

        aRes = GetRangeRepFromTableAndCells( aTblName,
                aStartCell, aEndCell, sal_False );
    }

    return aRes;
}

uno::Sequence< beans::PropertyValue > SAL_CALL SwChartDataProvider::detectArguments(
        const uno::Reference< chart2::data::XDataSource >& xDataSource )
    throw (uno::RuntimeException)
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

    SwFrmFmt *pTableFmt = 0;
    SwTable  *pTable    = 0;
    OUString  aTableName;
    sal_Int32 nTableRows = 0;
    sal_Int32 nTableCols = 0;

    
    std::vector< std::vector< sal_Char > > aMap;

    uno::Sequence< sal_Int32 > aSequenceMapping( nNumDS_LDS );
    sal_Int32 *pSequenceMapping = aSequenceMapping.getArray();

    OUString aCellRanges;
    sal_Int16 nDtaSrcIsColumns = -1;
    sal_Int32 nLabelSeqLen  = -1;   
                                    
                                    
                                    
                                    
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

        
        
        sal_Int32 nCurLabelSeqLen   = -1;
        sal_Int32 nCurValuesSeqLen  = -1;
        if (xCurLabel.is())
            nCurLabelSeqLen = xCurLabel->getData().getLength();
        if (xCurValues.is())
            nCurValuesSeqLen = xCurValues->getData().getLength();

        
        if (nLabelSeqLen == -1)     
            nLabelSeqLen = nCurLabelSeqLen;
        if (nLabelSeqLen != nCurLabelSeqLen)
            nLabelSeqLen = -2;  

        
        
        OUString aLabelTblName, aLabelStartCell, aLabelEndCell;
        OUString aValuesTblName, aValuesStartCell, aValuesEndCell;
        OUString aLabelRange, aValuesRange;
        if (xCurLabel.is())
            aLabelRange = xCurLabel->getSourceRangeRepresentation();
        if (xCurValues.is())
            aValuesRange = xCurValues->getSourceRangeRepresentation();
        if ((!aLabelRange.isEmpty() && !GetTableAndCellsFromRangeRep( aLabelRange,
                aLabelTblName, aLabelStartCell, aLabelEndCell ))  ||
            !GetTableAndCellsFromRangeRep( aValuesRange,
                aValuesTblName, aValuesStartCell, aValuesEndCell ))
        {
            return aResult; 
        }

        
        if (aTableName.isEmpty())
            aTableName = aValuesTblName;  
        if (aTableName.isEmpty() ||
             aTableName != aValuesTblName ||
            (!aLabelTblName.isEmpty() && aTableName != aLabelTblName))
        {
            return aResult; 
        }


        
        
        //
        sal_Int32 nFirstCol = -1, nFirstRow = -1, nLastCol = -1, nLastRow = -1;
        OUString aCell( !aLabelStartCell.isEmpty() ? aLabelStartCell : aValuesStartCell );
        OSL_ENSURE( !aCell.isEmpty() , "start cell missing?" );
        sw_GetCellPosition( aCell, nFirstCol, nFirstRow);
        sw_GetCellPosition( aValuesEndCell, nLastCol, nLastRow);
        //
        sal_Int16 nDirection = -1;  
        if (nFirstCol == nLastCol && nFirstRow == nLastRow) 
        {
            OSL_ENSURE( nCurLabelSeqLen == 0 && nCurValuesSeqLen == 1,
                    "trying to determine 'DataRowSource': something's fishy... should have been a single cell");
            (void)nCurValuesSeqLen;
            nDirection = 0;     
        }
        else    
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
        
        if (nDtaSrcIsColumns == -1)     
            nDtaSrcIsColumns = nDirection;
        if (nDtaSrcIsColumns != nDirection)
        {
            nDtaSrcIsColumns = -2;  
        }


        if (nDtaSrcIsColumns == 0 || nDtaSrcIsColumns == 1)
        {
            
            //
            OSL_ENSURE( nDtaSrcIsColumns == 0  ||   /* rows */
                        nDtaSrcIsColumns == 1,      /* columns */
                    "unexpected value for 'nDtaSrcIsColumns'" );
            pSequenceMapping[nDS1] = nDtaSrcIsColumns ? nFirstCol : nFirstRow;


            
            //
            GetTableByName( *pDoc, aTableName, &pTableFmt, &pTable );
            if (!pTable || pTable->IsTblComplex())
                return aResult; 
            nTableRows = pTable->GetTabLines().size();
            nTableCols = pTable->GetTabLines().front()->GetTabBoxes().size();
            aMap.resize( nTableRows );
            for (sal_Int32 i = 0;  i < nTableRows;  ++i)
                aMap[i].resize( nTableCols );
            //
            if (!aLabelStartCell.isEmpty() && !aLabelEndCell.isEmpty())
            {
                sal_Int32 nStartCol = -1, nStartRow = -1, nEndCol = -1, nEndRow = -1;
                sw_GetCellPosition( aLabelStartCell, nStartCol, nStartRow );
                sw_GetCellPosition( aLabelEndCell,   nEndCol,   nEndRow );
                if (nStartRow < 0 || nEndRow >= nTableRows ||
                    nStartCol < 0 || nEndCol >= nTableCols)
                {
                    return aResult; 
                }
                for (sal_Int32 i = nStartRow;  i <= nEndRow;  ++i)
                {
                    for (sal_Int32 k = nStartCol;  k <= nEndCol;  ++k)
                    {
                        sal_Char &rChar = aMap[i][k];
                        if (rChar == '\0')   
                            rChar = 'L';
                        else
                            return aResult; 
                    }
                }
            }
            if (!aValuesStartCell.isEmpty() && !aValuesEndCell.isEmpty())
            {
                sal_Int32 nStartCol = -1, nStartRow = -1, nEndCol = -1, nEndRow = -1;
                sw_GetCellPosition( aValuesStartCell, nStartCol, nStartRow );
                sw_GetCellPosition( aValuesEndCell,   nEndCol,   nEndRow );
                if (nStartRow < 0 || nEndRow >= nTableRows ||
                    nStartCol < 0 || nEndCol >= nTableCols)
                {
                    return aResult; 
                }
                for (sal_Int32 i = nStartRow;  i <= nEndRow;  ++i)
                {
                    for (sal_Int32 k = nStartCol;  k <= nEndCol;  ++k)
                    {
                        sal_Char &rChar = aMap[i][k];
                        if (rChar == '\0')   
                            rChar = 'x';
                        else
                            return aResult; 
                    }
                }
            }
        }

#if OSL_DEBUG_LEVEL > 0
        
        
        {
            sal_Int32 nStartRow = -1, nStartCol = -1, nEndRow = -1, nEndCol = -1;
            if (xCurLabel.is())
            {
                sw_GetCellPosition( aLabelStartCell, nStartCol, nStartRow);
                sw_GetCellPosition( aLabelEndCell,   nEndCol,   nEndRow);
                OSL_ENSURE( (nStartCol == nEndCol && (nEndRow - nStartRow + 1) == xCurLabel->getData().getLength()) ||
                            (nStartRow == nEndRow && (nEndCol - nStartCol + 1) == xCurLabel->getData().getLength()),
                        "label sequence length does not match range representation!" );
            }
            if (xCurValues.is())
            {
                sw_GetCellPosition( aValuesStartCell, nStartCol, nStartRow);
                sw_GetCellPosition( aValuesEndCell,   nEndCol,   nEndRow);
                OSL_ENSURE( (nStartCol == nEndCol && (nEndRow - nStartRow + 1) == xCurValues->getData().getLength()) ||
                            (nStartRow == nEndRow && (nEndCol - nStartCol + 1) == xCurValues->getData().getLength()),
                        "value sequence length does not match range representation!" );
            }
        }
#endif
    } 


    
    //
    OUString aCellRangeBase = aTableName + ".";
    OUString aCurRange;
    for (sal_Int32 i = 0;  i < nTableRows;  ++i)
    {
        for (sal_Int32 k = 0;  k < nTableCols;  ++k)
        {
            if (aMap[i][k] != '\0')  
            {
                
                sal_Int32 nRowIndex1 = i;   
                sal_Int32 nColIndex1 = k;   
                sal_Int32 nRowSubLen = 0;
                sal_Int32 nColSubLen = 0;
                while (nRowIndex1 < nTableRows && aMap[nRowIndex1++][k] != '\0')
                    ++nRowSubLen;
                
                
                while (nColIndex1 < nTableCols && aMap[i][nColIndex1] != '\0'
                                       && aMap[i + nRowSubLen-1][nColIndex1] != '\0')
                {
                    ++nColIndex1;
                    ++nColSubLen;
                }
                OUString aStartCell( sw_GetCellName( k, i ) );
                OUString aEndCell( sw_GetCellName( k + nColSubLen - 1, i + nRowSubLen - 1) );
                aCurRange = aCellRangeBase;
                aCurRange += aStartCell;
                aCurRange += ":";
                aCurRange += aEndCell;
                if (!aCellRanges.isEmpty())
                    aCellRanges += ";";
                aCellRanges += aCurRange;

                
                for (sal_Int32 nRowIndex2 = 0;  nRowIndex2 < nRowSubLen;  ++nRowIndex2)
                    for (sal_Int32 nColumnIndex2 = 0;  nColumnIndex2 < nColSubLen;  ++nColumnIndex2)
                        aMap[i + nRowIndex2][k + nColumnIndex2] = '\0';
            }
        }
    }
    
    
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


    
    //
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
            return aResult; 
        pSequenceMapping[i] = pIt - pSortedMapping;

        if (i != pSequenceMapping[i])
            bNeedSequenceMapping = true;
    }

    
    
    if (!bNeedSequenceMapping)
        aSequenceMapping.realloc(0);

    //
    
    //
    OSL_ENSURE(nLabelSeqLen >= 0 || nLabelSeqLen == -2 /*not used*/,
            "unexpected value for 'nLabelSeqLen'" );
    sal_Bool bFirstCellIsLabel = sal_False;     
    if (nLabelSeqLen > 0) 
        bFirstCellIsLabel = sal_True;
    //
    OSL_ENSURE( !aSortedCellRanges.isEmpty(), "CellRangeRepresentation missing" );
    OUString aBrokenCellRangeForExport( GetBrokenCellRangeForExport( aSortedCellRanges ) );
    //
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
        const OUString& rRangeRepresentation, sal_Bool bTestOnly )
    throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    if (bDisposed)
        throw lang::DisposedException();

    SwFrmFmt    *pTblFmt    = 0;    
    SwUnoCrsr   *pUnoCrsr   = 0;    
    GetFormatAndCreateCursorFromRangeRep( pDoc, rRangeRepresentation,
                                          &pTblFmt, &pUnoCrsr );
    if (!pTblFmt || !pUnoCrsr)
        throw lang::IllegalArgumentException();

    
    OUString aCellRange( GetCellRangeName( *pTblFmt, *pUnoCrsr ) );
    SwRangeDescriptor aDesc;
    FillRangeDescriptor( aDesc, aCellRange );
    if (aDesc.nTop != aDesc.nBottom  &&  aDesc.nLeft != aDesc.nRight)
        throw lang::IllegalArgumentException();

    OSL_ENSURE( pTblFmt && pUnoCrsr, "table format or cursor missing" );
    uno::Reference< chart2::data::XDataSequence > xDataSeq;
    if (!bTestOnly)
        xDataSeq = new SwChartDataSequence( *this, *pTblFmt, pUnoCrsr );

    return xDataSeq;
}

sal_Bool SAL_CALL SwChartDataProvider::createDataSequenceByRangeRepresentationPossible(
        const OUString& rRangeRepresentation )
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    sal_Bool bPossible = sal_True;
    try
    {
        Impl_createDataSequenceByRangeRepresentation( rRangeRepresentation, sal_True );
    }
    catch (lang::IllegalArgumentException &)
    {
        bPossible = sal_False;
    }

    return bPossible;
}

uno::Reference< chart2::data::XDataSequence > SAL_CALL SwChartDataProvider::createDataSequenceByRangeRepresentation(
        const OUString& rRangeRepresentation )
    throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return Impl_createDataSequenceByRangeRepresentation( rRangeRepresentation );
}

uno::Reference< sheet::XRangeSelection > SAL_CALL SwChartDataProvider::getRangeSelection(  )
    throw (uno::RuntimeException)
{
    
    return uno::Reference< sheet::XRangeSelection >();
}

void SAL_CALL SwChartDataProvider::dispose(  )
    throw (uno::RuntimeException)
{
    bool bMustDispose( false );
    {
        osl::MutexGuard  aGuard( GetChartMutex() );
        bMustDispose = !bDisposed;
        if (!bDisposed)
            bDisposed = sal_True;
    }
    if (bMustDispose)
    {
        
        Map_Set_DataSequenceRef_t::iterator aIt( aDataSequences.begin() );
        while (aIt != aDataSequences.end())
        {
            DisposeAllDataSequences( (*aIt).first );
            ++aIt;
        }
        
        aDataSequences.clear();

        
        lang::EventObject aEvtObj( dynamic_cast< chart2::data::XDataSequence * >(this) );
        aEvtListeners.disposeAndClear( aEvtObj );
    }
}

void SAL_CALL SwChartDataProvider::addEventListener(
        const uno::Reference< lang::XEventListener >& rxListener )
    throw (uno::RuntimeException)
{
    osl::MutexGuard  aGuard( GetChartMutex() );
    if (!bDisposed && rxListener.is())
        aEvtListeners.addInterface( rxListener );
}

void SAL_CALL SwChartDataProvider::removeEventListener(
        const uno::Reference< lang::XEventListener >& rxListener )
    throw (uno::RuntimeException)
{
    osl::MutexGuard  aGuard( GetChartMutex() );
    if (!bDisposed && rxListener.is())
        aEvtListeners.removeInterface( rxListener );
}

OUString SAL_CALL SwChartDataProvider::getImplementationName(  )
    throw (uno::RuntimeException)
{
    return OUString("SwChartDataProvider");
}

sal_Bool SAL_CALL SwChartDataProvider::supportsService(const OUString& rServiceName )
    throw (uno::RuntimeException)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL SwChartDataProvider::getSupportedServiceNames(  )
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Sequence< OUString > aRes(1);
    aRes.getArray()[0] = "com.sun.star.chart2.data.DataProvider";
    return aRes;
}

void SwChartDataProvider::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
{
    
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
           pTable->GetFrmFmt()->GetDoc()->GetChartControllerHelper().StartOrContinueLocking();

        const Set_DataSequenceRef_t &rSet = aDataSequences[ pTable ];
        Set_DataSequenceRef_t::const_iterator aIt( rSet.begin() );
        while (aIt != rSet.end())
        {
            uno::Reference< chart2::data::XDataSequence > xTemp(*aIt);  
            uno::Reference< util::XModifiable > xRef( xTemp, uno::UNO_QUERY );
            if (xRef.is())
            {
                
                xRef->setModified( sal_True );
            }
            ++aIt;
        }
    }
}

sal_Bool SwChartDataProvider::DeleteBox( const SwTable *pTable, const SwTableBox &rBox )
{
    sal_Bool bRes = sal_False;
    OSL_ENSURE( pTable, "table pointer is NULL" );
    if (pTable)
    {
        if (!bDisposed)
            pTable->GetFrmFmt()->GetDoc()->GetChartControllerHelper().StartOrContinueLocking();

        Set_DataSequenceRef_t &rSet = aDataSequences[ pTable ];

        
        Set_DataSequenceRef_t::iterator aIt( rSet.begin() );
        Set_DataSequenceRef_t::iterator aEndIt( rSet.end() );
        Set_DataSequenceRef_t::iterator aDelIt;     
        while (aIt != aEndIt)
        {
            SwChartDataSequence *pDataSeq = 0;
            sal_Bool bNowEmpty = sal_False;
            sal_Bool bSeqDisposed = sal_False;

            
            uno::Reference< chart2::data::XDataSequence > xTemp(*aIt);  
            uno::Reference< chart2::data::XDataSequence > xRef( xTemp, uno::UNO_QUERY );
            if (xRef.is())
            {
                
                pDataSeq = static_cast< SwChartDataSequence * >( xRef.get() );
                if (pDataSeq)
                {
                    try
                    {
                        bNowEmpty = pDataSeq->DeleteBox( rBox );
                    }
                    catch (const lang::DisposedException&)
                    {
                        bNowEmpty = sal_True;
                        bSeqDisposed = sal_True;
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
                    pDataSeq->dispose();    
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
            pTable->GetFrmFmt()->GetDoc()->GetChartControllerHelper().StartOrContinueLocking();

        
        
        
        
        const Set_DataSequenceRef_t aSet( aDataSequences[ pTable ] );

        Set_DataSequenceRef_t::const_iterator aIt( aSet.begin() );
        Set_DataSequenceRef_t::const_iterator aEndIt( aSet.end() );
        while (aIt != aEndIt)
        {
            uno::Reference< chart2::data::XDataSequence > xTemp(*aIt);  
            uno::Reference< lang::XComponent > xRef( xTemp, uno::UNO_QUERY );
            if (xRef.is())
            {
                xRef->dispose();
            }
            ++aIt;
        }
    }
}







//













//
void SwChartDataProvider::AddRowCols(
        const SwTable &rTable,
        const SwSelBoxes& rBoxes,
        sal_uInt16 nLines, sal_Bool bBehind )
{
    if (rTable.IsTblComplex())
        return;

    const size_t nBoxes = rBoxes.size();
    if (nBoxes < 1 || nLines < 1)
        return;

    SwTableBox* pFirstBox   = rBoxes[0];
    SwTableBox* pLastBox    = rBoxes.back();

    if (pFirstBox && pLastBox)
    {
        sal_Int32 nFirstCol = -1, nFirstRow = -1, nLastCol = -1, nLastRow = -1;
        sw_GetCellPosition( pFirstBox->GetName(), nFirstCol, nFirstRow  );
        sw_GetCellPosition( pLastBox->GetName(),  nLastCol,  nLastRow );

        bool bAddCols = false;  
        if (nFirstCol == nLastCol && nFirstRow != nLastRow)
            bAddCols = true;
        if (nFirstCol == nLastCol || nFirstRow == nLastRow)
        {
            
            sal_Int32 nFirstNewCol = nFirstCol;
            sal_Int32 nFirstNewRow = bBehind ?  nFirstRow + 1 : nFirstRow - nLines;
            if (bAddCols)
            {
                OSL_ENSURE( nFirstCol == nLastCol, "column indices seem broken" );
                nFirstNewCol = bBehind ?  nFirstCol + 1 : nFirstCol - nLines;
                nFirstNewRow = nFirstRow;
            }

            
            const Set_DataSequenceRef_t &rSet = aDataSequences[ &rTable ];
            Set_DataSequenceRef_t::const_iterator aIt( rSet.begin() );
            while (aIt != rSet.end())
            {
                uno::Reference< chart2::data::XDataSequence > xTemp(*aIt);  
                uno::Reference< chart2::data::XTextualDataSequence > xRef( xTemp, uno::UNO_QUERY );
                if (xRef.is())
                {
                    const sal_Int32 nLen = xRef->getTextualData().getLength();
                    if (nLen > 1) 
                    {
                        SwChartDataSequence *pDataSeq = 0;
                        uno::Reference< lang::XUnoTunnel > xTunnel( xRef, uno::UNO_QUERY );
                        if(xTunnel.is())
                        {
                            pDataSeq = reinterpret_cast< SwChartDataSequence * >(
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
                                    
                                    pDataSeq->ExtendTo( true, nFirstNewRow, nLines );
                                }
                                else if (bAddCols && eDRSource == chart::ChartDataRowSource_ROWS)
                                {
                                    
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


OUString SAL_CALL SwChartDataProvider::convertRangeToXML( const OUString& rRangeRepresentation )
    throw ( uno::RuntimeException, lang::IllegalArgumentException )
{
    SolarMutexGuard aGuard;
    if (bDisposed)
        throw lang::DisposedException();

    OUString aRes;
    OUString aRangeRepresentation( rRangeRepresentation );

    
    
    sal_Int32 nNumRanges = comphelper::string::getTokenCount(aRangeRepresentation, ';');
    SwTable* pFirstFoundTable = 0;  
    for (sal_uInt16 i = 0;  i < nNumRanges;  ++i)
    {
        OUString aRange( aRangeRepresentation.getToken(i, ';') );
        SwFrmFmt    *pTblFmt  = 0;      
        GetFormatAndCreateCursorFromRangeRep( pDoc, aRange, &pTblFmt, NULL );
        if (!pTblFmt)
            throw lang::IllegalArgumentException();
        SwTable* pTable = SwTable::FindTable( pTblFmt );
        if  (pTable->IsTblComplex())
            throw uno::RuntimeException();

        
        if (!pFirstFoundTable)
            pFirstFoundTable = pTable;
        if (pTable != pFirstFoundTable)
            throw lang::IllegalArgumentException();

        OUString aTblName;
        OUString aStartCell;
        OUString aEndCell;
        if (!GetTableAndCellsFromRangeRep( aRange, aTblName, aStartCell, aEndCell ))
            throw lang::IllegalArgumentException();

        sal_Int32 nCol, nRow;
        sw_GetCellPosition( aStartCell, nCol, nRow );
        if (nCol < 0 || nRow < 0)
            throw uno::RuntimeException();

        
        
        XMLRangeHelper::CellRange aCellRange;
        aCellRange.aTableName = aTblName;
        aCellRange.aUpperLeft.nColumn   = nCol;
        aCellRange.aUpperLeft.nRow      = nRow;
        aCellRange.aUpperLeft.bIsEmpty  = false;
        if (aStartCell != aEndCell && !aEndCell.isEmpty())
        {
            sw_GetCellPosition( aEndCell, nCol, nRow );
            if (nCol < 0 || nRow < 0)
                throw uno::RuntimeException();

            aCellRange.aLowerRight.nColumn   = nCol;
            aCellRange.aLowerRight.nRow      = nRow;
            aCellRange.aLowerRight.bIsEmpty  = false;
        }
        OUString aTmp( XMLRangeHelper::getXMLStringFromCellRange( aCellRange ) );
        if (!aRes.isEmpty()) 
            aRes += " ";
        aRes += aTmp;
    }

    return aRes;
}

OUString SAL_CALL SwChartDataProvider::convertRangeFromXML( const OUString& rXMLRange )
    throw ( uno::RuntimeException, lang::IllegalArgumentException )
{
    SolarMutexGuard aGuard;
    if (bDisposed)
        throw lang::DisposedException();

    OUString aRes;
    OUString aXMLRange( rXMLRange );

    
    
    sal_Int32 nNumRanges = comphelper::string::getTokenCount(aXMLRange, ' ');
    OUString aFirstFoundTable; 
    for (sal_uInt16 i = 0;  i < nNumRanges;  ++i)
    {
        OUString aRange( aXMLRange.getToken(i, ' ') );

        
        
        XMLRangeHelper::CellRange aCellRange( XMLRangeHelper::getCellRangeFromXMLString( aRange ));

        
        if (aFirstFoundTable.isEmpty())
            aFirstFoundTable = aCellRange.aTableName;
        if (aCellRange.aTableName != aFirstFoundTable)
            throw lang::IllegalArgumentException();

        OUString aTmp = aCellRange.aTableName + "." +
                        sw_GetCellName( aCellRange.aUpperLeft.nColumn,
                                 aCellRange.aUpperLeft.nRow );
        
        if (!aCellRange.aLowerRight.bIsEmpty)
        {
            aTmp += ":";
            aTmp += sw_GetCellName( aCellRange.aLowerRight.nColumn,
                                     aCellRange.aLowerRight.nRow );
        }

        if (!aRes.isEmpty()) 
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
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return aLDS;
}

OUString SAL_CALL SwChartDataSource::getImplementationName(  )
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return OUString("SwChartDataSource");
}

sal_Bool SAL_CALL SwChartDataSource::supportsService(const OUString& rServiceName )
    throw (uno::RuntimeException)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL SwChartDataSource::getSupportedServiceNames(  )
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Sequence< OUString > aRes(1);
    aRes.getArray()[0] = "com.sun.star.chart2.data.DataSource";
    return aRes;
}

SwChartDataSequence::SwChartDataSequence(
        SwChartDataProvider &rProvider,
        SwFrmFmt   &rTblFmt,
        SwUnoCrsr  *pTableCursor ) :
    SwClient( &rTblFmt ),
    aEvtListeners( GetChartMutex() ),
    aModifyListeners( GetChartMutex() ),
    aRowLabelText( SW_RES( STR_CHART2_ROW_LABEL_TEXT ) ),
    aColLabelText( SW_RES( STR_CHART2_COL_LABEL_TEXT ) ),
    xDataProvider( &rProvider ),
    pDataProvider( &rProvider ),
    pTblCrsr( pTableCursor ),
    aCursorDepend( this, pTableCursor ),
    _pPropSet( aSwMapProvider.GetPropertySet( PROPERTY_MAP_CHART2_DATA_SEQUENCE ) )
{
    bDisposed = sal_False;

    acquire();
    try
    {
        const SwTable* pTable = SwTable::FindTable( &rTblFmt );
        if (pTable)
        {
            uno::Reference< chart2::data::XDataSequence > xRef( dynamic_cast< chart2::data::XDataSequence * >(this), uno::UNO_QUERY );
            pDataProvider->AddDataSequence( *pTable, xRef );
            pDataProvider->addEventListener( dynamic_cast< lang::XEventListener * >(this) );
        }
        else {
            OSL_FAIL( "table missing" );
        }
    }
    catch (uno::RuntimeException &)
    {
        throw;
    }
    catch (uno::Exception &)
    {
    }
    release();

#if OSL_DEBUG_LEVEL > 0
    
    
    SwUnoTableCrsr* pUnoTblCrsr = dynamic_cast<SwUnoTableCrsr*>(pTblCrsr);
    OSL_ENSURE(pUnoTblCrsr, "SwChartDataSequence: cursor not SwUnoTableCrsr");
    (void) pUnoTblCrsr;
#endif
}

SwChartDataSequence::SwChartDataSequence( const SwChartDataSequence &rObj ) :
    SwChartDataSequenceBaseClass(),
    SwClient( rObj.GetFrmFmt() ),
    aEvtListeners( GetChartMutex() ),
    aModifyListeners( GetChartMutex() ),
    aRole( rObj.aRole ),
    aRowLabelText( SW_RES(STR_CHART2_ROW_LABEL_TEXT) ),
    aColLabelText( SW_RES(STR_CHART2_COL_LABEL_TEXT) ),
    xDataProvider( rObj.pDataProvider ),
    pDataProvider( rObj.pDataProvider ),
    pTblCrsr( rObj.pTblCrsr->Clone() ),
    aCursorDepend( this, pTblCrsr ),
    _pPropSet( rObj._pPropSet )
{
    bDisposed = sal_False;

    acquire();
    try
    {
        const SwTable* pTable = SwTable::FindTable( GetFrmFmt() );
        if (pTable)
        {
            uno::Reference< chart2::data::XDataSequence > xRef( dynamic_cast< chart2::data::XDataSequence * >(this), uno::UNO_QUERY );
            pDataProvider->AddDataSequence( *pTable, xRef );
            pDataProvider->addEventListener( dynamic_cast< lang::XEventListener * >(this) );
        }
        else {
            OSL_FAIL( "table missing" );
        }
    }
    catch (uno::RuntimeException &)
    {
        throw;
    }
    catch (uno::Exception &)
    {
    }
    release();

#if OSL_DEBUG_LEVEL > 0
    
    
    SwUnoTableCrsr* pUnoTblCrsr = dynamic_cast<SwUnoTableCrsr*>(pTblCrsr);
    OSL_ENSURE(pUnoTblCrsr, "SwChartDataSequence: cursor not SwUnoTableCrsr");
    (void) pUnoTblCrsr;
#endif
}

SwChartDataSequence::~SwChartDataSequence()
{
    
    

    delete pTblCrsr;
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
    throw(uno::RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == memcmp( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >(this) );
    }
    return 0;
}

uno::Sequence< uno::Any > SAL_CALL SwChartDataSequence::getData(  )
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (bDisposed)
        throw lang::DisposedException();

    uno::Sequence< uno::Any > aRes;
    SwFrmFmt* pTblFmt = GetFrmFmt();
    if(pTblFmt)
    {
        SwTable* pTable = SwTable::FindTable( pTblFmt );
        if(!pTable->IsTblComplex())
        {
            SwRangeDescriptor aDesc;
            if (FillRangeDescriptor( aDesc, GetCellRangeName( *pTblFmt, *pTblCrsr ) ))
            {
                
                
                
                
                SwXCellRange aRange( pTblCrsr->Clone(), *pTblFmt, aDesc );
                aRange.GetDataSequence( &aRes, 0, 0 );
            }
        }
    }
    return aRes;
}

OUString SAL_CALL SwChartDataSequence::getSourceRangeRepresentation(  )
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (bDisposed)
        throw lang::DisposedException();

    OUString aRes;
    SwFrmFmt* pTblFmt = GetFrmFmt();
    if (pTblFmt)
    {
        aRes = pTblFmt->GetName();
        OUString aCellRange( GetCellRangeName( *pTblFmt, *pTblCrsr ) );
        OSL_ENSURE( !aCellRange.isEmpty(), "failed to get cell range" );
        aRes += ".";
        aRes += aCellRange;
    }
    return aRes;
}

uno::Sequence< OUString > SAL_CALL SwChartDataSequence::generateLabel(
        chart2::data::LabelOrigin eLabelOrigin )
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (bDisposed)
        throw lang::DisposedException();

    uno::Sequence< OUString > aLabels;

    {
        SwRangeDescriptor aDesc;
        bool bOk = false;
        SwFrmFmt* pTblFmt = GetFrmFmt();
        SwTable* pTable = pTblFmt ? SwTable::FindTable( pTblFmt ) : 0;
        if (!pTblFmt || !pTable || pTable->IsTblComplex())
            throw uno::RuntimeException();
        else
        {
            OUString aCellRange( GetCellRangeName( *pTblFmt, *pTblCrsr ) );
            OSL_ENSURE( !aCellRange.isEmpty(), "failed to get cell range" );
            bOk = FillRangeDescriptor( aDesc, aCellRange );
            OSL_ENSURE( bOk, "falied to get SwRangeDescriptor" );
        }
        if (bOk)
        {
            aDesc.Normalize();
            sal_Int32 nColSpan = aDesc.nRight - aDesc.nLeft + 1;
            sal_Int32 nRowSpan = aDesc.nBottom - aDesc.nTop + 1;
            OSL_ENSURE( nColSpan == 1 || nRowSpan == 1,
                    "unexpected range of selected cells" );

            OUString aTxt;    
            bool bReturnEmptyTxt = false;
            bool bUseCol = true;
            if (eLabelOrigin == chart2::data::LabelOrigin_COLUMN)
                bUseCol = true;
            else if (eLabelOrigin == chart2::data::LabelOrigin_ROW)
                bUseCol = false;
            else if (eLabelOrigin == chart2::data::LabelOrigin_SHORT_SIDE)
            {
                bUseCol = nColSpan < nRowSpan;
                bReturnEmptyTxt = nColSpan == nRowSpan;
            }
            else if (eLabelOrigin == chart2::data::LabelOrigin_LONG_SIDE)
            {
                bUseCol = nColSpan > nRowSpan;
                bReturnEmptyTxt = nColSpan == nRowSpan;
            }
            else {
                OSL_FAIL( "unexpected case" );
            }

            
            //
            sal_Int32 nSeqLen = bUseCol ? nColSpan : nRowSpan;
            aLabels.realloc( nSeqLen );
            OUString *pLabels = aLabels.getArray();
            for (sal_Int32 i = 0;  i < nSeqLen;  ++i)
            {
                if (!bReturnEmptyTxt)
                {
                    aTxt = bUseCol ? aColLabelText : aRowLabelText;
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
                            aTxt = aTxt.replaceFirst( aRplc, aNew );
                        }
                    }
                }
                pLabels[i] = aTxt;
            }
        }
    }

    return aLabels;
}

::sal_Int32 SAL_CALL SwChartDataSequence::getNumberFormatKeyByIndex(
    ::sal_Int32 /*nIndex*/ )
    throw (lang::IndexOutOfBoundsException,
           uno::RuntimeException)
{
    return 0;
}

uno::Sequence< OUString > SAL_CALL SwChartDataSequence::getTextualData(  )
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (bDisposed)
        throw lang::DisposedException();

    uno::Sequence< OUString > aRes;
    SwFrmFmt* pTblFmt = GetFrmFmt();
    if(pTblFmt)
    {
        SwTable* pTable = SwTable::FindTable( pTblFmt );
        if(!pTable->IsTblComplex())
        {
            SwRangeDescriptor aDesc;
            if (FillRangeDescriptor( aDesc, GetCellRangeName( *pTblFmt, *pTblCrsr ) ))
            {
                
                
                
                
                SwXCellRange aRange( pTblCrsr->Clone(), *pTblFmt, aDesc );
                aRange.GetDataSequence( 0, &aRes, 0 );
            }
        }
    }
    return aRes;
}

uno::Sequence< double > SAL_CALL SwChartDataSequence::getNumericalData(  )
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (bDisposed)
        throw lang::DisposedException();

    uno::Sequence< double > aRes;
    SwFrmFmt* pTblFmt = GetFrmFmt();
    if(pTblFmt)
    {
        SwTable* pTable = SwTable::FindTable( pTblFmt );
        if(!pTable->IsTblComplex())
        {
            SwRangeDescriptor aDesc;
            if (FillRangeDescriptor( aDesc, GetCellRangeName( *pTblFmt, *pTblCrsr ) ))
            {
                
                
                
                
                SwXCellRange aRange( pTblCrsr->Clone(), *pTblFmt, aDesc );

                
                
                aRange.GetDataSequence( 0, 0, &aRes, sal_True );
            }
        }
    }
    return aRes;
}

uno::Reference< util::XCloneable > SAL_CALL SwChartDataSequence::createClone(  )
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (bDisposed)
        throw lang::DisposedException();
    return new SwChartDataSequence( *this );
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL SwChartDataSequence::getPropertySetInfo(  )
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (bDisposed)
        throw lang::DisposedException();

    static uno::Reference< beans::XPropertySetInfo > xRes = _pPropSet->getPropertySetInfo();
    return xRes;
}

void SAL_CALL SwChartDataSequence::setPropertyValue(
        const OUString& rPropertyName,
        const uno::Any& rValue )
    throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (bDisposed)
        throw lang::DisposedException();

    if (rPropertyName == UNO_NAME_ROLE)
    {
        if ( !(rValue >>= aRole) )
            throw lang::IllegalArgumentException();
    }
    else
        throw beans::UnknownPropertyException();
}

uno::Any SAL_CALL SwChartDataSequence::getPropertyValue(
        const OUString& rPropertyName )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (bDisposed)
        throw lang::DisposedException();

    uno::Any aRes;
    if (rPropertyName == UNO_NAME_ROLE)
        aRes <<= aRole;
    else
        throw beans::UnknownPropertyException();

    return aRes;
}

void SAL_CALL SwChartDataSequence::addPropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/ )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_FAIL( "not implemented" );
}

void SAL_CALL SwChartDataSequence::removePropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/ )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_FAIL( "not implemented" );
}

void SAL_CALL SwChartDataSequence::addVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/ )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_FAIL( "not implemented" );
}

void SAL_CALL SwChartDataSequence::removeVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/ )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_FAIL( "not implemented" );
}

OUString SAL_CALL SwChartDataSequence::getImplementationName(  )
    throw (uno::RuntimeException)
{
    return OUString("SwChartDataSequence");
}

sal_Bool SAL_CALL SwChartDataSequence::supportsService(const OUString& rServiceName )
    throw (uno::RuntimeException)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL SwChartDataSequence::getSupportedServiceNames(  )
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Sequence< OUString > aRes(1);
    aRes.getArray()[0] = "com.sun.star.chart2.data.DataSequence";
    return aRes;
}

void SwChartDataSequence::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew );

    
    if(!GetRegisteredIn() || !aCursorDepend.GetRegisteredIn())
    {
        pTblCrsr = 0;
        dispose();
    }
    else
    {
        setModified( sal_True );
    }
}

sal_Bool SAL_CALL SwChartDataSequence::isModified(  )
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (bDisposed)
        throw lang::DisposedException();

    return sal_True;
}

void SAL_CALL SwChartDataSequence::setModified(
        ::sal_Bool bModified )
    throw (beans::PropertyVetoException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (bDisposed)
        throw lang::DisposedException();

    if (bModified)
        LaunchModifiedEvent( aModifyListeners, dynamic_cast< XModifyBroadcaster * >(this) );
}

void SAL_CALL SwChartDataSequence::addModifyListener(
        const uno::Reference< util::XModifyListener >& rxListener )
    throw (uno::RuntimeException)
{
    osl::MutexGuard  aGuard( GetChartMutex() );
    if (!bDisposed && rxListener.is())
        aModifyListeners.addInterface( rxListener );
}

void SAL_CALL SwChartDataSequence::removeModifyListener(
        const uno::Reference< util::XModifyListener >& rxListener )
    throw (uno::RuntimeException)
{
    osl::MutexGuard  aGuard( GetChartMutex() );
    if (!bDisposed && rxListener.is())
        aModifyListeners.removeInterface( rxListener );
}

void SAL_CALL SwChartDataSequence::disposing( const lang::EventObject& rSource )
    throw (uno::RuntimeException)
{
    if (bDisposed)
        throw lang::DisposedException();
    if (rSource.Source == xDataProvider)
    {
        pDataProvider = 0;
        xDataProvider.clear();
    }
}

void SAL_CALL SwChartDataSequence::dispose(  )
    throw (uno::RuntimeException)
{
    bool bMustDispose( false );
    {
        osl::MutexGuard  aGuard( GetChartMutex() );
        bMustDispose = !bDisposed;
        if (!bDisposed)
            bDisposed = sal_True;
    }
    if (bMustDispose)
    {
        bDisposed = sal_True;
        if (pDataProvider)
        {
            const SwTable* pTable = SwTable::FindTable( GetFrmFmt() );
            if (pTable)
            {
                uno::Reference< chart2::data::XDataSequence > xRef( dynamic_cast< chart2::data::XDataSequence * >(this), uno::UNO_QUERY );
                pDataProvider->RemoveDataSequence( *pTable, xRef );
            }
            else {
                OSL_FAIL( "table missing" );
            }

            
            
            
            //
            
            
            //
            
            
            
            
            //
            
            
            
            
            SwModify* pLclRegisteredIn = GetRegisteredInNonConst();
            if (pLclRegisteredIn && pLclRegisteredIn->GetDepends())
            {
                pLclRegisteredIn->Remove(this);
                pTblCrsr = NULL;
            }
        }

        
        lang::EventObject aEvtObj( dynamic_cast< chart2::data::XDataSequence * >(this) );
        aModifyListeners.disposeAndClear( aEvtObj );
        aEvtListeners.disposeAndClear( aEvtObj );
    }
}

void SAL_CALL SwChartDataSequence::addEventListener(
        const uno::Reference< lang::XEventListener >& rxListener )
    throw (uno::RuntimeException)
{
    osl::MutexGuard  aGuard( GetChartMutex() );
    if (!bDisposed && rxListener.is())
        aEvtListeners.addInterface( rxListener );
}

void SAL_CALL SwChartDataSequence::removeEventListener(
        const uno::Reference< lang::XEventListener >& rxListener )
    throw (uno::RuntimeException)
{
    osl::MutexGuard  aGuard( GetChartMutex() );
    if (!bDisposed && rxListener.is())
        aEvtListeners.removeInterface( rxListener );
}

sal_Bool SwChartDataSequence::DeleteBox( const SwTableBox &rBox )
{
    if (bDisposed)
        throw lang::DisposedException();

#if OSL_DEBUG_LEVEL > 1
    OUString aBoxName( rBox.GetName() );
#endif

    
    sal_Bool bNowEmpty = sal_False;

    
    
    //
    const SwStartNode* pPointStartNode = pTblCrsr->GetPoint()->nNode.GetNode().FindTableBoxStartNode();
    const SwStartNode* pMarkStartNode  = pTblCrsr->GetMark()->nNode.GetNode().FindTableBoxStartNode();
    //
    if (!pTblCrsr->HasMark() || (pPointStartNode == rBox.GetSttNd()  &&  pMarkStartNode == rBox.GetSttNd()))
    {
        bNowEmpty = sal_True;
    }
    else if (pPointStartNode == rBox.GetSttNd()  ||  pMarkStartNode == rBox.GetSttNd())
    {
        sal_Int32 nPointRow = -1, nPointCol = -1;
        sal_Int32 nMarkRow  = -1, nMarkCol  = -1;
        const SwTable* pTable = SwTable::FindTable( GetFrmFmt() );
        OUString aPointCellName( pTable->GetTblBox( pPointStartNode->GetIndex() )->GetName() );
        OUString aMarkCellName( pTable->GetTblBox( pMarkStartNode->GetIndex() )->GetName() );

        sw_GetCellPosition( aPointCellName, nPointCol, nPointRow );
        sw_GetCellPosition( aMarkCellName,  nMarkCol,  nMarkRow );
        OSL_ENSURE( nPointRow >= 0 && nPointCol >= 0, "invalid row and col" );
        OSL_ENSURE( nMarkRow >= 0 && nMarkCol >= 0, "invalid row and col" );

        
        OSL_ENSURE( nPointRow == nMarkRow || nPointCol == nMarkCol,
                "row/col indices not matching" );
        OSL_ENSURE( nPointRow != nMarkRow || nPointCol != nMarkCol,
                "point and mark are identical" );
        bool bMoveVertical      = (nPointCol == nMarkCol);
        bool bMoveHorizontal    = (nPointRow == nMarkRow);

        
        bool bMoveLeft  = false;    
        bool bMoveUp    = false;    
        if (bMoveVertical)
        {
            if (pPointStartNode == rBox.GetSttNd()) 
                bMoveUp = nPointRow > nMarkRow;
            else    
                bMoveUp = nMarkRow > nPointRow;
        }
        else if (bMoveHorizontal)
        {
            if (pPointStartNode == rBox.GetSttNd()) 
                bMoveLeft = nPointCol > nMarkCol;
            else    
                bMoveLeft = nMarkCol > nPointCol;
        }
        else {
            OSL_FAIL( "neither vertical nor horizontal movement" );
        }

        
        sal_Int32 nRow = (pPointStartNode == rBox.GetSttNd()) ? nPointRow : nMarkRow;
        sal_Int32 nCol = (pPointStartNode == rBox.GetSttNd()) ? nPointCol : nMarkCol;
        if (bMoveVertical)
            nRow += bMoveUp ? -1 : +1;
        if (bMoveHorizontal)
            nCol += bMoveLeft ? -1 : +1;
        OUString aNewCellName = sw_GetCellName( nCol, nRow );
        SwTableBox* pNewBox = (SwTableBox*) pTable->GetTblBox( aNewCellName );

        if (pNewBox)    
        {
            
            
            SwNodeIndex aIdx( *pNewBox->GetSttNd(), +1 );
            
            
            SwCntntNode *pCNd = aIdx.GetNode().GetCntntNode();
            if (!pCNd)
                pCNd = GetFrmFmt()->GetDoc()->GetNodes().GoNext( &aIdx );
            
            SwPosition aNewPos( *pCNd );   

            
            if (pMarkStartNode == rBox.GetSttNd() && !pTblCrsr->HasMark())
                pTblCrsr->SetMark();

            
            SwPosition *pPos = (pPointStartNode == rBox.GetSttNd()) ?
                        pTblCrsr->GetPoint() : pTblCrsr->GetMark();
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
    SwFrmFmt* pTblFmt = GetFrmFmt();
    if(pTblFmt)
    {
        SwTable* pTable = SwTable::FindTable( pTblFmt );
        if(!pTable->IsTblComplex())
        {
            FillRangeDescriptor( rRangeDesc, GetCellRangeName( *pTblFmt, *pTblCrsr ) );
        }
    }
}

/**
SwChartDataSequence::ExtendTo

extends the data-sequence by new cells added at the end of the direction
the data-sequence points to.
If the cells are already within the range of the sequence nothing needs
to be done.
If the cells are beyond the end of the sequence (are not adjacent to the
current last cell) nothing can be done. Only if the cells are adjacent to
the last cell they can be added.

@returns     true if the data-sequence was changed.
@param       bExtendCols
             specifies if columns or rows are to be extended
@param       nFirstNew
             index of first new row/col to be included in data-sequence
@param       nLastNew
             index of last new row/col to be included in data-sequence
*/
bool SwChartDataSequence::ExtendTo( bool bExtendCol,
        sal_Int32 nFirstNew, sal_Int32 nCount )
{
    bool bChanged = false;

    SwUnoTableCrsr* pUnoTblCrsr = dynamic_cast<SwUnoTableCrsr*>(pTblCrsr);
    

    const SwStartNode *pStartNd  = 0;
    const SwTableBox  *pStartBox = 0;
    const SwTableBox  *pEndBox   = 0;

    const SwTable* pTable = SwTable::FindTable( GetFrmFmt() );
    OSL_ENSURE( !pTable->IsTblComplex(), "table too complex" );
    if (nCount < 1 || nFirstNew < 0 || pTable->IsTblComplex())
        return false;

    //
    
    //
    pStartNd = pUnoTblCrsr->GetPoint()->nNode.GetNode().FindTableBoxStartNode();
    pEndBox = pTable->GetTblBox( pStartNd->GetIndex() );
    const OUString aEndBox( pEndBox->GetName() );
    //
    pStartNd = pUnoTblCrsr->GetMark()->nNode.GetNode().FindTableBoxStartNode();
    pStartBox = pTable->GetTblBox( pStartNd->GetIndex() );
    const OUString aStartBox( pStartBox->GetName() );
    //
    OUString aCellRange( aStartBox );     
    aCellRange += ":";
    aCellRange += aEndBox;
    SwRangeDescriptor aDesc;
    FillRangeDescriptor( aDesc, aCellRange );

    OUString aNewStartCell;
    OUString aNewEndCell;
    if (bExtendCol && aDesc.nBottom + 1 == nFirstNew)
    {
        
        
        OSL_ENSURE( aDesc.nLeft == aDesc.nRight, "data-sequence is not a column" );
        aNewStartCell = sw_GetCellName(aDesc.nLeft,  aDesc.nTop);
        aNewEndCell   = sw_GetCellName(aDesc.nRight, aDesc.nBottom + nCount);
        bChanged = true;
    }
    else if (bExtendCol && aDesc.nTop - nCount == nFirstNew)
    {
        
        
        OSL_ENSURE( aDesc.nLeft == aDesc.nRight, "data-sequence is not a column" );
        aNewStartCell = sw_GetCellName(aDesc.nLeft,  aDesc.nTop - nCount);
        aNewEndCell   = sw_GetCellName(aDesc.nRight, aDesc.nBottom);
        bChanged = true;
    }
    else if (!bExtendCol && aDesc.nRight + 1 == nFirstNew)
    {
        
        
        OSL_ENSURE( aDesc.nTop == aDesc.nBottom, "data-sequence is not a row" );
        aNewStartCell = sw_GetCellName(aDesc.nLeft, aDesc.nTop);
        aNewEndCell   = sw_GetCellName(aDesc.nRight + nCount, aDesc.nBottom);
        bChanged = true;
    }
    else if (!bExtendCol && aDesc.nLeft - nCount == nFirstNew)
    {
        
        
        OSL_ENSURE( aDesc.nTop == aDesc.nBottom, "data-sequence is not a row" );
        aNewStartCell = sw_GetCellName(aDesc.nLeft - nCount, aDesc.nTop);
        aNewEndCell   = sw_GetCellName(aDesc.nRight, aDesc.nBottom);
        bChanged = true;
    }

    if (bChanged)
    {
        
        const SwTableBox *pNewStartBox = pTable->GetTblBox( aNewStartCell );
        const SwTableBox *pNewEndBox   = pTable->GetTblBox( aNewEndCell );
        pUnoTblCrsr->SetMark();
        pUnoTblCrsr->GetPoint()->nNode = *pNewEndBox->GetSttNd();
        pUnoTblCrsr->GetMark()->nNode  = *pNewStartBox->GetSttNd();
        pUnoTblCrsr->Move( fnMoveForward, fnGoNode );
        pUnoTblCrsr->MakeBoxSels();
    }

    return bChanged;
}

SwChartLabeledDataSequence::SwChartLabeledDataSequence() :
    aEvtListeners( GetChartMutex() ),
    aModifyListeners( GetChartMutex() )
{
    bDisposed = sal_False;
}

SwChartLabeledDataSequence::~SwChartLabeledDataSequence()
{
}

uno::Reference< chart2::data::XDataSequence > SAL_CALL SwChartLabeledDataSequence::getValues(  )
    throw (uno::RuntimeException)
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

    
    uno::Reference< util::XModifyBroadcaster > xMB( rxDest, uno::UNO_QUERY );
    if (xMB.is())
        xMB->removeModifyListener( xML );
    uno::Reference< lang::XComponent > xC( rxDest, uno::UNO_QUERY );
    if (xC.is())
        xC->removeEventListener( xEL );

    rxDest = rxSource;

    
    xC = uno::Reference< lang::XComponent >( rxDest, uno::UNO_QUERY );
    if (xC.is())
        xC->addEventListener( xEL );
    xMB = uno::Reference< util::XModifyBroadcaster >( rxDest, uno::UNO_QUERY );
    if (xMB.is())
        xMB->addModifyListener( xML );
}

void SAL_CALL SwChartLabeledDataSequence::setValues(
        const uno::Reference< chart2::data::XDataSequence >& rxSequence )
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (bDisposed)
        throw lang::DisposedException();

    if (xData != rxSequence)
    {
        SetDataSequence( xData, rxSequence );
        
        LaunchModifiedEvent( aModifyListeners, dynamic_cast< XModifyBroadcaster * >(this) );
    }
}

uno::Reference< chart2::data::XDataSequence > SAL_CALL SwChartLabeledDataSequence::getLabel(  )
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (bDisposed)
        throw lang::DisposedException();
    return xLabels;
}

void SAL_CALL SwChartLabeledDataSequence::setLabel(
        const uno::Reference< chart2::data::XDataSequence >& rxSequence )
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (bDisposed)
        throw lang::DisposedException();

    if (xLabels != rxSequence)
    {
        SetDataSequence( xLabels, rxSequence );
        
        LaunchModifiedEvent( aModifyListeners, dynamic_cast< XModifyBroadcaster * >(this) );
    }
}

uno::Reference< util::XCloneable > SAL_CALL SwChartLabeledDataSequence::createClone(  )
    throw (uno::RuntimeException)
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
    throw (uno::RuntimeException)
{
    return OUString("SwChartLabeledDataSequence");
}

sal_Bool SAL_CALL SwChartLabeledDataSequence::supportsService(
        const OUString& rServiceName )
    throw (uno::RuntimeException)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL SwChartLabeledDataSequence::getSupportedServiceNames(  )
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Sequence< OUString > aRes(1);
    aRes.getArray()[0] = "com.sun.star.chart2.data.LabeledDataSequence";

    return aRes;
}

void SAL_CALL SwChartLabeledDataSequence::disposing(
        const lang::EventObject& rSource )
    throw (uno::RuntimeException)
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
    throw (uno::RuntimeException)
{
    if (rEvent.Source == xData || rEvent.Source == xLabels)
    {
        LaunchModifiedEvent( aModifyListeners, dynamic_cast< XModifyBroadcaster * >(this) );
    }
}

void SAL_CALL SwChartLabeledDataSequence::addModifyListener(
        const uno::Reference< util::XModifyListener >& rxListener )
    throw (uno::RuntimeException)
{
    osl::MutexGuard  aGuard( GetChartMutex() );
    if (!bDisposed && rxListener.is())
        aModifyListeners.addInterface( rxListener );
}

void SAL_CALL SwChartLabeledDataSequence::removeModifyListener(
        const uno::Reference< util::XModifyListener >& rxListener )
    throw (uno::RuntimeException)
{
    osl::MutexGuard  aGuard( GetChartMutex() );
    if (!bDisposed && rxListener.is())
        aModifyListeners.removeInterface( rxListener );
}

void SAL_CALL SwChartLabeledDataSequence::dispose(  )
    throw (uno::RuntimeException)
{
    bool bMustDispose( false );
    {
        osl::MutexGuard  aGuard( GetChartMutex() );
        bMustDispose = !bDisposed;
        if (!bDisposed)
            bDisposed = sal_True;
    }
    if (bMustDispose)
    {
        bDisposed = sal_True;

        
        lang::EventObject aEvtObj( dynamic_cast< chart2::data::XLabeledDataSequence * >(this) );
        aModifyListeners.disposeAndClear( aEvtObj );
        aEvtListeners.disposeAndClear( aEvtObj );
    }
}

void SAL_CALL SwChartLabeledDataSequence::addEventListener(
        const uno::Reference< lang::XEventListener >& rxListener )
    throw (uno::RuntimeException)
{
    osl::MutexGuard  aGuard( GetChartMutex() );
    if (!bDisposed && rxListener.is())
        aEvtListeners.addInterface( rxListener );
}

void SAL_CALL SwChartLabeledDataSequence::removeEventListener(
        const uno::Reference< lang::XEventListener >& rxListener )
    throw (uno::RuntimeException)
{
    osl::MutexGuard  aGuard( GetChartMutex() );
    if (!bDisposed && rxListener.is())
        aEvtListeners.removeInterface( rxListener );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
