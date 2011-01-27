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


#include <salhelper/condition.hxx>
#include <osl/time.h>


using namespace salhelper;


/******************************************************************
 *                                                                *
 *                     Condition                                  *
 *                                                                *
 ******************************************************************/

Condition::Condition(osl::Mutex& aMutex)
    : m_aMutex(aMutex),
      m_aCondition(osl_createCondition())
{
}


Condition::~Condition()
{
    osl_destroyCondition(m_aCondition);
}


/******************************************************************
 *                                                                *
 *              ConditionModifier                                 *
 *                                                                *
 ******************************************************************/

ConditionModifier::ConditionModifier(Condition& aCond)
    : m_aCond(aCond)
{
    m_aCond.m_aMutex.acquire();
}


ConditionModifier::~ConditionModifier()
{
    if(m_aCond.applies())
        osl_setCondition(m_aCond.m_aCondition);

    m_aCond.m_aMutex.release();
}



/******************************************************************
 *                                                                *
 *              ConditionWaiter                                   *
 *                                                                *
 ******************************************************************/

ConditionWaiter::timedout::timedout() {}

ConditionWaiter::timedout::timedout(timedout const &) {}

ConditionWaiter::timedout::~timedout() {}

ConditionWaiter::timedout &
ConditionWaiter::timedout::operator =(timedout const &) { return *this; }

ConditionWaiter::ConditionWaiter(Condition& aCond)
    : m_aCond(aCond)
{
    while(true) {
        osl_waitCondition(m_aCond.m_aCondition,0);
        m_aCond.m_aMutex.acquire();

        if(m_aCond.applies())
            break;
        else {
            osl_resetCondition(m_aCond.m_aCondition);
            m_aCond.m_aMutex.release();
        }
    }
}


ConditionWaiter::ConditionWaiter(Condition& aCond,sal_uInt32 milliSec)
    throw(
        ConditionWaiter::timedout
    )
    : m_aCond(aCond)
{
    TimeValue aTime;
    aTime.Seconds = milliSec / 1000;
    aTime.Nanosec = 1000000 * ( milliSec % 1000 );

    while(true) {
        if( osl_waitCondition(m_aCond.m_aCondition,&aTime) ==
            osl_cond_result_timeout )
            throw timedout();

        m_aCond.m_aMutex.acquire();

        if(m_aCond.applies())
            break;
        else {
            osl_resetCondition(m_aCond.m_aCondition);
            m_aCond.m_aMutex.release();
        }
    }
}


ConditionWaiter::~ConditionWaiter()
{
    if(! m_aCond.applies())
        osl_resetCondition(m_aCond.m_aCondition);
    m_aCond.m_aMutex.release();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
