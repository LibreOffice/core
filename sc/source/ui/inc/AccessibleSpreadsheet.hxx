/*************************************************************************
 *
 *  $RCSfile: AccessibleSpreadsheet.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: sab $ $Date: 2002-02-14 16:47:38 $
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

class ScTabViewShell;

/** @descr
        This base class provides an implementation of the
        <code>AccessibleTable</code> service.
*/
class ScAccessibleSpreadsheet
    :   public  ScAccessibleTableBase
{
public:
    //=====  internal  ========================================================
    ScAccessibleSpreadsheet (
        const ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::accessibility::XAccessible>& rxParent,
        ScTabViewShell* pViewShell,
        sal_uInt16  nTab,
        ScSplitPos eSplitPos);
    virtual ~ScAccessibleSpreadsheet ();

    void SetDefunc();

    //=====  SfxListener  =====================================================

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    //=====  XAccessibleTable  ================================================

    /// Returns the Accessible at a specified row and column in the table.
    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible > SAL_CALL
                getAccessibleCellAt( sal_Int32 nRow, sal_Int32 nColumn )
                    throw (::com::sun::star::uno::RuntimeException);

    //=====  XAccessibleComponent  ============================================

    /** Returns the Accessible child that is rendered under the given point.

        @param aPoint
            Coordinates of the test point for which to find the Accessible
            child.

        @return
            If there is one child which is rendered so that its bounding box
            contains the test point then a reference to that object is
            returned.  If there is more than one child which satisfies that
            condition then a reference to that one is returned that is
            painted on top of the others.  If no there is no child which is
            rendered at the test point an empty reference is returned.
    */
    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible >
        SAL_CALL getAccessibleAt(
        const ::com::sun::star::awt::Point& rPoint )
        throw (::com::sun::star::uno::RuntimeException);

    //=====  XAccessibleContext  ==============================================

    /// Return the set of current states.
    virtual ::com::sun::star::uno::Reference<
            ::drafts::com::sun::star::accessibility::XAccessibleStateSet> SAL_CALL
        getAccessibleStateSet (void)
        throw (::com::sun::star::uno::RuntimeException);

    //=====  XServiceInfo  ====================================================

    /** Returns an identifier for the implementation of this object.
    */
    virtual ::rtl::OUString SAL_CALL
        getImplementationName (void)
        throw (::com::sun::star::uno::RuntimeException);

protected:
    /// Return the object's current bounding box relative to the desktop.
    virtual Rectangle GetBoundingBoxOnScreen(void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Return the object's current bounding box relative to the parent object.
    virtual Rectangle GetBoundingBox(void)
        throw (::com::sun::star::uno::RuntimeException);
private:
    ScTabViewShell* mpViewShell;
    ScSplitPos      meSplitPos;
    ScAddress       maActiveCell;

    sal_Bool IsDefunc(
        const com::sun::star::uno::Reference<
        ::drafts::com::sun::star::accessibility::XAccessibleStateSet>& rxParentStates);
    sal_Bool IsEditable(
        const com::sun::star::uno::Reference<
        ::drafts::com::sun::star::accessibility::XAccessibleStateSet>& rxParentStates);
    sal_Bool IsCompleteSheetSelected(
        const com::sun::star::uno::Reference<
        ::drafts::com::sun::star::accessibility::XAccessibleStateSet>& rxParentStates);
    sal_Bool IsShowing(
        const com::sun::star::uno::Reference<
        ::drafts::com::sun::star::accessibility::XAccessibleStateSet>& rxParentStates);
    sal_Bool IsVisible(
        const com::sun::star::uno::Reference<
        ::drafts::com::sun::star::accessibility::XAccessibleStateSet>& rxParentStates);

    ScDocument* GetDocument(ScTabViewShell* mpViewShell);
};


#endif
