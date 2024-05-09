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

#include <memory>
#include <mutex>
#include <string_view>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <comphelper/sequence.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <editeng/svxenum.hxx>
#include <localedata.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <sal/macros.h>
#include <o3tl/string_view.hxx>

namespace com::sun::star::uno { class XComponentContext; }

using namespace com::sun::star::i18n;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star;
using i18npool::DataLocaleLibrary;

typedef OUString const * (* MyFuncOUString_Type)( sal_Int16&);
typedef OUString const ** (* MyFunc_Type2)( sal_Int16&, sal_Int16& );
typedef OUString const *** (* MyFunc_Type3)( sal_Int16&, sal_Int16&, sal_Int16& );
typedef OUString const * (* MyFunc_FormatCode)( sal_Int16&, sal_Unicode const *&, sal_Unicode const *& );

#ifndef DISABLE_DYNLOADING

#ifdef SAL_DLLPREFIX
// mostly "lib*.so"
constexpr const char* lcl_DATA_EN = SAL_DLLPREFIX "localedata_en" SAL_DLLEXTENSION;
constexpr const char* lcl_DATA_ES = SAL_DLLPREFIX "localedata_es" SAL_DLLEXTENSION;
constexpr const char* lcl_DATA_EURO = SAL_DLLPREFIX "localedata_euro" SAL_DLLEXTENSION;
constexpr const char* lcl_DATA_OTHERS = SAL_DLLPREFIX "localedata_others" SAL_DLLEXTENSION;
#else
// mostly "*.dll"
constexpr const char* lcl_DATA_EN = "localedata_en" SAL_DLLEXTENSION;
constexpr const char* lcl_DATA_ES = "localedata_es" SAL_DLLEXTENSION;
constexpr const char* lcl_DATA_EURO = "localedata_euro" SAL_DLLEXTENSION;
constexpr const char* lcl_DATA_OTHERS = "localedata_others" SAL_DLLEXTENSION;
#endif

static const char* getLibraryName(DataLocaleLibrary aLib) {
    switch(aLib) {
        case DataLocaleLibrary::EN:
            return lcl_DATA_EN;
        case DataLocaleLibrary::ES:
            return lcl_DATA_ES;
        case DataLocaleLibrary::EURO:
            return lcl_DATA_EURO;
        case DataLocaleLibrary::OTHERS:
            return lcl_DATA_OTHERS;
    }
    return nullptr;
}

constexpr struct {
    const char* pLocale;
    i18npool::DataLocaleLibrary pLib;
} aLibTable[] = {
    { "en_US",  DataLocaleLibrary::EN },
    { "en_AU",  DataLocaleLibrary::EN },
    { "en_BZ",  DataLocaleLibrary::EN },
    { "en_CA",  DataLocaleLibrary::EN },
    { "en_GB",  DataLocaleLibrary::EN },
    { "en_IE",  DataLocaleLibrary::EN },
    { "en_JM",  DataLocaleLibrary::EN },
    { "en_NZ",  DataLocaleLibrary::EN },
    { "en_PH",  DataLocaleLibrary::EN },
    { "en_TT",  DataLocaleLibrary::EN },
    { "en_ZA",  DataLocaleLibrary::EN },
    { "en_ZW",  DataLocaleLibrary::EN },
    { "en_NA",  DataLocaleLibrary::EN },
    { "en_GH",  DataLocaleLibrary::EN },
    { "en_MW",  DataLocaleLibrary::EN },
    { "en_GM",  DataLocaleLibrary::EN },
    { "en_BW",  DataLocaleLibrary::EN },
    { "en_ZM",  DataLocaleLibrary::EN },
    { "en_LK",  DataLocaleLibrary::EN },
    { "en_NG",  DataLocaleLibrary::EN },
    { "en_KE",  DataLocaleLibrary::EN },
    { "en_DK",  DataLocaleLibrary::EN },
    { "en_MU",  DataLocaleLibrary::EN },

    { "es_ES",  DataLocaleLibrary::ES },
    { "es_AR",  DataLocaleLibrary::ES },
    { "es_BO",  DataLocaleLibrary::ES },
    { "es_CL",  DataLocaleLibrary::ES },
    { "es_CO",  DataLocaleLibrary::ES },
    { "es_CR",  DataLocaleLibrary::ES },
    { "es_DO",  DataLocaleLibrary::ES },
    { "es_EC",  DataLocaleLibrary::ES },
    { "es_GT",  DataLocaleLibrary::ES },
    { "es_HN",  DataLocaleLibrary::ES },
    { "es_MX",  DataLocaleLibrary::ES },
    { "es_NI",  DataLocaleLibrary::ES },
    { "es_PA",  DataLocaleLibrary::ES },
    { "es_PE",  DataLocaleLibrary::ES },
    { "es_PR",  DataLocaleLibrary::ES },
    { "es_PY",  DataLocaleLibrary::ES },
    { "es_SV",  DataLocaleLibrary::ES },
    { "es_UY",  DataLocaleLibrary::ES },
    { "es_VE",  DataLocaleLibrary::ES },
    { "gl_ES",  DataLocaleLibrary::ES },
    { "oc_ES_aranes",  DataLocaleLibrary::ES },

    { "de_DE",  DataLocaleLibrary::EURO },
    { "de_AT",  DataLocaleLibrary::EURO },
    { "de_CH",  DataLocaleLibrary::EURO },
    { "de_LI",  DataLocaleLibrary::EURO },
    { "de_LU",  DataLocaleLibrary::EURO },
    { "fr_FR",  DataLocaleLibrary::EURO },
    { "fr_BE",  DataLocaleLibrary::EURO },
    { "fr_CA",  DataLocaleLibrary::EURO },
    { "fr_CH",  DataLocaleLibrary::EURO },
    { "fr_LU",  DataLocaleLibrary::EURO },
    { "fr_MC",  DataLocaleLibrary::EURO },
    { "fr_BF",  DataLocaleLibrary::EURO },
    { "fr_CI",  DataLocaleLibrary::EURO },
    { "fr_ML",  DataLocaleLibrary::EURO },
    { "fr_SN",  DataLocaleLibrary::EURO },
    { "fr_BJ",  DataLocaleLibrary::EURO },
    { "fr_NE",  DataLocaleLibrary::EURO },
    { "fr_TG",  DataLocaleLibrary::EURO },
    { "it_IT",  DataLocaleLibrary::EURO },
    { "it_CH",  DataLocaleLibrary::EURO },
    { "sl_SI",  DataLocaleLibrary::EURO },
    { "sv_SE",  DataLocaleLibrary::EURO },
    { "sv_FI",  DataLocaleLibrary::EURO },
    { "ca_ES",  DataLocaleLibrary::EURO },
    { "ca_ES_valencia",  DataLocaleLibrary::EURO },
    { "cs_CZ",  DataLocaleLibrary::EURO },
    { "sk_SK",  DataLocaleLibrary::EURO },
    { "da_DK",  DataLocaleLibrary::EURO },
    { "el_GR",  DataLocaleLibrary::EURO },
    { "fi_FI",  DataLocaleLibrary::EURO },
    { "is_IS",  DataLocaleLibrary::EURO },
    { "nl_BE",  DataLocaleLibrary::EURO },
    { "nl_NL",  DataLocaleLibrary::EURO },
    { "no_NO",  DataLocaleLibrary::EURO },
    { "nn_NO",  DataLocaleLibrary::EURO },
    { "nb_NO",  DataLocaleLibrary::EURO },
    { "nds_DE", DataLocaleLibrary::EURO },
    { "pl_PL",  DataLocaleLibrary::EURO },
    { "pt_PT",  DataLocaleLibrary::EURO },
    { "pt_BR",  DataLocaleLibrary::EURO },  // needs to be in EURO because inherits from pt_PT
    { "pt_MZ",  DataLocaleLibrary::EURO },  // needs to be in EURO because inherits from pt_PT
    { "ru_RU",  DataLocaleLibrary::EURO },
    { "tr_TR",  DataLocaleLibrary::EURO },
    { "tt_RU",  DataLocaleLibrary::EURO },
    { "et_EE",  DataLocaleLibrary::EURO },
    { "vro_EE", DataLocaleLibrary::EURO },
    { "lb_LU",  DataLocaleLibrary::EURO },
    { "lt_LT",  DataLocaleLibrary::EURO },
    { "lv_LV",  DataLocaleLibrary::EURO },
    { "uk_UA",  DataLocaleLibrary::EURO },
    { "ro_RO",  DataLocaleLibrary::EURO },
    { "cy_GB",  DataLocaleLibrary::EURO },
    { "bg_BG",  DataLocaleLibrary::EURO },
    { "sr_Latn_ME",  DataLocaleLibrary::EURO },
    { "sr_Latn_RS",  DataLocaleLibrary::EURO },
    { "sr_Latn_CS",  DataLocaleLibrary::EURO },
    { "sr_ME",  DataLocaleLibrary::EURO },
    { "sr_RS",  DataLocaleLibrary::EURO },
    { "sr_CS",  DataLocaleLibrary::EURO },
    { "hr_HR",  DataLocaleLibrary::EURO },
    { "bs_BA",  DataLocaleLibrary::EURO },
    { "eu_ES",  DataLocaleLibrary::EURO },
    { "fo_FO",  DataLocaleLibrary::EURO },
    { "ga_IE",  DataLocaleLibrary::EURO },
    { "gd_GB",  DataLocaleLibrary::EURO },
    { "ka_GE",  DataLocaleLibrary::EURO },
    { "be_BY",  DataLocaleLibrary::EURO },
    { "kl_GL",  DataLocaleLibrary::EURO },
    { "mk_MK",  DataLocaleLibrary::EURO },
    { "br_FR",  DataLocaleLibrary::EURO },
    { "la_VA",  DataLocaleLibrary::EURO },
    { "cv_RU",  DataLocaleLibrary::EURO },
    { "wa_BE",  DataLocaleLibrary::EURO },
    { "fur_IT", DataLocaleLibrary::EURO },
    { "gsc_FR", DataLocaleLibrary::EURO },
    { "fy_NL",  DataLocaleLibrary::EURO },
    { "oc_FR_lengadoc",  DataLocaleLibrary::EURO },
    { "mt_MT",  DataLocaleLibrary::EURO },
    { "sc_IT",  DataLocaleLibrary::EURO },
    { "ast_ES", DataLocaleLibrary::EURO },
    { "ltg_LV", DataLocaleLibrary::EURO },
    { "hsb_DE", DataLocaleLibrary::EURO },
    { "dsb_DE", DataLocaleLibrary::EURO },
    { "rue_SK", DataLocaleLibrary::EURO },
    { "an_ES",  DataLocaleLibrary::EURO },
    { "myv_RU", DataLocaleLibrary::EURO },
    { "lld_IT", DataLocaleLibrary::EURO },
    { "cu_RU",  DataLocaleLibrary::EURO },
    { "vec_IT", DataLocaleLibrary::EURO },
    { "szl_PL", DataLocaleLibrary::EURO },
    { "lij_IT", DataLocaleLibrary::EURO },

    { "ja_JP",  DataLocaleLibrary::OTHERS },
    { "ko_KR",  DataLocaleLibrary::OTHERS },
    { "zh_CN",  DataLocaleLibrary::OTHERS },
    { "zh_HK",  DataLocaleLibrary::OTHERS },
    { "zh_SG",  DataLocaleLibrary::OTHERS },
    { "zh_TW",  DataLocaleLibrary::OTHERS },
    { "zh_MO",  DataLocaleLibrary::OTHERS },
    { "en_HK",  DataLocaleLibrary::OTHERS },  // needs to be in OTHERS instead of EN because currency inherited from zh_HK

    { "ar_EG",  DataLocaleLibrary::OTHERS },
    { "ar_DZ",  DataLocaleLibrary::OTHERS },
    { "ar_LB",  DataLocaleLibrary::OTHERS },
    { "ar_SA",  DataLocaleLibrary::OTHERS },
    { "ar_TN",  DataLocaleLibrary::OTHERS },
    { "he_IL",  DataLocaleLibrary::OTHERS },
    { "hi_IN",  DataLocaleLibrary::OTHERS },
    { "kn_IN",  DataLocaleLibrary::OTHERS },
    { "ta_IN",  DataLocaleLibrary::OTHERS },
    { "te_IN",  DataLocaleLibrary::OTHERS },
    { "gu_IN",  DataLocaleLibrary::OTHERS },
    { "mr_IN",  DataLocaleLibrary::OTHERS },
    { "pa_IN",  DataLocaleLibrary::OTHERS },
    { "bn_IN",  DataLocaleLibrary::OTHERS },
    { "or_IN",  DataLocaleLibrary::OTHERS },
    { "en_IN",  DataLocaleLibrary::OTHERS },  // keep in OTHERS for IN
    { "ml_IN",  DataLocaleLibrary::OTHERS },
    { "bn_BD",  DataLocaleLibrary::OTHERS },
    { "th_TH",  DataLocaleLibrary::OTHERS },

    { "af_ZA",  DataLocaleLibrary::OTHERS },
    { "hu_HU",  DataLocaleLibrary::OTHERS },
    { "id_ID",  DataLocaleLibrary::OTHERS },
    { "ms_MY",  DataLocaleLibrary::OTHERS },
    { "en_MY",  DataLocaleLibrary::OTHERS },  // needs to be in OTHERS instead of EN because currency inherited from ms_MY
    { "ia",     DataLocaleLibrary::OTHERS },
    { "mn_Cyrl_MN",  DataLocaleLibrary::OTHERS },
    { "az_AZ",  DataLocaleLibrary::OTHERS },
    { "sw_TZ",  DataLocaleLibrary::OTHERS },
    { "km_KH",  DataLocaleLibrary::OTHERS },
    { "lo_LA",  DataLocaleLibrary::OTHERS },
    { "rw_RW",  DataLocaleLibrary::OTHERS },
    { "eo",     DataLocaleLibrary::OTHERS },
    { "dz_BT",  DataLocaleLibrary::OTHERS },
    { "ne_NP",  DataLocaleLibrary::OTHERS },
    { "zu_ZA",  DataLocaleLibrary::OTHERS },
    { "nso_ZA", DataLocaleLibrary::OTHERS },
    { "vi_VN",  DataLocaleLibrary::OTHERS },
    { "tn_ZA",  DataLocaleLibrary::OTHERS },
    { "xh_ZA",  DataLocaleLibrary::OTHERS },
    { "st_ZA",  DataLocaleLibrary::OTHERS },
    { "ss_ZA",  DataLocaleLibrary::OTHERS },
    { "ve_ZA",  DataLocaleLibrary::OTHERS },
    { "nr_ZA",  DataLocaleLibrary::OTHERS },
    { "ts_ZA",  DataLocaleLibrary::OTHERS },
    { "kmr_Latn_TR",  DataLocaleLibrary::OTHERS },
    { "ak_GH",  DataLocaleLibrary::OTHERS },
    { "af_NA",  DataLocaleLibrary::OTHERS },
    { "am_ET",  DataLocaleLibrary::OTHERS },
    { "ti_ER",  DataLocaleLibrary::OTHERS },
    { "tg_TJ",  DataLocaleLibrary::OTHERS },
    { "ky_KG",  DataLocaleLibrary::OTHERS },
    { "kk_KZ",  DataLocaleLibrary::OTHERS },
    { "fa_IR",  DataLocaleLibrary::OTHERS },
    { "ha_Latn_GH",  DataLocaleLibrary::OTHERS },
    { "ee_GH",  DataLocaleLibrary::OTHERS },
    { "sg_CF",  DataLocaleLibrary::OTHERS },
    { "lg_UG",  DataLocaleLibrary::OTHERS },
    { "uz_UZ",  DataLocaleLibrary::OTHERS },
    { "ln_CD",  DataLocaleLibrary::OTHERS },
    { "hy_AM",  DataLocaleLibrary::OTHERS },
    { "hil_PH", DataLocaleLibrary::OTHERS },
    { "so_SO",  DataLocaleLibrary::OTHERS },
    { "gug_PY", DataLocaleLibrary::OTHERS },
    { "tk_TM",  DataLocaleLibrary::OTHERS },
    { "my_MM",  DataLocaleLibrary::OTHERS },
    { "shs_CA", DataLocaleLibrary::OTHERS },
    { "tpi_PG", DataLocaleLibrary::OTHERS },
    { "ar_OM",  DataLocaleLibrary::OTHERS },
    { "ug_CN",  DataLocaleLibrary::OTHERS },
    { "om_ET",  DataLocaleLibrary::OTHERS },
    { "plt_MG", DataLocaleLibrary::OTHERS },
    { "mai_IN", DataLocaleLibrary::OTHERS },
    { "yi_US",  DataLocaleLibrary::OTHERS },
    { "haw_US", DataLocaleLibrary::OTHERS },
    { "lif_NP", DataLocaleLibrary::OTHERS },
    { "ur_PK",  DataLocaleLibrary::OTHERS },
    { "ht_HT",  DataLocaleLibrary::OTHERS },
    { "jbo",    DataLocaleLibrary::OTHERS },
    { "kab_DZ", DataLocaleLibrary::OTHERS },
    { "pt_AO",  DataLocaleLibrary::OTHERS },
    { "pjt_AU", DataLocaleLibrary::OTHERS },
    { "pap_BQ", DataLocaleLibrary::OTHERS },
    { "pap_CW", DataLocaleLibrary::OTHERS },
    { "ebo_CG", DataLocaleLibrary::OTHERS },
    { "tyx_CG", DataLocaleLibrary::OTHERS },
    { "axk_CG", DataLocaleLibrary::OTHERS },
    { "beq_CG", DataLocaleLibrary::OTHERS },
    { "bkw_CG", DataLocaleLibrary::OTHERS },
    { "bvx_CG", DataLocaleLibrary::OTHERS },
    { "dde_CG", DataLocaleLibrary::OTHERS },
    { "iyx_CG", DataLocaleLibrary::OTHERS },
    { "kkw_CG", DataLocaleLibrary::OTHERS },
    { "kng_CG", DataLocaleLibrary::OTHERS },
    { "ldi_CG", DataLocaleLibrary::OTHERS },
    { "mdw_CG", DataLocaleLibrary::OTHERS },
    { "mkw_CG", DataLocaleLibrary::OTHERS },
    { "njx_CG", DataLocaleLibrary::OTHERS },
    { "ngz_CG", DataLocaleLibrary::OTHERS },
    { "njy_CG", DataLocaleLibrary::OTHERS },
    { "puu_CG", DataLocaleLibrary::OTHERS },
    { "sdj_CG", DataLocaleLibrary::OTHERS },
    { "tek_CG", DataLocaleLibrary::OTHERS },
    { "tsa_CG", DataLocaleLibrary::OTHERS },
    { "vif_CG", DataLocaleLibrary::OTHERS },
    { "xku_CG", DataLocaleLibrary::OTHERS },
    { "yom_CG", DataLocaleLibrary::OTHERS },
    { "sid_ET", DataLocaleLibrary::OTHERS },
    { "bo_CN",  DataLocaleLibrary::OTHERS },
    { "bo_IN",  DataLocaleLibrary::OTHERS },
    { "ar_AE",  DataLocaleLibrary::OTHERS },
    { "ar_KW",  DataLocaleLibrary::OTHERS },
    { "bm_ML",  DataLocaleLibrary::OTHERS },
    { "pui_CO", DataLocaleLibrary::OTHERS },
    { "lgr_SB", DataLocaleLibrary::OTHERS },
    { "mos_BF", DataLocaleLibrary::OTHERS },
    { "ny_MW",  DataLocaleLibrary::OTHERS },
    { "ar_BH",  DataLocaleLibrary::OTHERS },
    { "ar_IQ",  DataLocaleLibrary::OTHERS },
    { "ar_JO",  DataLocaleLibrary::OTHERS },
    { "ar_LY",  DataLocaleLibrary::OTHERS },
    { "ar_MA",  DataLocaleLibrary::OTHERS },
    { "ar_QA",  DataLocaleLibrary::OTHERS },
    { "ar_SY",  DataLocaleLibrary::OTHERS },
    { "ar_YE",  DataLocaleLibrary::OTHERS },
    { "ilo_PH", DataLocaleLibrary::OTHERS },
    { "ha_Latn_NG",  DataLocaleLibrary::OTHERS },
    { "min_ID", DataLocaleLibrary::OTHERS },
    { "sun_ID", DataLocaleLibrary::OTHERS },
    { "en_IL",  DataLocaleLibrary::OTHERS },  // needs to be in OTHERS instead of EN because inherits from he_IL
    { "pdc_US", DataLocaleLibrary::OTHERS },
    { "dv_MV",  DataLocaleLibrary::OTHERS },
    { "mfe_MU", DataLocaleLibrary::OTHERS },
    { "sat_IN", DataLocaleLibrary::OTHERS }
};

#else

#include "localedata_static.hxx"

#endif

const sal_Unicode cUnder = '_';
const sal_Unicode cHyphen = '-';

const sal_Int16 nbOfLocales = SAL_N_ELEMENTS(aLibTable);

namespace i18npool {

// static
Sequence< CalendarItem > LocaleDataImpl::downcastCalendarItems( const Sequence< CalendarItem2 > & rCi )
{
    return comphelper::containerToSequence<CalendarItem>(rCi);
}


// static
Calendar LocaleDataImpl::downcastCalendar( const Calendar2 & rC )
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


LocaleDataImpl::LocaleDataImpl()
{
}
LocaleDataImpl::~LocaleDataImpl()
{
}


LocaleDataItem SAL_CALL
LocaleDataImpl::getLocaleItem( const Locale& rLocale )
{
    MyFuncOUString_Type func = reinterpret_cast<MyFuncOUString_Type>(getFunctionSymbol( rLocale, "getLocaleItem" ));

    if ( func ) {
        sal_Int16 dataItemCount = 0;
        OUString const *dataItem = func(dataItemCount);

        LocaleDataItem item{
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
                };
        return item;
    }
    else {
        LocaleDataItem item1;
        return item1;
    }
}


LocaleDataItem2 SAL_CALL
LocaleDataImpl::getLocaleItem2( const Locale& rLocale )
{
    MyFuncOUString_Type func = reinterpret_cast<MyFuncOUString_Type>(getFunctionSymbol( rLocale, "getLocaleItem" ));

    if ( func ) {
        sal_Int16 dataItemCount = 0;
        OUString const *dataItem = func(dataItemCount);

        assert(dataItemCount >= 18);

        LocaleDataItem2 item{
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
                dataItem[17],
                dataItemCount >= 19 ? dataItem[18] : OUString()
                };
        return item;
    }
    else {
        LocaleDataItem2 item1;
        return item1;
    }
}

#ifndef DISABLE_DYNLOADING

extern "C" { static void thisModule() {} }

#endif

namespace
{

// implement the lookup table as a safe static object
class lcl_LookupTableHelper
{
public:
    lcl_LookupTableHelper();
    ~lcl_LookupTableHelper();

    oslGenericFunction getFunctionSymbolByName(
            const OUString& localeName, const char* pFunction,
            std::optional<LocaleDataLookupTableItem>& rOutCachedItem );

private:
    std::mutex maMutex;
    ::std::vector< LocaleDataLookupTableItem >  maLookupTable;
};

// from instance.hxx: Helper base class for a late-initialized
// (default-constructed) static variable, implementing the double-checked
// locking pattern correctly.
// usage:  lcl_LookupTableHelper & rLookupTable = lcl_LookupTableStatic::get();
// retrieves the singleton lookup table instance
lcl_LookupTableHelper& lcl_LookupTableStatic()
{
    static lcl_LookupTableHelper SINGLETON;
    return SINGLETON;
}

lcl_LookupTableHelper::lcl_LookupTableHelper()
{
}

lcl_LookupTableHelper::~lcl_LookupTableHelper()
{
    for ( const LocaleDataLookupTableItem& item : maLookupTable ) {
        delete item.module;
    }
}

oslGenericFunction lcl_LookupTableHelper::getFunctionSymbolByName(
        const OUString& localeName, const char* pFunction,
        std::optional<LocaleDataLookupTableItem>& rOutCachedItem )
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
            aFallback = LocaleDataImpl::getFirstLocaleServiceName( aFbLocale);
    }

    for (const auto & i : aLibTable)
    {
        if (localeName.equalsAscii(i.pLocale) ||
                (bFallback && aFallback.equalsAscii(i.pLocale)))
        {
#ifndef DISABLE_DYNLOADING
            {
                std::unique_lock aGuard( maMutex );
                for (LocaleDataLookupTableItem & rCurrent : maLookupTable)
                {
                    if (rCurrent.dllName == i.pLib)
                    {
                        rOutCachedItem.emplace( rCurrent );
                        rOutCachedItem->localeName = i.pLocale;
                        OString sSymbolName = OString::Concat(pFunction) + "_" +
                                rOutCachedItem->localeName;
                        return rOutCachedItem->module->getFunctionSymbol(
                                sSymbolName.getStr());
                    }
                }
            }
            // Library not loaded, load it and add it to the list.
            std::unique_ptr<osl::Module> module(new osl::Module());
            if ( module->loadRelative(&thisModule, getLibraryName(i.pLib)) )
            {
                std::unique_lock aGuard( maMutex );
                auto pTmpModule = module.get();
                maLookupTable.emplace_back(i.pLib, module.release(), i.pLocale);
                rOutCachedItem.emplace( maLookupTable.back() );
                OString sSymbolName = OString::Concat(pFunction) + "_" + rOutCachedItem->localeName;
                return pTmpModule->getFunctionSymbol(sSymbolName.getStr());
            }
            else
                module.reset();
#else
            (void) rOutCachedItem;

            if( strcmp(pFunction, "getAllCalendars") == 0 )
                return i.getAllCalendars;
            else if( strcmp(pFunction, "getAllCurrencies") == 0 )
                return i.getAllCurrencies;
            else if( strcmp(pFunction, "getAllFormats0") == 0 )
                return i.getAllFormats0;
            else if( strcmp(pFunction, "getBreakIteratorRules") == 0 )
                return i.getBreakIteratorRules;
            else if( strcmp(pFunction, "getCollationOptions") == 0 )
                return i.getCollationOptions;
            else if( strcmp(pFunction, "getCollatorImplementation") == 0 )
                return i.getCollatorImplementation;
            else if( strcmp(pFunction, "getContinuousNumberingLevels") == 0 )
                return i.getContinuousNumberingLevels;
            else if( strcmp(pFunction, "getDateAcceptancePatterns") == 0 )
                return i.getDateAcceptancePatterns;
            else if( strcmp(pFunction, "getFollowPageWords") == 0 )
                return i.getFollowPageWords;
            else if( strcmp(pFunction, "getForbiddenCharacters") == 0 )
                return i.getForbiddenCharacters;
            else if( strcmp(pFunction, "getIndexAlgorithm") == 0 )
                return i.getIndexAlgorithm;
            else if( strcmp(pFunction, "getLCInfo") == 0 )
                return i.getLCInfo;
            else if( strcmp(pFunction, "getLocaleItem") == 0 )
                return i.getLocaleItem;
            else if( strcmp(pFunction, "getOutlineNumberingLevels") == 0 )
                return i.getOutlineNumberingLevels;
            else if( strcmp(pFunction, "getReservedWords") == 0 )
                return i.getReservedWords;
            else if( strcmp(pFunction, "getSearchOptions") == 0 )
                return i.getSearchOptions;
            else if( strcmp(pFunction, "getTransliterations") == 0 )
                return i.getTransliterations;
            else if( strcmp(pFunction, "getUnicodeScripts") == 0 )
                return i.getUnicodeScripts;
            else if( strcmp(pFunction, "getAllFormats1") == 0 )
                return i.getAllFormats1;
#endif
        }
    }
    return nullptr;
}

} // anonymous namespace


// REF values equal offsets of counts within getAllCalendars() data structure!
#define REF_DAYS         0
#define REF_MONTHS       1
#define REF_GMONTHS      2
#define REF_PMONTHS      3
#define REF_ERAS         4
#define REF_OFFSET_COUNT 5

Sequence< CalendarItem2 > &LocaleDataImpl::getCalendarItemByName(const OUString& name,
        const Locale& rLocale, const Sequence< Calendar2 >& calendarsSeq, sal_Int16 item)
{
    if (ref_name != name) {
        OUString aLocStr, id;
        sal_Int32 nLastUnder = name.lastIndexOf( cUnder);
        SAL_WARN_IF( nLastUnder < 1, "i18npool",
                "LocaleDataImpl::getCalendarItemByName - no '_' or first in name can't be right: " << name);
        if (nLastUnder >= 0)
        {
            aLocStr = name.copy( 0, nLastUnder);
            if (nLastUnder + 1 < name.getLength())
                id = name.copy( nLastUnder + 1);
        }
        Locale loc( LanguageTag::convertToLocale( aLocStr.replace( cUnder, cHyphen)));
        Sequence < Calendar2 > cals;
        if (loc == rLocale) {
            cals = calendarsSeq;
        } else {
            cals = getAllCalendars2(loc);
        }
        auto pCal = std::find_if(std::cbegin(cals), std::cend(cals),
            [&id](const Calendar2& rCal) { return id == rCal.Name; });
        if (pCal != std::cend(cals))
            ref_cal = *pCal;
        else {
            // Referred locale not found, return name for en_US locale.
            cals = getAllCalendars2( Locale(u"en"_ustr, u"US"_ustr, OUString()) );
            if (!cals.hasElements())
                throw RuntimeException();
            ref_cal = cals.getConstArray()[0];
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
            OSL_FAIL( "LocaleDataImpl::getCalendarItemByName: unhandled REF_* case");
            [[fallthrough]];
        case REF_ERAS:
            return ref_cal.Eras;
    }
}

Sequence< CalendarItem2 > LocaleDataImpl::getCalendarItems(
        OUString const * allCalendars, sal_Int16 & rnOffset,
        const sal_Int16 nWhichItem, const sal_Int16 nCalendar,
        const Locale & rLocale, const Sequence< Calendar2 > & calendarsSeq )
{
    Sequence< CalendarItem2 > aItems;
    if ( allCalendars[rnOffset] == std::u16string_view(u"ref") )
    {
        aItems = getCalendarItemByName( allCalendars[rnOffset+1], rLocale, calendarsSeq, nWhichItem);
        rnOffset += 2;
    }
    else
    {
        const sal_Int32 nSize = allCalendars[nWhichItem][nCalendar];
        aItems.realloc( nSize);
        switch (nWhichItem)
        {
            case REF_DAYS:
            case REF_MONTHS:
            case REF_GMONTHS:
            case REF_PMONTHS:
                for (CalendarItem2& rItem : asNonConstRange(aItems))
                {
                    rItem = CalendarItem2{ allCalendars[rnOffset],
                            allCalendars[rnOffset+1],
                            allCalendars[rnOffset+2], allCalendars[rnOffset+3]};
                    rnOffset += 4;
                }
                break;
            case REF_ERAS:
                // Absent narrow name.
                for (CalendarItem2& rItem : asNonConstRange(aItems))
                {
                    rItem = CalendarItem2{ allCalendars[rnOffset],
                            allCalendars[rnOffset+1],
                            allCalendars[rnOffset+2], OUString()};
                    rnOffset += 3;
                }
                break;
            default:
                OSL_FAIL( "LocaleDataImpl::getCalendarItems: unhandled REF_* case");
        }
    }
    return aItems;
}

Sequence< Calendar2 > SAL_CALL
LocaleDataImpl::getAllCalendars2( const Locale& rLocale )
{

    OUString const * allCalendars = nullptr;

    MyFuncOUString_Type func = reinterpret_cast<MyFuncOUString_Type>(getFunctionSymbol( rLocale, "getAllCalendars" ));

    if ( func ) {
        sal_Int16 calendarsCount = 0;
        allCalendars = func(calendarsCount);

        Sequence< Calendar2 > calendarsSeq(calendarsCount);
        auto calendarsSeqRange = asNonConstRange(calendarsSeq);
        sal_Int16 offset = REF_OFFSET_COUNT;
        for(sal_Int16 i = 0; i < calendarsCount; i++) {
            OUString calendarID(allCalendars[offset]);
            offset++;
            bool defaultCalendar = allCalendars[offset][0] != 0;
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
            calendarsSeqRange[i] = Calendar2(days, months, gmonths, pmonths, eras, startOfWeekDay,
                    minimalDaysInFirstWeek, defaultCalendar, calendarID);
        }
        return calendarsSeq;
    }
    else {
        return {};
    }
}


Sequence< Calendar > SAL_CALL
LocaleDataImpl::getAllCalendars( const Locale& rLocale )
{
    const Sequence< Calendar2 > aCal2( getAllCalendars2( rLocale));
    std::vector<Calendar> aCal1;
    aCal1.reserve(aCal2.getLength());
    std::transform(aCal2.begin(), aCal2.end(), std::back_inserter(aCal1),
        [](const Calendar2& rCal2) { return downcastCalendar(rCal2); });
    return comphelper::containerToSequence(aCal1);
}


Sequence< Currency2 > SAL_CALL
LocaleDataImpl::getAllCurrencies2( const Locale& rLocale )
{
    MyFuncOUString_Type func = reinterpret_cast<MyFuncOUString_Type>(getFunctionSymbol( rLocale, "getAllCurrencies" ));

    if ( func ) {
        sal_Int16 currencyCount = 0;
        OUString const *allCurrencies = func(currencyCount);

        Sequence< Currency2 > seq(currencyCount);
        auto seqRange = asNonConstRange(seq);
        for(int i = 0, nOff = 0; i < currencyCount; i++, nOff += 8 ) {
            seqRange[i] = Currency2(
                    allCurrencies[nOff], // string ID
                    allCurrencies[nOff+1], // string Symbol
                    allCurrencies[nOff+2], // string BankSymbol
                    allCurrencies[nOff+3], // string Name
                    allCurrencies[nOff+4][0] != 0, // boolean Default
                    allCurrencies[nOff+5][0] != 0, // boolean UsedInCompatibleFormatCodes
                    allCurrencies[nOff+6][0],   // short DecimalPlaces
                    allCurrencies[nOff+7][0] != 0 // boolean LegacyOnly
                    );
        }
        return seq;
    }
    else {
        return {};
    }
}


Sequence< Currency > SAL_CALL
LocaleDataImpl::getAllCurrencies( const Locale& rLocale )
{
    return comphelper::containerToSequence<Currency>(getAllCurrencies2(rLocale));
}


Sequence< FormatElement > SAL_CALL
LocaleDataImpl::getAllFormats( const Locale& rLocale )
{
    const int SECTIONS = 2;
    struct FormatSection
    {
        MyFunc_FormatCode         func;
        sal_Unicode const        *from;
        sal_Unicode const        *to;
        OUString const           *formatArray;
        sal_Int16                 formatCount;

        FormatSection() : func(nullptr), from(nullptr), to(nullptr), formatArray(nullptr), formatCount(0) {}
        sal_Int16 getFunc( LocaleDataImpl& rLocaleData, const Locale& rL, const char* pName )
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
    auto seqRange = asNonConstRange(seq);
    sal_Int32 f = 0;
    for (const FormatSection & s : section)
    {
        OUString const * const formatArray = s.formatArray;
        if ( formatArray )
        {
            for (int i = 0, nOff = 0; i < s.formatCount; ++i, nOff += 7, ++f)
            {
                seqRange[f] = FormatElement(
                        formatArray[nOff].replaceAll(s.from, s.to),
                        formatArray[nOff + 1],
                        formatArray[nOff + 2],
                        formatArray[nOff + 3],
                        formatArray[nOff + 4],
                        formatArray[nOff + 5][0],
                        formatArray[nOff + 6][0] != 0);
            }
        }
    }
    return seq;
}


Sequence< OUString > SAL_CALL
LocaleDataImpl::getDateAcceptancePatterns( const Locale& rLocale )
{
    MyFuncOUString_Type func = reinterpret_cast<MyFuncOUString_Type>(getFunctionSymbol( rLocale, "getDateAcceptancePatterns" ));

    if (func)
    {
        sal_Int16 patternsCount = 0;
        OUString const *patternsArray = func( patternsCount );
        Sequence< OUString > seq( patternsCount );
        auto seqRange = asNonConstRange(seq);
        for (sal_Int16 i = 0; i < patternsCount; ++i)
        {
            seqRange[i] = patternsArray[i];
        }
        return seq;
    }
    else
    {
        return {};
    }
}


#define COLLATOR_OFFSET_ALGO    0
#define COLLATOR_OFFSET_DEFAULT 1
#define COLLATOR_OFFSET_RULE    2
#define COLLATOR_ELEMENTS       3

OUString
LocaleDataImpl::getCollatorRuleByAlgorithm( const Locale& rLocale, std::u16string_view algorithm )
{
    MyFuncOUString_Type func = reinterpret_cast<MyFuncOUString_Type>(getFunctionSymbol( rLocale, "getCollatorImplementation" ));
    if ( func ) {
        sal_Int16 collatorCount = 0;
        OUString const *collatorArray = func(collatorCount);
        for(sal_Int16 i = 0; i < collatorCount; i++)
            if (algorithm == collatorArray[i * COLLATOR_ELEMENTS + COLLATOR_OFFSET_ALGO])
                return collatorArray[i * COLLATOR_ELEMENTS + COLLATOR_OFFSET_RULE];
    }
    return OUString();
}


Sequence< Implementation > SAL_CALL
LocaleDataImpl::getCollatorImplementations( const Locale& rLocale )
{
    MyFuncOUString_Type func = reinterpret_cast<MyFuncOUString_Type>(getFunctionSymbol( rLocale, "getCollatorImplementation" ));

    if ( func ) {
        sal_Int16 collatorCount = 0;
        OUString const *collatorArray = func(collatorCount);
        Sequence< Implementation > seq(collatorCount);
        auto seqRange = asNonConstRange(seq);
        for(sal_Int16 i = 0; i < collatorCount; i++) {
            seqRange[i] = Implementation(
                    collatorArray[i * COLLATOR_ELEMENTS + COLLATOR_OFFSET_ALGO],
                    collatorArray[i * COLLATOR_ELEMENTS + COLLATOR_OFFSET_DEFAULT][0] != 0);
        }
        return seq;
    }
    else {
        return {};
    }
}

Sequence< OUString > SAL_CALL
LocaleDataImpl::getCollationOptions( const Locale& rLocale )
{
    MyFuncOUString_Type func = reinterpret_cast<MyFuncOUString_Type>(getFunctionSymbol( rLocale, "getCollationOptions" ));

    if ( func ) {
        sal_Int16 optionsCount = 0;
        OUString const *optionsArray = func(optionsCount);
        Sequence< OUString > seq(optionsCount);
        auto seqRange = asNonConstRange(seq);
        for(sal_Int16 i = 0; i < optionsCount; i++) {
            seqRange[i] = optionsArray[i];
        }
        return seq;
    }
    else {
        return {};
    }
}

Sequence< OUString > SAL_CALL
LocaleDataImpl::getSearchOptions( const Locale& rLocale )
{
    MyFuncOUString_Type func = reinterpret_cast<MyFuncOUString_Type>(getFunctionSymbol( rLocale, "getSearchOptions" ));

    if ( func ) {
        sal_Int16 optionsCount = 0;
        OUString const *optionsArray = func(optionsCount);
        return Sequence< OUString >(optionsArray, optionsCount);
    }
    else {
        return {};
    }
}

OUString const *
LocaleDataImpl::getIndexArray(const Locale& rLocale, sal_Int16& indexCount)
{
    MyFuncOUString_Type func = reinterpret_cast<MyFuncOUString_Type>(getFunctionSymbol( rLocale, "getIndexAlgorithm" ));

    if (func)
        return func(indexCount);
    return nullptr;
}

Sequence< OUString >
LocaleDataImpl::getIndexAlgorithm( const Locale& rLocale )
{
    sal_Int16 indexCount = 0;
    OUString const *indexArray = getIndexArray(rLocale, indexCount);

    if ( indexArray ) {
        Sequence< OUString > seq(indexCount);
        auto seqRange = asNonConstRange(seq);
        for(sal_Int16 i = 0; i < indexCount; i++) {
            seqRange[i] = indexArray[i*5];
        }
        return seq;
    }
    else {
        return {};
    }
}

OUString
LocaleDataImpl::getDefaultIndexAlgorithm( const Locale& rLocale )
{
    sal_Int16 indexCount = 0;
    OUString const *indexArray = getIndexArray(rLocale, indexCount);

    if ( indexArray ) {
        for(sal_Int16 i = 0; i < indexCount; i++) {
            if (indexArray[i*5 + 3][0])
                return indexArray[i*5];
        }
    }
    return OUString();
}

bool
LocaleDataImpl::hasPhonetic( const Locale& rLocale )
{
    sal_Int16 indexCount = 0;
    OUString const *indexArray = getIndexArray(rLocale, indexCount);

    if ( indexArray ) {
        for(sal_Int16 i = 0; i < indexCount; i++) {
            if (indexArray[i*5 + 4][0])
                return true;
        }
    }
    return false;
}

OUString const *
LocaleDataImpl::getIndexArrayForAlgorithm(const Locale& rLocale, std::u16string_view algorithm)
{
    sal_Int16 indexCount = 0;
    OUString const *indexArray = getIndexArray(rLocale, indexCount);
    if ( indexArray ) {
        for(sal_Int16 i = 0; i < indexCount; i++) {
            if (algorithm == indexArray[i*5])
                return indexArray+i*5;
        }
    }
    return nullptr;
}

bool
LocaleDataImpl::isPhonetic( const Locale& rLocale, std::u16string_view algorithm )
{
    OUString const *indexArray = getIndexArrayForAlgorithm(rLocale, algorithm);
    return indexArray && indexArray[4][0];
}

OUString
LocaleDataImpl::getIndexKeysByAlgorithm( const Locale& rLocale, std::u16string_view algorithm )
{
    OUString const *indexArray = getIndexArrayForAlgorithm(rLocale, algorithm);
    return indexArray ? (OUString::Concat(u"0-9") + indexArray[2]) : OUString();
}

OUString
LocaleDataImpl::getIndexModuleByAlgorithm( const Locale& rLocale, std::u16string_view algorithm )
{
    OUString const *indexArray = getIndexArrayForAlgorithm(rLocale, algorithm);
    return indexArray ? indexArray[1] : OUString();
}

Sequence< UnicodeScript >
LocaleDataImpl::getUnicodeScripts( const Locale& rLocale )
{
    MyFuncOUString_Type func = reinterpret_cast<MyFuncOUString_Type>(getFunctionSymbol( rLocale, "getUnicodeScripts" ));

    if ( func ) {
        sal_Int16 scriptCount = 0;
        OUString const *scriptArray = func(scriptCount);
        Sequence< UnicodeScript > seq(scriptCount);
        auto seqRange = asNonConstRange(seq);
        for(sal_Int16 i = 0; i < scriptCount; i++) {
            seqRange[i] = UnicodeScript( o3tl::toInt32(scriptArray[i].subView(0, 1)) );
        }
        return seq;
    }
    else {
        return {};
    }
}

Sequence< OUString >
LocaleDataImpl::getFollowPageWords( const Locale& rLocale )
{
    MyFuncOUString_Type func = reinterpret_cast<MyFuncOUString_Type>(getFunctionSymbol( rLocale, "getFollowPageWords" ));

    if ( func ) {
        sal_Int16 wordCount = 0;
        OUString const *wordArray = func(wordCount);
        return Sequence< OUString >(wordArray, wordCount);
    }
    else {
        return {};
    }
}

Sequence< OUString > SAL_CALL
LocaleDataImpl::getTransliterations( const Locale& rLocale )
{
    MyFuncOUString_Type func = reinterpret_cast<MyFuncOUString_Type>(getFunctionSymbol( rLocale, "getTransliterations" ));

    if ( func ) {
        sal_Int16 transliterationsCount = 0;
        const OUString *transliterationsArray = func(transliterationsCount);
        return Sequence< OUString >(transliterationsArray, transliterationsCount);
    }
    else {
        return {};
    }


}


LanguageCountryInfo SAL_CALL
LocaleDataImpl::getLanguageCountryInfo( const Locale& rLocale )
{
    MyFuncOUString_Type func = reinterpret_cast<MyFuncOUString_Type>(getFunctionSymbol( rLocale, "getLCInfo" ));

    if ( func ) {
        sal_Int16 LCInfoCount = 0;
        OUString const *LCInfoArray = func(LCInfoCount);
        LanguageCountryInfo info{LCInfoArray[0],
                LCInfoArray[1],
                LCInfoArray[2],
                LCInfoArray[3],
                LCInfoArray[4]};
        return info;
    }
    else {
        LanguageCountryInfo info1;
        return info1;
    }

}


ForbiddenCharacters SAL_CALL
LocaleDataImpl::getForbiddenCharacters( const Locale& rLocale )
{
    MyFuncOUString_Type func = reinterpret_cast<MyFuncOUString_Type>(getFunctionSymbol( rLocale, "getForbiddenCharacters" ));

    if ( func ) {
        sal_Int16 LCForbiddenCharactersCount = 0;
        OUString const *LCForbiddenCharactersArray = func(LCForbiddenCharactersCount);
        assert(LCForbiddenCharactersCount == 3);
        ForbiddenCharacters chars{
            LCForbiddenCharactersArray[0], LCForbiddenCharactersArray[1]};
        return chars;
    }
    else {
        ForbiddenCharacters chars1;
        return chars1;
    }
}

OUString
LocaleDataImpl::getHangingCharacters( const Locale& rLocale )
{
    MyFuncOUString_Type func = reinterpret_cast<MyFuncOUString_Type>(getFunctionSymbol( rLocale, "getForbiddenCharacters" ));

    if ( func ) {
        sal_Int16 LCForbiddenCharactersCount = 0;
        const OUString *LCForbiddenCharactersArray = func(LCForbiddenCharactersCount);
        assert(LCForbiddenCharactersCount == 3);
        return LCForbiddenCharactersArray[2];
    }

    return OUString();
}

Sequence< OUString >
LocaleDataImpl::getBreakIteratorRules( const Locale& rLocale  )
{
    MyFuncOUString_Type func = reinterpret_cast<MyFuncOUString_Type>(getFunctionSymbol( rLocale, "getBreakIteratorRules" ));

    if ( func ) {
        sal_Int16 LCBreakIteratorRuleCount = 0;
        OUString const *LCBreakIteratorRulesArray = func(LCBreakIteratorRuleCount);
        return Sequence< OUString >(LCBreakIteratorRulesArray, LCBreakIteratorRuleCount);
    }
    else {
        return {};
    }
}


Sequence< OUString > SAL_CALL
LocaleDataImpl::getReservedWord( const Locale& rLocale  )
{
    MyFuncOUString_Type func = reinterpret_cast<MyFuncOUString_Type>(getFunctionSymbol( rLocale, "getReservedWords" ));

    if ( func ) {
        sal_Int16 LCReservedWordsCount = 0;
        OUString const *LCReservedWordsArray = func(LCReservedWordsCount);
        return Sequence< OUString >(LCReservedWordsArray, LCReservedWordsCount);
    }
    else {
        return {};
    }
}


Sequence< Sequence<beans::PropertyValue> >
LocaleDataImpl::getContinuousNumberingLevels( const lang::Locale& rLocale )
{
    // load symbol
    MyFunc_Type2 func = reinterpret_cast<MyFunc_Type2>(getFunctionSymbol( rLocale, "getContinuousNumberingLevels" ));

    if ( func )
    {
        // invoke function
        sal_Int16 nStyles;
        sal_Int16 nAttributes;
        OUString const ** p0 = func( nStyles, nAttributes );

        // allocate memory for nAttributes attributes for each of the nStyles styles.
        Sequence< Sequence<beans::PropertyValue> > pv( nStyles );
        auto pvRange = asNonConstRange(pv);
        for( auto& i : pvRange ) {
            i = Sequence<beans::PropertyValue>( nAttributes );
        }

        OUString const ** pStyle = p0;
        for( int i=0;  i<nStyles;  i++ ) {
            OUString const * pAttribute = pStyle[i];
            auto pvElementRange = asNonConstRange(pvRange[i]);
            for( int j=0;  j<nAttributes;  j++ ) { // prefix, numberingtype, ...
                OUString const & pString = pAttribute[j];
                beans::PropertyValue& rVal = pvElementRange[j];
                OUString sVal;
                if( 0 != j && 2 != j )
                    sVal = pString;
                else if( !pString.isEmpty() )
                    sVal = pString.copy( 0, 1 );

                switch( j )
                {
                    case 0:
                        rVal.Name = "Prefix";
                        rVal.Value <<= sVal;
                        break;
                    case 1:
                        rVal.Name = "NumberingType";
                        rVal.Value <<= static_cast<sal_Int16>(sVal.toInt32());
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
                        rVal.Value <<= static_cast<sal_Int16>(sVal.toInt32());
                        break;
                    default:
                        OSL_ASSERT(false);
                }
            }
        }
        return pv;
    }

    return Sequence< Sequence<beans::PropertyValue> >();
}

// OutlineNumbering helper class

namespace {

struct OutlineNumberingLevel_Impl
{
    OUString        sPrefix;
    sal_Int16       nNumType; //css::style::NumberingType
    OUString        sSuffix;
    sal_Unicode     cBulletChar;
    OUString        sBulletFontName;
    sal_Int16       nParentNumbering;
    sal_Int32       nLeftMargin;
    sal_Int32       nSymbolTextDistance;
    sal_Int32       nFirstLineOffset;
    sal_Int16       nAdjust;
    OUString        sTransliteration;
    sal_Int32       nNatNum;
};

class OutlineNumbering : public cppu::WeakImplHelper < container::XIndexAccess >
{
    // OutlineNumbering helper class

    std::unique_ptr<const OutlineNumberingLevel_Impl[]> m_pOutlineLevels;
    sal_Int16                         m_nCount;
public:
    OutlineNumbering(std::unique_ptr<const OutlineNumberingLevel_Impl[]> pOutlineLevels, int nLevels);

    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) override;
    virtual Any SAL_CALL getByIndex( sal_Int32 Index ) override;

    //XElementAccess
    virtual Type SAL_CALL getElementType(  ) override;
    virtual sal_Bool SAL_CALL hasElements(  ) override;
};

}

Sequence< Reference<container::XIndexAccess> >
LocaleDataImpl::getOutlineNumberingLevels( const lang::Locale& rLocale )
{
    // load symbol
    MyFunc_Type3 func = reinterpret_cast<MyFunc_Type3>(getFunctionSymbol( rLocale, "getOutlineNumberingLevels" ));

    if ( func )
    {
        int i;
        // invoke function
        sal_Int16 nStyles;
        sal_Int16 nLevels;
        sal_Int16 nAttributes;
        OUString const *** p0 = func( nStyles, nLevels, nAttributes );

        Sequence< Reference<container::XIndexAccess> > aRet( nStyles );
        auto aRetRange = asNonConstRange(aRet);
        OUString const *** pStyle = p0;
        for( i=0;  i<nStyles;  i++ )
        {
            int j;

            std::unique_ptr<OutlineNumberingLevel_Impl[]> level(new OutlineNumberingLevel_Impl[ nLevels+1 ]);
            OUString const ** pLevel = pStyle[i];
            for( j = 0;  j < nLevels;  j++ )
            {
                OUString const * pAttribute = pLevel[j];
                for( int k=0; k<nAttributes; k++ )
                {
                    OUString tmp( pAttribute[k] );
                    switch( k )
                    {
                        case 0: level[j].sPrefix             = tmp;             break;
                        case 1: level[j].nNumType            = sal::static_int_cast<sal_Int16>(tmp.toInt32()); break;
                        case 2: level[j].sSuffix             = tmp;             break;
                        case 3: level[j].cBulletChar         = sal::static_int_cast<sal_Unicode>(tmp.toUInt32(16)); break; // base 16
                        case 4: level[j].sBulletFontName     = tmp;             break;
                        case 5: level[j].nParentNumbering    = sal::static_int_cast<sal_Int16>(tmp.toInt32()); break;
                        case 6: level[j].nLeftMargin         = tmp.toInt32();   break;
                        case 7: level[j].nSymbolTextDistance = tmp.toInt32();   break;
                        case 8: level[j].nFirstLineOffset    = tmp.toInt32();   break;
                        case 9: level[j].nAdjust             = sal::static_int_cast<sal_Int16>(tmp.toInt32()); break;
                        case 10: level[j].sTransliteration = tmp; break;
                        case 11: level[j].nNatNum    = tmp.toInt32();   break;
                        default:
                                 OSL_ASSERT(false);
                    }
                }
            }
            level[j].sPrefix.clear();
            level[j].nNumType            = 0;
            level[j].sSuffix.clear();
            level[j].nAdjust = 0;
            level[j].cBulletChar         = 0;
            level[j].sBulletFontName.clear();
            level[j].nParentNumbering    = 0;
            level[j].nLeftMargin         = 0;
            level[j].nSymbolTextDistance = 0;
            level[j].nFirstLineOffset    = 0;
            level[j].sTransliteration.clear();
            level[j].nNatNum             = 0;
            aRetRange[i] = new OutlineNumbering( std::move(level), nLevels );
        }
        return aRet;
    }
    else {
        return {};
    }
}

// helper functions

oslGenericFunction LocaleDataImpl::getFunctionSymbol( const Locale& rLocale, const char* pFunction )
{
    lcl_LookupTableHelper & rLookupTable = lcl_LookupTableStatic();

    if (moCachedItem && moCachedItem->equals(rLocale))
    {
        OString sSymbolName = OString::Concat(pFunction) + "_" +
                moCachedItem->localeName;
        return moCachedItem->module->getFunctionSymbol(sSymbolName.getStr());
    }

    oslGenericFunction pSymbol = nullptr;
    std::optional<LocaleDataLookupTableItem> oCachedItem;

    // Load function with name <func>_<lang>_<country> or <func>_<bcp47> and
    // fallbacks.
    pSymbol = rLookupTable.getFunctionSymbolByName( LocaleDataImpl::getFirstLocaleServiceName( rLocale),
            pFunction, oCachedItem);
    if (!pSymbol)
    {
        ::std::vector< OUString > aFallbacks( LocaleDataImpl::getFallbackLocaleServiceNames( rLocale));
        for (const auto& rFallback : aFallbacks)
        {
            pSymbol = rLookupTable.getFunctionSymbolByName(rFallback, pFunction, oCachedItem);
            if (pSymbol)
                break;
        }
    }
    if (!pSymbol)
    {
        // load default function with name <func>_en_US
        pSymbol = rLookupTable.getFunctionSymbolByName(u"en_US"_ustr, pFunction, oCachedItem);
    }

    if (!pSymbol)
        // Appropriate symbol could not be found.  Give up.
        throw RuntimeException();

    if (oCachedItem)
        moCachedItem = std::move(oCachedItem);
    if (moCachedItem)
        moCachedItem->aLocale = rLocale;

    return pSymbol;
}

Sequence< Locale > SAL_CALL
LocaleDataImpl::getAllInstalledLocaleNames()
{
    Sequence< lang::Locale > seq( nbOfLocales );
    auto seqRange = asNonConstRange(seq);
    sal_Int16 nInstalled = 0;

    for(const auto & i : aLibTable) {
        OUString name = OUString::createFromAscii( i.pLocale );

        // Check if the locale is really available and not just in the table,
        // don't allow fall backs.
        std::optional<LocaleDataLookupTableItem> oCachedItem;
        if (lcl_LookupTableStatic().getFunctionSymbolByName( name, "getLocaleItem", oCachedItem )) {
            if( oCachedItem )
                moCachedItem = std::move( oCachedItem );
            seqRange[nInstalled++] = LanguageTag::convertToLocale( name.replace( cUnder, cHyphen), false);
        }
    }
    if ( nInstalled < nbOfLocales )
        seq.realloc( nInstalled );          // reflect reality

    return seq;
}

using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;

OutlineNumbering::OutlineNumbering(std::unique_ptr<const OutlineNumberingLevel_Impl[]> pOutlnLevels, int nLevels) :
    m_pOutlineLevels(std::move(pOutlnLevels)),
    m_nCount(sal::static_int_cast<sal_Int16>(nLevels))
{
}

sal_Int32 OutlineNumbering::getCount(  )
{
    return m_nCount;
}

Any OutlineNumbering::getByIndex( sal_Int32 nIndex )
{
    if(nIndex < 0 || nIndex >= m_nCount)
        throw IndexOutOfBoundsException();
    const OutlineNumberingLevel_Impl* pTemp = m_pOutlineLevels.get();
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
    pValues[4].Value <<= pTemp->sBulletFontName;
    pValues[5].Name = "ParentNumbering";
    pValues[5].Value <<= pTemp->nParentNumbering;
    pValues[6].Name = "LeftMargin";
    pValues[6].Value <<= pTemp->nLeftMargin;
    pValues[7].Name = "SymbolTextDistance";
    pValues[7].Value <<= pTemp->nSymbolTextDistance;
    pValues[8].Name = "FirstLineOffset";
    pValues[8].Value <<= pTemp->nFirstLineOffset;
    pValues[9].Name = "Adjust";
    pValues[9].Value <<= pTemp->nAdjust;
    pValues[10].Name = "Transliteration";
    pValues[10].Value <<= pTemp->sTransliteration;
    pValues[11].Name = "NatNum";
    pValues[11].Value <<= pTemp->nNatNum;
    aRet <<= aOutlineNumbering;
    return aRet;
}

Type OutlineNumbering::getElementType(  )
{
    return cppu::UnoType<Sequence<PropertyValue>>::get();
}

sal_Bool OutlineNumbering::hasElements(  )
{
    return m_nCount > 0;
}

OUString SAL_CALL
LocaleDataImpl::getImplementationName()
{
    return u"com.sun.star.i18n.LocaleDataImpl"_ustr;
}

sal_Bool SAL_CALL LocaleDataImpl::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SAL_CALL
LocaleDataImpl::getSupportedServiceNames()
{
    Sequence< OUString > aRet {
        u"com.sun.star.i18n.LocaleData"_ustr,
        u"com.sun.star.i18n.LocaleData2"_ustr
    };
    return aRet;
}

// static
OUString LocaleDataImpl::getFirstLocaleServiceName( const css::lang::Locale & rLocale )
{
    if (rLocale.Language == I18NLANGTAG_QLT)
        return rLocale.Variant.replace( cHyphen, cUnder);
    else if (!rLocale.Country.isEmpty())
        return rLocale.Language + "_" + rLocale.Country;
    else
        return rLocale.Language;
}

// static
::std::vector< OUString > LocaleDataImpl::getFallbackLocaleServiceNames( const css::lang::Locale & rLocale )
{
    ::std::vector< OUString > aVec;
    if (rLocale.Language == I18NLANGTAG_QLT)
    {
        aVec = LanguageTag( rLocale).getFallbackStrings( false);
        for (auto& rItem : aVec)
        {
            rItem = rItem.replace(cHyphen, cUnder);
        }
    }
    else if (!rLocale.Country.isEmpty())
    {
        aVec.push_back( rLocale.Language);
    }
    // else nothing, language-only was the first
    return aVec;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_i18n_LocaleDataImpl_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new i18npool::LocaleDataImpl());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
