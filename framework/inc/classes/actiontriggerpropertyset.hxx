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

#pragma once

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/weak.hxx>
#include <rtl/ustring.hxx>

#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>

#define SERVICENAME_ACTIONTRIGGER "com.sun.star.ui.ActionTrigger"
#define IMPLEMENTATIONNAME_ACTIONTRIGGER "com.sun.star.comp.ui.ActionTrigger"

namespace framework
{

class ActionTriggerPropertySet final : private cppu::BaseMutex,
                                    public css::lang::XServiceInfo ,
                                    public css::lang::XTypeProvider,
                                    public ::cppu::OBroadcastHelper             ,
                                    public ::cppu::OPropertySetHelper           ,   // -> XPropertySet, XFastPropertySet, XMultiPropertySet
                                    public ::cppu::OWeakObject
{
    public:
        ActionTriggerPropertySet();
        virtual ~ActionTriggerPropertySet() override;

        // XInterface
        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) override;
        virtual void SAL_CALL acquire() noexcept override;
        virtual void SAL_CALL release() noexcept override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

        // XTypeProvider
        virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) override;

    private:

        //  OPropertySetHelper

        virtual sal_Bool SAL_CALL convertFastPropertyValue( css::uno::Any&       aConvertedValue,
                                                            css::uno::Any&       aOldValue,
                                                            sal_Int32                       nHandle,
                                                            const css::uno::Any& aValue          ) override;

        virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const css::uno::Any& aValue ) override;

        using cppu::OPropertySetHelper::getFastPropertyValue;
        virtual void SAL_CALL getFastPropertyValue( css::uno::Any& aValue, sal_Int32 nHandle ) const override;

        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;

        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() override;

        static css::uno::Sequence< css::beans::Property > impl_getStaticPropertyDescriptor();

        //  helper

        /// @throws css::lang::IllegalArgumentException
        bool impl_tryToChangeProperty(  const   OUString&            aCurrentValue   ,
                                        const   css::uno::Any&       aNewValue       ,
                                            css::uno::Any&           aOldValue       ,
                                            css::uno::Any&           aConvertedValue );

        /// @throws css::lang::IllegalArgumentException
        bool impl_tryToChangeProperty(  const   css::uno::Reference< css::awt::XBitmap >&  xBitmap,
                                        const   css::uno::Any&   aNewValue       ,
                                            css::uno::Any&           aOldValue       ,
                                            css::uno::Any&           aConvertedValue );

        /// @throws css::lang::IllegalArgumentException
        bool impl_tryToChangeProperty(  const   css::uno::Reference< css::uno::XInterface >& xInterface,
                                        const   css::uno::Any&   aNewValue       ,
                                            css::uno::Any&           aOldValue       ,
                                            css::uno::Any&           aConvertedValue );

        //  members

        OUString                                    m_aCommandURL;
        OUString                                    m_aHelpURL;
        OUString                                    m_aText;
        css::uno::Reference< css::awt::XBitmap >    m_xBitmap;
        css::uno::Reference< css::uno::XInterface > m_xActionTriggerContainer;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
