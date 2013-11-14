/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/




#ifndef _SC_ACCESSIBLESPREADSHEET_HXX
#define _SC_ACCESSIBLESPREADSHEET_HXX

#include "AccessibleTableBase.hxx"
#include "viewdata.hxx"

#include <vector>

//IAccessibility2 Implementation 2009-----
#include "rangelst.hxx"
#include <map>
//-----IAccessibility2 Implementation 2009
class ScMyAddress : public ScAddress
{
public:
    ScMyAddress() : ScAddress() {}
    ScMyAddress(SCCOL nColP, SCROW nRowP, SCTAB nTabP) : ScAddress(nColP, nRowP, nTabP) {}
    ScMyAddress(const ScAddress& rAddress) : ScAddress(rAddress) {}

    sal_Bool operator< ( const ScMyAddress& rAddress ) const
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
    using ScAccessibleTableBase::addEventListener;
    using ScAccessibleTableBase::disposing;

     virtual void SAL_CALL disposing();

    void CompleteSelectionChanged(sal_Bool bNewState);

    virtual void LostFocus();

    virtual void GotFocus();

    void BoundingBoxChanged();

    void VisAreaChanged();

    ///=====  SfxListener  =====================================================

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    ///=====  XAccessibleTable  ================================================

    /// Returns the row headers as an AccessibleTable.
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleTable > SAL_CALL
                getAccessibleRowHeaders(  )
                    throw (::com::sun::star::uno::RuntimeException);

    /// Returns the column headers as an AccessibleTable.
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleTable > SAL_CALL
                getAccessibleColumnHeaders(  )
                    throw (::com::sun::star::uno::RuntimeException);

    /// Returns the selected rows in a table.
    virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL
                getSelectedAccessibleRows(  )
                    throw (::com::sun::star::uno::RuntimeException);

    /// Returns the selected columns in a table.
    virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL
                getSelectedAccessibleColumns(  )
                    throw (::com::sun::star::uno::RuntimeException);

    /// Returns a boolean value indicating whether the specified row is selected.
    virtual sal_Bool SAL_CALL
                isAccessibleRowSelected( sal_Int32 nRow )
                    throw (::com::sun::star::uno::RuntimeException,
                    ::com::sun::star::lang::IndexOutOfBoundsException);

    /// Returns a boolean value indicating whether the specified column is selected.
    virtual sal_Bool SAL_CALL
                isAccessibleColumnSelected( sal_Int32 nColumn )
                    throw (::com::sun::star::uno::RuntimeException,
                    ::com::sun::star::lang::IndexOutOfBoundsException);

    /// Returns the Accessible at a specified row and column in the table.
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL
                getAccessibleCellAt( sal_Int32 nRow, sal_Int32 nColumn )
                    throw (::com::sun::star::uno::RuntimeException,
                            ::com::sun::star::lang::IndexOutOfBoundsException);

    ScAccessibleCell* GetAccessibleCellAt(sal_Int32 nRow, sal_Int32 nColumn);

    /// Returns a boolean value indicating whether the accessible at a specified row and column is selected.
    virtual sal_Bool SAL_CALL
                isAccessibleSelected( sal_Int32 nRow, sal_Int32 nColumn )
                    throw (::com::sun::star::uno::RuntimeException,
                    ::com::sun::star::lang::IndexOutOfBoundsException);

    ///=====  XAccessibleComponent  ============================================

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        SAL_CALL getAccessibleAtPoint(
        const ::com::sun::star::awt::Point& rPoint )
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL grabFocus(  )
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getForeground(  )
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getBackground(  )
        throw (::com::sun::star::uno::RuntimeException);

    ///=====  XAccessibleContext  ==============================================

    /// Return NULL to indicate that an empty relation set.
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessibleRelationSet> SAL_CALL
        getAccessibleRelationSet(void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Return the set of current states.
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessibleStateSet> SAL_CALL
        getAccessibleStateSet(void)
        throw (::com::sun::star::uno::RuntimeException);

    ///=====  XAccessibleSelection  ===========================================

    virtual void SAL_CALL
        selectAccessibleChild( sal_Int32 nChildIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
        ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        clearAccessibleSelection(  )
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        selectAllAccessibleChildren(  )
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL
        getSelectedAccessibleChildCount(  )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > SAL_CALL
        getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
        ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        deselectAccessibleChild( sal_Int32 nChildIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
        ::com::sun::star::uno::RuntimeException);

    ///=====  XServiceInfo  ====================================================

    /** Returns an identifier for the implementation of this object.
    */
    virtual ::rtl::OUString SAL_CALL
        getImplementationName(void)
        throw (::com::sun::star::uno::RuntimeException);

    /** Returns a list of all supported services.
    */
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL
        getSupportedServiceNames(void)
        throw (::com::sun::star::uno::RuntimeException);

    ///=====  XTypeProvider  ===================================================

    /** Returns a implementation id.
    */
    virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL
        getImplementationId(void)
        throw (::com::sun::star::uno::RuntimeException);

    ///=====  XAccessibleEventBroadcaster  =====================================

    /** Add listener that is informed of future changes of name,
          description and so on events.
    */
    virtual void SAL_CALL
        addEventListener(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::accessibility::XAccessibleEventListener>& xListener)
        throw (com::sun::star::uno::RuntimeException);
//IAccessibility2 Implementation 2009-----
    //=====  XAccessibleTableSelection  ============================================
    virtual sal_Bool SAL_CALL selectRow( sal_Int32 row )
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException) ;
    virtual sal_Bool SAL_CALL selectColumn( sal_Int32 column )
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException) ;
    virtual sal_Bool SAL_CALL unselectRow( sal_Int32 row )
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException) ;
    virtual sal_Bool SAL_CALL unselectColumn( sal_Int32 column )
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException) ;
//-----IAccessibility2 Implementation 2009

protected:
    /// Return the object's current bounding box relative to the desktop.
    virtual Rectangle GetBoundingBoxOnScreen(void) const
        throw (::com::sun::star::uno::RuntimeException);

    /// Return the object's current bounding box relative to the parent object.
    virtual Rectangle GetBoundingBox(void) const
        throw (::com::sun::star::uno::RuntimeException);
private:
    ScTabViewShell* mpViewShell;
    ScRangeList*    mpMarkedRanges;
    std::vector<ScMyAddress>* mpSortedMarkedCells;
    ScAccessibleDocument* mpAccDoc;
    ScAccessibleCell*   mpAccCell;
    Rectangle       maVisCells;
    ScSplitPos      meSplitPos;
    ScAddress       maActiveCell;
    SCTAB           mnTab;
    sal_Bool        mbIsSpreadsheet;
    sal_Bool        mbHasSelection;
    sal_Bool        mbDelIns;
    sal_Bool        mbIsFocusSend;

    sal_Bool IsDefunc(
        const com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleStateSet>& rxParentStates);
    sal_Bool IsEditable(
        const com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleStateSet>& rxParentStates);
    sal_Bool IsFocused();
    sal_Bool IsCompleteSheetSelected();

    void SelectCell(sal_Int32 nRow, sal_Int32 nCol, sal_Bool bDeselect);
    void CreateSortedMarkedCells();
    void AddMarkedRange(const ScRange& rRange);

    ScDocument* GetDocument(ScTabViewShell* pViewShell);
    Rectangle   GetVisArea(ScTabViewShell* pViewShell, ScSplitPos eSplitPos);
    Rectangle   GetVisCells(const Rectangle& rVisArea);
//IAccessibility2 Implementation 2009-----
    //void CreateSortedMarkedCells();
    //void AddMarkedRange(const ScRange& rRange);
    typedef std::vector<ScMyAddress> VEC_MYADDR;

    typedef std::map<ScMyAddress,com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > >
        MAP_ADDR_XACC;
    MAP_ADDR_XACC m_mapSelectionSend;
    void RemoveSelection(ScMarkData &refScMarkData);
    sal_Bool IsSameMarkCell();
    void CommitFocusCell(const ScAddress &aNewCell);
public:
    void FireFirstCellFocus();
private:
    sal_Bool m_bFormulaMode;
    sal_Bool m_bFormulaLastMode;
    ScAddress m_aFormulaActiveCell;
    MAP_ADDR_XACC m_mapFormulaSelectionSend;
    VEC_MYADDR m_vecFormulaLastMyAddr;
    ScAccessibleCell*   m_pAccFormulaCell;
    sal_uInt16 m_nMinX;
    sal_uInt16 m_nMaxX;
    sal_Int32 m_nMinY;
    sal_Int32 m_nMaxY;
    sal_Int32 GetRowAll() const { return m_nMaxY - m_nMinY + 1 ; }
    sal_uInt16  GetColAll() const { return m_nMaxX - m_nMinX + 1; }
    void NotifyRefMode();
    void RemoveFormulaSelection(sal_Bool bRemoveAll =sal_False);
    sal_Bool CheckChildIndex(sal_Int32)const ;
    ScAddress GetChildIndexAddress(sal_Int32) const;
    sal_Int32 GetAccessibleIndexFormula( sal_Int32 nRow, sal_Int32 nColumn );
    sal_Bool GetFormulaCurrentFocusCell(ScAddress &addr);
public:
    sal_Bool IsScAddrFormulaSel (const ScAddress &addr) const ;
    sal_Bool IsFormulaMode() ;
    ::com::sun::star::uno::Reference < ::com::sun::star::accessibility::XAccessible > GetActiveCell();
    ScRange m_aLastWithInMarkRange;
    String m_strCurCellValue;
    ScRangeList m_LastMarkedRanges;
    typedef std::vector<ScRange*> VEC_RANGE;
    VEC_RANGE m_vecTempRange;
    typedef std::pair<sal_uInt16,sal_uInt16> PAIR_COL;
    typedef std::vector<PAIR_COL> VEC_COL;
    VEC_COL m_vecTempCol;
    ScMyAddress CalcScAddressFromRangeList(ScRangeList *pMarkedRanges,sal_Int32 nSelectedChildIndex);
    sal_Bool CalcScRangeDifferenceMax(ScRange *pSrc,ScRange *pDest,int nMax,VEC_MYADDR &vecRet,int &nSize);
    sal_Bool CalcScRangeListDifferenceMax(ScRangeList *pSrc,ScRangeList *pDest,int nMax,VEC_MYADDR &vecRet);
    String m_strOldTabName;
//-----IAccessibility2 Implementation 2009
};


#endif
