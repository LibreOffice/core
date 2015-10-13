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

#ifndef INCLUDED_FRAMEWORK_INC_UIELEMENT_ITEMCONTAINER_HXX
#define INCLUDED_FRAMEWORK_INC_UIELEMENT_ITEMCONTAINER_HXX

#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <helper/shareablemutex.hxx>

#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>

#include <rtl/ustring.hxx>
#include <cppuhelper/implbase.hxx>

#include <vector>
#include <fwidllapi.h>

namespace framework
{
class ConstItemContainer;
class FWI_DLLPUBLIC ItemContainer :   public ::cppu::WeakImplHelper< css::container::XIndexContainer>
{
    friend class ConstItemContainer;

    public:
        ItemContainer( const ShareableMutex& );
        ItemContainer( const ConstItemContainer& rConstItemContainer, const ShareableMutex& rMutex );
        ItemContainer( const css::uno::Reference< css::container::XIndexAccess >& rItemAccessContainer, const ShareableMutex& rMutex );
        virtual ~ItemContainer();

        //  XInterface, XTypeProvider

        // XUnoTunnel
        static const css::uno::Sequence< sal_Int8 >&   GetUnoTunnelId() throw();
        static ItemContainer*                                       GetImplementation( const css::uno::Reference< css::uno::XInterface >& rxIFace ) throw();

        // XIndexContainer
        virtual void SAL_CALL insertByIndex( sal_Int32 Index, const css::uno::Any& Element )
            throw (css::lang::IllegalArgumentException, css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

        virtual void SAL_CALL removeByIndex( sal_Int32 Index )
            throw (css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

        // XIndexReplace
        virtual void SAL_CALL replaceByIndex( sal_Int32 Index, const css::uno::Any& Element )
            throw (css::lang::IllegalArgumentException, css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

        // XIndexAccess
        virtual sal_Int32 SAL_CALL getCount()
            throw (css::uno::RuntimeException, std::exception) override;

        virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
            throw (css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

        // XElementAccess
        virtual css::uno::Type SAL_CALL getElementType()
            throw (css::uno::RuntimeException, std::exception) override
        {
            return cppu::UnoType<css::uno::Sequence< css::beans::PropertyValue >>::get();
        }

        virtual sal_Bool SAL_CALL hasElements()
            throw (css::uno::RuntimeException, std::exception) override;

    private:
        ItemContainer();
        void copyItemContainer( const std::vector< css::uno::Sequence< css::beans::PropertyValue > >& rSourceVector, const ShareableMutex& rMutex );
        css::uno::Reference< css::container::XIndexAccess > deepCopyContainer( const css::uno::Reference< css::container::XIndexAccess >& rSubContainer, const ShareableMutex& rMutex );

        mutable ShareableMutex                                                               m_aShareMutex;
        std::vector< css::uno::Sequence< css::beans::PropertyValue > > m_aItemVector;
};

}

#endif // INCLUDED_FRAMEWORK_INC_UIELEMENT_ITEMCONTAINER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
