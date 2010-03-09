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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#include "charthelper.hxx"
#include "document.hxx"
#include "drwlayer.hxx"
#include "rangelst.hxx"
#include "chartlis.hxx"

//#include <vcl/svapp.hxx>
#include <svx/svditer.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpage.hxx>

#include <com/sun/star/chart2/data/XDataReceiver.hpp>

using namespace com::sun::star;
using ::com::sun::star::uno::Reference;


// ====================================================================

namespace
{


USHORT lcl_DoUpdateCharts( const ScAddress& rPos, ScDocument* pDoc, BOOL bAllCharts )
{
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel)
        return 0;

    USHORT nFound = 0;

    USHORT nPageCount = pModel->GetPageCount();
    for (USHORT nPageNo=0; nPageNo<nPageCount; nPageNo++)
    {
        SdrPage* pPage = pModel->GetPage(nPageNo);
        DBG_ASSERT(pPage,"Page ?");

        SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
        SdrObject* pObject = aIter.Next();
        while (pObject)
        {
            if ( pObject->GetObjIdentifier() == OBJ_OLE2 && pDoc->IsChart( pObject ) )
            {
                String aName = ((SdrOle2Obj*)pObject)->GetPersistName();
                BOOL bHit = TRUE;
                if ( !bAllCharts )
                {
                    ScRangeList aRanges;
                    BOOL bColHeaders = FALSE;
                    BOOL bRowHeaders = FALSE;
                    pDoc->GetOldChartParameters( aName, aRanges, bColHeaders, bRowHeaders );
                    bHit = aRanges.In( rPos );
                }
                if ( bHit )
                {
                    pDoc->UpdateChart( aName );
                    ++nFound;
                }
            }
            pObject = aIter.Next();
        }
    }
    return nFound;
}

BOOL lcl_AdjustRanges( ScRangeList& rRanges, SCTAB nSourceTab, SCTAB nDestTab, SCTAB nTabCount )
{
    //! if multiple sheets are copied, update references into the other copied sheets?

    BOOL bChanged = FALSE;

    ULONG nCount = rRanges.Count();
    for (ULONG i=0; i<nCount; i++)
    {
        ScRange* pRange = rRanges.GetObject(i);
        if ( pRange->aStart.Tab() == nSourceTab && pRange->aEnd.Tab() == nSourceTab )
        {
            pRange->aStart.SetTab( nDestTab );
            pRange->aEnd.SetTab( nDestTab );
            bChanged = TRUE;
        }
        if ( pRange->aStart.Tab() >= nTabCount )
        {
            pRange->aStart.SetTab( nTabCount > 0 ? ( nTabCount - 1 ) : 0 );
            bChanged = TRUE;
        }
        if ( pRange->aEnd.Tab() >= nTabCount )
        {
            pRange->aEnd.SetTab( nTabCount > 0 ? ( nTabCount - 1 ) : 0 );
            bChanged = TRUE;
        }
    }

    return bChanged;
}

}//end anonymous namespace

// === ScChartHelper ======================================

//static
USHORT ScChartHelper::DoUpdateCharts( const ScAddress& rPos, ScDocument* pDoc )
{
    return lcl_DoUpdateCharts( rPos, pDoc, FALSE );
}

//static
USHORT ScChartHelper::DoUpdateAllCharts( ScDocument* pDoc )
{
    return lcl_DoUpdateCharts( ScAddress(), pDoc, TRUE );
}

//static
void ScChartHelper::AdjustRangesOfChartsOnDestinationPage( ScDocument* pSrcDoc, ScDocument* pDestDoc, const SCTAB nSrcTab, const SCTAB nDestTab )
{
    if( !pSrcDoc || !pDestDoc )
        return;
    ScDrawLayer* pDrawLayer = pDestDoc->GetDrawLayer();
    if( !pDrawLayer )
        return;

    SdrPage* pDestPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nDestTab));
    if( pDestPage )
    {
        SdrObjListIter aIter( *pDestPage, IM_FLAT );
        SdrObject* pObject = aIter.Next();
        while( pObject )
        {
            if( pObject->GetObjIdentifier() == OBJ_OLE2 && ((SdrOle2Obj*)pObject)->IsChart() )
            {
                String aChartName = ((SdrOle2Obj*)pObject)->GetPersistName();

                Reference< chart2::XChartDocument > xChartDoc( pDestDoc->GetChartByName( aChartName ) );
                Reference< chart2::data::XDataReceiver > xReceiver( xChartDoc, uno::UNO_QUERY );
                if( xChartDoc.is() && xReceiver.is() && !xChartDoc->hasInternalDataProvider() )
                {
                    ::std::vector< ScRangeList > aRangesVector;
                    pDestDoc->GetChartRanges( aChartName, aRangesVector, pSrcDoc );

                    ::std::vector< ScRangeList >::iterator aIt( aRangesVector.begin() );
                    for( ; aIt!=aRangesVector.end(); aIt++ )
                    {
                        ScRangeList& rScRangeList( *aIt );
                        lcl_AdjustRanges( rScRangeList, nSrcTab, nDestTab, pDestDoc->GetTableCount() );
                    }
                    pDestDoc->SetChartRanges( aChartName, aRangesVector );
                }
            }
            pObject = aIter.Next();
        }
    }
}
