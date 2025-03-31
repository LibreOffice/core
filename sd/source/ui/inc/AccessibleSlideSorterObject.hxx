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

#include "AccessibleSlideSorterView.hxx"

#include <comphelper/compbase.hxx>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <rtl/ref.hxx>

class SdPage;
namespace sd::slidesorter { class SlideSorter; }

namespace accessibility {

/** This class makes page objects of the slide sorter accessible.
*/
class AccessibleSlideSorterObject
    : public cppu::ImplInheritanceHelper<comphelper::OAccessibleComponentHelper,
                                         css::accessibility::XAccessible, css::lang::XServiceInfo>
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
        const rtl::Reference<AccessibleSlideSorterView>& rxParent,
        ::sd::slidesorter::SlideSorter& rSlideSorter,
        sal_uInt16 nPageNumber);
    virtual ~AccessibleSlideSorterObject() override;

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

    //===== XAccessible =======================================================

    virtual css::uno::Reference<css::accessibility::XAccessibleContext > SAL_CALL
        getAccessibleContext() override;

    //=====  XAccessibleContext  ==============================================

    virtual sal_Int64 SAL_CALL
        getAccessibleChildCount() override;

    virtual css::uno::Reference< css::accessibility::XAccessible> SAL_CALL
        getAccessibleChild (sal_Int64 nIndex) override;

    virtual css::uno::Reference< css::accessibility::XAccessible> SAL_CALL
        getAccessibleParent() override;

    virtual sal_Int64 SAL_CALL
        getAccessibleIndexInParent() override;

    virtual sal_Int16 SAL_CALL
        getAccessibleRole() override;

    virtual OUString SAL_CALL
        getAccessibleDescription() override;

    virtual OUString SAL_CALL
        getAccessibleName() override;

    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet> SAL_CALL
        getAccessibleRelationSet() override;

    virtual sal_Int64 SAL_CALL
        getAccessibleStateSet() override;

    virtual css::lang::Locale SAL_CALL
        getLocale() override;

    // OAccessibleComponentHelper
    virtual css::awt::Rectangle implGetBounds() override;

    //=====  XAccessibleComponent  ================================================
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
        getAccessibleAtPoint (
            const css::awt::Point& aPoint) override;

    virtual void SAL_CALL grabFocus() override;

    virtual sal_Int32 SAL_CALL getForeground() override;

    virtual sal_Int32 SAL_CALL getBackground() override;

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

private:
    rtl::Reference<AccessibleSlideSorterView> mxParent;
    sal_uInt16 mnPageNumber;
    ::sd::slidesorter::SlideSorter& mrSlideSorter;
};

} // end of namespace ::accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
