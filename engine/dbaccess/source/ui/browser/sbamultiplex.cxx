/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <sbamultiplex.hxx>
#include <cppuhelper/queryinterface.hxx>

using namespace dbaui;

// the listener multiplexers

// XStatusListener
SbaXStatusMultiplexer::SbaXStatusMultiplexer(::cppu::OWeakObject& rSource, ::osl::Mutex& _rMutex)
    :OSbaWeakSubObject(rSource)
    ,OInterfaceContainerHelper3(_rMutex)
{
}

cpo::uno::Any  SAL_CALL SbaXStatusMultiplexer::queryInterface(const css::uno::Type& _rType)
{
    cpo::uno::Any aReturn = OSbaWeakSubObject::queryInterface(_rType);
    if (!aReturn.hasValue())
        aReturn = ::cppu::queryInterface(_rType,
            static_cast< css::frame::XStatusListener* >(this),
            static_cast< css::lang::XEventListener* >(static_cast< css::frame::XStatusListener* >(this))
        );

    return aReturn;
}
void SAL_CALL SbaXStatusMultiplexer::disposing(const css::lang::EventObject& )
{
}


void SAL_CALL SbaXStatusMultiplexer::statusChanged(const css::frame::FeatureStateEvent& e)
{
    m_aLastKnownStatus = e;
    m_aLastKnownStatus.Source = &m_rParent;
    notifyEach( &XStatusListener::statusChanged, m_aLastKnownStatus );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
