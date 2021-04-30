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

#ifndef INCLUDED_EDITENG_UNOFIELD_HXX
#define INCLUDED_EDITENG_UNOFIELD_HXX

#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/component.hxx>
#include <editeng/editengdllapi.h>

#include <cppuhelper/basemutex.hxx>
#include <memory>

namespace com::sun::star::beans { class XPropertySetInfo; }

class SvxUnoFieldData_Impl;
class SfxItemPropertySet;
class SvxFieldData;

/// @throws css::uno::Exception
/// @throws css::uno::RuntimeException
css::uno::Reference< css::uno::XInterface > EDITENG_DLLPUBLIC SvxUnoTextCreateTextField(
    const OUString& ServiceSpecifier );

class EDITENG_DLLPUBLIC SvxUnoTextField final : public cppu::BaseMutex,
                        public ::cppu::OComponentHelper,
                        public css::text::XTextField,
                        public css::beans::XPropertySet,
                        public css::lang::XServiceInfo,
                        public css::lang::XUnoTunnel
{
    css::uno::Reference< css::text::XTextRange > mxAnchor;
    const SfxItemPropertySet*   mpPropSet;
    sal_Int32                   mnServiceId;
    std::unique_ptr<SvxUnoFieldData_Impl>        mpImpl;
    css::uno::Sequence< css::uno::Type > maTypeSequence;

public:
    SvxUnoTextField( sal_Int32 nServiceId ) noexcept;
    SvxUnoTextField( css::uno::Reference< css::text::XTextRange > const & xAnchor, const OUString& rPresentation, const SvxFieldData* pFieldData ) noexcept;
    virtual ~SvxUnoTextField() noexcept override;

    // Internal
    std::unique_ptr<SvxFieldData> CreateFieldData() const noexcept;

    static const css::uno::Sequence< sal_Int8 > & getUnoTunnelId() noexcept;
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;

    // css::uno::XInterface
    virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type & rType ) override;
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL acquire() noexcept override;
    virtual void SAL_CALL release() noexcept override;

    // css::lang::XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) override;

    // XTextField
    virtual OUString SAL_CALL getPresentation( sal_Bool bShowCommand ) override;

    // XTextContent
    virtual void SAL_CALL attach( const css::uno::Reference< css::text::XTextRange >& xTextRange ) override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL getAnchor(  ) override;

    // css::lang::XComponent
    virtual void SAL_CALL dispose() override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    // OComponentHelper
    virtual void SAL_CALL disposing() override;

    // XServiceInfo
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual OUString SAL_CALL getImplementationName() override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
