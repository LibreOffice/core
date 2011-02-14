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

#include "xconnection.hxx"
#include "svdata.hxx"
#include "salinst.hxx"
#include "vcl/svapp.hxx"

namespace vcl
{
    class SolarMutexReleaser
    {
        sal_uLong mnReleased;
    public:
        SolarMutexReleaser()
        {
            mnReleased = Application::ReleaseSolarMutex();
        }

        ~SolarMutexReleaser()
        {
            if( mnReleased )
                Application::AcquireSolarMutex( mnReleased );
        }
    };
}

using namespace rtl;
using namespace osl;
using namespace vcl;
using namespace com::sun::star::uno;
using namespace com::sun::star::awt;


DisplayConnection::DisplayConnection()
{
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->mpDefInst->SetEventCallback( this, dispatchEvent );
    pSVData->mpDefInst->SetErrorEventCallback( this, dispatchErrorEvent );

    SalInstance::ConnectionIdentifierType eType;
    int nBytes;
    void* pBytes = pSVData->mpDefInst->GetConnectionIdentifier( eType, nBytes );
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
{
    ImplSVData* pSVData = ImplGetSVData();

    if( pSVData )
    {
        pSVData->mpDefInst->SetEventCallback( NULL, NULL );
        pSVData->mpDefInst->SetErrorEventCallback( NULL, NULL );
    }
}


void SAL_CALL DisplayConnection::addEventHandler( const Any& /*window*/, const Reference< XEventHandler >& handler, sal_Int32 /*eventMask*/ ) throw()
{
    MutexGuard aGuard( m_aMutex );

    m_aHandlers.push_back( handler );
}

void SAL_CALL DisplayConnection::removeEventHandler( const Any& /*window*/, const Reference< XEventHandler >& handler ) throw()
{
    MutexGuard aGuard( m_aMutex );

    m_aHandlers.remove( handler );
}

void SAL_CALL DisplayConnection::addErrorHandler( const Reference< XEventHandler >& handler ) throw()
{
    MutexGuard aGuard( m_aMutex );

    m_aErrorHandlers.push_back( handler );
}

void SAL_CALL DisplayConnection::removeErrorHandler( const Reference< XEventHandler >& handler ) throw()
{
    MutexGuard aGuard( m_aMutex );

    m_aErrorHandlers.remove( handler );
}

Any SAL_CALL DisplayConnection::getIdentifier() throw()
{
    return m_aAny;
}

void DisplayConnection::dispatchDowningEvent()
{
    SolarMutexReleaser aRel;

    MutexGuard aGuard( m_aMutex );
    Any aEvent;
    std::list< Reference< XEventHandler > > aLocalList( m_aHandlers );
    for( ::std::list< Reference< XEventHandler > >::const_iterator it = aLocalList.begin(); it != aLocalList.end(); ++it )
        (*it)->handleEvent( aEvent );
}

bool DisplayConnection::dispatchEvent( void* pThis, void* pData, int nBytes )
{
    SolarMutexReleaser aRel;

    DisplayConnection* This = (DisplayConnection*)pThis;

    Sequence< sal_Int8 > aSeq( (sal_Int8*)pData, nBytes );
    Any aEvent;
    aEvent <<= aSeq;
    ::std::list< Reference< XEventHandler > > handlers;
    {
        MutexGuard aGuard( This->m_aMutex );
        handlers = This->m_aHandlers;
    }
    for( ::std::list< Reference< XEventHandler > >::const_iterator it = handlers.begin(); it != handlers.end(); ++it )
        if( (*it)->handleEvent( aEvent ) )
            return true;
    return false;
}

bool DisplayConnection::dispatchErrorEvent( void* pThis, void* pData, int nBytes )
{
    SolarMutexReleaser aRel;

    DisplayConnection* This = (DisplayConnection*)pThis;

    Sequence< sal_Int8 > aSeq( (sal_Int8*)pData, nBytes );
    Any aEvent;
    aEvent <<= aSeq;
    ::std::list< Reference< XEventHandler > > handlers;
    {
        MutexGuard aGuard( This->m_aMutex );
        handlers = This->m_aErrorHandlers;
    }
    for( ::std::list< Reference< XEventHandler > >::const_iterator it = handlers.begin(); it != handlers.end(); ++it )
        if( (*it)->handleEvent( aEvent ) )
            return true;

    return false;
}
