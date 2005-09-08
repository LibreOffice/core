/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: condition.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 16:57:43 $
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


#ifndef _SALHELPER_CONDITION_HXX_
#define _SALHELPER_CONDITION_HXX_


#ifndef _OSL_CONDITION_H_
#include <osl/conditn.h>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif


namespace salhelper
{
    class ConditionModifier;
    class ConditionWaiter;


    class Condition
    {
        friend class ConditionModifier;
        friend class ConditionWaiter;

    public:

        Condition(osl::Mutex& aMutex);

        virtual ~Condition();


    protected:

        virtual bool applies() const = 0;


    private:
        Condition(Condition &); // not defined
        void operator =(Condition &); // not defined

        osl::Mutex&  m_aMutex;
        oslCondition m_aCondition;
    };



    class ConditionModifier
    {
    public:

        ConditionModifier(Condition& aCond);

        ~ConditionModifier();


    private:
        ConditionModifier(ConditionModifier &); // not defined
        void operator =(ConditionModifier &); // not defined

        Condition& m_aCond;
    };



    class ConditionWaiter
    {
    public:

        ConditionWaiter(Condition& aCond);

        struct timedout {
            timedout();

            timedout(timedout const &);

            virtual ~timedout();

            timedout & operator =(timedout const &);
        };

        ConditionWaiter(Condition& aCond,sal_uInt32 milliSec)
            throw(
                timedout
            );


        ~ConditionWaiter();


    private:
        ConditionWaiter(ConditionWaiter &); // not defined
        void operator =(ConditionWaiter &); // not defined

        Condition& m_aCond;
    };


}   // namespace salhelper


#endif
