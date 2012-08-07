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

#include "dlgedclip.hxx"
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/datatransfer/XMimeContentType.hpp>
#include <com/sun/star/datatransfer/XMimeContentTypeFactory.hpp>


using namespace comphelper;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::datatransfer::clipboard;


//----------------------------------------------------------------------------

DlgEdTransferableImpl::DlgEdTransferableImpl( const Sequence< DataFlavor >& aSeqFlavors, const Sequence< Any >& aSeqData )
{
    m_SeqFlavors = aSeqFlavors;
    m_SeqData = aSeqData;
}

//----------------------------------------------------------------------------

DlgEdTransferableImpl::~DlgEdTransferableImpl()
{
}

//----------------------------------------------------------------------------

sal_Bool DlgEdTransferableImpl::compareDataFlavors( const DataFlavor& lFlavor, const DataFlavor& rFlavor )
{
    bool bRet = false;

    // compare mime content types
    Reference< lang::XMultiServiceFactory >  xMSF = getProcessServiceFactory();
    Reference< datatransfer::XMimeContentTypeFactory >
        xMCntTypeFactory( xMSF->createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.datatransfer.MimeContentTypeFactory" ) ) ), UNO_QUERY );

    if ( xMCntTypeFactory.is( ) )
    {
        // compare full media types
        Reference< datatransfer::XMimeContentType > xLType = xMCntTypeFactory->createMimeContentType( lFlavor.MimeType );
        Reference< datatransfer::XMimeContentType > xRType = xMCntTypeFactory->createMimeContentType( rFlavor.MimeType );

        ::rtl::OUString aLFullMediaType = xLType->getFullMediaType();
        ::rtl::OUString aRFullMediaType = xRType->getFullMediaType();

        bRet = aLFullMediaType.equalsIgnoreAsciiCase( aRFullMediaType );
    }

    return bRet;
}

// XTransferable
//----------------------------------------------------------------------------

Any SAL_CALL DlgEdTransferableImpl::getTransferData( const DataFlavor& rFlavor ) throw(UnsupportedFlavorException, IOException, RuntimeException)
{
    const SolarMutexGuard aGuard;

    if ( !isDataFlavorSupported( rFlavor ) )
        throw UnsupportedFlavorException();

    Any aData;

    for ( sal_Int32 i = 0; i < m_SeqFlavors.getLength(); i++ )
    {
        if ( compareDataFlavors( m_SeqFlavors[i] , rFlavor ) )
        {
            aData = m_SeqData[i];
            break;
        }
    }

    return aData;
}

//----------------------------------------------------------------------------

Sequence< DataFlavor > SAL_CALL DlgEdTransferableImpl::getTransferDataFlavors(  ) throw(RuntimeException)
{
    const SolarMutexGuard aGuard;

    return m_SeqFlavors;
}

//----------------------------------------------------------------------------

sal_Bool SAL_CALL DlgEdTransferableImpl::isDataFlavorSupported( const DataFlavor& rFlavor ) throw(RuntimeException)
{
    const SolarMutexGuard aGuard;

    for ( sal_Int32 i = 0; i < m_SeqFlavors.getLength(); i++ )
        if ( compareDataFlavors( m_SeqFlavors[i] , rFlavor ) )
            return true;
    return false;
}

// XClipboardOwner
//----------------------------------------------------------------------------

void SAL_CALL DlgEdTransferableImpl::lostOwnership( const Reference< XClipboard >&, const Reference< XTransferable >& ) throw(RuntimeException)
{
    const SolarMutexGuard aGuard;

    m_SeqFlavors = Sequence< DataFlavor >();
    m_SeqData = Sequence< Any >();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
