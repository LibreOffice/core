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

#include "tools/SlotStateListener.hxx"
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>

#include <comphelper/processfactory.hxx>

using namespace ::com::sun::star;

namespace sd { namespace tools {

SlotStateListener::SlotStateListener (
    Link<const OUString&,void>& rCallback,
    const uno::Reference<frame::XDispatchProvider>& rxDispatchProvider,
    const OUString& rSlotName)
    : SlotStateListenerInterfaceBase(maMutex),
      maCallback(),
      mxDispatchProviderWeak(nullptr)
{
    SetCallback(rCallback);
    ConnectToDispatchProvider(rxDispatchProvider);
    ObserveSlot(rSlotName);
}

SlotStateListener::~SlotStateListener()
{
    ReleaseListeners();
}

void SlotStateListener::SetCallback (const Link<const OUString&,void>& rCallback)
{
    ThrowIfDisposed();

    maCallback = rCallback;
}

void SlotStateListener::ConnectToDispatchProvider (
    const uno::Reference<frame::XDispatchProvider>& rxDispatchProvider)
{
    ThrowIfDisposed();

    // When we are listening to state changes of slots of another frame then
    // release these listeners first.
    if ( ! maRegisteredURLList.empty())
        ReleaseListeners();

    mxDispatchProviderWeak = rxDispatchProvider;
}

void SlotStateListener::ObserveSlot (const OUString& rSlotName)
{
    ThrowIfDisposed();

    if (maCallback.IsSet())
    {
        // Connect the state change listener.
        util::URL aURL (MakeURL(rSlotName));
        uno::Reference<frame::XDispatch> xDispatch (GetDispatch(aURL));
        if (xDispatch.is())
        {
            maRegisteredURLList.push_back(aURL);
            xDispatch->addStatusListener(this,aURL);
        }
    }
}

void SlotStateListener::disposing()
{
    ReleaseListeners();
    mxDispatchProviderWeak = uno::WeakReference<frame::XDispatchProvider>(nullptr);
    maCallback = Link<const OUString&,void>();
}

util::URL SlotStateListener::MakeURL (const OUString& rSlotName)
{
    util::URL aURL;
    aURL.Complete = rSlotName;

    uno::Reference<util::XURLTransformer> xTransformer(util::URLTransformer::create(::comphelper::getProcessComponentContext()));
    xTransformer->parseStrict(aURL);

    return aURL;
}

uno::Reference<frame::XDispatch>
    SlotStateListener::GetDispatch (const util::URL& rURL) const
{
    uno::Reference<frame::XDispatch> xDispatch;

    uno::Reference<frame::XDispatchProvider> xDispatchProvider (mxDispatchProviderWeak);
    if (xDispatchProvider.is())
        xDispatch = xDispatchProvider->queryDispatch(rURL, OUString(), 0);

    return xDispatch;
}

void SlotStateListener::statusChanged (
    const frame::FeatureStateEvent& rState)
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    OUString sSlotName (rState.FeatureURL.Complete);
    maCallback.Call(sSlotName);
}

void SlotStateListener::ReleaseListeners()
{
    if ( ! maRegisteredURLList.empty())
    {
        RegisteredURLList::iterator iURL (maRegisteredURLList.begin());
        RegisteredURLList::iterator iEnd (maRegisteredURLList.end());
        for (; iURL!=iEnd; ++iURL)
        {
            uno::Reference<frame::XDispatch> xDispatch (GetDispatch(*iURL));
            if (xDispatch.is())
            {
                xDispatch->removeStatusListener(this,*iURL);
            }
        }
    }
}

//=====  lang::XEventListener  ================================================

void SAL_CALL SlotStateListener::disposing (
    const lang::EventObject& )
    throw (uno::RuntimeException, std::exception)
{
}

void SlotStateListener::ThrowIfDisposed()
    throw (lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException ("SlideSorterController object has already been disposed",
            static_cast<uno::XWeak*>(this));
    }
}

} } // end of namespace ::sd::tools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
