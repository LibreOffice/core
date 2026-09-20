/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <comphelper/OAccessible.hxx>
#include <cppuhelper/compbase.hxx>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <vcl/vclptr.hxx>
#include <vcl/window.hxx>

#include <memory>

namespace sd::slidesorter { class SlideSorter; }

namespace accessibility {

class AccessibleSlideSorterObject;

/** This class makes the SlideSorterViewShell accessible.  It uses objects
    of the AccessibleSlideSorterObject class to the make the page objects
    accessible.
*/
class AccessibleSlideSorterView final
    : public cppu::ImplInheritanceHelper<comphelper::OAccessible,
                                         css::accessibility::XAccessibleSelection,
                                         css::lang::XServiceInfo>
{
public:
    AccessibleSlideSorterView(
        ::sd::slidesorter::SlideSorter& rSlideSorter,
        vcl::Window* pParentWindow);

    void Init();

    virtual ~AccessibleSlideSorterView() override;

    void FireAccessibleEvent (
        short nEventId,
        const cpo::uno::Any& rOldValue,
        const cpo::uno::Any& rNewValue);

    virtual void disposing() override;

    /** Return the implementation object of the specified child.
        @param nIndex
            Index of the child for which to return the implementation object.
    */
    AccessibleSlideSorterObject* GetAccessibleChildImplementation (sal_Int32 nIndex);

    //=====  XAccessibleContext  ==============================================

    /// Return the number of currently visible children.
    virtual sal_Int64
        getAccessibleChildCount() override;

    /// Return the specified child or throw exception.
    virtual cpo::uno::Reference< css::accessibility::XAccessible>
        getAccessibleChild (sal_Int64 nIndex) override;

    virtual cpo::uno::Reference< css::accessibility::XAccessible>
        getAccessibleParent() override;

    virtual sal_Int64
        getAccessibleIndexInParent() override;

    virtual sal_Int16
        getAccessibleRole() override;

    virtual OUString
        getAccessibleDescription() override;

    virtual OUString
        getAccessibleName() override;

    /// Return NULL to indicate that an empty relation set.
    virtual cpo::uno::Reference<
            css::accessibility::XAccessibleRelationSet>
        getAccessibleRelationSet() override;

    virtual sal_Int64 getAccessibleStateSet() override;

    /** Return the parents locale or throw exception if this object has no
        parent yet/anymore.
    */
    virtual css::lang::Locale
        getLocale() override;

    // OAccessible
    virtual css::awt::Rectangle implGetBounds() override;

    //=====  XAccessibleComponent  ================================================

    virtual cpo::uno::Reference< css::accessibility::XAccessible >
        getAccessibleAtPoint (
            const css::awt::Point& aPoint) override;

    virtual void grabFocus() override;

    virtual sal_Int32 getForeground() override;

    virtual sal_Int32 getBackground() override;

    //===== XAccessibleSelection ==============================================

    virtual void
        selectAccessibleChild (sal_Int64 nChildIndex) override;

    virtual bool
        isAccessibleChildSelected( sal_Int64 nChildIndex ) override;

    virtual void
        clearAccessibleSelection(  ) override;

    virtual void
        selectAllAccessibleChildren(  ) override;

    virtual sal_Int64
        getSelectedAccessibleChildCount(  ) override;

    virtual cpo::uno::Reference<
        css::accessibility::XAccessible >
        getSelectedAccessibleChild( sal_Int64 nSelectedChildIndex ) override;

    virtual void
        deselectAccessibleChild( sal_Int64 nSelectedChildIndex ) override;

    //=====  XServiceInfo  ====================================================

    virtual OUString
        getImplementationName() override;

    virtual bool
        supportsService (const OUString& sServiceName) override;

    virtual cpo::uno::Sequence< OUString>
        getSupportedServiceNames() override;

    void SwitchViewActivated();
private:
    class Implementation;
    ::std::unique_ptr<Implementation> mpImpl;

    ::sd::slidesorter::SlideSorter& mrSlideSorter;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
