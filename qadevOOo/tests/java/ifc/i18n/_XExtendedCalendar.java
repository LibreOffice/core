/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
package ifc.i18n;

import com.sun.star.i18n.CalendarDisplayCode;
import com.sun.star.i18n.NativeNumberMode;
import com.sun.star.i18n.XExtendedCalendar;
import com.sun.star.i18n.XLocaleData;
import com.sun.star.lang.Locale;
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
    @Override
    public void before() throws Exception {
        Locale[] installed_locales = null;
        XLocaleData locData = UnoRuntime.queryInterface(
                        XLocaleData.class,
                            tParam.getMSF().createInstance(
                            "com.sun.star.i18n.LocaleData"));

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
