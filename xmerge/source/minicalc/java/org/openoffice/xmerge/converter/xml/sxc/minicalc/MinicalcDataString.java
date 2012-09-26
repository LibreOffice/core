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

package org.openoffice.xmerge.converter.xml.sxc.minicalc;

/**
 *  This class is used by <code>MinicalcDecoder</code> to manipulate a
 *  <code>String</code> containing MiniCalc cell data.
 *
 */
public class MinicalcDataString {

    /** The String representation of the MiniCalc data. */
    private String data = null;


    /**
     * Constructor stores the MiniCalc data <code>String</code>.
     *
     *  @param  data  A <code>String</code> containing MiniCalc
     *                cell data.
     */
    public MinicalcDataString(String data) {
        this.data = data;
    }


    /**
     *  Checks if the MiniCalc data <code>String</code> is a <i>formula</i>.
     *
     *  @return  true if the MiniCalc data <code>String</code> is a
     *           <i>formula</i>, false if the MiniCalc data <code>String</code>
     *           is not a <i>formula</i>.
     */
    public boolean isFormula() {

        if (data.startsWith("=")) {
            return true;
        }

        return false;
    }


    /**
     *  Checks if the MiniCalc data <code>String</code> is a <i>percentage</i>.
     *
     *  @return  true if the MiniCalc data <code>String</code> is a
     *           <i>percentage</i>, false if the MiniCalc data
     *           <code>String</code> is not a <i>percentage</i>.
     */
    public boolean isPercent() {

        if (data.endsWith("%")) {
            return true;
        }

        return false;
    }


    /**
     *  Checks if the MiniCalc data <code>String</code> is a
     *  <i>boolean</i> value.
     *
     *  @return  true if the MiniCalc data <code>String</code> is
     *           a <i>boolean</i>, false if the MiniCalc data
     *           <code>String</code> is not a <i>boolean</i>.
     */
    public boolean isBoolean() {

        if (data.equalsIgnoreCase("false") ||
            data.equalsIgnoreCase("true")) {
            return true;
        }

        return false;
    }


    /**
     *  Checks if the MiniCalc data <code>String</code> is a <i>date</i>.
     *
     *  @return  true if the MiniCalc data <code>String</code> is
     *           a <i>date</i>, false if the MiniCalc data <code>String</code>
     *           is not a <i>date</i>.
     */
    public boolean isDate() {

        // Starting index into the date string - month
        int start = 0;

        // Search for "/", which separates month from day
        int end = data.indexOf("/");

        // Separator was found
        if (end > 0) {

            String monthString = data.substring(start, end);

            try {
                Float f = Float.valueOf(monthString);
                if ((f.intValue() < 0) || (f.intValue() > 12)) {
                    return false;
                }
            }
            catch (NumberFormatException e) {

                // no, it is not a currency type
                return false;
            }

            // start is now the starting index of day
            start = end+1;

            // Search for "/", which separates day from year
            end = data.indexOf("/", start);

            // Separator was found
            if (end > 0) {

                String dayString = data.substring(start, end);

                try {
                    Float f = Float.valueOf(dayString);
                    if ((f.intValue() < 0) || (f.intValue() > 31))
                        return false;
                }
                catch (NumberFormatException e) {
                    // no, it is not a currency type
                    return false;
                }
            } else {
                return false;
            }

            // start is now at the starting index of the year
            start = end + 1;

            String yearString = data.substring(start);
            try {
                Float f = Float.valueOf(yearString);
                if (f.intValue() < 0) {
                    return false;
                }
            }
            catch (NumberFormatException e) {
                // no, it is not a currency type
                return false;
            }

        } else {
            return false;
        }

        return true;
    }


    /**
     *  Checks if the MiniCalc data <code>String</code> is a <i>time</i>.
     *
     *  @return  true if the MiniCalc data <code>String</code>
     *           is a <i>time</i>, false if the MiniCalc data
     *           <code>String</code> is not a <i>time</i>.
     */
    public boolean isTime() {

        // Starting index into the time string - hour
        int start = 0;

        // Search for ":", which separates hour from minute
        int end = data.indexOf(":");


        // Separator was found
        if (end > 0) {

            String hourString = data.substring(start, end);
            try {
                Float f = Float.valueOf(hourString);
                if ((f.intValue() < 0) || (f.intValue() > 24))
                    return false;
            }
            catch (NumberFormatException e) {
                // no, it is not a time type
                return false;
            }

            // start is now the starting index of minute
            start = end+1;

            // Search for ":", which separates minute from second
            end = data.indexOf(":", start);

            // Separator was found
            if (end > 0) {

                String minuteString = data.substring(start, end);

                try {
                    Float f = Float.valueOf(minuteString);
                    if ((f.intValue() < 0) || (f.intValue() > 60))
                        return false;
                }
                catch (NumberFormatException e) {
                    // no, it is not a time type
                    return false;
                }

                // start is now at the starting index of the seconds
                start = end+1;

                // The seconds are in the string
                if (data.length() > start) {

                    String secondString = data.substring(start);


                    try {
                        Float f = Float.valueOf(secondString);
                        if ((f.intValue() < 0) || (f.intValue() > 60))
                            return false;
                    }
                    catch (NumberFormatException e) {
                        // no, it is not a time type
                        return false;
                    }
                }

            }

            return true;

        }

        return false;
    }


    /**
     *  Checks if the MiniCalc data <code>String</code> is a <i>currency</i>
     *  value.
     *
     *  @return  true if the MiniCalc data <code>String</code> is
     *           a <i>currency</i>, false if the MiniCalc data
     *           <code>String</code> is not a <i>currency</i>.
     */
    public boolean isCurrency() {

        boolean result = false;

        // TODO - we currently only check for US currencies

        if (data.endsWith("$")) {
            String number = data.substring(0, data.length()-1);
            try {
                Float f = Float.valueOf(number);
                result = true;
            }
            catch (NumberFormatException e) {
                // no, it is not a currency type
                result = false;
            }
        }

        else if (data.startsWith("$")) {
            String number = data.substring(1, data.length());
            try {
                Float f = Float.valueOf(number);
                result = true;
            }
            catch (NumberFormatException e) {
                // no, it is not a currency type
                result = false;
            }
        }

        return result;

    }


    /**
     *  This method removes the percent sign from the MiniCalc data
     *  <code>String</code>.  If the percent sign is not the last
     *  character of the MiniCalc data <code>String</code>, the
     *  MiniCalc data <code>String</code> is returned.
     *
     *  @return  The MiniCalc data <code>String</code> minus the
     *           percent sign.  If the MiniCalc data <code>String</code>
     *           does not begin with a percent sign, the MiniCalc data
     *           <code>String</code> is returned.
     */
    public String percentRemoveSign() {

        String number = data;

        if (data.endsWith("%")) {
            // "%" is the last character, so remove
            number = data.substring(0, data.length()-1);

            try {
                Float f = Float.valueOf(number);
                float f1 = f.floatValue()/100f;
                Float f2 = new Float(f1);
                number =  f2.toString();
            }
            catch (NumberFormatException e) {
                // no, it is not a float type
            }
        }

        return number;
    }


    /**
     *  This method removes the currency sign from the MiniCalc data
     *  <code>String</code>.  If the currency sign is not the first or
     *  last character of the MiniCalc data <code>String</code>, the
     *  MiniCalc data <code>String</code> is returned.
     *
     *  @return  The MiniCalc data <code>String</code> minus the currency
     *           sign.  If the MiniCalc data <code>String</code> does not
     *           begin or end with a currency sign, the MiniCalc
     *           data <code>String</code> is returned.
     */
    public String currencyRemoveSign() {

        String number = data;

        // TODO - only works with US currencies

        if (data.endsWith("$")) {

            number = data.substring(0, data.length()-1);

        } else if (data.startsWith("$")) {

            number = data.substring(1, data.length());
        }

        return number;

    }


    /**
     *  <p>This method converts a MiniCalc date from MiniCalc
     *  format to StarOffice XML format.</p>
     *
     *  <p>MiniCalc format:</p>
     *
     *  <p><blockquote>
     *  MM/DD/YY or MM/DD/YYYY
     *  </blockquote></p>
     *
     *  <p>StarOffice XML format:</p>
     *
     *  <p><blockquote>
     *  YYYY-MM-DD
     *  </blockquote></p>
     *
     *  @return  The MiniCalc date converted to StarOffice XML
     *           format.
     */
    public String convertToStarDate() {

        // The output date string
        String out;

        String monthString = "01";
        String dayString = "01";
        String yearString = "1900";

        // Starting index into the date string - month
        int start = 0;

        // Search for "/", which separates month from day
        int end = data.indexOf("/");

        // Separator was found
        if (end > 0) {

            monthString = data.substring(start, end);

            Integer monthInt = new Integer(monthString);

            // Make sure month is 2 digits
            if (monthInt.intValue() < 10) {
                monthString = "0" + monthString;
            }

            // start is now the starting index of day
            start = end+1;

            // Search for "/", which separates day from year
            end = data.indexOf("/", start);

            // Separator was found
            if (end > 0) {

                dayString = data.substring(start, end);

                Integer dayInt = new Integer(dayString);

                // Make sure day is 2 digits
                if (dayInt.intValue() < 10) {
                    dayString = "0" + dayString;
                }

                // start is now at the starting index of the year
                start = end + 1;

                // The year is in the string
                if (data.length() > start) {

                    yearString = data.substring(start);

                    Integer yearInt = new Integer(yearString);
                    int year = yearInt.intValue();

                    if (year < 31) {

                        // MiniCalc years between 0 and 30 correspond to
                        // 2000 - 2030
                        year += 2000;

                    } else if (year < 100) {

                        // MiniCalc years between 31 and 99 correspond
                        // to 1931 - 1999
                        year += 1900;
                    }

                    yearString = Integer.toString(year);
                }
            }
        }

        // Set out to StarOffice XML date format
        out = yearString + "-" + monthString + "-" + dayString;

        return out;
    }


    /**
     *  This method converts the MiniCalc time from MiniCalc
     *  format to StarOffice XML format.
     *
     *  <p>MiniCalc format:</p>
     *
     *  <p><blockquote>
     *  hh:mm:ss
     *  </blockquote></p>
     *
     *  <p>StarOffice XML format:</p>
     *
     *  <p><blockquote>
     *  PThhHmmMssS
     *  </blockquote></p>
     *
     *  @return   The MiniCalc time converted to StarOffice XML
     *            format.
     */
    public String convertToStarTime() {

        // The output time string
        String out;

        String hourString = "00";
        String minuteString = "00";
        String secondString = "00";

        // Starting index into the time string - hour
        int start = 0;

        // Search for ":", which separates hour from minute
        int end = data.indexOf(":");

        // Separator was found
        if (end > 0) {

            hourString = data.substring(start, end);

            // start is now the starting index of minute
            start = end+1;

            // Search for ":", which separates minute from second
            end = data.indexOf(":", start);

            // Separator was found
            if (end > 0) {

                minuteString = data.substring(start, end);

                // start is now at the starting index of the seconds
                start = end+1;

                // The seconds are in the string
                if (data.length() > start) {

                    secondString = data.substring(start);
                }

            }
        }

        // TODO -  PT is for pacific time, where can we get the
        // localized value from?

        // Set to StarOffice XML time format
        out = "PT"+hourString+"H"+minuteString+"M"+secondString+"S";

        return out;
    }
}

