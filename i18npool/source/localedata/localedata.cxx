/*************************************************************************
 *
 *  $RCSfile: localedata.cxx,v $
 *
 *  $Revision: 1.28 $
 *
 *  last change: $Author: kz $ $Date: 2004-07-30 14:40:02 $
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

#include <localedata.hxx>

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#include <string.h>
#include <stdio.h>

#if OSL_DEBUG_LEVEL == 0
#  ifndef NDEBUG
#    define NDEBUG
#  endif
#endif
#include <assert.h>

using namespace com::sun::star::i18n;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star;
using namespace rtl;

static const sal_Char clocaledata[] = "com.sun.star.i18n.LocaleData";
static sal_Char charBuffer[256];

typedef sal_Unicode**   (SAL_CALL * MyFunc_Type)( sal_Int16&);
typedef sal_Unicode***  (SAL_CALL * MyFunc_Type2)( sal_Int16&, sal_Int16& );
typedef sal_Unicode**** (SAL_CALL * MyFunc_Type3)( sal_Int16&, sal_Int16&, sal_Int16& );
typedef sal_Unicode**   (SAL_CALL * MyFunc_FormatCode)( sal_Int16&, sal_Unicode*&, sal_Unicode*& );

static const char *lcl_DATA_EN = "localedata_en";
static const char *lcl_DATA_ES = "localedata_es";
static const char *lcl_DATA_EURO = "localedata_euro";
static const char *lcl_DATA_OTHERS = "localedata_others";

static const struct {
        const char* pLocale;
        const char* pDLL;
        const char* lLocale;
} aDllsTable[] = {
        { "en_US",  lcl_DATA_EN, "en" },
        { "en_AU",  lcl_DATA_EN, NULL },
        { "en_BZ",  lcl_DATA_EN, NULL },
        { "en_CA",  lcl_DATA_EN, NULL },
        { "en_GB",  lcl_DATA_EN, NULL },
        { "en_IE",  lcl_DATA_EN, NULL },
        { "en_JM",  lcl_DATA_EN, NULL },
        { "en_NZ",  lcl_DATA_EN, NULL },
        { "en_PH",  lcl_DATA_EN, NULL },
        { "en_TT",  lcl_DATA_EN, NULL },
        { "en_ZA",  lcl_DATA_EN, NULL },
        { "en_ZW",  lcl_DATA_EN, NULL },
        { "en_CB",  lcl_DATA_EN, NULL },

        { "es_ES",  lcl_DATA_ES, "es" },
        { "es_AR",  lcl_DATA_ES, NULL },
        { "es_BO",  lcl_DATA_ES, NULL },
        { "es_CL",  lcl_DATA_ES, NULL },
        { "es_CO",  lcl_DATA_ES, NULL },
        { "es_CR",  lcl_DATA_ES, NULL },
        { "es_DO",  lcl_DATA_ES, NULL },
        { "es_EC",  lcl_DATA_ES, NULL },
        { "es_GT",  lcl_DATA_ES, NULL },
        { "es_HN",  lcl_DATA_ES, NULL },
        { "es_MX",  lcl_DATA_ES, NULL },
        { "es_NI",  lcl_DATA_ES, NULL },
        { "es_PA",  lcl_DATA_ES, NULL },
        { "es_PE",  lcl_DATA_ES, NULL },
        { "es_PR",  lcl_DATA_ES, NULL },
        { "es_PY",  lcl_DATA_ES, NULL },
        { "es_SV",  lcl_DATA_ES, NULL },
        { "es_UY",  lcl_DATA_ES, NULL },
        { "es_VE",  lcl_DATA_ES, NULL },

        { "de_DE",  lcl_DATA_EURO, "de" },
        { "de_AT",  lcl_DATA_EURO, NULL },
        { "de_CH",  lcl_DATA_EURO, NULL },
        { "de_LI",  lcl_DATA_EURO, NULL },
        { "de_LU",  lcl_DATA_EURO, NULL },
        { "fr_FR",  lcl_DATA_EURO, "fr" },
        { "fr_BE",  lcl_DATA_EURO, NULL },
        { "fr_CA",  lcl_DATA_EURO, NULL },
        { "fr_CH",  lcl_DATA_EURO, NULL },
        { "fr_LU",  lcl_DATA_EURO, NULL },
        { "fr_MC",  lcl_DATA_EURO, NULL },
        { "it_IT",  lcl_DATA_EURO, "it" },
        { "it_CH",  lcl_DATA_EURO, NULL },
        { "sl_SI",  lcl_DATA_EURO, "sl" },
        { "sv_SE",  lcl_DATA_EURO, "sv" },
        { "sv_FI",  lcl_DATA_EURO, NULL },
        { "ca_ES",  lcl_DATA_EURO, "ca" },
        { "cs_CZ",  lcl_DATA_EURO, "cs" },
        { "sk_SK",  lcl_DATA_EURO, "sk" },
        { "da_DK",  lcl_DATA_EURO, "da" },
        { "el_GR",  lcl_DATA_EURO, "el" },
        { "fi_FI",  lcl_DATA_EURO, "fi" },
        { "is_IS",  lcl_DATA_EURO, "is" },
        { "nl_BE",  lcl_DATA_EURO, NULL },
        { "nl_NL",  lcl_DATA_EURO, "nl" },
        { "no_NO",  lcl_DATA_EURO, "no" },
        { "nn_NO",  lcl_DATA_EURO, "nn" },
        { "nb_NO",  lcl_DATA_EURO, "nb" },
        { "pl_PL",  lcl_DATA_EURO, "pl" },
        { "pt_BR",  lcl_DATA_EURO, "pt" },
        { "pt_PT",  lcl_DATA_EURO, NULL },
        { "ru_RU",  lcl_DATA_EURO, "ru" },
        { "tr_TR",  lcl_DATA_EURO, "tr" },
        { "et_EE",  lcl_DATA_EURO, "et" },
        { "lt_LT",  lcl_DATA_EURO, "lt" },
        { "lv_LV",  lcl_DATA_EURO, "lv" },
        { "uk_UA",  lcl_DATA_EURO, "uk" },
        { "ro_RO",  lcl_DATA_EURO, "ro" },
        { "cy_GB",  lcl_DATA_EURO, "cy" },
        { "bg_BG",  lcl_DATA_EURO, "bg" },
        { "sh_YU",  lcl_DATA_EURO, "sh" },
        { "sr_YU",  lcl_DATA_EURO, "sr" },
        { "hr_HR",  lcl_DATA_EURO, "hr" },
        { "bs_BA",  lcl_DATA_EURO, "bs" },

        { "ja_JP",  lcl_DATA_OTHERS, "ja" },
        { "ko_KR",  lcl_DATA_OTHERS, "ko" },
        { "zh_CN",  lcl_DATA_OTHERS, "zh" },
        { "zh_HK",  lcl_DATA_OTHERS, NULL },
        { "zh_SG",  lcl_DATA_OTHERS, NULL },
        { "zh_TW",  lcl_DATA_OTHERS, NULL },
        { "zh_MO",  lcl_DATA_OTHERS, NULL },

        { "ar_EG",  lcl_DATA_OTHERS, "ar" },
        { "ar_LB",  lcl_DATA_OTHERS, NULL },
        { "ar_SA",  lcl_DATA_OTHERS, NULL },
        { "ar_TN",  lcl_DATA_OTHERS, NULL },
        { "he_IL",  lcl_DATA_OTHERS, "he" },
        { "hi_IN",  lcl_DATA_OTHERS, "hi" },
        { "kn_IN",  lcl_DATA_OTHERS, "kn" },
        { "ta_IN",  lcl_DATA_OTHERS, "ta" },
        { "te_IN",  lcl_DATA_OTHERS, "te" },
        { "gu_IN",  lcl_DATA_OTHERS, "gu" },
        { "mr_IN",  lcl_DATA_OTHERS, "mr" },
        { "pa_IN",  lcl_DATA_OTHERS, "pa" },
        { "th_TH",  lcl_DATA_OTHERS, "th" },

        { "af_ZA",  lcl_DATA_OTHERS, "af" },
        { "hu_HU",  lcl_DATA_OTHERS, "hu" },
        { "id_ID",  lcl_DATA_OTHERS, "id" },
        { "ms_MY",  lcl_DATA_OTHERS, "ms" },
        { "ia",     lcl_DATA_OTHERS, "ia" },
        { "mn_MN",  lcl_DATA_OTHERS, "mn" },
        { "az_AZ",  lcl_DATA_OTHERS, "az" },

//      { "be_BY",  lcl_DATA, "be" },
//      { "br_AE",  lcl_DATA, "br" },

};

static const sal_Unicode under = sal_Unicode('_');

static const sal_Int16 nbOfLocales = sizeof(aDllsTable) / sizeof(aDllsTable[0]);

LocaleData::~LocaleData(){
        for (sal_Int32 l = 0; l < lookupTable.size(); l++) {
            cachedItem = lookupTable[l];
            delete cachedItem->module;
            delete cachedItem;
        }
        lookupTable.clear();
}


LocaleDataItem SAL_CALL
LocaleData::getLocaleItem( const Locale& rLocale ) throw(RuntimeException)
{
        sal_Int16 dataItemCount = 0;
        sal_Unicode **dataItem = NULL;

        MyFunc_Type func = (MyFunc_Type) getFunctionSymbol( rLocale, "getLocaleItem" );

        if ( func ) {
            dataItem = func(dataItemCount);

            LocaleDataItem item(
                    dataItem[0],
                    dataItem[1],
                    dataItem[2],
                    dataItem[3],
                    dataItem[4],
                    dataItem[5],
                    dataItem[6],
                    dataItem[7],
                    dataItem[8],
                    dataItem[9],
                    dataItem[10],
                    dataItem[11],
                    dataItem[12],
                    dataItem[13],
                    dataItem[14],
                    dataItem[15],
                    dataItem[16],
                    dataItem[17]
                    );
            return item;
        }
        else {
            LocaleDataItem item1;
            return item1;
        }
}

#define REF_DAYS 0
#define REF_MONTHS 1
#define REF_ERAS 2

Sequence< CalendarItem > &LocaleData::getCalendarItemByName(const OUString& name,
        const Locale& rLocale, const Sequence< Calendar >& calendarsSeq, sal_Int16 len, sal_Int16 item)
        throw(RuntimeException)
{
        if (!ref_name.equals(name)) {
            sal_Int32 index = 0;
            OUString language = name.getToken(0, under, index);
            OUString country = name.getToken(0, under, index);
            Locale loc(language, country, OUString());
            Sequence < Calendar > cals;
            if (loc == rLocale) {
                cals = calendarsSeq;
            } else {
                cals = getAllCalendars(loc);
                len = cals.getLength();
            }
            const OUString& id = name.getToken(0, under, index);
            for (index = 0; index < cals.getLength(); index++) {
                if (id.equals(cals[index].Name)) {
                    ref_cal = cals[index];
                    break;
                }
            }
            // Refered locale does not found, return name for en_US locale.
            if (index == cals.getLength()) {
                cals = getAllCalendars(
                        Locale(OUString::createFromAscii("en"), OUString::createFromAscii("US"), OUString()));
                if (cals.getLength() > 0)
                    ref_cal = cals[0];
                else
                    throw RuntimeException();
            }
            ref_name = name;
        }
        return item == REF_DAYS ? ref_cal.Days : item == REF_MONTHS ? ref_cal.Months : ref_cal.Eras;
}


Sequence< Calendar > SAL_CALL
LocaleData::getAllCalendars( const Locale& rLocale ) throw(RuntimeException)
{

        sal_Int16 calendarsCount = 0;
        sal_Unicode **allCalendars = NULL;

        MyFunc_Type func = (MyFunc_Type) getFunctionSymbol( rLocale, "getAllCalendars" );

        if ( func ) {
            allCalendars = func(calendarsCount);

            Sequence< Calendar > calendarsSeq(calendarsCount);
            sal_Int16 offset = 3;
            sal_Int16 i, j;
            for(i = 0; i < calendarsCount; i++) {
                Sequence< CalendarItem > days(allCalendars[0][i]);
                Sequence< CalendarItem > months(allCalendars[1][i]);
                Sequence< CalendarItem > eras(allCalendars[2][i]);
                OUString calendarID(allCalendars[offset]);
                offset++;
                sal_Bool defaultCalendar = allCalendars[offset][0];
                offset++;
                if (OUString(allCalendars[offset]).equalsAscii("ref")) {
                    days = getCalendarItemByName(OUString(allCalendars[offset+1]), rLocale, calendarsSeq, i, REF_DAYS);
                    offset += 2;
                } else {
                    for(j = 0; j < allCalendars[0][i]; j++) {
                        CalendarItem day(allCalendars[offset],
                            allCalendars[offset+1], allCalendars[offset+2]);
                        days[j] = day;
                        offset += 3;
                    }
                }
                if (OUString(allCalendars[offset]).equalsAscii("ref")) {
                    months = getCalendarItemByName(OUString(allCalendars[offset+1]), rLocale, calendarsSeq, i, REF_MONTHS);
                    offset += 2;
                } else {
                    for(j = 0; j < allCalendars[1][i]; j++) {
                        CalendarItem month(allCalendars[offset],
                            allCalendars[offset+1], allCalendars[offset+2]);
                        months[j] = month;
                        offset += 3;
                    }
                }
                if (OUString(allCalendars[offset]).equalsAscii("ref")) {
                    eras = getCalendarItemByName(OUString(allCalendars[offset+1]), rLocale, calendarsSeq, i, REF_ERAS);
                    offset += 2;
                } else {
                    for(j = 0; j < allCalendars[2][i]; j++) {
                        CalendarItem era(allCalendars[offset],
                            allCalendars[offset+1], allCalendars[offset+2]);
                        eras[j] = era;
                        offset += 3;
                    }
                }
                OUString startOfWeekDay(allCalendars[offset]);
                offset++;
                sal_Int16 minimalDaysInFirstWeek = allCalendars[offset][0];
                offset++;
                Calendar aCalendar(days, months, eras, startOfWeekDay,
                        minimalDaysInFirstWeek, defaultCalendar, calendarID);
                calendarsSeq[i] = aCalendar;
            }
            return calendarsSeq;
        }
        else {
            Sequence< Calendar > seq1(0);
            return seq1;
        }
}


Sequence< Currency > SAL_CALL
LocaleData::getAllCurrencies( const Locale& rLocale ) throw(RuntimeException)
{

        sal_Int16 currencyCount = 0;
        sal_Unicode **allCurrencies = NULL;

        MyFunc_Type func = (MyFunc_Type) getFunctionSymbol( rLocale, "getAllCurrencies" );

        if ( func ) {
            allCurrencies = func(currencyCount);

            Sequence< Currency > seq(currencyCount);
            for(int i = 0, nOff = 0; i < currencyCount; i++, nOff += 7 ) {
                Currency cur(
                    allCurrencies[nOff],        // string ID
                    allCurrencies[nOff+1],      // string Symbol
                    allCurrencies[nOff+2],      // string BankSymbol
                    allCurrencies[nOff+3],      // string Name
                    allCurrencies[nOff+4][0],   // boolean Default
                    allCurrencies[nOff+5][0],   // boolean UsedInCompatibleFormatCodes
                    allCurrencies[nOff+6][0]    // short DecimalPlaces
                    );
                    seq[i] = cur;
            }
            return seq;
        }
        else {
            Sequence< Currency > seq1(0);
            return seq1;
        }
}

// return a string resulting from replacing all occurrences of 'oldStr' string
// in 'formatCode' string with 'newStr' string
static sal_Unicode* SAL_CALL replace(sal_Unicode *formatCode, sal_Unicode* oldStr, sal_Unicode* newStr)
{
// make reasonable assumption of maximum length of formatCode.
#define MAX_FORMATCODE_LENTH 512
    static sal_Unicode str[MAX_FORMATCODE_LENTH];

    if (oldStr[0] == 0) // no replacement requires
        return formatCode;

    sal_Int32 i = 0, k = 0;
    while (formatCode[i] > 0 && k < MAX_FORMATCODE_LENTH) {
        sal_Int32 j = 0, last = k;
        // search oldStr in formatCode
        while (formatCode[i] > 0 && oldStr[j] > 0 && k < MAX_FORMATCODE_LENTH) {
            str[k++] = formatCode[i];
            if (formatCode[i++] != oldStr[j++])
                break;
        }
        if (oldStr[j] == 0) {
            // matched string found, do replacement
            k = last; j = 0;
            while (newStr[j] > 0 && k < MAX_FORMATCODE_LENTH)
                str[k++] = newStr[j++];
        }
    }
    if (k >= MAX_FORMATCODE_LENTH) // could not complete replacement, return original formatCode
        return formatCode;

    str[k] = 0;
    return str;
}

Sequence< FormatElement > SAL_CALL
LocaleData::getAllFormats( const Locale& rLocale ) throw(RuntimeException)
{
        sal_Int16 formatCount = 0;
        sal_Unicode *from, *to;
        sal_Unicode **formatArray = NULL;

        MyFunc_FormatCode func = (MyFunc_FormatCode) getFunctionSymbol( rLocale, "getAllFormats" );

        if ( func ) {
            formatArray = func(formatCount, from, to);

            Sequence< FormatElement > seq(formatCount);
            for(int i = 0, nOff = 0; i < formatCount; i++, nOff += 7 ) {
                FormatElement elem(replace(formatArray[nOff], from, to),
                formatArray[nOff+ 1],
                formatArray[nOff + 2],
                formatArray[nOff + 3],
                formatArray[nOff + 4],
                formatArray[nOff + 5][0],
                formatArray[nOff + 6][0]);
                seq[i] = elem;
            }
            return seq;
        }
        else {
            Sequence< FormatElement > seq1(0);
            return seq1;
        }

}

Sequence< Implementation > SAL_CALL
LocaleData::getCollatorImplementations( const Locale& rLocale ) throw(RuntimeException)
{
        sal_Int16 collatorCount = 0;
        sal_Unicode **collatorArray = NULL;

        MyFunc_Type func = (MyFunc_Type) getFunctionSymbol( rLocale, "getCollatorImplementation" );

        if ( func ) {
            collatorArray = func(collatorCount);
            Sequence< Implementation > seq(collatorCount);
            for(sal_Int16 i = 0; i < collatorCount; i++) {
              Implementation impl(collatorArray[i*2], collatorArray[i*2 + 1][0]);
              seq[i] = impl;
            }
            return seq;
        }
        else {
            Sequence< Implementation > seq1(0);
            return seq1;
        }
}

Sequence< OUString > SAL_CALL
LocaleData::getCollationOptions( const Locale& rLocale ) throw(RuntimeException)
{
        sal_Int16 optionsCount = 0;
        sal_Unicode **optionsArray = NULL;

        MyFunc_Type func = (MyFunc_Type) getFunctionSymbol( rLocale, "getCollationOptions" );

        if ( func ) {
            optionsArray = func(optionsCount);
            Sequence< OUString > seq(optionsCount);
            for(sal_Int16 i = 0; i < optionsCount; i++) {
                    seq[i] = OUString( optionsArray[i] );
            }
            return seq;
        }
        else {
            Sequence< OUString > seq1(0);
            return seq1;
        }
}

Sequence< OUString > SAL_CALL
LocaleData::getSearchOptions( const Locale& rLocale ) throw(RuntimeException)
{
        sal_Int16 optionsCount = 0;
        sal_Unicode **optionsArray = NULL;

        MyFunc_Type func = (MyFunc_Type) getFunctionSymbol( rLocale, "getSearchOptions" );

        if ( func ) {
            optionsArray = func(optionsCount);
            Sequence< OUString > seq(optionsCount);
            for(sal_Int16 i = 0; i < optionsCount; i++) {
                    seq[i] = OUString( optionsArray[i] );
            }
            return seq;
        }
        else {
            Sequence< OUString > seq1(0);
            return seq1;
        }
}

sal_Unicode ** SAL_CALL
LocaleData::getIndexArray(const Locale& rLocale, sal_Int16& indexCount)
{
        MyFunc_Type func = (MyFunc_Type) getFunctionSymbol( rLocale, "getIndexAlgorithm" );

        if (func)
            return func(indexCount);
        return NULL;
}

Sequence< OUString > SAL_CALL
LocaleData::getIndexAlgorithm( const Locale& rLocale ) throw(RuntimeException)
{
        sal_Int16 indexCount = 0;
        sal_Unicode **indexArray = getIndexArray(rLocale, indexCount);

        if ( indexArray ) {
            Sequence< OUString > seq(indexCount);
            for(sal_Int16 i = 0; i < indexCount; i++) {
              seq[i] = indexArray[i*4];
            }
            return seq;
        }
        else {
            Sequence< OUString > seq1(0);
            return seq1;
        }
}

OUString SAL_CALL
LocaleData::getDefaultIndexAlgorithm( const Locale& rLocale ) throw(RuntimeException)
{
        sal_Int16 indexCount = 0;
        sal_Unicode **indexArray = getIndexArray(rLocale, indexCount);

        if ( indexArray ) {
            for(sal_Int16 i = 0; i < indexCount; i++) {
              if (indexArray[i*4 + 2][0])
                  return OUString(indexArray[i*4]);
            }
        }
        return OUString();
}

sal_Bool SAL_CALL
LocaleData::hasPhonetic( const Locale& rLocale ) throw(RuntimeException)
{
        sal_Int16 indexCount = 0;
        sal_Unicode **indexArray = getIndexArray(rLocale, indexCount);

        if ( indexArray ) {
            for(sal_Int16 i = 0; i < indexCount; i++) {
              if (indexArray[i*4 + 3][0])
                  return sal_True;
            }
        }
        return sal_False;
}

sal_Bool SAL_CALL
LocaleData::isPhonetic( const Locale& rLocale, const OUString& algorithm ) throw(RuntimeException)
{
        sal_Int16 indexCount = 0;
        sal_Unicode **indexArray = getIndexArray(rLocale, indexCount);

        if ( indexArray ) {
            for(sal_Int16 i = 0; i < indexCount; i++) {
              if (algorithm == OUString(indexArray[i*4]))
                  return indexArray[i*4 + 3][0] ? sal_False : sal_True;
            }
        }
        return sal_False;
}

OUString SAL_CALL
LocaleData::getIndexKeysByAlgorithm( const Locale& rLocale, const OUString& algorithm ) throw(RuntimeException)
{
        sal_Int16 indexCount = 0;
        sal_Unicode **indexArray = getIndexArray(rLocale, indexCount);

        if ( indexArray ) {
            for(sal_Int16 i = 0; i < indexCount; i++) {
              if (algorithm.equals(indexArray[i*3]))
                  return OUString(indexArray[i*4 + 1]);
            }
        }
        return OUString();
}

Sequence< UnicodeScript > SAL_CALL
LocaleData::getUnicodeScripts( const Locale& rLocale ) throw(RuntimeException)
{
        sal_Int16 scriptCount = 0;
        sal_Unicode **scriptArray = NULL;

        MyFunc_Type func = (MyFunc_Type) getFunctionSymbol( rLocale, "getUnicodeScripts" );

        if ( func ) {
            scriptArray = func(scriptCount);
            Sequence< UnicodeScript > seq(scriptCount);
            for(sal_Int16 i = 0; i < scriptCount; i++) {
                    seq[i] = UnicodeScript( OUString(scriptArray[i]).toInt32() );
            }
            return seq;
        }
        else {
            Sequence< UnicodeScript > seq1(0);
            return seq1;
        }
}

Sequence< OUString > SAL_CALL
LocaleData::getFollowPageWords( const Locale& rLocale ) throw(RuntimeException)
{
        sal_Int16 wordCount = 0;
        sal_Unicode **wordArray = NULL;

        MyFunc_Type func = (MyFunc_Type) getFunctionSymbol( rLocale, "getFollowPageWords" );

        if ( func ) {
            wordArray = func(wordCount);
            Sequence< OUString > seq(wordCount);
            for(sal_Int16 i = 0; i < wordCount; i++) {
                    seq[i] = OUString(wordArray[i]);
            }
            return seq;
        }
        else {
            Sequence< OUString > seq1(0);
            return seq1;
        }
}

Sequence< OUString > SAL_CALL
LocaleData::getTransliterations( const Locale& rLocale ) throw(RuntimeException)
{

        sal_Int16 transliterationsCount = 0;
        sal_Unicode **transliterationsArray = NULL;

        MyFunc_Type func = (MyFunc_Type) getFunctionSymbol( rLocale, "getTransliterations" );

        if ( func ) {
            transliterationsArray = func(transliterationsCount);

            Sequence< OUString > seq(transliterationsCount);
            for(int i = 0; i < transliterationsCount; i++) {
                    OUString  elem(transliterationsArray[i]);
                    seq[i] = elem;
            }
            return seq;
        }
        else {
            Sequence< OUString > seq1(0);
            return seq1;
        }


}


LanguageCountryInfo SAL_CALL
LocaleData::getLanguageCountryInfo( const Locale& rLocale ) throw(RuntimeException)
{
        sal_Int16 LCInfoCount = 0;
        sal_Unicode **LCInfoArray = NULL;

        MyFunc_Type func = (MyFunc_Type) getFunctionSymbol( rLocale, "getLCInfo" );

        if ( func ) {
            LCInfoArray = func(LCInfoCount);
            LanguageCountryInfo info(LCInfoArray[0],
                                    LCInfoArray[1],
                                    LCInfoArray[2],
                                    LCInfoArray[3],
                                    LCInfoArray[4]);
            return info;
        }
        else {
            LanguageCountryInfo info1;
            return info1;
        }

}


ForbiddenCharacters SAL_CALL
LocaleData::getForbiddenCharacters( const Locale& rLocale ) throw(RuntimeException)
{
        sal_Int16 LCForbiddenCharactersCount = 0;
        sal_Unicode **LCForbiddenCharactersArray = NULL;

        MyFunc_Type func = (MyFunc_Type) getFunctionSymbol( rLocale, "getForbiddenCharacters" );

        if ( func ) {
            LCForbiddenCharactersArray = func(LCForbiddenCharactersCount);
            ForbiddenCharacters chars(LCForbiddenCharactersArray[0], LCForbiddenCharactersArray[1]);
            return chars;
        }
        else {
            ForbiddenCharacters chars1;
            return chars1;
        }
}


Sequence< OUString > SAL_CALL
LocaleData::getReservedWord( const Locale& rLocale  ) throw(RuntimeException)
{
        sal_Int16 LCReservedWordsCount = 0;
        sal_Unicode **LCReservedWordsArray = NULL;

        MyFunc_Type func = (MyFunc_Type) getFunctionSymbol( rLocale, "getReservedWords" );

        if ( func ) {
            LCReservedWordsArray = func(LCReservedWordsCount);
            Sequence< OUString > seq(LCReservedWordsCount);
            for(int i = 0; i < (LCReservedWordsCount); i++) {
                OUString  elem(LCReservedWordsArray[i]);
                seq[i] = elem;
            }
            return seq;
        }
        else {
            Sequence< OUString > seq1(0);
            return seq1;
        }
}


inline
OUString C2U( const char* s )
{
        return OUString::createFromAscii( s );
}

Sequence< Sequence<beans::PropertyValue> > SAL_CALL
LocaleData::getContinuousNumberingLevels( const lang::Locale& rLocale ) throw(RuntimeException)
{
         int i;

         // load symbol
         MyFunc_Type2 func = (MyFunc_Type2) getFunctionSymbol( rLocale, "getContinuousNumberingLevels" );

         if ( func )
         {
              // invoke function
              sal_Int16 nStyles;
              sal_Int16 nAttributes;
              sal_Unicode*** p0 = func( nStyles, nAttributes );

              // allocate memory for nAttributes attributes for each of the nStyles styles.
              Sequence< Sequence<beans::PropertyValue> > pv( nStyles );
              for( i=0; i<pv.getLength(); i++ ) {
                   pv[i] = Sequence<beans::PropertyValue>( nAttributes );
              }

              sal_Unicode*** pStyle = p0;
              for( i=0;  i<nStyles;  i++ ) {
                   sal_Unicode** pAttribute = pStyle[i];
                   for( int j=0;  j<nAttributes;  j++ ) { // prefix, numberingtype, ...
                        sal_Unicode* pString = pAttribute[j];
                        beans::PropertyValue& rVal = pv[i][j];
                        OUString sVal;
                        if( pString ) {
                            if( 0 != j && 2 != j )
                                sVal = pString;
                            else if( *pString )
                                sVal = OUString( pString, 1 );
                        }

                        switch( j )
                        {
                        case 0:
                             rVal.Name = C2U("Prefix");
                             rVal.Value <<= sVal;
                             break;
                        case 1:
                             rVal.Name = C2U("NumberingType");
                             rVal.Value <<= (sal_Int16) sVal.toInt32();
                             break;
                        case 2:
                             rVal.Name = C2U("Suffix");
                             rVal.Value <<= sVal;
                             break;
                        case 3:
                             rVal.Name = C2U("Transliteration");
                             rVal.Value <<= sVal;
                             break;
                        case 4:
                             rVal.Name = C2U("NatNum");
                             rVal.Value <<= (sal_Int16) sVal.toInt32();
                             break;
                        default:
                             assert(0);
                        }
                   }
              }
              return pv;
         }

         Sequence< Sequence<beans::PropertyValue> > seq1(0);
         return seq1;
}

// ============================================================================
// \/ OutlineNumbering helper class \/
//
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

namespace com{ namespace sun{ namespace star{ namespace lang {
        struct  Locale;
}}}}
//-----------------------------------------------------------------------------
struct OutlineNumberingLevel_Impl
{
        OUString        sPrefix;
        sal_Int16               nNumType; //com::sun::star::style::NumberingType
        OUString        sSuffix;
        sal_Unicode     cBulletChar;
        const sal_Char* sBulletFontName;
        sal_Int16               nParentNumbering;
        sal_Int32               nLeftMargin;
        sal_Int32               nSymbolTextDistance;
        sal_Int32               nFirstLineOffset;
        OUString        sTransliteration;
        sal_Int32               nNatNum;
};
//-----------------------------------------------------------------------------
class OutlineNumbering : public cppu::WeakImplHelper1 < container::XIndexAccess >
{
        const OutlineNumberingLevel_Impl* m_pOutlineLevels;
        sal_Int16                         m_nCount;
public:
//      OutlineNumbering(const OutlineNumberingLevel_Impl* pOutlineLevels);
        OutlineNumbering(const OutlineNumberingLevel_Impl* pOutlineLevels, int nLevels);
        ~OutlineNumbering();

        //XIndexAccess
        virtual sal_Int32 SAL_CALL getCount(  ) throw(RuntimeException);
        virtual Any SAL_CALL getByIndex( sal_Int32 Index )
            throw(IndexOutOfBoundsException, WrappedTargetException, RuntimeException);

        //XElementAccess
        virtual Type SAL_CALL getElementType(  ) throw(RuntimeException);
        virtual sal_Bool SAL_CALL hasElements(  ) throw(RuntimeException);
};

//
//  /\ OutlineNumbering helper class /\
// ============================================================================

static
sal_Char* U2C( OUString str )
{
     sal_Char* s = new sal_Char[ str.getLength()+1 ];
     int i;
     for( i = 0; i < str.getLength(); i++)
         s[i] = str[i];
     s[i]='\0';
     return s;
}


Sequence< Reference<container::XIndexAccess> > SAL_CALL
LocaleData::getOutlineNumberingLevels( const lang::Locale& rLocale ) throw(RuntimeException)
{
    int i;

    // load symbol
    MyFunc_Type3 func = (MyFunc_Type3) getFunctionSymbol( rLocale, "getOutlineNumberingLevels" );

    if ( func )
    {
        // invoke function
        sal_Int16 nStyles;
        sal_Int16 nLevels;
        sal_Int16 nAttributes;
        sal_Unicode**** p0 = func( nStyles, nLevels, nAttributes );

        Sequence< Reference<container::XIndexAccess> > aRet( nStyles );

        OUString aEmptyStr;

        sal_Unicode**** pStyle = p0;
        for( i=0;  i<nStyles;  i++ )
        {
            int j;

            OutlineNumberingLevel_Impl* level = new OutlineNumberingLevel_Impl[ nLevels+1 ];
            sal_Unicode*** pLevel = pStyle[i];
            for( j = 0;  j < nLevels;  j++ )
            {
                sal_Unicode** pAttribute = pLevel[j];
                for( int k=0; k<nAttributes; k++ )
                {
                    OUString tmp( pAttribute[k] );
                    switch( k )
                    {
                        case 0: level[j].sPrefix             = tmp;             break;
                        case 1: level[j].nNumType            = tmp.toInt32();   break;
                        case 2: level[j].sSuffix             = tmp;             break;
                        //case 3: level[j].cBulletChar         = tmp.toChar();    break;
                        case 3: level[j].cBulletChar         = tmp.toInt32(16); break; // base 16
                        case 4: level[j].sBulletFontName     = U2C( tmp );      break;
                        case 5: level[j].nParentNumbering    = tmp.toInt32();   break;
                        case 6: level[j].nLeftMargin         = tmp.toInt32();   break;
                        case 7: level[j].nSymbolTextDistance = tmp.toInt32();   break;
                        case 8: level[j].nFirstLineOffset    = tmp.toInt32();   break;
                        case 9: // Adjust
                        // these values seem to be hard-coded elsewhere:
                        // level[j].Value <<= (sal_Int16) text::HoriOrientation::LEFT;
                        // level[j].Value <<= (sal_Int16) style::HorizontalAlignment::LEFT;
                            break;
                        case 10: level[j].sTransliteration = tmp; break;
                        case 11: level[j].nNatNum    = tmp.toInt32();   break;
                        default:
                            assert(0);
                    }
                }
            }
            level[j].sPrefix             = aEmptyStr;
            level[j].nNumType            = 0;
            level[j].sSuffix             = aEmptyStr;
            level[j].cBulletChar         = 0;
            level[j].sBulletFontName     = 0;
            level[j].nParentNumbering    = 0;
            level[j].nLeftMargin         = 0;
            level[j].nSymbolTextDistance = 0;
            level[j].nFirstLineOffset    = 0;
            level[j].sTransliteration    = aEmptyStr;
            level[j].nNatNum             = 0;
            aRet[i] = new OutlineNumbering( level, nLevels );
        }
        return aRet;
    }
    else {
        Sequence< Reference<container::XIndexAccess> > seq1(0);
        return seq1;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////helper functions///////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

void* SAL_CALL LocaleData::getFunctionSymbol( const Locale& rLocale, const sal_Char* pFunction )
        throw(RuntimeException)
{
        OUStringBuffer aBuf(1);
        if (cachedItem && cachedItem->equals(rLocale)) {
            aBuf.ensureCapacity(strlen(pFunction) + 1 + strlen(cachedItem->localeName));
            return cachedItem->module->getSymbol(aBuf.appendAscii(pFunction).append(under).
                                        appendAscii(cachedItem->localeName).makeStringAndClear());
        }

        void* pSymbol = 0;
        static OUString tw(OUString::createFromAscii("TW"));
        static OUString en_US(OUString::createFromAscii("en_US"));

        sal_Int32 l = rLocale.Language.getLength();
        sal_Int32 c = rLocale.Country.getLength();
        sal_Int32 v = rLocale.Variant.getLength();
        aBuf.ensureCapacity(l+c+v+3);

        if ((l > 0 && c > 0 && v > 0 &&
                // load function with name <func>_<lang>_<country>_<varian>
                (pSymbol = getFunctionSymbolByName(aBuf.append(rLocale.Language).append(under).append(
                        rLocale.Country).append(under).append(rLocale.Variant).makeStringAndClear(), pFunction))) ||
            (l > 0 && c > 0 &&
                // load function with name <ase>_<lang>_<country>
                (pSymbol = getFunctionSymbolByName(aBuf.append(rLocale.Language).append(under).append(
                        rLocale.Country).makeStringAndClear(), pFunction))) ||
            (l > 0 && c > 0 && rLocale.Language.equalsAscii("zh") &&
                                (rLocale.Country.equalsAscii("HK") ||
                                rLocale.Country.equalsAscii("MO")) &&
                // if the country code is HK or MO, one more step to try TW.
                (pSymbol = getFunctionSymbolByName(aBuf.append(rLocale.Language).append(under).append(tw).makeStringAndClear(),
                        pFunction))) ||
            (l > 0 &&
                // load function with name <func>_<lang>
                (pSymbol = getFunctionSymbolByName(rLocale.Language, pFunction))) ||
                // load default function with name <func>_en_US
                (pSymbol = getFunctionSymbolByName(en_US, pFunction))) {
            cachedItem->aLocale = rLocale;
            return pSymbol;
        }
        throw RuntimeException();
        return pSymbol;
}


void* SAL_CALL LocaleData::getFunctionSymbolByName( const OUString& localeName, const sal_Char* pFunction )
{
        for( sal_Int16 i = 0; i < nbOfLocales; i++) {
            if (localeName.equalsAscii(aDllsTable[i].pLocale) ||
                        (aDllsTable[i].lLocale && localeName.equalsAscii(aDllsTable[i].lLocale))) {

                OUStringBuffer aBuf(strlen(aDllsTable[i].pLocale) + 1 + strlen(pFunction));
                for (sal_Int32 l = 0; l < lookupTable.size(); l++) {
                    cachedItem = lookupTable[l];
                    if (cachedItem->dllName == aDllsTable[i].pDLL) {
                        cachedItem->localeName = aDllsTable[i].pLocale;
                        return cachedItem->module->getSymbol(aBuf.appendAscii(pFunction).append(under).
                                            appendAscii(cachedItem->localeName).makeStringAndClear());
                    }
                }
                //dll not loaded, load it and add it to the list
#ifdef SAL_DLLPREFIX
        aBuf.ensureCapacity(strlen(aDllsTable[i].pDLL) + 6);    // mostly "lib*.so"
                aBuf.appendAscii( SAL_DLLPREFIX ).appendAscii(aDllsTable[i].pDLL).appendAscii( SAL_DLLEXTENSION );
#else
        aBuf.ensureCapacity(strlen(aDllsTable[i].pDLL) + 4);    // mostly "*.dll"
        aBuf.appendAscii(aDllsTable[i].pDLL).appendAscii( SAL_DLLEXTENSION );
#endif
                osl::Module *module = new osl::Module();
                if ( module->load(aBuf.makeStringAndClear()) ) {
                    lookupTable.push_back(cachedItem = new lookupTableItem(aDllsTable[i].pDLL, module));
                    cachedItem->localeName = aDllsTable[i].pLocale;
                    return module->getSymbol(aBuf.appendAscii(pFunction).append(under).
                                            appendAscii(cachedItem->localeName).makeStringAndClear());
                }
                else
                    delete module;
            }
        }
        return NULL;
}

Sequence< Locale > SAL_CALL
LocaleData::getAllInstalledLocaleNames() throw(RuntimeException)
{
        Sequence< lang::Locale > seq( nbOfLocales );
        OUString empStr;
        sal_Int16 nInstalled = 0;

        for( sal_Int16 i=0; i<nbOfLocales; i++ ) {
            OUString name = OUString::createFromAscii( aDllsTable[i].pLocale );

            // Check if the locale is really available and not just in the table,
            // don't allow fall backs.
            if (getFunctionSymbolByName( name, "getLocaleItem" )) {
                sal_Int32 index = 0;
                lang::Locale tmpLocale(name.getToken(0, under, index), empStr, empStr);
                if (index >= 0) {
                    tmpLocale.Country = name.getToken(0, under, index);
                    if (index >= 0)
                        tmpLocale.Variant = name.getToken(0, under, index);
                }
                seq[nInstalled++] = tmpLocale;
            }
        }
        if ( nInstalled < nbOfLocales )
            seq.realloc( nInstalled );          // reflect reality

        return seq;
}

// ============================================================================

using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::text;

// // bad: can't have empty prefix ...
// OutlineNumbering::OutlineNumbering(const OutlineNumberingLevel_Impl* pOutlnLevels) :
//         m_pOutlineLevels(pOutlnLevels),
//         m_nCount(0)
// {
//         const OutlineNumberingLevel_Impl* pTemp = m_pOutlineLevels;
//         while((pTemp++)->sPrefix)
//                 m_nCount++;
// }

OutlineNumbering::OutlineNumbering(const OutlineNumberingLevel_Impl* pOutlnLevels, int nLevels) :
        m_pOutlineLevels(pOutlnLevels),
        m_nCount(nLevels)
{
}

OutlineNumbering::~OutlineNumbering()
{
}

sal_Int32 OutlineNumbering::getCount(  ) throw(RuntimeException)
{
        return m_nCount;
}

Any OutlineNumbering::getByIndex( sal_Int32 nIndex )
        throw(IndexOutOfBoundsException, WrappedTargetException, RuntimeException)
{
        if(nIndex < 0 || nIndex >= m_nCount)
                throw IndexOutOfBoundsException();
        const OutlineNumberingLevel_Impl* pTemp = m_pOutlineLevels;
        pTemp += nIndex;
        Any aRet;

        Sequence<PropertyValue> aOutlineNumbering(12);
        PropertyValue* pValues = aOutlineNumbering.getArray();
        pValues[0].Name = C2U( "Prefix");
        pValues[0].Value <<= pTemp->sPrefix;
        pValues[1].Name = C2U("NumberingType");
        pValues[1].Value <<= pTemp->nNumType;
        pValues[2].Name = C2U("Suffix");
        pValues[2].Value <<= pTemp->sSuffix;
        pValues[3].Name = C2U("BulletChar");
        pValues[3].Value <<= OUString(&pTemp->cBulletChar, 1);
        pValues[4].Name = C2U("BulletFontName");
        pValues[4].Value <<= C2U(pTemp->sBulletFontName);
        pValues[5].Name = C2U("ParentNumbering");
        pValues[5].Value <<= pTemp->nParentNumbering;
        pValues[6].Name = C2U("LeftMargin");
        pValues[6].Value <<= pTemp->nLeftMargin;
        pValues[7].Name = C2U("SymbolTextDistance");
        pValues[7].Value <<= pTemp->nSymbolTextDistance;
        pValues[8].Name = C2U("FirstLineOffset");
        pValues[8].Value <<= pTemp->nFirstLineOffset;
        pValues[9].Name = C2U("Adjust");
        pValues[9].Value <<= (sal_Int16)HoriOrientation::LEFT;
        pValues[10].Name = C2U("Transliteration");
        pValues[10].Value <<= pTemp->sTransliteration;
        pValues[11].Name = C2U("NatNum");
        pValues[11].Value <<= pTemp->nNatNum;
        aRet <<= aOutlineNumbering;
        return aRet;
}

Type OutlineNumbering::getElementType(  ) throw(RuntimeException)
{
        return ::getCppuType((Sequence<PropertyValue>*)0);
}

sal_Bool OutlineNumbering::hasElements(  ) throw(RuntimeException)
{
        return m_nCount > 0;
}

OUString SAL_CALL
LocaleData::getImplementationName() throw( RuntimeException )
{
        return OUString::createFromAscii(clocaledata);
}

sal_Bool SAL_CALL
LocaleData::supportsService(const OUString& rServiceName)
                throw( RuntimeException )
{
        return !rServiceName.compareToAscii(clocaledata);
}

Sequence< OUString > SAL_CALL
LocaleData::getSupportedServiceNames() throw( RuntimeException )
{
        Sequence< OUString > aRet(1);
        aRet[0] = OUString::createFromAscii(clocaledata);
        return aRet;
}
