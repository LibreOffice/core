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
#ifndef INCLUDED_I18NPOOL_INC_DEFAULTNUMBERINGPROVIDER_HXX
#define INCLUDED_I18NPOOL_INC_DEFAULTNUMBERINGPROVIDER_HXX

#include <com/sun/star/text/XDefaultNumberingProvider.hpp>
#include <com/sun/star/text/XNumberingFormatter.hpp>
#include <com/sun/star/text/XNumberingTypeInfo.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase.hxx>
#include <rtl/ref.hxx>
#include <map>

namespace com::sun::star::container { class XHierarchicalNameAccess; }
namespace com::sun::star::uno { class XComponentContext; }
namespace i18npool {
    class TransliterationImpl;
    class NativeNumberSupplierService;
    struct Supported_NumberingType;
}

namespace i18npool {

class DefaultNumberingProvider final : public cppu::WeakImplHelper
<
    css::text::XDefaultNumberingProvider,
    css::text::XNumberingFormatter,
    css::text::XNumberingTypeInfo,
    css::lang::XServiceInfo
>
{
public:
    DefaultNumberingProvider(
        const css::uno::Reference < css::uno::XComponentContext >& rxContext );
    virtual ~DefaultNumberingProvider() override;

    //XDefaultNumberingProvider
    virtual css::uno::Sequence< css::uno::Reference<
        css::container::XIndexAccess > > SAL_CALL
        getDefaultOutlineNumberings( const css::lang::Locale& aLocale ) override;

    virtual css::uno::Sequence< css::uno::Sequence<
        css::beans::PropertyValue > > SAL_CALL
        getDefaultContinuousNumberingLevels( const css::lang::Locale& aLocale ) override;

    //XNumberingFormatter
    virtual OUString SAL_CALL makeNumberingString(
        const css::uno::Sequence<
        css::beans::PropertyValue >& aProperties,
        const css::lang::Locale& aLocale ) override;

    //XNumberingTypeInfo
    virtual css::uno::Sequence< sal_Int16 > SAL_CALL getSupportedNumberingTypes(  ) override;
    virtual sal_Int16 SAL_CALL getNumberingType( const OUString& NumberingIdentifier ) override;
    virtual sal_Bool SAL_CALL hasNumberingType( const OUString& NumberingIdentifier ) override;
    virtual OUString SAL_CALL getNumberingIdentifier( sal_Int16 NumberingType ) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
private:
    css::uno::Reference < css::uno::XComponentContext > m_xContext;
    css::uno::Reference < css::container::XHierarchicalNameAccess > xHierarchicalNameAccess;
    rtl::Reference<TransliterationImpl> translit;
    rtl::Reference<NativeNumberSupplierService> mxNatNum;
    std::map<OUString, const Supported_NumberingType*> maSupportedTypesCache;

    /// @throws css::uno::RuntimeException
    OUString makeNumberingIdentifier( sal_Int16 index );
    /// @throws css::uno::RuntimeException
    bool isScriptFlagEnabled(const OUString& aName );
};

}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
