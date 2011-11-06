/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _I18N_TRANSLITERATIONIMPL_HXX_
#define _I18N_TRANSLITERATIONIMPL_HXX_

#include <com/sun/star/i18n/XLocaleData.hpp>
#include <com/sun/star/i18n/XExtendedTransliteration.hpp>
#include <cppuhelper/implbase2.hxx> // helper for implementations
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <sal/types.h>

namespace com { namespace sun { namespace star { namespace i18n {

#if OSL_DEBUG_LEVEL > 1
#define RETURN_DEBUG_STRING (rtl::OUString::createFromAscii("return from transliterationImpl"));
#define RETURN_DEBUG_NUMBER 'TRAN'
#else
#define RETURN_DEBUG_STRING
#define RETURN_DEBUG_NUMBER
#endif

class TransliterationImpl : public cppu::WeakImplHelper2
<
    com::sun::star::i18n::XExtendedTransliteration,
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

    // Methods in XExtendedTransliteration
    virtual rtl::OUString SAL_CALL transliterateString2String( const rtl::OUString& inStr, sal_Int32 startPos, sal_Int32 nCount )
        throw(com::sun::star::uno::RuntimeException) ;
    virtual rtl::OUString SAL_CALL transliterateChar2String( sal_Unicode inChar )
        throw(com::sun::star::uno::RuntimeException) ;
    virtual sal_Unicode SAL_CALL transliterateChar2Char( sal_Unicode inChar )
        throw(com::sun::star::i18n::MultipleCharsOutputException,
                com::sun::star::uno::RuntimeException) ;

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
    com::sun::star::uno::Reference< com::sun::star::i18n::XExtendedTransliteration > bodyCascade[maxCascade];
    sal_Int16 numCascade;
    sal_Bool caseignoreOnly;
    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > xSMgr;
    com::sun::star::uno::Reference< XLocaleData > localedata;
    com::sun::star::uno::Reference< com::sun::star::i18n::XExtendedTransliteration > caseignore;

    /** structure to cache the last transliteration body used. */
    struct TransBody
    {
        ::osl::Mutex mutex;
        ::rtl::OUString Name;
        ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XExtendedTransliteration > Body;
    };
    static TransBody lastTransBody;

    virtual sal_Bool SAL_CALL loadModuleByName( const rtl::OUString& implName,
        com::sun::star::uno::Reference<com::sun::star::i18n::XExtendedTransliteration> & body, const com::sun::star::lang::Locale& rLocale)
        throw(com::sun::star::uno::RuntimeException);

    void clear();

    void loadBody( ::rtl::OUString &implName,
        ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XExtendedTransliteration >& body )
        throw (::com::sun::star::uno::RuntimeException);

    com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getRange(
        const com::sun::star::uno::Sequence< rtl::OUString > &inStrs,
        sal_Int32 length, const sal_Int16 _nCascade)
        throw(com::sun::star::uno::RuntimeException);
};

} } } }

#endif
