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



namespace uno           = ::com::sun::star::uno;
namespace lang          = ::com::sun::star::lang;
using uno::UNO_QUERY;



// must be aligned with configmgr/source/misc/configinteractionhandler
static char const CONFIG_ERROR_HANDLER[] = "configuration.interaction-handler";



// ConfigurationErrorHandler


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


ConfigurationErrorHandler::~ConfigurationErrorHandler()
{
    deactivate();
}


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


ConfigurationErrorHandler::InteractionHandler ConfigurationErrorHandler::getDefaultInteractionHandler()
{
    uno::Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
    InteractionHandler xHandler( com::sun::star::task::InteractionHandler::createWithParent(xContext, 0), UNO_QUERY );
    return xHandler;
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
