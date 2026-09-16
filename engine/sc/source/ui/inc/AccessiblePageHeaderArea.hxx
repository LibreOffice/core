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

#include "AccessibleContextBase.hxx"
#include <editeng/svxenum.hxx>

class EditTextObject;
namespace accessibility
{
    class AccessibleTextHelper;
}
class ScPreviewShell;

class ScAccessiblePageHeaderArea final
    :   public ScAccessibleContextBase
{
public:
    ScAccessiblePageHeaderArea(
        const cpo::uno::Reference<css::accessibility::XAccessible>& rxParent,
        ScPreviewShell* pViewShell,
        const EditTextObject* pEditObj,
        SvxAdjust eAdjust);
protected:
    virtual ~ScAccessiblePageHeaderArea() override;
public:
    const EditTextObject* GetEditTextObject() const { return mpEditObj.get(); }

    using ScAccessibleContextBase::disposing;
    virtual void disposing() override;

   ///=====  SfxListener  =====================================================

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    ///=====  XAccessibleComponent  ============================================

    virtual cpo::uno::Reference< css::accessibility::XAccessible >
        getAccessibleAtPoint(
        const css::awt::Point& rPoint ) override;

    ///=====  XAccessibleContext  ==============================================

    /// Return the number of currently visible children.
    /// override to calculate this on demand
    virtual sal_Int64
        getAccessibleChildCount() override;

    /// Return the specified child or NULL if index is invalid.
    /// override to calculate this on demand
    virtual cpo::uno::Reference< css::accessibility::XAccessible>
        getAccessibleChild(sal_Int64 nIndex) override;

    /// Return the set of current states.
    virtual sal_Int64
        getAccessibleStateSet() override;

protected:
    virtual OUString createAccessibleDescription() override;
    virtual OUString createAccessibleName() override;

    virtual AbsoluteScreenPixelRectangle GetBoundingBoxOnScreen() override;
    virtual tools::Rectangle GetBoundingBox() override;

private:
    std::unique_ptr<EditTextObject> mpEditObj;
    std::unique_ptr<accessibility::AccessibleTextHelper> mpTextHelper;
    ScPreviewShell* mpViewShell;
    SvxAdjust       meAdjust;

    void CreateTextHelper();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
