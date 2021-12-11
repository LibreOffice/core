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

#include <helper/propertysetcontainer.hxx>
#include <com/sun/star/awt/XPopupMenu.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>

inline constexpr OUStringLiteral IMPLEMENTATIONNAME_ROOTACTIONTRIGGERCONTAINER = u"com.sun.star.comp.ui.RootActionTriggerContainer";

namespace framework
{

class RootActionTriggerContainer final : public PropertySetContainer,
                                    public css::lang::XMultiServiceFactory,
                                    public css::lang::XServiceInfo,
                                    public css::lang::XUnoTunnel,
                                    public css::lang::XTypeProvider,
                                    public css::container::XNamed
{
    public:
        RootActionTriggerContainer(const css::uno::Reference<css::awt::XPopupMenu>& rMenu, const OUString* pMenuIdentifier);
        virtual ~RootActionTriggerContainer() override;

        // XInterface
        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) override;
        virtual void SAL_CALL acquire() noexcept override;
        virtual void SAL_CALL release() noexcept override;

        // XMultiServiceFactory
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance( const OUString& aServiceSpecifier ) override;
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithArguments( const OUString& ServiceSpecifier, const css::uno::Sequence< css::uno::Any >& Arguments ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getAvailableServiceNames() override;

        // XIndexContainer
        virtual void SAL_CALL insertByIndex( sal_Int32 Index, const css::uno::Any& Element ) override;

        virtual void SAL_CALL removeByIndex( sal_Int32 Index ) override;

        // XIndexReplace
        virtual void SAL_CALL replaceByIndex( sal_Int32 Index, const css::uno::Any& Element ) override;

        // XIndexAccess
        virtual sal_Int32 SAL_CALL getCount() override;

        virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

        // XElementAccess
        virtual css::uno::Type SAL_CALL getElementType() override;

        virtual sal_Bool SAL_CALL hasElements() override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

        // XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;
        static const css::uno::Sequence<sal_Int8>& getUnoTunnelId() noexcept;

        // XTypeProvider
        virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) override;

        // XNamed
        virtual OUString SAL_CALL getName(  ) override;
        virtual void SAL_CALL setName( const OUString& aName ) override;

    private:
        void FillContainer();

        bool            m_bContainerCreated;
        css::uno::Reference<css::awt::XPopupMenu> m_xMenu;
        const OUString* m_pMenuIdentifier;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
