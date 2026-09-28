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

#include <mutationevent.hxx>

using namespace ::cpo::uno;
using namespace css::xml::dom;
using namespace css::xml::dom::events;

namespace DOM::events
{
    CMutationEvent::CMutationEvent()
        : m_attrChangeType(AttrChangeType_MODIFICATION)
    {
    }

    CMutationEvent::~CMutationEvent()
    {
    }

    Reference< XNode > CMutationEvent::getRelatedNode()
    {
        std::unique_lock const g(m_Mutex);
        return m_relatedNode;
    }

    OUString CMutationEvent::getPrevValue()
    {
        std::unique_lock const g(m_Mutex);
        return m_prevValue;
    }

    OUString CMutationEvent::getNewValue()
    {
        std::unique_lock const g(m_Mutex);
        return m_newValue;
    }

    OUString CMutationEvent::getAttrName()
    {
        std::unique_lock const g(m_Mutex);
        return m_attrName;
    }

    AttrChangeType CMutationEvent::getAttrChange()
    {
        std::unique_lock const g(m_Mutex);
        return m_attrChangeType;
    }

    void CMutationEvent::initMutationEvent(const OUString& typeArg,
        bool canBubbleArg, bool cancelableArg,
        const Reference< XNode >& relatedNodeArg, const OUString& prevValueArg,
        const OUString& newValueArg, const OUString& attrNameArg,
        AttrChangeType attrChangeArg)
    {
        CEvent::initEvent(typeArg, canBubbleArg, cancelableArg);

        std::unique_lock const g(m_Mutex);

        m_relatedNode = relatedNodeArg;
        m_prevValue = prevValueArg;
        m_newValue = newValueArg;
        m_attrName = attrNameArg;
        m_attrChangeType = attrChangeArg;
    }

    // delegate to CEvent, since we are inheriting from CEvent and XEvent
    OUString CMutationEvent::getType()
    {
        return CEvent::getType();
    }

    Reference< XEventTarget > CMutationEvent::getTarget()
    {
        return CEvent::getTarget();
    }

    Reference< XEventTarget > CMutationEvent::getCurrentTarget()
    {
        return CEvent::getCurrentTarget();
    }

    PhaseType CMutationEvent::getEventPhase()
    {
        return CEvent::getEventPhase();
    }

    bool CMutationEvent::getBubbles()
    {
        return CEvent::getBubbles();
    }

    bool CMutationEvent::getCancelable()
    {
        return CEvent::getCancelable();
    }

    css::util::Time CMutationEvent::getTimeStamp()
    {
        return CEvent::getTimeStamp();
    }

    void CMutationEvent::stopPropagation()
    {
        CEvent::stopPropagation();
    }
    void CMutationEvent::preventDefault()
    {
        CEvent::preventDefault();
    }

    void CMutationEvent::initEvent(const OUString& eventTypeArg, bool canBubbleArg,
        bool cancelableArg)
    {
        // base initializer
        CEvent::initEvent(eventTypeArg, canBubbleArg, cancelableArg);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
