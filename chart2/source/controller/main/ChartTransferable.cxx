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
    Graphic aGraphic( pExchgView->GetMarkedObjMetaFile(true));
    m_xMetaFileGraphic.set( aGraphic.GetXGraphic());
    if ( m_bDrawing )
    {
        m_pMarkedObjModel = ( pExchgView ? pExchgView->GetMarkedObjModel() : NULL );
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
            bResult = SetBitmapEx( aGraphic.GetBitmapEx(), rFlavor );
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
                OSL_FAIL( "ChartTransferable::WriteObject: unknown object id" );
            }
            break;
    }
    return bRet;
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
