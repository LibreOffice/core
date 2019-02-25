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
#ifndef INCLUDED_FORMS_SOURCE_XFORMS_XFORMSEVENT_HXX
#define INCLUDED_FORMS_SOURCE_XFORMS_XFORMSEVENT_HXX

#include <sal/types.h>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/xforms/XFormsEvent.hpp>
#include <com/sun/star/xml/dom/events/XEventTarget.hpp>

namespace com {
namespace sun {
namespace star {
namespace xforms {

class XFormsEventConcrete : public cppu::WeakImplHelper< XFormsEvent > {

    public:

        XFormsEventConcrete()
            : m_bubbles(false)
            , m_cancelable(false)
        {
        }

        virtual OUString SAL_CALL getType() override;
        virtual css::uno::Reference< css::xml::dom::events::XEventTarget > SAL_CALL getTarget() override;
        virtual css::uno::Reference< css::xml::dom::events::XEventTarget > SAL_CALL getCurrentTarget() override;
        virtual css::xml::dom::events::PhaseType SAL_CALL getEventPhase() override;
        virtual sal_Bool SAL_CALL getBubbles() override;
        virtual sal_Bool SAL_CALL getCancelable() override;
        virtual css::util::Time SAL_CALL getTimeStamp() override;
        virtual void SAL_CALL stopPropagation() override;
        virtual void SAL_CALL preventDefault() override;

        virtual void SAL_CALL initXFormsEvent(
                            const OUString& typeArg,
                            sal_Bool canBubbleArg,
                            sal_Bool cancelableArg ) override;

        virtual void SAL_CALL initEvent(
            const OUString& eventTypeArg,
            sal_Bool canBubbleArg,
            sal_Bool cancelableArg) override;

    private:

        OUString m_eventType;
        bool m_bubbles;
        bool m_cancelable;
};

} } } }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
