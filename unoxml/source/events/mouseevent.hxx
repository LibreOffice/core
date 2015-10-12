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

#ifndef INCLUDED_UNOXML_SOURCE_EVENTS_MOUSEEVENT_HXX
#define INCLUDED_UNOXML_SOURCE_EVENTS_MOUSEEVENT_HXX

#include <com/sun/star/xml/dom/events/PhaseType.hpp>
#include <com/sun/star/xml/dom/events/XMouseEvent.hpp>

#include <cppuhelper/implbase.hxx>

#include "uievent.hxx"

namespace DOM { namespace events {

typedef ::cppu::ImplInheritanceHelper< CUIEvent, css::xml::dom::events::XMouseEvent >
    CMouseEvent_Base;

class CMouseEvent
    : public CMouseEvent_Base
{
protected:
    sal_Int32 m_screenX;
    sal_Int32 m_screenY;
    sal_Int32 m_clientX;
    sal_Int32 m_clientY;
    bool m_ctrlKey;
    bool m_shiftKey;
    bool m_altKey;
    bool m_metaKey;
    sal_Int16 m_button;
    css::uno::Reference< css::xml::dom::events::XEventTarget > m_relatedTarget;

public:
    explicit CMouseEvent();

    virtual sal_Int32 SAL_CALL getScreenX() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getScreenY() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getClientX() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getClientY() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getCtrlKey() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getShiftKey() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getAltKey() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getMetaKey() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getButton() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::xml::dom::events::XEventTarget > SAL_CALL getRelatedTarget()  throw(css::uno::RuntimeException, std::exception) override;

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
                        const css::uno::Reference< css::xml::dom::events::XEventTarget >& relatedTargetArg)
        throw(css::uno::RuntimeException, std::exception) override;

    // delegate to CUIevent
    virtual css::uno::Reference< css::xml::dom::views::XAbstractView > SAL_CALL getView() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getDetail() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL initUIEvent(const OUString& typeArg,
                     sal_Bool canBubbleArg,
                     sal_Bool cancelableArg,
                     const css::uno::Reference< css::xml::dom::views::XAbstractView >& viewArg,
                     sal_Int32 detailArg) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getType() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::xml::dom::events::XEventTarget > SAL_CALL getTarget() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::xml::dom::events::XEventTarget > SAL_CALL getCurrentTarget() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::xml::dom::events::PhaseType SAL_CALL getEventPhase() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getBubbles() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getCancelable() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::util::Time SAL_CALL getTimeStamp() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL stopPropagation() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL preventDefault() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL initEvent(
        const OUString& eventTypeArg,
        sal_Bool canBubbleArg,
        sal_Bool cancelableArg)
        throw (css::uno::RuntimeException, std::exception) override;
};
}}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
