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
#ifndef INCLUDED_I18NPOOL_INC_TRANSLITERATION_COMMONCLASS_HXX
#define INCLUDED_I18NPOOL_INC_TRANSLITERATION_COMMONCLASS_HXX

#include <com/sun/star/i18n/XExtendedTransliteration.hpp>
#include <com/sun/star/i18n/TransliterationType.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase2.hxx>
#include <rtl/ustrbuf.h>
#include <rtl/ustring.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

class transliteration_commonclass : public cppu::WeakImplHelper2<
                                                                  com::sun::star::i18n::XExtendedTransliteration,
                                                                  com::sun::star::lang::XServiceInfo
                                                                >
{
public:
        transliteration_commonclass();

        // Methods which are shared.
        void SAL_CALL
        loadModule( TransliterationModules modName, const com::sun::star::lang::Locale& rLocale )
        throw(com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        void SAL_CALL
        loadModuleNew( const com::sun::star::uno::Sequence< TransliterationModulesNew >& modName, const com::sun::star::lang::Locale& rLocale )
        throw(com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        void SAL_CALL
        loadModuleByImplName( const OUString& implName, const com::sun::star::lang::Locale& rLocale )
        throw(com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        void SAL_CALL
        loadModulesByImplNames(const com::sun::star::uno::Sequence< OUString >& modNamelist, const com::sun::star::lang::Locale& rLocale)
        throw(com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        com::sun::star::uno::Sequence< OUString > SAL_CALL
        getAvailableModules( const com::sun::star::lang::Locale& rLocale, sal_Int16 sType )
        throw(com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // Methods which should be implemented in each transliteration module.
        virtual OUString SAL_CALL getName() throw(com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual sal_Int16 SAL_CALL getType(  ) throw(com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE = 0;

        virtual OUString SAL_CALL
        transliterate( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, com::sun::star::uno::Sequence< sal_Int32 >& offset  )
        throw(com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE = 0;

        virtual OUString SAL_CALL
        folding( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, com::sun::star::uno::Sequence< sal_Int32 >& offset)
        throw(com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE = 0;

        // Methods in XExtendedTransliteration
        virtual OUString SAL_CALL
        transliterateString2String( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount )
        throw(com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual OUString SAL_CALL
        transliterateChar2String( sal_Unicode inChar)
        throw(com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Unicode SAL_CALL
        transliterateChar2Char( sal_Unicode inChar )
        throw(com::sun::star::i18n::MultipleCharsOutputException,
                com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE = 0;

        virtual sal_Bool SAL_CALL
        equals( const OUString& str1, sal_Int32 pos1, sal_Int32 nCount1, sal_Int32& nMatch1, const OUString& str2, sal_Int32 pos2, sal_Int32 nCount2, sal_Int32& nMatch2 )
        throw(com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE = 0;

        virtual com::sun::star::uno::Sequence< OUString > SAL_CALL
        transliterateRange( const OUString& str1, const OUString& str2 )
        throw(com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE = 0;

        virtual sal_Int32 SAL_CALL
        compareSubstring( const OUString& s1, sal_Int32 off1, sal_Int32 len1, const OUString& s2, sal_Int32 off2, sal_Int32 len2)
        throw(com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual sal_Int32 SAL_CALL
        compareString( const OUString& s1, const OUString& s2)
        throw(com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        //XServiceInfo
        virtual OUString SAL_CALL getImplementationName()
            throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName)
            throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
        virtual com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
            throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
protected:
        com::sun::star::lang::Locale   aLocale;
        const sal_Char* transliterationName;
        const sal_Char* implementationName;
        bool useOffset;
};

} } } }

#endif // INCLUDED_I18NPOOL_INC_TRANSLITERATION_COMMONCLASS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
