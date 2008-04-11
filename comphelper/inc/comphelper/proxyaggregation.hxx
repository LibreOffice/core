/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: proxyaggregation.hxx,v $
 * $Revision: 1.7 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef COMPHELPER_PROXY_AGGREGATION
#define COMPHELPER_PROXY_AGGREGATION

#include <com/sun/star/uno/XAggregation.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <cppuhelper/compbase_ex.hxx>
#include "comphelper/comphelperdllapi.h"

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

//.............................................................................
namespace comphelper
{
//.............................................................................

    //=========================================================================
    //= OProxyAggregation
    //=========================================================================
    /** helper class for aggregating a proxy for a foreign object
    */
    class OProxyAggregation
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation >             m_xProxyAggregate;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XTypeProvider >           m_xProxyTypeAccess;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    m_xORB;

    protected:
        inline const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& getORB()
        {
            return m_xORB;
        }

    protected:
        OProxyAggregation( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB );
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
        OProxyAggregation( );                                       // never implemented
        OProxyAggregation( const OProxyAggregation& );              // never implemented
        OProxyAggregation& operator=( const OProxyAggregation& );   // never implemented
    };

    //=========================================================================
    //= OComponentProxyAggregationHelper
    //=========================================================================
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
        using OProxyAggregation::getORB;

        // XInterface
        ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& _rType ) throw (::com::sun::star::uno::RuntimeException);

        // XTypeProvider
        DECLARE_XTYPEPROVIDER( )

    protected:
        OComponentProxyAggregationHelper(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
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
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

        // XComponent
        virtual void SAL_CALL dispose() throw( ::com::sun::star::uno::RuntimeException );

    private:
        COMPHELPER_DLLPRIVATE OComponentProxyAggregationHelper( );                                                  // never implemented
        COMPHELPER_DLLPRIVATE OComponentProxyAggregationHelper( const OComponentProxyAggregationHelper& );          // never implemented
        COMPHELPER_DLLPRIVATE OComponentProxyAggregationHelper& operator=( const OComponentProxyAggregationHelper& );   // never implemented
    };

    //=========================================================================
    //= OComponentProxyAggregation
    //=========================================================================
    typedef ::cppu::WeakComponentImplHelperBase OComponentProxyAggregation_CBase;

    class COMPHELPER_DLLPUBLIC OComponentProxyAggregation   :public OBaseMutex
                                        ,public OComponentProxyAggregation_CBase
                                        ,public OComponentProxyAggregationHelper
    {
    protected:
        OComponentProxyAggregation(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& _rxComponent
        );

        virtual ~OComponentProxyAggregation();

        // XInterface
        DECLARE_XINTERFACE()
        // XTypeProvider
        DECLARE_XTYPEPROVIDER()

        // OComponentHelper
        virtual void SAL_CALL disposing()  throw (::com::sun::star::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& _rSource ) throw (::com::sun::star::uno::RuntimeException);

        // XComponent/OComponentProxyAggregationHelper
        virtual void SAL_CALL dispose() throw( ::com::sun::star::uno::RuntimeException );

    protected:
        // be called from within the dtor of derived classes
        void implEnsureDisposeInDtor( );

    private:
        COMPHELPER_DLLPRIVATE OComponentProxyAggregation( );                                                // never implemented
        COMPHELPER_DLLPRIVATE OComponentProxyAggregation( const OComponentProxyAggregation& );          // never implemented
        COMPHELPER_DLLPRIVATE OComponentProxyAggregation& operator=( const OComponentProxyAggregation& );   // never implemented
    };

//.............................................................................
}   // namespace comphelper
//.............................................................................


#endif // COMPHELPER_PROXY_AGGREGATION
