/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include <localedata.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <rtl/ustrbuf.hxx>
#include <string.h>
#include <stdio.h>
#include "rtl/instance.hxx"
#include <sal/macros.h>

using namespace com::sun::star::i18n;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star;


static const sal_Char clocaledata[] = "com.sun.star.i18n.LocaleData";

typedef sal_Unicode**   (SAL_CALL * MyFunc_Type)( sal_Int16&);
typedef sal_Unicode***  (SAL_CALL * MyFunc_Type2)( sal_Int16&, sal_Int16& );
typedef sal_Unicode**** (SAL_CALL * MyFunc_Type3)( sal_Int16&, sal_Int16&, sal_Int16& );
typedef sal_Unicode const * const * (SAL_CALL * MyFunc_FormatCode)( sal_Int16&, sal_Unicode const *&, sal_Unicode const *& );

#ifndef DISABLE_DYNLOADING

static const char *lcl_DATA_EN = "localedata_en";
static const char *lcl_DATA_ES = "localedata_es";
static const char *lcl_DATA_EURO = "localedata_euro";
static const char *lcl_DATA_OTHERS = "localedata_others";

#endif

#ifndef DISABLE_DYNLOADING

static const struct {
    const char* pLocale;
    const char* pLib;
} aLibTable[] = {
    { "en_US",  lcl_DATA_EN },
    { "en_AU",  lcl_DATA_EN },
    { "en_BZ",  lcl_DATA_EN },
    { "en_CA",  lcl_DATA_EN },
    { "en_GB",  lcl_DATA_EN },
    { "en_IE",  lcl_DATA_EN },
    { "en_JM",  lcl_DATA_EN },
    { "en_NZ",  lcl_DATA_EN },
    { "en_PH",  lcl_DATA_EN },
    { "en_TT",  lcl_DATA_EN },
    { "en_ZA",  lcl_DATA_EN },
    { "en_ZW",  lcl_DATA_EN },
    { "en_NA",  lcl_DATA_EN },
    { "en_GH",  lcl_DATA_EN },
    { "en_MW",  lcl_DATA_EN },

    { "es_ES",  lcl_DATA_ES },
    { "es_AR",  lcl_DATA_ES },
    { "es_BO",  lcl_DATA_ES },
    { "es_CL",  lcl_DATA_ES },
    { "es_CO",  lcl_DATA_ES },
    { "es_CR",  lcl_DATA_ES },
    { "es_DO",  lcl_DATA_ES },
    { "es_EC",  lcl_DATA_ES },
    { "es_GT",  lcl_DATA_ES },
    { "es_HN",  lcl_DATA_ES },
    { "es_MX",  lcl_DATA_ES },
    { "es_NI",  lcl_DATA_ES },
    { "es_PA",  lcl_DATA_ES },
    { "es_PE",  lcl_DATA_ES },
    { "es_PR",  lcl_DATA_ES },
    { "es_PY",  lcl_DATA_ES },
    { "es_SV",  lcl_DATA_ES },
    { "es_UY",  lcl_DATA_ES },
    { "es_VE",  lcl_DATA_ES },
    { "gl_ES",  lcl_DATA_ES },

    { "de_DE",  lcl_DATA_EURO },
    { "de_AT",  lcl_DATA_EURO },
    { "de_CH",  lcl_DATA_EURO },
    { "de_LI",  lcl_DATA_EURO },
    { "de_LU",  lcl_DATA_EURO },
    { "fr_FR",  lcl_DATA_EURO },
    { "fr_BE",  lcl_DATA_EURO },
    { "fr_CA",  lcl_DATA_EURO },
    { "fr_CH",  lcl_DATA_EURO },
    { "fr_LU",  lcl_DATA_EURO },
    { "fr_MC",  lcl_DATA_EURO },
    { "it_IT",  lcl_DATA_EURO },
    { "it_CH",  lcl_DATA_EURO },
    { "sl_SI",  lcl_DATA_EURO },
    { "sv_SE",  lcl_DATA_EURO },
    { "sv_FI",  lcl_DATA_EURO },
    { "ca_ES",  lcl_DATA_EURO },
    { "cs_CZ",  lcl_DATA_EURO },
    { "sk_SK",  lcl_DATA_EURO },
    { "da_DK",  lcl_DATA_EURO },
    { "el_GR",  lcl_DATA_EURO },
    { "fi_FI",  lcl_DATA_EURO },
    { "is_IS",  lcl_DATA_EURO },
    { "nl_BE",  lcl_DATA_EURO },
    { "nl_NL",  lcl_DATA_EURO },
    { "no_NO",  lcl_DATA_EURO },
    { "nn_NO",  lcl_DATA_EURO },
    { "nb_NO",  lcl_DATA_EURO },
    { "pl_PL",  lcl_DATA_EURO },
    { "pt_BR",  lcl_DATA_EURO },
    { "pt_PT",  lcl_DATA_EURO },
    { "ru_RU",  lcl_DATA_EURO },
    { "tr_TR",  lcl_DATA_EURO },
    { "et_EE",  lcl_DATA_EURO },
    { "lb_LU",  lcl_DATA_EURO },
    { "lt_LT",  lcl_DATA_EURO },
    { "lv_LV",  lcl_DATA_EURO },
    { "uk_UA",  lcl_DATA_EURO },
    { "ro_RO",  lcl_DATA_EURO },
    { "cy_GB",  lcl_DATA_EURO },
    { "bg_BG",  lcl_DATA_EURO },
    { "sh_ME",  lcl_DATA_EURO },
    { "sh_RS",  lcl_DATA_EURO },
    { "sh_YU",  lcl_DATA_EURO },
    { "sr_ME",  lcl_DATA_EURO },
    { "sr_RS",  lcl_DATA_EURO },
    { "sr_YU",  lcl_DATA_EURO },
    { "hr_HR",  lcl_DATA_EURO },
    { "bs_BA",  lcl_DATA_EURO },
    { "eu",     lcl_DATA_EURO },
    { "fo_FO",  lcl_DATA_EURO },
    { "ga_IE",  lcl_DATA_EURO },
    { "gd_GB",  lcl_DATA_EURO },
    { "ka_GE",  lcl_DATA_EURO },
    { "be_BY",  lcl_DATA_EURO },
    { "kl_GL",  lcl_DATA_EURO },
    { "mk_MK",  lcl_DATA_EURO },
    { "br_FR",  lcl_DATA_EURO },
    { "la_VA",  lcl_DATA_EURO },
    { "cv_RU",  lcl_DATA_EURO },
    { "wa_BE",  lcl_DATA_EURO },
    { "fur_IT", lcl_DATA_EURO },
    { "gsc_FR", lcl_DATA_EURO },
    { "fy_NL",  lcl_DATA_EURO },
    { "oc_FR",  lcl_DATA_EURO },
    { "mt_MT",  lcl_DATA_EURO },
    { "sc_IT",  lcl_DATA_EURO },
    { "ast_ES", lcl_DATA_EURO },
    { "ltg_LV", lcl_DATA_EURO },
    { "hsb_DE", lcl_DATA_EURO },
    { "dsb_DE", lcl_DATA_EURO },
    { "rue_SK", lcl_DATA_EURO },
    { "an_ES",  lcl_DATA_EURO },
    { "myv_RU", lcl_DATA_EURO },

    { "ja_JP",  lcl_DATA_OTHERS },
    { "ko_KR",  lcl_DATA_OTHERS },
    { "zh_CN",  lcl_DATA_OTHERS },
    { "zh_HK",  lcl_DATA_OTHERS },
    { "zh_SG",  lcl_DATA_OTHERS },
    { "zh_TW",  lcl_DATA_OTHERS },
    { "zh_MO",  lcl_DATA_OTHERS },

    { "ar_EG",  lcl_DATA_OTHERS },
    { "ar_DZ",  lcl_DATA_OTHERS },
    { "ar_LB",  lcl_DATA_OTHERS },
    { "ar_SA",  lcl_DATA_OTHERS },
    { "ar_TN",  lcl_DATA_OTHERS },
    { "he_IL",  lcl_DATA_OTHERS },
    { "hi_IN",  lcl_DATA_OTHERS },
    { "kn_IN",  lcl_DATA_OTHERS },
    { "ta_IN",  lcl_DATA_OTHERS },
    { "te_IN",  lcl_DATA_OTHERS },
    { "gu_IN",  lcl_DATA_OTHERS },
    { "mr_IN",  lcl_DATA_OTHERS },
    { "pa_IN",  lcl_DATA_OTHERS },
    { "bn_IN",  lcl_DATA_OTHERS },
    { "or_IN",  lcl_DATA_OTHERS },
    { "en_IN",  lcl_DATA_OTHERS },
    { "ml_IN",  lcl_DATA_OTHERS },
    { "bn_BD",  lcl_DATA_OTHERS },
    { "th_TH",  lcl_DATA_OTHERS },

    { "af_ZA",  lcl_DATA_OTHERS },
    { "hu_HU",  lcl_DATA_OTHERS },
    { "id_ID",  lcl_DATA_OTHERS },
    { "ms_MY",  lcl_DATA_OTHERS },
    { "ia",     lcl_DATA_OTHERS },
    { "mn_MN",  lcl_DATA_OTHERS },
    { "az_AZ",  lcl_DATA_OTHERS },
    { "sw_TZ",  lcl_DATA_OTHERS },
    { "km_KH",  lcl_DATA_OTHERS },
    { "lo_LA",  lcl_DATA_OTHERS },
    { "rw_RW",  lcl_DATA_OTHERS },
    { "eo",     lcl_DATA_OTHERS },
    { "dz_BT",  lcl_DATA_OTHERS },
    { "ne_NP",  lcl_DATA_OTHERS },
    { "zu_ZA",  lcl_DATA_OTHERS },
    { "nso_ZA", lcl_DATA_OTHERS },
    { "vi_VN",  lcl_DATA_OTHERS },
    { "tn_ZA",  lcl_DATA_OTHERS },
    { "xh_ZA",  lcl_DATA_OTHERS },
    { "st_ZA",  lcl_DATA_OTHERS },
    { "ss_ZA",  lcl_DATA_OTHERS },
    { "ve_ZA",  lcl_DATA_OTHERS },
    { "nr_ZA",  lcl_DATA_OTHERS },
    { "ts_ZA",  lcl_DATA_OTHERS },
    { "ku_TR",  lcl_DATA_OTHERS },
    { "ak_GH",  lcl_DATA_OTHERS },
    { "af_NA",  lcl_DATA_OTHERS },
    { "am_ET",  lcl_DATA_OTHERS },
    { "ti_ER",  lcl_DATA_OTHERS },
    { "tg_TJ",  lcl_DATA_OTHERS },
    { "ky_KG",  lcl_DATA_OTHERS },
    { "kk_KZ",  lcl_DATA_OTHERS },
    { "fa_IR",  lcl_DATA_OTHERS },
    { "ha_GH",  lcl_DATA_OTHERS },
    { "ee_GH",  lcl_DATA_OTHERS },
    { "sg_CF",  lcl_DATA_OTHERS },
    { "lg_UG",  lcl_DATA_OTHERS },
    { "uz_UZ",  lcl_DATA_OTHERS },
    { "ln_CD",  lcl_DATA_OTHERS },
    { "hy_AM",  lcl_DATA_OTHERS },
    { "hil_PH", lcl_DATA_OTHERS },
    { "so_SO",  lcl_DATA_OTHERS },
    { "gug_PY", lcl_DATA_OTHERS },
    { "tk_TM",  lcl_DATA_OTHERS },
    { "my_MM",  lcl_DATA_OTHERS },
    { "shs_CA", lcl_DATA_OTHERS },
    { "tpi_PG", lcl_DATA_OTHERS },
    { "ar_OM",  lcl_DATA_OTHERS },
    { "ug_CN",  lcl_DATA_OTHERS },
    { "om_ET",  lcl_DATA_OTHERS },
    { "plt_MG", lcl_DATA_OTHERS },
    { "mai_IN", lcl_DATA_OTHERS },
    { "yi_US",  lcl_DATA_OTHERS },
    { "haw_US", lcl_DATA_OTHERS },
    { "lif_NP", lcl_DATA_OTHERS },
    { "ur_PK",  lcl_DATA_OTHERS },
    { "ht_HT",  lcl_DATA_OTHERS },
    { "jbo",    lcl_DATA_OTHERS },
    { "kab_DZ", lcl_DATA_OTHERS },
    { "pt_AO",  lcl_DATA_OTHERS },
    { "pjt_AU", lcl_DATA_OTHERS },
    { "pap_CW", lcl_DATA_OTHERS },
    { "ebo_CG", lcl_DATA_OTHERS },
    { "tyx_CG", lcl_DATA_OTHERS },
    { "axk_CG", lcl_DATA_OTHERS },
    { "beq_CG", lcl_DATA_OTHERS },
    { "bkw_CG", lcl_DATA_OTHERS },
    { "bvx_CG", lcl_DATA_OTHERS },
    { "dde_CG", lcl_DATA_OTHERS },
    { "iyx_CG", lcl_DATA_OTHERS },
    { "kkw_CG", lcl_DATA_OTHERS },
    { "kng_CG", lcl_DATA_OTHERS },
    { "ldi_CG", lcl_DATA_OTHERS },
    { "mdw_CG", lcl_DATA_OTHERS },
    { "mkw_CG", lcl_DATA_OTHERS },
    { "njx_CG", lcl_DATA_OTHERS },
    { "ngz_CG", lcl_DATA_OTHERS },
    { "njy_CG", lcl_DATA_OTHERS },
    { "puu_CG", lcl_DATA_OTHERS },
    { "sdj_CG", lcl_DATA_OTHERS },
    { "tek_CG", lcl_DATA_OTHERS },
    { "tsa_CG", lcl_DATA_OTHERS },
    { "vif_CG", lcl_DATA_OTHERS },
    { "xku_CG", lcl_DATA_OTHERS },
    { "yom_CG", lcl_DATA_OTHERS },
    { "sid_ET", lcl_DATA_OTHERS },
    { "bo_CN",  lcl_DATA_OTHERS },
    { "bo_IN",  lcl_DATA_OTHERS }
};

#else

#include "localedata_static.hxx"

#endif

static const sal_Unicode cUnder = sal_Unicode('_');
static const sal_Unicode cHyphen = sal_Unicode('-');

static const sal_Int16 nbOfLocales = SAL_N_ELEMENTS(aLibTable);

struct LocaleDataLookupTableItem
{
    LocaleDataLookupTableItem(const sal_Char *name, osl::Module* m, const sal_Char* lname) : dllName(name), module(m), localeName(lname)
    {
    }
    const sal_Char* dllName;
    osl::Module *module;
    const sal_Char* localeName;

    com::sun::star::lang::Locale aLocale;
    sal_Bool equals(const com::sun::star::lang::Locale& rLocale)
    {
        return (rLocale == aLocale);
    }
};


// static
Sequence< CalendarItem > LocaleData::downcastCalendarItems( const Sequence< CalendarItem2 > & rCi )
{
    sal_Int32 nSize = rCi.getLength();
    Sequence< CalendarItem > aCi( nSize);
    CalendarItem* p1 = aCi.getArray();
    const CalendarItem2* p2 = rCi.getConstArray();
    for (sal_Int32 i=0; i < nSize; ++i, ++p1, ++p2)
        *p1 = *p2;
    return aCi;
}


// static
Calendar LocaleData::downcastCalendar( const Calendar2 & rC )
{
    Calendar aCal(
            downcastCalendarItems( rC.Days),
            downcastCalendarItems( rC.Months),
            downcastCalendarItems( rC.Eras),
            rC.StartOfWeek,
            rC.MinimumNumberOfDaysForFirstWeek,
            rC.Default,
            rC.Name
            );
    return aCal;
}


LocaleData::LocaleData()
{
}
LocaleData::~LocaleData()
{
}


LocaleDataItem SAL_CALL
LocaleData::getLocaleItem( const Locale& rLocale ) throw(RuntimeException)
{
    sal_Unicode **dataItem = NULL;

    MyFunc_Type func = (MyFunc_Type) getFunctionSymbol( rLocale, "getLocaleItem" );

    if ( func ) {
        sal_Int16 dataItemCount = 0;
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

#ifndef DISABLE_DYNLOADING

extern "C" { static void SAL_CALL thisModule() {} }

#endif

namespace
{

// implement the lookup table as a safe static object
class lcl_LookupTableHelper
{
public:
    lcl_LookupTableHelper();
    ~lcl_LookupTableHelper();

    oslGenericFunction SAL_CALL getFunctionSymbolByName(
            const OUString& localeName, const sal_Char* pFunction,
            LocaleDataLookupTableItem** pOutCachedItem );

private:
    ::osl::Mutex maMutex;
    ::std::vector< LocaleDataLookupTableItem* >  maLookupTable;
};

// from instance.hxx: Helper base class for a late-initialized
// (default-constructed) static variable, implementing the double-checked
// locking pattern correctly.
// usage:  lcl_LookupTableHelper & rLookupTable = lcl_LookupTableStatic::get();
// retrieves the singleton lookup table instance
struct lcl_LookupTableStatic : public ::rtl::Static< lcl_LookupTableHelper, lcl_LookupTableStatic >
{};

lcl_LookupTableHelper::lcl_LookupTableHelper()
{
}

lcl_LookupTableHelper::~lcl_LookupTableHelper()
{
    LocaleDataLookupTableItem* pItem = 0;

    std::vector<LocaleDataLookupTableItem*>::const_iterator aEnd(maLookupTable.end());
    std::vector<LocaleDataLookupTableItem*>::iterator aIter(maLookupTable.begin());

    for ( ; aIter != aEnd; ++aIter ) {
        pItem = *aIter;
        delete pItem->module;
        delete pItem;
    }
    maLookupTable.clear();
}

oslGenericFunction SAL_CALL lcl_LookupTableHelper::getFunctionSymbolByName(
        const OUString& localeName, const sal_Char* pFunction,
        LocaleDataLookupTableItem** pOutCachedItem )
{
    OUString aFallback;
    bool bFallback = (localeName.indexOf( cUnder) < 0);
    if (bFallback)
    {
        Locale aLocale;
        aLocale.Language = localeName;
        Locale aFbLocale = MsLangId::getFallbackLocale( aLocale);
        if (aFbLocale == aLocale)
            bFallback = false;  // may be a "language-only-locale" like Interlingua (ia)
        else
            aFallback = LocaleData::getFirstLocaleServiceName( aFbLocale);
    }

    for ( sal_Int16 i = 0; i < nbOfLocales; i++)
    {
        if (localeName.equalsAscii(aLibTable[i].pLocale) ||
                (bFallback && aFallback.equalsAscii(aLibTable[i].pLocale)))
        {
#ifndef DISABLE_DYNLOADING
            LocaleDataLookupTableItem* pCurrent = 0;
            OUStringBuffer aBuf(strlen(aLibTable[i].pLocale) + 1 + strlen(pFunction));
            {
                ::osl::MutexGuard aGuard( maMutex );
                for (size_t l = 0; l < maLookupTable.size(); l++)
                {
                    pCurrent = maLookupTable[l];
                    if (pCurrent->dllName == aLibTable[i].pLib)
                    {
                        OSL_ASSERT( pOutCachedItem );
                        if( pOutCachedItem )
                        {
                            (*pOutCachedItem) = new LocaleDataLookupTableItem( *pCurrent );
                            (*pOutCachedItem)->localeName = aLibTable[i].pLocale;
                            return (*pOutCachedItem)->module->getFunctionSymbol(
                                    aBuf.appendAscii( pFunction).append( cUnder).
                                    appendAscii( (*pOutCachedItem)->localeName).makeStringAndClear());
                        }
                        else
                            return NULL;
                    }
                }
            }
            // Library not loaded, load it and add it to the list.
#ifdef SAL_DLLPREFIX
            aBuf.ensureCapacity(strlen(aLibTable[i].pLib) + 6);    // mostly "lib*.so"
            aBuf.appendAscii( SAL_DLLPREFIX ).appendAscii(aLibTable[i].pLib).appendAscii( SAL_DLLEXTENSION );
#else
            aBuf.ensureCapacity(strlen(aLibTable[i].pLib) + 4);    // mostly "*.dll"
            aBuf.appendAscii(aLibTable[i].pLib).appendAscii( SAL_DLLEXTENSION );
#endif
            osl::Module *module = new osl::Module();
            if ( module->loadRelative(&thisModule, aBuf.makeStringAndClear()) )
            {
                ::osl::MutexGuard aGuard( maMutex );
                LocaleDataLookupTableItem* pNewItem = new LocaleDataLookupTableItem(aLibTable[i].pLib, module, aLibTable[i].pLocale);
                maLookupTable.push_back(pNewItem);
                OSL_ASSERT( pOutCachedItem );
                if( pOutCachedItem )
                {
                    (*pOutCachedItem) = new LocaleDataLookupTableItem( *pNewItem );
                    return module->getFunctionSymbol(
                            aBuf.appendAscii(pFunction).append(cUnder).
                            appendAscii((*pOutCachedItem)->localeName).makeStringAndClear());
                }
                else
                    return NULL;
            }
            else
                delete module;
#else
            (void) pOutCachedItem;

            if( strcmp(pFunction, "getAllCalendars") == 0 )
                return aLibTable[i].getAllCalendars;
            else if( strcmp(pFunction, "getAllCurrencies") == 0 )
                return aLibTable[i].getAllCurrencies;
            else if( strcmp(pFunction, "getAllFormats0") == 0 )
                return aLibTable[i].getAllFormats0;
            else if( strcmp(pFunction, "getBreakIteratorRules") == 0 )
                return aLibTable[i].getBreakIteratorRules;
            else if( strcmp(pFunction, "getCollationOptions") == 0 )
                return aLibTable[i].getCollationOptions;
            else if( strcmp(pFunction, "getCollatorImplementation") == 0 )
                return aLibTable[i].getCollatorImplementation;
            else if( strcmp(pFunction, "getContinuousNumberingLevels") == 0 )
                return aLibTable[i].getContinuousNumberingLevels;
            else if( strcmp(pFunction, "getDateAcceptancePatterns") == 0 )
                return aLibTable[i].getDateAcceptancePatterns;
            else if( strcmp(pFunction, "getFollowPageWords") == 0 )
                return aLibTable[i].getFollowPageWords;
            else if( strcmp(pFunction, "getForbiddenCharacters") == 0 )
                return aLibTable[i].getForbiddenCharacters;
            else if( strcmp(pFunction, "getIndexAlgorithm") == 0 )
                return aLibTable[i].getIndexAlgorithm;
            else if( strcmp(pFunction, "getLCInfo") == 0 )
                return aLibTable[i].getLCInfo;
            else if( strcmp(pFunction, "getLocaleItem") == 0 )
                return aLibTable[i].getLocaleItem;
            else if( strcmp(pFunction, "getOutlineNumberingLevels") == 0 )
                return aLibTable[i].getOutlineNumberingLevels;
            else if( strcmp(pFunction, "getReservedWords") == 0 )
                return aLibTable[i].getReservedWords;
            else if( strcmp(pFunction, "getSearchOptions") == 0 )
                return aLibTable[i].getSearchOptions;
            else if( strcmp(pFunction, "getTransliterations") == 0 )
                return aLibTable[i].getTransliterations;
            else if( strcmp(pFunction, "getUnicodeScripts") == 0 )
                return aLibTable[i].getUnicodeScripts;
            else if( strcmp(pFunction, "getAllFormats1") == 0 )
                return aLibTable[i].getAllFormats1;
#endif
        }
    }
    return NULL;
}

} // anonymous namespace


// REF values equal offsets of counts within getAllCalendars() data structure!
#define REF_DAYS         0
#define REF_MONTHS       1
#define REF_GMONTHS      2
#define REF_PMONTHS      3
#define REF_ERAS         4
#define REF_OFFSET_COUNT 5

Sequence< CalendarItem2 > &LocaleData::getCalendarItemByName(const OUString& name,
        const Locale& rLocale, const Sequence< Calendar2 >& calendarsSeq, sal_Int16 item)
        throw(RuntimeException)
{
    if (!ref_name.equals(name)) {
        sal_Int32 index = 0;
        OUString language = name.getToken(0, cUnder, index);
        OUString country = name.getToken(0, cUnder, index);
        Locale loc(language, country, OUString());
        Sequence < Calendar2 > cals;
        if (loc == rLocale) {
            cals = calendarsSeq;
        } else {
            cals = getAllCalendars2(loc);
        }
        const OUString& id = name.getToken(0, cUnder, index);
        for (index = 0; index < cals.getLength(); index++) {
            if (id.equals(cals[index].Name)) {
                ref_cal = cals[index];
                break;
            }
        }
        // Referred locale not found, return name for en_US locale.
        if (index == cals.getLength()) {
            cals = getAllCalendars2(
                    Locale(OUString("en"), OUString("US"), OUString()));
            if (cals.getLength() > 0)
                ref_cal = cals[0];
            else
                throw RuntimeException();
        }
        ref_name = name;
    }
    switch (item)
    {
        case REF_DAYS:
            return ref_cal.Days;
        case REF_MONTHS:
            return ref_cal.Months;
        case REF_GMONTHS:
            return ref_cal.GenitiveMonths;
        case REF_PMONTHS:
            return ref_cal.PartitiveMonths;
        default:
            OSL_FAIL( "LocaleData::getCalendarItemByName: unhandled REF_* case");
            // fallthru
        case REF_ERAS:
            return ref_cal.Eras;
    }
}


Sequence< CalendarItem2 > LocaleData::getCalendarItems(
        sal_Unicode const * const * const allCalendars, sal_Int16 & rnOffset,
        const sal_Int16 nWhichItem, const sal_Int16 nCalendar,
        const Locale & rLocale, const Sequence< Calendar2 > & calendarsSeq )
        throw(RuntimeException)
{
    Sequence< CalendarItem2 > aItems;
    if ( OUString( allCalendars[rnOffset] ) == "ref" )
    {
        aItems = getCalendarItemByName( OUString( allCalendars[rnOffset+1]), rLocale, calendarsSeq, nWhichItem);
        rnOffset += 2;
    }
    else
    {
        sal_Int32 nSize = allCalendars[nWhichItem][nCalendar];
        aItems.realloc( nSize);
        CalendarItem2* pItem = aItems.getArray();
        switch (nWhichItem)
        {
            case REF_DAYS:
            case REF_MONTHS:
            case REF_GMONTHS:
            case REF_PMONTHS:
                for (sal_Int16 j = 0; j < nSize; ++j, ++pItem)
                {
                    CalendarItem2 item( allCalendars[rnOffset], allCalendars[rnOffset+1],
                            allCalendars[rnOffset+2], allCalendars[rnOffset+3]);
                    *pItem = item;
                    rnOffset += 4;
                }
                break;
            case REF_ERAS:
                // Absent narrow name.
                for (sal_Int16 j = 0; j < nSize; ++j, ++pItem)
                {
                    CalendarItem2 item( allCalendars[rnOffset], allCalendars[rnOffset+1],
                            allCalendars[rnOffset+2], OUString());
                    *pItem = item;
                    rnOffset += 3;
                }
                break;
            default:
                OSL_FAIL( "LocaleData::getCalendarItems: unhandled REF_* case");
        }
    }
    return aItems;
}


Sequence< Calendar2 > SAL_CALL
LocaleData::getAllCalendars2( const Locale& rLocale ) throw(RuntimeException)
{

    sal_Unicode const * const * allCalendars = NULL;

    MyFunc_Type func = (MyFunc_Type) getFunctionSymbol( rLocale, "getAllCalendars" );

    if ( func ) {
        sal_Int16 calendarsCount = 0;
        allCalendars = func(calendarsCount);

        Sequence< Calendar2 > calendarsSeq(calendarsCount);
        sal_Int16 offset = REF_OFFSET_COUNT;
        for(sal_Int16 i = 0; i < calendarsCount; i++) {
            OUString calendarID(allCalendars[offset]);
            offset++;
            sal_Bool defaultCalendar = sal::static_int_cast<sal_Bool>( allCalendars[offset][0] );
            offset++;
            Sequence< CalendarItem2 > days = getCalendarItems( allCalendars, offset, REF_DAYS, i,
                    rLocale, calendarsSeq);
            Sequence< CalendarItem2 > months = getCalendarItems( allCalendars, offset, REF_MONTHS, i,
                    rLocale, calendarsSeq);
            Sequence< CalendarItem2 > gmonths = getCalendarItems( allCalendars, offset, REF_GMONTHS, i,
                    rLocale, calendarsSeq);
            Sequence< CalendarItem2 > pmonths = getCalendarItems( allCalendars, offset, REF_PMONTHS, i,
                    rLocale, calendarsSeq);
            Sequence< CalendarItem2 > eras = getCalendarItems( allCalendars, offset, REF_ERAS, i,
                    rLocale, calendarsSeq);
            OUString startOfWeekDay(allCalendars[offset]);
            offset++;
            sal_Int16 minimalDaysInFirstWeek = allCalendars[offset][0];
            offset++;
            Calendar2 aCalendar(days, months, gmonths, pmonths, eras, startOfWeekDay,
                    minimalDaysInFirstWeek, defaultCalendar, calendarID);
            calendarsSeq[i] = aCalendar;
        }
        return calendarsSeq;
    }
    else {
        Sequence< Calendar2 > seq1(0);
        return seq1;
    }
}


Sequence< Calendar > SAL_CALL
LocaleData::getAllCalendars( const Locale& rLocale ) throw(RuntimeException)
{
    const Sequence< Calendar2 > aCal2( getAllCalendars2( rLocale));
    sal_Int32 nLen = aCal2.getLength();
    Sequence< Calendar > aCal1( nLen);
    const Calendar2* p2 = aCal2.getConstArray();
    Calendar* p1 = aCal1.getArray();
    for (sal_Int32 i=0; i < nLen; ++i, ++p1, ++p2)
    {
        *p1 = downcastCalendar( *p2);
    }
    return aCal1;
}


Sequence< Currency2 > SAL_CALL
LocaleData::getAllCurrencies2( const Locale& rLocale ) throw(RuntimeException)
{
    sal_Unicode **allCurrencies = NULL;

    MyFunc_Type func = (MyFunc_Type) getFunctionSymbol( rLocale, "getAllCurrencies" );

    if ( func ) {
        sal_Int16 currencyCount = 0;
        allCurrencies = func(currencyCount);

        Sequence< Currency2 > seq(currencyCount);
        for(int i = 0, nOff = 0; i < currencyCount; i++, nOff += 8 ) {
            Currency2 cur(
                    allCurrencies[nOff],        // string ID
                    allCurrencies[nOff+1],      // string Symbol
                    allCurrencies[nOff+2],      // string BankSymbol
                    allCurrencies[nOff+3],      // string Name
                    allCurrencies[nOff+4][0] != 0, // boolean Default
                    allCurrencies[nOff+5][0] != 0, // boolean UsedInCompatibleFormatCodes
                    allCurrencies[nOff+6][0],   // short DecimalPlaces
                    allCurrencies[nOff+7][0] != 0 // boolean LegacyOnly
                    );
            seq[i] = cur;
        }
        return seq;
    }
    else {
        Sequence< Currency2 > seq1(0);
        return seq1;
    }
}


Sequence< Currency > SAL_CALL
LocaleData::getAllCurrencies( const Locale& rLocale ) throw(RuntimeException)
{
    Sequence< Currency2 > aCur2( getAllCurrencies2( rLocale));
    sal_Int32 nLen = aCur2.getLength();
    Sequence< Currency > aCur1( nLen);
    const Currency2* p2 = aCur2.getArray();
    Currency* p1 = aCur1.getArray();
    for (sal_Int32 i=0; i < nLen; ++i, ++p1, ++p2)
    {
        *p1 = *p2;
    }
    return aCur1;
}


// return a static (!) string resulting from replacing all occurrences of
// 'oldStr' string in 'formatCode' string with 'newStr' string
static const sal_Unicode * replace( sal_Unicode const * const formatCode, sal_Unicode const * const oldStr, sal_Unicode const * const newStr)
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
    const int SECTIONS = 2;
    struct FormatSection
    {
        MyFunc_FormatCode         func;
        sal_Unicode const        *from;
        sal_Unicode const        *to;
        sal_Unicode const *const *formatArray;
        sal_Int16                 formatCount;

        FormatSection() : func(0), from(0), to(0), formatArray(0), formatCount(0) {}
        sal_Int16 getFunc( LocaleData& rLocaleData, const Locale& rL, const char* pName )
        {
            func = reinterpret_cast<MyFunc_FormatCode>( rLocaleData.getFunctionSymbol( rL, pName));
            if (func)
                formatArray = func( formatCount, from, to);
            return formatCount;
        }
    } section[SECTIONS];

    sal_Int32 formatCount;
    formatCount  = section[0].getFunc( *this, rLocale, "getAllFormats0");
    formatCount += section[1].getFunc( *this, rLocale, "getAllFormats1");

    Sequence< FormatElement > seq(formatCount);
    sal_Int32 f = 0;
    for (int s = 0; s < SECTIONS; ++s)
    {
        sal_Unicode const * const * const formatArray = section[s].formatArray;
        if ( formatArray )
        {
            for (int i = 0, nOff = 0; i < section[s].formatCount; ++i, nOff += 7, ++f)
            {
                FormatElement elem(
                        replace( formatArray[nOff], section[s].from, section[s].to),
                        formatArray[nOff + 1],
                        formatArray[nOff + 2],
                        formatArray[nOff + 3],
                        formatArray[nOff + 4],
                        formatArray[nOff + 5][0],
                        sal::static_int_cast<sal_Bool>(formatArray[nOff + 6][0]));
                seq[f] = elem;
            }
        }
    }
    return seq;
}


Sequence< OUString > SAL_CALL
LocaleData::getDateAcceptancePatterns( const Locale& rLocale ) throw(RuntimeException)
{
    sal_Unicode **patternsArray = NULL;

    MyFunc_Type func = (MyFunc_Type) getFunctionSymbol( rLocale, "getDateAcceptancePatterns" );

    if (func)
    {
        sal_Int16 patternsCount = 0;
        patternsArray = func( patternsCount );
        Sequence< OUString > seq( patternsCount );
        for (sal_Int16 i = 0; i < patternsCount; ++i)
        {
            seq[i] = OUString( patternsArray[i] );
        }
        return seq;
    }
    else
    {
        Sequence< OUString > seq(0);
        return seq;
    }
}


#define COLLATOR_OFFSET_ALGO    0
#define COLLATOR_OFFSET_DEFAULT 1
#define COLLATOR_OFFSET_RULE    2
#define COLLATOR_ELEMENTS       3

OUString SAL_CALL
LocaleData::getCollatorRuleByAlgorithm( const Locale& rLocale, const OUString& algorithm ) throw(RuntimeException)
{
    MyFunc_Type func = (MyFunc_Type) getFunctionSymbol( rLocale, "getCollatorImplementation" );
    if ( func ) {
        sal_Int16 collatorCount = 0;
        sal_Unicode **collatorArray = func(collatorCount);
        for(sal_Int16 i = 0; i < collatorCount; i++)
            if (algorithm.equals(collatorArray[i * COLLATOR_ELEMENTS + COLLATOR_OFFSET_ALGO]))
                return OUString(collatorArray[i * COLLATOR_ELEMENTS + COLLATOR_OFFSET_RULE]);
    }
    return OUString();
}


Sequence< Implementation > SAL_CALL
LocaleData::getCollatorImplementations( const Locale& rLocale ) throw(RuntimeException)
{
    sal_Unicode **collatorArray = NULL;

    MyFunc_Type func = (MyFunc_Type) getFunctionSymbol( rLocale, "getCollatorImplementation" );

    if ( func ) {
        sal_Int16 collatorCount = 0;
        collatorArray = func(collatorCount);
        Sequence< Implementation > seq(collatorCount);
        for(sal_Int16 i = 0; i < collatorCount; i++) {
            Implementation impl(collatorArray[i * COLLATOR_ELEMENTS + COLLATOR_OFFSET_ALGO],
                    sal::static_int_cast<sal_Bool>(
                        collatorArray[i * COLLATOR_ELEMENTS + COLLATOR_OFFSET_DEFAULT][0]));
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
    sal_Unicode **optionsArray = NULL;

    MyFunc_Type func = (MyFunc_Type) getFunctionSymbol( rLocale, "getCollationOptions" );

    if ( func ) {
        sal_Int16 optionsCount = 0;
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
    sal_Unicode **optionsArray = NULL;

    MyFunc_Type func = (MyFunc_Type) getFunctionSymbol( rLocale, "getSearchOptions" );

    if ( func ) {
        sal_Int16 optionsCount = 0;
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
            seq[i] = indexArray[i*5];
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
            if (indexArray[i*5 + 3][0])
                return OUString(indexArray[i*5]);
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
            if (indexArray[i*5 + 4][0])
                return sal_True;
        }
    }
    return sal_False;
}

sal_Unicode ** SAL_CALL
LocaleData::getIndexArrayForAlgorithm(const Locale& rLocale, const OUString& algorithm)
{
    sal_Int16 indexCount = 0;
    sal_Unicode **indexArray = getIndexArray(rLocale, indexCount);
    if ( indexArray ) {
        for(sal_Int16 i = 0; i < indexCount; i++) {
            if (algorithm.equals(indexArray[i*5]))
                return indexArray+i*5;
        }
    }
    return NULL;
}

sal_Bool SAL_CALL
LocaleData::isPhonetic( const Locale& rLocale, const OUString& algorithm ) throw(RuntimeException)
{
    sal_Unicode **indexArray = getIndexArrayForAlgorithm(rLocale, algorithm);
    return (indexArray && indexArray[4][0]) ? sal_True : sal_False;
}

OUString SAL_CALL
LocaleData::getIndexKeysByAlgorithm( const Locale& rLocale, const OUString& algorithm ) throw(RuntimeException)
{
    sal_Unicode **indexArray = getIndexArrayForAlgorithm(rLocale, algorithm);
    return indexArray ? OUString("0-9")+OUString(indexArray[2]) : OUString();
}

OUString SAL_CALL
LocaleData::getIndexModuleByAlgorithm( const Locale& rLocale, const OUString& algorithm ) throw(RuntimeException)
{
    sal_Unicode **indexArray = getIndexArrayForAlgorithm(rLocale, algorithm);
    return indexArray ? OUString(indexArray[1]) : OUString();
}

Sequence< UnicodeScript > SAL_CALL
LocaleData::getUnicodeScripts( const Locale& rLocale ) throw(RuntimeException)
{
    sal_Unicode **scriptArray = NULL;

    MyFunc_Type func = (MyFunc_Type) getFunctionSymbol( rLocale, "getUnicodeScripts" );

    if ( func ) {
        sal_Int16 scriptCount = 0;
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
    sal_Unicode **wordArray = NULL;

    MyFunc_Type func = (MyFunc_Type) getFunctionSymbol( rLocale, "getFollowPageWords" );

    if ( func ) {
        sal_Int16 wordCount = 0;
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

    sal_Unicode **transliterationsArray = NULL;

    MyFunc_Type func = (MyFunc_Type) getFunctionSymbol( rLocale, "getTransliterations" );

    if ( func ) {
        sal_Int16 transliterationsCount = 0;
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

    sal_Unicode **LCInfoArray = NULL;

    MyFunc_Type func = (MyFunc_Type) getFunctionSymbol( rLocale, "getLCInfo" );

    if ( func ) {
        sal_Int16 LCInfoCount = 0;
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
    sal_Unicode **LCForbiddenCharactersArray = NULL;

    MyFunc_Type func = (MyFunc_Type) getFunctionSymbol( rLocale, "getForbiddenCharacters" );

    if ( func ) {
        sal_Int16 LCForbiddenCharactersCount = 0;
        LCForbiddenCharactersArray = func(LCForbiddenCharactersCount);
        ForbiddenCharacters chars(LCForbiddenCharactersArray[0], LCForbiddenCharactersArray[1]);
        return chars;
    }
    else {
        ForbiddenCharacters chars1;
        return chars1;
    }
}

OUString SAL_CALL
LocaleData::getHangingCharacters( const Locale& rLocale ) throw(RuntimeException)
{
    sal_Unicode **LCForbiddenCharactersArray = NULL;

    MyFunc_Type func = (MyFunc_Type) getFunctionSymbol( rLocale, "getForbiddenCharacters" );

    if ( func ) {
        sal_Int16 LCForbiddenCharactersCount = 0;
        LCForbiddenCharactersArray = func(LCForbiddenCharactersCount);
        return OUString(LCForbiddenCharactersArray[2]);
    }

    return OUString();
}

Sequence< OUString > SAL_CALL
LocaleData::getBreakIteratorRules( const Locale& rLocale  ) throw(RuntimeException)
{
    sal_Unicode **LCBreakIteratorRulesArray = NULL;

    MyFunc_Type func = (MyFunc_Type) getFunctionSymbol( rLocale, "getBreakIteratorRules" );

    if ( func ) {
        sal_Int16 LCBreakIteratorRuleCount = 0;
        LCBreakIteratorRulesArray = func(LCBreakIteratorRuleCount);
        Sequence< OUString > seq(LCBreakIteratorRuleCount);
        for(int i = 0; i < (LCBreakIteratorRuleCount); i++) {
            OUString  elem(LCBreakIteratorRulesArray[i]);
            seq[i] = elem;
        }
        return seq;
    }
    else {
        Sequence< OUString > seq1(0);
        return seq1;
    }
}


Sequence< OUString > SAL_CALL
LocaleData::getReservedWord( const Locale& rLocale  ) throw(RuntimeException)
{
    sal_Unicode **LCReservedWordsArray = NULL;

    MyFunc_Type func = (MyFunc_Type) getFunctionSymbol( rLocale, "getReservedWords" );

    if ( func ) {
        sal_Int16 LCReservedWordsCount = 0;
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


Sequence< Sequence<beans::PropertyValue> > SAL_CALL
LocaleData::getContinuousNumberingLevels( const lang::Locale& rLocale ) throw(RuntimeException)
{
    // load symbol
    MyFunc_Type2 func = (MyFunc_Type2) getFunctionSymbol( rLocale, "getContinuousNumberingLevels" );

    if ( func )
    {
        int i;
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
                        rVal.Name = "Prefix";
                        rVal.Value <<= sVal;
                        break;
                    case 1:
                        rVal.Name = "NumberingType";
                        rVal.Value <<= (sal_Int16) sVal.toInt32();
                        break;
                    case 2:
                        rVal.Name = "Suffix";
                        rVal.Value <<= sVal;
                        break;
                    case 3:
                        rVal.Name = "Transliteration";
                        rVal.Value <<= sVal;
                        break;
                    case 4:
                        rVal.Name = "NatNum";
                        rVal.Value <<= (sal_Int16) sVal.toInt32();
                        break;
                    default:
                        OSL_ASSERT(0);
                }
            }
        }
        return pv;
    }

    Sequence< Sequence<beans::PropertyValue> > seq1(0);
    return seq1;
}

// OutlineNumbering helper class

#include <com/sun/star/container/XIndexAccess.hpp>
#include <cppuhelper/implbase1.hxx>

namespace com{ namespace sun{ namespace star{ namespace lang {
    struct  Locale;
}}}}
//-----------------------------------------------------------------------------
struct OutlineNumberingLevel_Impl
{
    OUString        sPrefix;
    sal_Int16       nNumType; //com::sun::star::style::NumberingType
    OUString        sSuffix;
    sal_Unicode     cBulletChar;
    const sal_Char* sBulletFontName;
    sal_Int16       nParentNumbering;
    sal_Int32       nLeftMargin;
    sal_Int32       nSymbolTextDistance;
    sal_Int32       nFirstLineOffset;
    OUString        sTransliteration;
    sal_Int32       nNatNum;
};

class OutlineNumbering : public cppu::WeakImplHelper1 < container::XIndexAccess >
{
    // OutlineNumbering helper class

    const OutlineNumberingLevel_Impl* m_pOutlineLevels;
    sal_Int16                         m_nCount;
public:
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

static sal_Char* U2C( OUString str )
{
    sal_Char* s = new sal_Char[ str.getLength()+1 ];
    int i;
    for( i = 0; i < str.getLength(); i++)
        s[i] = sal::static_int_cast<sal_Char>( str[i] );
    s[i]='\0';
    return s;
}


Sequence< Reference<container::XIndexAccess> > SAL_CALL
LocaleData::getOutlineNumberingLevels( const lang::Locale& rLocale ) throw(RuntimeException)
{
    // load symbol
    MyFunc_Type3 func = (MyFunc_Type3) getFunctionSymbol( rLocale, "getOutlineNumberingLevels" );

    if ( func )
    {
        int i;
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
                        case 1: level[j].nNumType            = sal::static_int_cast<sal_Int16>(tmp.toInt32()); break;
                        case 2: level[j].sSuffix             = tmp;             break;
                        case 3: level[j].cBulletChar         = sal::static_int_cast<sal_Unicode>(tmp.toUInt32(16)); break; // base 16
                        case 4: level[j].sBulletFontName     = U2C( tmp );      break;
                        case 5: level[j].nParentNumbering    = sal::static_int_cast<sal_Int16>(tmp.toInt32()); break;
                        case 6: level[j].nLeftMargin         = tmp.toInt32();   break;
                        case 7: level[j].nSymbolTextDistance = tmp.toInt32();   break;
                        case 8: level[j].nFirstLineOffset    = tmp.toInt32();   break;
                        case 9: break;
                        case 10: level[j].sTransliteration = tmp; break;
                        case 11: level[j].nNatNum    = tmp.toInt32();   break;
                        default:
                                 OSL_ASSERT(0);
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

// helper functions

oslGenericFunction SAL_CALL LocaleData::getFunctionSymbol( const Locale& rLocale, const sal_Char* pFunction )
        throw(RuntimeException)
{
    lcl_LookupTableHelper & rLookupTable = lcl_LookupTableStatic::get();

    OUStringBuffer aBuf(1);
    if (cachedItem.get() && cachedItem->equals(rLocale))
    {
        aBuf.ensureCapacity(strlen(pFunction) + 1 + strlen(cachedItem->localeName));
        return cachedItem->module->getFunctionSymbol(aBuf.appendAscii(pFunction).append(cUnder).
                appendAscii(cachedItem->localeName).makeStringAndClear());
    }

    oslGenericFunction pSymbol = 0;
    LocaleDataLookupTableItem *pCachedItem = 0;

    // Load function with name <func>_<lang>_<country> or <func>_<bcp47> and
    // fallbacks.
    pSymbol = rLookupTable.getFunctionSymbolByName( LocaleData::getFirstLocaleServiceName( rLocale),
            pFunction, &pCachedItem);
    if (!pSymbol)
    {
        ::std::vector< OUString > aFallbacks( LocaleData::getFallbackLocaleServiceNames( rLocale));
        for (::std::vector< OUString >::const_iterator it( aFallbacks.begin()); it != aFallbacks.end(); ++it)
        {
            pSymbol = rLookupTable.getFunctionSymbolByName( *it, pFunction, &pCachedItem);
            if (pSymbol)
                break;
        }
    }
    if (!pSymbol)
    {
        // load default function with name <func>_en_US
        pSymbol = rLookupTable.getFunctionSymbolByName(OUString("en_US"), pFunction, &pCachedItem);
    }

    if (!pSymbol)
        // Appropriate symbol could not be found.  Give up.
        throw RuntimeException();

    if (pCachedItem)
        cachedItem.reset(pCachedItem);
    if (cachedItem.get())
        cachedItem->aLocale = rLocale;

    return pSymbol;
}

Sequence< Locale > SAL_CALL
LocaleData::getAllInstalledLocaleNames() throw(RuntimeException)
{
    Sequence< lang::Locale > seq( nbOfLocales );
    OUString empStr;
    sal_Int16 nInstalled = 0;

    for( sal_Int16 i=0; i<nbOfLocales; i++ ) {
        OUString name = OUString::createFromAscii( aLibTable[i].pLocale );

        // Check if the locale is really available and not just in the table,
        // don't allow fall backs.
        LocaleDataLookupTableItem *pCachedItem = 0;
        if (lcl_LookupTableStatic::get().getFunctionSymbolByName( name, "getLocaleItem", &pCachedItem )) {
            if( pCachedItem )
                cachedItem.reset( pCachedItem );
            seq[nInstalled++] = LanguageTag::convertToLocale( name.replace( cUnder, cHyphen), false);
        }
        else
        {
            delete pCachedItem;
        }
    }
    if ( nInstalled < nbOfLocales )
        seq.realloc( nInstalled );          // reflect reality

    return seq;
}

using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::text;

OutlineNumbering::OutlineNumbering(const OutlineNumberingLevel_Impl* pOutlnLevels, int nLevels) :
    m_pOutlineLevels(pOutlnLevels),
    m_nCount(sal::static_int_cast<sal_Int16>(nLevels))
{
}

OutlineNumbering::~OutlineNumbering()
{
    delete [] m_pOutlineLevels;
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
    pValues[0].Name = "Prefix";
    pValues[0].Value <<= pTemp->sPrefix;
    pValues[1].Name = "NumberingType";
    pValues[1].Value <<= pTemp->nNumType;
    pValues[2].Name = "Suffix";
    pValues[2].Value <<= pTemp->sSuffix;
    pValues[3].Name = "BulletChar";
    pValues[3].Value <<= OUString(&pTemp->cBulletChar, 1);
    pValues[4].Name = "BulletFontName";
    pValues[4].Value <<= OUString::createFromAscii(pTemp->sBulletFontName);
    pValues[5].Name = "ParentNumbering";
    pValues[5].Value <<= OUString::number(pTemp->nParentNumbering);
    pValues[6].Name = "LeftMargin";
    pValues[6].Value <<= pTemp->nLeftMargin;
    pValues[7].Name = "SymbolTextDistance";
    pValues[7].Value <<= pTemp->nSymbolTextDistance;
    pValues[8].Name = "FirstLineOffset";
    pValues[8].Value <<= pTemp->nFirstLineOffset;
    pValues[9].Name = "Adjust";
    pValues[9].Value <<= (sal_Int16)HoriOrientation::LEFT;
    pValues[10].Name = "Transliteration";
    pValues[10].Value <<= pTemp->sTransliteration;
    pValues[11].Name = "NatNum";
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
    return OUString(clocaledata);
}

sal_Bool SAL_CALL
LocaleData::supportsService(const OUString& rServiceName)
        throw( RuntimeException )
{
    return rServiceName == clocaledata;
}

Sequence< OUString > SAL_CALL
LocaleData::getSupportedServiceNames() throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString(clocaledata);
    return aRet;
}

// static
OUString LocaleData::getFirstLocaleServiceName( const com::sun::star::lang::Locale & rLocale )
{
    if (rLocale.Language == I18NLANGTAG_QLT)
        return rLocale.Variant.replace( cHyphen, cUnder);
    else if (!rLocale.Country.isEmpty())
        return rLocale.Language + "_" + rLocale.Country;
    else
        return rLocale.Language;
}

// static
::std::vector< OUString > LocaleData::getFallbackLocaleServiceNames( const com::sun::star::lang::Locale & rLocale )
{
    ::std::vector< OUString > aVec;
    if (rLocale.Language == I18NLANGTAG_QLT)
    {
        aVec = LanguageTag( rLocale).getFallbackStrings();
        aVec.erase( aVec.begin());
        for (::std::vector< OUString >::iterator it(aVec.begin()); it != aVec.end(); ++it)
        {
            *it = (*it).replace( cHyphen, cUnder);
        }
    }
    else if (!rLocale.Country.isEmpty())
    {
        aVec.push_back( rLocale.Language);
    }
    // else nothing, language-only was the first
    return aVec;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
