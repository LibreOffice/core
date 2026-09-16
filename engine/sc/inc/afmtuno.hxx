/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <rtl/ref.hxx>
#include <svl/itemprop.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <cppuhelper/implbase.hxx>

#include <climits>

class ScAutoFormatFieldObj;
class ScAutoFormatObj;

#define SC_AFMTOBJ_INVALID  USHRT_MAX

class ScAutoFormatsObj final : public ::cppu::WeakImplHelper<
                            css::container::XNameContainer,
                            css::container::XEnumerationAccess,
                            css::container::XIndexAccess,
                            css::lang::XServiceInfo >
{
private:
    static rtl::Reference<ScAutoFormatObj> GetObjectByIndex_Impl(sal_uInt16 nIndex);
    static rtl::Reference<ScAutoFormatObj> GetObjectByName_Impl(std::u16string_view aName);

public:
                            ScAutoFormatsObj();
    virtual                 ~ScAutoFormatsObj() override;

                            // XNameContainer
    virtual void   insertByName( const OUString& aName,
                                const cpo::uno::Any& aElement ) override;
    virtual void   removeByName( const OUString& Name ) override;

                            // XNameReplace
    virtual void   replaceByName( const OUString& aName,
                                const cpo::uno::Any& aElement ) override;

                            // XNameAccess
    virtual cpo::uno::Any getByName( const OUString& aName ) override;
    virtual cpo::uno::Sequence< OUString > getElementNames() override;
    virtual bool hasByName( const OUString& aName ) override;

                            // XElementAccess
    virtual cpo::uno::Type getElementType() override;
    virtual bool hasElements() override;

                            // XEnumerationAccess
    virtual cpo::uno::Reference< css::container::XEnumeration >
                            createEnumeration() override;

                            // XIndexAccess
    virtual sal_Int32 getCount() override;
    virtual cpo::uno::Any getByIndex( sal_Int32 Index ) override;

                            // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;
};

class ScAutoFormatObj final : public ::cppu::WeakImplHelper<
                            css::container::XIndexAccess,
                            css::container::XEnumerationAccess,
                            css::container::XNamed,
                            css::beans::XPropertySet,
                            css::lang::XServiceInfo >
{
private:
    SfxItemPropertySet      aPropSet;
    sal_uInt16              nFormatIndex;

    rtl::Reference<ScAutoFormatFieldObj> GetObjectByIndex_Impl(sal_uInt16 nIndex);

public:
                            ScAutoFormatObj(sal_uInt16 nIndex);
    virtual                 ~ScAutoFormatObj() override;

                            // called via getImplementation:
    bool                    IsInserted() const      { return nFormatIndex != SC_AFMTOBJ_INVALID; }
    void                    InitFormat( sal_uInt16 nNewIndex );

                            // XIndexAccess
    virtual sal_Int32 getCount() override;
    virtual cpo::uno::Any getByIndex( sal_Int32 Index ) override;

                            // XElementAccess
    virtual cpo::uno::Type getElementType() override;
    virtual bool hasElements() override;

                            // XEnumerationAccess
    virtual cpo::uno::Reference< css::container::XEnumeration >
                            createEnumeration() override;

                            // XNamed
    virtual OUString getName() override;
    virtual void   setName( const OUString& aName ) override;

                            // XPropertySet
    virtual cpo::uno::Reference< css::beans::XPropertySetInfo >
                            getPropertySetInfo() override;
    virtual void   setPropertyValue( const OUString& aPropertyName,
                                    const cpo::uno::Any& aValue ) override;
    virtual cpo::uno::Any getPropertyValue(
                                    const OUString& PropertyName ) override;
    virtual void   addPropertyChangeListener( const OUString& aPropertyName,
                                    const cpo::uno::Reference<
                                        css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void   removePropertyChangeListener( const OUString& aPropertyName,
                                    const cpo::uno::Reference<
                                        css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void   addVetoableChangeListener( const OUString& PropertyName,
                                    const cpo::uno::Reference<
                                        css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void   removeVetoableChangeListener( const OUString& PropertyName,
                                    const cpo::uno::Reference<
                                        css::beans::XVetoableChangeListener >& aListener ) override;

                            // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;
};

class ScAutoFormatFieldObj final : public ::cppu::WeakImplHelper<
                                css::beans::XPropertySet,
                                css::lang::XServiceInfo >
{
private:
    SfxItemPropertySet      aPropSet;
    sal_uInt16              nFormatIndex;
    sal_uInt16              nFieldIndex;

public:
                            ScAutoFormatFieldObj(sal_uInt16 nFormat, sal_uInt16 nField);
    virtual                 ~ScAutoFormatFieldObj() override;

                            // XPropertySet
    virtual cpo::uno::Reference< css::beans::XPropertySetInfo >
                            getPropertySetInfo() override;
    virtual void   setPropertyValue( const OUString& aPropertyName,
                                    const cpo::uno::Any& aValue ) override;
    virtual cpo::uno::Any getPropertyValue(
                                    const OUString& PropertyName ) override;
    virtual void   addPropertyChangeListener( const OUString& aPropertyName,
                                    const cpo::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void   removePropertyChangeListener( const OUString& aPropertyName,
                                    const cpo::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void   addVetoableChangeListener( const OUString& PropertyName,
                                    const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void   removeVetoableChangeListener( const OUString& PropertyName,
                                    const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

                            // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
