/*************************************************************************
 *
 *  $RCSfile: AccessibleDocument.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: sab $ $Date: 2002-01-18 09:52:47 $
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


#ifndef _SC_ACCESSIBILITY_ACCESSIBLEDOCUMENT_HXX
#define _SC_ACCESSIBILITY_ACCESSIBLEDOCUMENT_HXX

#ifndef _SVTOOLS_ACCESSIBILITY_ACCESSIBLE_DOCUMENT_BASE_HXX
#include <svtools/AccessibleDocumentBase.hxx>
#endif

#ifndef _COM_SUN_STAR_SHEET_XSPREADSHEETVIEW_HPP_
#include <com/sun/star/sheet/XSpreadsheetView.hpp>
#endif

#ifndef _COM_SUN_STAR_SHEET_XSPREADSHEETDOCUMENT_HPP_
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#endif

/** @descr
        This base class provides an implementation of the
        <code>AccessibleContext</code> service.
*/

class ScAccessibleDocument
    :   public SvAccessibleDocumentBase
{
public:
    //=====  internal  ========================================================
    ScAccessibleDocument(
        const ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::accessibility::XAccessible>& rxParent,
        const com::sun::star::uno::Reference
        < com::sun::star::sheet::XSpreadsheetView >& rxSheetView);
    virtual ~ScAccessibleDocument   (void);


    //=====  XAccessibleContext  ==============================================

    /// Return the number of currently visible children.
    virtual long SAL_CALL
        getAccessibleChildCount (void);

    /// Return the specified child or NULL if index is invalid.
    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleChild (long nIndex)
        throw (::com::sun::star::uno::RuntimeException);

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

private:
    /** UNO API representation of the view of this object.
    It is used to get all needed information. It gives access to the current sheet and so on.
    */
    com::sun::star::uno::Reference < com::sun::star::sheet::XSpreadsheetView > mxSheetView;

    com::sun::star::uno::Reference < com::sun::star::frame::XModel >
        getModel(const com::sun::star::uno::Reference
            < com::sun::star::sheet::XSpreadsheetView >& rxSheetView);
    sal_Int32 getVisibleTable();
    sal_Bool ScAccessibleDocument::HasDrawPages(
                com::sun::star::uno::Reference
                    <com::sun::star::sheet::XSpreadsheetDocument>& rxDoc);
};


#endif
