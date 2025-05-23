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

#pragma once

#include <config_options.h>
#include <sal/config.h>

#include "proxyaggregation.hxx"

#include <map>

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/accessibility/XAccessibleEventListener.hpp>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/basemutex.hxx>
#include <comphelper/uno3.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <comphelper/accessibleeventnotifier.hxx>
#include <rtl/ref.hxx>
#include <unotools/weakref.hxx>

namespace com::sun::star::uno { class XComponentContext; }

namespace accessibility
{

//= OAccessibleWrapper

class OAccessibleContextWrapper;
class OWrappedAccessibleChildrenManager;

using OAccessibleWrapper_Base = cppu::ImplHelper1<css::accessibility::XAccessible>;

/** a class which aggregates a proxy for an XAccessible, and wrapping the context returned by this
    XAccessible.
*/
class OAccessibleWrapper final : public OAccessibleWrapper_Base, public OComponentProxyAggregation

{
private:
    css::uno::Reference< css::accessibility::XAccessible >
            m_xParentAccessible;
    unotools::WeakReference<OAccessibleContextWrapper> m_aContext;
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
                getAccessibleContext(  ) override final;

protected:
    virtual ~OAccessibleWrapper( ) override;

private:
    OAccessibleWrapper( const OAccessibleWrapper& ) = delete;
    OAccessibleWrapper& operator=( const OAccessibleWrapper& ) = delete;
};


//= OAccessibleContextWrapperHelper


typedef ::cppu::ImplHelper1 <   css::accessibility::XAccessibleEventListener
                            >   OAccessibleContextWrapperHelper_Base;

/** Helper for wrapping an XAccessibleContext by aggregating a proxy for it.

    <p>This class does not have own ref counting. In addition, it does not implement
    the XAccessibleContext interface, but provides all the methods from this interface
    which must be implemented using the inner context (such as getAccessibleChild*).</p>

    <p>Children of the aggregated XAccessibleContext are wrapped, too.</p>

    <p>AccessibleEvents fired by the inner context are multiplexed, especially, any references to
    children in such events are translated. This means that even in such events, no un-wrapped object
    will ever leave this class - if the aggregated context notifies a child event, the child passed
    to the event is wrapped</p>

    @seealso OAccessibleContextWrapper
*/
class OAccessibleContextWrapperHelper : private OComponentProxyAggregationHelper,
                                        public OAccessibleContextWrapperHelper_Base
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
    css::uno::Any SAL_CALL queryInterface( const css::uno::Type& _rType ) override;

    // XTypeProvider
    DECLARE_XTYPEPROVIDER( )

    // XAccessibleEventListener
    virtual void SAL_CALL notifyEvent( const css::accessibility::AccessibleEventObject& aEvent ) override final;

    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

    // XComponent/OComponentProxyAggregationHelper
    virtual void SAL_CALL dispose() override;

    // own overridables
    /** notify an accessible event which has been translated (if necessary)

        <p>Usually, you derive your class from both OAccessibleContextWrapperHelper and XAccessibleEventBroadcaster,
        and simply call all your XAccessibleEventListener with the given event.</p>

        <p>The mutex of the BroadcastHelper passed to the instance's ctor is <em>not</em> locked when calling
        into this method</p>

        @throws css::uno::RuntimeException
    */
    virtual void notifyTranslatedEvent( const css::accessibility::AccessibleEventObject& _rEvent ) = 0;

protected:
    virtual ~OAccessibleContextWrapperHelper( ) override;

    OAccessibleContextWrapperHelper(const OAccessibleContextWrapperHelper&) = delete;
    OAccessibleContextWrapperHelper& operator=(const OAccessibleContextWrapperHelper&) = delete;
};


//= OAccessibleContextWrapper

typedef ::cppu::WeakComponentImplHelper<    css::accessibility::XAccessibleEventBroadcaster
                                        ,   css::accessibility::XAccessibleContext
                                        >   OAccessibleContextWrapper_CBase;

class OAccessibleContextWrapper final : public cppu::BaseMutex,
                                        public OAccessibleContextWrapper_CBase,
                                        public OAccessibleContextWrapperHelper
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
    virtual sal_Int64 SAL_CALL getAccessibleChildCount(  ) override final;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int64 i ) override final;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) override final;
    virtual sal_Int64 SAL_CALL getAccessibleIndexInParent(  ) override;
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) override final;
    virtual OUString SAL_CALL getAccessibleDescription(  ) override final;
    virtual OUString SAL_CALL getAccessibleName(  ) override final;
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) override final;
    virtual sal_Int64 SAL_CALL getAccessibleStateSet(  ) override final;
    virtual css::lang::Locale SAL_CALL getLocale(  ) override final;

    // XAccessibleEventBroadcaster
    virtual void SAL_CALL addAccessibleEventListener( const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener ) override final;
    virtual void SAL_CALL removeAccessibleEventListener( const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener ) override final;

    // OAccessibleContextWrapper
    virtual void notifyTranslatedEvent( const css::accessibility::AccessibleEventObject& _rEvent ) override final;

    // helper method for both 'disposing' methods
    void implDisposing(const css::lang::EventObject* pEvent);

    // OComponentHelper
    void SAL_CALL disposing() override final;

    // XAccessibleEventListener
    virtual void SAL_CALL disposing(const css::lang::EventObject& rEvent) override final;

protected:
    virtual ~OAccessibleContextWrapper() override;

private:
    OAccessibleContextWrapper( const OAccessibleContextWrapper& ) = delete;
    OAccessibleContextWrapper& operator=( const OAccessibleContextWrapper& ) = delete;
};


//= OWrappedAccessibleChildrenManager


typedef ::std::map  <   css::uno::Reference< css::accessibility::XAccessible >
                    ,   rtl::Reference<OAccessibleWrapper >
                    >   AccessibleMap;
                    // TODO: think about if we should hold these objects weak

typedef ::cppu::WeakImplHelper<   css::lang::XEventListener
                              >   OWrappedAccessibleChildrenManager_Base;
/** manages wrapping XAccessible's to XAccessible's
*/
class OWrappedAccessibleChildrenManager final : public OWrappedAccessibleChildrenManager_Base
{
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

    /** disposes (i.e. clears) the manager

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

private:
    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

    void    implTranslateChildEventValue( const css::uno::Any& _rInValue, css::uno::Any& _rOutValue );

    virtual ~OWrappedAccessibleChildrenManager( ) override;

    OWrappedAccessibleChildrenManager( const OWrappedAccessibleChildrenManager& ) = delete;
    OWrappedAccessibleChildrenManager& operator=( const OWrappedAccessibleChildrenManager& ) = delete;
};


}   // namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
