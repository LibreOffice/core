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
#ifndef INCLUDED_I18NPOOL_INC_COLLATOR_UNICODE_HXX
#define INCLUDED_I18NPOOL_INC_COLLATOR_UNICODE_HXX

#include <com/sun/star/i18n/XCollator.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase.hxx>
#include <osl/module.h>

#include <unicode/tblcoll.h>

//      ----------------------------------------------------
//      class Collator_Unicode
//      ----------------------------------------------------

namespace i18npool {

class Collator_Unicode : public cppu::WeakImplHelper < css::i18n::XCollator, css::lang::XServiceInfo >
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

protected:
    const sal_Char *implementationName;
private:
    RuleBasedCollator *uca_base, *collator;
#ifndef DISABLE_DYNLOADING
    oslModule hModule;
#endif
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
