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

#ifndef INCLUDED_FRAMEWORK_INC_UIELEMENT_ROOTITEMCONTAINER_HXX
#define INCLUDED_FRAMEWORK_INC_UIELEMENT_ROOTITEMCONTAINER_HXX

#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <helper/shareablemutex.hxx>

#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>

#include <rtl/ustring.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/propshlp.hxx>

#include <vector>
#include <fwidllapi.h>

namespace framework
{
class ConstItemContainer;

typedef ::cppu::WeakImplHelper<
            css::container::XIndexContainer,
            css::lang::XSingleComponentFactory,
            css::lang::XUnoTunnel > RootItemContainer_BASE;

class RootItemContainer :   private cppu::BaseMutex,
                            public ::cppu::OBroadcastHelper                         ,
                            public ::cppu::OPropertySetHelper                       ,
                            public RootItemContainer_BASE
{
    friend class ConstItemContainer;

    public:
        FWI_DLLPUBLIC RootItemContainer();
        FWI_DLLPUBLIC RootItemContainer( const com::sun::star::uno::Reference< com::sun::star::container::XIndexAccess >& rItemAccessContainer );
        virtual FWI_DLLPUBLIC ~RootItemContainer();

        // XInterface
        virtual void SAL_CALL acquire() throw () SAL_OVERRIDE
            { OWeakObject::acquire(); }
        virtual void SAL_CALL release() throw () SAL_OVERRIDE
            { OWeakObject::release(); }
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& type) throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        // XTypeProvider
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XUnoTunnel
        static FWI_DLLPUBLIC const ::com::sun::star::uno::Sequence< sal_Int8 >&   GetUnoTunnelId() throw();
        static FWI_DLLPUBLIC RootItemContainer*                                   GetImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rxIFace ) throw();
        sal_Int64                                                   SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rIdentifier ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XIndexContainer
        virtual void SAL_CALL insertByIndex( sal_Int32 Index, const ::com::sun::star::uno::Any& Element )
            throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual void SAL_CALL removeByIndex( sal_Int32 Index )
            throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XIndexReplace
        virtual void SAL_CALL replaceByIndex( sal_Int32 Index, const ::com::sun::star::uno::Any& Element )
            throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XIndexAccess
        virtual sal_Int32 SAL_CALL getCount()
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
            throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XElementAccess
        virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
        {
            return cppu::UnoType<com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >>::get();
        }

        virtual sal_Bool SAL_CALL hasElements()
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XSingleComponentFactory
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstanceWithContext( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& Context ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstanceWithArgumentsAndContext( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Arguments, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& Context ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    protected:
        //  OPropertySetHelper
        virtual sal_Bool                                            SAL_CALL convertFastPropertyValue        ( com::sun::star::uno::Any&        aConvertedValue ,
                                                                                                               com::sun::star::uno::Any&        aOldValue       ,
                                                                                                               sal_Int32                        nHandle         ,
                                                                                                               const com::sun::star::uno::Any&  aValue          ) throw( com::sun::star::lang::IllegalArgumentException ) SAL_OVERRIDE;
        virtual void                                                SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32                        nHandle         ,
                                                                                                               const com::sun::star::uno::Any&  aValue          ) throw( com::sun::star::uno::Exception, std::exception                 ) SAL_OVERRIDE;
        using cppu::OPropertySetHelper::getFastPropertyValue;
        virtual void                                                SAL_CALL getFastPropertyValue( com::sun::star::uno::Any&    aValue          ,
                                                                                                   sal_Int32                    nHandle         ) const SAL_OVERRIDE;
        virtual ::cppu::IPropertyArrayHelper&                       SAL_CALL getInfoHelper() SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        static const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > impl_getStaticPropertyDescriptor();

    private:
        RootItemContainer& operator=( const RootItemContainer& ) = delete;
        RootItemContainer( const RootItemContainer& ) = delete;

        com::sun::star::uno::Reference< com::sun::star::container::XIndexAccess > deepCopyContainer( const com::sun::star::uno::Reference< com::sun::star::container::XIndexAccess >& rSubContainer );

        mutable ShareableMutex                                                                  m_aShareMutex;
        std::vector< com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > >    m_aItemVector;
        OUString                                                                           m_aUIName;
};

}

#endif // INCLUDED_FRAMEWORK_INC_UIELEMENT_ROOTITEMCONTAINER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
