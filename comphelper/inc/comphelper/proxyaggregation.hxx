/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: proxyaggregation.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 02:37:35 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef COMPHELPER_PROXY_AGGREGATION
#define COMPHELPER_PROXY_AGGREGATION

#ifndef _COM_SUN_STAR_UNO_XAGGREGATION_HPP_
#include <com/sun/star/uno/XAggregation.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif
#ifndef _CPPUHELPER_COMPBASE_EX_HXX_
#include <cppuhelper/compbase_ex.hxx>
#endif
#ifndef INCLUDED_COMPHELPERDLLAPI_H
#include "comphelper/comphelperdllapi.h"
#endif

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
       - call aggregateProxyFor in your ctor
       - call implEnsureDisposeInDtor in your dtor

  - wrap a foreign object which is a XComponent, but already have ref-counting mechanisms
    inherited from somewhere else
    => use OComponentProxyAggregationHelper
       - override dispose - don't forget to call the base class' dispose!
       - call aggregateProxyFor in your ctor

  - wrap a foreign object which is no XComponent
    => use OProxyAggregation
       - call aggregateProxyFor in your ctor
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
        void aggregateProxyFor(
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
        OProxyAggregation::getORB;

        // XInterface
        ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& _rType ) throw (::com::sun::star::uno::RuntimeException);
        // still waiting to be overwritten
        virtual void SAL_CALL acquire(  ) throw () = 0;
        virtual void SAL_CALL release(  ) throw () = 0;

        // XTypeProvider
        DECLARE_XTYPEPROVIDER( )

    protected:
        OComponentProxyAggregationHelper(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            ::cppu::OBroadcastHelper& _rBHelper
        );
        ~OComponentProxyAggregationHelper( );

        /// to be called from within your ctor
        void aggregateProxyFor(
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
