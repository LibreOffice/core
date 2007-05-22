/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChartTransferable.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:05:12 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "ChartTransferable.hxx"

#include <vcl/graph.hxx>
#include <svx/svdmodel.hxx>

// header for class SdrView
#ifndef _SVDVIEW_HXX
#include <svx/svdview.hxx>
#endif

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
