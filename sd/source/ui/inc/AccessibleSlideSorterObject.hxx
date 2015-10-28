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

#ifndef INCLUDED_SD_SOURCE_UI_INC_ACCESSIBLESLIDESORTEROBJECT_HXX
#define INCLUDED_SD_SOURCE_UI_INC_ACCESSIBLESLIDESORTEROBJECT_HXX

#include "MutexOwner.hxx"
#include <cppuhelper/compbase.hxx>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/awt/XFocusListener.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/DisposedException.hpp>

class SdPage;

namespace sd { namespace slidesorter {
class SlideSorter;
} }

namespace accessibility {

typedef ::cppu::WeakComponentImplHelper<
    css::accessibility::XAccessible,
    css::accessibility::XAccessibleEventBroadcaster,
    css::accessibility::XAccessibleContext,
    css::accessibility::XAccessibleComponent,
    css::lang::XServiceInfo > AccessibleSlideSorterObjectBase;

/** This class makes page objects of the slide sorter accessible.
*/
class AccessibleSlideSorterObject
    : public ::sd::MutexOwner,
      public AccessibleSlideSorterObjectBase
{
public:
    /** Create a new object that represents a page object in the slide
        sorter.
        @param rxParent
            The accessible parent.
        @param rSlideSorter
            The slide sorter whose model manages the page.
        @param nPageNumber
            The number of the page in the range [0,nPageCount).
    */
    AccessibleSlideSorterObject(
        const css::uno::Reference<css::accessibility::XAccessible >& rxParent,
        ::sd::slidesorter::SlideSorter& rSlideSorter,
        sal_uInt16 nPageNumber);
    virtual ~AccessibleSlideSorterObject();

    /** Return the page that is made accessible by the called object.
    */
    SdPage* GetPage() const;

    /** The page number as given to the constructor.
    */
    sal_uInt16 GetPageNumber() const { return mnPageNumber;}

    void FireAccessibleEvent (
        short nEventId,
        const css::uno::Any& rOldValue,
        const css::uno::Any& rNewValue);

    virtual void SAL_CALL disposing() override;

    //===== XAccessible =======================================================

    virtual css::uno::Reference<css::accessibility::XAccessibleContext > SAL_CALL
        getAccessibleContext()
        throw (css::uno::RuntimeException, std::exception) override;

    //===== XAccessibleEventBroadcaster =======================================
    virtual void SAL_CALL
        addAccessibleEventListener(
            const css::uno::Reference<css::accessibility::XAccessibleEventListener >& rxListener)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL
        removeAccessibleEventListener(
            const css::uno::Reference<css::accessibility::XAccessibleEventListener >& rxListener )
        throw (css::uno::RuntimeException, std::exception) override;

    //=====  XAccessibleContext  ==============================================

    virtual sal_Int32 SAL_CALL
        getAccessibleChildCount() throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference< css::accessibility::XAccessible> SAL_CALL
        getAccessibleChild (sal_Int32 nIndex)
        throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference< css::accessibility::XAccessible> SAL_CALL
        getAccessibleParent()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Int32 SAL_CALL
        getAccessibleIndexInParent()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Int16 SAL_CALL
        getAccessibleRole()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL
        getAccessibleDescription()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL
        getAccessibleName()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet> SAL_CALL
        getAccessibleRelationSet()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference< css::accessibility::XAccessibleStateSet> SAL_CALL
        getAccessibleStateSet()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::lang::Locale SAL_CALL
        getLocale()
        throw (css::uno::RuntimeException,
            css::accessibility::IllegalAccessibleComponentStateException, std::exception) override;

    //=====  XAccessibleComponent  ================================================

    virtual sal_Bool SAL_CALL containsPoint (
        const css::awt::Point& aPoint)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
        getAccessibleAtPoint (
            const css::awt::Point& aPoint)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::awt::Rectangle SAL_CALL getBounds()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::awt::Point SAL_CALL getLocation()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::awt::Point SAL_CALL getLocationOnScreen()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::awt::Size SAL_CALL getSize()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL grabFocus()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Int32 SAL_CALL getForeground()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Int32 SAL_CALL getBackground()
        throw (css::uno::RuntimeException, std::exception) override;

    //=====  XServiceInfo  ====================================================

    /** Returns an identifier for the implementation of this object.
    */
    virtual OUString SAL_CALL
        getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    /** Return whether the specified service is supported by this class.
    */
    virtual sal_Bool SAL_CALL
        supportsService (const OUString& sServiceName)
        throw (css::uno::RuntimeException, std::exception) override;

    /** Returns a list of all supported services.
    */
    virtual css::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;

private:
    css::uno::Reference<css::accessibility::XAccessible> mxParent;
    sal_uInt16 mnPageNumber;
    ::sd::slidesorter::SlideSorter& mrSlideSorter;
    sal_uInt32 mnClientId;

    /** Check whether or not the object has been disposed (or is in the
        state of being disposed).  If that is the case then
        DisposedException is thrown to inform the (indirect) caller of the
        foul deed.
    */
    void ThrowIfDisposed()
        throw (css::lang::DisposedException);

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
