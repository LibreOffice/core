/*************************************************************************
 *
 *  $RCSfile: X11_transferable.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: pl $ $Date: 2001-02-21 16:34:09 $
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
 *  WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <X11_transferable.hxx>
#include <X11/Xatom.h>

#ifndef _COM_SUN_STAR_IO_IOEXCEPTION_HPP_
#include <com/sun/star/io/IOException.hpp>
#endif

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
    if( rFlavor.MimeType.equalsIgnoreCase( OUString::createFromAscii( "text/plain;charset=utf-16" ) ) )
    {
        int nLen = aData.getLength()/2;
        if( ((sal_Unicode*)aData.getConstArray())[nLen-1] == 0 )
            nLen--;
        OUString aString( (sal_Unicode*)aData.getConstArray(), nLen );
#ifdef DEBUG
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
        if( ! aFlavor.MimeType.equalsIgnoreCase( OUString::createFromAscii( "text/plain;charset=utf-16" ) ) &&
            aFlavor.DataType == getCppuType( (OUString*)0 ) )
            return false;
    }

    Sequence< DataFlavor > aFlavors( getTransferDataFlavors() );
    for( int i = 0; i < aFlavors.getLength(); i++ )
        if( aFlavor.MimeType.equalsIgnoreCase( aFlavors.getConstArray()[i].MimeType ) &&
            aFlavor.DataType == aFlavors.getConstArray()[i].DataType )
            return sal_True;

    return sal_False;
}

