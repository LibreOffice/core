/*************************************************************************
 *
 *  $RCSfile: _XCalendar.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 10:40:51 $
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

import lib.MultiMethodTest;

import com.sun.star.i18n.CalendarDisplayIndex;
import com.sun.star.i18n.CalendarFieldIndex;
import com.sun.star.i18n.CalendarItem;
import com.sun.star.i18n.XCalendar;
import com.sun.star.i18n.XLocaleData;
import com.sun.star.lang.Locale;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.i18n.XCalendar</code>
* interface methods :
* <ul>
*  <li><code> loadDefaultCalendar()</code></li>
*  <li><code> loadCalendar()</code></li>
*  <li><code> getLoadedCalendar()</code></li>
*  <li><code> getAllCalendars()</code></li>
*  <li><code> getUniqueID()</code></li>
*  <li><code> setDateTime()</code></li>
*  <li><code> getDateTime()</code></li>
*  <li><code> setValue()</code></li>
*  <li><code> getValue()</code></li>
*  <li><code> isValid()</code></li>
*  <li><code> addValue()</code></li>
*  <li><code> getFirstDayOfWeek()</code></li>
*  <li><code> setFirstDayOfWeek()</code></li>
*  <li><code> setMinimumNumberOfDaysForFirstWeek()</code></li>
*  <li><code> getMinimumNumberOfDaysForFirstWeek()</code></li>
*  <li><code> getNumberOfMonthsInYear()</code></li>
*  <li><code> getNumberOfDaysInWeek()</code></li>
*  <li><code> getMonths()</code></li>
*  <li><code> getDays()</code></li>
*  <li><code> getDisplayName()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.i18n.XCalendar
*/
public class _XCalendar extends MultiMethodTest {
    public XCalendar oObj = null;
    public String[][] calendars;
    public int[] count;
    public double newDTime = 1000.75;
    public short newValue = 0;
    public short firstDay = 2;
    public short mdfw = 3;
    Locale[] installed_locales;

    public void before() {
        XLocaleData locData = null;
        try {
            locData = (XLocaleData) UnoRuntime.queryInterface(
                XLocaleData.class,
                    ((XMultiServiceFactory)tParam.getMSF()).createInstance(
                    "com.sun.star.i18n.LocaleData"));
        } catch (com.sun.star.uno.Exception e) {

        }
        installed_locales = locData.getAllInstalledLocaleNames();
        calendars = new String[installed_locales.length][];
        count = new int[installed_locales.length];
    }

    /**
    * Loads default calendar for different locales. <p>
    * Has <b> OK </b> status if method loads calendar, that is
    * default for a given locale.
    */
    public void _loadDefaultCalendar() {
        boolean res = true;

        for (int i=0; i<installed_locales.length; i++) {
            String lang = "Language: "+installed_locales[i].Language +
                          ", Country: "+ installed_locales[i].Country +
                          ", Variant: "+ installed_locales[i].Country;
            oObj.loadDefaultCalendar(installed_locales[i]);
            if (oObj.getLoadedCalendar().Default) {
                //log.println(lang + " ... OK");
            } else {
                log.println(lang + " ... FAILED");
            }
            res &= oObj.getLoadedCalendar().Default;
        }

        tRes.tested("loadDefaultCalendar()", res);
    }

    /**
    * Tries to obtain calendars for a number of locales. <p>
    * Has <b> OK </b> status if the method returns more than zero calendars for
    * every locale.
    */
    public void _getAllCalendars() {
        boolean res = true;

        for (int i=0; i<installed_locales.length; i++) {
            String lang = "Language: "+installed_locales[i].Language +
                          ", Country: "+ installed_locales[i].Country +
                          ", Variant: "+ installed_locales[i].Country;
            calendars[i] = oObj.getAllCalendars(installed_locales[i]);
            count[i] = calendars[i].length-1;
            if (calendars[i].length > 0) {
                //log.println(lang + " ... OK");
            } else {
                log.println(lang + " ... FAILED");
            }
            res &= (calendars[i].length > 0);
        }
        tRes.tested("getAllCalendars()", res);
    }

    /**
    * Loads calendars for a number of locales. <p>
    * Has <b> OK </b> status if loaded calendar names are equal to gotten
    * calendar names after loading.<p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getAllCalendars() </code> : gets all calendars for a given
    *  locale </li>
    * </ul>
    */
    public void _loadCalendar() {
        boolean res = true;
        requiredMethod("getAllCalendars()");

        for (int i=0; i<installed_locales.length; i++) {
            String lang = "Language: "+installed_locales[i].Language +
                          ", Country: "+ installed_locales[i].Country +
                          ", Variant: "+ installed_locales[i].Country;
            oObj.loadCalendar(calendars[i][0], installed_locales[i]);
            if (calendars[i][0].equals(oObj.getLoadedCalendar().Name)) {
                //log.println(lang + " ... OK");
            } else {
                log.println(lang + " ... FAILED");
            }
            res &= calendars[i][0].equals(oObj.getLoadedCalendar().Name);
        }

        tRes.tested("loadCalendar()", res);
    }

    /**
    * Test calls the method, then result is checked. <p>
    * Has <b> OK </b> status if loaded calendar names are equal to gotten
    * calendar names after loading.<p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> loadCalendar() </code> : loads calendar using a given name
    *  and locale </li>
    * </ul>
    */
    public void _getLoadedCalendar() {
        boolean res = true;

        requiredMethod("loadCalendar()");
        for (int i=0; i<installed_locales.length; i++) {
            String lang = "Language: "+installed_locales[i].Language +
                          ", Country: "+ installed_locales[i].Country +
                          ", Variant: "+ installed_locales[i].Country;
            oObj.loadCalendar(calendars[i][0], installed_locales[i]);
            if (calendars[i][0].equals(oObj.getLoadedCalendar().Name)) {
                //log.println(lang + " ... OK");
            } else {
                log.println(lang + " ... FAILED");
            }
            res &= calendars[i][0].equals(oObj.getLoadedCalendar().Name);
        }
        tRes.tested("getLoadedCalendar()", res);
    }

    /**
    * Test calls the method, then result is checked. <p>
    * Has <b> OK </b> status if the method returns value that's equal to a
    * calendar name. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> loadCalendar() </code> :  loads calendar using a given name
    *  and locale </li>
    * </ul>
    */
    public void _getUniqueID() {
        boolean res = true;
        for (int i=0; i<installed_locales.length; i++) {
            String lang = "Language: "+installed_locales[i].Language +
                          ", Country: "+ installed_locales[i].Country +
                          ", Variant: "+ installed_locales[i].Country;
            oObj.loadCalendar(calendars[i][0], installed_locales[i]);
            String uID = oObj.getUniqueID();
            if (uID.equals(calendars[i][0])) {
                //log.println(lang + " ... OK");
            } else {
                log.println(lang + " ... FAILED");
            }
            res &= uID.equals(calendars[i][0]);
        }

        tRes.tested("getUniqueID()",res);
    }

    /**
    * Test calls the method, then result is checked. <p>
    * Has <b> OK </b> status if the method returns value, that's equal to
    * value set before. <p>
    */

    public void _setDateTime() {
        boolean res = true;
        for (int i=0; i<installed_locales.length; i++) {
            String lang = "Language: "+installed_locales[i].Language +
                          ", Country: "+ installed_locales[i].Country +
                          ", Variant: "+ installed_locales[i].Country;
            oObj.setDateTime(newDTime);
            double aDTime = oObj.getDateTime();
            if (aDTime == newDTime) {
                //log.println(lang + " ... OK");
            } else {
                log.println(lang + " ... FAILED");
            }
            res &= (aDTime == newDTime);
        }

        tRes.tested("setDateTime()", res);
    }

    /**
    * Test calls the method, then result is checked. <p>
    * Has <b> OK </b> status if the method returns value, that's equal to
    * value set before. <p>
    */

    public void _getDateTime() {
        boolean res = true;

        for (int i=0; i<installed_locales.length; i++) {
            String lang = "Language: "+installed_locales[i].Language +
                          ", Country: "+ installed_locales[i].Country +
                          ", Variant: "+ installed_locales[i].Country;
            oObj.setDateTime(newDTime);
            double aDTime = oObj.getDateTime();
            if (aDTime == newDTime) {
                //log.println(lang + " ... OK");
            } else {
                log.println(lang + " ... FAILED");
            }
            res &= (aDTime == newDTime);
        }

        tRes.tested("getDateTime()", res);
    }

    /**
    * Test calls the method, then result is checked. <p>
    * Has <b> OK </b> status if the method returns value, that's equal to
    * value set before. <p>
    */

    public void _setValue() {
        boolean res = true;
        for (int i=0; i<installed_locales.length; i++) {
            String error = "";
            String lang = "Language: "+installed_locales[i].Language +
                          ", Country: "+ installed_locales[i].Country +
                          ", Variant: "+ installed_locales[i].Country +
                          ", Name: "+calendars[i][count[i]];
            String[] names = new String[]{"DAY_OF_MONTH",
                "HOUR","MINUTE","SECOND","MILLISECOND",
                "YEAR","MONTH"};
            oObj.loadCalendar(calendars[i][count[i]],installed_locales[i]);
            short[] fields = new short[]{CalendarFieldIndex.DAY_OF_MONTH,
                                         CalendarFieldIndex.HOUR,
                                         CalendarFieldIndex.MINUTE,
                                         CalendarFieldIndex.SECOND,
                                         CalendarFieldIndex.MILLISECOND,
                                         CalendarFieldIndex.YEAR,
                                         CalendarFieldIndex.MONTH
            };
            for (int k=0; k<fields.length;k++) {
                //log.println("Setting: " + names[k]+" to "+ valueSet[k]);
                oObj.setDateTime(0.0);
                short set = oObj.getValue(fields[k]);
                oObj.setValue(fields[k],set);
                short get = oObj.getValue(fields[k]);
                if (get != set) {
                    error += "failed for "+names[k]+" expected "+
                                set+" gained "+get+" ; ";
                }
            }
            if (error.equals("")) {
                //log.println(lang + " ... OK");
            } else {
                log.println("*** "+lang + " ... FAILED ***");
                log.println(error);
            }
            res &= (error.equals(""));
        }

        tRes.tested("setValue()", res);
    }

    /**
    * Test calls the method, then result is checked. <p>
    * Has <b> OK </b> status if the method returns value, that's equal to
    * value set before. <p>
    */

    public void _getValue() {
        boolean res = true;

        requiredMethod("setValue()");
        short aValue = oObj.getValue(CalendarFieldIndex.MONTH);
        res &= (aValue == newValue);
        tRes.tested("getValue()", res);
    }

    /**
    * Test calls the method, then result is checked. <p>
    * Has <b> OK </b> status if value, added by the method is greater than
    * previously defined "newValue".
    * <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getValue() </code> : gets the value of a field </li>
    * </ul>
    */
    public void _addValue() {
        boolean res = true;

        requiredMethod("getValue()");
        oObj.addValue(CalendarFieldIndex.MONTH, 1);
        short aValue = oObj.getValue(CalendarFieldIndex.MONTH);
        res &= (aValue > newValue);
        tRes.tested("addValue()", res);
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown.
    */
    public void _setFirstDayOfWeek() {
        boolean res = true;

        oObj.setFirstDayOfWeek(firstDay);
        res &= true;
        tRes.tested("setFirstDayOfWeek()", res);
    }

    /**
    * Test calls the method, then result is checked. <p>
    * Has <b> OK </b> status if the method returns value that is equal to
    * value set before. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setFirstDayOfWeek() </code> : set the first day of a
    *  week</li>
    * </ul>
    */
    public void _getFirstDayOfWeek() {
        boolean res = true;

        requiredMethod("setFirstDayOfWeek()");
        short aFirstDayOfWeek = oObj.getFirstDayOfWeek();
        res &= (aFirstDayOfWeek == firstDay);
        tRes.tested("getFirstDayOfWeek()", res);
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown.
    */
    public void _setMinimumNumberOfDaysForFirstWeek() {
        boolean res = true;

        oObj.setMinimumNumberOfDaysForFirstWeek(mdfw);
        res &= true;
        tRes.tested("setMinimumNumberOfDaysForFirstWeek()", res);
    }

    /**
    * Test calls the method, then result is checked. <p>
    * Has <b> OK </b> status if the method returns value that is equal to
    * value set before. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setMinimumNumberOfDaysForFirstWeek() </code> : sets how
    *  many days of a week must reside in the first week of a year</li>
    * </ul>
    */
    public void _getMinimumNumberOfDaysForFirstWeek() {
        boolean res = true;

        requiredMethod("setMinimumNumberOfDaysForFirstWeek()");
        short aShort = oObj.getMinimumNumberOfDaysForFirstWeek();
        res &= (aShort == mdfw);
        tRes.tested("getMinimumNumberOfDaysForFirstWeek()", res);
    }

    /**
    * Test calls the method, then result is checked. <p>
    * Has <b> OK </b> status if the method returns 12.
    */
    public void _getNumberOfMonthsInYear() {
        boolean res = true;
        short aShort = oObj.getNumberOfMonthsInYear();

        res &= (aShort == (short) 12);
        tRes.tested("getNumberOfMonthsInYear()", res);
    }

    /**
    * Test calls the method, then result is checked. <p>
    * Has <b> OK </b> status if the method returns 7.
    */
    public void _getNumberOfDaysInWeek() {
        boolean res = true;
        short aShort = oObj.getNumberOfDaysInWeek();

        res &= (aShort == (short) 7);
        tRes.tested("getNumberOfDaysInWeek()", res);
    }

    /**
    * Test calls the method, then result is checked. <p>
    * Has <b> OK </b> status if length of array, returned by the method is 12.
    */
    public void _getMonths() {
        boolean res = true;
        CalendarItem[] months = oObj.getMonths();

        res &= (months.length == 12);
        tRes.tested("getMonths()", res);
    }

    /**
    * Test calls the method, then result is checked. <p>
    * Has <b> OK </b> status if length of array, returned by the method is 7.
    */
    public void _getDays() {
        boolean res = true;
        CalendarItem[] Days = oObj.getDays();

        res &= (Days.length == 7);
        tRes.tested("getDays()", res);
    }

    /**
    * After loading calendar, test calls the method, then result is checked.<p>
    * Has <b> OK </b> status if length of string, returned by the method is 3.
    */
    public void _getDisplayName() {
        boolean res = true;

        oObj.loadCalendar(calendars[0][0],installed_locales[0]);
        String DisplayName = oObj.getDisplayName(CalendarDisplayIndex.MONTH,
            newValue, (short) 0);
        res &= (DisplayName.length() == 3);
        tRes.tested("getDisplayName()", res);
    }


    /**
    * The test sets obviously wrong value, then calls a method. After that the
    * test sets correct value, and again calls a method. <p>
    * Has <b> OK </b> status if the method returns true when valid month is
    * set, and if the method returns false when set month is not valid.
    */
    public void _isValid() {
        boolean res = true;

        oObj.setValue(CalendarFieldIndex.MONTH, (short) 37);
        res &= !oObj.isValid();
        oObj.setValue(CalendarFieldIndex.MONTH, (short) 10);
        res &= oObj.isValid();

        tRes.tested("isValid()", res);
    }

    /**
    * Method returns locale for a given language and country.
    * @param localeIndex index of needed locale.
    */
/*    public Locale getLocale(int localeIndex) {
        return new Locale(languages[localeIndex], countries[localeIndex], "");
    }*/

}

