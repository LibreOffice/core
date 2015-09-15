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

#include <sal/config.h>

#include <boost/noncopyable.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/task/XInteractionHandler2.hpp>

#include "iahndl.hxx"
#include <comphelper/namedvaluecollection.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>

using namespace com::sun::star;

namespace {

class UUIInteractionHandler:
    public cppu::WeakImplHelper< com::sun::star::lang::XServiceInfo,
                                  com::sun::star::lang::XInitialization,
                                  com::sun::star::task::XInteractionHandler2 >,
    private boost::noncopyable
{
private:
    UUIInteractionHelper * m_pImpl;

public:
    explicit UUIInteractionHandler(com::sun::star::uno::Reference<
                  com::sun::star::uno::XComponentContext >
                  const & rxContext);

    virtual ~UUIInteractionHandler();

    virtual OUString SAL_CALL getImplementationName()
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL supportsService(OUString const &
                          rServiceName)
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual com::sun::star::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames()
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL
    initialize(
        com::sun::star::uno::Sequence< com::sun::star::uno::Any > const &
            rArguments)
        throw (com::sun::star::uno::Exception, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL
    handle(com::sun::star::uno::Reference<
           com::sun::star::task::XInteractionRequest > const &
       rRequest)
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL
        handleInteractionRequest(
            const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionRequest >& _Request
        )   throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
};

UUIInteractionHandler::UUIInteractionHandler(
    uno::Reference< uno::XComponentContext > const &
        rxContext)
        : m_pImpl(new UUIInteractionHelper(rxContext))
{
}

UUIInteractionHandler::~UUIInteractionHandler()
{
    delete m_pImpl;
}

OUString SAL_CALL UUIInteractionHandler::getImplementationName()
    throw (uno::RuntimeException, std::exception)
{
    return OUString("com.sun.star.comp.uui.UUIInteractionHandler");
}

sal_Bool SAL_CALL
UUIInteractionHandler::supportsService(OUString const & rServiceName)
    throw (uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL
UUIInteractionHandler::getSupportedServiceNames()
    throw (uno::RuntimeException, std::exception)
{
    uno::Sequence< OUString > aNames(3);
    aNames[0] = "com.sun.star.task.InteractionHandler";
    // added to indicate support for configuration.backend.MergeRecoveryRequest
    aNames[1] = "com.sun.star.configuration.backend.InteractionHandler";
    aNames[2] = "com.sun.star.uui.InteractionHandler";
    // for backwards compatibility
    return aNames;
}

void SAL_CALL
UUIInteractionHandler::initialize(
    uno::Sequence< uno::Any > const & rArguments)
    throw (uno::Exception, std::exception)
{
    uno::Reference<uno::XComponentContext> xContext = m_pImpl->getORB();
    delete m_pImpl;

    // The old-style InteractionHandler service supported a sequence of
    // PropertyValue, while the new-style service now uses constructors to pass
    // in Parent and Context values; for backwards compatibility, keep support
    // for a PropertyValue sequence, too:
    uno::Reference< awt::XWindow > xWindow;
    OUString aContext;
    if (!((rArguments.getLength() == 1 && (rArguments[0] >>= xWindow)) ||
          (rArguments.getLength() == 2 && (rArguments[0] >>= xWindow) &&
           (rArguments[1] >>= aContext))))
    {
        ::comphelper::NamedValueCollection aProperties( rArguments );
        if ( aProperties.has( "Parent" ) )
        {
            OSL_VERIFY( aProperties.get( "Parent" ) >>= xWindow );
        }
        if ( aProperties.has( "Context" ) )
        {
            OSL_VERIFY( aProperties.get( "Context" ) >>= aContext );
        }
    }

    m_pImpl = new UUIInteractionHelper(xContext, xWindow, aContext);
}

void SAL_CALL
UUIInteractionHandler::handle(
    uno::Reference< task::XInteractionRequest > const & rRequest)
    throw (uno::RuntimeException, std::exception)
{
    try
    {
        m_pImpl->handleRequest(rRequest);
    }
    catch (uno::RuntimeException const & ex)
    {
        throw uno::RuntimeException(ex.Message, *this);
    }
}

sal_Bool SAL_CALL UUIInteractionHandler::handleInteractionRequest(
    const uno::Reference< task::XInteractionRequest >& _Request ) throw ( uno::RuntimeException, std::exception )
{
    try
    {
        return m_pImpl->handleRequest( _Request );
    }
    catch (uno::RuntimeException const & ex)
    {
        throw uno::RuntimeException( ex.Message, *this );
    }
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_uui_UUIInteractionHandler_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new UUIInteractionHandler(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
