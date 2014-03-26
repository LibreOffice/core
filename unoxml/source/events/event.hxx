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

#include <cppuhelper/implbase1.hxx>

#include "../dom/node.hxx"


using namespace com::sun::star::uno;
using namespace com::sun::star::xml::dom;
using namespace com::sun::star::xml::dom::events;


namespace DOM {namespace events
{
class CEvent : public cppu::WeakImplHelper1< XEvent >
{
friend class CEventDispatcher;

protected:
    ::osl::Mutex m_Mutex;
    sal_Bool m_canceled;
    OUString m_eventType;
    Reference< XEventTarget > m_target;
    Reference< XEventTarget > m_currentTarget;
    PhaseType m_phase;
    sal_Bool m_bubbles;
    sal_Bool m_cancelable;
    com::sun::star::util::Time m_time;

public:

    explicit CEvent();

    virtual ~CEvent();
    virtual OUString SAL_CALL getType() throw (RuntimeException, std::exception) SAL_OVERRIDE;
    virtual Reference< XEventTarget > SAL_CALL getTarget() throw (RuntimeException, std::exception) SAL_OVERRIDE;
    virtual Reference< XEventTarget > SAL_CALL getCurrentTarget() throw (RuntimeException, std::exception) SAL_OVERRIDE;
    virtual PhaseType SAL_CALL getEventPhase() throw (RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL getBubbles() throw (RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL getCancelable() throw (RuntimeException, std::exception) SAL_OVERRIDE;
    virtual com::sun::star::util::Time SAL_CALL getTimeStamp() throw (RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL stopPropagation() throw (RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL preventDefault() throw (RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL initEvent(
        const OUString& eventTypeArg,
        sal_Bool canBubbleArg,
        sal_Bool cancelableArg)  throw (RuntimeException, std::exception) SAL_OVERRIDE;
};
}}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
