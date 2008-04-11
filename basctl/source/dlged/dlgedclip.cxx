/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dlgedclip.cxx,v $
 * $Revision: 1.6 $
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
#include "precompiled_basctl.hxx"
#include "dlgedclip.hxx"
#include <vos/mutex.hxx>
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


//============================================================================
// DlgEdTransferableImpl
//============================================================================

//----------------------------------------------------------------------------

DlgEdTransferableImpl::DlgEdTransferableImpl()
    :m_SeqFlavors(0)
    ,m_SeqData(0)
{
}

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
    sal_Bool bRet = sal_False;

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
    const ::vos::OGuard aGuard( Application::GetSolarMutex() );

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
    const ::vos::OGuard aGuard( Application::GetSolarMutex() );

    return m_SeqFlavors;
}

//----------------------------------------------------------------------------

sal_Bool SAL_CALL DlgEdTransferableImpl::isDataFlavorSupported( const DataFlavor& rFlavor ) throw(RuntimeException)
{
    const ::vos::OGuard aGuard( Application::GetSolarMutex() );

    sal_Bool bRet = sal_False;

    for ( sal_Int32 i = 0; i < m_SeqFlavors.getLength(); i++ )
    {
        if ( compareDataFlavors( m_SeqFlavors[i] , rFlavor ) )
        {
            bRet = sal_True;
            break;
        }
    }

    return bRet;
}

// XClipboardOwner
//----------------------------------------------------------------------------

void SAL_CALL DlgEdTransferableImpl::lostOwnership( const Reference< XClipboard >&, const Reference< XTransferable >& ) throw(RuntimeException)
{
    const ::vos::OGuard aGuard( Application::GetSolarMutex() );

    m_SeqFlavors = Sequence< DataFlavor >();
    m_SeqData = Sequence< Any >();
}

