/*************************************************************************
 *
 *  $RCSfile: accessiblewrapper.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-23 17:24:46 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef COMPHELPER_ACCESSIBLE_WRAPPER_HXX
#define COMPHELPER_ACCESSIBLE_WRAPPER_HXX

#ifndef _COM_SUN_STAR_UNO_XAGGREGATION_HPP_
#include <com/sun/star/uno/XAggregation.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HPP_
#include <drafts/com/sun/star/accessibility/XAccessible.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLECONTEXT_HPP_
#include <drafts/com/sun/star/accessibility/XAccessibleContext.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEEVENTBROADCASTER_HPP_
#include <drafts/com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEEVENTLISTENER_HPP_
#include <drafts/com/sun/star/accessibility/XAccessibleEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase2.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif
#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif
#ifndef COMPHELPER_ACCESSIBLE_EVENT_NOTIFIER
#include <comphelper/accessibleeventnotifier.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

//.............................................................................
namespace comphelper
{
//.............................................................................

    typedef ::std::map  <   ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible >
                        ,   ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible >
                        ,   ::comphelper::OInterfaceCompare< ::drafts::com::sun::star::accessibility::XAccessible >
                        >   AccessibleMap;
                        // TODO: think about if we should hold these objects weak
    //=========================================================================
    //= OWrappedAccessibleChildrenManager
    //=========================================================================
    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::lang::XEventListener
                                    >   OWrappedAccessibleChildrenManager_Base;
    /** manages wrapping XAccessible's to XAccessible's
    */
    class OWrappedAccessibleChildrenManager : public OWrappedAccessibleChildrenManager_Base
    {
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                                m_xORB;
        ::com::sun::star::uno::WeakReference< ::drafts::com::sun::star::accessibility::XAccessible >
                                m_aOwningAccessible;    // the XAccessible which belongs to the XAccessibleContext which we work for
        AccessibleMap           m_aChildrenMap;         // for caching children
        sal_Bool                m_bTransientChildren;   // are we prohibited to cache our children?

    public:
        /// ctor
        OWrappedAccessibleChildrenManager(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB
        );

        /** specifies if the children are to be consideren transient (i.e.: not cached)
            <p>to be called only once per lifetime</p>
        */
        void    setTransientChildren( sal_Bool _bSet = sal_True );

        /** sets the XAccessible which belongs to the XAccessibleContext which we work for
            <p>to be called only once per lifetime</p>
        */
        void    setOwningAccessible( const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible >& _rxAcc );

        /// retrieves a wrapper for the given accessible
        ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible >
                getAccessibleWrapperFor(
                    const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible >& _rxKey,
                    sal_Bool _bCreate = sal_True
                );

        /// erases the given key from the map (if it is present there)
        void    removeFromCache( const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible >& _rxKey );

        /// invalidates (i.e. empties) the map
        void    invalidateAll( );

        /// disposes (i.e. cleares) the manager
        void    dispose();

        /** handles a notification as got from the parent of the children we're managing
            <p>This applies only to the notifications which have a direct impact on our map.</p>
        */
        void    handleChildNotification( const ::drafts::com::sun::star::accessibility::AccessibleEventObject& _rEvent );

        /** translates events as got from the parent of the children we're managing
            <p>This applies only to the notifications which deal with child objects which we manage.</p>
        */
        void    translateAccessibleEvent(
            const   ::drafts::com::sun::star::accessibility::AccessibleEventObject& _rEvent,
                    ::drafts::com::sun::star::accessibility::AccessibleEventObject& _rTranslatedEvent
        );

    protected:
        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

    protected:
        void    implTranslateChildEventValue( const ::com::sun::star::uno::Any& _rInValue, ::com::sun::star::uno::Any& _rOutValue );

    protected:
        ~OWrappedAccessibleChildrenManager( );

    private:
        OWrappedAccessibleChildrenManager( );                                                       // never implemented
        OWrappedAccessibleChildrenManager( const OWrappedAccessibleChildrenManager& );              // never implemented
        OWrappedAccessibleChildrenManager& operator=( const OWrappedAccessibleChildrenManager& );   // never implemented
    };

    //=========================================================================
    //= OProxyAggregation
    //=========================================================================
    /** helper class for aggregating a proxy for a foreign object
    */
    class OProxyAggregation
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation >             m_xProxyAggregate;
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
    //= OComponentProxyAggregation
    //=========================================================================
    template < class INNER >
    class OComponentProxyAggregation    :public ::comphelper::OBaseMutex
                                        ,public ::cppu::WeakComponentImplHelper2    <   INNER
                                                                                    ,   com::sun::star::lang::XEventListener
                                                                                    >
                                        ,protected OProxyAggregation
    {
    private:
        typedef ::cppu::WeakComponentImplHelper2    <   INNER
                                                    ,   com::sun::star::lang::XEventListener
                                                    >   BASE;   // prevents some MSVC problems

    protected:
        ::com::sun::star::uno::Reference< INNER >   m_xInner;

    protected:
        // XInterface
        ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& _rType ) throw (::com::sun::star::uno::RuntimeException);
        // XTypeProvider
        DECLARE_XTYPEPROVIDER( );

    protected:
        OComponentProxyAggregation(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            const ::com::sun::star::uno::Reference< INNER >& _rxInner
        );
        ~OComponentProxyAggregation( );

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

        // OComponentHelper
        virtual void SAL_CALL disposing()  throw (::com::sun::star::uno::RuntimeException);

    protected:
        // be called from within the dtor of derived classes
        void implEnsureDisposeInDtor( );
    };

    //-------------------------------------------------------------------------
    template < class INNER >
    void OComponentProxyAggregation< INNER >::implEnsureDisposeInDtor( )
    {
        if ( !rBHelper.bDisposed )
        {
            acquire();  // to prevent duplicate dtor calls
            dispose();
        }
    }

    //-------------------------------------------------------------------------
    template < class INNER >
    ::com::sun::star::uno::Any SAL_CALL OComponentProxyAggregation< INNER >::queryInterface( const ::com::sun::star::uno::Type& _rType ) throw (::com::sun::star::uno::RuntimeException)
    {
        ::com::sun::star::uno::Any aReturn( BASE::queryInterface( _rType ) );
        if ( !aReturn.hasValue() )
            aReturn = OProxyAggregation::queryAggregation( _rType );
        return aReturn;
    }

    //-------------------------------------------------------------------------
    template < class INNER >
    IMPLEMENT_GET_IMPLEMENTATION_ID( OComponentProxyAggregation< INNER > )

    //-------------------------------------------------------------------------
    template < class INNER >
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL OComponentProxyAggregation< INNER >::getTypes(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        return ::comphelper::concatSequences(
            BASE::getTypes(),
            OProxyAggregation::getTypes()
        );
    }

    //-------------------------------------------------------------------------
    template < class INNER >
    OComponentProxyAggregation< INNER >::OComponentProxyAggregation( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
        const ::com::sun::star::uno::Reference< INNER >& _rxInner )
        :BASE( m_aMutex )
        ,OProxyAggregation( _rxORB )
        ,m_xInner( _rxInner )
    {
        OSL_ENSURE( _rxORB.is() && m_xInner.is(), "OComponentProxyAggregation::OComponentProxyAggregation: invalid arguments!" );

        // aggregate a proxy for the object
        aggregateProxyFor( m_xInner.get(), m_refCount, *this );

        // add as event listener to the inner context, because we want to be notified of disposals
        osl_incrementInterlockedCount( &m_refCount );
        {
            Reference< XComponent > xComp( m_xInner, UNO_QUERY );
            if ( xComp.is() )
                xComp->addEventListener( this );
        }
        osl_decrementInterlockedCount( &m_refCount );
    }

    //-------------------------------------------------------------------------
    template < class INNER >
    OComponentProxyAggregation< INNER >::~OComponentProxyAggregation( )
    {
        implEnsureDisposeInDtor();
        m_xInner.clear();
    }

    //-------------------------------------------------------------------------
    template < class INNER >
    void SAL_CALL OComponentProxyAggregation< INNER >::disposing( const ::com::sun::star::lang::EventObject& _rSource ) throw (::com::sun::star::uno::RuntimeException)
    {
        if ( _rSource.Source == m_xInner )
        {   // it's our inner context which is dying -> dispose ourself
            if ( !rBHelper.bDisposed && !rBHelper.bInDispose )
            {   // (if necessary only, of course)
                dispose();
            }
        }
        else
            OSL_ENSURE( sal_False, "OComponentProxyAggregation::disposing(EventObject): where did this come from?" );
    }

    //-------------------------------------------------------------------------
    template < class INNER >
    void SAL_CALL OComponentProxyAggregation< INNER >::disposing()  throw (::com::sun::star::uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // dispose our inner context
        // before we do this, remove ourself as listener - else in disposing( EventObject ), we
        // would dispose ourself a second time
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > xComp( m_xInner, ::com::sun::star::uno::UNO_QUERY );
        if ( xComp.is() )
        {
            xComp->removeEventListener( this );
            xComp->dispose();
            xComp.clear();
        }
    }

    //=========================================================================
    //= OAccessibleWrapper
    //=========================================================================

    class OAccessibleContextWrapper;

    typedef OComponentProxyAggregation  <   ::drafts::com::sun::star::accessibility::XAccessible
                                        >   OAccessibleWrapper_Base;

    /** a class which aggregates a proxy for an XAccessible, and wrapping the context returned by this
        XAccessible.
    */
    class OAccessibleWrapper : public OAccessibleWrapper_Base
    {
    private:
        ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible >
                m_xParentAccessible;
        ::com::sun::star::uno::WeakReference< ::drafts::com::sun::star::accessibility::XAccessibleContext >
                m_aContext;

    public:
        /** ctor

            @param _rxORB
                a service factory

            @param _rxInnerAccessible
                the object to wrap

            @param _rxParentAccessible
                The XAccessible which is our parent
        */
        OAccessibleWrapper(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible >& _rxInnerAccessible,
            const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible >& _rxParentAccessible
        );

        // returns the context without creating it
        ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessibleContext >
                    getContextNoCreate( ) const;

    protected:
        virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessibleContext > SAL_CALL
                    getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException);

        ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible >
                    getParent() const { return m_xParentAccessible; }

        // overridables
        virtual OAccessibleContextWrapper* createAccessibleContext(
                const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessibleContext >& _rxInnerContext
            );

    protected:
        ~OAccessibleWrapper( );

    private:
        OAccessibleWrapper( );                                      // never implemented
        OAccessibleWrapper( const OAccessibleWrapper& );            // never implemented
        OAccessibleWrapper& operator=( const OAccessibleWrapper& ); // never implemented
    };

    //=========================================================================
    //= OAccessibleContextWrapper
    //=========================================================================

    // for aggregating another context
    typedef OComponentProxyAggregation  <   ::drafts::com::sun::star::accessibility::XAccessibleContext
                                        >   OAccessibleContextWrapper_Base;

    // for multiplexing AccessibleEvents
    typedef ::cppu::ImplHelper2 <   ::drafts::com::sun::star::accessibility::XAccessibleEventBroadcaster
                                ,   ::drafts::com::sun::star::accessibility::XAccessibleEventListener
                                >   OAccessibleContextWrapper_MBase;

    /** wraps an XAccessibleContext by aggregating a proxy for it
    */
    class OAccessibleContextWrapper
                    :public OAccessibleContextWrapper_Base
                    ,public OAccessibleContextWrapper_MBase
    {
    private:
        typedef ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible >
                                Accessible;

    private:
        /// the XAccessible which created this context
        ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible >    m_xOwningAccessible;
        /// the XAccessible which is to be returned in getAccessibleParent
        ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible >    m_xParentAccessible;

        ::comphelper::AccessibleEventNotifier::TClientId    m_nNotifierClient;      // for notifying AccessibleEvents
        OWrappedAccessibleChildrenManager*                  m_pChildMapper;         // for mapping children from our inner context to our callers

    public:
        /** ctor

            @param _rxORB
                a service factory

            @param _rxInnerAccessibleContext
                the object to wrap

            @param _rxOwningAccessible
                The XAccessible which created this object. This is necessary because children
                of our wrapped context meed to be wrapped, too, and if they're asked for a parent,
                they of course should return the proper parent<br/>
                The object will be held with a hard reference

            @param _rxParentAccessible
                The XAccessible to return in the getAccessibleParent call
        */
        OAccessibleContextWrapper(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessibleContext >& _rxInnerAccessibleContext,
            const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible >& _rxOwningAccessible,
            const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible >& _rxParentAccessible
        );

    protected:
        // XInterface
        DECLARE_XINTERFACE( )
        // XTypeProvider
        DECLARE_XTYPEPROVIDER( )

        // XAccessibleContext
        virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getAccessibleDescription(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getAccessibleName(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::lang::Locale SAL_CALL getLocale(  ) throw (::drafts::com::sun::star::accessibility::IllegalAccessibleComponentStateException, ::com::sun::star::uno::RuntimeException);

        // XAccessibleEventBroadcaster
        virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessibleEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessibleEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);

        // XAccessibleEventListener
        virtual void SAL_CALL notifyEvent( const ::drafts::com::sun::star::accessibility::AccessibleEventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

        // OComponentHelper
        virtual void SAL_CALL disposing()  throw (::com::sun::star::uno::RuntimeException);

    protected:
        ~OAccessibleContextWrapper( );

        OAccessibleContextWrapper( );                                               // never implemented
        OAccessibleContextWrapper( const OAccessibleContextWrapper& );              // never implemented
        OAccessibleContextWrapper& operator=( const OAccessibleContextWrapper& );   // never implemented
    };

//.............................................................................
}   // namespace accessibility
//.............................................................................

#endif // COMPHELPER_ACCESSIBLE_WRAPPER_HXX
