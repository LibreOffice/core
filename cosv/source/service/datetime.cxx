/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/




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


