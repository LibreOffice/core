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
#include <com/sun/star/accessibility/XAccessibleTable.hpp>
#include <cppuhelper/implbase1.hxx>

class ScPreviewShell;
class ScPreviewTableInfo;

typedef cppu::ImplHelper1< css::accessibility::XAccessibleTable>
                    ScAccessiblePreviewTableImpl;

class ScAccessiblePreviewTable :
        public ScAccessibleContextBase,
        public ScAccessiblePreviewTableImpl
{
public:
    ScAccessiblePreviewTable( const css::uno::Reference<css::accessibility::XAccessible>& rxParent,
                            ScPreviewShell* pViewShell, sal_Int32 nIndex );

protected:
    virtual ~ScAccessiblePreviewTable() override;

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

    //=====  XAccessibleTable  ================================================

    virtual sal_Int32 SAL_CALL getAccessibleRowCount() override;
    virtual sal_Int32 SAL_CALL getAccessibleColumnCount() override;
    virtual OUString SAL_CALL getAccessibleRowDescription( sal_Int32 nRow ) override;
    virtual OUString SAL_CALL getAccessibleColumnDescription( sal_Int32 nColumn ) override;
    virtual sal_Int32 SAL_CALL getAccessibleRowExtentAt( sal_Int32 nRow, sal_Int32 nColumn ) override;
    virtual sal_Int32 SAL_CALL getAccessibleColumnExtentAt( sal_Int32 nRow, sal_Int32 nColumn ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleTable > SAL_CALL
                            getAccessibleRowHeaders() override;
    virtual css::uno::Reference< css::accessibility::XAccessibleTable > SAL_CALL
                            getAccessibleColumnHeaders() override;
    virtual css::uno::Sequence< sal_Int32 > SAL_CALL getSelectedAccessibleRows() override;
    virtual css::uno::Sequence< sal_Int32 > SAL_CALL getSelectedAccessibleColumns() override;
    virtual sal_Bool SAL_CALL isAccessibleRowSelected( sal_Int32 nRow ) override;
    virtual sal_Bool SAL_CALL isAccessibleColumnSelected( sal_Int32 nColumn ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
                            getAccessibleCellAt( sal_Int32 nRow, sal_Int32 nColumn ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
                            getAccessibleCaption() override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
                            getAccessibleSummary() override;
    virtual sal_Bool SAL_CALL isAccessibleSelected( sal_Int32 nRow, sal_Int32 nColumn ) override;
    virtual sal_Int32 SAL_CALL getAccessibleIndex( sal_Int32 nRow, sal_Int32 nColumn ) override;
    virtual sal_Int32 SAL_CALL getAccessibleRow( sal_Int32 nChildIndex ) override;
    virtual sal_Int32 SAL_CALL getAccessibleColumn( sal_Int32 nChildIndex ) override;

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

    //=====  XTypeProvider  ===================================================

    virtual css::uno::Sequence< css::uno::Type > SAL_CALL
        getTypes() override;

    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;

protected:
    virtual OUString createAccessibleDescription() override;
    virtual OUString createAccessibleName() override;

    virtual tools::Rectangle GetBoundingBoxOnScreen() const override;
    virtual tools::Rectangle GetBoundingBox() const override;

private:
    ScPreviewShell*     mpViewShell;
    sal_Int32           mnIndex;
    mutable std::unique_ptr<ScPreviewTableInfo> mpTableInfo;

    bool IsDefunc(
        const css::uno::Reference<css::accessibility::XAccessibleStateSet>& rxParentStates);

    void    FillTableInfo() const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
