/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Os2Transferable.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-20 16:39:22 $
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

#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif

#define INCL_WIN
#include <svpm.h>

#include <string.h>

#ifndef _COM_SUN_STAR_IO_IOEXCEPTION_HPP_
#include <com/sun/star/io/IOException.hpp>
#endif

#ifndef _DTRANS_OS2_TRANSFERABLE_HXX_
#include "Os2Transferable.hxx"
#endif

using namespace com::sun::star::datatransfer;
using namespace com::sun::star::lang;
using namespace com::sun::star::io;
using namespace com::sun::star::uno;
using namespace cppu;
using namespace osl;
using namespace rtl;
using namespace os2;

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
            aFlavor.MimeType = OUString::createFromAscii( "text/plain;charset=utf-16" );
            aFlavor.DataType = getCppuType( (OUString*)0 );
            //debug_printf("Os2Transferable::Os2Transferable pszText %s\n", pszText);
        }
        handle = UWinQueryClipbrdData( hAB, UCLIP_CF_BITMAP);
        if (handle) {
            aFlavor.MimeType = OUString::createFromAscii( "application/x-openoffice-bitmap;windows_formatname=\"Bitmap\"" );
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
    if( rFlavor.MimeType.equalsIgnoreAsciiCase( OUString::createFromAscii( "text/plain;charset=utf-16" ) ) )
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
    if( rFlavor.MimeType.equalsIgnoreAsciiCase( OUString::createFromAscii( "application/x-openoffice-bitmap;windows_formatname=\"Bitmap\"" ) ) )
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
        if( ! aFlavor.MimeType.equalsIgnoreAsciiCase( OUString::createFromAscii( "text/plain;charset=utf-16" ) ) &&
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

