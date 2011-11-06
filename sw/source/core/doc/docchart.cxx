/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


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
#include <ndole.hxx>
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
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>

#include <unochart.hxx>

using namespace com::sun::star;
using namespace com::sun::star::uno;


void SwTable::UpdateCharts() const
{
    GetFrmFmt()->GetDoc()->UpdateCharts( GetFrmFmt()->GetName() );
}

sal_Bool SwTable::IsTblComplexForChart( const String& rSelection,
                                    SwChartLines* pGetCLines ) const
{
    const SwTableBox* pSttBox, *pEndBox;
    if( 2 < rSelection.Len() )
    {
        // spitze Klammern am Anfang & Ende enfernen
        String sBox( rSelection );
        if( '<' == sBox.GetChar( 0  ) ) sBox.Erase( 0, 1 );
        if( '>' == sBox.GetChar( sBox.Len()-1  ) ) sBox.Erase( sBox.Len()-1 );

        xub_StrLen nTrenner = sBox.Search( ':' );
        ASSERT( STRING_NOTFOUND != nTrenner, "keine gueltige Selektion" );

        pSttBox = GetTblBox( sBox.Copy( 0, nTrenner ));
        pEndBox = GetTblBox( sBox.Copy( nTrenner+1 ));
    }
    else
    {
        const SwTableLines* pLns = &GetTabLines();
        pSttBox = (*pLns)[ 0 ]->GetTabBoxes()[ 0 ];
        while( !pSttBox->GetSttNd() )
            // bis zur Content Box!
            pSttBox = pSttBox->GetTabLines()[ 0 ]->GetTabBoxes()[ 0 ];

        const SwTableBoxes* pBoxes = &(*pLns)[ pLns->Count()-1 ]->GetTabBoxes();
        pEndBox = (*pBoxes)[ pBoxes->Count()-1 ];
        while( !pEndBox->GetSttNd() )
        {
            // bis zur Content Box!
            pLns = &pEndBox->GetTabLines();
            pBoxes = &(*pLns)[ pLns->Count()-1 ]->GetTabBoxes();
            pEndBox = (*pBoxes)[ pBoxes->Count()-1 ];
        }
    }

    return !pSttBox || !pEndBox || !::ChkChartSel( *pSttBox->GetSttNd(),
                                        *pEndBox->GetSttNd(), pGetCLines );
}



IMPL_LINK( SwDoc, DoUpdateAllCharts, Timer *, EMPTYARG )
{
    ViewShell* pVSh;
    GetEditShell( &pVSh );
    if( pVSh )
    {
        const SwFrmFmts& rTblFmts = *GetTblFrmFmts();
        for( sal_uInt16 n = 0; n < rTblFmts.Count(); ++n )
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
    return 0;
}

void SwDoc::_UpdateCharts( const SwTable& rTbl, ViewShell& rVSh ) const
{
    String aName( rTbl.GetFrmFmt()->GetName() );
    SwOLENode *pONd;
    SwStartNode *pStNd;
    SwNodeIndex aIdx( *GetNodes().GetEndOfAutotext().StartOfSectionNode(), 1 );
    while( 0 != (pStNd = aIdx.GetNode().GetStartNode()) )
    {
        aIdx++;
        SwFrm* pFrm;
        if( 0 != ( pONd = aIdx.GetNode().GetOLENode() ) &&
            aName.Equals( pONd->GetChartTblName() ) &&
            0 != ( pFrm = pONd->getLayoutFrm( rVSh.GetLayout() ) ) )
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
//  sal_Bool bStop = 1;

    const String aOldName( rTblFmt.GetName() );

    sal_Bool bNameFound = 0 == rNewName.Len();
    if( !bNameFound )
    {
        SwFrmFmt* pFmt;
        const SwFrmFmts& rTbl = *GetTblFrmFmts();
        for( sal_uInt16 i = rTbl.Count(); i; )
            if( !( pFmt = rTbl[ --i ] )->IsDefault() &&
                pFmt->GetName() == rNewName && IsUsed( *pFmt ) )
            {
                bNameFound = sal_True;
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
        aIdx++;
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
    vos::OGuard aGuard( Application::GetSolarMutex() );

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
            aIdx++;
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

