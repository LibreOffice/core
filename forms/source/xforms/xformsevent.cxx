/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: xformsevent.cxx,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_forms.hxx"
#include "xformsevent.hxx"

namespace com {
namespace sun {
namespace star {
namespace xforms {

using rtl::OUString;
using com::sun::star::uno::RuntimeException;

void SAL_CALL XFormsEventConcrete::initXFormsEvent(const OUString& typeArg,
    sal_Bool canBubbleArg, sal_Bool cancelableArg)
    throw (RuntimeException)
{
    initEvent(typeArg, canBubbleArg, cancelableArg);
}

OUString SAL_CALL XFormsEventConcrete::getType() throw (RuntimeException)
{
    return m_eventType;
}

XFormsEventConcrete::XEventTarget_t SAL_CALL XFormsEventConcrete::getTarget() throw (RuntimeException)
{
    return m_target;
}

XFormsEventConcrete::XEventTarget_t SAL_CALL XFormsEventConcrete::getCurrentTarget() throw (RuntimeException)
{
    return m_currentTarget;
}

XFormsEventConcrete::PhaseType_t SAL_CALL XFormsEventConcrete::getEventPhase() throw (RuntimeException)
{
    return m_phase;
}

sal_Bool SAL_CALL XFormsEventConcrete::getBubbles() throw (RuntimeException)
{
    return m_bubbles;
}

sal_Bool SAL_CALL XFormsEventConcrete::getCancelable() throw (RuntimeException)
{
    return m_cancelable;
}

XFormsEventConcrete::Time_t SAL_CALL XFormsEventConcrete::getTimeStamp() throw (RuntimeException)
{
    return m_time;
}

void SAL_CALL XFormsEventConcrete::stopPropagation() throw (RuntimeException)
{
    if(m_cancelable)
        m_canceled = sal_True;
}
void SAL_CALL XFormsEventConcrete::preventDefault() throw (RuntimeException)
{
}

void SAL_CALL XFormsEventConcrete::initEvent(const OUString& eventTypeArg, sal_Bool canBubbleArg,
    sal_Bool cancelableArg) throw (RuntimeException)
{
    m_eventType = eventTypeArg;
    m_bubbles = canBubbleArg;
    m_cancelable = cancelableArg;
}

} } } }
