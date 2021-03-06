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

#include "AccessibleContextBase.hxx"
#include <editeng/svxenum.hxx>
#include <rtl/ref.hxx>

class ScPreviewShell;
class EditTextObject;
class ScAccessiblePageHeaderArea;

class ScAccessiblePageHeader : public ScAccessibleContextBase
{
public:
    ScAccessiblePageHeader( const css::uno::Reference<css::accessibility::XAccessible>& rxParent,
                            ScPreviewShell* pViewShell, bool bHeader, sal_Int32 nIndex );

protected:
    virtual ~ScAccessiblePageHeader() override;

    using ScAccessibleContextBase::IsDefunc;

public:
    using ScAccessibleContextBase::disposing;
     virtual void SAL_CALL disposing() override;

    //=====  SfxListener  =====================================================

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    //=====  XAccessibleComponent  ============================================

    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
                            getAccessibleAtPoint( const css::awt::Point& aPoint ) override;
    virtual void SAL_CALL   grabFocus() override;

    //=====  XAccessibleContext  ==============================================

    virtual sal_Int32 SAL_CALL getAccessibleChildCount() override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
                            getAccessibleChild( sal_Int32 i ) override;
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent() override;
    virtual css::uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL
                            getAccessibleStateSet() override;

    //=====  XServiceInfo  ====================================================

    virtual OUString SAL_CALL getImplementationName() override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

protected:
    virtual OUString createAccessibleDescription() override;
    virtual OUString createAccessibleName() override;

    virtual tools::Rectangle GetBoundingBoxOnScreen() const override;
    virtual tools::Rectangle GetBoundingBox() const override;

private:
    ScPreviewShell*     mpViewShell;
    sal_Int32           mnIndex;
    bool                mbHeader;
    std::vector< rtl::Reference<ScAccessiblePageHeaderArea> >
                        maAreas;
    sal_Int32           mnChildCount;

    bool IsDefunc(
        const css::uno::Reference<css::accessibility::XAccessibleStateSet>& rxParentStates);

    void AddChild(const EditTextObject* pArea, sal_uInt32 nIndex, SvxAdjust eAdjust);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
