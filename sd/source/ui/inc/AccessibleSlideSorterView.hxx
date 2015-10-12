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
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/awt/XFocusListener.hpp>
#include <com/sun/star/document/XEventListener.hpp>
#include <vcl/vclptr.hxx>

#include <memory>

namespace vcl { class Window; }

namespace sd { namespace slidesorter {
class SlideSorter;
} }

namespace accessibility {

class AccessibleSlideSorterObject;

typedef ::cppu::WeakComponentImplHelper<
    ::com::sun::star::accessibility::XAccessible,
    ::com::sun::star::accessibility::XAccessibleEventBroadcaster,
    ::com::sun::star::accessibility::XAccessibleContext,
    ::com::sun::star::accessibility::XAccessibleComponent,
    ::com::sun::star::accessibility::XAccessibleSelection,
    ::com::sun::star::lang::XServiceInfo
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
        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> & rxParent,
        vcl::Window* pParentWindow);

    void Init();

    virtual ~AccessibleSlideSorterView();

    /** This method acts like a dispose call.  It sends a disposing to all
        of its listeners.  It may be called twice.
    */
    void Destroyed();

    void FireAccessibleEvent (
        short nEventId,
        const ::com::sun::star::uno::Any& rOldValue,
        const ::com::sun::star::uno::Any& rNewValue);

    virtual void SAL_CALL disposing() override;

    /** Return the implementation object of the specified child.
        @param nIndex
            Index of the child for which to return the implementation object.
    */
    AccessibleSlideSorterObject* GetAccessibleChildImplementation (sal_Int32 nIndex);

    //===== XAccessible =======================================================

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL
        getAccessibleContext()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    //===== XAccessibleEventBroadcaster =======================================
    virtual void SAL_CALL
        addAccessibleEventListener(
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessibleEventListener >& rxListener)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL
        removeAccessibleEventListener(
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessibleEventListener >& rxListener )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    //=====  XAccessibleContext  ==============================================

    /// Return the number of currently visible children.
    virtual sal_Int32 SAL_CALL
        getAccessibleChildCount() throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /// Return the specified child or throw exception.
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleChild (sal_Int32 nIndex)
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    /// Return a reference to the parent.
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleParent()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /// Return this objects index among the parents children.
    virtual sal_Int32 SAL_CALL
        getAccessibleIndexInParent()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /// Return this object's role.
    virtual sal_Int16 SAL_CALL
        getAccessibleRole()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /// Return this object's description.
    virtual OUString SAL_CALL
        getAccessibleDescription()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /// Return the object's current name.
    virtual OUString SAL_CALL
        getAccessibleName()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /// Return NULL to indicate that an empty relation set.
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessibleRelationSet> SAL_CALL
        getAccessibleRelationSet()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /// Return the set of current states.
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessibleStateSet> SAL_CALL
        getAccessibleStateSet()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /** Return the parents locale or throw exception if this object has no
        parent yet/anymore.
    */
    virtual ::com::sun::star::lang::Locale SAL_CALL
        getLocale()
        throw (::com::sun::star::uno::RuntimeException,
            ::com::sun::star::accessibility::IllegalAccessibleComponentStateException, std::exception) override;

    //=====  XAccessibleComponent  ================================================

    /** The default implementation uses the result of
        <member>getBounds</member> to determine whether the given point lies
        inside this object.
    */
    virtual sal_Bool SAL_CALL containsPoint (
        const ::com::sun::star::awt::Point& aPoint)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /** The default implementation returns an empty reference.
    */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > SAL_CALL
        getAccessibleAtPoint (
            const ::com::sun::star::awt::Point& aPoint)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /** The default implementation returns an empty rectangle.
    */
    virtual ::com::sun::star::awt::Rectangle SAL_CALL getBounds()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /** The default implementation uses the result of
        <member>getBounds</member> to determine the location.
    */
    virtual ::com::sun::star::awt::Point SAL_CALL getLocation()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /** The default implementation returns an empty position, i.e. the
    * result of the default constructor of <type>com::sun::star::awt::Point</type>.
    */
    virtual ::com::sun::star::awt::Point SAL_CALL getLocationOnScreen()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /** The default implementation uses the result of
        <member>getBounds</member> to determine the size.
    */
    virtual ::com::sun::star::awt::Size SAL_CALL getSize()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /** The default implementation does nothing.
    */
    virtual void SAL_CALL grabFocus()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /** Returns black as the default foreground color.
    */
    virtual sal_Int32 SAL_CALL getForeground()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /** Returns white as the default background color.
    */
    virtual sal_Int32 SAL_CALL getBackground()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    //===== XAccessibleSelection ==============================================

    virtual void SAL_CALL
        selectAccessibleChild (sal_Int32 nChildIndex)
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
            ::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL
        isAccessibleChildSelected( sal_Int32 nChildIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
            ::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL
        clearAccessibleSelection(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL
        selectAllAccessibleChildren(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual sal_Int32 SAL_CALL
        getSelectedAccessibleChildCount(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > SAL_CALL
        getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
            ::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL
        deselectAccessibleChild( sal_Int32 nSelectedChildIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
            ::com::sun::star::uno::RuntimeException, std::exception) override;

    //=====  XServiceInfo  ====================================================

    /** Returns an identifier for the implementation of this object.
    */
    virtual OUString SAL_CALL
        getImplementationName()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /** Return whether the specified service is supported by this class.
    */
    virtual sal_Bool SAL_CALL
        supportsService (const OUString& sServiceName)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /** Returns a list of all supported services.
    */
    virtual ::com::sun::star::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    void SwitchViewActivated();
private:
    class Implementation;
    ::std::unique_ptr<Implementation> mpImpl;

    ::sd::slidesorter::SlideSorter& mrSlideSorter;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible> mxParent;

    sal_uInt32 mnClientId;

    VclPtr<vcl::Window> mpContentWindow;

    /** Check whether or not the object has been disposed (or is in the
        state of being disposed).  If that is the case then
        DisposedException is thrown to inform the (indirect) caller of the
        foul deed.
    */
    void ThrowIfDisposed()
        throw (::com::sun::star::lang::DisposedException);

    /** Check whether or not the object has been disposed (or is in the
        state of being disposed).

        @return sal_True, if the object is disposed or in the course
        of being disposed. Otherwise, sal_False is returned.
    */
    bool IsDisposed();
};

} // end of namespace ::accessibility

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
