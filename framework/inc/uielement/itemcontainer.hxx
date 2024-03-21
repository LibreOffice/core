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

#include <helper/shareablemutex.hxx>

#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <cppuhelper/implbase.hxx>

#include <vector>

namespace framework
{
class ConstItemContainer;
class ItemContainer final : public ::cppu::WeakImplHelper< css::container::XIndexContainer>
{
    friend class ConstItemContainer;

    public:
        ItemContainer( const ShareableMutex& );
        ItemContainer( const ConstItemContainer& rConstItemContainer, const ShareableMutex& rMutex );
        ItemContainer( const css::uno::Reference< css::container::XIndexAccess >& rItemAccessContainer, const ShareableMutex& rMutex );
        virtual ~ItemContainer() override;

        //  XInterface, XTypeProvider

        // XIndexContainer
        virtual void SAL_CALL insertByIndex( sal_Int32 Index, const css::uno::Any& Element ) override;

        virtual void SAL_CALL removeByIndex( sal_Int32 Index ) override;

        // XIndexReplace
        virtual void SAL_CALL replaceByIndex( sal_Int32 Index, const css::uno::Any& Element ) override;

        // XIndexAccess
        virtual sal_Int32 SAL_CALL getCount() override;

        virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

        // XElementAccess
        virtual css::uno::Type SAL_CALL getElementType() override
        {
            return cppu::UnoType<css::uno::Sequence< css::beans::PropertyValue >>::get();
        }

        virtual sal_Bool SAL_CALL hasElements() override;

    private:
        void copyItemContainer( const std::vector< css::uno::Sequence< css::beans::PropertyValue > >& rSourceVector, const ShareableMutex& rMutex );
        static css::uno::Reference< css::container::XIndexAccess > deepCopyContainer( const css::uno::Reference< css::container::XIndexAccess >& rSubContainer, const ShareableMutex& rMutex );

        mutable ShareableMutex                                                               m_aShareMutex;
        std::vector< css::uno::Sequence< css::beans::PropertyValue > > m_aItemVector;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
