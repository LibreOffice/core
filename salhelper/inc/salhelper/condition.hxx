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


#ifndef _SALHELPER_CONDITION_HXX_
#define _SALHELPER_CONDITION_HXX_


#include <osl/conditn.h>
#include <osl/mutex.hxx>


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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
