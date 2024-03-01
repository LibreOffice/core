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

#include <config_options.h>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <cppuhelper/basemutex.hxx>
#include <comphelper/uno3.hxx>
#include <cppuhelper/compbase_ex.hxx>
#include <comphelper/comphelperdllapi.h>

namespace com::sun::star::uno {
    class XComponentContext;
}
namespace com::sun::star::uno { class XAggregation; }
namespace com::sun::star::lang { class XComponent; }

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
        css::uno::Reference< css::uno::XAggregation >             m_xProxyAggregate;
        css::uno::Reference< css::lang::XTypeProvider >           m_xProxyTypeAccess;
        css::uno::Reference< css::uno::XComponentContext >        m_xContext;

    protected:
        const css::uno::Reference< css::uno::XComponentContext >& getComponentContext() const
        {
            return m_xContext;
        }

    protected:
        OProxyAggregation( const css::uno::Reference< css::uno::XComponentContext >& _rxContext );
        ~OProxyAggregation();

        /// to be called from within your ctor
        void baseAggregateProxyFor(
            const css::uno::Reference< css::uno::XInterface >& _rxComponent,
            oslInterlockedCount& _rRefCount,
            ::cppu::OWeakObject& _rDelegator
        );

        // XInterface and XTypeProvider
        /// @throws css::uno::RuntimeException
        css::uno::Any SAL_CALL queryAggregation( const css::uno::Type& _rType );
        /// @throws css::uno::RuntimeException
        css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  );

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

    class OComponentProxyAggregationHelper :public ::cppu::ImplHelper1 <   css::lang::XEventListener
                                                                        >
                                            ,private OProxyAggregation
    {
    private:
        typedef ::cppu::ImplHelper1 <   css::lang::XEventListener
                                    >   BASE;   // prevents some MSVC problems

    protected:
        css::uno::Reference< css::lang::XComponent >
                                            m_xInner;
        ::cppu::OBroadcastHelper&           m_rBHelper;

    protected:
        // OProxyAggregation
        using OProxyAggregation::getComponentContext;

        // XInterface
        css::uno::Any SAL_CALL queryInterface( const css::uno::Type& _rType ) override;

        // XTypeProvider
        DECLARE_XTYPEPROVIDER( )

    protected:
        OComponentProxyAggregationHelper(
            const css::uno::Reference< css::uno::XComponentContext >& _rxContext,
            ::cppu::OBroadcastHelper& _rBHelper
        );
        virtual ~OComponentProxyAggregationHelper( );

        /// to be called from within your ctor
        void componentAggregateProxyFor(
            const css::uno::Reference< css::lang::XComponent >& _rxComponent,
            oslInterlockedCount& _rRefCount,
            ::cppu::OWeakObject& _rDelegator
        );

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

        // XComponent
        /// @throws css::uno::RuntimeException
        virtual void SAL_CALL dispose() = 0;

    private:
        OComponentProxyAggregationHelper( const OComponentProxyAggregationHelper& ) = delete;
        OComponentProxyAggregationHelper& operator=( const OComponentProxyAggregationHelper& ) = delete;
    };


    //= OComponentProxyAggregation

    class UNLESS_MERGELIBS_MORE(COMPHELPER_DLLPUBLIC) OComponentProxyAggregation : public cppu::BaseMutex
                                        ,public cppu::WeakComponentImplHelperBase
                                        ,public OComponentProxyAggregationHelper
    {
    protected:
        OComponentProxyAggregation(
            const css::uno::Reference< css::uno::XComponentContext >& _rxContext,
            const css::uno::Reference< css::lang::XComponent >& _rxComponent
        );

        virtual ~OComponentProxyAggregation() override;

        // XInterface
        DECLARE_XINTERFACE()
        // XTypeProvider
        DECLARE_XTYPEPROVIDER()

        // OComponentHelper
        virtual void SAL_CALL disposing() override;

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& _rSource ) override;

        // XComponent/OComponentProxyAggregationHelper
        virtual void SAL_CALL dispose() override;

    private:
        OComponentProxyAggregation( const OComponentProxyAggregation& ) = delete;
        OComponentProxyAggregation& operator=( const OComponentProxyAggregation& ) = delete;
    };


}   // namespace comphelper


#endif // INCLUDED_COMPHELPER_PROXYAGGREGATION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
