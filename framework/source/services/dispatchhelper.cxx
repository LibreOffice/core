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

#include <framework/dispatchhelper.hxx>
#include <macros/xserviceinfo.hxx>
#include <services.h>

#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>

#include <comphelper/profilezone.hxx>
#include <unotools/mediadescriptor.hxx>
#include <vcl/threadex.hxx>

namespace framework
{
// XInterface, XTypeProvider, XServiceInfo

DEFINE_XSERVICEINFO_MULTISERVICE_2(DispatchHelper, ::cppu::OWeakObject,
                                   "com.sun.star.frame.DispatchHelper",
                                   IMPLEMENTATIONNAME_DISPATCHHELPER)

DEFINE_INIT_SERVICE(DispatchHelper, {})

/** ctor.

    @param xSMGR    the global uno service manager, which can be used to create own needed services.
*/
DispatchHelper::DispatchHelper(const css::uno::Reference<css::uno::XComponentContext>& xContext)
    : m_xContext(xContext)
{
}

/** dtor.
*/
DispatchHelper::~DispatchHelper() {}

/** capsulate all steps of a dispatch request and provide so an easy way for dispatches.

    @param xDispatchProvider
                identifies the object, which provides may be valid dispatch objects for this execute.

    @param sURL
                describes the requested feature.

    @param sTargetFrameName
                points to the frame, which must be used (or may be created) for this dispatch.

    @param nSearchFlags
                in case the <var>sTargetFrameName</var> isn't unique, these flags regulate further searches.

    @param lArguments
                optional arguments for this request.

    @return An Any which capsulate a possible result of the internal wrapped dispatch.
 */
css::uno::Any SAL_CALL DispatchHelper::executeDispatch(
    const css::uno::Reference<css::frame::XDispatchProvider>& xDispatchProvider,
    const OUString& sURL, const OUString& sTargetFrameName, sal_Int32 nSearchFlags,
    const css::uno::Sequence<css::beans::PropertyValue>& lArguments)
{
    // check for valid parameters
    if ((!xDispatchProvider.is()) || (!m_xContext.is()) || (sURL.isEmpty()))
    {
        return css::uno::Any();
    }

    // parse given URL
    /* SAFE { */
    osl::ClearableMutexGuard aReadLock(m_mutex);
    css::uno::Reference<css::util::XURLTransformer> xParser
        = css::util::URLTransformer::create(m_xContext);
    aReadLock.clear();
    /* } SAFE */

    css::util::URL aURL;
    aURL.Complete = sURL;
    xParser->parseStrict(aURL);

    // search dispatcher
    css::uno::Reference<css::frame::XDispatch> xDispatch
        = xDispatchProvider->queryDispatch(aURL, sTargetFrameName, nSearchFlags);

    utl::MediaDescriptor aDescriptor(lArguments);
    bool bOnMainThread = aDescriptor.getUnpackedValueOrDefault("OnMainThread", false);

    if (bOnMainThread)
        return vcl::solarthread::syncExecute([this, &xDispatch, &aURL, &lArguments]() {
            return executeDispatch(xDispatch, aURL, true, lArguments);
        });
    else
        return executeDispatch(xDispatch, aURL, true, lArguments);
}

css::uno::Any
DispatchHelper::executeDispatch(const css::uno::Reference<css::frame::XDispatch>& xDispatch,
                                const css::util::URL& aURL, bool SyncronFlag,
                                const css::uno::Sequence<css::beans::PropertyValue>& lArguments)
{
    comphelper::ProfileZone aZone("executeDispatch");
    css::uno::Reference<css::uno::XInterface> xTHIS(static_cast<::cppu::OWeakObject*>(this),
                                                    css::uno::UNO_QUERY);
    m_aResult.clear();

    // check for valid parameters
    if (xDispatch.is())
    {
        css::uno::Reference<css::frame::XNotifyingDispatch> xNotifyDispatch(xDispatch,
                                                                            css::uno::UNO_QUERY);

        // make sure that synchronous execution is used (if possible)
        css::uno::Sequence<css::beans::PropertyValue> aArguments(lArguments);
        sal_Int32 nLength = lArguments.getLength();
        aArguments.realloc(nLength + 1);
        aArguments[nLength].Name = "SynchronMode";
        aArguments[nLength].Value <<= SyncronFlag;

        if (xNotifyDispatch.is())
        {
            // dispatch it with guaranteed notification
            // Here we can hope for a result ... instead of the normal dispatch.
            css::uno::Reference<css::frame::XDispatchResultListener> xListener(xTHIS,
                                                                               css::uno::UNO_QUERY);
            /* SAFE { */
            osl::ClearableMutexGuard aWriteLock(m_mutex);
            m_xBroadcaster.set(xNotifyDispatch, css::uno::UNO_QUERY);
            m_aBlock.reset();
            aWriteLock.clear();
            /* } SAFE */

            // dispatch it and wait for a notification
            // TODO/MBA: waiting in main thread?!
            xNotifyDispatch->dispatchWithNotification(aURL, aArguments, xListener);
            m_aBlock.wait(); // wait for result
        }
        else
        {
            // dispatch it without any chance to get a result
            xDispatch->dispatch(aURL, aArguments);
        }
    }

    return m_aResult;
}

/** callback for started dispatch with guaranteed notifications.

    We must save the result, so the method executeDispatch() can return it.
    Further we must release the broadcaster (otherwise it can't die)
    and unblock the waiting executeDispatch() request.

    @param  aResult
                describes the result of the dispatch operation
 */
void SAL_CALL DispatchHelper::dispatchFinished(const css::frame::DispatchResultEvent& aResult)
{
    osl::MutexGuard g(m_mutex);
    m_aResult <<= aResult;
    m_aBlock.set();
    m_xBroadcaster.clear();
}

/** we have to release our broadcaster reference.

    @param aEvent
                describe the source of this event and MUST be our save broadcaster!
 */
void SAL_CALL DispatchHelper::disposing(const css::lang::EventObject&)
{
    osl::MutexGuard g(m_mutex);
    m_aResult.clear();
    m_aBlock.set();
    m_xBroadcaster.clear();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
