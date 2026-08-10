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
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/xforms/XFormsEvent.hpp>
#include <com/sun/star/xml/dom/events/XEventTarget.hpp>

namespace com::sun::star::xforms {

class XFormsEventConcrete : public cppu::WeakImplHelper< XFormsEvent > {

    public:

        XFormsEventConcrete()
            : m_bubbles(false)
            , m_cancelable(false)
        {
        }

        virtual OUString getType() override;
        virtual css::uno::Reference< css::xml::dom::events::XEventTarget > getTarget() override;
        virtual css::uno::Reference< css::xml::dom::events::XEventTarget > getCurrentTarget() override;
        virtual css::xml::dom::events::PhaseType getEventPhase() override;
        virtual bool getBubbles() override;
        virtual bool getCancelable() override;
        virtual css::util::Time getTimeStamp() override;
        virtual void stopPropagation() override;
        virtual void preventDefault() override;

        virtual void initXFormsEvent(
                            const OUString& typeArg,
                            bool canBubbleArg,
                            bool cancelableArg ) override;

        virtual void initEvent(
            const OUString& eventTypeArg,
            bool canBubbleArg,
            bool cancelableArg) override;

    private:

        OUString m_eventType;
        bool m_bubbles;
        bool m_cancelable;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
