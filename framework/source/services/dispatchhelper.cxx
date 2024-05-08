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

#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>

#include <comphelper/profilezone.hxx>
#include <unotools/mediadescriptor.hxx>
#include <utility>
#include <vcl/threadex.hxx>
#include <cppuhelper/supportsservice.hxx>

namespace framework
{
// XInterface, XTypeProvider, XServiceInfo

OUString SAL_CALL DispatchHelper::getImplementationName()
{
    return u"com.sun.star.comp.framework.services.DispatchHelper"_ustr;
}

sal_Bool SAL_CALL DispatchHelper::supportsService(const OUString& sServiceName)
{
    return cppu::supportsService(this, sServiceName);
}

css::uno::Sequence<OUString> SAL_CALL DispatchHelper::getSupportedServiceNames()
{
    return { u"com.sun.star.frame.DispatchHelper"_ustr };
}

/** ctor.

    @param xSMGR    the global uno service manager, which can be used to create own needed services.
*/
DispatchHelper::DispatchHelper(css::uno::Reference<css::uno::XComponentContext> xContext)
    : m_xContext(std::move(xContext))
    , m_aBlockFlag(false)
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
    css::uno::Reference<css::util::XURLTransformer> xParser;
    /* SAFE { */
    {
        std::scoped_lock aReadLock(m_mutex);
        xParser = css::util::URLTransformer::create(m_xContext);
    }
    /* } SAFE */

    css::util::URL aURL;
    aURL.Complete = sURL;
    xParser->parseStrict(aURL);

    // search dispatcher
    css::uno::Reference<css::frame::XDispatch> xDispatch
        = xDispatchProvider->queryDispatch(aURL, sTargetFrameName, nSearchFlags);

    utl::MediaDescriptor aDescriptor(lArguments);
    bool bOnMainThread = aDescriptor.getUnpackedValueOrDefault(u"OnMainThread"_ustr, false);

    if (bOnMainThread)
        return vcl::solarthread::syncExecute([this, &xDispatch, &aURL, &lArguments]() {
            return executeDispatch(xDispatch, aURL, true, lArguments);
        });
    else
        return executeDispatch(xDispatch, aURL, true, lArguments);
}

const css::uno::Any&
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
        auto pArguments = aArguments.getArray();
        pArguments[nLength].Name = "SynchronMode";
        pArguments[nLength].Value <<= SyncronFlag;

        if (xNotifyDispatch.is())
        {
            // dispatch it with guaranteed notification
            // Here we can hope for a result ... instead of the normal dispatch.
            css::uno::Reference<css::frame::XDispatchResultListener> xListener(xTHIS,
                                                                               css::uno::UNO_QUERY);
            /* SAFE { */
            {
                std::scoped_lock aWriteLock(m_mutex);
                m_xBroadcaster = xNotifyDispatch;
                m_aBlockFlag = false;
            }
            /* } SAFE */

            // dispatch it and wait for a notification
            // TODO/MBA: waiting in main thread?!
            xNotifyDispatch->dispatchWithNotification(aURL, aArguments, xListener);

            std::unique_lock aWriteLock(m_mutex);
            m_aBlock.wait(aWriteLock, [this] { return m_aBlockFlag; }); // wait for result
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
    std::scoped_lock g(m_mutex);
    m_aResult <<= aResult;
    m_aBlockFlag = true;
    m_aBlock.notify_one();
    m_xBroadcaster.clear();
}

/** we have to release our broadcaster reference.

    @param aEvent
                describe the source of this event and MUST be our save broadcaster!
 */
void SAL_CALL DispatchHelper::disposing(const css::lang::EventObject&)
{
    std::scoped_lock g(m_mutex);
    m_aResult.clear();
    m_aBlockFlag = true;
    m_aBlock.notify_one();
    m_xBroadcaster.clear();
}
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
framework_DispatchHelper_get_implementation(css::uno::XComponentContext* context,
                                            css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new framework::DispatchHelper(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
