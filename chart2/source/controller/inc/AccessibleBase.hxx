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

#include <ObjectIdentifier.hxx>

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <comphelper/accessibleeventnotifier.hxx>
#include <cppuhelper/compbase.hxx>
#include <rtl/ref.hxx>
#include <tools/color.hxx>

#include <map>
#include <vector>
#include <memory>

#include <MutexContainer.hxx>

namespace com::sun::star::awt { class XWindow; }
namespace com::sun::star::chart2 { class XChartDocument; }
namespace com::sun::star::view { class XSelectionSupplier; }
namespace utl { class AccessibleStateSetHelper; }


class SdrView;

namespace accessibility
{
class IAccessibleViewForwarder;
}

namespace chart
{

class AccessibleBase;
class ObjectHierarchy;

typedef ObjectIdentifier AccessibleUniqueId;

struct AccessibleElementInfo
{
    AccessibleUniqueId m_aOID;

    css::uno::WeakReference< css::chart2::XChartDocument > m_xChartDocument;
    css::uno::WeakReference< css::view::XSelectionSupplier > m_xSelectionSupplier;
    css::uno::WeakReference< css::uno::XInterface >   m_xView;
    css::uno::WeakReference< css::awt::XWindow >      m_xWindow;

    std::shared_ptr< ObjectHierarchy > m_spObjectHierarchy;

    AccessibleBase * m_pParent;
    SdrView* m_pSdrView;
    ::accessibility::IAccessibleViewForwarder* m_pViewForwarder;
};

namespace impl
{
typedef ::cppu::WeakComponentImplHelper<
        css::accessibility::XAccessible,
        css::accessibility::XAccessibleContext,
        css::accessibility::XAccessibleComponent,
        css::accessibility::XAccessibleEventBroadcaster,
        css::lang::XServiceInfo,
        css::lang::XEventListener
        > AccessibleBase_Base;
}

/** Base class for all Chart Accessibility objects
 */
class AccessibleBase :
    public MutexContainer,
    public impl::AccessibleBase_Base
{
public:
    enum class EventType
    {
        GOT_SELECTION,
        LOST_SELECTION
    };

    AccessibleBase( const AccessibleElementInfo & rAccInfo,
                    bool bMayHaveChildren,
                    bool bAlwaysTransparent );
    virtual ~AccessibleBase() override;

protected:
    // for all calls to protected methods it is assumed that the mutex is locked
    // unless calls outside via UNO, e.g. event notification, are done

    /** @param bThrowException if true, a DisposedException is thrown if the
               object is already disposed
        @return true, if the component is already disposed and bThrowException is false,
                false otherwise
        @throws css::lang::DisposedException
     */
    bool             CheckDisposeState( bool bThrowException = true ) const;

    /** Events coming from the core have to be processed in this methods.  The
        default implementation returns false, which indicates that the object is
        not interested in the event.  To react on events you have to implement
        this method in derived classes.

        The default implementation iterates over all children and forwards the
        event until the first child returns true.

        @param nObjId contains the object id of chart objects.  If the object is
                no chart object, the event is not broadcast.
        @return If an object is the addressee of the event it should return
                true, false otherwise.
     */
    bool     NotifyEvent( EventType eType, const AccessibleUniqueId & rId );

    /** Adds a state to the set.

        @throws css::uno::RuntimeException
    */
    void             AddState( sal_Int16 aState );

    /** Removes a state from the set if the set contains the state, otherwise
        nothing is done.

        @throws css::uno::RuntimeException
    */
    void             RemoveState( sal_Int16 aState );

    /** has to be overridden by derived classes that support child elements.
        With this method a rescan is initiated that should result in a correct
        list of children.

        This method is called when access to any methods concerning children is
        invoked for the first time.
     */
    bool UpdateChildren();

    /** Is called by UpdateChildren.  This method is only called if an update is
        really necessary.
     */
    virtual bool ImplUpdateChildren();

    /** adds a child to the end of the internal vector of children.  As a
        result, the child-count increases by one, but all existing children keep
        their indices.

        Important: as the implementation is needed, this should remain the only
        method for adding children (i.e. there mustn't be an AddChild( Reference<
        XAccessible > ) or the like).
     */
    void         AddChild( AccessibleBase* pChild );

    /** removes a child from the internal vector.  All children with index
        greater than the index of the removed element get an index one less than
        before.
     */
    void         RemoveChildByOId( const ObjectIdentifier& rOId );

    /** Retrieve the pixel coordinates of logical coordinates (0,0) of the
        current logic coordinate system.  This can be used for
        getLocationOnScreen, if the coordinates of an object are not relative to
        its direct parent, but a parent higher up in hierarchy.

        @return the (x,y) pixel coordinates of the upper left corner
     */
    virtual css::awt::Point   GetUpperLeftOnScreen() const;

    /** This method creates an AccessibleEventObject and sends it to all
        listeners that are currently listening to this object

        If bSendGlobally is true, the event is also broadcast via
        vcl::unohelper::NotifyAccessibleStateEventGlobally()
     */
    void         BroadcastAccEvent( sal_Int16 nId,
                                    const css::uno::Any & rNew,
                                    const css::uno::Any & rOld,
                                    bool bSendGlobally = false ) const;

    /** Removes all children from the internal lists and broadcasts child remove
        events.

        This method cares about mutex locking, and thus should be called without
        the mutex locked.
     */
    void KillAllChildren();

    /** Is called from getAccessibleChild(). Before this method is called, an
        update of children is done if necessary.

        @throws css::lang::IndexOutOfBoundsException
        @throws css::uno::RuntimeException
     */
    virtual css::uno::Reference< css::accessibility::XAccessible >
        ImplGetAccessibleChildById( sal_Int32 i ) const;

    /** Is called from getAccessibleChildCount(). Before this method is called,
        an update of children is done if necessary.

        @throws css::uno::RuntimeException
     */
    virtual sal_Int32 ImplGetAccessibleChildCount() const;

    const AccessibleElementInfo& GetInfo() const { return m_aAccInfo;}
    void SetInfo( const AccessibleElementInfo & rNewInfo );
    const AccessibleUniqueId& GetId() const { return m_aAccInfo.m_aOID;}

    // ________ WeakComponentImplHelper (XComponent::dispose) ________
    virtual void SAL_CALL disposing() override;

    // ________ XAccessible ________
    virtual css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext() override;

    // ________ XAccessibleContext ________
    virtual sal_Int32 SAL_CALL getAccessibleChildCount() override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
        getAccessibleChild( sal_Int32 i ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
        getAccessibleParent() override;
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent() override;
    /// @return AccessibleRole.SHAPE
    virtual sal_Int16 SAL_CALL getAccessibleRole() override;
    // has to be implemented by derived classes
//     virtual OUString SAL_CALL getAccessibleName()
//         throw (css::uno::RuntimeException);
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL
        getAccessibleRelationSet() override;
    virtual css::uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL
        getAccessibleStateSet() override;
    virtual css::lang::Locale SAL_CALL getLocale() override;
    // has to be implemented by derived classes
//     virtual OUString SAL_CALL getAccessibleDescription()
//         throw (css::uno::RuntimeException);

    // ________ XAccessibleComponent ________
    virtual sal_Bool SAL_CALL containsPoint(
        const css::awt::Point& aPoint ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
        getAccessibleAtPoint( const css::awt::Point& aPoint ) override;
    // has to be defined in derived classes
    virtual css::awt::Rectangle SAL_CALL getBounds() override;
    virtual css::awt::Point SAL_CALL getLocation() override;
    virtual css::awt::Point SAL_CALL getLocationOnScreen() override;
    virtual css::awt::Size SAL_CALL getSize() override;
    virtual void SAL_CALL grabFocus() override;
    virtual sal_Int32 SAL_CALL getForeground() override;
    virtual sal_Int32 SAL_CALL getBackground() override;

    // ________ XServiceInfo ________
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(
        const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // ________ XEventListener ________
    virtual void SAL_CALL disposing(
        const css::lang::EventObject& Source ) override;

    // ________ XAccessibleEventBroadcaster ________
    virtual void SAL_CALL addAccessibleEventListener(
        const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener ) override;
    virtual void SAL_CALL removeAccessibleEventListener(
        const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener ) override;

private:
    enum eColorType
    {
        ACC_BASE_FOREGROUND,
        ACC_BASE_BACKGROUND
    };
    Color getColor( eColorType eColType );

private:
    /** type of the vector containing the accessible children
     */
    typedef std::vector< css::uno::Reference< css::accessibility::XAccessible > > ChildListVectorType;
    /** type of the hash containing a vector index for every AccessibleUniqueId
        of the object in the child list
     */
    typedef std::map< ObjectIdentifier, css::uno::Reference< css::accessibility::XAccessible > > ChildOIDMap;

    bool                                  m_bIsDisposed;
    const bool                            m_bMayHaveChildren;
    bool                                  m_bChildrenInitialized;
    ChildListVectorType                   m_aChildList;

    ChildOIDMap                           m_aChildOIDMap;

    ::comphelper::AccessibleEventNotifier::TClientId      m_nEventNotifierId;

    /** Implementation helper for getAccessibleStateSet()

        Note: This member must come before m_aStateSet!
     */
    rtl::Reference<::utl::AccessibleStateSetHelper>     m_xStateSetHelper;

    AccessibleElementInfo  m_aAccInfo;
    const bool             m_bAlwaysTransparent;
    /** denotes if the state-set is initialized.  On initialization the selected
        state is checked.

        This variable is monitored by the solar mutex!

        Note: declared volatile to enable double-check-locking
     */
    volatile bool          m_bStateSetInitialized;
};

}  // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
