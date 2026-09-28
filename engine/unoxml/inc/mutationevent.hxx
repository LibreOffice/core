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

#pragma once

#include <sal/types.h>

#include <cpo/uno/Reference.h>

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
    cpo::uno::Reference< css::xml::dom::XNode > m_relatedNode;
    OUString m_prevValue;
    OUString m_newValue;
    OUString m_attrName;
    css::xml::dom::events::AttrChangeType m_attrChangeType;

public:
    explicit CMutationEvent();

    virtual ~CMutationEvent() override;

    virtual cpo::uno::Reference< css::xml::dom::XNode > getRelatedNode() override;
    virtual OUString getPrevValue() override;
    virtual OUString getNewValue() override;
    virtual OUString getAttrName() override;
    virtual css::xml::dom::events::AttrChangeType getAttrChange() override;
    virtual void initMutationEvent(
                           const OUString& typeArg,
                           bool canBubbleArg,
                           bool cancelableArg,
                           const cpo::uno::Reference< css::xml::dom::XNode >& relatedNodeArg,
                           const OUString& prevValueArg,
                           const OUString& newValueArg,
                           const OUString& attrNameArg,
                           css::xml::dom::events::AttrChangeType attrChangeArg) override;

    // delegate to CEvent, since we are inheriting from CEvent and XEvent
    virtual OUString getType() override;
    virtual cpo::uno::Reference< css::xml::dom::events::XEventTarget > getTarget() override;
    virtual cpo::uno::Reference< css::xml::dom::events::XEventTarget > getCurrentTarget() override;
    virtual css::xml::dom::events::PhaseType getEventPhase() override;
    virtual bool getBubbles() override;
    virtual bool getCancelable() override;
    virtual css::util::Time getTimeStamp() override;
    virtual void stopPropagation() override;
    virtual void preventDefault() override;
    virtual void initEvent(
        const OUString& eventTypeArg,
        bool canBubbleArg,
        bool cancelableArg) override;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
