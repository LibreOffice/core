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

#ifndef INCLUDED_SD_SOURCE_UI_INC_ACCESSIBLESLIDESORTERVIEW_HXX
#define INCLUDED_SD_SOURCE_UI_INC_ACCESSIBLESLIDESORTERVIEW_HXX

#include "MutexOwner.hxx"
#include <cppuhelper/compbase.hxx>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <vcl/vclptr.hxx>
#include <vcl/window.hxx>

#include <memory>

namespace sd { namespace slidesorter {
class SlideSorter;
} }

namespace accessibility {

class AccessibleSlideSorterObject;

typedef ::cppu::WeakComponentImplHelper<
    css::accessibility::XAccessible,
    css::accessibility::XAccessibleEventBroadcaster,
    css::accessibility::XAccessibleContext,
    css::accessibility::XAccessibleComponent,
    css::accessibility::XAccessibleSelection,
    css::lang::XServiceInfo
    > AccessibleSlideSorterViewBase;

/** This class makes the SlideSorterViewShell accessible.  It uses objects
    of the AccessibleSlideSorterObject class to the make the page objects
    accessible.
*/
class AccessibleSlideSorterView
    : public ::sd::MutexOwner,
      public AccessibleSlideSorterViewBase
{
public:
    AccessibleSlideSorterView(
        ::sd::slidesorter::SlideSorter& rSlideSorter,
        vcl::Window* pParentWindow);

    void Init();

    virtual ~AccessibleSlideSorterView() override;

    /** This method acts like a dispose call.  It sends a disposing to all
        of its listeners.  It may be called twice.
    */
    void Destroyed();

    void FireAccessibleEvent (
        short nEventId,
        const css::uno::Any& rOldValue,
        const css::uno::Any& rNewValue);

    virtual void SAL_CALL disposing() override;

    /** Return the implementation object of the specified child.
        @param nIndex
            Index of the child for which to return the implementation object.
    */
    AccessibleSlideSorterObject* GetAccessibleChildImplementation (sal_Int32 nIndex);

    //===== XAccessible =======================================================

    virtual css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL
        getAccessibleContext() override;

    //===== XAccessibleEventBroadcaster =======================================
    virtual void SAL_CALL
        addAccessibleEventListener(
            const css::uno::Reference< css::accessibility::XAccessibleEventListener >& rxListener) override;

    virtual void SAL_CALL
        removeAccessibleEventListener(
            const css::uno::Reference< css::accessibility::XAccessibleEventListener >& rxListener ) override;

    //=====  XAccessibleContext  ==============================================

    /// Return the number of currently visible children.
    virtual sal_Int32 SAL_CALL
        getAccessibleChildCount() override;

    /// Return the specified child or throw exception.
    virtual css::uno::Reference< css::accessibility::XAccessible> SAL_CALL
        getAccessibleChild (sal_Int32 nIndex) override;

    /// Return a reference to the parent.
    virtual css::uno::Reference< css::accessibility::XAccessible> SAL_CALL
        getAccessibleParent() override;

    /// Return this objects index among the parents children.
    virtual sal_Int32 SAL_CALL
        getAccessibleIndexInParent() override;

    /// Return this object's role.
    virtual sal_Int16 SAL_CALL
        getAccessibleRole() override;

    /// Return this object's description.
    virtual OUString SAL_CALL
        getAccessibleDescription() override;

    /// Return the object's current name.
    virtual OUString SAL_CALL
        getAccessibleName() override;

    /// Return NULL to indicate that an empty relation set.
    virtual css::uno::Reference<
            css::accessibility::XAccessibleRelationSet> SAL_CALL
        getAccessibleRelationSet() override;

    /// Return the set of current states.
    virtual css::uno::Reference<
            css::accessibility::XAccessibleStateSet> SAL_CALL
        getAccessibleStateSet() override;

    /** Return the parents locale or throw exception if this object has no
        parent yet/anymore.
    */
    virtual css::lang::Locale SAL_CALL
        getLocale() override;

    //=====  XAccessibleComponent  ================================================

    /** The default implementation uses the result of
        <member>getBounds</member> to determine whether the given point lies
        inside this object.
    */
    virtual sal_Bool SAL_CALL containsPoint (
        const css::awt::Point& aPoint) override;

    /** The default implementation returns an empty reference.
    */
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
        getAccessibleAtPoint (
            const css::awt::Point& aPoint) override;

    /** The default implementation returns an empty rectangle.
    */
    virtual css::awt::Rectangle SAL_CALL getBounds() override;

    /** The default implementation uses the result of
        <member>getBounds</member> to determine the location.
    */
    virtual css::awt::Point SAL_CALL getLocation() override;

    /** The default implementation returns an empty position, i.e. the
    * result of the default constructor of <type>css::awt::Point</type>.
    */
    virtual css::awt::Point SAL_CALL getLocationOnScreen() override;

    /** The default implementation uses the result of
        <member>getBounds</member> to determine the size.
    */
    virtual css::awt::Size SAL_CALL getSize() override;

    /** The default implementation does nothing.
    */
    virtual void SAL_CALL grabFocus() override;

    /** Returns black as the default foreground color.
    */
    virtual sal_Int32 SAL_CALL getForeground() override;

    /** Returns white as the default background color.
    */
    virtual sal_Int32 SAL_CALL getBackground() override;

    //===== XAccessibleSelection ==============================================

    virtual void SAL_CALL
        selectAccessibleChild (sal_Int32 nChildIndex) override;

    virtual sal_Bool SAL_CALL
        isAccessibleChildSelected( sal_Int32 nChildIndex ) override;

    virtual void SAL_CALL
        clearAccessibleSelection(  ) override;

    virtual void SAL_CALL
        selectAllAccessibleChildren(  ) override;

    virtual sal_Int32 SAL_CALL
        getSelectedAccessibleChildCount(  ) override;

    virtual css::uno::Reference<
        css::accessibility::XAccessible > SAL_CALL
        getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) override;

    virtual void SAL_CALL
        deselectAccessibleChild( sal_Int32 nSelectedChildIndex ) override;

    //=====  XServiceInfo  ====================================================

    /** Returns an identifier for the implementation of this object.
    */
    virtual OUString SAL_CALL
        getImplementationName() override;

    /** Return whether the specified service is supported by this class.
    */
    virtual sal_Bool SAL_CALL
        supportsService (const OUString& sServiceName) override;

    /** Returns a list of all supported services.
    */
    virtual css::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames() override;

    void SwitchViewActivated();
private:
    class Implementation;
    ::std::unique_ptr<Implementation> mpImpl;

    ::sd::slidesorter::SlideSorter& mrSlideSorter;

    sal_uInt32 mnClientId;

    VclPtr<vcl::Window> mpContentWindow;

    /** Check whether or not the object has been disposed (or is in the
        state of being disposed).  If that is the case then
        DisposedException is thrown to inform the (indirect) caller of the
        foul deed.

        @throws css::lang::DisposedException
    */
    void ThrowIfDisposed();
};

} // end of namespace ::accessibility

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
