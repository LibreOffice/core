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
#ifndef INCLUDED_I18NPOOL_INC_TRANSLITERATIONIMPL_HXX
#define INCLUDED_I18NPOOL_INC_TRANSLITERATIONIMPL_HXX

#include <com/sun/star/i18n/XLocaleData4.hpp>
#include <com/sun/star/i18n/XExtendedTransliteration.hpp>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <sal/types.h>

namespace com { namespace sun { namespace star { namespace i18n {

#if OSL_DEBUG_LEVEL > 1
#define RETURN_DEBUG_STRING ("return from transliterationImpl");
#define RETURN_DEBUG_NUMBER 'TRAN'
#else
#define RETURN_DEBUG_STRING
#define RETURN_DEBUG_NUMBER
#endif

class TransliterationImpl : public cppu::WeakImplHelper
<
    css::i18n::XExtendedTransliteration,
    css::lang::XServiceInfo
>
{
public:
    // Constructors
    TransliterationImpl(const css::uno::Reference < css::uno::XComponentContext >& xContext);
    // Destructor
    virtual ~TransliterationImpl();

    // Methods
    virtual OUString SAL_CALL getName(  ) throw(css::uno::RuntimeException, std::exception) override ;
    virtual sal_Int16 SAL_CALL getType(  ) throw(css::uno::RuntimeException, std::exception) override ;

    virtual void SAL_CALL loadModule( TransliterationModules modName, const css::lang::Locale& rLocale )
        throw(css::uno::RuntimeException, std::exception) override ;
    virtual void SAL_CALL loadModuleNew( const css::uno::Sequence< TransliterationModulesNew >& modName,
        const css::lang::Locale& rLocale ) throw(css::uno::RuntimeException, std::exception) override ;
    virtual void SAL_CALL loadModuleByImplName( const OUString& implName,
        const css::lang::Locale& rLocale ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL loadModulesByImplNames(const css::uno::Sequence< OUString >& modNamelist,
        const css::lang::Locale& rLocale) throw(css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< OUString > SAL_CALL getAvailableModules(
        const css::lang::Locale& rLocale, sal_Int16 sType )
        throw(css::uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL transliterate( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
        css::uno::Sequence< sal_Int32 >& offset  ) throw(css::uno::RuntimeException, std::exception) override ;

    virtual OUString SAL_CALL folding( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
        css::uno::Sequence< sal_Int32 >& offset ) throw(css::uno::RuntimeException, std::exception) override;

    // Methods in XExtendedTransliteration
    virtual OUString SAL_CALL transliterateString2String( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount )
        throw(css::uno::RuntimeException, std::exception) override ;
    virtual OUString SAL_CALL transliterateChar2String( sal_Unicode inChar )
        throw(css::uno::RuntimeException, std::exception) override ;
    virtual sal_Unicode SAL_CALL transliterateChar2Char( sal_Unicode inChar )
        throw(css::i18n::MultipleCharsOutputException,
                css::uno::RuntimeException, std::exception) override ;

    virtual sal_Bool SAL_CALL equals( const OUString& str1, sal_Int32 pos1, sal_Int32 nCount1,
        sal_Int32& nMatch1, const OUString& str2, sal_Int32 pos2, sal_Int32 nCount2, sal_Int32& nMatch2 )
        throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL transliterateRange( const OUString& str1,
        const OUString& str2 ) throw(css::uno::RuntimeException, std::exception) override ;

    virtual sal_Int32 SAL_CALL compareSubstring( const OUString& s1, sal_Int32 off1, sal_Int32 len1,
        const OUString& s2, sal_Int32 off2, sal_Int32 len2) throw(css::uno::RuntimeException, std::exception) override;

    virtual sal_Int32 SAL_CALL compareString( const OUString& s1, const OUString& s2)
        throw(css::uno::RuntimeException, std::exception) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName)
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw( css::uno::RuntimeException, std::exception ) override;

private:
#define maxCascade 27
    css::uno::Reference< css::i18n::XExtendedTransliteration > bodyCascade[maxCascade];
    sal_Int16 numCascade;
    bool caseignoreOnly;
    css::uno::Reference< css::uno::XComponentContext > mxContext;
    css::uno::Reference< XLocaleData4 > mxLocaledata;
    css::uno::Reference< css::i18n::XExtendedTransliteration > caseignore;

    bool SAL_CALL loadModuleByName( const OUString& implName,
        css::uno::Reference<css::i18n::XExtendedTransliteration> & body, const css::lang::Locale& rLocale)
        throw(css::uno::RuntimeException);

    void clear();

    void loadBody( OUString &implName,
        css::uno::Reference< css::i18n::XExtendedTransliteration >& body )
        throw (css::uno::RuntimeException);

    css::uno::Sequence< OUString > SAL_CALL getRange(
        const css::uno::Sequence< OUString > &inStrs,
        sal_Int32 length, const sal_Int16 _nCascade)
        throw(css::uno::RuntimeException);
};

} } } }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
