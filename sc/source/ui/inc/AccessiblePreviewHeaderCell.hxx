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
#include <com/sun/star/accessibility/XAccessibleValue.hpp>
#include <tools/gen.hxx>
#include <address.hxx>
#include <cppuhelper/implbase1.hxx>

class ScPreviewShell;
class ScPreviewTableInfo;
namespace accessibility {
    class AccessibleTextHelper;
}

typedef cppu::ImplHelper1< css::accessibility::XAccessibleValue>
                    ScAccessiblePreviewHeaderCellImpl;

class ScAccessiblePreviewHeaderCell :
        public ScAccessibleContextBase,
        public ScAccessiblePreviewHeaderCellImpl
{
public:
    ScAccessiblePreviewHeaderCell( const css::uno::Reference<css::accessibility::XAccessible>& rxParent,
                            ScPreviewShell* pViewShell,
                            const ScAddress& rCellPos, bool bIsColHdr, bool bIsRowHdr,
                            sal_Int32 nIndex );

protected:
    virtual ~ScAccessiblePreviewHeaderCell() override;

    using ScAccessibleContextBase::IsDefunc;

public:
    using ScAccessibleContextBase::disposing;
     virtual void SAL_CALL disposing() override;

    //=====  SfxListener  =====================================================

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    ///=====  XInterface  =====================================================

    virtual css::uno::Any SAL_CALL queryInterface(
        css::uno::Type const & rType ) override;

    virtual void SAL_CALL acquire() throw () override;

    virtual void SAL_CALL release() throw () override;

    //=====  XAccessibleValue  ================================================

    virtual css::uno::Any SAL_CALL getCurrentValue() override;
    virtual sal_Bool SAL_CALL setCurrentValue( const css::uno::Any& aNumber ) override;
    virtual css::uno::Any SAL_CALL getMaximumValue() override;
    virtual css::uno::Any SAL_CALL getMinimumValue() override;

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

    ///=====  XTypeProvider  ===================================================

    virtual css::uno::Sequence< css::uno::Type > SAL_CALL
        getTypes() override;

    /** Returns an implementation id.
    */
    virtual css::uno::Sequence<sal_Int8> SAL_CALL
        getImplementationId() override;

protected:
    virtual OUString createAccessibleDescription() override;
    virtual OUString createAccessibleName() override;

    virtual tools::Rectangle GetBoundingBoxOnScreen() const override;
    virtual tools::Rectangle GetBoundingBox() const override;

private:
    ScPreviewShell*     mpViewShell;
    std::unique_ptr<accessibility::AccessibleTextHelper> mxTextHelper;
    sal_Int32           mnIndex;
    ScAddress           maCellPos;
    bool                mbColumnHeader;
    bool                mbRowHeader;
    mutable std::unique_ptr<ScPreviewTableInfo> mpTableInfo;

    bool IsDefunc(
        const css::uno::Reference<css::accessibility::XAccessibleStateSet>& rxParentStates);

    void CreateTextHelper();
    void    FillTableInfo() const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
