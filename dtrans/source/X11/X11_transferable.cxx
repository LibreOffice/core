/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: X11_transferable.cxx,v $
 * $Revision: 1.9 $
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
#include "precompiled_dtrans.hxx"

#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif

#include <X11_transferable.hxx>
#include <X11/Xatom.h>
#include <com/sun/star/io/IOException.hpp>

using namespace com::sun::star::datatransfer;
using namespace com::sun::star::lang;
using namespace com::sun::star::io;
using namespace com::sun::star::uno;
using namespace cppu;
using namespace osl;
using namespace rtl;


using namespace x11;


X11Transferable::X11Transferable(
    SelectionManager& rManager,
    const Reference< XInterface >& xCreator,
    Atom selection
    ) :
        m_rManager( rManager ),
        m_xCreator( xCreator ),
        m_aSelection( selection )
{
}

//==================================================================================================

X11Transferable::~X11Transferable()
{
}

//==================================================================================================

Any SAL_CALL X11Transferable::getTransferData( const DataFlavor& rFlavor )
    throw(UnsupportedFlavorException, IOException, RuntimeException)
{
    Any aRet;
    Sequence< sal_Int8 > aData;
    bool bSuccess = m_rManager.getPasteData( m_aSelection ? m_aSelection : XA_PRIMARY, rFlavor.MimeType, aData );
    if( ! bSuccess && m_aSelection == 0 )
        bSuccess = m_rManager.getPasteData( m_rManager.getAtom( OUString::createFromAscii( "CLIPBOARD" ) ), rFlavor.MimeType, aData );

    if( ! bSuccess )
    {
        throw UnsupportedFlavorException( rFlavor.MimeType, static_cast < XTransferable * > ( this ) );
    }
    if( rFlavor.MimeType.equalsIgnoreAsciiCase( OUString::createFromAscii( "text/plain;charset=utf-16" ) ) )
    {
        int nLen = aData.getLength()/2;
        if( ((sal_Unicode*)aData.getConstArray())[nLen-1] == 0 )
            nLen--;
        OUString aString( (sal_Unicode*)aData.getConstArray(), nLen );
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "X11Transferable::getTransferData( \"%s\" )\n -> \"%s\"\n",
             OUStringToOString( rFlavor.MimeType, RTL_TEXTENCODING_ISO_8859_1 ).getStr(),
             OUStringToOString( aString, RTL_TEXTENCODING_ISO_8859_1 ).getStr() );
#endif
        aRet <<= aString;
    }
    else
        aRet <<= aData;
    return aRet;
}

//==================================================================================================

Sequence< DataFlavor > SAL_CALL X11Transferable::getTransferDataFlavors()
    throw(RuntimeException)
{
    Sequence< DataFlavor > aFlavorList;
    bool bSuccess = m_rManager.getPasteDataTypes( m_aSelection ? m_aSelection : XA_PRIMARY, aFlavorList );
    if( ! bSuccess && m_aSelection == 0 )
        bSuccess = m_rManager.getPasteDataTypes( m_rManager.getAtom( OUString::createFromAscii( "CLIPBOARD" ) ), aFlavorList );

    return aFlavorList;
}

//==================================================================================================

sal_Bool SAL_CALL X11Transferable::isDataFlavorSupported( const DataFlavor& aFlavor )
    throw(RuntimeException)
{
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

