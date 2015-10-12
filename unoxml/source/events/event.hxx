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

#ifndef INCLUDED_UNOXML_SOURCE_EVENTS_EVENT_HXX
#define INCLUDED_UNOXML_SOURCE_EVENTS_EVENT_HXX

#include <sal/types.h>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/xml/dom/events/XEvent.hpp>
#include <com/sun/star/xml/dom/events/XEventTarget.hpp>
#include <com/sun/star/util/Time.hpp>

#include <cppuhelper/implbase.hxx>

#include "../dom/node.hxx"

namespace DOM {namespace events
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

    virtual ~CEvent();
    virtual OUString SAL_CALL getType() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::xml::dom::events::XEventTarget > SAL_CALL getTarget() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::xml::dom::events::XEventTarget > SAL_CALL getCurrentTarget() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::xml::dom::events::PhaseType SAL_CALL getEventPhase() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getBubbles() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getCancelable() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::util::Time SAL_CALL getTimeStamp() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL stopPropagation() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL preventDefault() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL initEvent(
        const OUString& eventTypeArg,
        sal_Bool canBubbleArg,
        sal_Bool cancelableArg)  throw (css::uno::RuntimeException, std::exception) override;
};
}}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
