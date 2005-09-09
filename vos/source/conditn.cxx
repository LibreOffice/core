/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: conditn.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:06:44 $
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

#ifndef _OSL_TIME_H_
#include <osl/time.h>
#endif
#include <vos/conditn.hxx>
#include <vos/diagnose.hxx>

#ifdef _USE_NAMESPACE
using namespace vos;
#endif


VOS_IMPLEMENT_CLASSINFO(VOS_CLASSNAME(OCondition, vos), VOS_NAMESPACE(OCondition, vos), VOS_NAMESPACE(OObject, vos), 0);

/// initial state of condition is not set
OCondition::OCondition()
{
    m_Condition= osl_createCondition();
}

OCondition::~OCondition()
{
    osl_destroyCondition(m_Condition);
}

/// set condition to sal_True => wait() will not block, check() returns sal_True
void OCondition::set()
{
    osl_setCondition(m_Condition);
}

/// set condition to sal_False => wait() will block, check() returns sal_False
void OCondition::reset()
{
    osl_resetCondition(m_Condition);
}

/** Blocks if condition is not set<BR>
    If condition has been destroyed prematurely, wait() will
    return with sal_False.
*/
OCondition::TResult OCondition::wait(const TimeValue* pTimeout)
{
    return (TResult)osl_waitCondition(m_Condition, pTimeout);
}

/** sal_True: condition is set <BR>
    sal_False: condition is not set <BR>
    does not block
*/
sal_Bool OCondition::check()
{
    return osl_checkCondition(m_Condition);
}

