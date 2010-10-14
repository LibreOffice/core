/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include <osl/time.h>
#include <vos/conditn.hxx>
#include <osl/diagnose.h>

using namespace vos;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
