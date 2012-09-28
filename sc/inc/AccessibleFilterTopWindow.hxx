/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include "AccessibleFilterMenu.hxx"
#include "cppuhelper/implbase1.hxx"

class ScCheckListMenuWindow;

class ScAccessibleFilterTopWindow : public ScAccessibleFilterMenu
{
public:
    ScAccessibleFilterTopWindow(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible>& rxParent,
            ScCheckListMenuWindow* pWin, const ::rtl::OUString& rName);
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

    enum ChildControlType {
        LISTBOX, TOGGLE_ALL, SINGLE_ON_BTN, SINGLE_OFF_BTN, OK_BTN, CANCEL_BTN
    };
    void setAccessibleChild(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible >& rAccessible,
        ChildControlType eType);

private:
    /** The top menu part */
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        mxAccMenu;

    /** check list box for field member visibility */
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        mxAccListBox;

    /** check box for toggling all field member's visibility. */
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        mxAccToggleAll;

    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        mxAccSingleOnBtn;

    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        mxAccSingleOffBtn;

    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        mxAccOkBtn;

    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        mxAccCancelBtn;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
