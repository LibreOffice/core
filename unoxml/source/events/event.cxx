/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

    OUString SAL_CALL CEvent::getType() throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_Mutex);
        return m_eventType;
    }

    Reference< XEventTarget > SAL_CALL
    CEvent::getTarget() throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_Mutex);
        return m_target;
    }

    Reference< XEventTarget > SAL_CALL
    CEvent::getCurrentTarget() throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_Mutex);
        return m_currentTarget;
    }

    PhaseType SAL_CALL CEvent::getEventPhase() throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_Mutex);
        return m_phase;
    }

    sal_Bool SAL_CALL CEvent::getBubbles() throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_Mutex);
        return m_bubbles;
    }

    sal_Bool SAL_CALL CEvent::getCancelable() throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_Mutex);
        return m_cancelable;
    }

    com::sun::star::util::Time SAL_CALL
    CEvent::getTimeStamp() throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_Mutex);
        return m_time;
    }

    void SAL_CALL CEvent::stopPropagation() throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_Mutex);
        if (m_cancelable) { m_canceled = sal_True; }
    }

    void SAL_CALL CEvent::preventDefault() throw (RuntimeException)
    {
    }

    void SAL_CALL
    CEvent::initEvent(OUString const& eventTypeArg, sal_Bool canBubbleArg,
        sal_Bool cancelableArg) throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_Mutex);

        m_eventType = eventTypeArg;
        m_bubbles = canBubbleArg;
        m_cancelable = cancelableArg;
    }

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
