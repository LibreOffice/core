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

#include <vcl/svapp.hxx>
#include <tools/debug.hxx>

#include <displayconnectiondispatch.hxx>
#include <svdata.hxx>
#include <salinst.hxx>

using namespace osl;
using namespace vcl;
using namespace com::sun::star::uno;
using namespace com::sun::star::awt;

DisplayConnectionDispatch::DisplayConnectionDispatch()
{
    m_ConnectionIdentifier = ImplGetSVData()->mpDefInst->GetConnectionIdentifier();
}

DisplayConnectionDispatch::~DisplayConnectionDispatch()
{}

void DisplayConnectionDispatch::start()
{
    DBG_TESTSOLARMUTEX();
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->mpDefInst->SetEventCallback( this );
}

void DisplayConnectionDispatch::terminate()
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
    std::vector< css::uno::Reference< XEventHandler > > aLocalList( m_aHandlers );
    for (auto const& elem : aLocalList)
        elem->handleEvent( aEvent );
}

void SAL_CALL DisplayConnectionDispatch::addEventHandler( const Any& /*window*/, const css::uno::Reference< XEventHandler >& handler, sal_Int32 /*eventMask*/ )
{
    MutexGuard aGuard( m_aMutex );

    m_aHandlers.push_back( handler );
}

void SAL_CALL DisplayConnectionDispatch::removeEventHandler( const Any& /*window*/, const css::uno::Reference< XEventHandler >& handler )
{
    MutexGuard aGuard( m_aMutex );

    m_aHandlers.erase( std::remove(m_aHandlers.begin(), m_aHandlers.end(), handler), m_aHandlers.end() );
}

void SAL_CALL DisplayConnectionDispatch::addErrorHandler( const css::uno::Reference< XEventHandler >& )
{
}

void SAL_CALL DisplayConnectionDispatch::removeErrorHandler( const css::uno::Reference< XEventHandler >& )
{
}

Any SAL_CALL DisplayConnectionDispatch::getIdentifier()
{
    return Any(m_ConnectionIdentifier);
}

bool DisplayConnectionDispatch::dispatchEvent( void const * pData, int nBytes )
{
    SolarMutexReleaser aRel;

    Sequence< sal_Int8 > aSeq( static_cast<const sal_Int8*>(pData), nBytes );
    Any aEvent;
    aEvent <<= aSeq;
    ::std::vector< css::uno::Reference< XEventHandler > > handlers;
    {
        MutexGuard aGuard( m_aMutex );
        handlers = m_aHandlers;
    }
    for (auto const& handle : handlers)
        if( handle->handleEvent( aEvent ) )
            return true;
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
