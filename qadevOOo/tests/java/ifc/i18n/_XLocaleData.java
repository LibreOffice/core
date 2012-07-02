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

import com.sun.star.i18n.Calendar;
import com.sun.star.i18n.Currency;
import com.sun.star.i18n.ForbiddenCharacters;
import com.sun.star.i18n.FormatElement;
import com.sun.star.i18n.Implementation;
import com.sun.star.i18n.LanguageCountryInfo;
import com.sun.star.i18n.LocaleDataItem;
import com.sun.star.i18n.XLocaleData;
import com.sun.star.lang.Locale;

/**
* Testing <code>com.sun.star.i18n.XLocaleData</code>
* interface methods:
* <ul>
*  <li><code> getLanguageCountryInfo() </code></li>
*  <li><code> getLocaleItem() </code></li>
*  <li><code> getAllCalendars() </code></li>
*  <li><code> getAllCurrencies() </code></li>
*  <li><code> getAllFormats() </code></li>
*  <li><code> getCollatorImplementations() </code></li>
*  <li><code> getSearchOptions() </code></li>
*  <li><code> getCollationOptions() </code></li>
*  <li><code> getTransliterations() </code></li>
*  <li><code> getForbiddenCharacters() </code></li>
*  <li><code> getReservedWord() </code></li>
*  <li><code> getAllInstalledLocaleNames() </code></li>
* </ul><p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.i18n.XLocaleData
*/
public class _XLocaleData extends MultiMethodTest {
    public XLocaleData oObj = null;
    public String[] languages = new String[]{"de","en","es","fr","ja","ko","ko"};
    public String[] countries = new String[]{"DE","US","ES","FR","JP","KR","KR"};


    /**
    * Test calls the method, then result is checked. <p>
    * Has <b> OK </b> status if structure, returned by the method includes
    * correct values of fields 'Language' and 'Country' for all given locales.
    */
    public void _getLanguageCountryInfo() {
        boolean res = true;
        LanguageCountryInfo lci = null;

        for (int i=0;i<7;i++) {
            lci = oObj.getLanguageCountryInfo(getLocale(i));
            /* For debug purposes
            log.println("Using: language="+languages[i]+" ; country="+countries[i]);
            log.println("Getting: ");
            log.println("\t Language="+lci.Language);
            log.println("\t LanguageDefaultName="+lci.LanguageDefaultName);
            log.println("\t Country="+lci.Country);
            log.println("\t CountryDefaultName="+lci.CountryDefaultName);
            boolean lang = ( lci.Language.equals(languages[i]) );
            if (!lang) log.println("getting false for language: "+lci.LanguageDefaultName);
            lang = ( lci.Country.equals(countries[i]) );
            if (!lang) log.println("getting false for country: "+lci.CountryDefaultName);
            */
            res &= ( ( lci.Language.equals(languages[i]) ) && ( lci.Country.equals(countries[i]) ) );
        }
        tRes.tested("getLanguageCountryInfo()",res);
    }

    /**
    * Test calls the method, then result is checked. <p>
    * Has <b> OK </b> status if structure, returned by the method consists of
    * non empty strings for all given locales.
    */
    public void _getLocaleItem() {
        boolean res = true;
        LocaleDataItem ldi = null;

        for (int i=0;i<7;i++) {
            ldi = oObj.getLocaleItem(getLocale(i));

            boolean locRes = true ;

            locRes &= (! ldi.dateSeparator.equals(""));
            locRes &= (! ldi.decimalSeparator.equals(""));
            locRes &= (! ldi.doubleQuotationEnd.equals(""));
            locRes &= (! ldi.doubleQuotationStart.equals(""));
            locRes &= (! ldi.listSeparator.equals(""));
            locRes &= (! ldi.LongDateDayOfWeekSeparator.equals(""));
            locRes &= (! ldi.LongDateDaySeparator.equals(""));
            locRes &= (! ldi.LongDateMonthSeparator.equals(""));
            locRes &= (! ldi.LongDateYearSeparator.equals(""));
            locRes &= (! ldi.measurementSystem.equals(""));
            locRes &= (! ldi.quotationEnd.equals(""));
            locRes &= (! ldi.quotationStart.equals(""));
            locRes &= (! ldi.thousandSeparator.equals(""));
            locRes &= (! ldi.time100SecSeparator.equals(""));
            locRes &= (! ldi.timeAM.equals(""));
            locRes &= (! ldi.timePM.equals(""));
            locRes &= (! ldi.timeSeparator.equals(""));
            locRes &= (! ldi.unoID.equals(""));

            if (!locRes) {
                /* for debugging puposes
                log.println("FAILED for: language="+languages[i]+" ; country="+countries[i]);
                log.println("Getting: ");
                log.println("\t DateSeparator="+ldi.dateSeparator);
                log.println("\t decimalSeparator="+ldi.decimalSeparator);
                log.println("\t doubleQuotationEnd="+ldi.doubleQuotationEnd);
                log.println("\t doubleQuotationStart="+ldi.doubleQuotationStart);
                log.println("\t listSeparator="+ldi.listSeparator);
                log.println("\t LongDateDayOfWeekSeparator="+ldi.LongDateDayOfWeekSeparator+"end");
                log.println("\t LongDateDaySeparator="+ldi.LongDateDaySeparator+"end");
                log.println("\t LongDateMonthSeparator="+ldi.LongDateMonthSeparator+"end");
                log.println("\t LongDateYearSeparator="+ldi.LongDateYearSeparator+"end");
                log.println("\t measurementSystem="+ldi.measurementSystem);
                log.println("\t quotationEnd="+ldi.quotationEnd);
                log.println("\t quotationStart="+ldi.quotationStart);
                log.println("\t thousandSeparator="+ldi.thousandSeparator);
                log.println("\t time100SecSeparator="+ldi.time100SecSeparator);
                log.println("\t timeAM="+ldi.timeAM);
                log.println("\t timePM="+ldi.timePM);
                log.println("\t timeSeparator="+ldi.timeSeparator);
                log.println("\t unoID="+ldi.unoID);
                 */
            }
        }
        tRes.tested("getLocaleItem()",res);
    }

    /**
    * Test calls the method for several locales; result is checked
    * after each call. <p>
    * Has <b> OK </b> status if all elements of the returned sequence are
    * correct for all given locales. (boolean method goodCalendar() with a
    * calendar as an argument returns true)
    */
    public void _getAllCalendars() {
        boolean res = true;
        boolean printit = false;
        Calendar[] calendar = new Calendar[1];

        for (int i=0;i<7;i++) {
            calendar = oObj.getAllCalendars(getLocale(i));
            for (int j=0;j<calendar.length;j++) {
                if (! goodCalendar(calendar[j]) ) {
                    printit = true;
                }
                res &= goodCalendar(calendar[j]);
            }
            if (printit) log.println("FAILED for: language="+languages[i]+" ; country="+countries[i]);
            printit = false;
        }
        tRes.tested("getAllCalendars()", res);
    }

    /**
    * Test calls the method for several locales; result is checked
    * after each call. <p>
    * Has <b> OK </b> status if all elements of the returned sequence are
    * correct for all given locales. (boolean method goodCurrency() with a
    * currency as an argument returns true)
    */
    public void _getAllCurrencies() {
        boolean res = true;
        boolean printit = false;
        Currency[] currency = new Currency[1];

        for (int i=0;i<7;i++) {
            currency = oObj.getAllCurrencies(getLocale(i));
            for (int j=0;j<currency.length;j++) {
                if (! goodCurrency(currency[j]) ) {
                    printit=true;
                }
                res &= goodCurrency(currency[j]);
            }
            if (printit) log.println("FAILED for: language="+languages[i]+" ; country="+countries[i]);
            printit =false;
        }
        tRes.tested("getAllCurrencies()",res);
    }

    /**
    * Test calls the method for several locales; result is checked
    * after each call. <p>
    * Has <b> OK </b> status if all elements of the returned sequence are
    * correct for all given locales. (boolean method goodFormat() with a
    * format as an argument returns true)
    */
    public void _getAllFormats() {
        boolean res = true;
        boolean printit = false;
        FormatElement[] format = new FormatElement[1];

        for (int i=0;i<7;i++) {
            format = oObj.getAllFormats(getLocale(i));
            for (int j=0;j<format.length;j++) {
                if (! goodFormat(format[j]) ) {
                    printit = true;
                }
                res &= goodFormat(format[j]);
            }
            if (printit) log.println("FAILED for: language="+languages[i]+" ; country="+countries[i]);
            printit =false;
        }
        tRes.tested("getAllFormats()",res);
    }

    /**
    * Test calls the method for several locales; result is checked
    * after each call. <p>
    * Has <b> OK </b> status if all structs, returned by the method have non
    * empty field 'UnoID' for all given locales.
    */
    public void _getCollatorImplementations() {
        boolean res = true;
        boolean printit = false;
        Implementation[] impl = new Implementation[1];

        for (int i=0;i<7;i++) {
            impl = oObj.getCollatorImplementations(getLocale(i));
            for (int j=0;j<impl.length;j++) {
                if ((impl[j].unoID.equals(""))) {
                    printit = true;
                }
                res &= (!impl[j].unoID.equals(""));
            }
            if (printit) log.println("FAILED for: language=" + languages[i]
                + " ; country=" + countries[i]);
            printit = false;
        }
        tRes.tested("getCollatorImplementations()", res);
    }

    /**
    * Test calls the method for several locales; result is checked
    * after each call. <p>
    * Has <b> OK </b> status if all strings, returned by the method are not
    * empty for all given locales.
    */
    public void _getSearchOptions() {
        boolean res = true;
        boolean printit = false;
        String[] str = new String[1];

        for (int i=0;i<7;i++) {
            str = oObj.getSearchOptions(getLocale(i));
            for (int j=0;j<str.length;j++) {
                if ((str[j].equals(""))) {
                    printit = true;
                }
                res &= (!str.equals(""));
            }
            if (printit) log.println("FAILED for: language=" + languages[i]
                + " ; country=" + countries[i]);
            printit = false;
        }
        tRes.tested("getSearchOptions()",res);
    }

    /**
    * Test calls the method for several locales; result is checked
    * after each call. <p>
    * Has <b> OK </b> status if all strings, returned by the method are not
    * empty for all given locales.
    */
    public void _getCollationOptions() {
        boolean res = true;
        boolean printit = false;
        String[] str = new String[1];

        for (int i=0;i<7;i++) {
            str = oObj.getCollationOptions(getLocale(i));
            for (int j=0;j<str.length;j++) {
                if ((str[j].equals(""))) {
                    printit = true;
                }
                res &= (!str.equals(""));
            }
            if (printit) log.println("FAILED for: language=" + languages[i]
                + " ; country=" + countries[i]);
            printit = false;
        }
        tRes.tested("getCollationOptions()", res);
    }

    /**
    * Test calls the method for several locales; result is checked
    * after each call. <p>
    * Has <b> OK </b> status if all strings, returned by the method are not
    * empty for all given locales.
    */
    public void _getTransliterations() {
        boolean res = true;
        boolean printit = false;
        String[] str = new String[1];

        for (int i=0;i<7;i++) {
            str = oObj.getTransliterations(getLocale(i));
            for (int j=0;j<str.length;j++) {
                if ((str[j].equals(""))) {
                    printit = true;
                }
                res &= (!str.equals(""));
            }
            if (printit) log.println("FAILED for: language=" + languages[i]
                + " ; country=" + countries[i]);
            printit = false;
        }
        tRes.tested("getTransliterations()", res);
    }

    /**
    * Test calls the method for several locales; result is checked
    * after each call. <p>
    * Has <b> OK </b> status if the method returns structure with non-empty
    * fields for all given locales.
    */
    public void _getForbiddenCharacters() {
        boolean res = true;
        ForbiddenCharacters fc = null;

        //the forbidden characters are only available for the asian languages
        for (int i=4;i<7;i++) {
            fc = oObj.getForbiddenCharacters(getLocale(i));
            res &= !( fc.beginLine.equals("") || fc.endLine.equals("") );
            if ( !res ) {
                log.println("FAILED for: language="+languages[i]+" ; country="+countries[i]);
            }
        }
        tRes.tested("getForbiddenCharacters()", res);
    }


    /**
    * Test calls the method for several locales; result is checked
    * after each call. <p>
    * Has <b> OK </b> status if all strings, returned by the method are not
    * empty for all given locales.
    */
    public void _getReservedWord() {
        boolean res = true;
        boolean printit = false;
        String[] str = new String[1];

        for (int i=0;i<7;i++) {
            str = oObj.getReservedWord(getLocale(i));
            for (int j=0;j<str.length;j++) {
                if ((str[j].equals(""))) {
                    printit = true;
                }
                res &= (!str.equals(""));
            }
            if (printit) log.println("FAILED for: language=" + languages[i]
                + " ; country=" + countries[i]);
            printit = false;
        }
        tRes.tested("getReservedWord()", res);
    }


    /**
    * Test calls the method. Then result is checked for all given locales.<p>
    * Has <b> OK </b> status if locale sequence, returned by the method contains
    * given locales.
    */
    public void _getAllInstalledLocaleNames() {
        boolean res = true;
        Locale[] locs = oObj.getAllInstalledLocaleNames();

        //check if the languages used here are part of this array
        for (int i=0;i<7;i++) {
            res &= contains(locs, getLocale(i));
        }
        tRes.tested("getAllInstalledLocaleNames()",res);
    }


    /**
    * Method returns locale for a given language and country.
    * @param k index of needed locale.
    * @return Locale by the index from arrays defined above
    */
    public Locale getLocale(int k) {
        return new Locale(languages[k],countries[k],"");
    }

    /**
    * Method checks given calendar for non empty fields.
    * @param calendar Calendar to be checked
    */
    public boolean goodCalendar(Calendar calendar) {
        boolean good = true;
        for (int i=0;i<calendar.Days.length;i++) {
            //log.println("Day "+i+"(AbbrevName): "+calendar.Days[i].AbbrevName);
            good &= (! calendar.Days[i].AbbrevName.equals("") );
            //log.println("Day "+i+"(FullName): "+calendar.Days[i].FullName);
            good &= (! calendar.Days[i].FullName.equals("") );
            //log.println("Day "+i+"(ID): "+calendar.Days[i].ID);
            good &= (! calendar.Days[i].ID.equals("") );
        }
        for (int i=0;i<calendar.Months.length;i++) {
            //log.println("Day "+i+"(AbbrevName): "+calendar.Months[i].AbbrevName);
            good &= (! calendar.Months[i].AbbrevName.equals("") );
            //log.println("Day "+i+"(FullName): "+calendar.Months[i].FullName);
            good &= (! calendar.Months[i].FullName.equals("") );
            //log.println("Day "+i+"(ID): "+calendar.Months[i].ID);
            good &= (! calendar.Months[i].ID.equals("") );
        }
        for (int i=0;i<calendar.Eras.length;i++) {
            //log.println("Era "+i+"(AbbrevName): "+calendar.Eras[i].AbbrevName);
            good &= (! calendar.Eras[i].AbbrevName.equals("") );
            //log.println("Era "+i+"(FullName): "+calendar.Eras[i].FullName);
            good &= (! calendar.Eras[i].FullName.equals("") );
            //log.println("Era "+i+"(ID): "+calendar.Eras[i].ID);
            good &= (! calendar.Eras[i].ID.equals("") );
        }
        //log.println("Start of Week: "+calendar.StartOfWeek);
        good &= (! calendar.StartOfWeek.equals("") );
        //log.println("MinimumNumberOfDaysForFirstWeek: "+calendar.MinimumNumberOfDaysForFirstWeek);
        //log.println("Default: "+calendar.Default);
        //log.println("Name: "+calendar.Name);
        good &= (! calendar.Name.equals("") );
        return good;
    }

    /**
    * Method checks given currency for non empty fields.
    * @param currency Currency to be checked
    */
    public boolean goodCurrency(Currency currency) {
        boolean good = true;
        good &= (!currency.BankSymbol.equals(""));
        good &= (!currency.Name.equals(""));
        good &= (!currency.Symbol.equals(""));
        return good;
    }

    /**
    * Method checks given format for non empty fields.
    * @param format Format to be checked
    */
    public boolean goodFormat(FormatElement format) {
        boolean good = true;
        good &= (!format.formatCode.equals(""));
        good &= (!format.formatKey.equals(""));
        good &= (!format.formatType.equals(""));
        good &= (!format.formatUsage.equals(""));
        return good;
    }

    /**
    * Method checks that locale sequence contains given locale.
    * @param locs Locale sequence
    * @param oneLoc given locale
    */
    public boolean contains(Locale[] locs, Locale oneLoc) {
        boolean cont = false;
        for (int j=0;j<locs.length;j++) {
            if ( (locs[j].Country.equals(oneLoc.Country)) &&
                (locs[j].Language.equals(oneLoc.Language)) ) {
                    cont=true;
                    break;
            }
        }
        if (!cont) log.println("Not contained: " + oneLoc.Language);
        return cont;
    }

} // end XLocaleData

