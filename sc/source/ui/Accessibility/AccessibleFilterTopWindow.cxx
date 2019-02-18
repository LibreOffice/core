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

#include <AccessibleFilterTopWindow.hxx>
#include <AccessibleFilterMenu.hxx>
#include <checklistmenu.hxx>

#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using ::com::sun::star::lang::IndexOutOfBoundsException;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::RuntimeException;

ScAccessibleFilterTopWindow::ScAccessibleFilterTopWindow(
    const Reference<XAccessible>& rxParent, ScCheckListMenuWindow* pWin, const OUString& rName) :
    ScAccessibleFilterMenu(rxParent, pWin, rName, ScMenuFloatingWindow::MENU_NOT_SELECTED)
{
    SetName(rName);
}

ScAccessibleFilterTopWindow::~ScAccessibleFilterTopWindow()
{
}

// XAccessibleContext

sal_Int32 ScAccessibleFilterTopWindow::getAccessibleChildCount()
{
    sal_Int32 nMenuCount = getMenuItemCount();
    return nMenuCount + 6;
}

Reference<XAccessible> ScAccessibleFilterTopWindow::getAccessibleChild(
    sal_Int32 nIndex)
{
    if (nIndex >= getAccessibleChildCount())
        throw IndexOutOfBoundsException();

    sal_Int32 nMenuCount = getMenuItemCount();
    if (nIndex < nMenuCount)
        return ScAccessibleFilterMenu::getAccessibleChild(nIndex);

    nIndex -= nMenuCount;
    switch (nIndex)
    {
        case 0:
            return mxAccEditSearchBox;
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

OUString ScAccessibleFilterTopWindow::getImplementationName()
{
    return OUString("ScAccessibleFilterTopWindow");
}

void ScAccessibleFilterTopWindow::setAccessibleChild(
    const Reference<XAccessible>& rAccessible, ChildControlType eType)
{
    switch (eType)
    {
        case EDIT_SEARCH_BOX:
            mxAccEditSearchBox = rAccessible;
        break;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
