/*************************************************************************
#*
#*  $RCSfile: localedata.hxx,v $
#*
#*  Localedata
#*
#*  Ersterstellung  SSE 06/07/2000
#*
#*  Letzte Aenderung    $Author: bustamam $ $Date: 2001-08-30 23:33:16 $
#*
#*  $Revision: 1.1 $
#*
#*  $Source: /zpool/svn/migration/cvs_rep_09_09_08/code/i18npool/inc/localedata.hxx,v $
#*
#*  Copyright (c) 2000 Sun Microsystems Inc.
#*
#*************************************************************************/
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
