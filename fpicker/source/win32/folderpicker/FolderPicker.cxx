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

#include <osl/diagnose.h>

#include "FolderPicker.hxx"
#include <com/sun/star/lang/DisposedException.hpp>
#include "WinFOPImpl.hxx"

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using com::sun::star::uno::Reference;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::XInterface;
using com::sun::star::lang::XMultiServiceFactory;
using com::sun::star::lang::XServiceInfo;
using com::sun::star::lang::DisposedException;
using com::sun::star::lang::IllegalArgumentException;
using rtl::OUString;
using osl::MutexGuard;

using namespace cppu;
using namespace com::sun::star::ui::dialogs;

//------------------------------------------------------------------------
// defines
//------------------------------------------------------------------------

#define FOLDERPICKER_IMPL_NAME  "com.sun.star.ui.dialogs.Win32FolderPicker"

//------------------------------------------------------------------------
// helper functions
//------------------------------------------------------------------------

namespace
{
    Sequence< OUString > SAL_CALL FolderPicker_getSupportedServiceNames()
    {
        Sequence< OUString > aRet(1);
        aRet[0] = OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.ui.dialogs.SystemFolderPicker"));
        return aRet;
    }
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

CFolderPicker::CFolderPicker( const Reference< XMultiServiceFactory >& xServiceMgr ) :
    m_xServiceMgr( xServiceMgr )
{
    m_pFolderPickerImpl = std::auto_ptr< CWinFolderPickerImpl > ( new CWinFolderPickerImpl( this ) );
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CFolderPicker::setTitle( const OUString& aTitle ) throw( RuntimeException )
{
    OSL_ASSERT( m_pFolderPickerImpl.get( ) );
    MutexGuard aGuard( m_aMutex );
    m_pFolderPickerImpl->setTitle( aTitle );
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CFolderPicker::setDisplayDirectory( const OUString& aDirectory )
    throw( IllegalArgumentException, RuntimeException )
{
    OSL_ASSERT( m_pFolderPickerImpl.get( ) );
    MutexGuard aGuard( m_aMutex );
    m_pFolderPickerImpl->setDisplayDirectory( aDirectory );
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

OUString SAL_CALL CFolderPicker::getDisplayDirectory( )
    throw( RuntimeException )
{
    OSL_ASSERT( m_pFolderPickerImpl.get( ) );
    MutexGuard aGuard( m_aMutex );
    return m_pFolderPickerImpl->getDisplayDirectory( );
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

OUString SAL_CALL CFolderPicker::getDirectory( ) throw( RuntimeException )
{
    OSL_ASSERT( m_pFolderPickerImpl.get( ) );
    MutexGuard aGuard( m_aMutex );
    return m_pFolderPickerImpl->getDirectory( );
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CFolderPicker::setDescription( const OUString& aDescription ) throw( RuntimeException )
{
    OSL_ASSERT( m_pFolderPickerImpl.get( ) );
    MutexGuard aGuard( m_aMutex );
    m_pFolderPickerImpl->setDescription( aDescription );
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

sal_Int16 SAL_CALL CFolderPicker::execute( )
    throw( RuntimeException )
{
    OSL_ASSERT( m_pFolderPickerImpl.get( ) );

    // we should not block in this call else
    // in the case of an event the client can'tgetPImplFromHandle( hWnd )
    // call another function an we run into a
    // deadlock !!!!!
    return m_pFolderPickerImpl->execute( );
}

// -------------------------------------------------
// XServiceInfo
// -------------------------------------------------

OUString SAL_CALL CFolderPicker::getImplementationName(  )
    throw( RuntimeException )
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM( FOLDERPICKER_IMPL_NAME ));
}

// -------------------------------------------------
//  XServiceInfo
// -------------------------------------------------

sal_Bool SAL_CALL CFolderPicker::supportsService( const OUString& ServiceName )
    throw( RuntimeException )
{
    Sequence < OUString > SupportedServicesNames = FolderPicker_getSupportedServiceNames();

    for ( sal_Int32 n = SupportedServicesNames.getLength(); n--; )
        if (SupportedServicesNames[n].compareTo(ServiceName) == 0)
            return sal_True;

    return sal_False;
}

// -------------------------------------------------
//  XServiceInfo
// -------------------------------------------------

Sequence< OUString > SAL_CALL CFolderPicker::getSupportedServiceNames(   )
    throw( RuntimeException )
{
    return FolderPicker_getSupportedServiceNames();
}

// -------------------------------------------------
//  XCancellable
// -------------------------------------------------

void SAL_CALL CFolderPicker::cancel( )
    throw(RuntimeException)
{
    OSL_ASSERT( m_pFolderPickerImpl.get( ) );
    MutexGuard aGuard( m_aMutex );
    m_pFolderPickerImpl->cancel( );
}

//------------------------------------------------
// overwrite base class method, which is called
// by base class dispose function
//------------------------------------------------

void SAL_CALL CFolderPicker::disposing()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
