/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: _XExtendedCalendar.java,v $
 * $Revision: 1.8 $
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
package ifc.i18n;

import com.sun.star.i18n.CalendarDisplayCode;
import com.sun.star.i18n.NativeNumberMode;
import com.sun.star.i18n.XExtendedCalendar;
import com.sun.star.i18n.XLocaleData;
import com.sun.star.lang.Locale;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.GregorianCalendar;
import lib.MultiMethodTest;

/**
 *
 */
public class _XExtendedCalendar extends MultiMethodTest {
    public XExtendedCalendar oObj = null;
    boolean useUSENLocale = false;
    /**
     * Load a calendar
     */
    public void before() {
        Locale[] installed_locales = null;
        XLocaleData locData = null;
        try {
            locData = (XLocaleData) UnoRuntime.queryInterface(
                XLocaleData.class,
                    ((XMultiServiceFactory)tParam.getMSF()).createInstance(
                    "com.sun.star.i18n.LocaleData"));
        } catch (com.sun.star.uno.Exception e) {

        }
        installed_locales = locData.getAllInstalledLocaleNames();
        // use first Locale as fallback, if US-English is not found
        Locale lo = installed_locales[0];
        for (int i=0; i<installed_locales.length; i++) {
            // search for "en" and "US"
            if (installed_locales[i].Language.equals("en") &&
                            installed_locales[i].Country.equals("US")) {
                lo = installed_locales[i];
                useUSENLocale = true;
            }
        }
        log.println("Choose Locale: '" + lo.Language + "', '" + lo.Country + "'");
        oObj.loadDefaultCalendar(lo);
    }


    public void _getDisplayString() {
        // against regression: the current state is the right one.
        boolean result = true;
        String[] displayString = new String[6];
        // build the defaults with the Java Calendar functions
        String[] expectedStringResult = new String[6];
        Calendar cal = new GregorianCalendar();
        Date actualDate = cal.getTime();

        SimpleDateFormat sdf = getSDF("yy");
        expectedStringResult[0] = "AD" + sdf.format(actualDate);

        sdf = getSDF("yyyy");
        expectedStringResult[1] = "AD" + sdf.format(actualDate);

        sdf = getSDF("MM");
        expectedStringResult[2] = sdf.format(actualDate);

        int month = cal.get(Calendar.MONTH) + 1;
        String quarter = "Q1";
        String longQuarter = "1st quarter";
        if (month > 3 && month < 7) { quarter = "Q2"; longQuarter = "2nd quarter"; }
        else if (month > 6 && month < 10) { quarter = "Q3"; longQuarter = "3rd quarter"; }
        else if (month > 10 && month < 13) {quarter = "Q4"; longQuarter = "4th quarter"; }
        expectedStringResult[3] = quarter;
        expectedStringResult[4] = longQuarter;

        sdf = getSDF("MMMM");
        expectedStringResult[5] = sdf.format(actualDate);

        displayString[0] = oObj.getDisplayString(CalendarDisplayCode.SHORT_YEAR_AND_ERA, NativeNumberMode.NATNUM0);
        displayString[1] = oObj.getDisplayString(CalendarDisplayCode.LONG_YEAR_AND_ERA, NativeNumberMode.NATNUM0);
        displayString[2] = oObj.getDisplayString(CalendarDisplayCode.LONG_MONTH, NativeNumberMode.NATNUM0);
        displayString[3] = oObj.getDisplayString(CalendarDisplayCode.SHORT_QUARTER, NativeNumberMode.NATNUM0);
        displayString[4] = oObj.getDisplayString(CalendarDisplayCode.LONG_QUARTER, NativeNumberMode.NATNUM0);
        displayString[5] = oObj.getDisplayString(CalendarDisplayCode.LONG_MONTH_NAME, NativeNumberMode.NATNUM0);

        for (int i=0; i<displayString.length; i++) {
            boolean locResult = false;
            if (useUSENLocale) {
                locResult = displayString[i].equals(expectedStringResult[i]);
                if (!locResult)
                    log.println("getDisplayString() result " + i + ": '" + displayString[i]
                                        + "', expected: '" + expectedStringResult[i] + "'");
                result &= locResult;
            }
            else { // no defaults for other locales, just expect a String
                locResult &= displayString[i] != null;
                if (!locResult)
                    log.println("getDisplayString() result " + i + " was 'null'");
                result &= locResult;
            }
        }
        tRes.tested("getDisplayString()", result);
    }

    private SimpleDateFormat getSDF(String format){
        if (useUSENLocale) return new SimpleDateFormat(format, java.util.Locale.US);
        return new SimpleDateFormat(format);
    }
}
