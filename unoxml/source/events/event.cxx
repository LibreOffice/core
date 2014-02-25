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

#include <event.hxx>

namespace DOM { namespace events
{

    CEvent::CEvent()
        : m_canceled(sal_False)
        , m_phase(PhaseType_CAPTURING_PHASE)
        , m_bubbles(sal_False)
        , m_cancelable(sal_True)
    {
    }

    CEvent::~CEvent()
    {
    }

    OUString SAL_CALL CEvent::getType() throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard const g(m_Mutex);
        return m_eventType;
    }

    Reference< XEventTarget > SAL_CALL
    CEvent::getTarget() throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard const g(m_Mutex);
        return m_target;
    }

    Reference< XEventTarget > SAL_CALL
    CEvent::getCurrentTarget() throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard const g(m_Mutex);
        return m_currentTarget;
    }

    PhaseType SAL_CALL CEvent::getEventPhase() throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard const g(m_Mutex);
        return m_phase;
    }

    sal_Bool SAL_CALL CEvent::getBubbles() throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard const g(m_Mutex);
        return m_bubbles;
    }

    sal_Bool SAL_CALL CEvent::getCancelable() throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard const g(m_Mutex);
        return m_cancelable;
    }

    com::sun::star::util::Time SAL_CALL
    CEvent::getTimeStamp() throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard const g(m_Mutex);
        return m_time;
    }

    void SAL_CALL CEvent::stopPropagation() throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard const g(m_Mutex);
        if (m_cancelable) { m_canceled = sal_True; }
    }

    void SAL_CALL CEvent::preventDefault() throw (RuntimeException, std::exception)
    {
    }

    void SAL_CALL
    CEvent::initEvent(OUString const& eventTypeArg, sal_Bool canBubbleArg,
        sal_Bool cancelableArg) throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard const g(m_Mutex);

        m_eventType = eventTypeArg;
        m_bubbles = canBubbleArg;
        m_cancelable = cancelableArg;
    }

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
