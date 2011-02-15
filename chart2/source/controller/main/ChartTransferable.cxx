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
#include "precompiled_chart2.hxx"

#include "ChartTransferable.hxx"

#include <unotools/streamwrap.hxx>
#include <vcl/graph.hxx>
#include <svl/itempool.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <svx/svditer.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/unomodel.hxx>

// header for class SdrView
#include <svx/svdview.hxx>

#define CHARTTRANSFER_OBJECTTYPE_DRAWMODEL      1

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

namespace chart
{

ChartTransferable::ChartTransferable( SdrModel* pDrawModel, SdrObject* pSelectedObj, bool bDrawing )
    :m_pMarkedObjModel( NULL )
    ,m_bDrawing( bDrawing )
{
    SdrExchangeView * pExchgView( new SdrView( pDrawModel ));
    SdrPageView* pPv = pExchgView->ShowSdrPage( pDrawModel->GetPage( 0 ));
    if( pSelectedObj )
        pExchgView->MarkObj( pSelectedObj, pPv );
    else
        pExchgView->MarkAllObj( pPv );
    Graphic aGraphic( pExchgView->GetMarkedObjMetaFile( sal_True ));
    m_xMetaFileGraphic.set( aGraphic.GetXGraphic());
    if ( m_bDrawing )
    {
        m_pMarkedObjModel = ( pExchgView ? pExchgView->GetAllMarkedModel() : NULL );
    }
    delete pExchgView;
}

ChartTransferable::~ChartTransferable()
{}

void ChartTransferable::AddSupportedFormats()
{
    if ( m_bDrawing )
    {
        AddFormat( SOT_FORMATSTR_ID_DRAWING );
    }
    AddFormat( SOT_FORMAT_GDIMETAFILE );
    AddFormat( SOT_FORMAT_BITMAP );
}

sal_Bool ChartTransferable::GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor )
{
    sal_uInt32  nFormat = SotExchange::GetFormat( rFlavor );
    sal_Bool    bResult = sal_False;

    if( HasFormat( nFormat ))
    {
        if ( nFormat == SOT_FORMATSTR_ID_DRAWING )
        {
            bResult = SetObject( m_pMarkedObjModel, CHARTTRANSFER_OBJECTTYPE_DRAWMODEL, rFlavor );
        }
        else if ( nFormat == FORMAT_GDIMETAFILE )
        {
            Graphic aGraphic( m_xMetaFileGraphic );
            bResult = SetGDIMetaFile( aGraphic.GetGDIMetaFile(), rFlavor );
        }
        else if( nFormat == FORMAT_BITMAP )
        {
            Graphic aGraphic( m_xMetaFileGraphic );
            bResult = SetBitmap( aGraphic.GetBitmap(), rFlavor );
        }
    }

    return bResult;
}

sal_Bool ChartTransferable::WriteObject( SotStorageStreamRef& rxOStm, void* pUserObject, sal_uInt32 nUserObjectId,
    const datatransfer::DataFlavor& /* rFlavor */ )
{
    // called from SetObject, put data into stream

    sal_Bool bRet = sal_False;
    switch ( nUserObjectId )
    {
        case CHARTTRANSFER_OBJECTTYPE_DRAWMODEL:
            {
                SdrModel* pMarkedObjModel = reinterpret_cast< SdrModel* >( pUserObject );
                if ( pMarkedObjModel )
                {
                    rxOStm->SetBufferSize( 0xff00 );

                    // #108584#
                    // for the changed pool defaults from drawing layer pool set those
                    // attributes as hard attributes to preserve them for saving
                    const SfxItemPool& rItemPool = pMarkedObjModel->GetItemPool();
                    const SvxFontHeightItem& rDefaultFontHeight = static_cast< const SvxFontHeightItem& >(
                        rItemPool.GetDefaultItem( EE_CHAR_FONTHEIGHT ) );
                    sal_uInt16 nCount = pMarkedObjModel->GetPageCount();
                    for ( sal_uInt16 i = 0; i < nCount; ++i )
                    {
                        const SdrPage* pPage = pMarkedObjModel->GetPage( i );
                        SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
                        while ( aIter.IsMore() )
                        {
                            SdrObject* pObj = aIter.Next();
                            const SvxFontHeightItem& rItem = static_cast< const SvxFontHeightItem& >(
                                pObj->GetMergedItem( EE_CHAR_FONTHEIGHT ) );
                            if ( rItem.GetHeight() == rDefaultFontHeight.GetHeight() )
                            {
                                pObj->SetMergedItem( rDefaultFontHeight );
                            }
                        }
                    }

                    Reference< io::XOutputStream > xDocOut( new utl::OOutputStreamWrapper( *rxOStm ) );
                    if ( SvxDrawingLayerExport( pMarkedObjModel, xDocOut ) )
                    {
                        rxOStm->Commit();
                    }

                    bRet = ( rxOStm->GetError() == ERRCODE_NONE );
                }
            }
            break;
        default:
            {
                DBG_ERROR( "ChartTransferable::WriteObject: unknown object id" );
            }
            break;
    }
    return bRet;
}

} //  namespace chart
