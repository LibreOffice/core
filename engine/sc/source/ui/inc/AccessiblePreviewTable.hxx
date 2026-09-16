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
#include <com/sun/star/accessibility/XAccessibleTable.hpp>

class ScPreviewShell;
class ScPreviewTableInfo;

class ScAccessiblePreviewTable
    : public cppu::ImplInheritanceHelper<ScAccessibleContextBase,
                                         css::accessibility::XAccessibleTable>
{
public:
    ScAccessiblePreviewTable( const cpo::uno::Reference<css::accessibility::XAccessible>& rxParent,
                            ScPreviewShell* pViewShell, sal_Int32 nIndex );

protected:
    virtual ~ScAccessiblePreviewTable() override;

    using ScAccessibleContextBase::IsDefunc;

public:
    using ScAccessibleContextBase::disposing;
     virtual void disposing() override;

    //=====  SfxListener  =====================================================

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    //=====  XAccessibleTable  ================================================

    virtual sal_Int32 getAccessibleRowCount() override;
    virtual sal_Int32 getAccessibleColumnCount() override;
    virtual OUString getAccessibleRowDescription( sal_Int32 nRow ) override;
    virtual OUString getAccessibleColumnDescription( sal_Int32 nColumn ) override;
    virtual sal_Int32 getAccessibleRowExtentAt( sal_Int32 nRow, sal_Int32 nColumn ) override;
    virtual sal_Int32 getAccessibleColumnExtentAt( sal_Int32 nRow, sal_Int32 nColumn ) override;
    virtual cpo::uno::Reference< css::accessibility::XAccessibleTable >
                            getAccessibleRowHeaders() override;
    virtual cpo::uno::Reference< css::accessibility::XAccessibleTable >
                            getAccessibleColumnHeaders() override;
    virtual cpo::uno::Sequence< sal_Int32 > getSelectedAccessibleRows() override;
    virtual cpo::uno::Sequence< sal_Int32 > getSelectedAccessibleColumns() override;
    virtual bool isAccessibleRowSelected( sal_Int32 nRow ) override;
    virtual bool isAccessibleColumnSelected( sal_Int32 nColumn ) override;
    virtual cpo::uno::Reference< css::accessibility::XAccessible >
                            getAccessibleCellAt( sal_Int32 nRow, sal_Int32 nColumn ) override;
    virtual cpo::uno::Reference< css::accessibility::XAccessible >
                            getAccessibleCaption() override;
    virtual cpo::uno::Reference< css::accessibility::XAccessible >
                            getAccessibleSummary() override;
    virtual bool isAccessibleSelected( sal_Int32 nRow, sal_Int32 nColumn ) override;
    virtual sal_Int64 getAccessibleIndex( sal_Int32 nRow, sal_Int32 nColumn ) override;
    virtual sal_Int32 getAccessibleRow( sal_Int64 nChildIndex ) override;
    virtual sal_Int32 getAccessibleColumn( sal_Int64 nChildIndex ) override;

    //=====  XAccessibleComponent  ============================================

    virtual cpo::uno::Reference< css::accessibility::XAccessible >
                            getAccessibleAtPoint( const css::awt::Point& aPoint ) override;
    virtual void   grabFocus() override;

    //=====  XAccessibleContext  ==============================================

    virtual sal_Int64 getAccessibleChildCount() override;
    virtual cpo::uno::Reference< css::accessibility::XAccessible >
                            getAccessibleChild( sal_Int64 i ) override;
    virtual sal_Int64 getAccessibleIndexInParent() override;
    virtual sal_Int64 getAccessibleStateSet() override;

protected:
    virtual OUString createAccessibleDescription() override;
    virtual OUString createAccessibleName() override;

    virtual AbsoluteScreenPixelRectangle GetBoundingBoxOnScreen() override;
    virtual tools::Rectangle GetBoundingBox() override;

private:
    ScPreviewShell*     mpViewShell;
    sal_Int32           mnIndex;
    mutable std::unique_ptr<ScPreviewTableInfo> mpTableInfo;

    bool IsDefunc(sal_Int64 nParentStates);

    void    FillTableInfo() const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
