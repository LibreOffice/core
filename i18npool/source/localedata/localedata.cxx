/*************************************************************************
 *
 *  $RCSfile: localedata.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: bustamam $ $Date: 2001-09-16 15:22:59 $
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



typedef sal_Unicode ** (SAL_CALL * MyFunc_Type)( sal_Int16&);
typedef sal_Unicode  *** (SAL_CALL * MyFunc_Type2)( sal_Int16&, sal_Int16& );
typedef sal_Unicode **** (SAL_CALL * MyFunc_Type3)( sal_Int16&, sal_Int16&, sal_Int16& );


static const ::rtl::OUString
    lcl_ASCII( ::rtl::OUString::createFromAscii("localedata_ascii" )),
    lcl_CJK( ::rtl::OUString::createFromAscii("localedata_CJK" )),
    lcl_CTL( ::rtl::OUString::createFromAscii("localedata_ctl" ));

const TableElement
LocaleData::dllsTable[] = {
    TableElement(::rtl::OUString::createFromAscii("en_US"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("de_DE"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("fr_FR"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("sv_SE"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("es_ES"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("it_IT"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("af_ZA"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("be_BY"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("bg_BG"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("br_AE"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("ca_ES"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("cs_CZ"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("da_DK"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("de_AT"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("de_CH"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("de_LI"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("de_LU"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("el_GR"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("en_AU"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("en_BZ"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("en_CA"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("en_GB"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("en_IE"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("en_JM"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("en_NZ"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("en_PH"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("en_TT"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("en_ZA"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("en_ZW"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("es_AR"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("es_BO"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("es_CL"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("es_CO"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("es_CR"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("es_DO"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("es_EC"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("es_GT"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("es_HN"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("es_MX"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("es_NI"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("es_PA"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("es_PE"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("es_PR"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("es_PY"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("es_SV"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("es_UY"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("es_VE"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("fi_FI"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("fr_BE"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("fr_CA"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("fr_CH"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("fr_LU"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("fr_MC"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("hu_HU"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("id_ID"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("is_IS"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("it_CH"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("nb_NO"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("nl_BE"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("nl_NL"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("no_NO"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("nn_NO"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("pl_PL"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("pt_BR"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("pt_PT"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("ru_RU"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("sv_FI"), lcl_ASCII),
    TableElement(::rtl::OUString::createFromAscii("tr_TR"), lcl_ASCII),

    TableElement(::rtl::OUString::createFromAscii("ja_JP"), lcl_CJK),
    TableElement(::rtl::OUString::createFromAscii("ko_KR"), lcl_CJK),
    TableElement(::rtl::OUString::createFromAscii("zh_CN"), lcl_CJK),
    TableElement(::rtl::OUString::createFromAscii("zh_HK"), lcl_CJK),
    TableElement(::rtl::OUString::createFromAscii("zh_SG"), lcl_CJK),
    TableElement(::rtl::OUString::createFromAscii("zh_TW"), lcl_CJK),
    TableElement(::rtl::OUString::createFromAscii("zh_MO"), lcl_CJK),

    TableElement(::rtl::OUString::createFromAscii("ar"), lcl_CTL),
    TableElement(::rtl::OUString::createFromAscii("th"), lcl_CTL),
    TableElement(::rtl::OUString::createFromAscii("he"), lcl_CTL),

};

const sal_Int16
LocaleData::nbOfLocales = sizeof(dllsTable) / sizeof(TableElement);



LocaleData::~LocaleData(){
    lookupTableItem *listItem = (lookupTableItem*)lookupTable.First();
    while ( listItem )
    {
        if ( listItem->dllHandle )
            osl_unloadModule(listItem->dllHandle);
        delete listItem;
        listItem = (lookupTableItem*)lookupTable.Next();
    }

    lookupTable.Clear();
}


::com::sun::star::i18n::LocaleDataItem SAL_CALL
LocaleData::getLocaleItem( const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException)
{
    sal_Int16 dataItemCount = 0;
    sal_Unicode **dataItem = NULL;

    MyFunc_Type func = (MyFunc_Type) getFunctionSymbol( rLocale, "getLocaleItem" );

    if ( func ) {
            dataItem = func(dataItemCount);

        ::com::sun::star::i18n::LocaleDataItem item(
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
        ::com::sun::star::i18n::LocaleDataItem item1;
        return item1;
    }
}



::com::sun::star::uno::Sequence< ::com::sun::star::i18n::Calendar > SAL_CALL
LocaleData::getAllCalendars( const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException)
{

    sal_Int16 calendarsCount = 0;
    sal_Unicode **allCalendars = NULL;

    MyFunc_Type func = (MyFunc_Type) getFunctionSymbol( rLocale, "getAllCalendars" );

    if ( func ) {
            allCalendars = func(calendarsCount);

        ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::Calendar > calendarsSeq(calendarsCount);
        sal_Int16 offset = 3;
        sal_Int16 i, j;
        for(i = 0; i < calendarsCount; i++) {
            ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::CalendarItem > days(allCalendars[0][i]);
            ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::CalendarItem > months(allCalendars[1][i]);
            ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::CalendarItem > eras(allCalendars[2][i]);
            ::rtl::OUString calendarID(allCalendars[offset]);
            offset++;
            sal_Bool defaultCalendar = allCalendars[offset][0];
            offset++;
            for(j = 0; j < allCalendars[0][i]; j++) {
                ::com::sun::star::i18n::CalendarItem day(allCalendars[offset], allCalendars[offset+1], allCalendars[offset+2]);
                days[j] = day;
                offset += 3;
            }
            for(j = 0; j < allCalendars[1][i]; j++) {
                ::com::sun::star::i18n::CalendarItem month(allCalendars[offset], allCalendars[offset+1], allCalendars[offset+2]);
                months[j] = month;
                offset += 3;
            }
            for(j = 0; j < allCalendars[2][i]; j++) {
                ::com::sun::star::i18n::CalendarItem era(allCalendars[offset], allCalendars[offset+1], allCalendars[offset+2]);
                eras[j] = era;
                offset += 3;
            }
            ::rtl::OUString startOfWeekDay(allCalendars[offset]);
            offset++;
            sal_Int16 minimalDaysInFirstWeek = allCalendars[offset][0];
            offset++;
            ::com::sun::star::i18n::Calendar aCalendar(days, months, eras, startOfWeekDay, minimalDaysInFirstWeek, defaultCalendar, calendarID);
            calendarsSeq[i] = aCalendar;
        }
        return calendarsSeq;
    }
    else {
        ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::Calendar > seq1(0);
        return seq1;
    }
}


::com::sun::star::uno::Sequence< ::com::sun::star::i18n::Currency > SAL_CALL
LocaleData::getAllCurrencies( const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException)
{

    sal_Int16 currencyCount = 0;
    sal_Unicode **allCurrencies = NULL;

    MyFunc_Type func = (MyFunc_Type) getFunctionSymbol( rLocale, "getAllCurrencies" );

    if ( func ) {
        allCurrencies = func(currencyCount);

        ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::Currency > seq(currencyCount);
        for(int i = 0, nOff = 0; i < currencyCount; i++, nOff += 7 )
        {
            ::com::sun::star::i18n::Currency cur(
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
        ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::Currency > seq1(0);
        return seq1;
    }
}


::com::sun::star::uno::Sequence< ::com::sun::star::i18n::FormatElement > SAL_CALL
LocaleData::getAllFormats( const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException)
{
    sal_Int16 formatCount = 0;
    sal_Unicode **formatArray = NULL;

    MyFunc_Type func = (MyFunc_Type) getFunctionSymbol( rLocale, "getAllFormats" );

    if ( func ) {
        formatArray = func(formatCount);

        ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::FormatElement > seq(formatCount);
        for(int i = 0, nOff = 0; i < formatCount; i++, nOff += 7 )
        {
            ::com::sun::star::i18n::FormatElement elem(formatArray[nOff],
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
        ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::FormatElement > seq1(0);
        return seq1;
    }

}

::com::sun::star::uno::Sequence< ::com::sun::star::i18n::Implementation > SAL_CALL
LocaleData::getCollatorImplementations( const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException)
{
    sal_Int16 collatorCount = 0;
    sal_Unicode **collatorArray = NULL;

    MyFunc_Type func = (MyFunc_Type) getFunctionSymbol( rLocale, "getCollatorImplementation" );

    if ( func ) {
        collatorArray = func(collatorCount);
        ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::Implementation > seq(collatorCount);
        for(sal_Int16 i = 0; i < collatorCount; i++) {
          ::com::sun::star::i18n::Implementation impl(collatorArray[i*2], collatorArray[i*2 + 1][0]);
          seq[i] = impl;
        }
        return seq;
    }
    else {
      ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::Implementation > seq1(0);
      return seq1;
    }
}

::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
LocaleData::getCollationOptions( const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException)
{
    sal_Int16 optionsCount = 0;
    sal_Unicode **optionsArray = NULL;

    MyFunc_Type func = (MyFunc_Type) getFunctionSymbol( rLocale, "getCollationOptions" );

    if ( func ) {
        optionsArray = func(optionsCount);
        ::com::sun::star::uno::Sequence< ::rtl::OUString > seq(optionsCount);
        for(sal_Int16 i = 0; i < optionsCount; i++) {
            seq[i] = ::rtl::OUString( optionsArray[i] );
        }
        return seq;
    }
    else {
        ::com::sun::star::uno::Sequence< ::rtl::OUString > seq1(0);
        return seq1;
    }
}

::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
LocaleData::getSearchOptions( const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException)
{
    sal_Int16 optionsCount = 0;
    sal_Unicode **optionsArray = NULL;

    MyFunc_Type func = (MyFunc_Type) getFunctionSymbol( rLocale, "getSearchOptions" );

    if ( func ) {
        optionsArray = func(optionsCount);
        ::com::sun::star::uno::Sequence< ::rtl::OUString > seq(optionsCount);
        for(sal_Int16 i = 0; i < optionsCount; i++) {
            seq[i] = ::rtl::OUString( optionsArray[i] );
        }
        return seq;
    }
    else {
        ::com::sun::star::uno::Sequence< ::rtl::OUString > seq1(0);
        return seq1;
    }
}


::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
LocaleData::getTransliterations( const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException)
{

    sal_Int16 transliterationsCount = 0;
    sal_Unicode **transliterationsArray = NULL;

    MyFunc_Type func = (MyFunc_Type) getFunctionSymbol( rLocale, "getTransliterations" );

    if ( func ) {
        transliterationsArray = func(transliterationsCount);

        ::com::sun::star::uno::Sequence< ::rtl::OUString > seq(transliterationsCount);
        for(int i = 0; i < transliterationsCount; i++) {
            ::rtl::OUString  elem(transliterationsArray[i]);
            seq[i] = elem;
        }
        return seq;
    }
    else {
        ::com::sun::star::uno::Sequence< ::rtl::OUString > seq1(0);
        return seq1;
    }


}


::com::sun::star::i18n::LanguageCountryInfo SAL_CALL
LocaleData::getLanguageCountryInfo( const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException)
{
    sal_Int16 LCInfoCount = 0;
    sal_Unicode **LCInfoArray = NULL;

    MyFunc_Type func = (MyFunc_Type) getFunctionSymbol( rLocale, "getLCInfo" );

    if ( func ) {
        LCInfoArray = func(LCInfoCount);
        ::com::sun::star::i18n::LanguageCountryInfo info(LCInfoArray[0],
                                            LCInfoArray[1],
                                            LCInfoArray[2],
                                            LCInfoArray[3],
                                            LCInfoArray[4]);
        return info;
    }
    else {
        ::com::sun::star::i18n::LanguageCountryInfo info1;
        return info1;
    }

}


::com::sun::star::i18n::ForbiddenCharacters SAL_CALL
LocaleData::getForbiddenCharacters( const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException)
{
    sal_Int16 LCForbiddenCharactersCount = 0;
    sal_Unicode **LCForbiddenCharactersArray = NULL;

    MyFunc_Type func = (MyFunc_Type) getFunctionSymbol( rLocale, "getForbiddenCharacters" );

    if ( func ) {
        LCForbiddenCharactersArray = func(LCForbiddenCharactersCount);
        ::com::sun::star::i18n::ForbiddenCharacters chars(LCForbiddenCharactersArray[0],
                                                          LCForbiddenCharactersArray[1]);
        return chars;
    }
    else {
        ::com::sun::star::i18n::ForbiddenCharacters chars1;
        return chars1;
    }
}


 ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
 LocaleData::getReservedWord( const ::com::sun::star::lang::Locale& rLocale  ) throw(::com::sun::star::uno::RuntimeException)
 {
    sal_Int16 LCReservedWordsCount = 0;
    sal_Unicode **LCReservedWordsArray = NULL;

    MyFunc_Type func = (MyFunc_Type) getFunctionSymbol( rLocale, "getReservedWords" );

    if ( func ) {
        LCReservedWordsArray = func(LCReservedWordsCount);
        ::com::sun::star::uno::Sequence< ::rtl::OUString > seq(LCReservedWordsCount);
        for(int i = 0; i < (LCReservedWordsCount); i++) {
            ::rtl::OUString  elem(LCReservedWordsArray[i]);
            seq[i] = elem;
        }
        return seq;
    }
    else {
        ::com::sun::star::uno::Sequence< ::rtl::OUString > seq1(0);
        return seq1;
    }
 }


#include <stdio.h>
#include <assert.h>
using namespace ::com::sun::star;

inline
::rtl::OUString C2U( const char* s )
{
     return ::rtl::OUString::createFromAscii( s );
}

uno::Sequence< uno::Sequence<beans::PropertyValue> > SAL_CALL
LocaleData::getContinuousNumberingLevels( const lang::Locale& rLocale ) throw(uno::RuntimeException)
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
          uno::Sequence< uno::Sequence<beans::PropertyValue> > pv( nStyles );
          for( i=0; i<pv.getLength(); i++ )
          {
               pv[i] = uno::Sequence<beans::PropertyValue>( nAttributes );
          }

          sal_Unicode*** pStyle = p0;
          for( i=0;  i<nStyles;  i++ )
          {
               sal_Unicode** pAttribute = pStyle[i];
               for( int j=0;  j<nAttributes;  j++ ) // prefix, numberingtype, ...
               {
                    sal_Unicode* pString = pAttribute[j];
                    beans::PropertyValue& rVal = pv[i][j];
                    rtl::OUString sVal;
                    if( pString )
                    {
                        if( 0 != j && 2 != j )
                            sVal = pString;
                        else if( *pString )
                            sVal = rtl::OUString( pString, 1 );
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
                    default:
                         assert(0);
                    }
               }
          }
          return pv;
     }

     uno::Sequence< uno::Sequence<beans::PropertyValue> > seq1(0);
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
    sal_Unicode     cPrefix;
    sal_Int16       nNumType; //com::sun::star::style::NumberingType
    sal_Unicode     cSuffix;
    sal_Unicode     cBulletChar;
    const sal_Char* sBulletFontName;
    sal_Int16       nParentNumbering;
    sal_Int32       nLeftMargin;
    sal_Int32       nSymbolTextDistance;
    sal_Int32       nFirstLineOffset;
    ::rtl::OUString sTransliteration;
};
//-----------------------------------------------------------------------------
class OutlineNumbering : public cppu::WeakImplHelper1
                                    < com::sun::star::container::XIndexAccess >
{
    const OutlineNumberingLevel_Impl*   m_pOutlineLevels;
    sal_Int16                           m_nCount;
public:
//  OutlineNumbering(const OutlineNumberingLevel_Impl* pOutlineLevels);
    OutlineNumbering(const OutlineNumberingLevel_Impl* pOutlineLevels, int nLevels);
    ~OutlineNumbering();

    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

};

//
//  /\ OutlineNumbering helper class /\
// ============================================================================

static
sal_Char* U2C( ::rtl::OUString str )
{
     sal_Char* s = new sal_Char[ str.getLength()+1 ];
     for(int i=0; i<str.getLength(); i++) s[i] = str[i];
     s[i]='\0';
     return s;
}



uno::Sequence< uno::Reference<container::XIndexAccess> > SAL_CALL
LocaleData::getOutlineNumberingLevels( const lang::Locale& rLocale ) throw(uno::RuntimeException)
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

          uno::Sequence< uno::Reference<container::XIndexAccess> > aRet( nStyles );

          rtl::OUString aEmptyStr;

          sal_Unicode**** pStyle = p0;
          for( i=0;  i<nStyles;  i++ )
          {
               OutlineNumberingLevel_Impl* level = new OutlineNumberingLevel_Impl[ nLevels+1 ];
               sal_Unicode*** pLevel = pStyle[i];
               for( int j=0;  j<nLevels;  j++ )
               {
                    sal_Unicode** pAttribute = pLevel[j];
                    for( int k=0; k<nAttributes; k++ )
                    {
                         ::rtl::OUString tmp( pAttribute[k] );
                         switch( k )
                         {
                         case 0: level[j].cPrefix             = tmp.toChar();    break;
                         case 1: level[j].nNumType            = tmp.toInt32();   break;
                         case 2: level[j].cSuffix             = tmp.toChar();    break;
//                       case 3: level[j].cBulletChar         = tmp.toChar();    break;
                         case 3: level[j].cBulletChar         = tmp.toInt32(16); break; // base 16
                         case 4: level[j].sBulletFontName     = U2C( tmp );      break;
                         case 5: level[j].nParentNumbering    = tmp.toInt32();   break;
                         case 6: level[j].nLeftMargin         = tmp.toInt32();   break;
                         case 7: level[j].nSymbolTextDistance = tmp.toInt32();   break;
                         case 8: level[j].nFirstLineOffset    = tmp.toInt32();   break;
                         case 9: // Adjust
                              // thise values seem to be hard-coded elsewhere:
//                           level[j].Value <<= (sal_Int16) text::HoriOrientation::LEFT;
//                           level[j].Value <<= (sal_Int16) style::HorizontalAlignment::LEFT;
                              break;
                         case 10: level[j].sTransliteration = tmp; break;
                         default:
                              assert(0);
                         }
                    }
               }
               level[j].cPrefix             = 0;
               level[j].nNumType            = 0;
               level[j].cSuffix             = 0;
               level[j].cBulletChar         = 0;
               level[j].sBulletFontName     = 0;
               level[j].nParentNumbering    = 0;
               level[j].nLeftMargin         = 0;
               level[j].nSymbolTextDistance = 0;
               level[j].nFirstLineOffset    = 0;
                  level[j].sTransliteration     = aEmptyStr;
               aRet[i] = new OutlineNumbering( level, nLevels );
          }
          return aRet;
     }
     else {
          uno::Sequence< uno::Reference<container::XIndexAccess> > seq1(0);
          return seq1;
     }
}

/////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////helper functions///////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

void* LocaleData::getFunctionSymbol( const ::com::sun::star::lang::Locale& rLocale,
            const sal_Char* pFunction, sal_Bool bFallBack )
{
    void* pSymbol = 0;
    ::rtl::OUString dllName, functionName;
    ::rtl::OUString aFunction( ::rtl::OUString::createFromAscii( pFunction ) );
    setFunctionName( rLocale, aFunction, dllName, functionName, bFallBack );
    oslModule hModule = getModuleHandle( dllName );
    if ( hModule )
        pSymbol = osl_getSymbol( hModule, functionName.pData );
    if ( !pSymbol && bFallBack )
    {
        ::com::sun::star::lang::Locale aLoc( rLocale );
        sal_Bool bLoop = sal_True;
        do
        {
            if ( hModule )
            {   // setFunctionName did find an entry in dllsTable but it's not
                // available in the library, try with locale fallback first
                if ( aLoc.Variant.getLength() )
                    aLoc.Variant = ::rtl::OUString();
                else if ( aLoc.Country.getLength() )
                    aLoc.Country = ::rtl::OUString();
                else
                    hModule = 0;    // last try with en_US
            }
            if ( !hModule )
            {   // not even the library was found, last resort
                aLoc.Language = ::rtl::OUString::createFromAscii( "en" );
                aLoc.Country = ::rtl::OUString::createFromAscii( "US" );
                bLoop = sal_False;
            }
            setFunctionName( aLoc, aFunction, dllName, functionName, bFallBack );
            oslModule hModule = getModuleHandle( dllName );
            if ( hModule )
                pSymbol = osl_getSymbol( hModule, functionName.pData );
        } while ( !pSymbol && bLoop );
    }
    return pSymbol;
}


oslModule
LocaleData::getModuleHandle(const ::rtl::OUString& dllName) {

    oslModule hModule = NULL;
    lookupTableItem *listItem = (lookupTableItem*)lookupTable.First();
    if(listItem) {
        do {
            if(dllName.equals(listItem->adllName)) {
                hModule = listItem->dllHandle;
                break;
            }
        } while((listItem = (lookupTableItem*)lookupTable.Next()));
    }
    if(!hModule) { //dll not loaded, load it and add it to the list
        hModule = osl_loadModule(dllName.pData, SAL_LOADMODULE_DEFAULT );
        lookupTableItem *newTableItem = new lookupTableItem();
        newTableItem->adllName = dllName;
        newTableItem->dllHandle = hModule;
        lookupTable.Insert(newTableItem);
    }
    return hModule;

}


void
LocaleData::setFunctionName( const ::com::sun::star::lang::Locale& rLocale,
            const ::rtl::OUString& function, ::rtl::OUString& dllName,
            ::rtl::OUString& functionName, sal_Bool bFallBack )
{
    TableElement dllEntry = getDLLName( rLocale, bFallBack );
    dllName = dllEntry.value;

    ::rtl::OUStringBuffer aBuf( function.getLength() + 1 + dllEntry.name.getLength() );
    aBuf.append( function );
    aBuf.append( sal_Unicode('_') );
    aBuf.append( dllEntry.name );
    functionName = aBuf.makeStringAndClear();

    ::rtl::OUString platformSuffix;
    #ifdef SAL_W32
        platformSuffix = ::rtl::OUString::createFromAscii(".dll");
    #else
        ::rtl::OUString platformPrefix = ::rtl::OUString::createFromAscii("lib");
        dllName = platformPrefix;
        dllName += dllEntry.value;
        platformSuffix = ::rtl::OUString::createFromAscii(".so");
    #endif
    dllName += platformSuffix;

}


TableElement SAL_CALL
LocaleData::getDLLName( const ::com::sun::star::lang::Locale& rLocale,
            sal_Bool bFallBack )
{

    ::rtl::OUString localeName2;
    TableElement dllEntry;
    sal_Bool found = sal_False;

    // for most used cases
    if( rLocale.Country.getLength() > 0 )
    {
        ::rtl::OUStringBuffer aBuf( rLocale.Language.getLength() + 1
            + rLocale.Country.getLength() );
        aBuf.append( rLocale.Language );
        aBuf.append( sal_Unicode('_') );
        aBuf.append( rLocale.Country );
        localeName2 = aBuf.makeStringAndClear();
    }
    // First look for object corresponding to Language_Country_Variant.
    // Since this is a rare case, string is not prebuild.
    if( rLocale.Variant.getLength() > 0 )
    {
        ::rtl::OUStringBuffer aBuf( localeName2.getLength() + 1
            + rLocale.Variant.getLength() );
        aBuf.append( localeName2 );
        aBuf.append( sal_Unicode('_') );
        aBuf.append( rLocale.Variant );
        found  = lookupDLLName( aBuf.makeStringAndClear(), dllEntry);
        if ( found || !bFallBack )
            return dllEntry;
    }
    // then look for object corresponding to Language_Country
    if(!found && localeName2.getLength() > 0)
    {
        found = lookupDLLName(localeName2, dllEntry);
        if ( found || !bFallBack )
            return dllEntry;
    }
    // then look for object corresponding to Language
    if(!found)
    {
        found = lookupDLLName( rLocale.Language, dllEntry );
        if ( found || !bFallBack )
            return dllEntry;
    }

    //could not find a match, return the default one (en_US)
    if(!found)
            return TableElement(::rtl::OUString::createFromAscii("en_US"), lcl_ASCII);
    return dllEntry;
}

sal_Bool  SAL_CALL
LocaleData::lookupDLLName(const ::rtl::OUString& localeName, TableElement& element) {

    for(sal_Int16 i = 0; i < nbOfLocales; i++) {
        if(dllsTable[i].name.equals(localeName)) {
            element = dllsTable[i];
            return true;
        }
    }
    return false;
}

::com::sun::star::uno::Sequence< com::sun::star::lang::Locale > SAL_CALL
LocaleData::getAllInstalledLocaleNames() throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Sequence< com::sun::star::lang::Locale > seq( nbOfLocales );

    sal_Int16 nInstalled = 0;
    rtl::OUString aEmptyStr;

    for( sal_Int16 i=0; i<nbOfLocales; i++ )
    {
        com::sun::star::lang::Locale tmpLocale;
        rtl::OUString name = dllsTable[i].name;

        int start = 0;
        int stop  = name.indexOf( '_', start );
        int last  = name.getLength() - 1;

        if( stop == -1 || stop == last )
        {
            tmpLocale.Language = name.copy( start );
            tmpLocale.Country  = aEmptyStr;
            tmpLocale.Variant  = aEmptyStr;
        }
        else
        {
            tmpLocale.Language = name.copy( start, stop-start );

            start = stop+1;
            stop  = name.indexOf( '_', start );

            if( stop == -1 || stop == last )
            {
                tmpLocale.Country = name.copy( start );
                tmpLocale.Variant = aEmptyStr;
            }
            else
            {
                tmpLocale.Country = name.copy( start, stop-start );
                tmpLocale.Variant = name.copy( stop+1 );
            }
        }

        // Check if the locale is really available and not just in the table,
        // don't allow fall backs.
        MyFunc_Type func = (MyFunc_Type) getFunctionSymbol( tmpLocale, "getLocaleItem", sal_False );
        if ( func )
            seq[nInstalled++] = tmpLocale;
    }

    if ( nInstalled < nbOfLocales )
        seq.realloc( nInstalled );      // reflect reality

    return seq;
}

// ============================================================================

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::text;
using namespace ::rtl;

// // bad: can't have empty prefix ...
// OutlineNumbering::OutlineNumbering(const OutlineNumberingLevel_Impl* pOutlnLevels) :
//         m_pOutlineLevels(pOutlnLevels),
//         m_nCount(0)
// {
//         const OutlineNumberingLevel_Impl* pTemp = m_pOutlineLevels;
//         while((pTemp++)->cPrefix)
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

    Sequence<PropertyValue> aOutlineNumbering(11);
    PropertyValue* pValues = aOutlineNumbering.getArray();
    pValues[0].Name = C2U( "Prefix");
    pValues[0].Value <<= OUString(&pTemp->cPrefix, 1);
    pValues[1].Name = C2U("NumberingType");
    pValues[1].Value <<= pTemp->nNumType;
    pValues[2].Name = C2U("Suffix");
    pValues[2].Value <<= OUString(&pTemp->cSuffix, 1);
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
