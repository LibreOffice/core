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
#include <cppuhelper/basemutex.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/uno3.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <comphelper/accessibleeventnotifier.hxx>
#include <comphelper/stl_types.hxx>
#include <comphelper/comphelperdllapi.h>
#include <rtl/ref.hxx>


namespace comphelper
{


    //= OAccessibleWrapper


    class OAccessibleContextWrapper;
    class OWrappedAccessibleChildrenManager;

    struct OAccessibleWrapper_Base :
        public ::cppu::ImplHelper < css::accessibility::XAccessible >
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
        css::uno::Reference< css::accessibility::XAccessible >
                m_xParentAccessible;
        css::uno::WeakReference< css::accessibility::XAccessibleContext >
                m_aContext;

    protected:
        css::uno::Reference< css::accessibility::XAccessible >
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
            const css::uno::Reference< css::uno::XComponentContext >&     _rxContext,
            const css::uno::Reference< css::accessibility::XAccessible >& _rxInnerAccessible,
            const css::uno::Reference< css::accessibility::XAccessible >& _rxParentAccessible
        );
        DECLARE_XINTERFACE()
        DECLARE_XTYPEPROVIDER()

        // returns the context without creating it
        css::uno::Reference< css::accessibility::XAccessibleContext >
                    getContextNoCreate( ) const;

    protected:
        virtual css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL
                    getAccessibleContext(  ) throw (css::uno::RuntimeException, std::exception) override;

        const css::uno::Reference< css::accessibility::XAccessible >&
                    getParent() const { return m_xParentAccessible; }

        // own overridables
        virtual OAccessibleContextWrapper* createAccessibleContext(
                const css::uno::Reference< css::accessibility::XAccessibleContext >& _rxInnerContext
            );

    protected:
        virtual ~OAccessibleWrapper( ) override;

    private:
        OAccessibleWrapper( const OAccessibleWrapper& ) = delete;
        OAccessibleWrapper& operator=( const OAccessibleWrapper& ) = delete;
    };


    //= OAccessibleContextWrapperHelper


    typedef ::cppu::ImplHelper  <   css::accessibility::XAccessibleEventListener
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
        css::uno::Reference< css::accessibility::XAccessibleContext >
                                                            m_xInnerContext;
        /// the XAccessible which created this context
        css::uno::Reference< css::accessibility::XAccessible >
                                                            m_xOwningAccessible;
        /// the XAccessible which is to be returned in getAccessibleParent
        css::uno::Reference< css::accessibility::XAccessible >
                                                            m_xParentAccessible;

        rtl::Reference<OWrappedAccessibleChildrenManager>   m_xChildMapper;     // for mapping children from our inner context to our callers

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
            const css::uno::Reference< css::uno::XComponentContext >& _rxContext,
            ::cppu::OBroadcastHelper& _rBHelper,
            const css::uno::Reference< css::accessibility::XAccessibleContext >& _rxInnerAccessibleContext,
            const css::uno::Reference< css::accessibility::XAccessible >& _rxOwningAccessible,
            const css::uno::Reference< css::accessibility::XAccessible >& _rxParentAccessible
        );

        /// to be called from within your ctor - does the aggregation of a proxy for m_xInnerContext
        void aggregateProxy(
            oslInterlockedCount& _rRefCount,
            ::cppu::OWeakObject& _rDelegator
        );

    protected:
        // XInterface
        css::uno::Any SAL_CALL queryInterface( const css::uno::Type& _rType ) throw (css::uno::RuntimeException, std::exception) override;

        // XTypeProvider
        DECLARE_XTYPEPROVIDER( )

        // XAccessibleContext
        sal_Int32 baseGetAccessibleChildCount(  ) throw (css::uno::RuntimeException, std::exception);
        css::uno::Reference< css::accessibility::XAccessible > baseGetAccessibleChild( sal_Int32 i ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception);
        css::uno::Reference< css::accessibility::XAccessibleRelationSet > baseGetAccessibleRelationSet(  ) throw (css::uno::RuntimeException, std::exception);

        // XAccessibleEventListener
        virtual void SAL_CALL notifyEvent( const css::accessibility::AccessibleEventObject& aEvent ) throw (css::uno::RuntimeException, std::exception) override;

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw (css::uno::RuntimeException, std::exception) override;

        // XComponent/OComponentProxyAggregationHelper
        virtual void SAL_CALL dispose() throw( css::uno::RuntimeException, std::exception ) override;

        // own overridables
        /** notify an accessible event which has been translated (if necessary)

            <p>Usually, you derive your class from both OAccessibleContextWrapperHelper and XAccessibleEventBroadcaster,
            and simply call all your XAccessibleEventListener with the given event.</p>

            <p>The mutex of the BroadcastHelper passed to the instance's ctor is <em>not</em> locked when calling
            into this method</p>
        */
        virtual void notifyTranslatedEvent( const css::accessibility::AccessibleEventObject& _rEvent ) throw (css::uno::RuntimeException) = 0;

    protected:
        virtual ~OAccessibleContextWrapperHelper( ) override;

        OAccessibleContextWrapperHelper(const OAccessibleContextWrapperHelper&) = delete;
        OAccessibleContextWrapperHelper& operator=(const OAccessibleContextWrapperHelper&) = delete;
    };


    //= OAccessibleContextWrapper

    typedef ::cppu::WeakComponentImplHelper<    css::accessibility::XAccessibleEventBroadcaster
                                            ,   css::accessibility::XAccessibleContext
                                            >   OAccessibleContextWrapper_CBase;

    class COMPHELPER_DLLPUBLIC OAccessibleContextWrapper
                    :public cppu::BaseMutex
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
            const css::uno::Reference< css::uno::XComponentContext >& _rxContext,
            const css::uno::Reference< css::accessibility::XAccessibleContext >& _rxInnerAccessibleContext,
            const css::uno::Reference< css::accessibility::XAccessible >& _rxOwningAccessible,
            const css::uno::Reference< css::accessibility::XAccessible >& _rxParentAccessible
        );

        // XInterface
        DECLARE_XINTERFACE( )
        // XTypeProvider
        DECLARE_XTYPEPROVIDER( )

        // XAccessibleContext
        virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getAccessibleDescription(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getAccessibleName(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::lang::Locale SAL_CALL getLocale(  ) throw (css::accessibility::IllegalAccessibleComponentStateException, css::uno::RuntimeException, std::exception) override;

        // XAccessibleEventBroadcaster
        virtual void SAL_CALL addAccessibleEventListener( const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeAccessibleEventListener( const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;

        // OAccessibleContextWrapper
        virtual void notifyTranslatedEvent( const css::accessibility::AccessibleEventObject& _rEvent ) throw (css::uno::RuntimeException) override;

        // OComponentHelper
        using OAccessibleContextWrapperHelper::disposing;
        virtual void SAL_CALL disposing()  throw (css::uno::RuntimeException) override;

    protected:
        virtual ~OAccessibleContextWrapper() override;

    private:
        OAccessibleContextWrapper( const OAccessibleContextWrapper& ) = delete;
        OAccessibleContextWrapper& operator=( const OAccessibleContextWrapper& ) = delete;
    };


    //= OWrappedAccessibleChildrenManager


    typedef ::std::map  <   css::uno::Reference< css::accessibility::XAccessible >
                        ,   css::uno::Reference< css::accessibility::XAccessible >
                        ,   OInterfaceCompare< css::accessibility::XAccessible >
                        >   AccessibleMap;
                        // TODO: think about if we should hold these objects weak

    typedef ::cppu::WeakImplHelper<   css::lang::XEventListener
                                  >   OWrappedAccessibleChildrenManager_Base;
    /** manages wrapping XAccessible's to XAccessible's
    */
    class COMPHELPER_DLLPUBLIC OWrappedAccessibleChildrenManager : public OWrappedAccessibleChildrenManager_Base
    {
    protected:
        css::uno::Reference< css::uno::XComponentContext >
                                m_xContext;
        css::uno::WeakReference< css::accessibility::XAccessible >
                                m_aOwningAccessible;    // the XAccessible which belongs to the XAccessibleContext which we work for
        AccessibleMap           m_aChildrenMap;         // for caching children
        bool                    m_bTransientChildren;   // are we prohibited to cache our children?

    public:
        /// ctor
        OWrappedAccessibleChildrenManager(
            const css::uno::Reference< css::uno::XComponentContext >& _rxContext
        );

        /** specifies if the children are to be considered transient (i.e.: not cached)
            <p>to be called only once per lifetime</p>
        */
        void    setTransientChildren( bool _bSet );

        /** sets the XAccessible which belongs to the XAccessibleContext which we work for
            <p>to be called only once per lifetime</p>
        */
        void    setOwningAccessible( const css::uno::Reference< css::accessibility::XAccessible >& _rxAcc );

        /// retrieves a wrapper for the given accessible
        css::uno::Reference< css::accessibility::XAccessible >
                getAccessibleWrapperFor(
                    const css::uno::Reference< css::accessibility::XAccessible >& _rxKey
                );

        /// erases the given key from the map (if it is present there)
        void    removeFromCache( const css::uno::Reference< css::accessibility::XAccessible >& _rxKey );

        /// invalidates (i.e. empties) the map
        void    invalidateAll( );

        /** disposes (i.e. cleares) the manager

            <p>Note that the XAccessibleContext's of the mapped XAccessible objects are disposed, too.</p>
        */
        void    dispose();

        /** handles a notification as got from the parent of the children we're managing
            <p>This applies only to the notifications which have a direct impact on our map.</p>
        */
        void    handleChildNotification( const css::accessibility::AccessibleEventObject& _rEvent );

        /** translates events as got from the parent of the children we're managing
            <p>This applies only to the notifications which deal with child objects which we manage.</p>
        */
        void    translateAccessibleEvent(
            const   css::accessibility::AccessibleEventObject& _rEvent,
                    css::accessibility::AccessibleEventObject& _rTranslatedEvent
        );

    protected:
        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw (css::uno::RuntimeException, std::exception) override;

    protected:
        void    implTranslateChildEventValue( const css::uno::Any& _rInValue, css::uno::Any& _rOutValue );

    protected:
        virtual ~OWrappedAccessibleChildrenManager( ) override;

    private:
        OWrappedAccessibleChildrenManager( const OWrappedAccessibleChildrenManager& ) = delete;
        OWrappedAccessibleChildrenManager& operator=( const OWrappedAccessibleChildrenManager& ) = delete;
    };


}   // namespace accessibility


#endif // INCLUDED_COMPHELPER_ACCESSIBLEWRAPPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
