/*************************************************************************
 *
 *  $RCSfile: transliterationImpl.hxx,v $
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
#ifndef _I18N_TRANSLITERATIONIMPL_HXX_
#define _I18N_TRANSLITERATIONIMPL_HXX_

#include <com/sun/star/i18n/XLocaleData.hpp>
#include <com/sun/star/i18n/XTransliteration.hpp>
#include <cppuhelper/implbase2.hxx> // helper for implementations
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <sal/types.h>

namespace com { namespace sun { namespace star { namespace i18n {

#ifdef DEBUG
#define RETURN_DEBUG_STRING (rtl::OUString::createFromAscii("return from transliterationImpl"));
#define RETURN_DEBUG_NUMBER 'TRAN'
#else
#define RETURN_DEBUG_STRING
#define RETURN_DEBUG_NUMBER
#endif

class TransliterationImpl : public cppu::WeakImplHelper2
<
    XTransliteration,
    com::sun::star::lang::XServiceInfo
>
{
public:
    // Constructors
    TransliterationImpl(const com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory >& xMSF);
    // Destructor
    ~TransliterationImpl();

    // Methods
    virtual rtl::OUString SAL_CALL getName(  ) throw(com::sun::star::uno::RuntimeException) ;
    virtual sal_Int16 SAL_CALL getType(  ) throw(com::sun::star::uno::RuntimeException) ;

    virtual void SAL_CALL loadModule( TransliterationModules modName, const com::sun::star::lang::Locale& rLocale )
        throw(com::sun::star::uno::RuntimeException) ;
    virtual void SAL_CALL loadModuleNew( const com::sun::star::uno::Sequence< TransliterationModulesNew >& modName,
        const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException) ;
    virtual void SAL_CALL loadModuleByImplName( const rtl::OUString& implName,
        const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL loadModulesByImplNames(const com::sun::star::uno::Sequence< rtl::OUString >& modNamelist,
        const com::sun::star::lang::Locale& rLocale) throw(com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getAvailableModules(
        const com::sun::star::lang::Locale& rLocale, sal_Int16 sType )
        throw(com::sun::star::uno::RuntimeException);

    virtual rtl::OUString SAL_CALL transliterate( const rtl::OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
        com::sun::star::uno::Sequence< sal_Int32 >& offset  ) throw(com::sun::star::uno::RuntimeException) ;

    virtual rtl::OUString SAL_CALL folding( const rtl::OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
        com::sun::star::uno::Sequence< sal_Int32 >& offset ) throw(com::sun::star::uno::RuntimeException);
/*
    virtual void SAL_CALL createCascadeInstance( const com::sun::star::uno::Sequence< rtl::OUString >& modNamelist,
        const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException) ;
    virtual rtl::OUString SAL_CALL folding_reverse( const rtl::OUString& inStr, sal_Int32 startPos,
        sal_Int32 nCount ) throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL transliterateMatch( const rtl::OUString& content, sal_Int32 pos1, sal_Int32 nCount1,
        const rtl::OUString& pattern, sal_Int32 pos2, sal_Int32 nCount2 )
        throw(com::sun::star::uno::RuntimeException);
*/
    virtual sal_Bool SAL_CALL equals( const rtl::OUString& str1, sal_Int32 pos1, sal_Int32 nCount1,
        sal_Int32& nMatch1, const rtl::OUString& str2, sal_Int32 pos2, sal_Int32 nCount2, sal_Int32& nMatch2 )
        throw(com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL transliterateRange( const rtl::OUString& str1,
        const rtl::OUString& str2 ) throw(com::sun::star::uno::RuntimeException) ;

    virtual sal_Int32 SAL_CALL compareSubstring( const rtl::OUString& s1, sal_Int32 off1, sal_Int32 len1,
        const rtl::OUString& s2, sal_Int32 off2, sal_Int32 len2) throw(com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL compareString( const rtl::OUString& s1, const rtl::OUString& s2)
        throw(com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName)
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void)
        throw( com::sun::star::uno::RuntimeException );

private:
#define maxCascade 27
    com::sun::star::uno::Reference< XTransliteration > bodyCascade[maxCascade];
    sal_Int16 numCascade;
    sal_Bool caseignoreOnly;
    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > xSMgr;
    com::sun::star::uno::Reference< XLocaleData > localedata;
    com::sun::star::uno::Reference< XTransliteration > caseignore;

    virtual sal_Bool SAL_CALL loadModuleByName( const rtl::OUString& implName,
        com::sun::star::uno::Reference<XTransliteration> & body, const com::sun::star::lang::Locale& rLocale)
        throw(com::sun::star::uno::RuntimeException);

    void clear();

    com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getRange(
        const com::sun::star::uno::Sequence< rtl::OUString > &inStrs,
        sal_Int32 length, const sal_Int16 _nCascade)
        throw(com::sun::star::uno::RuntimeException);
};

} } } }

#endif
