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
#ifndef _CHART2_ACCESSIBLEBASE_HXX_
#define _CHART2_ACCESSIBLEBASE_HXX_

#include "ObjectIdentifier.hxx"

#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <comphelper/accessibleeventnotifier.hxx>
#include <cppuhelper/compbase6.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <unotools/accessiblestatesethelper.hxx>

#include <vector>
#include <map>
#include <boost/shared_ptr.hpp>

#include "MutexContainer.hxx"

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

    ::com::sun::star::uno::WeakReference<
            ::com::sun::star::chart2::XChartDocument > m_xChartDocument;
    ::com::sun::star::uno::WeakReference<
            ::com::sun::star::view::XSelectionSupplier > m_xSelectionSupplier;
    ::com::sun::star::uno::WeakReference<
            ::com::sun::star::uno::XInterface >   m_xView;
    ::com::sun::star::uno::WeakReference<
            ::com::sun::star::awt::XWindow >      m_xWindow;

    ::boost::shared_ptr< ObjectHierarchy > m_spObjectHierarchy;

    AccessibleBase * m_pParent;
    SdrView* m_pSdrView;
    ::accessibility::IAccessibleViewForwarder* m_pViewForwarder;
};

namespace impl
{
typedef ::cppu::PartialWeakComponentImplHelper6<
        ::com::sun::star::accessibility::XAccessible,
        ::com::sun::star::accessibility::XAccessibleContext,
        ::com::sun::star::accessibility::XAccessibleComponent,
        ::com::sun::star::accessibility::XAccessibleEventBroadcaster,
        ::com::sun::star::lang::XServiceInfo,
        ::com::sun::star::lang::XEventListener
        > AccessibleBase_Base;
}

/** Base class for all Chart Accessibility objects
 */
class AccessibleBase :
    public MutexContainer,
    public impl::AccessibleBase_Base
{
public:
    enum EventType
    {
        OBJECT_CHANGE,
        GOT_SELECTION,
        LOST_SELECTION,
        PROPERTY_CHANGE
    };

    AccessibleBase( const AccessibleElementInfo & rAccInfo,
                    bool bMayHaveChildren,
                    bool bAlwaysTransparent = false );
    virtual ~AccessibleBase();

protected:
    // for all calls to protected methods it is assumed that the mutex is locked
    // unless calls outside via UNO, e.g. event notification, are done

    /** @param bThrowException if true, a DisposedException is thrown if the
               object is already disposed
        @return true, if the component is already disposed and bThrowException is false,
                false otherwise
     */
    bool             CheckDisposeState( bool bThrowException = true ) const throw (::com::sun::star::lang::DisposedException);

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
    virtual bool     NotifyEvent( EventType eType, const AccessibleUniqueId & rId );

    /** Adds a state to the set.
    */
    void             AddState( sal_Int16 aState ) throw (::com::sun::star::uno::RuntimeException);

    /** Removes a state from the set if the set contains the state, otherwise
        nothing is done.
    */
    void             RemoveState( sal_Int16 aState ) throw (::com::sun::star::uno::RuntimeException);

    /** has to be overloaded by derived classes that support child elements.
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
    virtual ::com::sun::star::awt::Point   GetUpperLeftOnScreen() const;

    /** This method creates an AccessibleEventObject and sends it to all
        listeners that are currently listening to this object

        If bSendGlobally is true, the event is also broadcast via
        vcl::unohelper::NotifyAccessibleStateEventGlobally()
     */
    void         BroadcastAccEvent( sal_Int16 nId,
                                    const ::com::sun::star::uno::Any & rNew,
                                    const ::com::sun::star::uno::Any & rOld,
                                    bool bSendGlobally = false ) const;

    /** Removes all children from the internal lists and broadcasts child remove
        events.

        This method cares about mutex locking, and thus should be called without
        the mutex locked.
     */
    virtual void KillAllChildren();

    /** Is called from getAccessibleChild(). Before this method is called, an
        update of children is done if necessary.
     */
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible >
        ImplGetAccessibleChildById( sal_Int32 i ) const
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::uno::RuntimeException);

    /** Is called from getAccessibleChildCount(). Before this method is called,
        an update of children is done if necessary.
     */
    virtual sal_Int32 ImplGetAccessibleChildCount() const
        throw (::com::sun::star::uno::RuntimeException);

    AccessibleElementInfo GetInfo() const;
    void SetInfo( const AccessibleElementInfo & rNewInfo );
    AccessibleUniqueId GetId() const;

    // ________ XComponent ________
    virtual void SAL_CALL dispose()throw (::com::sun::star::uno::RuntimeException)
        { WeakComponentImplHelperBase::dispose(); }
    virtual void SAL_CALL addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & xListener)throw (::com::sun::star::uno::RuntimeException)
        { WeakComponentImplHelperBase::addEventListener(xListener); }
    virtual void SAL_CALL removeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & xListener)throw (::com::sun::star::uno::RuntimeException)
        { WeakComponentImplHelperBase::removeEventListener(xListener); }

    // ________ WeakComponentImplHelper (XComponent::dispose) ________
    virtual void SAL_CALL disposing();

    // ________ XAccessible ________
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext()
        throw (::com::sun::star::uno::RuntimeException);

    // ________ XAccessibleContext ________
    virtual sal_Int32 SAL_CALL getAccessibleChildCount()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL
        getAccessibleChild( sal_Int32 i )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL
        getAccessibleParent()
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent()
        throw (::com::sun::star::uno::RuntimeException);
    /// @return AccessibleRole.SHAPE
    virtual sal_Int16 SAL_CALL getAccessibleRole()
        throw (::com::sun::star::uno::RuntimeException);
    // has to be implemented by derived classes
//     virtual OUString SAL_CALL getAccessibleName()
//         throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL
        getAccessibleRelationSet()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL
        getAccessibleStateSet()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::lang::Locale SAL_CALL getLocale()
        throw (::com::sun::star::accessibility::IllegalAccessibleComponentStateException,
               ::com::sun::star::uno::RuntimeException);
    // has to be implemented by derived classes
//     virtual OUString SAL_CALL getAccessibleDescription()
//         throw (::com::sun::star::uno::RuntimeException);

    // ________ XAccessibleComponent ________
    virtual sal_Bool SAL_CALL containsPoint(
        const ::com::sun::star::awt::Point& aPoint )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL
        getAccessibleAtPoint( const ::com::sun::star::awt::Point& aPoint )
        throw (::com::sun::star::uno::RuntimeException);
    // has to be defined in derived classes
    virtual ::com::sun::star::awt::Rectangle SAL_CALL getBounds()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Point SAL_CALL getLocation()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Point SAL_CALL getLocationOnScreen()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Size SAL_CALL getSize()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL grabFocus()
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getForeground()
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getBackground()
        throw (::com::sun::star::uno::RuntimeException);

    // ________ XServiceInfo ________
    virtual OUString SAL_CALL getImplementationName()
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService(
        const OUString& ServiceName )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw (::com::sun::star::uno::RuntimeException);

    // ________ XEventListener ________
    virtual void SAL_CALL disposing(
        const ::com::sun::star::lang::EventObject& Source )
        throw (::com::sun::star::uno::RuntimeException);

    // ________ XAccessibleEventBroadcaster ________
    virtual void SAL_CALL addAccessibleEventListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& xListener )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeAccessibleEventListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& xListener )
        throw (::com::sun::star::uno::RuntimeException);

private:
    enum eColorType
    {
        ACC_BASE_FOREGROUND,
        ACC_BASE_BACKGROUND
    };
    sal_Int32 getColor( eColorType eColType );

private:
    typedef ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible > tAccessible;
    /** type of the vector containing the accessible children
     */
    typedef ::std::vector< tAccessible > ChildListVectorType;
    /** type of the hash containing a vector index for every AccessibleUniqueId
        of the object in the child list
     */
    typedef ::std::map< ObjectIdentifier, tAccessible > ChildOIDMap;

    bool                                  m_bIsDisposed;
    const bool                            m_bMayHaveChildren;
    bool                                  m_bChildrenInitialized;
    ChildListVectorType                   m_aChildList;

    ChildOIDMap                           m_aChildOIDMap;

    ::comphelper::AccessibleEventNotifier::TClientId      m_nEventNotifierId;

    /** Implementation helper for getAccessibleStateSet()

        Note: This member must come before m_aStateSet!
     */
    ::utl::AccessibleStateSetHelper *     m_pStateSetHelper;
    /** this is returned in getAccessibleStateSet().

        The implementation is an ::utl::AccessibleStateSetHelper.  To access
        implementation methods use m_pStateSetHelper.

        Note: Keeping this reference ensures, that the helper object is only
              destroyed after this object has been disposed().
     */
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet >
        m_aStateSet;

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

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
