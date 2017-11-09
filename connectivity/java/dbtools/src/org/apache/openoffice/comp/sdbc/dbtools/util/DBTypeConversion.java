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

package org.apache.openoffice.comp.sdbc.dbtools.util;

import java.math.BigInteger;
import java.util.StringTokenizer;

import com.sun.star.util.Date;
import com.sun.star.util.DateTime;
import com.sun.star.util.Time;

public class DBTypeConversion {
    private static final int MAX_DAYS = 3636532;
    private static Date standardDate = new Date((short)1, (short)1, (short)1900);
    private static int aDaysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    private static final double fMilliSecondsPerDay = 86400000.0;

    public static double safeParseDouble(String value) {
        try {
            return Double.parseDouble(value);
        } catch (NumberFormatException numberFormatException) {
            return 0.0;
        }
    }

    public static float safeParseFloat(String value) {
        try {
            return Float.parseFloat(value);
        } catch (NumberFormatException numberFormatException) {
            return 0.0f;
        }
    }

    public static int safeParseInt(String value) {
        try {
            return Integer.parseInt(value);
        } catch (NumberFormatException numberFormatException) {
            return 0;
        }
    }

    public static long safeParseLong(String value) {
        try {
            return Long.parseLong(value);
        } catch (NumberFormatException numberFormatException) {
            return 0;
        }
    }

    public static float unsignedLongToFloat(long value) {
        float f = value & 0x7fffFFFFffffFFFFL;
        if (value < 0) {
            f += 0x1p63f;
        }
        return f;
    }

    public static double unsignedLongToDouble(long value) {
        double d = value & 0x7fffFFFFffffFFFFL;
        if (value < 0) {
            d += 0x1p63f;
        }
        return d;
    }

    public static int toUnsignedInt(byte value) {
        return value & 0xff;
    }

    public static int toUnsignedInt(short value) {
        return value & 0xffff;
    }

    public static String toUnsignedString(int value) {
        return Long.toString(value & 0xffffFFFFL);
    }

    public static String toUnsignedString(long value) {
        return new BigInteger(Long.toHexString(value), 16).toString();
    }

    public static void addDays(int nDays, Date _rDate) {
        int   nTempDays = implRelativeToAbsoluteNull( _rDate );

        nTempDays += nDays;
        if ( nTempDays > MAX_DAYS )
        {
            _rDate.Day      = 31;
            _rDate.Month    = 12;
            _rDate.Year     = 9999;
        }
        else if ( nTempDays <= 0 )
        {
            _rDate.Day      = 1;
            _rDate.Month    = 1;
            _rDate.Year     = 00;
        }
        else
            implBuildFromRelative( nTempDays, _rDate );
    }

    public static void subDays(int nDays, Date _rDate) {
        int   nTempDays = implRelativeToAbsoluteNull( _rDate );

        nTempDays -= nDays;
        if ( nTempDays > MAX_DAYS )
        {
            _rDate.Day      = 31;
            _rDate.Month    = 12;
            _rDate.Year     = 9999;
        }
        else if ( nTempDays <= 0 )
        {
            _rDate.Day      = 1;
            _rDate.Month    = 1;
            _rDate.Year     = 00;
        }
        else
            implBuildFromRelative( nTempDays, _rDate );
    }

    public static int getMsFromTime(final Time rVal) {
        int   nHour     = rVal.Hours;
        int   nMin      = rVal.Minutes;
        int   nSec      = rVal.Seconds;
        int   n100Sec   = rVal.HundredthSeconds;

        return ((nHour*3600000)+(nMin*60000)+(nSec*1000)+(n100Sec*10));
    }

    public static Date getStandardDate() {
        return standardDate;
    }

    private static int implDaysInMonth(int _nMonth, int _nYear) {
        if (_nMonth != 2)
            return aDaysInMonth[_nMonth-1];
        else {
            if (implIsLeapYear(_nYear))
                return aDaysInMonth[_nMonth-1] + 1;
            else
                return aDaysInMonth[_nMonth-1];
        }
    }

    private static void implBuildFromRelative( int nDays, Date date ) {
        int   nTempDays;
        int   i = 0;
        boolean    bCalc;

        do {
            nTempDays = nDays;
            date.Year = (short)((nTempDays / 365) - i);
            nTempDays -= (date.Year-1) * 365;
            nTempDays -= ((date.Year-1) / 4) - ((date.Year-1) / 100) + ((date.Year-1) / 400);
            bCalc = false;
            if ( nTempDays < 1 )
            {
                i++;
                bCalc = true;
            }
            else
            {
                if ( nTempDays > 365 )
                {
                    if ( (nTempDays != 366) || !implIsLeapYear( date.Year ) )
                    {
                        i--;
                        bCalc = true;
                    }
                }
            }
        }
        while ( bCalc );

        date.Month = 1;
        while ( nTempDays > implDaysInMonth( date.Month, date.Year ) )
        {
            nTempDays -= implDaysInMonth( date.Month, date.Year );
            date.Month++;
        }
        date.Day = (short)nTempDays;
    }

    private static boolean implIsLeapYear(int _nYear) {
        return  (   (   ((_nYear % 4) == 0)
                    &&  ((_nYear % 100) != 0)
                    )
                )
                ||  ((_nYear % 400) == 0)
                ;
    }


    private static int implRelativeToAbsoluteNull(final Date _rDate) {
        int nDays = 0;

        // ripped this code from the implementation of tools::Date
        int nNormalizedYear = _rDate.Year - 1;
        nDays = nNormalizedYear * 365;
        // leap years
        nDays += (nNormalizedYear / 4) - (nNormalizedYear / 100) + (nNormalizedYear / 400);

        for (int i = 1; i < _rDate.Month; ++i)
            nDays += implDaysInMonth(i, _rDate.Year);

        nDays += _rDate.Day;
        return nDays;
    }

    public static int toDays(Date rVal) {
        return toDays(rVal, getStandardDate());
    }

    public static int toDays(Date rVal, Date rNullDate) {
        return implRelativeToAbsoluteNull(rVal) - implRelativeToAbsoluteNull(rNullDate);
    }

    public static double toDouble(Date rVal) {
        return toDouble(rVal, getStandardDate());
    }

    public static double toDouble(Date rVal, Date _rNullDate) {
        return (double)toDays(rVal, _rNullDate);
    }

    public static double toDouble(DateTime _rVal) {
        return toDouble(_rVal, getStandardDate());
    }

    public static double toDouble(DateTime _rVal, Date _rNullDate) {
        long   nTime     = toDays(new Date(_rVal.Day, _rVal.Month, _rVal.Year), _rNullDate);
        Time aTimePart = new Time();

        aTimePart.Hours             = _rVal.Hours;
        aTimePart.Minutes           = _rVal.Minutes;
        aTimePart.Seconds           = _rVal.Seconds;
        aTimePart.HundredthSeconds  = _rVal.HundredthSeconds;

        return ((double)nTime) + toDouble(aTimePart);
    }

    public static double toDouble(Time rVal) {
        return (double)getMsFromTime(rVal) / fMilliSecondsPerDay;
    }

    public static Date toDate(double dVal) {
        return toDate(dVal, getStandardDate());
    }

    public static Date toDate(double dVal, Date _rNullDate) {
        Date aRet = _rNullDate;

        if (dVal >= 0)
            addDays((int)dVal,aRet);
        else
            subDays((int)(-dVal),aRet);
            //  x -= (sal_uInt32)(-nDays);

        return aRet;
    }

    public static Date toDate(String value) {
        String[] tokens = value.split("-");

        short  nYear   = 0,
                nMonth  = 0,
                nDay    = 0;
        if (tokens.length > 0) {
            nYear   = (short)safeParseInt(tokens[0]);
        }
        if (tokens.length > 1) {
            nMonth = (short)safeParseInt(tokens[1]);
        }
        if (tokens.length > 2) {
            nDay = (short)safeParseInt(tokens[2]);
        }

        return new Date(nDay,nMonth,nYear);
    }

    public static DateTime toDateTime(double dVal) {
        return toDateTime(dVal, getStandardDate());
    }

    public static DateTime toDateTime(double dVal, Date _rNullDate) {
        Date aDate = toDate(dVal, _rNullDate);
        Time aTime = toTime(dVal);

        DateTime xRet = new DateTime();

        xRet.Day                = aDate.Day;
        xRet.Month              = aDate.Month;
        xRet.Year               = aDate.Year;

        xRet.HundredthSeconds   = aTime.HundredthSeconds;
        xRet.Minutes            = aTime.Minutes;
        xRet.Seconds            = aTime.Seconds;
        xRet.Hours              = aTime.Hours;


        return xRet;
    }

    public static DateTime toDateTime(String _sSQLString) {
        // the date part
        int nSeparation = _sSQLString.indexOf( ' ' );
        String dateString;
        String timeString = "";
        if (nSeparation >= 0) {
            dateString = _sSQLString.substring(0, nSeparation);
            timeString = _sSQLString.substring(nSeparation + 1);
        } else {
            dateString = _sSQLString;
        }
        Date aDate = toDate(dateString);
        Time aTime = new Time();

        if ( -1 != nSeparation )
            aTime = toTime( timeString );

        return new DateTime(aTime.HundredthSeconds,aTime.Seconds,aTime.Minutes,aTime.Hours,aDate.Day,aDate.Month,aDate.Year);
    }

    public static Time toTime(int _nVal) {
        Time aReturn = new Time();
        aReturn.Hours = (short)(((int)(_nVal >= 0 ? _nVal : _nVal*-1)) / 1000000);
        aReturn.Minutes = (short)((((int)(_nVal >= 0 ? _nVal : _nVal*-1)) / 10000) % 100);
        aReturn.Seconds = (short)((((int)(_nVal >= 0 ? _nVal : _nVal*-1)) / 100) % 100);
        aReturn.HundredthSeconds = (short)(((int)(_nVal >= 0 ? _nVal : _nVal*-1)) % 100);
        return aReturn;
    }

    public static Time toTime(double dVal) {
        int nDays     = (int)dVal;
        int nMS = (int)((dVal - (double)nDays) * fMilliSecondsPerDay + 0.5);

        short nSign;
        if ( nMS < 0 )
        {
            nMS *= -1;
            nSign = -1;
        }
        else
            nSign = 1;

        Time xRet = new Time();
        // Zeit normalisieren
        // we have to sal_Int32 here because otherwise we get an overflow
        int nHundredthSeconds = nMS/10;
        int nSeconds          = nHundredthSeconds / 100;
        int nMinutes          = nSeconds / 60;

        xRet.HundredthSeconds       = (short)(nHundredthSeconds % 100);
        xRet.Seconds                = (short)(nSeconds % 60);
        xRet.Hours                  = (short)(nMinutes / 60);
        xRet.Minutes                = (short)(nMinutes % 60);

        // Zeit zusammenbauen
        int nTime = (int)(xRet.HundredthSeconds + (xRet.Seconds*100) + (xRet.Minutes*10000) + (xRet.Hours*1000000)) * nSign;

        if(nTime < 0)
        {
            xRet.HundredthSeconds   = 99;
            xRet.Minutes            = 59;
            xRet.Seconds            = 59;
            xRet.Hours              = 23;
        }
        return xRet;
    }

    public static Time toTime(String _sSQLString) {
        short  nHour   = 0,
                    nMinute = 0,
                    nSecond = 0,
                    nHundredthSeconds   = 0;
        StringTokenizer tokenizer = new StringTokenizer(_sSQLString, ":");
        if (tokenizer.hasMoreTokens()) {
            nHour = (short)safeParseInt(tokenizer.nextToken());
        }
        if (tokenizer.hasMoreTokens()) {
            nMinute = (short)safeParseInt(tokenizer.nextToken());
        }
        if (tokenizer.hasMoreTokens()) {
            String secondAndNano = tokenizer.nextToken();
            int dot = secondAndNano.indexOf(".");
            if (dot >= 0) {
                nSecond = (short)safeParseInt(secondAndNano.substring(0, dot));
                String nano = secondAndNano.substring(dot + 1);
                if (nano.length() > 2) {
                    nano = nano.substring(0, 2);
                }
                nano = nano + "00".substring(0, 2 - nano.length());
                nHundredthSeconds = (short)safeParseInt(nano);
            } else {
                nSecond = (short)safeParseInt(secondAndNano);
            }
        }
        return new Time(nHundredthSeconds,nSecond,nMinute,nHour);
    }

    /// Return the date in the format %04d-%02d-%02d.
    public static String toDateString(Date date) {
        return String.format("%04d-%02d-%02d",
                toUnsignedInt(date.Year),
                toUnsignedInt(date.Month),
                toUnsignedInt(date.Day));
    }

    /// Return the time in the format %02d:%02d:%02d.
    public static String toTimeString(Time time) {
        return String.format("%02d:%02d:%02d",
                toUnsignedInt(time.Hours),
                toUnsignedInt(time.Minutes),
                toUnsignedInt(time.Seconds));
    }

    /// Return the DateTime in the format %04d-%02d-%02d %02d:%02d:%02d.%d.
    public static String toDateTimeString(DateTime dateTime) {
        return String.format("%04d-%02d-%02d %02d:%02d:%02d.%d",
                toUnsignedInt(dateTime.Year),
                toUnsignedInt(dateTime.Month),
                toUnsignedInt(dateTime.Day),
                toUnsignedInt(dateTime.Hours),
                toUnsignedInt(dateTime.Minutes),
                toUnsignedInt(dateTime.Seconds),
                toUnsignedInt(dateTime.HundredthSeconds));
    }
}
