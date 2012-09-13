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
#ifndef _I18N_COLLATORIMPL_HXX_
#define _I18N_COLLATORIMPL_HXX_

#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/i18n/XLocaleData4.hpp>
#include <com/sun/star/i18n/XCollator.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <sal/alloca.h>
#include <vector>

namespace com { namespace sun { namespace star { namespace i18n {
//      ----------------------------------------------------
//      class CollatorImpl
//      ----------------------------------------------------
class CollatorImpl : public cppu::WeakImplHelper2
<
    XCollator,
    com::sun::star::lang::XServiceInfo
>
{
public:

    // Constructors
    CollatorImpl( const com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory >& rxMSF );
    // Destructor
    ~CollatorImpl();

    virtual sal_Int32 SAL_CALL compareSubstring(const rtl::OUString& s1, sal_Int32 off1, sal_Int32 len1,
        const rtl::OUString& s2, sal_Int32 off2, sal_Int32 len2) throw(com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL compareString( const rtl::OUString& s1,
        const rtl::OUString& s2) throw(com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL loadDefaultCollator( const lang::Locale& rLocale,  sal_Int32 collatorOptions)
        throw(com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL loadCollatorAlgorithm(  const rtl::OUString& impl, const lang::Locale& rLocale,
        sal_Int32 collatorOptions) throw(com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL loadCollatorAlgorithmWithEndUserOption( const rtl::OUString& impl, const lang::Locale& rLocale,
        const com::sun::star::uno::Sequence< sal_Int32 >& collatorOptions) throw(com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL listCollatorAlgorithms( const lang::Locale& rLocale )
        throw(com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL listCollatorOptions( const rtl::OUString& collatorAlgorithmName )
        throw(com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName() throw( com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames() throw( com::sun::star::uno::RuntimeException );

protected:
    lang::Locale nLocale;
private :
    struct lookupTableItem {
        lang::Locale aLocale;
        rtl::OUString algorithm;
        rtl::OUString service;
        com::sun::star::uno::Reference < XCollator > xC;
        lookupTableItem(const lang::Locale& rLocale, const rtl::OUString& _algorithm, const rtl::OUString& _service,
        com::sun::star::uno::Reference < XCollator >& _xC) : aLocale(rLocale), algorithm(_algorithm), service(_service), xC(_xC) {}
        sal_Bool SAL_CALL equals(const lang::Locale& rLocale, const rtl::OUString& _algorithm) {
        return aLocale.Language == rLocale.Language &&
            aLocale.Country == rLocale.Country &&
            aLocale.Variant == rLocale.Variant &&
            algorithm == _algorithm;
        }
    };
    std::vector<lookupTableItem*> lookupTable;
    lookupTableItem *cachedItem;

    // Service Factory
    com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory > xMSF;
    // lang::Locale Data
    com::sun::star::uno::Reference < XLocaleData4 > mxLocaleData;

    sal_Bool SAL_CALL createCollator(const lang::Locale& rLocale, const rtl::OUString& serviceName,
        const rtl::OUString& rSortAlgorithm) throw(com::sun::star::uno::RuntimeException);
    void SAL_CALL loadCachedCollator(const lang::Locale& rLocale, const rtl::OUString& rSortAlgorithm)
        throw(com::sun::star::uno::RuntimeException);
};

} } } }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
