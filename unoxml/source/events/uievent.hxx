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

#ifndef INCLUDED_UNOXML_SOURCE_EVENTS_UIEVENT_HXX
#define INCLUDED_UNOXML_SOURCE_EVENTS_UIEVENT_HXX

#include <sal/types.h>

#include <com/sun/star/xml/dom/events/PhaseType.hpp>
#include <com/sun/star/xml/dom/events/XUIEvent.hpp>
#include <com/sun/star/xml/dom/views/XAbstractView.hpp>

#include <cppuhelper/implbase1.hxx>

#include "event.hxx"


using namespace com::sun::star::xml::dom::views;

namespace DOM { namespace events {

typedef ::cppu::ImplInheritanceHelper1< CEvent, XUIEvent > CUIEvent_Base;

class CUIEvent
    : public CUIEvent_Base
{
protected:
    sal_Int32 m_detail;
    Reference< XAbstractView > m_view;

public:
    explicit CUIEvent();

    virtual Reference< XAbstractView > SAL_CALL getView() throw(RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getDetail() throw(RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL initUIEvent(const OUString& typeArg,
                     sal_Bool canBubbleArg,
                     sal_Bool cancelableArg,
                     const Reference< XAbstractView >& viewArg,
                     sal_Int32 detailArg) throw(RuntimeException, std::exception) SAL_OVERRIDE;

    // delegate to CEvent, since we are inheriting from CEvent and XEvent
    virtual OUString SAL_CALL getType() throw (RuntimeException, std::exception) SAL_OVERRIDE;
    virtual Reference< XEventTarget > SAL_CALL getTarget() throw (RuntimeException, std::exception) SAL_OVERRIDE;
    virtual Reference< XEventTarget > SAL_CALL getCurrentTarget() throw (RuntimeException, std::exception) SAL_OVERRIDE;
    virtual PhaseType SAL_CALL getEventPhase() throw (RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL getBubbles() throw (RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL getCancelable() throw (RuntimeException, std::exception) SAL_OVERRIDE;
    virtual com::sun::star::util::Time SAL_CALL getTimeStamp() throw (RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL stopPropagation() throw (RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL preventDefault() throw (RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL initEvent(
        const OUString& eventTypeArg,
        sal_Bool canBubbleArg,
        sal_Bool cancelableArg)
        throw (RuntimeException, std::exception) SAL_OVERRIDE;
};
}}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
