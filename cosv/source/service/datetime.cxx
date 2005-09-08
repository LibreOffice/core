/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: datetime.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:04:29 $
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


#include <precomp.h>
#include <cosv/datetime.hxx>


// NOT FULLY DECLARED SERVICES


namespace csv
{


Date::Date()
    :   nData( 0 )
{
}

Date::Date( unsigned            i_nDay,
            unsigned            i_nMonth,
            unsigned            i_nYear )
    :   nData( (i_nDay << 24) + (i_nMonth << 16) + i_nYear )
{
}

const Date &
Date::Null_()
{
    static const Date C_DateNull_(0,0,0);
    return C_DateNull_;
}


Time::Time()
    :   nData( 0 )
{
}

Time::Time( unsigned            i_nHour,
            unsigned            i_nMinutes,
            unsigned            i_nSeconds,
            unsigned            i_nSeconds100 )
    :   nData( (i_nHour << 24) + (i_nMinutes << 16) + (i_nSeconds << 8) + i_nSeconds100 )
{
}

const Time &
Time::Null_()
{
    static const Time C_TimeNull_(0,0);
    return C_TimeNull_;
}



}   // namespace csv


