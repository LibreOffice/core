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
#pragma once

#include <com/sun/star/i18n/XCollator.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase.hxx>
#include <osl/module.h>

#include <unicode/tblcoll.h>

//      ----------------------------------------------------
//      class Collator_Unicode
//      ----------------------------------------------------

namespace i18npool {

class Collator_Unicode final : public cppu::WeakImplHelper < css::i18n::XCollator, css::lang::XServiceInfo >
{
public:
    // Constructors
    Collator_Unicode();
    // Destructor
    virtual ~Collator_Unicode() override;

    sal_Int32 SAL_CALL compareSubstring( const OUString& s1, sal_Int32 off1, sal_Int32 len1,
        const OUString& s2, sal_Int32 off2, sal_Int32 len2) override;

    sal_Int32 SAL_CALL compareString( const OUString& s1, const OUString& s2) override;

    sal_Int32 SAL_CALL loadCollatorAlgorithm( const OUString& impl, const css::lang::Locale& rLocale,
        sal_Int32 collatorOptions) override;


    // following 4 methods are implemented in collatorImpl.
    sal_Int32 SAL_CALL loadDefaultCollator( const css::lang::Locale&,  sal_Int32) override {throw css::uno::RuntimeException();}
    void SAL_CALL loadCollatorAlgorithmWithEndUserOption( const OUString&, const css::lang::Locale&,
        const css::uno::Sequence< sal_Int32 >&) override {throw css::uno::RuntimeException();}
    css::uno::Sequence< OUString > SAL_CALL listCollatorAlgorithms( const css::lang::Locale&) override {throw css::uno::RuntimeException();}
    css::uno::Sequence< sal_Int32 > SAL_CALL listCollatorOptions( const OUString& ) override {throw css::uno::RuntimeException();}

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

private:
    std::unique_ptr<icu::RuleBasedCollator> uca_base, collator;
};

}

extern "C" {
    // The generated functions have names that
    // start with get_collator_data_ to avoid clashing with a few
    // functions in the generated libindex_data that are called just
    // get_zh_pinyin for instance.

    const sal_uInt8* get_collator_data_ca_charset();
    const sal_uInt8* get_collator_data_cu_charset();
    const sal_uInt8* get_collator_data_dz_charset();
    const sal_uInt8* get_collator_data_hu_charset();
    const sal_uInt8* get_collator_data_ja_charset();
    const sal_uInt8* get_collator_data_ja_phonetic_alphanumeric_first();
    const sal_uInt8* get_collator_data_ja_phonetic_alphanumeric_last();
    const sal_uInt8* get_collator_data_ko_charset();
    const sal_uInt8* get_collator_data_ku_alphanumeric();
    const sal_uInt8* get_collator_data_ln_charset();
    const sal_uInt8* get_collator_data_my_dictionary();
    const sal_uInt8* get_collator_data_ne_charset();
    const sal_uInt8* get_collator_data_sid_charset();
    const sal_uInt8* get_collator_data_vro_alphanumeric();
    const sal_uInt8* get_collator_data_zh_TW_charset();
    const sal_uInt8* get_collator_data_zh_TW_radical();
    const sal_uInt8* get_collator_data_zh_TW_stroke();
    const sal_uInt8* get_collator_data_zh_charset();
    const sal_uInt8* get_collator_data_zh_pinyin();
    const sal_uInt8* get_collator_data_zh_radical();
    const sal_uInt8* get_collator_data_zh_stroke();
    const sal_uInt8* get_collator_data_zh_zhuyin();

    size_t get_collator_data_ca_charset_length();
    size_t get_collator_data_cu_charset_length();
    size_t get_collator_data_dz_charset_length();
    size_t get_collator_data_hu_charset_length();
    size_t get_collator_data_ja_charset_length();
    size_t get_collator_data_ja_phonetic_alphanumeric_first_length();
    size_t get_collator_data_ja_phonetic_alphanumeric_last_length();
    size_t get_collator_data_ko_charset_length();
    size_t get_collator_data_ku_alphanumeric_length();
    size_t get_collator_data_ln_charset_length();
    size_t get_collator_data_my_dictionary_length();
    size_t get_collator_data_ne_charset_length();
    size_t get_collator_data_sid_charset_length();
    size_t get_collator_data_vro_alphanumeric_length();
    size_t get_collator_data_zh_TW_charset_length();
    size_t get_collator_data_zh_TW_radical_length();
    size_t get_collator_data_zh_TW_stroke_length();
    size_t get_collator_data_zh_charset_length();
    size_t get_collator_data_zh_pinyin_length();
    size_t get_collator_data_zh_radical_length();
    size_t get_collator_data_zh_stroke_length();
    size_t get_collator_data_zh_zhuyin_length();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
