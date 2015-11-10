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

#include "vcl/svapp.hxx"

#include "xconnection.hxx"
#include "svdata.hxx"
#include "salinst.hxx"

using namespace osl;
using namespace vcl;
using namespace com::sun::star::uno;
using namespace com::sun::star::awt;

DisplayConnection::DisplayConnection()
{
    SalInstance::ConnectionIdentifierType eType;
    int nBytes;
    void* pBytes = ImplGetSVData()->mpDefInst->GetConnectionIdentifier( eType, nBytes );
    switch( eType )
    {
        case SalInstance::AsciiCString:
            m_aAny <<= OUString::createFromAscii( static_cast<sal_Char*>(pBytes) );
            break;
        case SalInstance::Blob:
            m_aAny <<= Sequence< sal_Int8 >( static_cast<sal_Int8*>(pBytes), nBytes );
            break;
    }
}

DisplayConnection::~DisplayConnection()
{}

void DisplayConnection::start()
{
    DBG_TESTSOLARMUTEX();
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->mpDefInst->SetEventCallback( this );
}

void DisplayConnection::terminate()
{
    DBG_TESTSOLARMUTEX();
    ImplSVData* pSVData = ImplGetSVData();

    if( pSVData )
    {
        pSVData->mpDefInst->SetEventCallback( nullptr );
    }

    SolarMutexReleaser aRel;

    MutexGuard aGuard( m_aMutex );
    Any aEvent;
    std::list< css::uno::Reference< XEventHandler > > aLocalList( m_aHandlers );
    for( ::std::list< css::uno::Reference< XEventHandler > >::const_iterator it = aLocalList.begin(); it != aLocalList.end(); ++it )
        (*it)->handleEvent( aEvent );
}

void SAL_CALL DisplayConnection::addEventHandler( const Any& /*window*/, const css::uno::Reference< XEventHandler >& handler, sal_Int32 /*eventMask*/ ) throw(std::exception)
{
    MutexGuard aGuard( m_aMutex );

    m_aHandlers.push_back( handler );
}

void SAL_CALL DisplayConnection::removeEventHandler( const Any& /*window*/, const css::uno::Reference< XEventHandler >& handler ) throw(std::exception)
{
    MutexGuard aGuard( m_aMutex );

    m_aHandlers.remove( handler );
}

void SAL_CALL DisplayConnection::addErrorHandler( const css::uno::Reference< XEventHandler >& handler ) throw(std::exception)
{
    MutexGuard aGuard( m_aMutex );

    m_aErrorHandlers.push_back( handler );
}

void SAL_CALL DisplayConnection::removeErrorHandler( const css::uno::Reference< XEventHandler >& handler ) throw(std::exception)
{
    MutexGuard aGuard( m_aMutex );

    m_aErrorHandlers.remove( handler );
}

Any SAL_CALL DisplayConnection::getIdentifier() throw(std::exception)
{
    return m_aAny;
}

bool DisplayConnection::dispatchEvent( void* pData, int nBytes )
{
    SolarMutexReleaser aRel;

    Sequence< sal_Int8 > aSeq( static_cast<sal_Int8*>(pData), nBytes );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
