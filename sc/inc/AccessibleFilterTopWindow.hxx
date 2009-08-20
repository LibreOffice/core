/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: AccessibleDataPilotControl.hxx,v $
 * $Revision: 1.6 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SC_ACCESSIBLEFILTERTOPWINDOW_HXX
#define SC_ACCESSIBLEFILTERTOPWINDOW_HXX

#include "AccessibleContextBase.hxx"
#include "cppuhelper/implbase1.hxx"

class ScDPFieldPopupWindow;
class ScDocument;

class ScAccessibleFilterTopWindow : public ScAccessibleContextBase
{
public:
    ScAccessibleFilterTopWindow(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible>& rxParent,
            ScDPFieldPopupWindow* pWin, const ::rtl::OUString& rName, ScDocument* pDoc);
    virtual ~ScAccessibleFilterTopWindow();

    // XAccessibleContext

    virtual sal_Int32 SAL_CALL getAccessibleChildCount()
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleChild(sal_Int32 nIndex)
            throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IndexOutOfBoundsException);

    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw (::com::sun::star::uno::RuntimeException);

    // Non-UNO Methods

    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        getAccessibleChildMenu();

    void setAccessibleChildListBox(
        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& rAccessible);

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        mxAccessibleMenu;

    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        mxAccessibleListBox;

    ScDPFieldPopupWindow* mpWindow;
    ScDocument* mpDoc;
};

#endif
