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
#include "precompiled_framework.hxx"

//_______________________________________________
// my own includes
#include <dispatch/loaddispatcher.hxx>
#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>

//_______________________________________________
// interface includes
#include <com/sun/star/frame/DispatchResultState.hpp>

//_______________________________________________
// includes of other projects

//_______________________________________________
// namespace

namespace framework{

namespace css = ::com::sun::star;

//_______________________________________________
// declarations

/*-----------------------------------------------
    20.08.2003 09:52
-----------------------------------------------*/
LoadDispatcher::LoadDispatcher(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR       ,
                               const css::uno::Reference< css::frame::XFrame >&              xOwnerFrame ,
                               const ::rtl::OUString                                         sTargetName ,
                                     sal_Int32                                               nSearchFlags)
    : ThreadHelpBase(            )
    , m_xSMGR       (xSMGR       )
    , m_xOwnerFrame (xOwnerFrame )
    , m_sTarget     (sTargetName )
    , m_nSearchFlags(nSearchFlags)
    , m_aLoader     (xSMGR       )
{
}

/*-----------------------------------------------
    20.08.2003 09:12
-----------------------------------------------*/
LoadDispatcher::~LoadDispatcher()
{
    m_xSMGR.clear();
}

/*-----------------------------------------------
    20.08.2003 09:58
-----------------------------------------------*/
void SAL_CALL LoadDispatcher::dispatchWithNotification(const css::util::URL&                                             aURL      ,
                                                       const css::uno::Sequence< css::beans::PropertyValue >&            lArguments,
                                                       const css::uno::Reference< css::frame::XDispatchResultListener >& xListener )
    throw(css::uno::RuntimeException)
{
    impl_dispatch( aURL, lArguments, xListener );
}

/*-----------------------------------------------
    20.08.2003 09:16
-----------------------------------------------*/
void SAL_CALL LoadDispatcher::dispatch(const css::util::URL&                                  aURL      ,
                                       const css::uno::Sequence< css::beans::PropertyValue >& lArguments)
    throw(css::uno::RuntimeException)
{
    impl_dispatch( aURL, lArguments, css::uno::Reference< css::frame::XDispatchResultListener >() );
}

/*-----------------------------------------------
    14.04.2008
-----------------------------------------------*/
css::uno::Any SAL_CALL LoadDispatcher::dispatchWithReturnValue( const css::util::URL& rURL,
                                                                const css::uno::Sequence< css::beans::PropertyValue >& lArguments )
    throw( css::uno::RuntimeException )
{
    return impl_dispatch( rURL, lArguments, css::uno::Reference< css::frame::XDispatchResultListener >());
}

/*-----------------------------------------------
    20.08.2003 10:48
-----------------------------------------------*/
void SAL_CALL LoadDispatcher::addStatusListener(const css::uno::Reference< css::frame::XStatusListener >& /*xListener*/,
                                                const css::util::URL&                                     /*aURL*/     )
    throw(css::uno::RuntimeException)
{
}

/*-----------------------------------------------
    20.08.2003 10:49
-----------------------------------------------*/
void SAL_CALL LoadDispatcher::removeStatusListener(const css::uno::Reference< css::frame::XStatusListener >& /*xListener*/,
                                                   const css::util::URL&                                     /*aURL*/     )
    throw(css::uno::RuntimeException)
{
}

/*-----------------------------------------------
    20.08.2003 09:58
-----------------------------------------------*/
css::uno::Any LoadDispatcher::impl_dispatch( const css::util::URL& rURL,
                                             const css::uno::Sequence< css::beans::PropertyValue >& lArguments,
                                             const css::uno::Reference< css::frame::XDispatchResultListener >& xListener )
{
    // Attention: May be nobody outside hold such temp. dispatch object alive (because
    // the container in which we resists isnt implemented threadsafe but updated by a timer
    // and clear our reference ...) we should hold us self alive!
    css::uno::Reference< css::uno::XInterface > xThis(static_cast< css::frame::XNotifyingDispatch* >(this), css::uno::UNO_QUERY);

    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);

    // We are the only client of this load env object ... but
    // may a dispatch request before is still in progress (?!).
    // Then we should wait a little bit and block this new request.
    // In case we run into the timeout, we should reject this new request
    // and return "FAILED" as result. Otherwhise we can start this new operation.
    if (!m_aLoader.waitWhileLoading(2000)) // => 2 sec.
    {
        if (xListener.is())
            xListener->dispatchFinished(
                css::frame::DispatchResultEvent(xThis, css::frame::DispatchResultState::DONTKNOW, css::uno::Any())); // DONTKNOW? ... not realy started ... not realy failed :-)
    }

    css::uno::Reference< css::frame::XFrame > xBaseFrame(m_xOwnerFrame.get(), css::uno::UNO_QUERY);
    if (!xBaseFrame.is())
    {
        if (xListener.is())
            xListener->dispatchFinished(
                css::frame::DispatchResultEvent(xThis, css::frame::DispatchResultState::FAILURE, css::uno::Any()));
    }

    // OK ... now the internal loader seems to be useable for new requests
    // and our owner frame seems to be valid for such operations.
    // Initialize it with all new but needed properties and start the loading.
    css::uno::Reference< css::lang::XComponent > xComponent;
    try
    {
        m_aLoader.initializeLoading( rURL.Complete, lArguments, xBaseFrame, m_sTarget, m_nSearchFlags, (LoadEnv::EFeature)(LoadEnv::E_ALLOW_CONTENTHANDLER | LoadEnv::E_WORK_WITH_UI));
        m_aLoader.startLoading();
        m_aLoader.waitWhileLoading(); // wait for ever!
        xComponent = m_aLoader.getTargetComponent();

        // TODO thinking about asynchronous operations and listener support
    }
    catch(const LoadEnvException&)
        { xComponent.clear(); }

    if (xListener.is())
    {
        if (xComponent.is())
            xListener->dispatchFinished(
                css::frame::DispatchResultEvent(xThis, css::frame::DispatchResultState::SUCCESS, css::uno::Any()));
        else
            xListener->dispatchFinished(
                css::frame::DispatchResultEvent(xThis, css::frame::DispatchResultState::FAILURE, css::uno::Any()));
    }

    // return the model - like loadComponentFromURL()
    css::uno::Any aRet;
    if ( xComponent.is () )
        aRet = css::uno::makeAny( xComponent );

    aReadLock.unlock();
    // <- SAFE ----------------------------------
    return aRet;
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
