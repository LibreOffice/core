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

#pragma once

#include <sal/types.h>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/xml/dom/events/XEvent.hpp>
#include <com/sun/star/xml/dom/events/XEventTarget.hpp>
#include <com/sun/star/util/Time.hpp>

#include <cppuhelper/implbase.hxx>

namespace DOM::events
{
class CEvent : public cppu::WeakImplHelper< css::xml::dom::events::XEvent >
{
friend class CEventDispatcher;

protected:
    ::osl::Mutex m_Mutex;
    bool m_canceled;
    OUString m_eventType;
    css::uno::Reference< css::xml::dom::events::XEventTarget > m_target;
    css::uno::Reference< css::xml::dom::events::XEventTarget > m_currentTarget;
    css::xml::dom::events::PhaseType m_phase;
    bool m_bubbles;
    bool m_cancelable;
    css::util::Time m_time;

public:

    explicit CEvent();

    virtual ~CEvent() override;
    virtual OUString SAL_CALL getType() override;
    virtual css::uno::Reference< css::xml::dom::events::XEventTarget > SAL_CALL getTarget() override;
    virtual css::uno::Reference< css::xml::dom::events::XEventTarget > SAL_CALL getCurrentTarget() override;
    virtual css::xml::dom::events::PhaseType SAL_CALL getEventPhase() override;
    virtual sal_Bool SAL_CALL getBubbles() override;
    virtual sal_Bool SAL_CALL getCancelable() override;
    virtual css::util::Time SAL_CALL getTimeStamp() override;
    virtual void SAL_CALL stopPropagation() override;
    virtual void SAL_CALL preventDefault() override;
    virtual void SAL_CALL initEvent(
        const OUString& eventTypeArg,
        sal_Bool canBubbleArg,
        sal_Bool cancelableArg) override;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
