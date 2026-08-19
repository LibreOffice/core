/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#pragma once

#include <com/sun/star/i18n/XExtendedTransliteration.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase.hxx>
#include <rtl/ustring.hxx>

namespace i18npool {

class transliteration_commonclass : public cppu::WeakImplHelper<
                                                                  css::i18n::XExtendedTransliteration,
                                                                  css::lang::XServiceInfo
                                                                >
{
public:
        transliteration_commonclass();

        // Methods which are shared.
        void
        loadModule( css::i18n::TransliterationModules modName, const css::lang::Locale& rLocale ) override;

        void
        loadModuleNew( const cpo::uno::Sequence< css::i18n::TransliterationModulesNew >& modName, const css::lang::Locale& rLocale ) override;

        void
        loadModuleByImplName( const OUString& implName, const css::lang::Locale& rLocale ) override;

        void
        loadModulesByImplNames(const cpo::uno::Sequence< OUString >& modNamelist, const css::lang::Locale& rLocale) override;

        cpo::uno::Sequence< OUString >
        getAvailableModules( const css::lang::Locale& rLocale, sal_Int16 sType ) override;

        // Methods which should be implemented in each transliteration module.
        virtual OUString getName() override;

        virtual sal_Int16 getType(  ) override = 0;

        virtual OUString
        transliterate( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, cpo::uno::Sequence< sal_Int32 >& offset ) override final
            { return transliterateImpl( inStr, startPos, nCount, &offset ); }

        virtual OUString
        folding( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, cpo::uno::Sequence< sal_Int32 >& offset) override final
            { return foldingImpl( inStr, startPos, nCount, &offset ); }

        // Methods in XExtendedTransliteration
        virtual OUString
        transliterateString2String( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount ) override;
        virtual OUString
        transliterateChar2String( sal_Unicode inChar) override;
        virtual sal_Unicode
        transliterateChar2Char( sal_Unicode inChar ) override = 0;

        virtual bool
        equals( const OUString& str1, sal_Int32 pos1, sal_Int32 nCount1, sal_Int32& nMatch1, const OUString& str2, sal_Int32 pos2, sal_Int32 nCount2, sal_Int32& nMatch2 ) override = 0;

        virtual cpo::uno::Sequence< OUString >
        transliterateRange( const OUString& str1, const OUString& str2 ) override = 0;

        virtual sal_Int32
        compareSubstring( const OUString& s1, sal_Int32 off1, sal_Int32 len1, const OUString& s2, sal_Int32 off2, sal_Int32 len2) override;

        virtual sal_Int32
        compareString( const OUString& s1, const OUString& s2) override;

        //XServiceInfo
        virtual OUString getImplementationName() override;
        virtual bool supportsService(const OUString& ServiceName) override;
        virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;
protected:
        virtual OUString
        transliterateImpl( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, cpo::uno::Sequence< sal_Int32 >* pOffset ) = 0;

        virtual OUString
        foldingImpl( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, cpo::uno::Sequence< sal_Int32 >* pOffset ) = 0;

        css::lang::Locale   aLocale;
        const char*         transliterationName;
        const char*         implementationName;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
