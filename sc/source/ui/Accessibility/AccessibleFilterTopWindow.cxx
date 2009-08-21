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

// MARKER(update_precomp.py): autogen include statement, do not remove

#include "precompiled_sc.hxx"
#include "AccessibleFilterTopWindow.hxx"
#include "AccessibleFilterMenu.hxx"
#include "dpcontrol.hxx"

#include <com/sun/star/accessibility/AccessibleRole.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using ::com::sun::star::lang::IndexOutOfBoundsException;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::RuntimeException;
using ::rtl::OUString;

ScAccessibleFilterTopWindow::ScAccessibleFilterTopWindow(
    const Reference<XAccessible>& rxParent, ScDPFieldPopupWindow* pWin, const OUString& rName, ScDocument* pDoc) :
    ScAccessibleContextBase(rxParent, AccessibleRole::PANEL),
    mpWindow(pWin),
    mpDoc(pDoc)
{
    fprintf(stdout, "ScAccessibleFilterTopWindow::ScAccessibleFilterTopWindow:   ctor (%p)\n", this);
    SetName(rName);
}

ScAccessibleFilterTopWindow::~ScAccessibleFilterTopWindow()
{
    fprintf(stdout, "ScAccessibleFilterTopWindow::~ScAccessibleFilterTopWindow:   dtor (%p)\n", this);
}

// XAccessibleContext

sal_Int32 ScAccessibleFilterTopWindow::getAccessibleChildCount() throw (RuntimeException)
{
    return 7;
}

Reference<XAccessible> ScAccessibleFilterTopWindow::getAccessibleChild(
    sal_Int32 nIndex) throw (RuntimeException, IndexOutOfBoundsException)
{
    if (nIndex >= 7)
        throw IndexOutOfBoundsException();

    switch (nIndex)
    {
        case 0:
            return getAccessibleChildMenu();
        case 1:
            return mxAccListBox;
        case 2:
            return mxAccToggleAll;
        case 3:
            return mxAccSingleOnBtn;
        case 4:
            return mxAccSingleOffBtn;
        case 5:
            return mxAccOkBtn;
        case 6:
            return mxAccCancelBtn;
        default:
            ;
    }

    return Reference<XAccessible>();
}

OUString ScAccessibleFilterTopWindow::getImplementationName() throw (RuntimeException)
{
    return OUString::createFromAscii("ScAccessibleFilterTopWindow");
}

Reference<XAccessible> ScAccessibleFilterTopWindow::getAccessibleChildMenu()
{
    if (!mxAccMenu.is())
        mxAccMenu.set(new ScAccessibleFilterMenu(this, mpWindow, getAccessibleName(), 999, mpDoc));
    return mxAccMenu;
}

void ScAccessibleFilterTopWindow::setAccessibleChild(
    const Reference<XAccessible>& rAccessible, ChildControlType eType)
{
    switch (eType)
    {
        case LISTBOX:
            mxAccListBox = rAccessible;
        break;
        case TOGGLE_ALL:
            mxAccToggleAll = rAccessible;
        break;
        case SINGLE_ON_BTN:
            mxAccSingleOnBtn = rAccessible;
        break;
        case SINGLE_OFF_BTN:
            mxAccSingleOffBtn = rAccessible;
        break;
        case OK_BTN:
            mxAccOkBtn = rAccessible;
        break;
        case CANCEL_BTN:
            mxAccCancelBtn = rAccessible;
        break;
    }
}

