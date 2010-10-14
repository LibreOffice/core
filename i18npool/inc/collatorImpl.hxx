/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _I18N_COLLATORIMPL_HXX_
#define _I18N_COLLATORIMPL_HXX_

#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/i18n/XLocaleData.hpp>
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
    com::sun::star::uno::Reference < XLocaleData > localedata;

    sal_Bool SAL_CALL createCollator(const lang::Locale& rLocale, const rtl::OUString& serviceName,
        const rtl::OUString& rSortAlgorithm) throw(com::sun::star::uno::RuntimeException);
    void SAL_CALL loadCachedCollator(const lang::Locale& rLocale, const rtl::OUString& rSortAlgorithm)
        throw(com::sun::star::uno::RuntimeException);
};

} } } }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
