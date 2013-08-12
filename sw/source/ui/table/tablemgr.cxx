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


#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include <com/sun/star/beans/PropertyState.hpp>

#include <sot/storage.hxx>
#include <comphelper/classids.hxx>
#include <svx/charthelper.hxx>

#include "edtwin.hxx"
#include "wrtsh.hxx"
#include "cmdid.h"
#include "frmatr.hxx"
#include "view.hxx"
#include "basesh.hxx"
#include "swundo.hxx"
#include "tablemgr.hxx"
#include "frmfmt.hxx"
#include "instable.hxx"
#include "swerror.h"
#include "table.hrc"
#include "swabstdlg.hxx"
#include "swcli.hxx"
#include "docsh.hxx"
#include "unotbl.hxx"
#include "unochart.hxx"

using namespace ::com::sun::star;

// Adjust line height (dialogue)

void SwTableFUNC::ColWidthDlg( Window *pParent )
{
    InitTabCols();
    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

    VclAbstractDialog* pDlg = pFact->CreateSwTableWidthDlg(pParent, *this);
    OSL_ENSURE(pDlg, "Dialogdiet fail!");
    pDlg->Execute();
    delete pDlg;
}

// Determine the width


SwTwips SwTableFUNC::GetColWidth(sal_uInt16 nNum) const
{
    SwTwips nWidth = 0;

    if( aCols.Count() > 0 )
    {
        if(aCols.Count() == GetColCount())
        {
            nWidth = (SwTwips)((nNum == aCols.Count()) ?
                    aCols.GetRight() - aCols[nNum-1] :
                    nNum == 0 ? aCols[nNum] - aCols.GetLeft() :
                                aCols[nNum] - aCols[nNum-1]);
        }
        else
        {
            SwTwips nRValid = nNum < GetColCount() ?
                            aCols[(sal_uInt16)GetRightSeparator((int)nNum)]:
                                    aCols.GetRight();
            SwTwips nLValid = nNum ?
                            aCols[(sal_uInt16)GetRightSeparator((int)nNum - 1)]:
                                    aCols.GetLeft();
            nWidth = nRValid - nLValid;
        }
    }
    else
        nWidth = aCols.GetRight();

    return nWidth;
}



SwTwips SwTableFUNC::GetMaxColWidth( sal_uInt16 nNum ) const
{
    OSL_ENSURE(nNum <= aCols.Count(), "Index out of Area");

    if ( GetColCount() > 0 )
    {
        // The maximum width arises from the own width and
        // the width each of the neighbor cells reduced by MINLAY.
        SwTwips nMax =  nNum == 0 ?
            GetColWidth(1) - MINLAY :
                nNum == GetColCount() ?
                    GetColWidth( nNum-1 ) - MINLAY :
                        GetColWidth(nNum - 1) + GetColWidth( nNum + 1 ) - 2 * MINLAY;

        return nMax + GetColWidth(nNum) ;
    }
    else
        return GetColWidth(nNum);
}



void SwTableFUNC::SetColWidth(sal_uInt16 nNum, SwTwips nNewWidth )
{
    // set current width
    // move all of the following
    sal_Bool bCurrentOnly = sal_False;

    if ( aCols.Count() > 0 )
    {
        if(aCols.Count() != GetColCount())
            bCurrentOnly = sal_True;
        SwTwips nWidth = GetColWidth(nNum);

        int nDiff = (int)(nNewWidth - nWidth);
        if( !nNum )
            aCols[ static_cast< sal_uInt16 >(GetRightSeparator(0)) ] += nDiff;
        else if( nNum < GetColCount()  )
        {
            if(nDiff < GetColWidth(nNum + 1) - MINLAY)
                aCols[ static_cast< sal_uInt16 >(GetRightSeparator(nNum)) ] += nDiff;
            else
            {
                int nDiffLeft = nDiff - (int)GetColWidth(nNum + 1) + (int)MINLAY;
                aCols[ static_cast< sal_uInt16 >(GetRightSeparator(nNum)) ] += (nDiff - nDiffLeft);
                aCols[ static_cast< sal_uInt16 >(GetRightSeparator(nNum - 1)) ] -= nDiffLeft;
            }
        }
        else
            aCols[ static_cast< sal_uInt16 >(GetRightSeparator(nNum-1)) ] -= nDiff;
    }
    else
        aCols.SetRight( std::min( nNewWidth, aCols.GetRightMax()) );

    pSh->StartAllAction();
    pSh->SetTabCols( aCols, bCurrentOnly );
    pSh->EndAllAction();
}



void SwTableFUNC::InitTabCols()
{
    OSL_ENSURE(pSh, "no Shell");

    if( pFmt && pSh)
        pSh->GetTabCols( aCols );
}



SwTableFUNC::SwTableFUNC(SwWrtShell *pShell, sal_Bool bCopyFmt)
    : pFmt(pShell->GetTableFmt()),
      pSh(pShell),
      bCopy(bCopyFmt)
{
        // if applicable copy the format for edit
    if( pFmt && bCopy )
        pFmt = new SwFrmFmt( *pFmt );
}



SwTableFUNC::~SwTableFUNC()
{
    if(bCopy)
        delete pFmt;
}

void SwTableFUNC::UpdateChart()
{
    //Update of the fields triggered by the user, all Charts of
    //the table will be brought up to date
    SwFrmFmt *pFmt2 = pSh->GetTableFmt();
    if ( pFmt2 && pSh->HasOLEObj( pFmt2->GetName() ) )
    {
        pSh->StartAllAction();
        pSh->UpdateCharts( pFmt2->GetName() );
        pSh->EndAllAction();
    }
}

uno::Reference< frame::XModel > SwTableFUNC::InsertChart(
        uno::Reference< chart2::data::XDataProvider > &rxDataProvider,
        sal_Bool bFillWithData,
        const OUString &rCellRange,
        SwFlyFrmFmt** ppFlyFrmFmt )
{
    uno::Reference< frame::XModel > xChartModel;
    pSh->StartUndo( UNDO_UI_INSERT_CHART );
    pSh->StartAllAction();

    OUString aName;
    if (pSh->IsCrsrInTbl())
    {
        aName = pSh->GetTableFmt()->GetName();
        // insert node before table
        pSh->MoveTable( fnTableCurr, fnTableStart );
        pSh->Up( sal_False, 1, sal_False );
        if ( pSh->IsCrsrInTbl() )
        {
            if ( aName != pSh->GetTableFmt()->GetName() )
                pSh->Down( sal_False, 1, sal_False ); // two adjacent tables
        }
        pSh->SplitNode();
    }

    // insert chart
    OUString aObjName;
    comphelper::EmbeddedObjectContainer aCnt;
    uno::Reference < embed::XEmbeddedObject > xObj =
        aCnt.CreateEmbeddedObject( SvGlobalName( SO3_SCH_CLASSID ).GetByteSequence(), aObjName );

    ::svt::EmbeddedObjectRef aEmbObjRef( xObj, ::com::sun::star::embed::Aspects::MSOLE_CONTENT );
    if ( xObj.is() )
    {

        SwFlyFrmFmt* pTmp = 0;
        pSh->InsertOleObject( aEmbObjRef, &pTmp );
        if (ppFlyFrmFmt)
            *ppFlyFrmFmt = pTmp;

        uno::Reference< embed::XComponentSupplier > xCompSupp( xObj, uno::UNO_QUERY );
        if( xCompSupp.is())
        {
            xChartModel.set( xCompSupp->getComponent(), uno::UNO_QUERY );
            if( xChartModel.is() )
                xChartModel->lockControllers(); //#i79578# don't request a new replacement image for charts to often - block change notifications
        }

        // set the table name at the OLE-node
        if (!aName.isEmpty())
            pSh->SetChartName( aName );
    }
    pSh->EndAllAction();

    if ( xObj.is() )
    {
        // Let the chart be activated after the inserting
        SfxInPlaceClient* pClient = pSh->GetView().FindIPClient( xObj, &pSh->GetView().GetEditWin() );
        if ( !pClient )
        {
            pClient = new SwOleClient( &pSh->GetView(), &pSh->GetView().GetEditWin(), aEmbObjRef );
            pSh->SetCheckForOLEInCaption( sal_True );
        }
        pSh->CalcAndSetScale( aEmbObjRef );
        //#50270# We don't need to handle errors,
        //this does the DoVerb in the SfxViewShell.
        ErrCode nErr = pClient->DoVerb( SVVERB_SHOW );
        (void) nErr;

        // #i121334#
        ChartHelper::AdaptDefaultsForChart( xObj );
    }

    uno::Reference< chart2::data::XDataReceiver > xDataReceiver( xChartModel, uno::UNO_QUERY );
    if (bFillWithData && xDataReceiver.is() && rxDataProvider.is())
    {
        xDataReceiver->attachDataProvider( rxDataProvider );

        uno::Reference< util::XNumberFormatsSupplier > xNumberFormatsSupplier( pSh->GetView().GetDocShell()->GetModel(), uno::UNO_QUERY );
        xDataReceiver->attachNumberFormatsSupplier( xNumberFormatsSupplier );

        // default values for ranges that do not consist of a single row or column
        bool bHasCategories = true;
        bool bFirstCellAsLabel = true;
        chart::ChartDataRowSource eDataRowSource = chart::ChartDataRowSource_COLUMNS;

        SwRangeDescriptor aDesc;
        FillRangeDescriptor( aDesc, rCellRange );
        bool bSingleRowCol = aDesc.nTop == aDesc.nBottom || aDesc.nLeft == aDesc.nRight;
        if (bSingleRowCol)
        {
            aDesc.Normalize();
            sal_Int32 nRowLen = aDesc.nRight  - aDesc.nLeft + 1;
            sal_Int32 nColLen = aDesc.nBottom - aDesc.nTop + 1;

            bHasCategories = false;
            if (nRowLen == 1 && nColLen == 1)
                bFirstCellAsLabel   = false;
            else if (nRowLen > 1)
                eDataRowSource = chart::ChartDataRowSource_ROWS;
            else if (nColLen > 1)
                eDataRowSource = chart::ChartDataRowSource_COLUMNS;
            else {
                OSL_FAIL("unexpected state" );
            }
        }

        uno::Sequence< beans::PropertyValue > aArgs( 4 );
        aArgs[0] = beans::PropertyValue(
            OUString("CellRangeRepresentation"), -1,
            uno::makeAny( rCellRange ), beans::PropertyState_DIRECT_VALUE );
        aArgs[1] = beans::PropertyValue(
            OUString("HasCategories"), -1,
            uno::makeAny( bHasCategories ), beans::PropertyState_DIRECT_VALUE );
        aArgs[2] = beans::PropertyValue(
            OUString("FirstCellAsLabel"), -1,
            uno::makeAny( bFirstCellAsLabel ), beans::PropertyState_DIRECT_VALUE );
        aArgs[3] = beans::PropertyValue(
            OUString("DataRowSource"), -1,
            uno::makeAny( eDataRowSource ), beans::PropertyState_DIRECT_VALUE );
        xDataReceiver->setArguments( aArgs );
    }

    pSh->EndUndo( UNDO_UI_INSERT_CHART );

    if( xChartModel.is() )
        xChartModel->unlockControllers(); //#i79578# don't request a new replacement image for charts to often
    return xChartModel;
}

sal_uInt16  SwTableFUNC::GetCurColNum() const
{
    sal_uInt16 nPos = pSh->GetCurTabColNum();
    sal_uInt16 nCount = 0;
    for(sal_uInt16 i = 0; i < nPos; i++ )
        if(aCols.IsHidden(i))
            nCount ++;
    return nPos - nCount;
}




sal_uInt16  SwTableFUNC::GetColCount() const
{
    sal_uInt16 nCount = 0;
    for(sal_uInt16 i = 0; i < aCols.Count(); i++ )
        if(aCols.IsHidden(i))
            nCount ++;
    return aCols.Count() - nCount;
}



int SwTableFUNC::GetRightSeparator(int nNum) const
{
    OSL_ENSURE( nNum < (int)GetColCount() ,"Index out of range");
    int i = 0;
    while( nNum >= 0 )
    {
        if( !aCols.IsHidden( static_cast< sal_uInt16 >(i)) )
            nNum--;
        i++;
    }
    return i - 1;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
