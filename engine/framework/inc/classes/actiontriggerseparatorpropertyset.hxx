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

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/weak.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>

inline constexpr OUString SERVICENAME_ACTIONTRIGGERSEPARATOR = u"com.sun.star.ui.ActionTriggerSeparator"_ustr;
inline constexpr OUString IMPLEMENTATIONNAME_ACTIONTRIGGERSEPARATOR = u"com.sun.star.comp.ui.ActionTriggerSeparator"_ustr;

namespace framework
{

class ActionTriggerSeparatorPropertySet final : private cppu::BaseMutex,
                                            public css::lang::XServiceInfo ,
                                            public css::lang::XTypeProvider,
                                            public ::cppu::OBroadcastHelper             ,
                                            public ::cppu::OPropertySetHelper           ,   // -> XPropertySet, XFastPropertySet, XMultiPropertySet
                                            public ::cppu::OWeakObject
{
    public:
        ActionTriggerSeparatorPropertySet();
        virtual ~ActionTriggerSeparatorPropertySet() override;

        // XInterface
        virtual cpo::uno::Any queryInterface( const cpo::uno::Type& aType ) override;
        virtual void acquire() noexcept override;
        virtual void release() noexcept override;

        // XServiceInfo
        virtual OUString getImplementationName(  ) override;
        virtual bool supportsService( const OUString& ServiceName ) override;
        virtual cpo::uno::Sequence< OUString > getSupportedServiceNames(  ) override;

        // XTypeProvider
        virtual cpo::uno::Sequence< cpo::uno::Type > getTypes(  ) override;
        virtual cpo::uno::Sequence< sal_Int8 > getImplementationId(  ) override;

    private:

        //  OPropertySetHelper

        virtual bool convertFastPropertyValue( cpo::uno::Any&       aConvertedValue,
                                                            cpo::uno::Any&       aOldValue,
                                                            sal_Int32                       nHandle,
                                                            const cpo::uno::Any& aValue          ) override;

        virtual void setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const cpo::uno::Any& aValue ) override;

        using cppu::OPropertySetHelper::getFastPropertyValue;
        virtual void getFastPropertyValue( cpo::uno::Any& aValue, sal_Int32 nHandle ) const override;

        virtual ::cppu::IPropertyArrayHelper& getInfoHelper() override;

        virtual css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo() override;

        static cpo::uno::Sequence< css::beans::Property > impl_getStaticPropertyDescriptor();

        //  helper

        /// @throws css::lang::IllegalArgumentException
        /// @throws css::uno::RuntimeException
        static bool impl_tryToChangeProperty(  sal_Int16                           aCurrentValue   ,
                                            const   cpo::uno::Any&   aNewValue       ,
                                            cpo::uno::Any&           aOldValue       ,
                                            cpo::uno::Any&           aConvertedValue );

        //  members

        sal_Int16   m_nSeparatorType;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
