/*************************************************************************
 *
 *  $RCSfile: eeobj.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: avy $ $Date: 2001-03-14 10:03:49 $
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
    else if ( nT == SOT_FORMATSTR_ID_EDITENGINE )
    {
        GetStream().Seek( STREAM_SEEK_TO_END );
        ULONG nLen = GetStream().Tell();
        GetStream().Seek(0);

        uno::Sequence< sal_Int8 > aSeq( nLen );
        memcpy( aSeq.getArray(), GetStream().GetData(), nLen );
        aAny <<= aSeq;
    }
    else if ( nT == SOT_FORMAT_RTF )
    {
        vos::OGuard aVclGuard( Application::GetSolarMutex() );

        SfxItemPool* pTmpPool = EditEngine::CreatePool( FALSE );
        EditEngine* pEditEngine = new EditEngine( pTmpPool );

        GetStream().Seek(0);
        pEditEngine->Read( GetStream(), EE_FORMAT_BIN );
        GetStream().Seek(0);

        SvMemoryStream aRTFStream;
        pEditEngine->Write( aRTFStream, EE_FORMAT_RTF );
        ULONG nLen = aRTFStream.Tell();

        uno::Sequence< sal_Int8 > aSeq( nLen );
        memcpy( aSeq.getArray(), aRTFStream.GetData(), nLen );
        aAny <<= aSeq;

        delete pEditEngine;
        delete pTmpPool;
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
    if ( ( nT == SOT_FORMAT_STRING ) || ( nT == SOT_FORMAT_RTF ) || ( nT == SOT_FORMATSTR_ID_EDITENGINE ) )
        bSupported = sal_True;

    return bSupported;
}
