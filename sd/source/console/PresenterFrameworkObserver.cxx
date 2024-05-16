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
#include <utility>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;


namespace sdext::presenter {

PresenterFrameworkObserver::PresenterFrameworkObserver (
    css::uno::Reference<css::drawing::framework::XConfigurationController> xController,
    const Action& rAction)
    : PresenterFrameworkObserverInterfaceBase(m_aMutex),
      mxConfigurationController(std::move(xController)),
      maAction(rAction)
{
    if ( ! mxConfigurationController.is())
        throw lang::IllegalArgumentException();

    if (mxConfigurationController->hasPendingRequests())
    {
        mxConfigurationController->addConfigurationChangeListener(
            this,
            u"ConfigurationUpdateEnd"_ustr,
            Any());
    }
    else
    {
        rAction(true);
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
        rAction);
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
    if (mxConfigurationController != nullptr)
    {
        mxConfigurationController->removeConfigurationChangeListener(this);
        mxConfigurationController = nullptr;
    }
}

void SAL_CALL PresenterFrameworkObserver::disposing (const lang::EventObject& rEvent)
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
    const ConfigurationChangeEvent& /*rEvent*/)
{
    Action aAction(maAction);
    Shutdown();
    aAction(true);

    maAction = nullptr;
    dispose();
}

}  // end of namespace ::sdext::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
