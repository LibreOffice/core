/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include <localtime.hxx>
#include <limits.h>
#include <unicode/timezone.h>
#include <memory>

const long DAY_SEC =24 * 60 * 60;
const long YEAR_SEC = 365 * DAY_SEC;
const long FOURYEAR_SEC = 4 * YEAR_SEC + DAY_SEC;
#ifndef LONG_MAX
const long LONG_MAX=2147483647;
#endif
//01-01-70 was a Thursday
const long BASE_DOW = 4;

bool LtgGmTime(long rtime,LtTm& rtm)
{
    if (rtime < 0)
    {
        return false;
    }
    //is-current-year-a-leap-year flag
    int islpyr = 0;

    long tmptim;
    long caltim = rtime;
    tmptim = static_cast<long>(caltim / FOURYEAR_SEC);
    caltim -= tmptim * FOURYEAR_SEC;

    //Determine which year of the interval

    // 1970, 1974, 1978,...,etc.
    tmptim = (tmptim * 4) + 70;

    if (caltim >= YEAR_SEC)
    {
        //1971, 1975, 1979,...,etc.
        tmptim++;
        caltim -= YEAR_SEC;

        if ( caltim >= YEAR_SEC )
        {
            // 1972, 1976, 1980,...,etc.
            tmptim++;
            caltim -= YEAR_SEC;

            //Note, it takes 366 days-worth of seconds to get past a leap year.
            if (caltim >= (YEAR_SEC + DAY_SEC))
            {
                //1973, 1977, 1981,...,etc.
                tmptim++;
                caltim -= (YEAR_SEC + DAY_SEC);
            }
            else
            {
                //In a leap year after all, set the flag.
                islpyr++;
            }
        }
    }

    //tmptim now holds the value for tm_year. caltim now holds the
    //number of elapsed seconds since the beginning of that year.

    rtm.tm_year = tmptim;

    //Determine days since January 1 (0 - 365). This is the tm_yday value.
    //Leave caltim with number of elapsed seconds in that day.

    rtm.tm_yday = static_cast<long>(caltim / DAY_SEC);
    caltim -= rtm.tm_yday * DAY_SEC;

    //Determine months since January (0 - 11) and day of month (1 - 31)

    long const * mdays;
    if ( islpyr )
    {
        static long const lpdays[] =
            {-1, 30, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
        mdays = lpdays;
    }
    else
    {
        static long const days[] =
            {-1, 30, 58, 89, 119, 150, 180, 211, 242, 272, 303, 333, 364};
        mdays = days;
    }

    for ( tmptim = 1 ; mdays[tmptim] < rtm.tm_yday ; tmptim++ ) ;

    rtm.tm_mon = --tmptim;

    rtm.tm_mday = rtm.tm_yday - mdays[tmptim];

    //Determine days since Sunday (0 - 6)

    rtm.tm_wday = (static_cast<long>(rtime / DAY_SEC) + BASE_DOW) % 7;

    //Determine hours since midnight (0 - 23), minutes after the hour
    //(0 - 59), and seconds after the minute (0 - 59).

    rtm.tm_hour = static_cast<long>(caltim / 3600);
    caltim -= rtm.tm_hour * 3600;

    rtm.tm_min = static_cast<long>(caltim / 60);
    rtm.tm_sec = static_cast<long>(caltim - (rtm.tm_min) * 60);

    //adjust year & month
    rtm.tm_year += 1900;
    ++(rtm.tm_mon);

    return true;

};
bool LtgLocalTime(long rtime,LtTm& rtm)
{
    if (rtime < 0)
    {
        return false;
    }

    if ((rtime > 3 * DAY_SEC)&&(rtime < LONG_MAX - 3 * DAY_SEC))
    {
        std::unique_ptr<icu::TimeZone> pLocalZone(icu::TimeZone::createDefault());
        long offset = (pLocalZone->getRawOffset())/1000;
        pLocalZone.reset();
        long ltime = rtime + offset;
        return LtgGmTime(ltime,rtm);
    }
    return false;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
