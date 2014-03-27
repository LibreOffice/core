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
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/xforms/XFormsEvent.hpp>
#include <com/sun/star/xml/dom/events/XEventTarget.hpp>

namespace com {
namespace sun {
namespace star {
namespace xforms {

class XFormsEventConcrete : public cppu::WeakImplHelper1< XFormsEvent > {

    public:

        typedef com::sun::star::uno::RuntimeException RuntimeException_t;
        typedef com::sun::star::uno::Reference< com::sun::star::xml::dom::events::XEventTarget > XEventTarget_t;
        typedef com::sun::star::xml::dom::events::PhaseType PhaseType_t;
        typedef com::sun::star::util::Time Time_t;

        inline XFormsEventConcrete( void ) : m_canceled(sal_False) {}
        virtual ~XFormsEventConcrete( void ) {}

        virtual OUString SAL_CALL getType() throw (RuntimeException_t, std::exception) SAL_OVERRIDE;
        virtual XEventTarget_t SAL_CALL getTarget() throw (RuntimeException_t, std::exception) SAL_OVERRIDE;
        virtual XEventTarget_t SAL_CALL getCurrentTarget() throw (RuntimeException_t, std::exception) SAL_OVERRIDE;
        virtual PhaseType_t SAL_CALL getEventPhase() throw (RuntimeException_t, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL getBubbles() throw (RuntimeException_t, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL getCancelable() throw (RuntimeException_t, std::exception) SAL_OVERRIDE;
        virtual Time_t SAL_CALL getTimeStamp() throw (RuntimeException_t, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL stopPropagation() throw (RuntimeException_t, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL preventDefault() throw (RuntimeException_t, std::exception) SAL_OVERRIDE;

        virtual void SAL_CALL initXFormsEvent(
                            const OUString& typeArg,
                            sal_Bool canBubbleArg,
                            sal_Bool cancelableArg )
                            throw (RuntimeException_t, std::exception) SAL_OVERRIDE;

        virtual void SAL_CALL initEvent(
            const OUString& eventTypeArg,
            sal_Bool canBubbleArg,
            sal_Bool cancelableArg)
            throw (RuntimeException_t, std::exception) SAL_OVERRIDE;

    private:

        sal_Bool m_canceled;

    protected:

        OUString m_eventType;
        XEventTarget_t m_target;
        XEventTarget_t m_currentTarget;
        PhaseType_t m_phase;
        sal_Bool m_bubbles;
        sal_Bool m_cancelable;
        Time_t m_time;
};

} } } }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
