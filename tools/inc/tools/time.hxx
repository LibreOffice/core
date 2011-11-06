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


#ifndef _TOOLS_TIME_HXX
#define _TOOLS_TIME_HXX

#include "tools/toolsdllapi.h"
#include <tools/solar.h>

class ResId;

// --------
// - Time -
// --------

class TOOLS_DLLPUBLIC Time
{
private:
    sal_Int32           nTime;

public:
                    Time();
                    Time( const ResId & rResId );
                    Time( sal_Int32 _nTime ) { Time::nTime = _nTime; }
                    Time( const Time& rTime );
                    Time( sal_uIntPtr nHour, sal_uIntPtr nMin,
                          sal_uIntPtr nSec = 0, sal_uIntPtr n100Sec = 0 );

    void            SetTime( sal_Int32 nNewTime ) { nTime = nNewTime; }
    sal_Int32       GetTime() const { return nTime; }

    void            SetHour( sal_uInt16 nNewHour );
    void            SetMin( sal_uInt16 nNewMin );
    void            SetSec( sal_uInt16 nNewSec );
    void            Set100Sec( sal_uInt16 nNew100Sec );
    sal_uInt16          GetHour() const
                        { sal_uIntPtr nTempTime = (nTime >= 0) ? nTime : nTime*-1;
                          return (sal_uInt16)(nTempTime / 1000000); }
    sal_uInt16          GetMin() const
                        { sal_uIntPtr nTempTime = (nTime >= 0) ? nTime : nTime*-1;
                          return (sal_uInt16)((nTempTime / 10000) % 100); }
    sal_uInt16          GetSec() const
                        { sal_uIntPtr nTempTime = (nTime >= 0) ? nTime : nTime*-1;
                          return (sal_uInt16)((nTempTime / 100) % 100); }
    sal_uInt16          Get100Sec() const
                        { sal_uIntPtr nTempTime = (nTime >= 0) ? nTime : nTime*-1;
                          return (sal_uInt16)(nTempTime % 100); }

    sal_Int32       GetMSFromTime() const;
    void            MakeTimeFromMS( sal_Int32 nMS );

                    /// 12 hours == 0.5 days
    double          GetTimeInDays() const;

    sal_Bool            IsBetween( const Time& rFrom, const Time& rTo ) const
                        { return ((nTime >= rFrom.nTime) && (nTime <= rTo.nTime)); }

    sal_Bool            IsEqualIgnore100Sec( const Time& rTime ) const;

    sal_Bool            operator ==( const Time& rTime ) const
                        { return (nTime == rTime.nTime); }
    sal_Bool            operator !=( const Time& rTime ) const
                        { return (nTime != rTime.nTime); }
    sal_Bool            operator  >( const Time& rTime ) const
                        { return (nTime > rTime.nTime); }
    sal_Bool            operator  <( const Time& rTime ) const
                        { return (nTime < rTime.nTime); }
    sal_Bool            operator >=( const Time& rTime ) const
                        { return (nTime >= rTime.nTime); }
    sal_Bool            operator <=( const Time& rTime ) const
                        { return (nTime <= rTime.nTime); }

    static Time     GetUTCOffset();
    static sal_uIntPtr  GetSystemTicks();       // Elapsed time
    static sal_uIntPtr  GetProcessTicks();      // CPU time

    void            ConvertToUTC()       { *this -= Time::GetUTCOffset(); }
    void            ConvertToLocalTime() { *this += Time::GetUTCOffset(); }

    Time&           operator =( const Time& rTime );
    Time            operator -() const
                        { return Time( nTime * -1 ); }
    Time&           operator +=( const Time& rTime );
    Time&           operator -=( const Time& rTime );
    TOOLS_DLLPUBLIC friend Time     operator +( const Time& rTime1, const Time& rTime2 );
    TOOLS_DLLPUBLIC friend Time     operator -( const Time& rTime1, const Time& rTime2 );
};

#endif // _TOOLS_TIME_HXX
