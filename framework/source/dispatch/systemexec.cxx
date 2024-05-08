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

#include <dispatch/systemexec.hxx>
#include <services.h>

#include <com/sun/star/system/SystemShellExecute.hpp>
#include <com/sun/star/util/PathSubstitution.hpp>
#include <com/sun/star/util/XStringSubstitution.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/frame/DispatchResultState.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <utility>

namespace framework{

constexpr OUString PROTOCOL_VALUE = u"systemexecute:"_ustr;

// XInterface, XTypeProvider, XServiceInfo

OUString SAL_CALL SystemExec::getImplementationName()
{
    return u"com.sun.star.comp.framework.SystemExecute"_ustr;
}

sal_Bool SAL_CALL SystemExec::supportsService( const OUString& sServiceName )
{
    return cppu::supportsService(this, sServiceName);
}

css::uno::Sequence< OUString > SAL_CALL SystemExec::getSupportedServiceNames()
{
    return { SERVICENAME_PROTOCOLHANDLER };
}

SystemExec::SystemExec( css::uno::Reference< css::uno::XComponentContext > xContext )
        : m_xContext    (std::move( xContext                     ))
{
}

SystemExec::~SystemExec()
{
}

css::uno::Reference< css::frame::XDispatch > SAL_CALL SystemExec::queryDispatch( const css::util::URL&  aURL    ,
                                                                                 const OUString&,
                                                                                       sal_Int32 )
{
    css::uno::Reference< css::frame::XDispatch > xDispatcher;
    if (aURL.Complete.startsWith(PROTOCOL_VALUE))
        xDispatcher = this;
    return xDispatcher;
}

css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL SystemExec::queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& lDescriptor )
{
    sal_Int32 nCount = lDescriptor.getLength();
    css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > lDispatcher( nCount );
    auto lDispatcherRange = asNonConstRange(lDispatcher);
    for( sal_Int32 i=0; i<nCount; ++i )
    {
        lDispatcherRange[i] = queryDispatch(
                            lDescriptor[i].FeatureURL,
                            lDescriptor[i].FrameName,
                            lDescriptor[i].SearchFlags);
    }
    return lDispatcher;
}

void SAL_CALL SystemExec::dispatch( const css::util::URL&                                  aURL       ,
                                    const css::uno::Sequence< css::beans::PropertyValue >& lArguments )
{
    dispatchWithNotification(aURL, lArguments, css::uno::Reference< css::frame::XDispatchResultListener >());
}

void SAL_CALL SystemExec::dispatchWithNotification( const css::util::URL&                                             aURL      ,
                                                    const css::uno::Sequence< css::beans::PropertyValue >&,
                                                    const css::uno::Reference< css::frame::XDispatchResultListener >& xListener )
{
    // convert "systemexec:file:///c:/temp/test.html" => "file:///c:/temp/test.html"
    sal_Int32 c = aURL.Complete.getLength()-PROTOCOL_VALUE.getLength();
    if (c<1) // we don't check for valid URLs here! The system will show an error message ...
    {
        impl_notifyResultListener(xListener, css::frame::DispatchResultState::FAILURE);
        return;
    }
    OUString sSystemURLWithVariables = aURL.Complete.copy(PROTOCOL_VALUE.getLength(), c);

    // TODO check security settings ...

    try
    {
        css::uno::Reference< css::util::XStringSubstitution > xPathSubst( css::util::PathSubstitution::create(m_xContext) );

        OUString sSystemURL = xPathSubst->substituteVariables(sSystemURLWithVariables, true); // sal_True force an exception if unknown variables exists !

        css::uno::Reference< css::system::XSystemShellExecute > xShell = css::system::SystemShellExecute::create( m_xContext );

        xShell->execute(sSystemURL, OUString(), css::system::SystemShellExecuteFlags::URIS_ONLY);
        impl_notifyResultListener(xListener, css::frame::DispatchResultState::SUCCESS);
    }
    catch(const css::uno::Exception&)
        {
            impl_notifyResultListener(xListener, css::frame::DispatchResultState::FAILURE);
        }
}

void SAL_CALL SystemExec::addStatusListener( const css::uno::Reference< css::frame::XStatusListener >&,
                                             const css::util::URL& )
{
    // not supported yet
}

void SAL_CALL SystemExec::removeStatusListener( const css::uno::Reference< css::frame::XStatusListener >&,
                                                const css::util::URL& )
{
    // not supported yet
}

void SystemExec::impl_notifyResultListener(const css::uno::Reference< css::frame::XDispatchResultListener >& xListener,
                                           const sal_Int16                                                   nState   )
{
    if (xListener.is())
    {
        css::frame::DispatchResultEvent aEvent;
        aEvent.State = nState;
        xListener->dispatchFinished(aEvent);
    }
}

}       //  namespace framework

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
framework_SystemExecute_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& )
{
    return cppu::acquire(new framework::SystemExec(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
