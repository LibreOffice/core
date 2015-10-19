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

namespace com { namespace sun { namespace star { namespace i18n {
//      ----------------------------------------------------
//      class CollatorImpl
//      ----------------------------------------------------
class CollatorImpl : public cppu::WeakImplHelper
<
    XCollator,
    css::lang::XServiceInfo
>
{
public:

    // Constructors
    CollatorImpl( const css::uno::Reference < css::uno::XComponentContext >& rxContext );
    // Destructor
    virtual ~CollatorImpl();

    virtual sal_Int32 SAL_CALL compareSubstring(const OUString& s1, sal_Int32 off1, sal_Int32 len1,
        const OUString& s2, sal_Int32 off2, sal_Int32 len2) throw(css::uno::RuntimeException, std::exception) override;

    virtual sal_Int32 SAL_CALL compareString( const OUString& s1,
        const OUString& s2) throw(css::uno::RuntimeException, std::exception) override;

    virtual sal_Int32 SAL_CALL loadDefaultCollator( const lang::Locale& rLocale,  sal_Int32 collatorOptions)
        throw(css::uno::RuntimeException, std::exception) override;

    virtual sal_Int32 SAL_CALL loadCollatorAlgorithm(  const OUString& impl, const lang::Locale& rLocale,
        sal_Int32 collatorOptions) throw(css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL loadCollatorAlgorithmWithEndUserOption( const OUString& impl, const lang::Locale& rLocale,
        const css::uno::Sequence< sal_Int32 >& collatorOptions) throw(css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< OUString > SAL_CALL listCollatorAlgorithms( const lang::Locale& rLocale )
        throw(css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< sal_Int32 > SAL_CALL listCollatorOptions( const OUString& collatorAlgorithmName )
        throw(css::uno::RuntimeException, std::exception) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

protected:
    lang::Locale nLocale;
private:
    struct lookupTableItem {
        lang::Locale aLocale;
        OUString algorithm;
        OUString service;
        css::uno::Reference < XCollator > xC;
        lookupTableItem(const lang::Locale& rLocale, const OUString& _algorithm, const OUString& _service,
        css::uno::Reference < XCollator >& _xC) : aLocale(rLocale), algorithm(_algorithm), service(_service), xC(_xC) {}
        bool SAL_CALL equals(const lang::Locale& rLocale, const OUString& _algorithm) {
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
    css::uno::Reference < XLocaleData4 >                mxLocaleData;

    bool SAL_CALL createCollator(const lang::Locale& rLocale, const OUString& serviceName,
        const OUString& rSortAlgorithm) throw(css::uno::RuntimeException);
    void SAL_CALL loadCachedCollator(const lang::Locale& rLocale, const OUString& rSortAlgorithm)
        throw(css::uno::RuntimeException);
};

} } } }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
