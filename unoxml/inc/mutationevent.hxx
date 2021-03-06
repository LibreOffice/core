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

#pragma once

#include <sal/types.h>

#include <com/sun/star/uno/Reference.h>

#include <com/sun/star/xml/dom/events/PhaseType.hpp>
#include <com/sun/star/xml/dom/events/AttrChangeType.hpp>
#include <com/sun/star/xml/dom/events/XMutationEvent.hpp>

#include <cppuhelper/implbase.hxx>

#include "event.hxx"

namespace DOM::events {

typedef ::cppu::ImplInheritanceHelper< CEvent, css::xml::dom::events::XMutationEvent >
    CMutationEvent_Base;

class CMutationEvent final
    : public CMutationEvent_Base
{
    css::uno::Reference< css::xml::dom::XNode > m_relatedNode;
    OUString m_prevValue;
    OUString m_newValue;
    OUString m_attrName;
    css::xml::dom::events::AttrChangeType m_attrChangeType;

public:
    explicit CMutationEvent();

    virtual ~CMutationEvent() override;

    virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL getRelatedNode() override;
    virtual OUString SAL_CALL getPrevValue() override;
    virtual OUString SAL_CALL getNewValue() override;
    virtual OUString SAL_CALL getAttrName() override;
    virtual css::xml::dom::events::AttrChangeType SAL_CALL getAttrChange() override;
    virtual void SAL_CALL initMutationEvent(
                           const OUString& typeArg,
                           sal_Bool canBubbleArg,
                           sal_Bool cancelableArg,
                           const css::uno::Reference< css::xml::dom::XNode >& relatedNodeArg,
                           const OUString& prevValueArg,
                           const OUString& newValueArg,
                           const OUString& attrNameArg,
                           css::xml::dom::events::AttrChangeType attrChangeArg) override;

    // delegate to CEvent, since we are inheriting from CEvent and XEvent
    virtual OUString SAL_CALL getType() override;
    virtual css::uno::Reference< css::xml::dom::events::XEventTarget > SAL_CALL getTarget() override;
    virtual css::uno::Reference< css::xml::dom::events::XEventTarget > SAL_CALL getCurrentTarget() override;
    virtual css::xml::dom::events::PhaseType SAL_CALL getEventPhase() override;
    virtual sal_Bool SAL_CALL getBubbles() override;
    virtual sal_Bool SAL_CALL getCancelable() override;
    virtual css::util::Time SAL_CALL getTimeStamp() override;
    virtual void SAL_CALL stopPropagation() override;
    virtual void SAL_CALL preventDefault() override;
    virtual void SAL_CALL initEvent(
        const OUString& eventTypeArg,
        sal_Bool canBubbleArg,
        sal_Bool cancelableArg) override;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
