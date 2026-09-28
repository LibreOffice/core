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

#include <com/sun/star/accessibility/XAccessibleTable.hpp>
#include <com/sun/star/accessibility/XAccessibleTableSelection.hpp>
#include <com/sun/star/accessibility/XAccessibleExtendedAttributes.hpp>
#include <vector>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>

#include <svl/listener.hxx>
#include <unotools/weakref.hxx>

#include "acccontext.hxx"

class SwTabFrame;
class SwAccessibleTableData_Impl;
class SwTableBox;
class SwSelBoxes;

using SwAccessibleTable_BASE
    = cppu::ImplInheritanceHelper<SwAccessibleContext, css::accessibility::XAccessibleTable,
                                  css::accessibility::XAccessibleSelection,
                                  css::accessibility::XAccessibleTableSelection,
                                  css::accessibility::XAccessibleExtendedAttributes>;
class SwAccessibleTable :
        public SwAccessibleTable_BASE,
        public SvtListener
{
    std::unique_ptr<SwAccessibleTableData_Impl> mpTableData;    // the table's data, protected by SolarMutex
    OUString m_sDesc;
    typedef std::vector< unotools::WeakReference<SwAccessibleContext> > Cells_t;
    Cells_t m_vecCellAdd;
    Cells_t m_vecCellRemove;

    const SwSelBoxes *GetSelBoxes() const;

    void FireTableChangeEvent( const SwAccessibleTableData_Impl& rTableData );

    /** get the SwTableBox* for the given child */
    const SwTableBox* GetTableBox( sal_Int64 ) const;

    bool IsChildSelected( sal_Int64 nChildIndex ) const;

    sal_Int64 GetIndexOfSelectedChild( sal_Int64 nSelectedChildIndex ) const;

protected:
    // Set states for getAccessibleStateSet.
    // This derived class additionally sets MULTISELECTABLE(+)
    virtual void GetStates( sal_Int64& rStateSet ) override;

    virtual ~SwAccessibleTable() override;

    // #i77106#
    void SetDesc( const OUString& sNewDesc )
    {
        m_sDesc = sNewDesc;
    }

    virtual std::unique_ptr<SwAccessibleTableData_Impl> CreateNewTableData(); // #i77106#

    // force update of table data
    void UpdateTableData();

    // remove the current table data
    void ClearTableData();

    // get table data, update if necessary
    inline SwAccessibleTableData_Impl& GetTableData();

    // Is table data available?
    bool HasTableData() const { return (mpTableData != nullptr); }

    virtual void Notify(const SfxHint&) override;

public:
    SwAccessibleTable(std::shared_ptr<SwAccessibleMap> const& pInitMap,
                      const SwTabFrame* pTableFrame);

    // XAccessibleContext

    /// Return this object's description.
    virtual OUString
        getAccessibleDescription() override;

    // XAccessibleTable

    virtual sal_Int32 getAccessibleRowCount() override;
    virtual sal_Int32 getAccessibleColumnCount(  ) override;
    virtual OUString getAccessibleRowDescription(
            sal_Int32 nRow ) override;
    virtual OUString getAccessibleColumnDescription(
            sal_Int32 nColumn ) override;
    virtual sal_Int32 getAccessibleRowExtentAt(
            sal_Int32 nRow, sal_Int32 nColumn ) override;
    virtual sal_Int32 getAccessibleColumnExtentAt(
               sal_Int32 nRow, sal_Int32 nColumn ) override;
    virtual cpo::uno::Reference<
                css::accessibility::XAccessibleTable >
        getAccessibleRowHeaders(  ) override;
    virtual cpo::uno::Reference<
                css::accessibility::XAccessibleTable >
        getAccessibleColumnHeaders(  ) override;
    virtual cpo::uno::Sequence< sal_Int32 >
        getSelectedAccessibleRows(  ) override;
    virtual cpo::uno::Sequence< sal_Int32 >
        getSelectedAccessibleColumns(  ) override;
    virtual bool isAccessibleRowSelected( sal_Int32 nRow ) override;
    virtual bool isAccessibleColumnSelected( sal_Int32 nColumn ) override;
    virtual cpo::uno::Reference<
        css::accessibility::XAccessible >
        getAccessibleCellAt( sal_Int32 nRow, sal_Int32 nColumn ) override;
    virtual cpo::uno::Reference<
        css::accessibility::XAccessible >
        getAccessibleCaption(  ) override;
    virtual cpo::uno::Reference<
        css::accessibility::XAccessible >
        getAccessibleSummary(  ) override;
    virtual bool isAccessibleSelected(
            sal_Int32 nRow, sal_Int32 nColumn ) override;
    virtual sal_Int64 getAccessibleIndex(
            sal_Int32 nRow, sal_Int32 nColumn ) override;
    virtual sal_Int32 getAccessibleRow( sal_Int64 nChildIndex ) override;
    virtual sal_Int32 getAccessibleColumn( sal_Int64 nChildIndex ) override;
    // XAccessibleTableSelection
    virtual bool selectRow( sal_Int32 row ) override ;
    virtual bool selectColumn( sal_Int32 column ) override ;
    virtual bool unselectRow( sal_Int32 row ) override;
    virtual bool unselectColumn( sal_Int32 column ) override;

    // XAccessibleExtendedAttributes
    virtual OUString getExtendedAttributes() override;

    // C++ interface

    // The object has been moved by the layout
    virtual void InvalidatePosOrSize( const SwRect& rOldBox ) override;

    // The object is not visible any longer and should be destroyed
    virtual void Dispose(bool bRecursive, bool bCanSkipInvisible = true) override;

    virtual void DisposeChild( const sw::access::SwAccessibleChild& rFrameOrObj,
                               bool bRecursive, bool bCanSkipInvisible ) override;
    virtual void InvalidateChildPosOrSize( const sw::access::SwAccessibleChild& rFrameOrObj,
                                           const SwRect& rFrame ) override;

    // XAccessibleSelection

    virtual void selectAccessibleChild(
        sal_Int64 nChildIndex ) override;

    virtual bool isAccessibleChildSelected(
        sal_Int64 nChildIndex ) override;

    virtual void clearAccessibleSelection(  ) override;

    virtual void selectAllAccessibleChildren(  ) override;

    virtual sal_Int64 getSelectedAccessibleChildCount(  ) override;

    virtual cpo::uno::Reference< css::accessibility::XAccessible > getSelectedAccessibleChild(
        sal_Int64 nSelectedChildIndex ) override;

    // index has to be treated as global child index.
    virtual void deselectAccessibleChild(
        sal_Int64 nChildIndex ) override;

    // XAccessibleComponent
    sal_Int32 getBackground() override;

    void FireSelectionEvent( );
    void AddSelectionCell(SwAccessibleContext*, bool bAddOrRemove);
};

inline SwAccessibleTableData_Impl& SwAccessibleTable::GetTableData()
{
    if( !mpTableData )
        UpdateTableData();
    return *mpTableData;
}

// #i77106# - subclass to represent table column headers
class SwAccessibleTableColHeaders : public SwAccessibleTable
{
protected:
    virtual ~SwAccessibleTableColHeaders() override
    {}

    virtual std::unique_ptr<SwAccessibleTableData_Impl> CreateNewTableData() override;
    virtual void Notify(const SfxHint&) override;

public:
    SwAccessibleTableColHeaders(std::shared_ptr<SwAccessibleMap> const& pMap,
                                const SwTabFrame *pTabFrame);

    // XAccessibleContext

    /// Return the number of currently visible children.
    virtual sal_Int64 getAccessibleChildCount() override;

    /// Return the specified child or NULL if index is invalid.
    virtual cpo::uno::Reference< css::accessibility::XAccessible>
        getAccessibleChild (sal_Int64 nIndex) override;

    // XAccessibleTable

    virtual cpo::uno::Reference<
                css::accessibility::XAccessibleTable >
        getAccessibleRowHeaders(  ) override;
    virtual cpo::uno::Reference<
                css::accessibility::XAccessibleTable >
        getAccessibleColumnHeaders(  ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
