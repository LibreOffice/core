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

#include <sal/config.h>

#include <rtl/ref.hxx>

#include "AccessibleDocumentBase.hxx"

class ScPreviewShell;
class ScNotesChildren;
class ScShapeChildren;
class ScAccessiblePreviewTable;
class ScAccessiblePageHeader;

class ScAccessibleDocumentPagePreview
    :   public ScAccessibleDocumentBase
{
public:
    //=====  internal  ========================================================
    ScAccessibleDocumentPagePreview(
        const css::uno::Reference<css::accessibility::XAccessible>& rxParent,
        ScPreviewShell* pViewShell );
protected:
    virtual ~ScAccessibleDocumentPagePreview() override;

    using ScAccessibleDocumentBase::IsDefunc;

public:
    using ScAccessibleContextBase::disposing;

     virtual void SAL_CALL disposing() override;

   ///=====  SfxListener  =====================================================

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    ///=====  XAccessibleComponent  ============================================

    virtual css::uno::Reference< css::accessibility::XAccessible >
        SAL_CALL getAccessibleAtPoint(
        const css::awt::Point& rPoint ) override;

    virtual void SAL_CALL grabFocus(  ) override;

    ///=====  XAccessibleContext  ==============================================

    /// Return the number of currently visible children.
    virtual sal_Int32 SAL_CALL getAccessibleChildCount() override;

    /// Return the specified child or NULL if index is invalid.
    virtual css::uno::Reference< css::accessibility::XAccessible> SAL_CALL
        getAccessibleChild(sal_Int32 nIndex) override;

    /// Return the set of current states.
    virtual css::uno::Reference<
            css::accessibility::XAccessibleStateSet> SAL_CALL
        getAccessibleStateSet() override;

    virtual OUString SAL_CALL getAccessibleName() override;
    ///=====  XServiceInfo  ====================================================

    /** Returns an identifier for the implementation of this object.
    */
    virtual OUString SAL_CALL
        getImplementationName() override;

    /** Returns a list of all supported services.
    */
    virtual css::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames() override;

    ///=====  XTypeProvider  ===================================================

    /** Returns an implementation id.
    */
    virtual css::uno::Sequence<sal_Int8> SAL_CALL
        getImplementationId() override;

    ///=====  internal  ========================================================

protected:
    /// Return this object's description.
    virtual OUString
        createAccessibleDescription() override;

    /// Return the object's current name.
    virtual OUString
        createAccessibleName() override;

public: // needed in ScShapeChildren
    /// Return the object's current bounding box relative to the desktop.
    virtual tools::Rectangle GetBoundingBoxOnScreen() const override;

protected:
    /// Return the object's current bounding box relative to the parent object.
    virtual tools::Rectangle GetBoundingBox() const override;

private:
    ScPreviewShell* mpViewShell;
    std::unique_ptr<ScNotesChildren> mpNotesChildren;
    std::unique_ptr<ScShapeChildren> mpShapeChildren;
    rtl::Reference<ScAccessiblePreviewTable> mpTable;
    rtl::Reference<ScAccessiblePageHeader> mpHeader;
    rtl::Reference<ScAccessiblePageHeader> mpFooter;

    bool IsDefunc(
        const css::uno::Reference<css::accessibility::XAccessibleStateSet>& rxParentStates);

    ScNotesChildren* GetNotesChildren();
    ScShapeChildren* GetShapeChildren();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
