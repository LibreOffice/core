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
    SetName(rName);
}

ScAccessibleFilterTopWindow::~ScAccessibleFilterTopWindow()
{
}

// XAccessibleContext

sal_Int32 ScAccessibleFilterTopWindow::getAccessibleChildCount() throw (RuntimeException)
{
    return 2;
}

Reference<XAccessible> ScAccessibleFilterTopWindow::getAccessibleChild(
    sal_Int32 nIndex) throw (RuntimeException, IndexOutOfBoundsException)
{
    if (nIndex >= 2)
        throw IndexOutOfBoundsException();

    if (nIndex == 0)
        return getAccessibleChildMenu();

    if (nIndex == 1)
        return mxAccessibleListBox;

    return Reference<XAccessible>();
}

OUString ScAccessibleFilterTopWindow::getImplementationName() throw (RuntimeException)
{
    return OUString::createFromAscii("ScAccessibleFilterTopWindow");
}

Reference<XAccessible> ScAccessibleFilterTopWindow::getAccessibleChildMenu()
{
    if (!mxAccessibleMenu.is())
        mxAccessibleMenu.set(new ScAccessibleFilterMenu(this, mpWindow, getAccessibleName(), mpDoc));
    return mxAccessibleMenu;
}

void ScAccessibleFilterTopWindow::setAccessibleChildListBox(const Reference<XAccessible>& rAccessible)
{
    mxAccessibleListBox = rAccessible;
}

