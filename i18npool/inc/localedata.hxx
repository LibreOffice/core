/*************************************************************************
 *
 *  $RCSfile: localedata.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: kz $ $Date: 2004-07-30 14:37:30 $
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
#ifndef _I18N_LOCALEDATA_HXX_
#define _I18N_LOCALEDATA_HXX_

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <comphelper/processfactory.hxx>

#include <com/sun/star/i18n/XLocaleData.hpp>


#include <cppuhelper/implbase2.hxx> // helper for implementations
#include <cppu/macros.hxx>
#include <com/sun/star/uno/Reference.h>

#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/i18n/Calendar.hpp>
#include <com/sun/star/i18n/FormatElement.hpp>
#include <com/sun/star/i18n/Currency.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/i18n/LocaleDataItem.hpp>
#include <com/sun/star/i18n/UnicodeScript.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/XInterface.hpp>
#include <rtl/ustring.hxx>
#include <vector>
#include <osl/module.hxx>

//
#ifndef _I18N_DEFAULT_NUMBERING_PROVIDER_HXX_
#include <defaultnumberingprovider.hxx>
#endif
#ifndef _COM_SUN_STAR_STYLE_NUMBERINGTYPE_HPP_
#include <com/sun/star/style/NumberingType.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_HORIORIENTATION_HPP_
#include <com/sun/star/text/HoriOrientation.hpp>
#endif
//

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

namespace com { namespace sun { namespace star { namespace i18n {

class LocaleData : public cppu::WeakImplHelper2
<
    XLocaleData,
    com::sun::star::lang::XServiceInfo
>
{
public:
    LocaleData(){
        cachedItem = NULL;
    }
    ~LocaleData();

    virtual LanguageCountryInfo SAL_CALL getLanguageCountryInfo( const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException);
    virtual LocaleDataItem SAL_CALL getLocaleItem( const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Sequence< Calendar > SAL_CALL getAllCalendars( const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Sequence< Currency > SAL_CALL getAllCurrencies( const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Sequence< FormatElement > SAL_CALL getAllFormats( const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Sequence< Implementation > SAL_CALL getCollatorImplementations( const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getTransliterations( const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException);
    virtual ForbiddenCharacters SAL_CALL getForbiddenCharacters( const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getReservedWord( const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException) ;
    virtual com::sun::star::uno::Sequence< com::sun::star::lang::Locale > SAL_CALL getAllInstalledLocaleNames() throw(com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSearchOptions( const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getCollationOptions( const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Sequence< com::sun::star::uno::Sequence< beans::PropertyValue > > SAL_CALL getContinuousNumberingLevels( const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Sequence< com::sun::star::uno::Reference< container::XIndexAccess > > SAL_CALL getOutlineNumberingLevels( const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException);

    // following methods are used by indexentry service
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getIndexAlgorithm( const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException);
    virtual rtl::OUString SAL_CALL getDefaultIndexAlgorithm( const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException);
    virtual rtl::OUString SAL_CALL getIndexKeysByAlgorithm( const com::sun::star::lang::Locale& rLocale, const rtl::OUString& algorithm ) throw(com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Sequence< UnicodeScript > SAL_CALL getUnicodeScripts( const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getFollowPageWords( const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasPhonetic( const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isPhonetic( const com::sun::star::lang::Locale& rLocale, const rtl::OUString& algorithm ) throw(com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName() throw( com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames() throw( com::sun::star::uno::RuntimeException );

private :
#if defined(_MSC_VER) && (_MSC_VER >= 1310 )
    inline friend sal_Bool operator ==(const com::sun::star::lang::Locale& l1, const com::sun::star::lang::Locale& l2);
#else
    inline friend sal_Bool operator ==(const com::sun::star::lang::Locale& l1, const com::sun::star::lang::Locale& l2) {
        return l1.Language == l2.Language && l1.Country == l2.Country && l1.Variant == l2.Variant;
    };
#endif

    struct lookupTableItem {
        lookupTableItem(const sal_Char *name, osl::Module* m) : dllName(name), module(m) {}
        const sal_Char* dllName;
        const sal_Char* localeName;
        com::sun::star::lang::Locale aLocale;
        osl::Module *module;
        sal_Bool equals(const com::sun::star::lang::Locale& rLocale) {
        return (rLocale == aLocale);
        }
    };
    std::vector<lookupTableItem*> lookupTable;
    lookupTableItem *cachedItem;

    void* SAL_CALL getFunctionSymbol( const com::sun::star::lang::Locale& rLocale, const sal_Char* pFunction ) throw( com::sun::star::uno::RuntimeException );
    void* SAL_CALL getFunctionSymbolByName( const rtl::OUString& localeName, const sal_Char* pFunction );
    sal_Unicode ** SAL_CALL getIndexArray(const com::sun::star::lang::Locale& rLocale, sal_Int16& indexCount);
    com::sun::star::i18n::Calendar ref_cal;
    rtl::OUString ref_name;
    com::sun::star::uno::Sequence< com::sun::star::i18n::CalendarItem > &
        getCalendarItemByName(const rtl::OUString& name,
        const com::sun::star::lang::Locale& rLocale,
        const com::sun::star::uno::Sequence< com::sun::star::i18n::Calendar >& calendarsSeq,
        sal_Int16 len, sal_Int16 item) throw( com::sun::star::uno::RuntimeException );
};
#if defined(_MSC_VER) && (_MSC_VER >= 1310 )
    inline sal_Bool operator ==(const com::sun::star::lang::Locale& l1, const com::sun::star::lang::Locale& l2) {
        return l1.Language == l2.Language && l1.Country == l2.Country && l1.Variant == l2.Variant;
    };
#endif

} } } }

#endif // _I18N_LOCALEDATA_HXX_
