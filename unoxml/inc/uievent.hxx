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

#ifndef INCLUDED_UNOXML_INC_UIEVENT_HXX
#define INCLUDED_UNOXML_INC_UIEVENT_HXX

#include <sal/types.h>

#include <com/sun/star/xml/dom/events/PhaseType.hpp>
#include <com/sun/star/xml/dom/events/XUIEvent.hpp>
#include <com/sun/star/xml/dom/views/XAbstractView.hpp>

#include <cppuhelper/implbase.hxx>

#include "event.hxx"

namespace DOM { namespace events {

typedef ::cppu::ImplInheritanceHelper< CEvent, css::xml::dom::events::XUIEvent > CUIEvent_Base;

class CUIEvent
    : public CUIEvent_Base
{
    sal_Int32 m_detail;
    css::uno::Reference< css::xml::dom::views::XAbstractView > m_view;

public:
    explicit CUIEvent();

    virtual css::uno::Reference< css::xml::dom::views::XAbstractView > SAL_CALL getView() override;
    virtual sal_Int32 SAL_CALL getDetail() override;
    virtual void SAL_CALL initUIEvent(const OUString& typeArg,
                     sal_Bool canBubbleArg,
                     sal_Bool cancelableArg,
                     const css::uno::Reference< css::xml::dom::views::XAbstractView >& viewArg,
                     sal_Int32 detailArg) override;

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
}}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
