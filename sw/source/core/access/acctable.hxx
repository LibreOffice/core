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

#include <com/sun/star/accessibility/XAccessibleTable.hpp>
#include <com/sun/star/accessibility/XAccessibleTableSelection.hpp>
#include <vector>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>

#include <svl/listener.hxx>

#include "acccontext.hxx"

class SwTabFrame;
class SwAccessibleTableData_Impl;
class SwTableBox;
class SwSelBoxes;

namespace sw::access {
    class SwAccessibleChild;
}

class SwAccessibleTable :
        public SwAccessibleContext,
        public css::accessibility::XAccessibleTable,
        public css::accessibility::XAccessibleSelection,
        public css::accessibility::XAccessibleTableSelection,
        public SvtListener
{
    std::unique_ptr<SwAccessibleTableData_Impl> mpTableData;    // the table's data, protected by SolarMutex
    OUString m_sDesc;
    const SwSelBoxes *GetSelBoxes() const;

    void FireTableChangeEvent( const SwAccessibleTableData_Impl& rTableData );

    /** get the SwTableBox* for the given child */
    const SwTableBox* GetTableBox( sal_Int32 ) const;

    bool IsChildSelected( sal_Int32 nChildIndex ) const;

    sal_Int32 GetIndexOfSelectedChild( sal_Int32 nSelectedChildIndex ) const;

protected:
    // Set states for getAccessibleStateSet.
    // This derived class additionally sets MULTISELECTABLE(+)
    virtual void GetStates( ::utl::AccessibleStateSetHelper& rStateSet ) override;

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

    // Is table data evailable?
    bool HasTableData() const { return (mpTableData != nullptr); }

    virtual void Notify(const SfxHint&) override;

public:
    SwAccessibleTable(std::shared_ptr<SwAccessibleMap> const& pInitMap,
                      const SwTabFrame* pTableFrame);

    // XInterface

    // (XInterface methods need to be implemented to disambiguate
    // between those inherited through SwAccessibleContext and
    // XAccessibleTable).

    virtual css::uno::Any SAL_CALL queryInterface(
        const css::uno::Type& aType ) override;

    virtual void SAL_CALL acquire(  ) noexcept override
        { SwAccessibleContext::acquire(); };

    virtual void SAL_CALL release(  ) noexcept override
        { SwAccessibleContext::release(); };

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) override;

    // XAccessibleContext

    /// Return this object's description.
    virtual OUString SAL_CALL
        getAccessibleDescription() override;

    // XAccessibleTable

    virtual sal_Int32 SAL_CALL getAccessibleRowCount() override;
    virtual sal_Int32 SAL_CALL getAccessibleColumnCount(  ) override;
    virtual OUString SAL_CALL getAccessibleRowDescription(
            sal_Int32 nRow ) override;
    virtual OUString SAL_CALL getAccessibleColumnDescription(
            sal_Int32 nColumn ) override;
    virtual sal_Int32 SAL_CALL getAccessibleRowExtentAt(
            sal_Int32 nRow, sal_Int32 nColumn ) override;
    virtual sal_Int32 SAL_CALL getAccessibleColumnExtentAt(
               sal_Int32 nRow, sal_Int32 nColumn ) override;
    virtual css::uno::Reference<
                css::accessibility::XAccessibleTable >
        SAL_CALL getAccessibleRowHeaders(  ) override;
    virtual css::uno::Reference<
                css::accessibility::XAccessibleTable >
        SAL_CALL getAccessibleColumnHeaders(  ) override;
    virtual css::uno::Sequence< sal_Int32 > SAL_CALL
        getSelectedAccessibleRows(  ) override;
    virtual css::uno::Sequence< sal_Int32 > SAL_CALL
        getSelectedAccessibleColumns(  ) override;
    virtual sal_Bool SAL_CALL isAccessibleRowSelected( sal_Int32 nRow ) override;
    virtual sal_Bool SAL_CALL isAccessibleColumnSelected( sal_Int32 nColumn ) override;
    virtual css::uno::Reference<
        css::accessibility::XAccessible > SAL_CALL
        getAccessibleCellAt( sal_Int32 nRow, sal_Int32 nColumn ) override;
    virtual css::uno::Reference<
        css::accessibility::XAccessible > SAL_CALL
        getAccessibleCaption(  ) override;
    virtual css::uno::Reference<
        css::accessibility::XAccessible > SAL_CALL
        getAccessibleSummary(  ) override;
    virtual sal_Bool SAL_CALL isAccessibleSelected(
            sal_Int32 nRow, sal_Int32 nColumn ) override;
    virtual sal_Int32 SAL_CALL getAccessibleIndex(
            sal_Int32 nRow, sal_Int32 nColumn ) override;
    virtual sal_Int32 SAL_CALL getAccessibleRow( sal_Int32 nChildIndex ) override;
    virtual sal_Int32 SAL_CALL getAccessibleColumn( sal_Int32 nChildIndex ) override;
    // XAccessibleTableSelection
    virtual sal_Bool SAL_CALL selectRow( sal_Int32 row ) override ;
    virtual sal_Bool SAL_CALL selectColumn( sal_Int32 column ) override ;
    virtual sal_Bool SAL_CALL unselectRow( sal_Int32 row ) override;
    virtual sal_Bool SAL_CALL unselectColumn( sal_Int32 column ) override;
    // XServiceInfo

    /** Returns an identifier for the implementation of this object.
    */
    virtual OUString SAL_CALL
        getImplementationName() override;

    /** Return whether the specified service is supported by this class.
    */
    virtual sal_Bool SAL_CALL
        supportsService (const OUString& sServiceName) override;

    /** Returns a list of all supported services.  In this case that is just
        the AccessibleContext service.
    */
    virtual css::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames() override;

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

    virtual void SAL_CALL selectAccessibleChild(
        sal_Int32 nChildIndex ) override;

    virtual sal_Bool SAL_CALL isAccessibleChildSelected(
        sal_Int32 nChildIndex ) override;

    virtual void SAL_CALL clearAccessibleSelection(  ) override;

    virtual void SAL_CALL selectAllAccessibleChildren(  ) override;

    virtual sal_Int32 SAL_CALL getSelectedAccessibleChildCount(  ) override;

    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild(
        sal_Int32 nSelectedChildIndex ) override;

    // index has to be treated as global child index.
    virtual void SAL_CALL deselectAccessibleChild(
        sal_Int32 nChildIndex ) override;

    // XAccessibleComponent
    sal_Int32 SAL_CALL getBackground() override;
    typedef std::vector< std::pair<SwAccessibleContext*,
        css::uno::WeakReference<css::accessibility::XAccessible> > > Cells_t;
    Cells_t m_vecCellAdd;
    Cells_t m_vecCellRemove;
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

    // XInterface

    virtual css::uno::Any SAL_CALL queryInterface(
        const css::uno::Type& aType ) override;

    // XAccessibleContext

    /// Return the number of currently visible children.
    virtual sal_Int32 SAL_CALL getAccessibleChildCount() override;

    /// Return the specified child or NULL if index is invalid.
    virtual css::uno::Reference< css::accessibility::XAccessible> SAL_CALL
        getAccessibleChild (sal_Int32 nIndex) override;

    // XAccessibleTable

    virtual css::uno::Reference<
                css::accessibility::XAccessibleTable >
        SAL_CALL getAccessibleRowHeaders(  ) override;
    virtual css::uno::Reference<
                css::accessibility::XAccessibleTable >
        SAL_CALL getAccessibleColumnHeaders(  ) override;

    // XServiceInfo

    /** Returns an identifier for the implementation of this object.
    */
    virtual OUString SAL_CALL
        getImplementationName() override;

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
