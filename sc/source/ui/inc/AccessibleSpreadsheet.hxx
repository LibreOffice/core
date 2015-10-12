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

#include "rangelst.hxx"
#include <map>

class ScMyAddress : public ScAddress
{
public:
    ScMyAddress() : ScAddress() {}
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
class ScRangeList;

/** @descr
        This base class provides an implementation of the
        <code>AccessibleTable</code> service.
*/
class ScAccessibleSpreadsheet
    :   public  ScAccessibleTableBase
{
public:
    //=====  internal  ========================================================
    ScAccessibleSpreadsheet(
        ScAccessibleDocument* pAccDoc,
        ScTabViewShell* pViewShell,
        SCTAB   nTab,
        ScSplitPos eSplitPos);
protected:
    ScAccessibleSpreadsheet(
        ScAccessibleSpreadsheet& rParent,
        const ScRange& rRange );

    virtual ~ScAccessibleSpreadsheet();

    void ConstructScAccessibleSpreadsheet(
        ScAccessibleDocument* pAccDoc,
        ScTabViewShell* pViewShell,
        SCTAB nTab,
        ScSplitPos eSplitPos);

    using ScAccessibleTableBase::IsDefunc;

public:
    using ScAccessibleTableBase::disposing;

     virtual void SAL_CALL disposing() override;

    void CompleteSelectionChanged(bool bNewState);

    void LostFocus();
    void GotFocus();

    void BoundingBoxChanged();
    void VisAreaChanged();

private:
    ///=====  SfxListener  =====================================================
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    ///=====  XAccessibleTable  ================================================

    /// Returns the row headers as an AccessibleTable.
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleTable > SAL_CALL
                getAccessibleRowHeaders(  )
                    throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /// Returns the column headers as an AccessibleTable.
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleTable > SAL_CALL
                getAccessibleColumnHeaders(  )
                    throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /// Returns the selected rows in a table.
    virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL
                getSelectedAccessibleRows(  )
                    throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /// Returns the selected columns in a table.
    virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL
                getSelectedAccessibleColumns(  )
                    throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /// Returns a boolean value indicating whether the specified row is selected.
    virtual sal_Bool SAL_CALL
                isAccessibleRowSelected( sal_Int32 nRow )
                    throw (::com::sun::star::uno::RuntimeException,
                    ::com::sun::star::lang::IndexOutOfBoundsException, std::exception) override;

    /// Returns a boolean value indicating whether the specified column is selected.
    virtual sal_Bool SAL_CALL
                isAccessibleColumnSelected( sal_Int32 nColumn )
                    throw (::com::sun::star::uno::RuntimeException,
                    ::com::sun::star::lang::IndexOutOfBoundsException, std::exception) override;

    /// Returns the Accessible at a specified row and column in the table.
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL
                getAccessibleCellAt( sal_Int32 nRow, sal_Int32 nColumn )
                    throw (::com::sun::star::uno::RuntimeException,
                            ::com::sun::star::lang::IndexOutOfBoundsException, std::exception) override;

    rtl::Reference<ScAccessibleCell> GetAccessibleCellAt(sal_Int32 nRow, sal_Int32 nColumn);

    /// Returns a boolean value indicating whether the accessible at a specified row and column is selected.
    virtual sal_Bool SAL_CALL
                isAccessibleSelected( sal_Int32 nRow, sal_Int32 nColumn )
                    throw (::com::sun::star::uno::RuntimeException,
                    ::com::sun::star::lang::IndexOutOfBoundsException, std::exception) override;

    ///=====  XAccessibleComponent  ============================================

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        SAL_CALL getAccessibleAtPoint(
        const ::com::sun::star::awt::Point& rPoint )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL grabFocus(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual sal_Int32 SAL_CALL getForeground(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual sal_Int32 SAL_CALL getBackground(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    ///=====  XAccessibleContext  ==============================================

    /// Return NULL to indicate that an empty relation set.
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessibleRelationSet> SAL_CALL
        getAccessibleRelationSet()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /// Return the set of current states.
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessibleStateSet> SAL_CALL
        getAccessibleStateSet()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    ///=====  XAccessibleSelection  ===========================================

    virtual void SAL_CALL
        selectAccessibleChild( sal_Int32 nChildIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::uno::RuntimeException,
               std::exception) override;

    virtual void SAL_CALL
        clearAccessibleSelection(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL
        selectAllAccessibleChildren(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual sal_Int32 SAL_CALL
        getSelectedAccessibleChildCount(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > SAL_CALL
        getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
        ::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL
        deselectAccessibleChild( sal_Int32 nChildIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::uno::RuntimeException,
               std::exception) override;

    ///=====  XServiceInfo  ====================================================

    /** Returns an identifier for the implementation of this object.
    */
    virtual OUString SAL_CALL
        getImplementationName()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /** Returns a list of all supported services.
    */
    virtual ::com::sun::star::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    ///=====  XTypeProvider  ===================================================

    /** Returns a implementation id.
    */
    virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL
        getImplementationId()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    ///=====  XAccessibleEventBroadcaster  =====================================

    /** Add listener that is informed of future changes of name,
          description and so on events.
    */
    virtual void SAL_CALL
        addAccessibleEventListener(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::accessibility::XAccessibleEventListener>& xListener)
        throw (com::sun::star::uno::RuntimeException, std::exception) override;
    //=====  XAccessibleTableSelection  ============================================
    virtual sal_Bool SAL_CALL selectRow( sal_Int32 row )
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException,
               std::exception) override;
    virtual sal_Bool SAL_CALL selectColumn( sal_Int32 column )
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException,
               std::exception) override;
    virtual sal_Bool SAL_CALL unselectRow( sal_Int32 row )
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException,
               std::exception) override;
    virtual sal_Bool SAL_CALL unselectColumn( sal_Int32 column )
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException,
               std::exception) override;

protected:
    /// Return the object's current bounding box relative to the desktop.
    virtual Rectangle GetBoundingBoxOnScreen() const
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /// Return the object's current bounding box relative to the parent object.
    virtual Rectangle GetBoundingBox() const
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
private:
    ScTabViewShell* mpViewShell;
    ScRangeList*    mpMarkedRanges;
    std::vector<ScMyAddress>* mpSortedMarkedCells;
    ScAccessibleDocument* mpAccDoc;
    rtl::Reference<ScAccessibleCell> mpAccCell;
    Rectangle       maVisCells;
    ScSplitPos      meSplitPos;
    ScAddress       maActiveCell;
    SCTAB           mnTab;
    bool            mbIsSpreadsheet;
    bool            mbHasSelection;
    bool            mbDelIns;
    bool            mbIsFocusSend;

    bool IsDefunc(
        const com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleStateSet>& rxParentStates);
    bool IsEditable(
        const com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleStateSet>& rxParentStates);
    bool IsFocused();
    bool IsCompleteSheetSelected();

    void SelectCell(sal_Int32 nRow, sal_Int32 nCol, bool bDeselect);

    static ScDocument* GetDocument(ScTabViewShell* pViewShell);
    static Rectangle   GetVisArea(ScTabViewShell* pViewShell, ScSplitPos eSplitPos);
    Rectangle   GetVisCells(const Rectangle& rVisArea);
    typedef std::vector<ScMyAddress> VEC_MYADDR;

    typedef std::map<ScMyAddress,com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > >
        MAP_ADDR_XACC;
    MAP_ADDR_XACC m_mapSelectionSend;
    void RemoveSelection(ScMarkData &refScMarkData);
    bool IsSameMarkCell();
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
    typedef std::vector<ScRange*> VEC_RANGE;
    VEC_RANGE     m_vecTempRange;
    typedef std::pair<sal_uInt16,sal_uInt16> PAIR_COL;
    typedef std::vector<PAIR_COL> VEC_COL;
    VEC_COL       m_vecTempCol;
    OUString      m_strOldTabName;

    ::com::sun::star::uno::Reference < ::com::sun::star::accessibility::XAccessible > GetActiveCell();
    bool IsScAddrFormulaSel (const ScAddress &addr) const;
    bool IsFormulaMode();
    ScMyAddress CalcScAddressFromRangeList(ScRangeList *pMarkedRanges,sal_Int32 nSelectedChildIndex);
    static bool CalcScRangeDifferenceMax(ScRange *pSrc,ScRange *pDest,int nMax,VEC_MYADDR &vecRet,int &nSize);
    static bool CalcScRangeListDifferenceMax(ScRangeList *pSrc,ScRangeList *pDest,int nMax,VEC_MYADDR &vecRet);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
