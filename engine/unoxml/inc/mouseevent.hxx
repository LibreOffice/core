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

#include <com/sun/star/xml/dom/events/PhaseType.hpp>
#include <com/sun/star/xml/dom/events/XMouseEvent.hpp>

#include <cppuhelper/implbase.hxx>

#include "uievent.hxx"

namespace DOM::events {

typedef ::cppu::ImplInheritanceHelper< CUIEvent, css::xml::dom::events::XMouseEvent >
    CMouseEvent_Base;

class CMouseEvent final
    : public CMouseEvent_Base
{
    sal_Int32 m_screenX;
    sal_Int32 m_screenY;
    sal_Int32 m_clientX;
    sal_Int32 m_clientY;
    bool m_ctrlKey;
    bool m_shiftKey;
    bool m_altKey;
    bool m_metaKey;
    sal_Int16 m_button;

public:
    explicit CMouseEvent();

    virtual sal_Int32 getScreenX() override;
    virtual sal_Int32 getScreenY() override;
    virtual sal_Int32 getClientX() override;
    virtual sal_Int32 getClientY() override;
    virtual bool getCtrlKey() override;
    virtual bool getShiftKey() override;
    virtual bool getAltKey() override;
    virtual bool getMetaKey() override;
    virtual sal_Int16 getButton() override;
    virtual cpo::uno::Reference< css::xml::dom::events::XEventTarget > getRelatedTarget() override;

    virtual void initMouseEvent(
                        const OUString& typeArg,
                        bool canBubbleArg,
                        bool cancelableArg,
                        const cpo::uno::Reference< css::xml::dom::views::XAbstractView >& viewArg,
                        sal_Int32 detailArg,
                        sal_Int32 screenXArg,
                        sal_Int32 screenYArg,
                        sal_Int32 clientXArg,
                        sal_Int32 clientYArg,
                        bool ctrlKeyArg,
                        bool altKeyArg,
                        bool shiftKeyArg,
                        bool metaKeyArg,
                        sal_Int16 buttonArg,
                        const cpo::uno::Reference< css::xml::dom::events::XEventTarget >& relatedTargetArg) override;

    // delegate to CUIevent
    virtual cpo::uno::Reference< css::xml::dom::views::XAbstractView > getView() override;
    virtual sal_Int32 getDetail() override;
    virtual void initUIEvent(const OUString& typeArg,
                     bool canBubbleArg,
                     bool cancelableArg,
                     const cpo::uno::Reference< css::xml::dom::views::XAbstractView >& viewArg,
                     sal_Int32 detailArg) override;
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
