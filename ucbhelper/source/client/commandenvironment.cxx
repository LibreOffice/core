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
#include <ucbhelper/commandenvironment.hxx>

using namespace com::sun::star::lang;
using namespace com::sun::star::task;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;

namespace ucbhelper
{

//=========================================================================
//=========================================================================
//
// struct CommandEnvironment_Impl.
//
//=========================================================================
//=========================================================================

struct CommandEnvironment_Impl
{
    Reference< XInteractionHandler > m_xInteractionHandler;
    Reference< XProgressHandler >    m_xProgressHandler;

    CommandEnvironment_Impl(
        const Reference< XInteractionHandler >& rxInteractionHandler,
        const Reference< XProgressHandler >& rxProgressHandler )
    : m_xInteractionHandler( rxInteractionHandler ),
      m_xProgressHandler( rxProgressHandler ) {}
};

//=========================================================================
//=========================================================================
//
// CommandEnvironment Implementation.
//
//=========================================================================
//=========================================================================

CommandEnvironment::CommandEnvironment(
        const Reference< XInteractionHandler >& rxInteractionHandler,
        const Reference< XProgressHandler >& rxProgressHandler )
{
    m_pImpl = new CommandEnvironment_Impl( rxInteractionHandler,
                                           rxProgressHandler );
}

//=========================================================================
// virtual
CommandEnvironment::~CommandEnvironment()
{
    delete m_pImpl;
}

//=========================================================================
//
// XInterface methods
//
//=========================================================================

XINTERFACE_IMPL_2( CommandEnvironment,
                   XTypeProvider,
                   XCommandEnvironment );

//=========================================================================
//
// XTypeProvider methods
//
//=========================================================================

XTYPEPROVIDER_IMPL_2( CommandEnvironment,
                      XTypeProvider,
                      XCommandEnvironment );

//=========================================================================
//
// XCommandEnvironemnt methods.
//
//=========================================================================

// virtual
Reference< XInteractionHandler > SAL_CALL
CommandEnvironment::getInteractionHandler()
    throw ( RuntimeException )
{
    return m_pImpl->m_xInteractionHandler;
}

//=========================================================================
// virtual
Reference< XProgressHandler > SAL_CALL
CommandEnvironment::getProgressHandler()
    throw ( RuntimeException )
{
    return m_pImpl->m_xProgressHandler;
}

} /* namespace ucbhelper */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
