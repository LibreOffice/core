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
#include <com/sun/star/i18n/XTransliteration.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>

#include <transliterationImpl.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

class DefaultNumberingProvider : public cppu::WeakImplHelper
<
    css::text::XDefaultNumberingProvider,
    css::text::XNumberingFormatter,
    css::text::XNumberingTypeInfo,
    css::lang::XServiceInfo
>
{
    void impl_loadTranslit();
public:
    DefaultNumberingProvider(
        const css::uno::Reference < css::uno::XComponentContext >& rxContext );
    virtual ~DefaultNumberingProvider();

    //XDefaultNumberingProvider
    virtual css::uno::Sequence< css::uno::Reference<
        css::container::XIndexAccess > > SAL_CALL
        getDefaultOutlineNumberings( const css::lang::Locale& aLocale )
        throw(css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< css::uno::Sequence<
        css::beans::PropertyValue > > SAL_CALL
        getDefaultContinuousNumberingLevels( const css::lang::Locale& aLocale )
        throw(css::uno::RuntimeException, std::exception) override;

    //XNumberingFormatter
    virtual OUString SAL_CALL makeNumberingString(
        const css::uno::Sequence<
        css::beans::PropertyValue >& aProperties,
        const css::lang::Locale& aLocale )
        throw(css::lang::IllegalArgumentException,
        css::uno::RuntimeException, std::exception) override;

    //XNumberingTypeInfo
    virtual css::uno::Sequence< sal_Int16 > SAL_CALL getSupportedNumberingTypes(  )
        throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getNumberingType( const OUString& NumberingIdentifier )
        throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasNumberingType( const OUString& NumberingIdentifier )
        throw(css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getNumberingIdentifier( sal_Int16 NumberingType )
        throw(css::uno::RuntimeException, std::exception) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName)
                throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                throw( css::uno::RuntimeException, std::exception ) override;
private:
    css::uno::Reference < css::uno::XComponentContext > m_xContext;
    css::uno::Reference < css::container::XHierarchicalNameAccess > xHierarchicalNameAccess;
    TransliterationImpl* translit;
    OUString SAL_CALL makeNumberingIdentifier( sal_Int16 index )
        throw(css::uno::RuntimeException, std::exception);
    bool SAL_CALL isScriptFlagEnabled(const OUString& aName )
        throw(css::uno::RuntimeException);
};
} } } }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
