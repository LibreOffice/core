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
#include "precompiled_vcl.hxx"

#include "svsys.h"
#include "rtl/ref.hxx"
#include "vcl/xconnection.hxx"
#include "vcl/svdata.hxx"
#include "vcl/salinst.hxx"
#include "vcl/svapp.hxx"

namespace {

namespace css = com::sun::star;

}

using namespace osl;
using namespace vcl;
using namespace com::sun::star::uno;
using namespace com::sun::star::awt;

using ::rtl::OUString;

DisplayConnection::DisplayConnection()
{
    SalInstance::ConnectionIdentifierType eType;
    int nBytes;
    void* pBytes = ImplGetSVData()->mpDefInst->GetConnectionIdentifier( eType, nBytes );
    switch( eType )
    {
        case SalInstance::AsciiCString:
            m_aAny <<= OUString::createFromAscii( (sal_Char*)pBytes );
            break;
        case SalInstance::Blob:
            m_aAny <<= Sequence< sal_Int8 >( (sal_Int8*)pBytes, nBytes );
            break;
    }
}

DisplayConnection::~DisplayConnection()
{}

void DisplayConnection::start()
{
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->mpDefInst->SetEventCallback( this );
}

void DisplayConnection::terminate()
{
    ImplSVData* pSVData = ImplGetSVData();

    if( pSVData )
    {
        pSVData->mpDefInst->SetEventCallback( NULL );
    }

    SolarMutexReleaser aRel;

    MutexGuard aGuard( m_aMutex );
    Any aEvent;
    std::list< css::uno::Reference< XEventHandler > > aLocalList( m_aHandlers );
    for( ::std::list< css::uno::Reference< XEventHandler > >::const_iterator it = aLocalList.begin(); it != aLocalList.end(); ++it )
        (*it)->handleEvent( aEvent );
}

void SAL_CALL DisplayConnection::addEventHandler( const Any& /*window*/, const css::uno::Reference< XEventHandler >& handler, sal_Int32 /*eventMask*/ ) throw()
{
    MutexGuard aGuard( m_aMutex );

    m_aHandlers.push_back( handler );
}

void SAL_CALL DisplayConnection::removeEventHandler( const Any& /*window*/, const css::uno::Reference< XEventHandler >& handler ) throw()
{
    MutexGuard aGuard( m_aMutex );

    m_aHandlers.remove( handler );
}

void SAL_CALL DisplayConnection::addErrorHandler( const css::uno::Reference< XEventHandler >& handler ) throw()
{
    MutexGuard aGuard( m_aMutex );

    m_aErrorHandlers.push_back( handler );
}

void SAL_CALL DisplayConnection::removeErrorHandler( const css::uno::Reference< XEventHandler >& handler ) throw()
{
    MutexGuard aGuard( m_aMutex );

    m_aErrorHandlers.remove( handler );
}

Any SAL_CALL DisplayConnection::getIdentifier() throw()
{
    return m_aAny;
}

bool DisplayConnection::dispatchEvent( void* pData, int nBytes )
{
    SolarMutexReleaser aRel;

    Sequence< sal_Int8 > aSeq( (sal_Int8*)pData, nBytes );
    Any aEvent;
    aEvent <<= aSeq;
    ::std::list< css::uno::Reference< XEventHandler > > handlers;
    {
        MutexGuard aGuard( m_aMutex );
        handlers = m_aHandlers;
    }
    for( ::std::list< css::uno::Reference< XEventHandler > >::const_iterator it = handlers.begin(); it != handlers.end(); ++it )
        if( (*it)->handleEvent( aEvent ) )
            return true;
    return false;
}

bool DisplayConnection::dispatchErrorEvent( void* pData, int nBytes )
{
    SolarMutexReleaser aRel;

    Sequence< sal_Int8 > aSeq( (sal_Int8*)pData, nBytes );
    Any aEvent;
    aEvent <<= aSeq;
    ::std::list< css::uno::Reference< XEventHandler > > handlers;
    {
        MutexGuard aGuard( m_aMutex );
        handlers = m_aErrorHandlers;
    }
    for( ::std::list< css::uno::Reference< XEventHandler > >::const_iterator it = handlers.begin(); it != handlers.end(); ++it )
        if( (*it)->handleEvent( aEvent ) )
            return true;

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
