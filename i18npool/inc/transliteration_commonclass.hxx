/*************************************************************************
 *
 *  $RCSfile: transliteration_commonclass.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: bustamam $ $Date: 2002-03-26 13:36:40 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
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
#ifndef _I18N_TRANSLITERATION_TRANSLITERATION_COMMONCLASS_H_
#define _I18N_TRANSLITERATION_TRANSLITERATION_COMMONCLASS_H_

#include <com/sun/star/i18n/XTransliteration.hpp>
#include <com/sun/star/i18n/TransliterationType.hpp>
#include <cppuhelper/implbase1.hxx>
#include <rtl/ustrbuf.h>
#include <rtl/ustring.hxx>
#include <x_rtl_ustring.h>

namespace com { namespace sun { namespace star { namespace i18n {

class transliteration_commonclass : public cppu::WeakImplHelper1< XTransliteration >
{
public:
    transliteration_commonclass();

    // Methods which are shared.
    void SAL_CALL
    loadModule( TransliterationModules modName, const com::sun::star::lang::Locale& rLocale )
    throw(com::sun::star::uno::RuntimeException);

    void SAL_CALL
    loadModuleNew( const com::sun::star::uno::Sequence< TransliterationModulesNew >& modName, const com::sun::star::lang::Locale& rLocale )
    throw(com::sun::star::uno::RuntimeException);

    void SAL_CALL
    loadModuleByImplName( const rtl::OUString& implName, const com::sun::star::lang::Locale& rLocale )
    throw(com::sun::star::uno::RuntimeException);

    void SAL_CALL
    loadModulesByImplNames(const com::sun::star::uno::Sequence< rtl::OUString >& modNamelist, const com::sun::star::lang::Locale& rLocale)
    throw(com::sun::star::uno::RuntimeException);

    com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL
    getAvailableModules( const com::sun::star::lang::Locale& rLocale, sal_Int16 sType )
    throw(com::sun::star::uno::RuntimeException);

    // Methods which should be implemented in each transliteration module.
    virtual rtl::OUString SAL_CALL getName() throw(com::sun::star::uno::RuntimeException);

    virtual sal_Int16 SAL_CALL getType(  ) throw(com::sun::star::uno::RuntimeException) = 0;

    virtual rtl::OUString SAL_CALL
    transliterate( const rtl::OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, com::sun::star::uno::Sequence< sal_Int32 >& offset  )
    throw(com::sun::star::uno::RuntimeException) = 0;

    virtual rtl::OUString SAL_CALL
    folding( const rtl::OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, com::sun::star::uno::Sequence< sal_Int32 >& offset)
    throw(com::sun::star::uno::RuntimeException) = 0;

    virtual sal_Bool SAL_CALL
    equals( const rtl::OUString& str1, sal_Int32 pos1, sal_Int32 nCount1, sal_Int32& nMatch1, const rtl::OUString& str2, sal_Int32 pos2, sal_Int32 nCount2, sal_Int32& nMatch2 )
    throw(com::sun::star::uno::RuntimeException) = 0;

    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL
    transliterateRange( const rtl::OUString& str1, const rtl::OUString& str2 )
    throw(com::sun::star::uno::RuntimeException) = 0;

    virtual sal_Int32 SAL_CALL
    compareSubstring( const rtl::OUString& s1, sal_Int32 off1, sal_Int32 len1, const rtl::OUString& s2, sal_Int32 off2, sal_Int32 len2)
    throw(com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL
    compareString( const rtl::OUString& s1, const rtl::OUString& s2)
    throw(com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName()
        throw( com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName)
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames()
        throw( com::sun::star::uno::RuntimeException );
protected:
    com::sun::star::lang::Locale   aLocale;
    sal_Char* transliterationName;
    sal_Char* implementationName;
};

} } } }

#endif // _I18N_TRANSLITERATION_TRANSLITERATION_COMMONCLASS_H_
