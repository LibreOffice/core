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
        FWI_DLLPUBLIC RootItemContainer( const css::uno::Reference< css::container::XIndexAccess >& rItemAccessContainer );
        virtual FWI_DLLPUBLIC ~RootItemContainer();

        // XInterface
        virtual void SAL_CALL acquire() throw () override
            { OWeakObject::acquire(); }
        virtual void SAL_CALL release() throw () override
            { OWeakObject::release(); }
        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& type) throw ( css::uno::RuntimeException, std::exception ) override;

        // XTypeProvider
        virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) throw(css::uno::RuntimeException, std::exception) override;

        // XUnoTunnel
        static FWI_DLLPUBLIC const css::uno::Sequence< sal_Int8 >&   GetUnoTunnelId() throw();
        static FWI_DLLPUBLIC RootItemContainer*                                   GetImplementation( const css::uno::Reference< css::uno::XInterface >& rxIFace ) throw();
        sal_Int64                                                   SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& rIdentifier ) throw(css::uno::RuntimeException, std::exception) override;

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

        // XSingleComponentFactory
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithContext( const css::uno::Reference< css::uno::XComponentContext >& Context ) throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithArgumentsAndContext( const css::uno::Sequence< css::uno::Any >& Arguments, const css::uno::Reference< css::uno::XComponentContext >& Context ) throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;

    protected:
        //  OPropertySetHelper
        virtual sal_Bool                                            SAL_CALL convertFastPropertyValue        ( css::uno::Any&        aConvertedValue ,
                                                                                                               css::uno::Any&        aOldValue       ,
                                                                                                               sal_Int32                        nHandle         ,
                                                                                                               const css::uno::Any&  aValue          ) throw( css::lang::IllegalArgumentException ) override;
        virtual void                                                SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32                        nHandle         ,
                                                                                                               const css::uno::Any&  aValue          ) throw( css::uno::Exception, std::exception                 ) override;
        using cppu::OPropertySetHelper::getFastPropertyValue;
        virtual void                                                SAL_CALL getFastPropertyValue( css::uno::Any&    aValue          ,
                                                                                                   sal_Int32                    nHandle         ) const override;
        virtual ::cppu::IPropertyArrayHelper&                       SAL_CALL getInfoHelper() override;
        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() throw (css::uno::RuntimeException, std::exception) override;

        static const css::uno::Sequence< css::beans::Property > impl_getStaticPropertyDescriptor();

    private:
        RootItemContainer& operator=( const RootItemContainer& ) = delete;
        RootItemContainer( const RootItemContainer& ) = delete;

        css::uno::Reference< css::container::XIndexAccess > deepCopyContainer( const css::uno::Reference< css::container::XIndexAccess >& rSubContainer );

        mutable ShareableMutex                                            m_aShareMutex;
        std::vector< css::uno::Sequence< css::beans::PropertyValue > >    m_aItemVector;
        OUString                                                          m_aUIName;
};

}

#endif // INCLUDED_FRAMEWORK_INC_UIELEMENT_ROOTITEMCONTAINER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
