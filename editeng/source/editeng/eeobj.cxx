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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_editeng.hxx"


#include <eeng_pch.hxx>

#include <eeobj.hxx>
#include <sot/exchange.hxx>
#include <sot/formats.hxx>
#include <editeng/editeng.hxx>
#include <svl/itempool.hxx>
#include <osl/mutex.hxx>
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
    if ( ( nT == SOT_FORMAT_STRING ) || ( nT == SOT_FORMAT_RTF ) || ( nT == SOT_FORMATSTR_ID_EDITENGINE ) )
        bSupported = sal_True;

    return bSupported;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
