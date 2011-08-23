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


#include <precomp.h>
#include <cosv/datetime.hxx>


// NOT FULLY DECLARED SERVICES


namespace csv
{


Date::Date()
    :	nData( 0 )
{
}

Date::Date( unsigned 			i_nDay,
            unsigned 			i_nMonth,
            unsigned 			i_nYear )
    :	nData( (i_nDay << 24) + (i_nMonth << 16) + i_nYear )
{
}

const Date &
Date::Null_()
{
    static const Date C_DateNull_(0,0,0);
    return C_DateNull_;
}


Time::Time()
    :	nData( 0 )
{
}

Time::Time( unsigned 			i_nHour,
            unsigned 			i_nMinutes,
            unsigned 			i_nSeconds,
            unsigned            i_nSeconds100 )
    :	nData( (i_nHour << 24) + (i_nMinutes << 16) + (i_nSeconds << 8) + i_nSeconds100 )
{
}

const Time &
Time::Null_()
{
    static const Time C_TimeNull_(0,0);
    return C_TimeNull_;
}



}   // namespace csv


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
