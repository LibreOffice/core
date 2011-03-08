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

#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif

#define INCL_WIN
#include <svpm.h>

#include <string.h>
#include <com/sun/star/io/IOException.hpp>
#include "Os2Transferable.hxx"

using namespace com::sun::star::datatransfer;
using namespace com::sun::star::lang;
using namespace com::sun::star::io;
using namespace com::sun::star::uno;
using namespace cppu;
using namespace osl;
using namespace os2;

using ::rtl::OUString;

// =======================================================================

Os2Transferable::Os2Transferable(
    const Reference< XInterface >& xCreator ) :
        m_xCreator( xCreator )
{
    debug_printf("Os2Transferable::Os2Transferable %08x\n", this);
    hAB = WinQueryAnchorBlock( HWND_DESKTOP );

    // query clipboard data to get mimetype
    if( UWinOpenClipbrd( hAB ) )
    {
        ULONG handle = UWinQueryClipbrdData( hAB, UCLIP_CF_UNICODETEXT);
        if (handle) {
            aFlavor.MimeType = OUString(RTL_CONSTASCII_USTRINGPARAM("text/plain;charset=utf-16"));
            aFlavor.DataType = getCppuType( (OUString*)0 );
            //debug_printf("Os2Transferable::Os2Transferable pszText %s\n", pszText);
        }
        handle = UWinQueryClipbrdData( hAB, UCLIP_CF_BITMAP);
        if (handle) {
            aFlavor.MimeType = OUString(RTL_CONSTASCII_USTRINGPARAM( "application/x-openoffice-bitmap;windows_formatname=\"Bitmap\"" ));
            aFlavor.DataType = getCppuType( (OUString*)0 );
            //debug_printf("Os2Transferable::Os2Transferable pszText %s\n", pszText);
        }
        UWinCloseClipbrd( hAB);
    }
    else
    {
        debug_printf("Os2Transferable::Os2Transferable failed to open clipboard\n");
    }

}

//==================================================================================================

Os2Transferable::~Os2Transferable()
{
    debug_printf("Os2Transferable::~Os2Transferable %08x\n", this);
}

//==================================================================================================

Any SAL_CALL Os2Transferable::getTransferData( const DataFlavor& rFlavor )
    throw(UnsupportedFlavorException, IOException, RuntimeException)
{
    debug_printf("Os2Transferable::getTransferData %08x\n", this);
    debug_printf("Os2Transferable::getTransferData mimetype: %s\n", CHAR_POINTER(rFlavor.MimeType));
    Any aRet;
    Sequence< sal_Int8 > aData;

    // retrieve unicode text
    if( rFlavor.MimeType.equalsIgnoreAsciiCase( OUString(RTL_CONSTASCII_USTRINGPARAM("text/plain;charset=utf-16")) ) )
    {
        if( UWinOpenClipbrd( hAB ) )
        {
            // check if clipboard has text format
            sal_Unicode* pszText = (sal_Unicode*) UWinQueryClipbrdData( hAB, UCLIP_CF_UNICODETEXT);
            if (pszText) {
                // convert to oustring and return it
                OUString aString( pszText);
                aRet <<= aString;
            }
            UWinCloseClipbrd( hAB );
            if (pszText)
                return aRet;
        }
    }

    // retrieve bitmap
    if( rFlavor.MimeType.equalsIgnoreAsciiCase( OUString(RTL_CONSTASCII_USTRINGPARAM( "application/x-openoffice-bitmap;windows_formatname=\"Bitmap\"" )) ) )
    {
        if( UWinOpenClipbrd( hAB ) )
        {
            // check if clipboard has text format
            ULONG handle = UWinQueryClipbrdData( hAB, UCLIP_CF_BITMAP);
            if (handle) {
                Sequence< sal_Int8 > winDIBStream;
                // convert to oustring and return it
                if (OS2HandleToOOoBmp( handle, &winDIBStream))
                    aRet <<= winDIBStream;
                else
                    handle = 0;
            }
            UWinCloseClipbrd( hAB );
            if (handle)
                return aRet;
        }
    }

    // clipboard format unsupported, throw exception
    throw UnsupportedFlavorException( rFlavor.MimeType, static_cast < XTransferable * > ( this ) );
}

//==================================================================================================

Sequence< DataFlavor > SAL_CALL Os2Transferable::getTransferDataFlavors()
    throw(RuntimeException)
{
    debug_printf("Os2Transferable::getTransferDataFlavors %08x\n", this);
    Sequence< DataFlavor > aFlavorList(1);
    aFlavorList[0] = aFlavor;
    debug_printf("Os2Transferable::getTransferDataFlavors mimetype: %s\n", CHAR_POINTER(aFlavor.MimeType));
    return aFlavorList;
}

//==================================================================================================

sal_Bool SAL_CALL Os2Transferable::isDataFlavorSupported( const DataFlavor& aFlavor )
    throw(RuntimeException)
{
    debug_printf("Os2Transferable::isDataFlavorSupported %08x\n", this);
    debug_printf("Os2Transferable::isDataFlavorSupported %s\n", CHAR_POINTER(aFlavor.MimeType));

    if( aFlavor.DataType != getCppuType( (Sequence< sal_Int8 >*)0 ) )
    {
        if( ! aFlavor.MimeType.equalsIgnoreAsciiCase( OUString(RTL_CONSTASCII_USTRINGPARAM("text/plain;charset=utf-16")) ) &&
            aFlavor.DataType == getCppuType( (OUString*)0 ) )
            return false;
    }

    Sequence< DataFlavor > aFlavors( getTransferDataFlavors() );
    for( int i = 0; i < aFlavors.getLength(); i++ )
        if( aFlavor.MimeType.equalsIgnoreAsciiCase( aFlavors.getConstArray()[i].MimeType ) &&
            aFlavor.DataType == aFlavors.getConstArray()[i].DataType )
            return sal_True;

    return sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
