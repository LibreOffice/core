/*************************************************************************
 *
 *  $RCSfile: localedatawrapper.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: er $ $Date: 2000-11-04 21:46:24 $
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

#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#define _UNOTOOLS_LOCALEDATAWRAPPER_HXX

#ifndef _TOOLS_INTN_HXX
#include <tools/intn.hxx>       // enum MeasurementSystem
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef _COM_SUN_STAR_I18N_XLOCALEDATA_HPP_
#include <com/sun/star/i18n/XLocaleData.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_LOCALEITEM_HPP_
#include <com/sun/star/i18n/LocaleItem.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_RESERVEDWORDS_HPP_
#include <com/sun/star/i18n/reservedWords.hpp>
#endif

namespace com { namespace sun { namespace star {
    namespace lang {
        class XMultiServiceFactory;
    }
}}}


class LocaleDataWrapper
{
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xSMgr;
    ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XLocaleData > xLD;
    ::com::sun::star::lang::Locale          aLocale;
    ::com::sun::star::i18n::LocaleDataItem  aLocaleDataItem;
    ::com::sun::star::uno::Sequence< ::rtl::OUString >  aReservedWordSeq;
    // cached items
    String                      aLocaleItem[::com::sun::star::i18n::LocaleItem::COUNT];
    String                      aReservedWord[::com::sun::star::i18n::reservedWords::COUNT];
    String                      aCurrSymbol;
    String                      aCurrBankSymbol;
    USHORT                      nCurrPositiveFormat;
    USHORT                      nCurrNegativeFormat;
    USHORT                      nCurrDigits;
    BOOL                        bLocaleDataItemValid;
    BOOL                        bReservedWordValid;

    // dummies, to be implemented or provided by XML locale data
    String                      aLongDateDayOfWeekSep;
    sal_Unicode                 cCurrZeroChar;


                                // not implemented, prevent usage
                                LocaleDataWrapper( const LocaleDataWrapper& );
            LocaleDataWrapper&  operator=( const LocaleDataWrapper& );

                                // whenever Locale changes
            void                invalidateData();

            void                getOneLocaleItemImpl( sal_Int16 nItem );
            const String&       getOneLocaleItem( sal_Int16 nItem ) const;

            void                getOneReservedWordImpl( sal_Int16 nWord );
            const String&       getOneReservedWord( sal_Int16 nWord ) const;

            void                getCurrSymbolsImpl();
            void                getCurrFormatsImpl();

            void                scanCurrFormat( const String& rCode,
                                    xub_StrLen nStart, xub_StrLen& nSign,
                                    xub_StrLen& nPar, xub_StrLen& nNum,
                                    xub_StrLen& nBlank, xub_StrLen& nSym );

public:
                                LocaleDataWrapper(
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & xSF,
                                    const ::com::sun::star::lang::Locale& rLocale
                                    );

                                ~LocaleDataWrapper();


    /// set a new Locale to request
            void                setLocale( const ::com::sun::star::lang::Locale& rLocale );

    /// get current requested Locale
    const ::com::sun::star::lang::Locale& getLocale() const { return aLocale; }

    /// get current loaded Locale, which might differ from the requested Locale
    ::com::sun::star::lang::Locale getLoadedLocale() const;


    // Wrapper implementations of class LocaleData

    ::com::sun::star::i18n::LanguageCountryInfo getLanguageCountryInfo() const;
    ::com::sun::star::i18n::LocaleDataItem getLocaleItem() const;
    ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::Calendar > getAllCalendars() const;
    ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::Currency > getAllCurrencies() const;
    ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::FormatElement > getAllFormats() const;
    ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::Implementation > getCollatorImplementations() const;
    ::com::sun::star::uno::Sequence< ::rtl::OUString > getTransliterations() const;
    ::com::sun::star::i18n::ForbiddenCharacters getForbiddenCharacters() const;
    ::com::sun::star::uno::Sequence< ::rtl::OUString > getReservedWord() const;


    /// maps the LocaleData string to the International enum
            MeasurementSystem   mapMeasurementStringToEnum( const String& rMS ) const;


    // Functionality of class International methods, LocaleItem

    inline  const String&       getDateSep() const
                                    { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::DATE_SEPARATOR ); }
    inline  const String&       getNumThousandSep() const
                                    { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::THOUSAND_SEPARATOR ); }
    inline  const String&       getNumDecimalSep() const
                                    { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::DECIMAL_SEPARATOR ); }
    inline  const String&       getTimeSep() const
                                    { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::TIME_SEPARATOR ); }
    inline  const String&       getTime100SecSep() const
                                    { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::TIME_100SEC_SEPARATOR ); }
    inline  const String&       getListSep() const
                                    { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::LIST_SEPARATOR ); }
    inline  const String&       getQuotationMarkStart() const
                                    { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::SINGLE_QUOTATION_START ); }
    inline  const String&       getQuotationMarkEnd() const
                                    { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::SINGLE_QUOTATION_END ); }
    inline  const String&       getDoubleQuotationMarkStart() const
                                    { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::DOUBLE_QUOTATION_START ); }
    inline  const String&       getDoubleQuotationMarkEnd() const
                                    { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::DOUBLE_QUOTATION_END ); }
    inline  const String&       getMeasurementSystem() const
                                    { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::MEASUREMENT_SYSTEM ); }
    inline  MeasurementSystem   getMeasurementSystemEnum() const
                                    { return mapMeasurementStringToEnum( getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::MEASUREMENT_SYSTEM ) ); }
    inline  const String&       getTimeAM() const
                                    { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::TIME_AM ); }
    inline  const String&       getTimePM() const
                                    { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::TIME_PM ); }

            const String&       getCurrSymbol() const;
            const String&       getCurrBankSymbol() const;
            USHORT              getCurrPositiveFormat() const;
            USHORT              getCurrNegativeFormat() const;
            USHORT              getCurrDigits() const;

    // dummy returns
    inline  const String&       getLongDateDayOfWeekSep() const
                                    { return aLongDateDayOfWeekSep; }
    inline  sal_Unicode         getCurrZeroChar() const
                                    { return cCurrZeroChar; }


    // reserved words

    inline  const String&       getTrueWord() const
                                    { return getOneReservedWord( ::com::sun::star::i18n::reservedWords::TRUE_WORD ); }
    inline  const String&       getFalseWord() const
                                    { return getOneReservedWord( ::com::sun::star::i18n::reservedWords::FALSE_WORD ); }

#ifndef PRODUCT
            ByteString&         AppendLocaleInfo( ByteString& rDebugMsg ) const;
#endif

};


#endif // _UNOTOOLS_LOCALEDATAWRAPPER_HXX
