/*************************************************************************
 *
 *  $RCSfile: accessiblewrapper.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: vg $ $Date: 2003-05-19 12:56:44 $
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

#ifndef COMPHELPER_PROXY_AGGREGATION
#include <comphelper/proxyaggregation.hxx>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HPP_
#include <com/sun/star/accessibility/XAccessible.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLECONTEXT_HPP_
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEEVENTBROADCASTER_HPP_
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEEVENTLISTENER_HPP_
#include <com/sun/star/accessibility/XAccessibleEventListener.hpp>
#endif
#ifndef _CPPUHELPER_COMPBASE3_HXX_
#include <cppuhelper/compbase3.hxx>
#endif
#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase2.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
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

    //=========================================================================
    //= OAccessibleWrapper
    //=========================================================================

    class OAccessibleContextWrapper;
    class OWrappedAccessibleChildrenManager;

    typedef ::cppu::ImplHelper1 <   ::com::sun::star::accessibility::XAccessible
                                >   OAccessibleWrapper_Base;

    /** a class which aggregates a proxy for an XAccessible, and wrapping the context returned by this
        XAccessible.
    */
    class OAccessibleWrapper:public OAccessibleWrapper_Base
                            ,public OComponentProxyAggregation

    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                m_xParentAccessible;
        ::com::sun::star::uno::WeakReference< ::com::sun::star::accessibility::XAccessibleContext >
                m_aContext;

    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                m_xInnerAccessible;

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
            const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _rxInnerAccessible,
            const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _rxParentAccessible
        );
        DECLARE_XINTERFACE()
        DECLARE_XTYPEPROVIDER()

        // returns the context without creating it
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
                    getContextNoCreate( ) const;

    protected:
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL
                    getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException);

        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                    getParent() const { return m_xParentAccessible; }

        // own overridables
        virtual OAccessibleContextWrapper* createAccessibleContext(
                const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >& _rxInnerContext
            );

    protected:
        ~OAccessibleWrapper( );

    private:
        OAccessibleWrapper( );                                      // never implemented
        OAccessibleWrapper( const OAccessibleWrapper& );            // never implemented
        OAccessibleWrapper& operator=( const OAccessibleWrapper& ); // never implemented
    };

    //=========================================================================
    //= OAccessibleContextWrapperHelper
    //=========================================================================

    typedef ::cppu::ImplHelper1 <   ::com::sun::star::accessibility::XAccessibleEventListener
                                >   OAccessibleContextWrapperHelper_Base;

    /** Helper for wrapping an XAccessibleContext by aggregating a proxy for it.

        <p>This class does not have own ref counting. In addition, it does not implement
        the XAccesibleContext interface, but provides all the methods from this interface
        which must be implemented using the inner context (such as getAccessibleChild*).</p>

        <p>Children of the aggregated XAccessibleContext are wrapped, too.</p>

        <p>AccessibleEvents fired by the inner context are multiplexed, especially, any references to
        children in such events are translated. This means that even in such events, no un-wrapped object
        will ever leave this class - if the aggregated context notifies an child event, the child passed
        to the event is wrapped</p>

        @seealso OAccessibleContextWrapper
    */
    class OAccessibleContextWrapperHelper
                :private OComponentProxyAggregationHelper
                ,public OAccessibleContextWrapperHelper_Base
    {
    protected:
        /// the context we're wrapping (properly typed, in opposite to OComponentProxyAggregationHelper::m_xInner)
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
                                                            m_xInnerContext;
        /// the XAccessible which created this context
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                                                            m_xOwningAccessible;
        /// the XAccessible which is to be returned in getAccessibleParent
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                                                            m_xParentAccessible;

        OWrappedAccessibleChildrenManager*                  m_pChildMapper;         // for mapping children from our inner context to our callers

    protected:
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
        OAccessibleContextWrapperHelper(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            ::cppu::OBroadcastHelper& _rBHelper,
            const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >& _rxInnerAccessibleContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _rxOwningAccessible,
            const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _rxParentAccessible
        );

        /// to be called from within your ctor - does the aggregation of a proxy for m_xInnerContext
        void aggregateProxy(
            oslInterlockedCount& _rRefCount,
            ::cppu::OWeakObject& _rDelegator
        );

    protected:
        // XInterface
        ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& _rType ) throw (::com::sun::star::uno::RuntimeException);
        // still waiting to be overwritten
        virtual void SAL_CALL acquire(  ) throw () = 0;
        virtual void SAL_CALL release(  ) throw () = 0;

        // XTypeProvider
        DECLARE_XTYPEPROVIDER( )

        // XAccessibleContext
        virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (::com::sun::star::uno::RuntimeException);

        // XAccessibleEventListener
        virtual void SAL_CALL notifyEvent( const ::com::sun::star::accessibility::AccessibleEventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

        // XComponent/OComponentProxyAggregationHelper
        virtual void SAL_CALL dispose() throw( ::com::sun::star::uno::RuntimeException );

        // own overridables
        /** notify an accessible event which has been translated (if necessary)

            <p>Usually, you derive your clas from both OAccessibleContextWrapperHelper and XAccessibleEventBroadcaster,
            and simply call all your XAccessibleEventListener with the given event.</p>

            <p>The mutex of the BroadcastHelper passed to the instance's ctor is <em>not</em> locked when calling
            into this method</p>
        */
        virtual void notifyTranslatedEvent( const ::com::sun::star::accessibility::AccessibleEventObject& _rEvent ) throw (::com::sun::star::uno::RuntimeException) = 0;

    protected:
        ~OAccessibleContextWrapperHelper( );

        OAccessibleContextWrapperHelper( );                                             // never implemented
        OAccessibleContextWrapperHelper( const OAccessibleContextWrapperHelper& );              // never implemented
        OAccessibleContextWrapperHelper& operator=( const OAccessibleContextWrapperHelper& );   // never implemented
    };

    //=========================================================================
    //= OAccessibleContextWrapper
    //=========================================================================
    typedef ::cppu::WeakComponentImplHelper2<   ::com::sun::star::accessibility::XAccessibleEventBroadcaster
                                            ,   ::com::sun::star::accessibility::XAccessibleContext
                                            >   OAccessibleContextWrapper_CBase;

    class OAccessibleContextWrapper
                    :public OBaseMutex
                    ,public OAccessibleContextWrapper_CBase
                    ,public OAccessibleContextWrapperHelper
    {
    private:
        ::comphelper::AccessibleEventNotifier::TClientId    m_nNotifierClient;      // for notifying AccessibleEvents

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
            const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >& _rxInnerAccessibleContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _rxOwningAccessible,
            const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _rxParentAccessible
        );

        // XInterface
        DECLARE_XINTERFACE( )
        // XTypeProvider
        DECLARE_XTYPEPROVIDER( )

        // XAccessibleContext
        virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getAccessibleDescription(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getAccessibleName(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::lang::Locale SAL_CALL getLocale(  ) throw (::com::sun::star::accessibility::IllegalAccessibleComponentStateException, ::com::sun::star::uno::RuntimeException);

        // XAccessibleEventBroadcaster
        virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);

        // OAccessibleContextWrapper
        virtual void notifyTranslatedEvent( const ::com::sun::star::accessibility::AccessibleEventObject& _rEvent ) throw (::com::sun::star::uno::RuntimeException);

        // XComponent/OComponentProxyAggregationHelper
        virtual void SAL_CALL dispose() throw( ::com::sun::star::uno::RuntimeException );

        // OComponentHelper
        virtual void SAL_CALL disposing()  throw (::com::sun::star::uno::RuntimeException);

    protected:
        virtual ~OAccessibleContextWrapper();

    private:
        OAccessibleContextWrapper();                                                // never implemented
        OAccessibleContextWrapper( const OAccessibleContextWrapper& );              // never implemented
        OAccessibleContextWrapper& operator=( const OAccessibleContextWrapper& );   // never implemented
    };

    //=========================================================================
    //= OWrappedAccessibleChildrenManager
    //=========================================================================

    typedef ::std::map  <   ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                        ,   ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                        ,   OInterfaceCompare< ::com::sun::star::accessibility::XAccessible >
                        >   AccessibleMap;
                        // TODO: think about if we should hold these objects weak

    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::lang::XEventListener
                                    >   OWrappedAccessibleChildrenManager_Base;
    /** manages wrapping XAccessible's to XAccessible's
    */
    class OWrappedAccessibleChildrenManager : public OWrappedAccessibleChildrenManager_Base
    {
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                                m_xORB;
        ::com::sun::star::uno::WeakReference< ::com::sun::star::accessibility::XAccessible >
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
        void    setOwningAccessible( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _rxAcc );

        /// retrieves a wrapper for the given accessible
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                getAccessibleWrapperFor(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _rxKey,
                    sal_Bool _bCreate = sal_True
                );

        /// erases the given key from the map (if it is present there)
        void    removeFromCache( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _rxKey );

        /// invalidates (i.e. empties) the map
        void    invalidateAll( );

        /** disposes (i.e. cleares) the manager

            <p>Note that the XAccessibleContext's of the mapped XAccessible objects are disposed, too.</p>
        */
        void    dispose();

        /** handles a notification as got from the parent of the children we're managing
            <p>This applies only to the notifications which have a direct impact on our map.</p>
        */
        void    handleChildNotification( const ::com::sun::star::accessibility::AccessibleEventObject& _rEvent );

        /** translates events as got from the parent of the children we're managing
            <p>This applies only to the notifications which deal with child objects which we manage.</p>
        */
        void    translateAccessibleEvent(
            const   ::com::sun::star::accessibility::AccessibleEventObject& _rEvent,
                    ::com::sun::star::accessibility::AccessibleEventObject& _rTranslatedEvent
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

//.............................................................................
}   // namespace accessibility
//.............................................................................

#endif // COMPHELPER_ACCESSIBLE_WRAPPER_HXX
