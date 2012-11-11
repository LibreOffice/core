/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

bool SwTable::IsTblComplexForChart( const String& rSelection ) const
{
    const SwTableBox* pSttBox, *pEndBox;
    if( 2 < rSelection.Len() )
    {
        // Remove brackets at the beginning and from the end
        String sBox( rSelection );
        if( '<' == sBox.GetChar( 0  ) ) sBox.Erase( 0, 1 );
        if( '>' == sBox.GetChar( sBox.Len()-1  ) ) sBox.Erase( sBox.Len()-1 );

        xub_StrLen nSeperator = sBox.Search( ':' );
        OSL_ENSURE( STRING_NOTFOUND != nSeperator, "no valid selection" );

        pSttBox = GetTblBox( sBox.Copy( 0, nSeperator ));
        pEndBox = GetTblBox( sBox.Copy( nSeperator+1 ));
    }
    else
    {
        const SwTableLines* pLns = &GetTabLines();
        pSttBox = (*pLns)[ 0 ]->GetTabBoxes().front();
        while( !pSttBox->GetSttNd() )
            // Until the Content Box!
            pSttBox = pSttBox->GetTabLines().front()->GetTabBoxes().front();

        const SwTableBoxes* pBoxes = &pLns->back()->GetTabBoxes();
        pEndBox = pBoxes->back();
        while( !pEndBox->GetSttNd() )
        {
            // Until the Content Box!
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
    ViewShell* pVSh;
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

void SwDoc::_UpdateCharts( const SwTable& rTbl, ViewShell& rVSh ) const
{
    String aName( rTbl.GetFrmFmt()->GetName() );
    SwOLENode *pONd;
    SwStartNode *pStNd;
    SwNodeIndex aIdx( *GetNodes().GetEndOfAutotext().StartOfSectionNode(), 1 );
    while( 0 != (pStNd = aIdx.GetNode().GetStartNode()) )
    {
        ++aIdx;
        if( 0 != ( pONd = aIdx.GetNode().GetOLENode() ) &&
            aName.Equals( pONd->GetChartTblName() ) &&
            pONd->getLayoutFrm( rVSh.GetLayout() ) )
        {
            SwChartDataProvider *pPCD = GetChartDataProvider();
            if (pPCD)
                pPCD->InvalidateTable( &rTbl );
            // following this the framework will now take care of repainting
            // the chart or it's replacement image...
        }
        aIdx.Assign( *pStNd->EndOfSectionNode(), + 1 );
    }
}

void SwDoc::UpdateCharts( const String &rName ) const
{
    SwTable* pTmpTbl = SwTable::FindTable( FindTblFmtByName( rName ) );
    if( pTmpTbl )
    {
        ViewShell* pVSh;
        GetEditShell( &pVSh );

        if( pVSh )
            _UpdateCharts( *pTmpTbl, *pVSh );
    }
}

void SwDoc::SetTableName( SwFrmFmt& rTblFmt, const String &rNewName )
{
    const String aOldName( rTblFmt.GetName() );

    bool bNameFound = 0 == rNewName.Len();
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

            ViewShell* pVSh;
            GetEditShell( &pVSh );

            SwTable* pTable = SwTable::FindTable( &rTblFmt );
            SwChartDataProvider *pPCD = GetChartDataProvider();
            if (pPCD)
                pPCD->InvalidateTable( pTable );
            // following this the framework will now take care of repainting
            // the chart or it's replacement image...
        }
        aIdx.Assign( *pStNd->EndOfSectionNode(), + 1 );
    }
    SetModified();
}


SwChartDataProvider * SwDoc::GetChartDataProvider( bool bCreate ) const
{
    // since there must be only one instance of this object per document
    // we need a mutex here
    SolarMutexGuard aGuard;

    if (bCreate && !aChartDataProviderImplRef.get())
    {
        aChartDataProviderImplRef = comphelper::ImplementationReference< SwChartDataProvider
            , chart2::data::XDataProvider >( new SwChartDataProvider( this ) );
    }
    return aChartDataProviderImplRef.get();
}


void SwDoc::CreateChartInternalDataProviders( const SwTable *pTable )
{
    if (pTable)
    {
        String aName( pTable->GetFrmFmt()->GetName() );
        SwOLENode *pONd;
        SwStartNode *pStNd;
        SwNodeIndex aIdx( *GetNodes().GetEndOfAutotext().StartOfSectionNode(), 1 );
        while (0 != (pStNd = aIdx.GetNode().GetStartNode()))
        {
            ++aIdx;
            if( 0 != ( pONd = aIdx.GetNode().GetOLENode() ) &&
                aName.Equals( pONd->GetChartTblName() ) /* OLE node is chart? */ &&
                0 != (pONd->getLayoutFrm( GetCurrentLayout() )) /* chart frame is not hidden */ )
            {
                uno::Reference < embed::XEmbeddedObject > xIP = pONd->GetOLEObj().GetOleRef();
                if ( svt::EmbeddedObjectRef::TryRunningState( xIP ) )
                {
                    uno::Reference< chart2::XChartDocument > xChart( xIP->getComponent(), UNO_QUERY );
                    if (xChart.is())
                        xChart->createInternalDataProvider( sal_True );

                    // there may be more than one chart for each table thus we need to continue the loop...
                }
            }
            aIdx.Assign( *pStNd->EndOfSectionNode(), + 1 );
        }
    }
}


SwChartLockController_Helper & SwDoc::GetChartControllerHelper()
{
    if (!pChartControllerHelper)
    {
        pChartControllerHelper = new SwChartLockController_Helper( this );
    }
    return *pChartControllerHelper;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
