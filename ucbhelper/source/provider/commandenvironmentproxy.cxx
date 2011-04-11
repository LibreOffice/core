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
#include "precompiled_ucbhelper.hxx"

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/ucb/XContentIdentifierFactory.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <com/sun/star/ucb/XContentProviderManager.hpp>
#include <osl/mutex.hxx>
#include <ucbhelper/commandenvironmentproxy.hxx>

using namespace com::sun::star::lang;
using namespace com::sun::star::task;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;

namespace ucbhelper
{

//=========================================================================
//=========================================================================
//
// struct CommandEnvironmentProxy_Impl.
//
//=========================================================================
//=========================================================================

struct CommandEnvironmentProxy_Impl
{
    osl::Mutex                       m_aMutex;
    Reference< XCommandEnvironment > m_xEnv;
    Reference< XInteractionHandler > m_xInteractionHandler;
    Reference< XProgressHandler >    m_xProgressHandler;
    sal_Bool m_bGotInteractionHandler;
    sal_Bool m_bGotProgressHandler;

    CommandEnvironmentProxy_Impl(
        const Reference< XCommandEnvironment >& rxEnv )
    : m_xEnv( rxEnv ), m_bGotInteractionHandler( sal_False ),
      m_bGotProgressHandler( sal_False ) {}
};

//=========================================================================
//=========================================================================
//
// CommandEnvironmentProxy Implementation.
//
//=========================================================================
//=========================================================================

CommandEnvironmentProxy::CommandEnvironmentProxy(
                        const Reference< XCommandEnvironment >& rxEnv )
{
    m_pImpl = new CommandEnvironmentProxy_Impl( rxEnv );
}

//=========================================================================
// virtual
CommandEnvironmentProxy::~CommandEnvironmentProxy()
{
    delete m_pImpl;
}

//=========================================================================
//
// XInterface methods
//
//=========================================================================

XINTERFACE_IMPL_2( CommandEnvironmentProxy,
                   XTypeProvider,
                   XCommandEnvironment );

//=========================================================================
//
// XTypeProvider methods
//
//=========================================================================

XTYPEPROVIDER_IMPL_2( CommandEnvironmentProxy,
                      XTypeProvider,
                      XCommandEnvironment );

//=========================================================================
//
// XCommandEnvironemnt methods.
//
//=========================================================================

// virtual
Reference< XInteractionHandler > SAL_CALL
CommandEnvironmentProxy::getInteractionHandler()
    throw ( RuntimeException )
{
    if ( m_pImpl->m_xEnv.is() )
    {
        if ( !m_pImpl->m_bGotInteractionHandler )
        {
            osl::MutexGuard aGuard( m_pImpl->m_aMutex );
            if ( !m_pImpl->m_bGotInteractionHandler )
            {
                m_pImpl->m_xInteractionHandler
                                = m_pImpl->m_xEnv->getInteractionHandler();
                m_pImpl->m_bGotInteractionHandler = sal_True;
            }
        }
    }
    return m_pImpl->m_xInteractionHandler;
}

//=========================================================================
// virtual
Reference< XProgressHandler > SAL_CALL
CommandEnvironmentProxy::getProgressHandler()
    throw ( RuntimeException )
{
    if ( m_pImpl->m_xEnv.is() )
    {
        if ( !m_pImpl->m_bGotProgressHandler )
        {
            osl::MutexGuard aGuard( m_pImpl->m_aMutex );
            if ( !m_pImpl->m_bGotProgressHandler )
            {
                m_pImpl->m_xProgressHandler
                                = m_pImpl->m_xEnv->getProgressHandler();
                m_pImpl->m_bGotProgressHandler = sal_True;
            }
        }
    }
    return m_pImpl->m_xProgressHandler;
}

} /* namespace ucbhelper */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
