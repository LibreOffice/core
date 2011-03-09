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

#include <mutationevent.hxx>

namespace DOM { namespace events
{
    CMutationEvent::CMutationEvent()
        : CMutationEvent_Base()
        , m_attrChangeType(AttrChangeType_MODIFICATION)
    {
    }

    CMutationEvent::~CMutationEvent()
    {
    }

    Reference< XNode > SAL_CALL CMutationEvent::getRelatedNode() throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_Mutex);
        return m_relatedNode;
    }

    OUString SAL_CALL CMutationEvent::getPrevValue() throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_Mutex);
        return m_prevValue;
    }

    OUString SAL_CALL CMutationEvent::getNewValue() throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_Mutex);
        return m_newValue;
    }

    OUString SAL_CALL CMutationEvent::getAttrName() throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_Mutex);
        return m_attrName;
    }

    AttrChangeType SAL_CALL CMutationEvent::getAttrChange() throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_Mutex);
        return m_attrChangeType;
    }

    void SAL_CALL CMutationEvent::initMutationEvent(const OUString& typeArg,
        sal_Bool canBubbleArg, sal_Bool cancelableArg,
        const Reference< XNode >& relatedNodeArg, const OUString& prevValueArg,
        const OUString& newValueArg, const OUString& attrNameArg,
        AttrChangeType attrChangeArg) throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_Mutex);

        CEvent::initEvent(typeArg, canBubbleArg, cancelableArg);
        m_relatedNode = relatedNodeArg;
        m_prevValue = prevValueArg;
        m_newValue = newValueArg;
        m_attrName = attrNameArg;
        m_attrChangeType = attrChangeArg;
    }

    // delegate to CEvent, since we are inheriting from CEvent and XEvent
    OUString SAL_CALL CMutationEvent::getType() throw (RuntimeException)
    {
        return CEvent::getType();
    }

    Reference< XEventTarget > SAL_CALL CMutationEvent::getTarget() throw (RuntimeException)
    {
        return CEvent::getTarget();
    }

    Reference< XEventTarget > SAL_CALL CMutationEvent::getCurrentTarget() throw (RuntimeException)
    {
        return CEvent::getCurrentTarget();
    }

    PhaseType SAL_CALL CMutationEvent::getEventPhase() throw (RuntimeException)
    {
        return CEvent::getEventPhase();
    }

    sal_Bool SAL_CALL CMutationEvent::getBubbles() throw (RuntimeException)
    {
        return CEvent::getBubbles();
    }

    sal_Bool SAL_CALL CMutationEvent::getCancelable() throw (RuntimeException)
    {
        return CEvent::getCancelable();
    }

    com::sun::star::util::Time SAL_CALL CMutationEvent::getTimeStamp() throw (RuntimeException)
    {
        return CEvent::getTimeStamp();
    }

    void SAL_CALL CMutationEvent::stopPropagation() throw (RuntimeException)
    {
        CEvent::stopPropagation();
    }
    void SAL_CALL CMutationEvent::preventDefault() throw (RuntimeException)
    {
        CEvent::preventDefault();
    }

    void SAL_CALL CMutationEvent::initEvent(const OUString& eventTypeArg, sal_Bool canBubbleArg,
        sal_Bool cancelableArg) throw (RuntimeException)
    {
        // base initializer
        CEvent::initEvent(eventTypeArg, canBubbleArg, cancelableArg);
    }
}}

