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

#ifndef EVENT_MUTATIONEVENT_HXX
#define EVENT_MUTATIONEVENT_HXX

#include <sal/types.h>

#include <com/sun/star/uno/Reference.h>

#include <com/sun/star/xml/dom/events/PhaseType.hpp>
#include <com/sun/star/xml/dom/events/AttrChangeType.hpp>
#include <com/sun/star/xml/dom/events/XMutationEvent.hpp>

#include <cppuhelper/implbase1.hxx>

#include "event.hxx"

namespace DOM { namespace events {

typedef ::cppu::ImplInheritanceHelper1< CEvent, XMutationEvent >
    CMutationEvent_Base;

class CMutationEvent
    : public CMutationEvent_Base
{
protected:
    Reference< XNode > m_relatedNode;
    OUString m_prevValue;
    OUString m_newValue;
    OUString m_attrName;
    AttrChangeType m_attrChangeType;

public:
    explicit CMutationEvent();

    virtual ~CMutationEvent();

    virtual Reference< XNode > SAL_CALL getRelatedNode() throw (RuntimeException);
    virtual OUString SAL_CALL getPrevValue() throw (RuntimeException);
    virtual OUString SAL_CALL getNewValue() throw (RuntimeException);
    virtual OUString SAL_CALL getAttrName() throw (RuntimeException);
    virtual AttrChangeType SAL_CALL getAttrChange() throw (RuntimeException);
    virtual void SAL_CALL initMutationEvent(
                           const OUString& typeArg,
                           sal_Bool canBubbleArg,
                           sal_Bool cancelableArg,
                           const Reference< XNode >& relatedNodeArg,
                           const OUString& prevValueArg,
                           const OUString& newValueArg,
                           const OUString& attrNameArg,
                           AttrChangeType attrChangeArg) throw (RuntimeException);

    // delegate to CEvent, since we are inheriting from CEvent and XEvent
    virtual OUString SAL_CALL getType() throw (RuntimeException);
    virtual Reference< XEventTarget > SAL_CALL getTarget() throw (RuntimeException);
    virtual Reference< XEventTarget > SAL_CALL getCurrentTarget() throw (RuntimeException);
    virtual PhaseType SAL_CALL getEventPhase() throw (RuntimeException);
    virtual sal_Bool SAL_CALL getBubbles() throw (RuntimeException);
    virtual sal_Bool SAL_CALL getCancelable() throw (RuntimeException);
    virtual com::sun::star::util::Time SAL_CALL getTimeStamp() throw (RuntimeException);
    virtual void SAL_CALL stopPropagation() throw (RuntimeException);
    virtual void SAL_CALL preventDefault() throw (RuntimeException);
    virtual void SAL_CALL initEvent(
        const OUString& eventTypeArg,
        sal_Bool canBubbleArg,
        sal_Bool cancelableArg)
        throw (RuntimeException);
};
}}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
