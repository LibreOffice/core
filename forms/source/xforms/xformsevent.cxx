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
    throw (RuntimeException, std::exception)
{
    initEvent(typeArg, canBubbleArg, cancelableArg);
}

OUString SAL_CALL XFormsEventConcrete::getType() throw (RuntimeException, std::exception)
{
    return m_eventType;
}

css::uno::Reference< css::xml::dom::events::XEventTarget > SAL_CALL XFormsEventConcrete::getTarget() throw (RuntimeException, std::exception)
{
    return m_target;
}

css::uno::Reference< css::xml::dom::events::XEventTarget > SAL_CALL XFormsEventConcrete::getCurrentTarget() throw (RuntimeException, std::exception)
{
    return m_currentTarget;
}

css::xml::dom::events::PhaseType SAL_CALL XFormsEventConcrete::getEventPhase() throw (RuntimeException, std::exception)
{
    return m_phase;
}

sal_Bool SAL_CALL XFormsEventConcrete::getBubbles() throw (RuntimeException, std::exception)
{
    return m_bubbles;
}

sal_Bool SAL_CALL XFormsEventConcrete::getCancelable() throw (RuntimeException, std::exception)
{
    return m_cancelable;
}

css::util::Time SAL_CALL XFormsEventConcrete::getTimeStamp() throw (RuntimeException, std::exception)
{
    return m_time;
}

void SAL_CALL XFormsEventConcrete::stopPropagation() throw (RuntimeException, std::exception)
{
    if(m_cancelable)
        m_canceled = true;
}
void SAL_CALL XFormsEventConcrete::preventDefault() throw (RuntimeException, std::exception)
{
}

void SAL_CALL XFormsEventConcrete::initEvent(const OUString& eventTypeArg, sal_Bool canBubbleArg,
    sal_Bool cancelableArg) throw (RuntimeException, std::exception)
{
    m_eventType = eventTypeArg;
    m_bubbles = canBubbleArg;
    m_cancelable = cancelableArg;
}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
