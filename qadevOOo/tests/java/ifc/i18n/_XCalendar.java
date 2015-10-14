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

import lib.MultiMethodTest;

import com.sun.star.i18n.CalendarDisplayIndex;
import com.sun.star.i18n.CalendarFieldIndex;
import com.sun.star.i18n.CalendarItem;
import com.sun.star.i18n.XCalendar;
import com.sun.star.i18n.XLocaleData;
import com.sun.star.lang.Locale;
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
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.i18n.XCalendar
*/
public class _XCalendar extends MultiMethodTest {
    private boolean debug = false;
    public XCalendar oObj = null;
    public String[][] calendars;
    public int[] count;
    public double newDTime = 1000.75;
    public short newValue = 2;
    public short firstDay = 2;
    public short mdfw = 3;
    double aOriginalDTime = 0;
    Locale[] installed_locales;

    @Override
    public void before() {
        XLocaleData locData = null;
        try {
            locData = UnoRuntime.queryInterface(
                XLocaleData.class,
                    tParam.getMSF().createInstance(
                    "com.sun.star.i18n.LocaleData"));
        } catch (com.sun.star.uno.Exception e) {

        }
        installed_locales = locData.getAllInstalledLocaleNames();
        calendars = new String[installed_locales.length][];
        count = new int[installed_locales.length];
        oObj.loadDefaultCalendar(installed_locales[0]);
        aOriginalDTime = oObj.getDateTime();

        debug = tParam.getBool("DebugIsActive");
    }

    /**
     * Restore the changed time during the test to the original value of the
     * machine: has to be correct for the following interface tests.
     */
    @Override
    public void after() {
        oObj.loadDefaultCalendar(installed_locales[0]);
        oObj.setDateTime(aOriginalDTime);
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
        for (int i = 0; i < installed_locales.length; i++) {
            String error = "";
            String lang = "Language: " + installed_locales[i].Language + ", Country: " + installed_locales[i].Country
                    + ", Variant: " + installed_locales[i].Variant + ", Name: " + calendars[i][count[i]];
            String[] names = new String[] { "DAY_OF_MONTH", "HOUR", "MINUTE", "SECOND", "MILLISECOND", "YEAR",
                    "MONTH" };
            oObj.loadCalendar(calendars[i][count[i]], installed_locales[i]);
            short[] fields = new short[] { CalendarFieldIndex.DAY_OF_MONTH, CalendarFieldIndex.HOUR,
                    CalendarFieldIndex.MINUTE, CalendarFieldIndex.SECOND, CalendarFieldIndex.MILLISECOND,
                    CalendarFieldIndex.YEAR, CalendarFieldIndex.MONTH };
            for (int k = 0; k < fields.length; k++) {

                oObj.setDateTime(0.0);

                // save the current values for debug purposes
                short[] oldValues = new short[fields.length];
                for (int n = 0; n < oldValues.length; n++) {
                    oldValues[n] = oObj.getValue(fields[n]);
                }

                short set = oObj.getValue(fields[k]);
                if (fields[k] == CalendarFieldIndex.MONTH)
                    set = newValue;
                oObj.setValue(fields[k], set);
                short get = oObj.getValue(fields[k]);
                if (get != set) {
                    if (debug)
                        log.println("ERROR occurred: tried to set " + names[k] + " to value " + set);
                    log.println("list of values BEFORE set " + names[k] + " to value " + set + ":");
                    for (int n = 0; n < oldValues.length; n++) {
                        log.println(names[n] + ":" + oldValues[n]);
                    }
                    log.println("list of values AFTER set " + names[k] + " to value " + set + ":");
                    for (int n = 0; n < fields.length; n++) {
                        log.println(names[n] + ":" + oObj.getValue(fields[n]));
                    }

                    error += "failed for " + names[k] + " expected " + set + " gained " + get + " ; \n";
                }
            }
            if (error.equals("")) {
                log.println(lang + " ... OK");
            } else {
                log.println("*** " + lang + " ... FAILED ***");
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
        if (!res){
            log.println("the returned value is not the expected value:");
            log.println("expexted: " + newValue + "  returned value: " + aValue);
        }
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
        if (!res){
            log.println("the returned value is not the expected value:");
            log.println("expexted: " + newValue + "  returned value: " + aValue);
        }
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

        oObj.loadDefaultCalendar(installed_locales[0]);
        oObj.setValue(CalendarFieldIndex.MONTH, (short) 37);
        res &= !oObj.isValid();
        oObj.setValue(CalendarFieldIndex.MONTH, (short) 10);
        res &= oObj.isValid();

        tRes.tested("isValid()", res);
    }

}

