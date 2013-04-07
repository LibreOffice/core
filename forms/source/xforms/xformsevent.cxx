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


#include "xformsevent.hxx"

namespace com {
namespace sun {
namespace star {
namespace xforms {

using com::sun::star::uno::RuntimeException;

void SAL_CALL XFormsEventConcrete::initXFormsEvent(const OUString& typeArg,
    sal_Bool canBubbleArg, sal_Bool cancelableArg)
    throw (RuntimeException)
{
    initEvent(typeArg, canBubbleArg, cancelableArg);
}

OUString SAL_CALL XFormsEventConcrete::getType() throw (RuntimeException)
{
    return m_eventType;
}

XFormsEventConcrete::XEventTarget_t SAL_CALL XFormsEventConcrete::getTarget() throw (RuntimeException)
{
    return m_target;
}

XFormsEventConcrete::XEventTarget_t SAL_CALL XFormsEventConcrete::getCurrentTarget() throw (RuntimeException)
{
    return m_currentTarget;
}

XFormsEventConcrete::PhaseType_t SAL_CALL XFormsEventConcrete::getEventPhase() throw (RuntimeException)
{
    return m_phase;
}

sal_Bool SAL_CALL XFormsEventConcrete::getBubbles() throw (RuntimeException)
{
    return m_bubbles;
}

sal_Bool SAL_CALL XFormsEventConcrete::getCancelable() throw (RuntimeException)
{
    return m_cancelable;
}

XFormsEventConcrete::Time_t SAL_CALL XFormsEventConcrete::getTimeStamp() throw (RuntimeException)
{
    return m_time;
}

void SAL_CALL XFormsEventConcrete::stopPropagation() throw (RuntimeException)
{
    if(m_cancelable)
        m_canceled = sal_True;
}
void SAL_CALL XFormsEventConcrete::preventDefault() throw (RuntimeException)
{
}

void SAL_CALL XFormsEventConcrete::initEvent(const OUString& eventTypeArg, sal_Bool canBubbleArg,
    sal_Bool cancelableArg) throw (RuntimeException)
{
    m_eventType = eventTypeArg;
    m_bubbles = canBubbleArg;
    m_cancelable = cancelableArg;
}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
