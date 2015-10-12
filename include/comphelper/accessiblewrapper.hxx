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

#ifndef INCLUDED_COMPHELPER_ACCESSIBLEWRAPPER_HXX
#define INCLUDED_COMPHELPER_ACCESSIBLEWRAPPER_HXX

#include <sal/config.h>

#include <map>

#include <comphelper/proxyaggregation.hxx>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/accessibility/XAccessibleEventListener.hpp>
#include <cppuhelper/compbase.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/uno3.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <comphelper/accessibleeventnotifier.hxx>
#include <comphelper/stl_types.hxx>
#include <comphelper/comphelperdllapi.h>


namespace comphelper
{



    //= OAccessibleWrapper


    class OAccessibleContextWrapper;
    class OWrappedAccessibleChildrenManager;

    struct OAccessibleWrapper_Base :
        public ::cppu::ImplHelper1 < ::com::sun::star::accessibility::XAccessible >
    {
    protected:
        ~OAccessibleWrapper_Base() {}
    };

    /** a class which aggregates a proxy for an XAccessible, and wrapping the context returned by this
        XAccessible.
    */
    class COMPHELPER_DLLPUBLIC OAccessibleWrapper:public OAccessibleWrapper_Base
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
            @param _rxContext
                a service factory

            @param _rxInnerAccessible
                the object to wrap

            @param _rxParentAccessible
                The XAccessible which is our parent
        */
        OAccessibleWrapper(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext,
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
                    getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                    getParent() const { return m_xParentAccessible; }

        // own overridables
        virtual OAccessibleContextWrapper* createAccessibleContext(
                const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >& _rxInnerContext
            );

    protected:
        virtual ~OAccessibleWrapper( );

    private:
        OAccessibleWrapper( const OAccessibleWrapper& ) = delete;
        OAccessibleWrapper& operator=( const OAccessibleWrapper& ) = delete;
    };


    //= OAccessibleContextWrapperHelper


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
    class COMPHELPER_DLLPUBLIC OAccessibleContextWrapperHelper
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

            @param _rxContext
                a service factory

            @param _rxInnerAccessibleContext
                the object to wrap

            @param _rxOwningAccessible
                The XAccessible which created this object. This is necessary because children
                of our wrapped context need to be wrapped, too, and if they're asked for a parent,
                they of course should return the proper parent<br/>
                The object will be held with a hard reference

            @param _rxParentAccessible
                The XAccessible to return in the getAccessibleParent call
        */
        OAccessibleContextWrapperHelper(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext,
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
        ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& _rType ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XTypeProvider
        DECLARE_XTYPEPROVIDER( )

        // XAccessibleContext
        sal_Int32 baseGetAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > baseGetAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception);
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet > baseGetAccessibleRelationSet(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);

        // XAccessibleEventListener
        virtual void SAL_CALL notifyEvent( const ::com::sun::star::accessibility::AccessibleEventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XComponent/OComponentProxyAggregationHelper
        virtual void SAL_CALL dispose() throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        // own overridables
        /** notify an accessible event which has been translated (if necessary)

            <p>Usually, you derive your class from both OAccessibleContextWrapperHelper and XAccessibleEventBroadcaster,
            and simply call all your XAccessibleEventListener with the given event.</p>

            <p>The mutex of the BroadcastHelper passed to the instance's ctor is <em>not</em> locked when calling
            into this method</p>
        */
        virtual void notifyTranslatedEvent( const ::com::sun::star::accessibility::AccessibleEventObject& _rEvent ) throw (::com::sun::star::uno::RuntimeException) = 0;

    protected:
        virtual ~OAccessibleContextWrapperHelper( );

        OAccessibleContextWrapperHelper( );                                             // never implemented
        OAccessibleContextWrapperHelper( const OAccessibleContextWrapperHelper& );              // never implemented
        OAccessibleContextWrapperHelper& operator=( const OAccessibleContextWrapperHelper& );   // never implemented
    };


    //= OAccessibleContextWrapper

    typedef ::cppu::WeakComponentImplHelper<    ::com::sun::star::accessibility::XAccessibleEventBroadcaster
                                            ,   ::com::sun::star::accessibility::XAccessibleContext
                                            >   OAccessibleContextWrapper_CBase;

    class COMPHELPER_DLLPUBLIC OAccessibleContextWrapper
                    :public OBaseMutex
                    ,public OAccessibleContextWrapper_CBase
                    ,public OAccessibleContextWrapperHelper
    {
    private:
        ::comphelper::AccessibleEventNotifier::TClientId    m_nNotifierClient;      // for notifying AccessibleEvents

    public:
        /** ctor

            @param _rxContext
                a service factory

            @param _rxInnerAccessibleContext
                the object to wrap

            @param _rxOwningAccessible
                The XAccessible which created this object. This is necessary because children
                of our wrapped context need to be wrapped, too, and if they're asked for a parent,
                they of course should return the proper parent<br/>
                The object will be held with a hard reference

            @param _rxParentAccessible
                The XAccessible to return in the getAccessibleParent call
        */
        OAccessibleContextWrapper(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >& _rxInnerAccessibleContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _rxOwningAccessible,
            const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _rxParentAccessible
        );

        // XInterface
        DECLARE_XINTERFACE( )
        // XTypeProvider
        DECLARE_XTYPEPROVIDER( )

        // XAccessibleContext
        virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual OUString SAL_CALL getAccessibleDescription(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual OUString SAL_CALL getAccessibleName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::lang::Locale SAL_CALL getLocale(  ) throw (::com::sun::star::accessibility::IllegalAccessibleComponentStateException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XAccessibleEventBroadcaster
        virtual void SAL_CALL addAccessibleEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL removeAccessibleEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // OAccessibleContextWrapper
        virtual void notifyTranslatedEvent( const ::com::sun::star::accessibility::AccessibleEventObject& _rEvent ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;

        // OComponentHelper
        using OAccessibleContextWrapperHelper::disposing;
        virtual void SAL_CALL disposing()  throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;

    protected:
        virtual ~OAccessibleContextWrapper();

    private:
        OAccessibleContextWrapper( const OAccessibleContextWrapper& ) = delete;
        OAccessibleContextWrapper& operator=( const OAccessibleContextWrapper& ) = delete;
    };


    //= OWrappedAccessibleChildrenManager


    typedef ::std::map  <   ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                        ,   ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                        ,   OInterfaceCompare< ::com::sun::star::accessibility::XAccessible >
                        >   AccessibleMap;
                        // TODO: think about if we should hold these objects weak

    typedef ::cppu::WeakImplHelper<   ::com::sun::star::lang::XEventListener
                                  >   OWrappedAccessibleChildrenManager_Base;
    /** manages wrapping XAccessible's to XAccessible's
    */
    class COMPHELPER_DLLPUBLIC OWrappedAccessibleChildrenManager : public OWrappedAccessibleChildrenManager_Base
    {
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
                                m_xContext;
        ::com::sun::star::uno::WeakReference< ::com::sun::star::accessibility::XAccessible >
                                m_aOwningAccessible;    // the XAccessible which belongs to the XAccessibleContext which we work for
        AccessibleMap           m_aChildrenMap;         // for caching children
        bool                m_bTransientChildren;   // are we prohibited to cache our children?

    public:
        /// ctor
        OWrappedAccessibleChildrenManager(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext
        );

        /** specifies if the children are to be consideren transient (i.e.: not cached)
            <p>to be called only once per lifetime</p>
        */
        void    setTransientChildren( bool _bSet = true );

        /** sets the XAccessible which belongs to the XAccessibleContext which we work for
            <p>to be called only once per lifetime</p>
        */
        void    setOwningAccessible( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _rxAcc );

        /// retrieves a wrapper for the given accessible
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                getAccessibleWrapperFor(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _rxKey,
                    bool _bCreate = true
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
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    protected:
        void    implTranslateChildEventValue( const ::com::sun::star::uno::Any& _rInValue, ::com::sun::star::uno::Any& _rOutValue );

    protected:
        virtual ~OWrappedAccessibleChildrenManager( );

    private:
        OWrappedAccessibleChildrenManager( const OWrappedAccessibleChildrenManager& ) = delete;
        OWrappedAccessibleChildrenManager& operator=( const OWrappedAccessibleChildrenManager& ) = delete;
    };


}   // namespace accessibility


#endif // INCLUDED_COMPHELPER_ACCESSIBLEWRAPPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
