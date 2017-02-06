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

#include <X11_transferable.hxx>
#include <X11/Xatom.h>
#include <com/sun/star/datatransfer/UnsupportedFlavorException.hpp>
#include <com/sun/star/io/IOException.hpp>

using namespace com::sun::star::datatransfer;
using namespace com::sun::star::lang;
using namespace com::sun::star::io;
using namespace com::sun::star::uno;
using namespace cppu;
using namespace osl;

using namespace x11;

X11Transferable::X11Transferable(
    SelectionManager& rManager,
    Atom selection
    ) :
        m_rManager( rManager ),
        m_aSelection( selection )
{
}

X11Transferable::~X11Transferable()
{
}

Any SAL_CALL X11Transferable::getTransferData( const DataFlavor& rFlavor )
{
    Any aRet;
    Sequence< sal_Int8 > aData;
    bool bSuccess = m_rManager.getPasteData( m_aSelection ? m_aSelection : XA_PRIMARY, rFlavor.MimeType, aData );
    if( ! bSuccess && m_aSelection == 0 )
        bSuccess = m_rManager.getPasteData( m_rManager.getAtom( "CLIPBOARD" ), rFlavor.MimeType, aData );

    if( ! bSuccess )
    {
        throw UnsupportedFlavorException( rFlavor.MimeType, static_cast < XTransferable * > ( this ) );
    }
    if( rFlavor.MimeType.equalsIgnoreAsciiCase( "text/plain;charset=utf-16" ) )
    {
        int nLen = aData.getLength()/2;
        if( reinterpret_cast<sal_Unicode const *>(aData.getConstArray())[nLen-1] == 0 )
            nLen--;
        OUString aString( reinterpret_cast<sal_Unicode const *>(aData.getConstArray()), nLen );
        SAL_INFO( "vcl.unx.dtrans", "X11Transferable::getTransferData( \"" << rFlavor.MimeType << "\" )\n -> \"" << aString << "\"\n");
        aRet <<= aString.replaceAll("\r\n", "\n");
    }
    else
        aRet <<= aData;
    return aRet;
}

Sequence< DataFlavor > SAL_CALL X11Transferable::getTransferDataFlavors()
{
    Sequence< DataFlavor > aFlavorList;
    bool bSuccess = m_rManager.getPasteDataTypes( m_aSelection ? m_aSelection : XA_PRIMARY, aFlavorList );
    if( ! bSuccess && m_aSelection == 0 )
        m_rManager.getPasteDataTypes( m_rManager.getAtom( "CLIPBOARD" ), aFlavorList );

    return aFlavorList;
}

sal_Bool SAL_CALL X11Transferable::isDataFlavorSupported( const DataFlavor& aFlavor )
{
    if( aFlavor.DataType != cppu::UnoType<Sequence< sal_Int8 >>::get() )
    {
        if( ! aFlavor.MimeType.equalsIgnoreAsciiCase( "text/plain;charset=utf-16" ) &&
            aFlavor.DataType == cppu::UnoType<OUString>::get() )
            return false;
    }

    Sequence< DataFlavor > aFlavors( getTransferDataFlavors() );
    for( int i = 0; i < aFlavors.getLength(); i++ )
        if( aFlavor.MimeType.equalsIgnoreAsciiCase( aFlavors.getConstArray()[i].MimeType ) &&
            aFlavor.DataType == aFlavors.getConstArray()[i].DataType )
            return true;

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
