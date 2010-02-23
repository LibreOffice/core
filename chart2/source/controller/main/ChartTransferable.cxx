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

#include <vcl/graph.hxx>
#include <svx/svdmodel.hxx>

// header for class SdrView
#include <svx/svdview.hxx>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

namespace chart
{

ChartTransferable::ChartTransferable( SdrModel * pDrawModel, SdrObject * pSelectedObj )
{
    SdrExchangeView * pExchgView( new SdrView( pDrawModel ));
    SdrPageView* pPv = pExchgView->ShowSdrPage( pDrawModel->GetPage( 0 ));
    if( pSelectedObj )
        pExchgView->MarkObj( pSelectedObj, pPv );
    else
        pExchgView->MarkAllObj( pPv );
    Graphic aGraphic( pExchgView->GetMarkedObjMetaFile( TRUE ));
    m_xMetaFileGraphic.set( aGraphic.GetXGraphic());
    delete pExchgView;
}

ChartTransferable::~ChartTransferable()
{}

void ChartTransferable::AddSupportedFormats()
{
    AddFormat( SOT_FORMAT_GDIMETAFILE );
    AddFormat( SOT_FORMAT_BITMAP );
}

sal_Bool ChartTransferable::GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor )
{
    sal_uInt32  nFormat = SotExchange::GetFormat( rFlavor );
    sal_Bool    bResult = sal_False;

    if( HasFormat( nFormat ))
    {
        if( nFormat == FORMAT_GDIMETAFILE )
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


} //  namespace chart
