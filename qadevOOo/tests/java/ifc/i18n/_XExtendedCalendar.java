/*************************************************************************
 *
 *  $RCSfile: _XExtendedCalendar.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Date: 2004-07-23 10:45:11 $
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
import java.text.DecimalFormat;
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

        SimpleDateFormat sdf = new SimpleDateFormat("yy");
        expectedStringResult[0] = "AD" + sdf.format(actualDate);

        sdf = new SimpleDateFormat("yyyy");
        expectedStringResult[1] = "AD" + sdf.format(actualDate);

        sdf = new SimpleDateFormat("MM");
        expectedStringResult[2] = sdf.format(actualDate);

        int month = cal.get(Calendar.MONTH) + 1;
        String quarter = "Q1";
        String longQuarter = "1st quarter";
        if (month > 3 && month < 7) { quarter = "Q2"; longQuarter = "2nd quarter"; }
        else if (month > 6 && month < 10) { quarter = "Q3"; longQuarter = "3rd quarter"; }
        else { quarter = "Q4"; longQuarter = "4th quarter"; }
        expectedStringResult[3] = quarter;
        expectedStringResult[4] = longQuarter;

        sdf = new SimpleDateFormat("MMMMMMMMM");
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
}
