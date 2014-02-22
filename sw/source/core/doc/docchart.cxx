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

#include <com/sun/star/frame/XModel.hpp>

#include <com/sun/star/chart2/XChartDocument.hpp>

#include <float.h>
#include <hintids.hxx>
#include <vcl/window.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <ndindex.hxx>
#include <swtable.hxx>
#include <ndtxt.hxx>
#include <calc.hxx>
#include <frmfmt.hxx>
#include <cellfml.hxx>
#include <viewsh.hxx>
#include <ndole.hxx>
#include <calbck.hxx>
#include <cntfrm.hxx>
#include <swtblfmt.hxx>
#include <tblsel.hxx>
#include <cellatr.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>

#include <unochart.hxx>

using namespace com::sun::star;
using namespace com::sun::star::uno;

void SwTable::UpdateCharts() const
{
    GetFrmFmt()->GetDoc()->UpdateCharts( GetFrmFmt()->GetName() );
}

bool SwTable::IsTblComplexForChart( const OUString& rSelection ) const
{
    const SwTableBox* pSttBox, *pEndBox;
    if( 2 < rSelection.getLength() )
    {
        
        OUString sBox( rSelection );
        if( '<' == sBox[0] ) sBox = sBox.copy( 0, 1 );
        if( '>' == sBox[ sBox.getLength()-1  ] ) sBox = sBox.copy( 0, sBox.getLength()-1 );

        sal_Int32 nSeparator = sBox.indexOf( ':' );
        OSL_ENSURE( -1 != nSeparator, "no valid selection" );

        pSttBox = GetTblBox( sBox.copy( 0, nSeparator ));
        pEndBox = GetTblBox( sBox.copy( nSeparator+1 ));
    }
    else
    {
        const SwTableLines* pLns = &GetTabLines();
        pSttBox = (*pLns)[ 0 ]->GetTabBoxes().front();
        while( !pSttBox->GetSttNd() )
            
            pSttBox = pSttBox->GetTabLines().front()->GetTabBoxes().front();

        const SwTableBoxes* pBoxes = &pLns->back()->GetTabBoxes();
        pEndBox = pBoxes->back();
        while( !pEndBox->GetSttNd() )
        {
            
            pLns = &pEndBox->GetTabLines();
            pBoxes = &pLns->back()->GetTabBoxes();
            pEndBox = pBoxes->back();
        }
    }

    return !pSttBox || !pEndBox || !::ChkChartSel( *pSttBox->GetSttNd(),
                                        *pEndBox->GetSttNd() );
}

void SwDoc::DoUpdateAllCharts()
{
    SwViewShell* pVSh;
    GetEditShell( &pVSh );
    if( pVSh )
    {
        const SwFrmFmts& rTblFmts = *GetTblFrmFmts();
        for( sal_uInt16 n = 0; n < rTblFmts.size(); ++n )
        {
            SwTable* pTmpTbl;
            const SwTableNode* pTblNd;
            SwFrmFmt* pFmt = rTblFmts[ n ];

            if( 0 != ( pTmpTbl = SwTable::FindTable( pFmt ) ) &&
                0 != ( pTblNd = pTmpTbl->GetTableNode() ) &&
                pTblNd->GetNodes().IsDocNodes() )
            {
                _UpdateCharts( *pTmpTbl, *pVSh );
            }
        }
    }
}

void SwDoc::_UpdateCharts( const SwTable& rTbl, SwViewShell& rVSh ) const
{
    OUString aName( rTbl.GetFrmFmt()->GetName() );
    SwOLENode *pONd;
    SwStartNode *pStNd;
    SwNodeIndex aIdx( *GetNodes().GetEndOfAutotext().StartOfSectionNode(), 1 );
    while( 0 != (pStNd = aIdx.GetNode().GetStartNode()) )
    {
        ++aIdx;
        if( 0 != ( pONd = aIdx.GetNode().GetOLENode() ) &&
            aName == pONd->GetChartTblName() &&
            pONd->getLayoutFrm( rVSh.GetLayout() ) )
        {
            SwChartDataProvider *pPCD = GetChartDataProvider();
            if (pPCD)
                pPCD->InvalidateTable( &rTbl );
            
            
        }
        aIdx.Assign( *pStNd->EndOfSectionNode(), + 1 );
    }
}

void SwDoc::UpdateCharts( const OUString &rName ) const
{
    SwTable* pTmpTbl = SwTable::FindTable( FindTblFmtByName( rName ) );
    if( pTmpTbl )
    {
        SwViewShell* pVSh;
        GetEditShell( &pVSh );

        if( pVSh )
            _UpdateCharts( *pTmpTbl, *pVSh );
    }
}

void SwDoc::SetTableName( SwFrmFmt& rTblFmt, const OUString &rNewName )
{
    const OUString aOldName( rTblFmt.GetName() );

    bool bNameFound = rNewName.isEmpty();
    if( !bNameFound )
    {
        SwFrmFmt* pFmt;
        const SwFrmFmts& rTbl = *GetTblFrmFmts();
        for( sal_uInt16 i = rTbl.size(); i; )
            if( !( pFmt = rTbl[ --i ] )->IsDefault() &&
                pFmt->GetName() == rNewName && IsUsed( *pFmt ) )
            {
                bNameFound = true;
                break;
            }
    }

    if( !bNameFound )
        rTblFmt.SetName( rNewName, sal_True );
    else
        rTblFmt.SetName( GetUniqueTblName(), sal_True );

    SwStartNode *pStNd;
    SwNodeIndex aIdx( *GetNodes().GetEndOfAutotext().StartOfSectionNode(), 1 );
    while ( 0 != (pStNd = aIdx.GetNode().GetStartNode()) )
    {
        ++aIdx;
        SwOLENode *pNd = aIdx.GetNode().GetOLENode();
        if( pNd && aOldName == pNd->GetChartTblName() )
        {
            pNd->SetChartTblName( rNewName );

            SwViewShell* pVSh;
            GetEditShell( &pVSh );

            SwTable* pTable = SwTable::FindTable( &rTblFmt );
            SwChartDataProvider *pPCD = GetChartDataProvider();
            if (pPCD)
                pPCD->InvalidateTable( pTable );
            
            
        }
        aIdx.Assign( *pStNd->EndOfSectionNode(), + 1 );
    }
    SetModified();
}

SwChartDataProvider * SwDoc::GetChartDataProvider( bool bCreate ) const
{
    
    
    SolarMutexGuard aGuard;

    if (bCreate && !maChartDataProviderImplRef.get())
    {
        maChartDataProviderImplRef = comphelper::ImplementationReference< SwChartDataProvider
            , chart2::data::XDataProvider >( new SwChartDataProvider( this ) );
    }
    return maChartDataProviderImplRef.get();
}

void SwDoc::CreateChartInternalDataProviders( const SwTable *pTable )
{
    if (pTable)
    {
        OUString aName( pTable->GetFrmFmt()->GetName() );
        SwOLENode *pONd;
        SwStartNode *pStNd;
        SwNodeIndex aIdx( *GetNodes().GetEndOfAutotext().StartOfSectionNode(), 1 );
        while (0 != (pStNd = aIdx.GetNode().GetStartNode()))
        {
            ++aIdx;
            if( 0 != ( pONd = aIdx.GetNode().GetOLENode() ) &&
                aName == pONd->GetChartTblName() /* OLE node is chart? */ &&
                0 != (pONd->getLayoutFrm( GetCurrentLayout() )) /* chart frame is not hidden */ )
            {
                uno::Reference < embed::XEmbeddedObject > xIP = pONd->GetOLEObj().GetOleRef();
                if ( svt::EmbeddedObjectRef::TryRunningState( xIP ) )
                {
                    uno::Reference< chart2::XChartDocument > xChart( xIP->getComponent(), UNO_QUERY );
                    if (xChart.is())
                        xChart->createInternalDataProvider( sal_True );

                    
                }
            }
            aIdx.Assign( *pStNd->EndOfSectionNode(), + 1 );
        }
    }
}

SwChartLockController_Helper & SwDoc::GetChartControllerHelper()
{
    if (!mpChartControllerHelper)
    {
        mpChartControllerHelper = new SwChartLockController_Helper( this );
    }
    return *mpChartControllerHelper;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
