/*************************************************************************
 *
 *  $RCSfile: condition.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: svesik $ $Date: 2004-04-21 12:30:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#include <salhelper/condition.hxx>

#ifndef _OSL_TIME_H_
#include <osl/time.h>
#endif


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
