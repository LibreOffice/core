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
class FWI_DLLPUBLIC ItemContainer :   public ::cppu::WeakImplHelper< ::com::sun::star::container::XIndexContainer>
{
    friend class ConstItemContainer;

    public:
        ItemContainer( const ShareableMutex& );
        ItemContainer( const ConstItemContainer& rConstItemContainer, const ShareableMutex& rMutex );
        ItemContainer( const com::sun::star::uno::Reference< com::sun::star::container::XIndexAccess >& rItemAccessContainer, const ShareableMutex& rMutex );
        virtual ~ItemContainer();

        //  XInterface, XTypeProvider

        // XUnoTunnel
        static const ::com::sun::star::uno::Sequence< sal_Int8 >&   GetUnoTunnelId() throw();
        static ItemContainer*                                       GetImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rxIFace ) throw();

        // XIndexContainer
        virtual void SAL_CALL insertByIndex( sal_Int32 Index, const ::com::sun::star::uno::Any& Element )
            throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

        virtual void SAL_CALL removeByIndex( sal_Int32 Index )
            throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

        // XIndexReplace
        virtual void SAL_CALL replaceByIndex( sal_Int32 Index, const ::com::sun::star::uno::Any& Element )
            throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

        // XIndexAccess
        virtual sal_Int32 SAL_CALL getCount()
            throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
            throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

        // XElementAccess
        virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
            throw (::com::sun::star::uno::RuntimeException, std::exception) override
        {
            return cppu::UnoType<com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >>::get();
        }

        virtual sal_Bool SAL_CALL hasElements()
            throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    private:
        ItemContainer();
        void copyItemContainer( const std::vector< com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > >& rSourceVector, const ShareableMutex& rMutex );
        com::sun::star::uno::Reference< com::sun::star::container::XIndexAccess > deepCopyContainer( const com::sun::star::uno::Reference< com::sun::star::container::XIndexAccess >& rSubContainer, const ShareableMutex& rMutex );

        mutable ShareableMutex                                                               m_aShareMutex;
        std::vector< com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > > m_aItemVector;
};

}

#endif // INCLUDED_FRAMEWORK_INC_UIELEMENT_ITEMCONTAINER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
