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

#include "PresenterFrameworkObserver.hxx"
#include <com/sun/star/lang/IllegalArgumentException.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;


namespace sdext { namespace presenter {

PresenterFrameworkObserver::PresenterFrameworkObserver (
    const css::uno::Reference<css::drawing::framework::XConfigurationController>&rxController,
    const OUString& rsEventName,
    const Predicate& rPredicate,
    const Action& rAction)
    : PresenterFrameworkObserverInterfaceBase(m_aMutex),
      mxConfigurationController(rxController),
      maPredicate(rPredicate),
      maAction(rAction)
{
    if ( ! mxConfigurationController.is())
        throw lang::IllegalArgumentException();

    if (mxConfigurationController->hasPendingRequests())
    {
        if (!rsEventName.isEmpty())
        {
            mxConfigurationController->addConfigurationChangeListener(
                this,
                rsEventName,
                Any());
        }
        mxConfigurationController->addConfigurationChangeListener(
            this,
            "ConfigurationUpdateEnd",
            Any());
    }
    else
    {
        rAction(maPredicate());
    }
}

PresenterFrameworkObserver::~PresenterFrameworkObserver()
{
}

void PresenterFrameworkObserver::RunOnUpdateEnd (
    const css::uno::Reference<css::drawing::framework::XConfigurationController>&rxController,
    const Action& rAction)
{
    new PresenterFrameworkObserver(
        rxController,
        OUString(),
        &PresenterFrameworkObserver::True,
        rAction);
}

bool PresenterFrameworkObserver::True()
{
    return true;
}

void SAL_CALL PresenterFrameworkObserver::disposing()
{
    if (maAction)
        maAction(false);
    Shutdown();
}

void PresenterFrameworkObserver::Shutdown()
{
    maAction = Action();
    maPredicate = Predicate();

    if (mxConfigurationController != nullptr)
    {
        mxConfigurationController->removeConfigurationChangeListener(this);
        mxConfigurationController = nullptr;
    }
}

void SAL_CALL PresenterFrameworkObserver::disposing (const lang::EventObject& rEvent)
    throw (RuntimeException, std::exception)
{
    if ( ! rEvent.Source.is())
        return;

    if (rEvent.Source == mxConfigurationController)
    {
        mxConfigurationController = nullptr;
        if (maAction)
            maAction(false);
    }
}

void SAL_CALL PresenterFrameworkObserver::notifyConfigurationChange (
    const ConfigurationChangeEvent& rEvent)
    throw (RuntimeException, std::exception)
{
    bool bDispose(false);

    Action aAction (maAction);
    Predicate aPredicate (maPredicate);
    if (rEvent.Type == "ConfigurationUpdateEnd")
    {
        Shutdown();
        aAction(aPredicate());
        bDispose = true;
    }
    else if (aPredicate())
    {
        Shutdown();
        aAction(true);
        bDispose = true;
    }

    if (bDispose)
    {
        maAction = nullptr;
        dispose();
    }
}

} }  // end of namespace ::sdext::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
