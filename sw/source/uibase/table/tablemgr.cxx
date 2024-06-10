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
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/embed/EmbedVerbs.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>

#include <comphelper/classids.hxx>
#include <svx/charthelper.hxx>
#include <svtools/embedhlp.hxx>

#include <edtwin.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <swundo.hxx>
#include <tablemgr.hxx>
#include <frmfmt.hxx>
#include <swabstdlg.hxx>
#include <swcli.hxx>
#include <docsh.hxx>
#include <unotbl.hxx>
#include <unochart.hxx>

#include <comphelper/lok.hxx>

using namespace ::com::sun::star;

// Determine the width
SwTwips SwTableFUNC::GetColWidth(sal_uInt16 nNum) const
{
    SwTwips nWidth = 0;

    if( m_aCols.Count() > 0 )
    {
        if(m_aCols.Count() == GetColCount())
        {
            if(nNum == m_aCols.Count())
                nWidth = m_aCols.GetRight() - m_aCols[nNum-1];
            else
            {
                if(nNum == 0)
                    nWidth = m_aCols[nNum] - m_aCols.GetLeft();
                else
                    nWidth = m_aCols[nNum] - m_aCols[nNum-1];
            }
        }
        else
        {
            SwTwips nRValid = nNum < GetColCount() ?
                              m_aCols[GetRightSeparator(nNum)] :
                              m_aCols.GetRight();
            SwTwips nLValid = nNum ?
                              m_aCols[GetRightSeparator(nNum - 1)] :
                              m_aCols.GetLeft();
            nWidth = nRValid - nLValid;
        }
    }
    else
        nWidth = m_aCols.GetRight();
    return nWidth;
}

SwTwips SwTableFUNC::GetMaxColWidth( sal_uInt16 nNum ) const
{
    OSL_ENSURE(nNum <= m_aCols.Count(), "Index out of Area");

    if ( GetColCount() > 0 )
    {
        // The maximum width arises from the own width and
        // the width each of the neighbor cells reduced by MINLAY.
        SwTwips nMax;
        if(nNum == 0)
            nMax = GetColWidth(1) - MINLAY;
        else
        {
            nMax = GetColWidth(nNum-1);
            if(nNum == GetColCount())
                nMax -= MINLAY;
            else
                nMax += GetColWidth(nNum+1) - 2 * MINLAY;
        }
        return nMax + GetColWidth(nNum);
    }
    else
        return GetColWidth(nNum);
}

void SwTableFUNC::SetColWidth(sal_uInt16 nNum, SwTwips nNewWidth )
{
    // set current width
    // move all of the following
    bool bCurrentOnly = false;

    if ( m_aCols.Count() > 0 )
    {
        if(m_aCols.Count() != GetColCount())
            bCurrentOnly = true;
        SwTwips nWidth = GetColWidth(nNum);

        int nDiff = static_cast<int>(nNewWidth - nWidth);
        if( !nNum )
            m_aCols[ GetRightSeparator(0) ] += nDiff;
        else if( nNum < GetColCount()  )
        {
            if(nDiff < GetColWidth(nNum + 1) - MINLAY)
                m_aCols[ GetRightSeparator(nNum) ] += nDiff;
            else
            {
                int nDiffLeft = nDiff - static_cast<int>(GetColWidth(nNum + 1)) + int(MINLAY);
                m_aCols[ GetRightSeparator(nNum) ] += (nDiff - nDiffLeft);
                m_aCols[ GetRightSeparator(nNum - 1) ] -= nDiffLeft;
            }
        }
        else
            m_aCols[ GetRightSeparator(nNum-1) ] -= nDiff;
    }
    else
        m_aCols.SetRight( std::min( nNewWidth, SwTwips(m_aCols.GetRightMax()) ) );

    m_pSh->StartAllAction();
    m_pSh->SetTabCols( m_aCols, bCurrentOnly );
    m_pSh->EndAllAction();
}

void SwTableFUNC::InitTabCols()
{
    OSL_ENSURE(m_pSh, "no Shell");

    if( m_pFormat && m_pSh)
        m_pSh->GetTabCols( m_aCols );
}

SwTableFUNC::SwTableFUNC(SwWrtShell *pShell)
    : m_pFormat(pShell->GetTableFormat()),
      m_pSh(pShell)
{
}

SwTableFUNC::~SwTableFUNC()
{
}

void SwTableFUNC::UpdateChart()
{
    //Update of the fields triggered by the user, all Charts of
    //the table will be brought up to date
    SwFrameFormat *pFormat2 = m_pSh->GetTableFormat();
    if ( pFormat2 && m_pSh->HasOLEObj( pFormat2->GetName() ) )
    {
        m_pSh->StartAllAction();
        m_pSh->UpdateCharts( pFormat2->GetName() );
        m_pSh->EndAllAction();
    }
}

uno::Reference< frame::XModel > SwTableFUNC::InsertChart(
        uno::Reference< chart2::data::XDataProvider > const &rxDataProvider,
        bool bFillWithData,
        const OUString &rCellRange,
        SwFlyFrameFormat** ppFlyFrameFormat )
{
    uno::Reference< frame::XModel > xChartModel;
    m_pSh->StartUndo( SwUndoId::UI_INSERT_CHART );
    m_pSh->StartAllAction();

    OUString aName;
    if (m_pSh->IsCursorInTable())
    {
        aName = m_pSh->GetTableFormat()->GetName();
        // insert node before table
        m_pSh->MoveTable( GotoCurrTable, fnTableStart );
        m_pSh->Up( false );
        if ( m_pSh->IsCursorInTable() )
        {
            if ( aName != m_pSh->GetTableFormat()->GetName() )
                m_pSh->Down( false ); // two adjacent tables
        }
        m_pSh->SplitNode();
    }

    // insert chart
    OUString aObjName;
    comphelper::EmbeddedObjectContainer aCnt;
    uno::Reference < embed::XEmbeddedObject > xObj =
        aCnt.CreateEmbeddedObject( SvGlobalName( SO3_SCH_CLASSID ).GetByteSequence(), aObjName );

    ::svt::EmbeddedObjectRef aEmbObjRef( xObj, css::embed::Aspects::MSOLE_CONTENT );
    if ( xObj.is() )
    {

        SwFlyFrameFormat* pTmp = nullptr;
        m_pSh->InsertOleObject( aEmbObjRef, &pTmp );
        if (ppFlyFrameFormat)
            *ppFlyFrameFormat = pTmp;

        xChartModel.set( xObj->getComponent(), uno::UNO_QUERY );
        if( xChartModel.is() )
        {
            // Create a default chart type.
            uno::Reference<chart2::XChartDocument> xChartDoc(xChartModel, uno::UNO_QUERY);
            if (xChartDoc.is())
                xChartDoc->createDefaultChart();

            xChartModel->lockControllers(); //#i79578# don't request a new replacement image for charts to often - block change notifications
        }

        // set the table name at the OLE-node
        if (!aName.isEmpty())
            m_pSh->SetChartName( aName );
    }
    m_pSh->EndAllAction();

    if (xObj.is() && !comphelper::LibreOfficeKit::isActive())
    {
        // Let the chart be activated after the inserting (unless
        // via LibreOfficeKit)
        SfxInPlaceClient* pClient = m_pSh->GetView().FindIPClient( xObj, &m_pSh->GetView().GetEditWin() );
        if ( !pClient )
        {
            pClient = new SwOleClient( &m_pSh->GetView(), &m_pSh->GetView().GetEditWin(), aEmbObjRef );
            m_pSh->SetCheckForOLEInCaption( true );
        }
        m_pSh->CalcAndSetScale( aEmbObjRef );
        //#50270# We don't need to handle errors,
        //this does the DoVerb in the SfxViewShell.
        ErrCodeMsg nErr = pClient->DoVerb(embed::EmbedVerbs::MS_OLEVERB_SHOW);
        (void) nErr;

        // #i121334#
        ChartHelper::AdaptDefaultsForChart( xObj );
    }

    uno::Reference< chart2::data::XDataReceiver > xDataReceiver( xChartModel, uno::UNO_QUERY );
    if (bFillWithData && xDataReceiver.is() && rxDataProvider.is())
    {
        xDataReceiver->attachDataProvider( rxDataProvider );

        uno::Reference< util::XNumberFormatsSupplier > xNumberFormatsSupplier( m_pSh->GetView().GetDocShell()->GetModel(), uno::UNO_QUERY );
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

        uno::Sequence< beans::PropertyValue > aArgs{
            beans::PropertyValue(
                u"CellRangeRepresentation"_ustr, -1,
                uno::Any( rCellRange ), beans::PropertyState_DIRECT_VALUE ),
            beans::PropertyValue(
                u"HasCategories"_ustr, -1,
                uno::Any( bHasCategories ), beans::PropertyState_DIRECT_VALUE ),
            beans::PropertyValue(
                u"FirstCellAsLabel"_ustr, -1,
                uno::Any( bFirstCellAsLabel ), beans::PropertyState_DIRECT_VALUE ),
            beans::PropertyValue(
                u"DataRowSource"_ustr, -1,
                uno::Any( eDataRowSource ), beans::PropertyState_DIRECT_VALUE )
        };
        xDataReceiver->setArguments( aArgs );
    }

    m_pSh->EndUndo( SwUndoId::UI_INSERT_CHART );

    if( xChartModel.is() )
        xChartModel->unlockControllers(); //#i79578# don't request a new replacement image for charts to often
    return xChartModel;
}

sal_uInt16  SwTableFUNC::GetCurColNum() const
{
    const size_t nPos = m_pSh->GetCurTabColNum();
    size_t nCount = 0;
    for( size_t i = 0; i < nPos; i++ )
        if(m_aCols.IsHidden(i))
            nCount ++;
    return nPos - nCount;
}

sal_uInt16  SwTableFUNC::GetColCount() const
{
    size_t nCount = 0;
    for(size_t i = 0; i < m_aCols.Count(); i++ )
        if(m_aCols.IsHidden(i))
            nCount ++;
    return m_aCols.Count() - nCount;
}

int SwTableFUNC::GetRightSeparator(int nNum) const
{
    OSL_ENSURE( nNum < static_cast<int>(GetColCount()) ,"Index out of range");
    int i = 0;
    while( nNum >= 0 )
    {
        if( !m_aCols.IsHidden(i) )
            nNum--;
        i++;
    }
    return i - 1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
