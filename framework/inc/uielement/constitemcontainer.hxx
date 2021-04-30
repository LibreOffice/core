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

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>

#include <rtl/ustring.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/propshlp.hxx>

#include <vector>

namespace framework
{

class RootItemContainer;
class ItemContainer;
class ConstItemContainer final : public ::cppu::WeakImplHelper<
                                                    css::container::XIndexAccess,
                                                    css::lang::XUnoTunnel       ,
                                                    css::beans::XFastPropertySet,
                                                    css::beans::XPropertySet >
{
    friend class RootItemContainer;
    friend class ItemContainer;

    public:
        ConstItemContainer();
        ConstItemContainer( const ItemContainer& rtemContainer );
        ConstItemContainer( const css::uno::Reference< css::container::XIndexAccess >& rSourceContainer, bool bFastCopy = false );
        virtual ~ConstItemContainer() override;

        // XUnoTunnel
        static const css::uno::Sequence< sal_Int8 >&   getUnoTunnelId() noexcept;
        sal_Int64                                                   SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& rIdentifier ) override;

        // XIndexAccess
        virtual sal_Int32 SAL_CALL getCount() override;

        virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

        // XElementAccess
        virtual css::uno::Type SAL_CALL getElementType() override
        {
            return cppu::UnoType<css::uno::Sequence< css::beans::PropertyValue >>::get();
        }

        virtual sal_Bool SAL_CALL hasElements() override;

        // XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
        virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
        virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
        virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
        virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
        virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
        virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

        // XFastPropertySet
        virtual void SAL_CALL setFastPropertyValue( sal_Int32 nHandle, const css::uno::Any& aValue ) override;
        virtual css::uno::Any SAL_CALL getFastPropertyValue( sal_Int32 nHandle ) override;

    private:
        ::cppu::IPropertyArrayHelper& getInfoHelper();
        css::uno::Sequence< css::beans::Property > impl_getStaticPropertyDescriptor();

        void copyItemContainer( const std::vector< css::uno::Sequence< css::beans::PropertyValue > >& rSourceVector );
        css::uno::Reference< css::container::XIndexAccess > deepCopyContainer( const css::uno::Reference< css::container::XIndexAccess >& rSubContainer );

        std::vector< css::uno::Sequence< css::beans::PropertyValue > >       m_aItemVector;
        OUString                                                             m_aUIName;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
