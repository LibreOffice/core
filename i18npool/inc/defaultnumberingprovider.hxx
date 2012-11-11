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
#ifndef _I18N_DEFAULT_NUMBERING_PROVIDER_HXX_
#define _I18N_DEFAULT_NUMBERING_PROVIDER_HXX_

#include <com/sun/star/text/XDefaultNumberingProvider.hpp>
#include <com/sun/star/text/XNumberingFormatter.hpp>
#include <com/sun/star/text/XNumberingTypeInfo.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase4.hxx>
#include <com/sun/star/i18n/XTransliteration.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>

#include <transliterationImpl.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

class DefaultNumberingProvider : public cppu::WeakImplHelper4
<
    com::sun::star::text::XDefaultNumberingProvider,
    com::sun::star::text::XNumberingFormatter,
    com::sun::star::text::XNumberingTypeInfo,
    com::sun::star::lang::XServiceInfo
>
{
    void GetCharStrN( sal_Int32 nValue, sal_Int16 nType, rtl::OUString& rStr ) const;
    void GetCharStr( sal_Int32 nValue, sal_Int16 nType, rtl::OUString& rStr ) const;
    void GetRomanString( sal_Int32 nValue, sal_Int16 nType, rtl::OUString& rStr ) const;
    void impl_loadTranslit();
public:
    DefaultNumberingProvider(
        const com::sun::star::uno::Reference < com::sun::star::uno::XComponentContext >& rxContext );
    ~DefaultNumberingProvider();

    //XDefaultNumberingProvider
    virtual com::sun::star::uno::Sequence< com::sun::star::uno::Reference<
        com::sun::star::container::XIndexAccess > > SAL_CALL
        getDefaultOutlineNumberings( const com::sun::star::lang::Locale& aLocale )
        throw(com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Sequence< com::sun::star::uno::Sequence<
        com::sun::star::beans::PropertyValue > > SAL_CALL
        getDefaultContinuousNumberingLevels( const com::sun::star::lang::Locale& aLocale )
        throw(com::sun::star::uno::RuntimeException);

    //XNumberingFormatter
    virtual rtl::OUString SAL_CALL makeNumberingString(
        const com::sun::star::uno::Sequence<
        com::sun::star::beans::PropertyValue >& aProperties,
        const com::sun::star::lang::Locale& aLocale )
        throw(com::sun::star::lang::IllegalArgumentException,
        com::sun::star::uno::RuntimeException);

    //XNumberingTypeInfo
    virtual com::sun::star::uno::Sequence< sal_Int16 > SAL_CALL getSupportedNumberingTypes(  )
        throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getNumberingType( const rtl::OUString& NumberingIdentifier )
        throw(com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasNumberingType( const rtl::OUString& NumberingIdentifier )
        throw(com::sun::star::uno::RuntimeException);
    virtual rtl::OUString SAL_CALL getNumberingIdentifier( sal_Int16 NumberingType )
        throw(com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void)
                throw( com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName)
                throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void)
                throw( com::sun::star::uno::RuntimeException );
private:
    com::sun::star::uno::Reference < com::sun::star::uno::XComponentContext > m_xContext;
    com::sun::star::uno::Reference < com::sun::star::container::XHierarchicalNameAccess > xHierarchicalNameAccess;
    TransliterationImpl* translit;
    rtl::OUString SAL_CALL makeNumberingIdentifier( sal_Int16 index )
        throw(com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL isScriptFlagEnabled(const rtl::OUString& aName )
        throw(com::sun::star::uno::RuntimeException);
};
} } } }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
