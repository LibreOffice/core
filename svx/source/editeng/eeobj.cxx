/*************************************************************************
 *
 *  $RCSfile: eeobj.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: mt $ $Date: 2002-01-16 10:40:10 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#include <eeng_pch.hxx>

#pragma hdrstop

#include <eeobj.hxx>
#include <sot/exchange.hxx>
#include <sot/formats.hxx>
#include <editeng.hxx>
#include <svtools/itempool.hxx>
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
using namespace ::com::sun::star;


EditDataObject::EditDataObject()
{
}

EditDataObject::~EditDataObject()
{
}

// uno::XInterface
uno::Any EditDataObject::queryInterface( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType, SAL_STATIC_CAST( datatransfer::XTransferable*, this ) );
    return (aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ));
}

// datatransfer::XTransferable
uno::Any EditDataObject::getTransferData( const datatransfer::DataFlavor& rFlavor ) throw(datatransfer::UnsupportedFlavorException, io::IOException, uno::RuntimeException)
{
    uno::Any aAny;

    ULONG nT = SotExchange::GetFormat( rFlavor );
    if ( nT == SOT_FORMAT_STRING )
    {
        aAny <<= (::rtl::OUString)GetString();
    }
    else if ( ( nT == SOT_FORMATSTR_ID_EDITENGINE ) || ( nT == SOT_FORMAT_RTF ) )
    {
        // MT 01/2002: No RTF on demand any more:
        // 1) Was not working, because I had to flush() the clipboard immediately anyway
        // 2) Don't have the old pool defaults and the StyleSheetPool here.

        SvMemoryStream* pStream = ( nT == SOT_FORMATSTR_ID_EDITENGINE ) ? &GetStream() : &GetRTFStream();
        pStream->Seek( STREAM_SEEK_TO_END );
        ULONG nLen = pStream->Tell();
        pStream->Seek(0);

        uno::Sequence< sal_Int8 > aSeq( nLen );
        memcpy( aSeq.getArray(), pStream->GetData(), nLen );
        aAny <<= aSeq;
    }
    else
    {
        datatransfer::UnsupportedFlavorException aException;
        throw( aException );
    }

    return aAny;
}

uno::Sequence< datatransfer::DataFlavor > EditDataObject::getTransferDataFlavors(  ) throw(uno::RuntimeException)
{
    uno::Sequence< datatransfer::DataFlavor > aDataFlavors(3);
    SotExchange::GetFormatDataFlavor( SOT_FORMATSTR_ID_EDITENGINE, aDataFlavors.getArray()[0] );
    SotExchange::GetFormatDataFlavor( SOT_FORMAT_STRING, aDataFlavors.getArray()[1] );
    SotExchange::GetFormatDataFlavor( SOT_FORMAT_RTF, aDataFlavors.getArray()[2] );

    return aDataFlavors;
}

sal_Bool EditDataObject::isDataFlavorSupported( const datatransfer::DataFlavor& rFlavor ) throw(uno::RuntimeException)
{
    sal_Bool bSupported = sal_False;

    ULONG nT = SotExchange::GetFormat( rFlavor );
    if ( ( nT == SOT_FORMAT_STRING ) || ( nT == SOT_FORMAT_RTF ) /* || ( nT == SOT_FORMAT_XML ) */ || ( nT == SOT_FORMATSTR_ID_EDITENGINE ) )
        bSupported = sal_True;

    return bSupported;
}
