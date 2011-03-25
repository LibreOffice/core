/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 * Copyright 2010 Novell, Inc.
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
#include "precompiled_fpicker.hxx"

#include "ODMAFolderPicker.hxx"

#define _SVSTDARR_STRINGSDTOR
#include "svl/svstdarr.hxx"

#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <cppuhelper/factory.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <unotools/pathoptions.hxx>

#ifndef ODMA_LIB_HXX
#include <odma_lib.hxx>
#endif

// using ----------------------------------------------------------------

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

//------------------------------------------------------------------------------------
// class ODMAFolderPicker
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
ODMAFolderPicker::ODMAFolderPicker( const Reference < XMultiServiceFactory >& xFactory ) :
    cppu::WeakComponentImplHelper2<
      XFolderPicker,
      XServiceInfo>( m_rbHelperMtx ),
    m_bUseDMS( sal_False )
{
    m_xInterface = xFactory->createInstance(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ui.dialogs.SystemFolderPicker" ) ));
}

// XExecutableDialog functions

void SAL_CALL ODMAFolderPicker::setTitle( const ::rtl::OUString& _rTitle ) throw (RuntimeException)
{
    if (m_bUseDMS)
        return;

    Reference< XExecutableDialog > xExecutableDialog( m_xInterface, UNO_QUERY );
    xExecutableDialog->setTitle( _rTitle);
}

sal_Int16 SAL_CALL ODMAFolderPicker::execute( )
    throw (RuntimeException)
{
    if (m_bUseDMS)
        throw new RuntimeException( );

    Reference< XExecutableDialog > xExecutableDialog( m_xInterface, UNO_QUERY );
    return xExecutableDialog->execute();
}

// XFolderPicker functions

void SAL_CALL ODMAFolderPicker::setDisplayDirectory( const ::rtl::OUString& aDirectory )
    throw( IllegalArgumentException, RuntimeException )
{
    if (m_bUseDMS)
        return;

    Reference< XFolderPicker > xFolderPicker( m_xInterface, UNO_QUERY );
    xFolderPicker->setDisplayDirectory( aDirectory );
}

::rtl::OUString SAL_CALL ODMAFolderPicker::getDisplayDirectory( )
    throw( RuntimeException )
{
    if (m_bUseDMS)
        return rtl::OUString();

    Reference< XFolderPicker > xFolderPicker( m_xInterface, UNO_QUERY );
    return xFolderPicker->getDisplayDirectory();
}

::rtl::OUString SAL_CALL ODMAFolderPicker::getDirectory( )
    throw( RuntimeException )
{
    if (m_bUseDMS)
        return rtl::OUString();

    Reference< XFolderPicker > xFolderPicker( m_xInterface, UNO_QUERY );
    return xFolderPicker->getDirectory();
}

void SAL_CALL ODMAFolderPicker::setDescription( const ::rtl::OUString& aDescription )
    throw( RuntimeException )
{
    if (m_bUseDMS)
        return;

    Reference< XFolderPicker > xFolderPicker( m_xInterface, UNO_QUERY );
    xFolderPicker->setDescription( aDescription );
}

// XServiceInfo

::rtl::OUString SAL_CALL ODMAFolderPicker::getImplementationName( )
    throw( RuntimeException )
{
    return impl_getStaticImplementationName();
}

sal_Bool SAL_CALL ODMAFolderPicker::supportsService( const ::rtl::OUString& sServiceName )
    throw( RuntimeException )
{
    Sequence< ::rtl::OUString > seqServiceNames = getSupportedServiceNames();
    const ::rtl::OUString* pArray = seqServiceNames.getConstArray();
    for ( sal_Int32 i = 0; i < seqServiceNames.getLength(); i++ )
    {
        if ( sServiceName == pArray[i] )
        {
            return sal_True ;
        }
    }
    return sal_False ;
}

Sequence< ::rtl::OUString > SAL_CALL ODMAFolderPicker::getSupportedServiceNames()
    throw( RuntimeException )
{
    return impl_getStaticSupportedServiceNames();
}

Sequence< ::rtl::OUString > ODMAFolderPicker::impl_getStaticSupportedServiceNames( )
{
    Sequence< ::rtl::OUString > seqServiceNames( 1 );
    ::rtl::OUString* pArray = seqServiceNames.getArray();
    pArray[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ui.dialogs.ODMAFolderPicker" ));
    return seqServiceNames ;
}

::rtl::OUString ODMAFolderPicker::impl_getStaticImplementationName( )
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.svtools.ODMAFolderPicker" ));
}

Reference< XInterface > SAL_CALL ODMAFolderPicker::impl_createInstance( const Reference< XComponentContext >& rxContext )
    throw( Exception )
{
    Reference< XMultiServiceFactory > xServiceManager (rxContext->getServiceManager(), UNO_QUERY_THROW);
    return Reference< XInterface >( *new ODMAFolderPicker( xServiceManager ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
