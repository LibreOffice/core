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

#include "acccontext.hxx"
#include <com/sun/star/accessibility/XAccessibleValue.hpp>
#include "accselectionhelper.hxx"

class SwCellFrame;
class SwAccessibleTable;
class SwFrameFormat;

using SwAccessibleCell_BASE = cppu::ImplInheritanceHelper<SwAccessibleContext,
                                                          css::accessibility::XAccessibleValue,
                                                          css::accessibility::XAccessibleSelection,
                                                          css::accessibility::XAccessibleExtendedAttributes>;
class SwAccessibleCell : public SwAccessibleCell_BASE
{
    // Implementation for XAccessibleSelection interface
    SwAccessibleSelectionHelper m_aSelectionHelper;
    bool    m_bIsSelected;    // protected by base class mutex

    bool    IsSelected();

    bool InvalidateMyCursorPos();
    bool InvalidateChildrenCursorPos( const SwFrame *pFrame );

    rtl::Reference<SwAccessibleTable> m_pAccTable;

protected:
    // Set states for getAccessibleStateSet.
    // This derived class additionally sets SELECTABLE(1) and SELECTED(+)
    virtual void GetStates( sal_Int64& rStateSet ) override;

    virtual void InvalidateCursorPos_() override;

    virtual ~SwAccessibleCell() override;

public:
    SwAccessibleCell(std::shared_ptr<SwAccessibleMap> const& pInitMap,
                     const SwCellFrame *pCellFrame);

    virtual bool HasCursor() override;   // required by map to remember that object

    // XAccessibleContext

    /// Return this object's description.
    virtual OUString SAL_CALL
        getAccessibleDescription() override;

    // XServiceInfo

    // Returns an identifier for the implementation of this object.
    virtual OUString SAL_CALL
        getImplementationName() override;

    virtual css::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames() override;

    virtual void Dispose(bool bRecursive, bool bCanSkipInvisible = true) override;

    virtual void InvalidatePosOrSize( const SwRect& rFrame ) override;

    // XAccessibleExtendedAttributes
    OUString SAL_CALL getExtendedAttributes() override;
private:
    SwFrameFormat* GetTableBoxFormat() const;

public:
    // XAccessibleValue
    virtual css::uno::Any SAL_CALL getCurrentValue( ) override;
    virtual sal_Bool SAL_CALL setCurrentValue( const css::uno::Any& aNumber ) override;
    virtual css::uno::Any SAL_CALL getMaximumValue(  ) override;
    virtual css::uno::Any SAL_CALL getMinimumValue(  ) override;
    virtual css::uno::Any SAL_CALL getMinimumIncrement(  ) override;

    // XAccessibleComponent
    sal_Int32 SAL_CALL getBackground() override;

    // XAccessibleSelection
    virtual void SAL_CALL selectAccessibleChild( sal_Int64 nChildIndex ) override;

    virtual sal_Bool SAL_CALL isAccessibleChildSelected( sal_Int64 nChildIndex ) override;
    virtual void SAL_CALL clearAccessibleSelection(  ) override;
    virtual void SAL_CALL selectAllAccessibleChildren(  ) override;
    virtual sal_Int64 SAL_CALL getSelectedAccessibleChildCount(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild(
        sal_Int64 nSelectedChildIndex ) override;

    virtual void SAL_CALL deselectAccessibleChild( sal_Int64 nSelectedChildIndex ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
