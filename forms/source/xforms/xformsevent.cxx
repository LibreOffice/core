/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xformsevent.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 00:06:21 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
