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

#include "configinit.hxx"

#include "desktop.hrc"
#include "app.hxx"
#include <comphelper/processfactory.hxx>
#include <uno/current_context.hxx>
#include <cppuhelper/implbase1.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>
#include <stdio.h>
#include <com/sun/star/task/InteractionHandler.hpp>

// ----------------------------------------------------------------------------

namespace uno           = ::com::sun::star::uno;
namespace lang          = ::com::sun::star::lang;
using rtl::OUString;
using uno::UNO_QUERY;

// ----------------------------------------------------------------------------

// must be aligned with configmgr/source/misc/configinteractionhandler
static char const CONFIG_ERROR_HANDLER[] = "configuration.interaction-handler";
// ----------------------------------------------------------------------------

#define OUSTRING( constascii ) OUString( RTL_CONSTASCII_USTRINGPARAM( constascii ) )

#define k_ERRORHANDLER OUSTRING( CONFIGURATION_ERROR_HANDLER )

// ----------------------------------------------------------------------------
// ConfigurationErrorHandler
// ----------------------------------------------------------------------------

namespace
{
    typedef uno::Reference< uno::XCurrentContext > CurrentContext;
    class SimpleCurrentContext : public cppu::WeakImplHelper1< uno::XCurrentContext >
    {
        CurrentContext m_xChainedContext;
    public:
        explicit
        SimpleCurrentContext(const CurrentContext & xChainedContext)
        : m_xChainedContext(xChainedContext)
        {}

        void install()      { uno::setCurrentContext(this); }
        void deinstall()    { uno::setCurrentContext(m_xChainedContext); }

        uno::Any getChainedValueByName( OUString const & aName) const
        {
            return m_xChainedContext.is()
                            ? m_xChainedContext->getValueByName(aName)
                            : uno::Any();
        }

        // XCurrentContext
        virtual uno::Any SAL_CALL
            getValueByName( OUString const & aName)
                throw (uno::RuntimeException);
    };

    uno::Any SAL_CALL
        SimpleCurrentContext::getValueByName( OUString const & aName)
            throw (uno::RuntimeException)
    {
        return getChainedValueByName(aName);
    }

}

// ----------------------------------------------------------------------------
class ConfigurationErrorHandler::Context : public SimpleCurrentContext
{
public:
    Context()
    : SimpleCurrentContext( uno::getCurrentContext() )
    {
    }

    ~Context()
    {
    }

    // XCurrentContext
    virtual uno::Any SAL_CALL
        getValueByName( OUString const & aName)
            throw (uno::RuntimeException);

private:
    InteractionHandler  m_xHandler;
};

//------------------------------------------------------------------------------
uno::Any SAL_CALL ConfigurationErrorHandler::Context::getValueByName( OUString const & aName)
        throw (uno::RuntimeException)
{
    if ( aName == CONFIG_ERROR_HANDLER )
    {
        if ( !m_xHandler.is() )
            m_xHandler = ConfigurationErrorHandler::getDefaultInteractionHandler();
        return uno::Any( m_xHandler );
    }
    return SimpleCurrentContext::getValueByName( aName );
}

//------------------------------------------------------------------------------
ConfigurationErrorHandler::~ConfigurationErrorHandler()
{
    deactivate();
}

//------------------------------------------------------------------------------
/// installs the handler into the current context
void ConfigurationErrorHandler::activate()
{
    if (!m_pContext)
    {
        m_pContext = new Context;
        m_pContext->acquire();
    }
    m_pContext->install();
}

//------------------------------------------------------------------------------
/// deinstalls the handler from the current context, restoring the previous context
void ConfigurationErrorHandler::deactivate()
{
    if (m_pContext)
    {
        m_pContext->deinstall();
        m_pContext->release();
        m_pContext = 0;
    }
}
//------------------------------------------------------------------------------

ConfigurationErrorHandler::InteractionHandler ConfigurationErrorHandler::getDefaultInteractionHandler()
{
    uno::Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
    InteractionHandler xHandler( com::sun::star::task::InteractionHandler::createDefault(xContext), UNO_QUERY_THROW );
    return xHandler;
}
//------------------------------------------------------------------------------



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
