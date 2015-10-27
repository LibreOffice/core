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

#include "AccessibleGlobal.hxx"
#include "AccessibleFilterMenu.hxx"
#include "AccessibleFilterMenuItem.hxx"
#include "global.hxx"
#include "docpool.hxx"

#include <tools/gen.hxx>
#include <editeng/unoedsrc.hxx>
#include <editeng/editdata.hxx>
#include <editeng/outliner.hxx>
#include <vcl/unohelp.hxx>
#include "checklistmenu.hxx"

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleStateSet.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::accessibility::AccessibleStateType;

using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::lang::IndexOutOfBoundsException;
using ::com::sun::star::lang::IllegalArgumentException;
using ::com::sun::star::uno::RuntimeException;
using ::std::for_each;
using ::std::vector;

namespace {

class AddRemoveEventListener : public ::std::unary_function<void, Reference<XAccessible> >
{
public:
    explicit AddRemoveEventListener(const Reference<XAccessibleEventListener>& rListener, bool bAdd) :
        mxListener(rListener), mbAdd(bAdd) {}

    void operator() (const Reference<XAccessible>& xAccessible) const
    {
        if (!xAccessible.is())
            return;

        Reference<XAccessibleEventBroadcaster> xBc(xAccessible, UNO_QUERY);
        if (xBc.is())
        {
            if (mbAdd)
                xBc->addAccessibleEventListener(mxListener);
            else
                xBc->removeAccessibleEventListener(mxListener);
        }
    }
private:
    Reference<XAccessibleEventListener> mxListener;
    bool mbAdd;
};

}

ScAccessibleFilterMenu::ScAccessibleFilterMenu(const Reference<XAccessible>& rxParent, ScMenuFloatingWindow* pWin, const OUString& rName, size_t nMenuPos) :
    ScAccessibleContextBase(rxParent, AccessibleRole::MENU),
    mnMenuPos(nMenuPos),
    mpWindow(pWin),
    mbEnabled(true)
{
    SetName(rName);
}

ScAccessibleFilterMenu::~ScAccessibleFilterMenu()
{
}

// XAccessibleComponent

Reference<XAccessible> ScAccessibleFilterMenu::getAccessibleAtPoint( const css::awt::Point& /*rPoint*/ )
        throw (RuntimeException, std::exception)
{
    return this;
}

bool ScAccessibleFilterMenu::isVisible() throw (RuntimeException, std::exception)
{
    return mpWindow->IsVisible();
}

void ScAccessibleFilterMenu::grabFocus()
    throw (RuntimeException, std::exception)
{
}

sal_Int32 ScAccessibleFilterMenu::getForeground()
    throw (RuntimeException, std::exception)
{
    return 0;
}

sal_Int32 ScAccessibleFilterMenu::getBackground()
    throw (RuntimeException, std::exception)
{
    return 0;
}

// XAccessibleContext

OUString ScAccessibleFilterMenu::getAccessibleName() throw (RuntimeException, std::exception)
{
    return ScAccessibleContextBase::getAccessibleName();
}

sal_Int32 ScAccessibleFilterMenu::getAccessibleChildCount()
    throw (RuntimeException, std::exception)
{
    return getMenuItemCount();
}

Reference<XAccessible> ScAccessibleFilterMenu::getAccessibleChild(sal_Int32 nIndex)
    throw (RuntimeException, IndexOutOfBoundsException, std::exception)
{
    if (maMenuItems.size() <= static_cast<size_t>(nIndex))
        throw IndexOutOfBoundsException();

    return maMenuItems[nIndex];
}

Reference<XAccessibleStateSet> ScAccessibleFilterMenu::getAccessibleStateSet()
    throw (RuntimeException, std::exception)
{
    updateStates();
    return mxStateSet;
}

OUString ScAccessibleFilterMenu::getImplementationName()
    throw (RuntimeException, std::exception)
{
    return OUString("ScAccessibleFilterMenu");
}

// XAccessibleEventBroadcaster

void ScAccessibleFilterMenu::addAccessibleEventListener(
        const css::uno::Reference<css::accessibility::XAccessibleEventListener>& xListener)
    throw (css::uno::RuntimeException, std::exception)
{
    ScAccessibleContextBase::addAccessibleEventListener(xListener);
    for_each(maMenuItems.begin(), maMenuItems.end(), AddRemoveEventListener(xListener, true));
}

void ScAccessibleFilterMenu::removeAccessibleEventListener(
        const css::uno::Reference<css::accessibility::XAccessibleEventListener>& xListener)
    throw (css::uno::RuntimeException, std::exception)
{
    ScAccessibleContextBase::removeAccessibleEventListener(xListener);
    for_each(maMenuItems.begin(), maMenuItems.end(), AddRemoveEventListener(xListener, false));
}

// XAccessibleSelection

void ScAccessibleFilterMenu::selectAccessibleChild(sal_Int32 nChildIndex)
    throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    if (static_cast<size_t>(nChildIndex) >= maMenuItems.size())
        throw IndexOutOfBoundsException();

    mpWindow->setSelectedMenuItem(nChildIndex, false, true);
}

sal_Bool ScAccessibleFilterMenu::isAccessibleChildSelected(sal_Int32 nChildIndex)
    throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    if (static_cast<size_t>(nChildIndex) >= maMenuItems.size())
        throw IndexOutOfBoundsException();

    return mpWindow->isMenuItemSelected(static_cast<size_t>(nChildIndex));
}

void ScAccessibleFilterMenu::clearAccessibleSelection() throw (RuntimeException, std::exception)
{
    mpWindow->clearSelectedMenuItem();
}

void ScAccessibleFilterMenu::selectAllAccessibleChildren() throw (RuntimeException, std::exception)
{
    // not supported - this is a menu, you can't select all menu items.
}

sal_Int32 ScAccessibleFilterMenu::getSelectedAccessibleChildCount() throw (RuntimeException, std::exception)
{
    // Since this is a menu, either one menu item is selected, or none at all.
    return mpWindow->getSelectedMenuItem() == ScMenuFloatingWindow::MENU_NOT_SELECTED ? 0 : 1;
}

Reference<XAccessible> ScAccessibleFilterMenu::getSelectedAccessibleChild(sal_Int32 nChildIndex)
    throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    if (static_cast<size_t>(nChildIndex) >= maMenuItems.size())
        throw IndexOutOfBoundsException();

    return maMenuItems[nChildIndex];
}

void ScAccessibleFilterMenu::deselectAccessibleChild(sal_Int32 nChildIndex) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    if (static_cast<size_t>(nChildIndex) >= maMenuItems.size())
        throw IndexOutOfBoundsException();

    mpWindow->selectMenuItem(nChildIndex, false, false);
}

// XInterface

uno::Any SAL_CALL ScAccessibleFilterMenu::queryInterface( uno::Type const & rType )
    throw (RuntimeException, std::exception)
{
    Any any = ScAccessibleContextBase::queryInterface(rType);
    if (any.hasValue())
        return any;

    return ScAccessibleFilterMenu_BASE::queryInterface(rType);
}

void SAL_CALL ScAccessibleFilterMenu::acquire() throw ()
{
    ScAccessibleContextBase::acquire();
}

void SAL_CALL ScAccessibleFilterMenu::release() throw ()
{
    ScAccessibleContextBase::release();
}

// XTypeProvider

Sequence<sal_Int8> ScAccessibleFilterMenu::getImplementationId()
    throw (RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}

Rectangle ScAccessibleFilterMenu::GetBoundingBoxOnScreen() const
    throw (RuntimeException, std::exception)
{
    if (mnMenuPos == ScMenuFloatingWindow::MENU_NOT_SELECTED)
        return Rectangle();

    // Menu object's bounding box is the bounding box of the menu item that
    // launches the menu, which belongs to the parent window.
    ScMenuFloatingWindow* pParentWin = mpWindow->getParentMenuWindow();
    if (!pParentWin)
        return Rectangle();

    if (!pParentWin->IsVisible())
        return Rectangle();

    Point aPos = pParentWin->OutputToAbsoluteScreenPixel(Point(0,0));
    Point aMenuPos;
    Size aMenuSize;
    pParentWin->getMenuItemPosSize(mnMenuPos, aMenuPos, aMenuSize);
    Rectangle aRect(aPos + aMenuPos, aMenuSize);
    return aRect;
}

Rectangle ScAccessibleFilterMenu::GetBoundingBox() const
    throw (RuntimeException, std::exception)
{
    if (mnMenuPos == ScMenuFloatingWindow::MENU_NOT_SELECTED)
        return Rectangle();

    // Menu object's bounding box is the bounding box of the menu item that
    // launches the menu, which belongs to the parent window.
    ScMenuFloatingWindow* pParentWin = mpWindow->getParentMenuWindow();
    if (!pParentWin)
        return Rectangle();

    if (!pParentWin->IsVisible())
        return Rectangle();

    Point aMenuPos;
    Size aMenuSize;
    pParentWin->getMenuItemPosSize(mnMenuPos, aMenuPos, aMenuSize);
    Rectangle aRect(aMenuPos, aMenuSize);
    return aRect;
}

void ScAccessibleFilterMenu::appendMenuItem(const OUString& rName, bool bEnabled, size_t nMenuPos)
{
    // Check whether this menu item is a sub menu or a regular menu item.
    ScMenuFloatingWindow* pSubMenu = mpWindow->getSubMenuWindow(nMenuPos);
    Reference<XAccessible> xAccessible;
    if (pSubMenu)
    {
        xAccessible = pSubMenu->CreateAccessible();
        ScAccessibleFilterMenu* p =
            static_cast<ScAccessibleFilterMenu*>(xAccessible.get());
        p->setEnabled(bEnabled);
        p->setMenuPos(nMenuPos);
    }
    else
    {
        xAccessible.set(new ScAccessibleFilterMenuItem(this, mpWindow, rName, nMenuPos));
        ScAccessibleFilterMenuItem* p =
            static_cast<ScAccessibleFilterMenuItem*>(xAccessible.get());
        p->setEnabled(bEnabled);
    }
    maMenuItems.push_back(xAccessible);
}

void ScAccessibleFilterMenu::setMenuPos(size_t nMenuPos)
{
    mnMenuPos = nMenuPos;
}

void ScAccessibleFilterMenu::setEnabled(bool bEnabled)
{
    mbEnabled = bEnabled;
}

sal_Int32 ScAccessibleFilterMenu::getMenuItemCount() const
{
    return maMenuItems.size();
}

bool ScAccessibleFilterMenu::isSelected() const
{
    // Check to see if any of the child menu items is selected.
    return mpWindow->isMenuItemSelected(mnMenuPos);
}

bool ScAccessibleFilterMenu::isFocused() const
{
    return isSelected();
}

void ScAccessibleFilterMenu::updateStates()
{
    if (!mxStateSet.is())
        mxStateSet.set(new ScAccessibleStateSet);

    ScAccessibleStateSet* p = static_cast<ScAccessibleStateSet*>(
        mxStateSet.get());

    p->clear();

    p->insert(ENABLED);
    p->insert(FOCUSABLE);
    p->insert(SELECTABLE);
    p->insert(SENSITIVE);
    p->insert(OPAQUE);

    if (isFocused())
        p->insert(FOCUSED);

    if (isSelected())
        p->insert(SELECTED);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
