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

#ifndef INCLUDED_COMPHELPER_PROXYAGGREGATION_HXX
#define INCLUDED_COMPHELPER_PROXYAGGREGATION_HXX

#include <com/sun/star/uno/XAggregation.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <cppuhelper/compbase_ex.hxx>
#include <comphelper/comphelperdllapi.h>

namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
} } } }

/* class hierarchy herein:

         +-------------------+          helper class for aggregating the proxy to another object
         | OProxyAggregation |          - not ref counted
         +-------------------+          - no UNO implementation, i.e. not derived from XInterface
                   ^                      (neither direct nor indirect)
                   |
                   |
  +----------------------------------+  helper class for aggregating a proxy to an XComponent
  | OComponentProxyAggregationHelper |  - life time coupling: if the inner component (the "aggregate")
  +----------------------------------+    is disposed, the outer (the delegator) is disposed, too, and
                   ^                      vice versa
                   |                    - UNO based, implementing XEventListener
                   |
     +----------------------------+     component aggregating another XComponent
     | OComponentProxyAggregation |     - life time coupling as above
     +----------------------------+     - ref-counted
                                        - implements an XComponent itself

  If you need to

  - wrap a foreign object which is a XComponent
    => use OComponentProxyAggregation
       - call componentAggregateProxyFor in your ctor
       - call implEnsureDisposeInDtor in your dtor

  - wrap a foreign object which is a XComponent, but already have ref-counting mechanisms
    inherited from somewhere else
    => use OComponentProxyAggregationHelper
       - override dispose - don't forget to call the base class' dispose!
       - call componentAggregateProxyFor in your ctor

  - wrap a foreign object which is no XComponent
    => use OProxyAggregation
       - call baseAggregateProxyFor in your ctor
*/


namespace comphelper
{



    //= OProxyAggregation

    /** helper class for aggregating a proxy for a foreign object
    */
    class OProxyAggregation
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation >             m_xProxyAggregate;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XTypeProvider >           m_xProxyTypeAccess;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >        m_xContext;

    protected:
        inline const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& getComponentContext()
        {
            return m_xContext;
        }

    protected:
        OProxyAggregation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext );
        ~OProxyAggregation();

        /// to be called from within your ctor
        void baseAggregateProxyFor(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxComponent,
            oslInterlockedCount& _rRefCount,
            ::cppu::OWeakObject& _rDelegator
        );

        // XInterface and XTypeProvider
        ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw (::com::sun::star::uno::RuntimeException);
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw (::com::sun::star::uno::RuntimeException);

    private:
        OProxyAggregation( const OProxyAggregation& ) = delete;
        OProxyAggregation& operator=( const OProxyAggregation& ) = delete;
    };


    //= OComponentProxyAggregationHelper

    /** a helper class for aggregating a proxy to an XComponent

        <p>The object couples the life time of itself and the component: if one of the both
        dies (in a sense of being disposed), the other one dies, too.</p>

        <p>The class itself does not implement XComponent so you need to forward any XComponent::dispose
        calls which your derived class gets to the dispose method of this class.</p>
    */

    class COMPHELPER_DLLPUBLIC OComponentProxyAggregationHelper :public ::cppu::ImplHelper1 <   com::sun::star::lang::XEventListener
                                                                        >
                                            ,private OProxyAggregation
    {
    private:
        typedef ::cppu::ImplHelper1 <   ::com::sun::star::lang::XEventListener
                                    >   BASE;   // prevents some MSVC problems

    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >
                                            m_xInner;
        ::cppu::OBroadcastHelper&           m_rBHelper;

    protected:
        // OProxyAggregation
        using OProxyAggregation::getComponentContext;

        // XInterface
        ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& _rType ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        // XTypeProvider
        DECLARE_XTYPEPROVIDER( )

    protected:
        OComponentProxyAggregationHelper(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext,
            ::cppu::OBroadcastHelper& _rBHelper
        );
        virtual ~OComponentProxyAggregationHelper( );

        /// to be called from within your ctor
        void componentAggregateProxyFor(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& _rxComponent,
            oslInterlockedCount& _rRefCount,
            ::cppu::OWeakObject& _rDelegator
        );

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        // XComponent
        virtual void SAL_CALL dispose() throw( ::com::sun::star::uno::RuntimeException, std::exception ) = 0;

    private:
        OComponentProxyAggregationHelper( const OComponentProxyAggregationHelper& ) = delete;
        OComponentProxyAggregationHelper& operator=( const OComponentProxyAggregationHelper& ) = delete;
    };


    //= OComponentProxyAggregation

    class COMPHELPER_DLLPUBLIC OComponentProxyAggregation   :public OBaseMutex
                                        ,public cppu::WeakComponentImplHelperBase
                                        ,public OComponentProxyAggregationHelper
    {
    protected:
        OComponentProxyAggregation(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& _rxComponent
        );

        virtual ~OComponentProxyAggregation();

        // XInterface
        DECLARE_XINTERFACE()
        // XTypeProvider
        DECLARE_XTYPEPROVIDER()

        // OComponentHelper
        virtual void SAL_CALL disposing()  throw (::com::sun::star::uno::RuntimeException) override;

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& _rSource ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        // XComponent/OComponentProxyAggregationHelper
        virtual void SAL_CALL dispose() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    protected:
        // be called from within the dtor of derived classes
        void implEnsureDisposeInDtor( );

    private:
        OComponentProxyAggregation( const OComponentProxyAggregation& ) = delete;
        OComponentProxyAggregation& operator=( const OComponentProxyAggregation& ) = delete;
    };


}   // namespace comphelper



#endif // INCLUDED_COMPHELPER_PROXYAGGREGATION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
