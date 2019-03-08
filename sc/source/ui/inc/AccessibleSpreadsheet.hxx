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

#ifndef INCLUDED_SC_SOURCE_UI_INC_ACCESSIBLESPREADSHEET_HXX
#define INCLUDED_SC_SOURCE_UI_INC_ACCESSIBLESPREADSHEET_HXX

#include <sal/config.h>

#include <rtl/ref.hxx>

#include "AccessibleTableBase.hxx"
#include "viewdata.hxx"

#include <vector>

#include <rangelst.hxx>
#include <map>

class ScMyAddress : public ScAddress
{
public:
    ScMyAddress(SCCOL nColP, SCROW nRowP, SCTAB nTabP) : ScAddress(nColP, nRowP, nTabP) {}
    ScMyAddress(const ScAddress& rAddress) : ScAddress(rAddress) {}

    bool operator< ( const ScMyAddress& rAddress ) const
    {
        if( Row() != rAddress.Row() )
            return (Row() < rAddress.Row());
        else
            return (Col() < rAddress.Col());
    }
};

class ScTabViewShell;
class ScAccessibleDocument;
class ScAccessibleCell;

/** @descr
        This base class provides an implementation of the
        <code>AccessibleTable</code> service.
*/
class ScAccessibleSpreadsheet final
    :   public  ScAccessibleTableBase
{
public:
    ScAccessibleSpreadsheet(
        ScAccessibleDocument* pAccDoc,
        ScTabViewShell* pViewShell,
        SCTAB   nTab,
        ScSplitPos eSplitPos);

    using ScAccessibleTableBase::disposing;

    virtual void SAL_CALL disposing() override;

    void CompleteSelectionChanged(bool bNewState);

    void LostFocus();
    void GotFocus();

    void BoundingBoxChanged();
    void VisAreaChanged();

private:
    ScAccessibleSpreadsheet(
        ScAccessibleSpreadsheet& rParent,
        const ScRange& rRange );

    virtual ~ScAccessibleSpreadsheet() override;

    void ConstructScAccessibleSpreadsheet(
        ScAccessibleDocument* pAccDoc,
        ScTabViewShell* pViewShell,
        SCTAB nTab,
        ScSplitPos eSplitPos);

    using ScAccessibleTableBase::IsDefunc;

    ///=====  SfxListener  =====================================================
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    ///=====  XAccessibleTable  ================================================

    /// Returns the row headers as an AccessibleTable.
    virtual css::uno::Reference< css::accessibility::XAccessibleTable > SAL_CALL
                getAccessibleRowHeaders(  ) override;

    /// Returns the column headers as an AccessibleTable.
    virtual css::uno::Reference< css::accessibility::XAccessibleTable > SAL_CALL
                getAccessibleColumnHeaders(  ) override;

    /// Returns the selected rows in a table.
    virtual css::uno::Sequence< sal_Int32 > SAL_CALL
                getSelectedAccessibleRows(  ) override;

    /// Returns the selected columns in a table.
    virtual css::uno::Sequence< sal_Int32 > SAL_CALL
                getSelectedAccessibleColumns(  ) override;

    /// Returns a boolean value indicating whether the specified row is selected.
    virtual sal_Bool SAL_CALL
                isAccessibleRowSelected( sal_Int32 nRow ) override;

    /// Returns a boolean value indicating whether the specified column is selected.
    virtual sal_Bool SAL_CALL
                isAccessibleColumnSelected( sal_Int32 nColumn ) override;

    /// Returns the Accessible at a specified row and column in the table.
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
                getAccessibleCellAt( sal_Int32 nRow, sal_Int32 nColumn ) override;

    rtl::Reference<ScAccessibleCell> GetAccessibleCellAt(sal_Int32 nRow, sal_Int32 nColumn);

    /// Returns a boolean value indicating whether the accessible at a specified row and column is selected.
    virtual sal_Bool SAL_CALL
                isAccessibleSelected( sal_Int32 nRow, sal_Int32 nColumn ) override;

    ///=====  XAccessibleComponent  ============================================

    virtual css::uno::Reference< css::accessibility::XAccessible >
        SAL_CALL getAccessibleAtPoint(
        const css::awt::Point& rPoint ) override;

    virtual void SAL_CALL grabFocus(  ) override;

    virtual sal_Int32 SAL_CALL getForeground(  ) override;

    virtual sal_Int32 SAL_CALL getBackground(  ) override;

    ///=====  XAccessibleContext  ==============================================

    /// Return NULL to indicate that an empty relation set.
    virtual css::uno::Reference<css::accessibility::XAccessibleRelationSet> SAL_CALL
        getAccessibleRelationSet() override;

    /// Return the set of current states.
    virtual css::uno::Reference<css::accessibility::XAccessibleStateSet> SAL_CALL
        getAccessibleStateSet() override;

    ///=====  XAccessibleSelection  ===========================================

    virtual void SAL_CALL
        selectAccessibleChild( sal_Int32 nChildIndex ) override;

    virtual void SAL_CALL
        clearAccessibleSelection(  ) override;

    virtual void SAL_CALL
        selectAllAccessibleChildren(  ) override;

    virtual sal_Int32 SAL_CALL
        getSelectedAccessibleChildCount(  ) override;

    virtual css::uno::Reference<css::accessibility::XAccessible > SAL_CALL
        getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) override;

    virtual void SAL_CALL
        deselectAccessibleChild( sal_Int32 nChildIndex ) override;

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

    /** Returns a implementation id.
    */
    virtual css::uno::Sequence<sal_Int8> SAL_CALL
        getImplementationId() override;

    ///=====  XAccessibleEventBroadcaster  =====================================

    /** Add listener that is informed of future changes of name,
          description and so on events.
    */
    virtual void SAL_CALL
        addAccessibleEventListener(
            const css::uno::Reference<css::accessibility::XAccessibleEventListener>& xListener) override;
    //=====  XAccessibleTableSelection  ============================================
    virtual sal_Bool SAL_CALL selectRow( sal_Int32 row ) override;
    virtual sal_Bool SAL_CALL selectColumn( sal_Int32 column ) override;
    virtual sal_Bool SAL_CALL unselectRow( sal_Int32 row ) override;
    virtual sal_Bool SAL_CALL unselectColumn( sal_Int32 column ) override;

    /// Return the object's current bounding box relative to the desktop.
    virtual tools::Rectangle GetBoundingBoxOnScreen() const override;

    /// Return the object's current bounding box relative to the parent object.
    virtual tools::Rectangle GetBoundingBox() const override;

    ScTabViewShell* mpViewShell;
    std::unique_ptr<ScRangeList>  mpMarkedRanges;
    ScAccessibleDocument* mpAccDoc;
    rtl::Reference<ScAccessibleCell> mpAccCell;
    ScSplitPos      meSplitPos;
    ScAddress       maActiveCell;
    SCTAB           mnTab;
    bool const      mbIsSpreadsheet;
    bool            mbDelIns;
    bool            mbIsFocusSend;

    bool IsDefunc(
        const css::uno::Reference<css::accessibility::XAccessibleStateSet>& rxParentStates);
    bool IsEditable();
    bool IsFocused();
    bool IsCompleteSheetSelected();

    void SelectCell(sal_Int32 nRow, sal_Int32 nCol, bool bDeselect);

    static ScDocument* GetDocument(ScTabViewShell* pViewShell);
    typedef std::vector<ScMyAddress> VEC_MYADDR;

    typedef std::map<ScMyAddress,css::uno::Reference< css::accessibility::XAccessible > >
        MAP_ADDR_XACC;
    MAP_ADDR_XACC m_mapSelectionSend;
    void RemoveSelection(const ScMarkData &refScMarkData);
    void CommitFocusCell(const ScAddress &aNewCell);
public:
    void FireFirstCellFocus();
private:
    bool          m_bFormulaMode;
    bool          m_bFormulaLastMode;
    ScAddress     m_aFormulaActiveCell;
    MAP_ADDR_XACC m_mapFormulaSelectionSend;
    VEC_MYADDR    m_vecFormulaLastMyAddr;
    rtl::Reference<ScAccessibleCell> m_pAccFormulaCell;
    sal_uInt16    m_nMinX;
    sal_uInt16    m_nMaxX;
    sal_Int32     m_nMinY;
    sal_Int32     m_nMaxY;

    sal_Int32 GetRowAll() const { return m_nMaxY - m_nMinY + 1 ; }
    sal_uInt16  GetColAll() const { return m_nMaxX - m_nMinX + 1; }
    void NotifyRefMode();
    void RemoveFormulaSelection(bool bRemoveAll = false);
    bool CheckChildIndex(sal_Int32) const;
    ScAddress GetChildIndexAddress(sal_Int32) const;
    sal_Int32 GetAccessibleIndexFormula( sal_Int32 nRow, sal_Int32 nColumn );
    bool GetFormulaCurrentFocusCell(ScAddress &addr);
public:
    ScRange       m_aLastWithInMarkRange;
    OUString      m_strCurCellValue;
    ScRangeList   m_LastMarkedRanges;
    typedef std::vector<ScRange> VEC_RANGE;
    VEC_RANGE     m_vecTempRange;
    typedef std::pair<sal_uInt16,sal_uInt16> PAIR_COL;
    typedef std::vector<PAIR_COL> VEC_COL;
    VEC_COL       m_vecTempCol;
    OUString      m_strOldTabName;

    css::uno::Reference < css::accessibility::XAccessible > GetActiveCell();
    bool IsScAddrFormulaSel (const ScAddress &addr) const;
    bool IsFormulaMode();
    ScMyAddress CalcScAddressFromRangeList(ScRangeList *pMarkedRanges,sal_Int32 nSelectedChildIndex);
    static bool CalcScRangeDifferenceMax(const ScRange & rSrc, const ScRange & rDest,int nMax,VEC_MYADDR &vecRet,int &nSize);
    static bool CalcScRangeListDifferenceMax(ScRangeList *pSrc,ScRangeList *pDest,int nMax,VEC_MYADDR &vecRet);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
