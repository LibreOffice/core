/*************************************************************************
 *
 *  $RCSfile: localedata.hxx,v $
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
#ifndef _I18N_LOCALEDATA_HXX_
#define _I18N_LOCALEDATA_HXX_

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <comphelper/processfactory.hxx>

#include <com/sun/star/i18n/XLocaleData.hpp>


#include <cppuhelper/implbase1.hxx> // helper for implementations
#include <cppu/macros.hxx>
#include <com/sun/star/uno/Reference.h>

#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/i18n/Calendar.hpp>
#include <com/sun/star/i18n/FormatElement.hpp>
#include <com/sun/star/i18n/Currency.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/i18n/LocaleDataItem.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/XInterface.hpp>
#include <rtl/ustring.hxx>
#include <tools/string.hxx>
#include <tools/list.hxx>
#include <tableelement.h>
#include <osl/module.h>

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

class LocaleData : public cppu::WeakImplHelper1< ::com::sun::star::i18n::XLocaleData>
{


public:
    LocaleData(){
    }
    ~LocaleData();

    virtual ::com::sun::star::i18n::LanguageCountryInfo SAL_CALL getLanguageCountryInfo( const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::i18n::LocaleDataItem SAL_CALL getLocaleItem( const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::Calendar > SAL_CALL getAllCalendars( const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::Currency > SAL_CALL getAllCurrencies( const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::FormatElement > SAL_CALL getAllFormats( const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::Implementation > SAL_CALL getCollatorImplementations( const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getTransliterations( const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::i18n::ForbiddenCharacters SAL_CALL getForbiddenCharacters( const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getReservedWord( const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException) ;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::lang::Locale > SAL_CALL getAllInstalledLocaleNames() throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSearchOptions( const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getCollationOptions( const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > > SAL_CALL getContinuousNumberingLevels( const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > > SAL_CALL getOutlineNumberingLevels( const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException);
private :
    struct lookupTableItem {
        ::rtl::OUString adllName;
        oslModule dllHandle;
    };

    List lookupTable;

    static const TableElement  dllsTable[];
    static const sal_Int16 nbOfLocales;
    void* getFunctionSymbol( const ::com::sun::star::lang::Locale& rLocale,
            const sal_Char* pFunction, sal_Bool bFallBack = sal_True );
    void setFunctionName( const ::com::sun::star::lang::Locale& rLocale,
            const ::rtl::OUString& function, ::rtl::OUString& dllName,
            ::rtl::OUString& functionName, sal_Bool bFallBack );
    sal_Bool  SAL_CALL lookupDLLName(const ::rtl::OUString& localeName, TableElement& element);
    TableElement SAL_CALL getDLLName( const ::com::sun::star::lang::Locale& rLocale,
            sal_Bool bFallBack );
    oslModule getModuleHandle(const ::rtl::OUString& dllName);
};

#endif // _I18N_LOCALEDATA_HXX_
