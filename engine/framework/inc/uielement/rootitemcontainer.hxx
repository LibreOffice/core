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

#include <helper/shareablemutex.hxx>

#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <rtl/ustring.hxx>
#include <rtl/ref.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/propshlp.hxx>

#include <vector>

namespace framework
{
class ConstItemContainer;
class ItemContainer;

typedef ::cppu::WeakImplHelper<
            css::container::XIndexContainer,
            css::lang::XSingleComponentFactory > RootItemContainer_BASE;

class RootItemContainer final : private cppu::BaseMutex,
                            public ::cppu::OBroadcastHelper                         ,
                            public ::cppu::OPropertySetHelper                       ,
                            public RootItemContainer_BASE
{
    friend class ConstItemContainer;

    public:
        RootItemContainer();
        RootItemContainer( const css::uno::Reference< css::container::XIndexAccess >& rItemAccessContainer );
        virtual ~RootItemContainer() override;

        // XInterface
        virtual void acquire() noexcept override
            { OWeakObject::acquire(); }
        virtual void release() noexcept override
            { OWeakObject::release(); }
        virtual cpo::uno::Any queryInterface( const cpo::uno::Type& type) override;

        // XTypeProvider
        virtual cpo::uno::Sequence< cpo::uno::Type > getTypes(  ) override;

        // XIndexContainer
        virtual void insertByIndex( sal_Int32 Index, const cpo::uno::Any& Element ) override;

        virtual void removeByIndex( sal_Int32 Index ) override;

        // XIndexReplace
        virtual void replaceByIndex( sal_Int32 Index, const cpo::uno::Any& Element ) override;

        // XIndexAccess
        virtual sal_Int32 getCount() override;

        virtual cpo::uno::Any getByIndex( sal_Int32 Index ) override;

        // XElementAccess
        virtual cpo::uno::Type getElementType() override
        {
            return cppu::UnoType<cpo::uno::Sequence< css::beans::PropertyValue >>::get();
        }

        virtual bool hasElements() override;

        // XSingleComponentFactory
        virtual css::uno::Reference< css::uno::XInterface > createInstanceWithContext( const css::uno::Reference< css::uno::XComponentContext >& Context ) override;
        virtual css::uno::Reference< css::uno::XInterface > createInstanceWithArgumentsAndContext( const cpo::uno::Sequence< cpo::uno::Any >& Arguments, const css::uno::Reference< css::uno::XComponentContext >& Context ) override;

    private:
        //  OPropertySetHelper
        virtual bool                                            convertFastPropertyValue        ( cpo::uno::Any&        aConvertedValue ,
                                                                                                               cpo::uno::Any&        aOldValue       ,
                                                                                                               sal_Int32                        nHandle         ,
                                                                                                               const cpo::uno::Any&  aValue          ) override;
        virtual void                                                setFastPropertyValue_NoBroadcast( sal_Int32                        nHandle         ,
                                                                                                               const cpo::uno::Any&  aValue          ) override;
        using cppu::OPropertySetHelper::getFastPropertyValue;
        virtual void                                                getFastPropertyValue( cpo::uno::Any&    aValue          ,
                                                                                                   sal_Int32                    nHandle         ) const override;
        virtual ::cppu::IPropertyArrayHelper&                       getInfoHelper() override;
        virtual css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo() override;

        static cpo::uno::Sequence< css::beans::Property > impl_getStaticPropertyDescriptor();

        RootItemContainer& operator=( const RootItemContainer& ) = delete;
        RootItemContainer( const RootItemContainer& ) = delete;

        rtl::Reference< ItemContainer > deepCopyContainer( const css::uno::Reference< css::container::XIndexAccess >& rSubContainer );

        mutable ShareableMutex                                            m_aShareMutex;
        std::vector< cpo::uno::Sequence< css::beans::PropertyValue > >    m_aItemVector;
        OUString                                                          m_aUIName;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
