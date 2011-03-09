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

#include <uievent.hxx>

namespace DOM { namespace events
{
    CUIEvent::CUIEvent()
        : CUIEvent_Base()
        , m_detail(0)
    {
    }

    Reference< XAbstractView > SAL_CALL
    CUIEvent::getView() throw(RuntimeException)
    {
        ::osl::MutexGuard const g(m_Mutex);
        return m_view;
    }

    sal_Int32 SAL_CALL CUIEvent::getDetail() throw(RuntimeException)
    {
        ::osl::MutexGuard const g(m_Mutex);
        return m_detail;
    }

    void SAL_CALL CUIEvent::initUIEvent(const OUString& typeArg,
                     sal_Bool canBubbleArg,
                     sal_Bool cancelableArg,
                     const Reference< XAbstractView >& viewArg,
                     sal_Int32 detailArg) throw(RuntimeException)
    {
        ::osl::MutexGuard const g(m_Mutex);

        CEvent::initEvent(typeArg, canBubbleArg, cancelableArg);
        m_view = viewArg;
        m_detail = detailArg;
    }


    // delegate to CEvent, since we are inheriting from CEvent and XEvent
    OUString SAL_CALL CUIEvent::getType() throw (RuntimeException)
    {
        return CEvent::getType();
    }

    Reference< XEventTarget > SAL_CALL CUIEvent::getTarget() throw (RuntimeException)
    {
        return CEvent::getTarget();
    }

    Reference< XEventTarget > SAL_CALL CUIEvent::getCurrentTarget() throw (RuntimeException)
    {
        return CEvent::getCurrentTarget();
    }

    PhaseType SAL_CALL CUIEvent::getEventPhase() throw (RuntimeException)
    {
        return CEvent::getEventPhase();
    }

    sal_Bool SAL_CALL CUIEvent::getBubbles() throw (RuntimeException)
    {
        return CEvent::getBubbles();
    }

    sal_Bool SAL_CALL CUIEvent::getCancelable() throw (RuntimeException)
    {
        // mutation events cannot be canceled
        return sal_False;
    }

    com::sun::star::util::Time SAL_CALL CUIEvent::getTimeStamp() throw (RuntimeException)
    {
        return CEvent::getTimeStamp();
    }

    void SAL_CALL CUIEvent::stopPropagation() throw (RuntimeException)
    {
        CEvent::stopPropagation();
    }
    void SAL_CALL CUIEvent::preventDefault() throw (RuntimeException)
    {
        CEvent::preventDefault();
    }

    void SAL_CALL CUIEvent::initEvent(const OUString& eventTypeArg, sal_Bool canBubbleArg,
        sal_Bool cancelableArg) throw (RuntimeException)
    {
        // base initializer
        CEvent::initEvent(eventTypeArg, canBubbleArg, cancelableArg);
    }
}}
