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

    virtual sal_Int32 SAL_CALL getScreenX() override;
    virtual sal_Int32 SAL_CALL getScreenY() override;
    virtual sal_Int32 SAL_CALL getClientX() override;
    virtual sal_Int32 SAL_CALL getClientY() override;
    virtual sal_Bool SAL_CALL getCtrlKey() override;
    virtual sal_Bool SAL_CALL getShiftKey() override;
    virtual sal_Bool SAL_CALL getAltKey() override;
    virtual sal_Bool SAL_CALL getMetaKey() override;
    virtual sal_Int16 SAL_CALL getButton() override;
    virtual css::uno::Reference< css::xml::dom::events::XEventTarget > SAL_CALL getRelatedTarget() override;

    virtual void SAL_CALL initMouseEvent(
                        const OUString& typeArg,
                        sal_Bool canBubbleArg,
                        sal_Bool cancelableArg,
                        const css::uno::Reference< css::xml::dom::views::XAbstractView >& viewArg,
                        sal_Int32 detailArg,
                        sal_Int32 screenXArg,
                        sal_Int32 screenYArg,
                        sal_Int32 clientXArg,
                        sal_Int32 clientYArg,
                        sal_Bool ctrlKeyArg,
                        sal_Bool altKeyArg,
                        sal_Bool shiftKeyArg,
                        sal_Bool metaKeyArg,
                        sal_Int16 buttonArg,
                        const css::uno::Reference< css::xml::dom::events::XEventTarget >& relatedTargetArg) override;

    // delegate to CUIevent
    virtual css::uno::Reference< css::xml::dom::views::XAbstractView > SAL_CALL getView() override;
    virtual sal_Int32 SAL_CALL getDetail() override;
    virtual void SAL_CALL initUIEvent(const OUString& typeArg,
                     sal_Bool canBubbleArg,
                     sal_Bool cancelableArg,
                     const css::uno::Reference< css::xml::dom::views::XAbstractView >& viewArg,
                     sal_Int32 detailArg) override;
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
