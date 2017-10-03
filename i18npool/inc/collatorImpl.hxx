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
#ifndef INCLUDED_I18NPOOL_INC_COLLATORIMPL_HXX
#define INCLUDED_I18NPOOL_INC_COLLATORIMPL_HXX

#include <comphelper/processfactory.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/i18n/XLocaleData4.hpp>
#include <com/sun/star/i18n/XCollator.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <vector>

namespace i18npool {

//      ----------------------------------------------------
//      class CollatorImpl
//      ----------------------------------------------------
class CollatorImpl : public cppu::WeakImplHelper
<
    css::i18n::XCollator,
    css::lang::XServiceInfo
>
{
public:

    // Constructors
    CollatorImpl( const css::uno::Reference < css::uno::XComponentContext >& rxContext );
    // Destructor
    virtual ~CollatorImpl() override;

    virtual sal_Int32 SAL_CALL compareSubstring(const OUString& s1, sal_Int32 off1, sal_Int32 len1,
        const OUString& s2, sal_Int32 off2, sal_Int32 len2) override;

    virtual sal_Int32 SAL_CALL compareString( const OUString& s1,
        const OUString& s2) override;

    virtual sal_Int32 SAL_CALL loadDefaultCollator( const css::lang::Locale& rLocale,  sal_Int32 collatorOptions) override;

    virtual sal_Int32 SAL_CALL loadCollatorAlgorithm(  const OUString& impl, const css::lang::Locale& rLocale,
        sal_Int32 collatorOptions) override;

    virtual void SAL_CALL loadCollatorAlgorithmWithEndUserOption( const OUString& impl, const css::lang::Locale& rLocale,
        const css::uno::Sequence< sal_Int32 >& collatorOptions) override;

    virtual css::uno::Sequence< OUString > SAL_CALL listCollatorAlgorithms( const css::lang::Locale& rLocale ) override;

    virtual css::uno::Sequence< sal_Int32 > SAL_CALL listCollatorOptions( const OUString& collatorAlgorithmName ) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

protected:
    css::lang::Locale nLocale;
private:
    struct lookupTableItem {
        css::lang::Locale aLocale;
        OUString algorithm;
        OUString service;
        css::uno::Reference < XCollator > xC;
        lookupTableItem(const css::lang::Locale& rLocale, const OUString& _algorithm, const OUString& _service,
                        css::uno::Reference < XCollator > const & _xC) : aLocale(rLocale), algorithm(_algorithm), service(_service), xC(_xC) {}
        bool SAL_CALL equals(const css::lang::Locale& rLocale, const OUString& _algorithm) {
        return aLocale.Language == rLocale.Language &&
            aLocale.Country == rLocale.Country &&
            aLocale.Variant == rLocale.Variant &&
            algorithm == _algorithm;
        }
    };
    std::vector<lookupTableItem*>                       lookupTable;
    lookupTableItem *                                   cachedItem;

    // Service Factory
    css::uno::Reference < css::uno::XComponentContext > m_xContext;
    // lang::Locale Data
    css::uno::Reference < css::i18n::XLocaleData4 >     mxLocaleData;

    /// @throws css::uno::RuntimeException
    bool SAL_CALL createCollator(const css::lang::Locale& rLocale, const OUString& serviceName,
        const OUString& rSortAlgorithm);
    /// @throws css::uno::RuntimeException
    void SAL_CALL loadCachedCollator(const css::lang::Locale& rLocale, const OUString& rSortAlgorithm);
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
