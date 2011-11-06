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
    ScAccessibleFilterMenu(rxParent, pWin, rName, ScMenuFloatingWindow::MENU_NOT_SELECTED, pDoc),
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
    sal_Int32 nMenuCount = getMenuItemCount();
    return nMenuCount + 6;
}

Reference<XAccessible> ScAccessibleFilterTopWindow::getAccessibleChild(
    sal_Int32 nIndex) throw (RuntimeException, IndexOutOfBoundsException)
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
            return mxAccListBox;
        case 1:
            return mxAccToggleAll;
        case 2:
            return mxAccSingleOnBtn;
        case 3:
            return mxAccSingleOffBtn;
        case 4:
            return mxAccOkBtn;
        case 5:
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

