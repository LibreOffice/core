/*************************************************************************
 *
 *  $RCSfile: AccessibleSpreadsheet.hxx,v $
 *
 *  $Revision: 1.23 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 17:15:54 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#ifndef _SC_ACCESSIBLESPREADSHEET_HXX
#define _SC_ACCESSIBLESPREADSHEET_HXX

#include "AccessibleTableBase.hxx"
#ifndef SC_VIEWDATA_HXX
#include "viewdata.hxx"
#endif

#include <vector>

class ScMyAddress : public ScAddress
{
public:
    ScMyAddress() : ScAddress() {}
    ScMyAddress(sal_uInt16 nCol, sal_uInt16 nRow, sal_uInt16 nTab) : ScAddress(nCol, nRow, nTab) {}
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
        sal_uInt16  nTab,
        ScSplitPos eSplitPos);
protected:
    virtual ~ScAccessibleSpreadsheet();
public:

     virtual void SAL_CALL disposing();

    void CompleteSelectionChanged(sal_Bool bNewState);

    virtual void LostFocus();

    virtual void GotFocus();

    void BoundingBoxChanged();

    void VisAreaChanged();

    ///=====  SfxListener  =====================================================

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    ///=====  XAccessibleTable  ================================================

    /// Returns the selected rows in a table.
    virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL
                getSelectedAccessibleRows(  )
                    throw (::com::sun::star::uno::RuntimeException);

    /// Returns the selected columns in a table.
    virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL
                getSelectedAccessibleColumns(  )
                    throw (::com::sun::star::uno::RuntimeException);

    ///  Returns a boolean value indicating whether the specified row is selected.
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
        selectAllAccessible(  )
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
};


#endif
