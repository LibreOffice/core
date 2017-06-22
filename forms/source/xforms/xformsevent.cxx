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
{
    initEvent(typeArg, canBubbleArg, cancelableArg);
}

OUString SAL_CALL XFormsEventConcrete::getType()
{
    return m_eventType;
}

css::uno::Reference< css::xml::dom::events::XEventTarget > SAL_CALL XFormsEventConcrete::getTarget()
{
    return m_target;
}

css::uno::Reference< css::xml::dom::events::XEventTarget > SAL_CALL XFormsEventConcrete::getCurrentTarget()
{
    return m_currentTarget;
}

css::xml::dom::events::PhaseType SAL_CALL XFormsEventConcrete::getEventPhase()
{
    return css::xml::dom::events::PhaseType_CAPTURING_PHASE;
}

sal_Bool SAL_CALL XFormsEventConcrete::getBubbles()
{
    return m_bubbles;
}

sal_Bool SAL_CALL XFormsEventConcrete::getCancelable()
{
    return m_cancelable;
}

css::util::Time SAL_CALL XFormsEventConcrete::getTimeStamp()
{
    return m_time;
}

void SAL_CALL XFormsEventConcrete::stopPropagation()
{
}
void SAL_CALL XFormsEventConcrete::preventDefault()
{
}

void SAL_CALL XFormsEventConcrete::initEvent(const OUString& eventTypeArg, sal_Bool canBubbleArg,
    sal_Bool cancelableArg)
{
    m_eventType = eventTypeArg;
    m_bubbles = canBubbleArg;
    m_cancelable = cancelableArg;
}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
