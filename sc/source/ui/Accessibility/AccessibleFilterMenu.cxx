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


#include "AccessibleGlobal.hxx"
#include "AccessibleFilterMenu.hxx"
#include "AccessibleFilterMenuItem.hxx"
#include "global.hxx"
#include "docpool.hxx"

#include "tools/gen.hxx"
#include "editeng/unoedsrc.hxx"
#include "editeng/editdata.hxx"
#include "editeng/outliner.hxx"
#include "vcl/unohelp.hxx"
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
using ::rtl::OUString;
using ::std::for_each;
using ::std::vector;

// ============================================================================

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
                xBc->addEventListener(mxListener);
            else
                xBc->removeEventListener(mxListener);
        }
    }
private:
    Reference<XAccessibleEventListener> mxListener;
    bool mbAdd;
};

}

// ============================================================================

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

Reference<XAccessible> ScAccessibleFilterMenu::getAccessibleAtPoint( const ::com::sun::star::awt::Point& /*rPoint*/ )
        throw (RuntimeException)
{
    return this;
}

sal_Bool ScAccessibleFilterMenu::isVisible() throw (RuntimeException)
{
    return mpWindow->IsVisible();
}

void ScAccessibleFilterMenu::grabFocus()
    throw (RuntimeException)
{
}

sal_Int32 ScAccessibleFilterMenu::getForeground()
    throw (RuntimeException)
{
    return 0;
}

sal_Int32 ScAccessibleFilterMenu::getBackground()
    throw (RuntimeException)
{
    return 0;
}

// XAccessibleContext

OUString ScAccessibleFilterMenu::getAccessibleName() throw (RuntimeException)
{
    return ScAccessibleContextBase::getAccessibleName();
}

sal_Int32 ScAccessibleFilterMenu::getAccessibleChildCount()
    throw (RuntimeException)
{
    return getMenuItemCount();
}

Reference<XAccessible> ScAccessibleFilterMenu::getAccessibleChild(sal_Int32 nIndex)
    throw (RuntimeException, IndexOutOfBoundsException)
{
    if (maMenuItems.size() <= static_cast<size_t>(nIndex))
        throw IndexOutOfBoundsException();

    return maMenuItems[nIndex];
}

Reference<XAccessibleStateSet> ScAccessibleFilterMenu::getAccessibleStateSet()
    throw (RuntimeException)
{
    updateStates();
    return mxStateSet;
}

OUString ScAccessibleFilterMenu::getImplementationName()
    throw (RuntimeException)
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM("ScAccessibleFilterMenu"));
}

// XAccessibleEventBroadcaster

void ScAccessibleFilterMenu::addEventListener(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessibleEventListener>& xListener)
    throw (com::sun::star::uno::RuntimeException)
{
    ScAccessibleContextBase::addEventListener(xListener);
    for_each(maMenuItems.begin(), maMenuItems.end(), AddRemoveEventListener(xListener, true));
}

void ScAccessibleFilterMenu::removeEventListener(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessibleEventListener>& xListener)
    throw (com::sun::star::uno::RuntimeException)
{
    ScAccessibleContextBase::removeEventListener(xListener);
    for_each(maMenuItems.begin(), maMenuItems.end(), AddRemoveEventListener(xListener, false));
}

// XAccessibleSelection

void ScAccessibleFilterMenu::selectAccessibleChild(sal_Int32 nChildIndex)
    throw (IndexOutOfBoundsException, RuntimeException)
{
    if (static_cast<size_t>(nChildIndex) >= maMenuItems.size())
        throw IndexOutOfBoundsException();

    mpWindow->setSelectedMenuItem(nChildIndex, false, true);
}

sal_Bool ScAccessibleFilterMenu::isAccessibleChildSelected(sal_Int32 nChildIndex)
    throw (IndexOutOfBoundsException, RuntimeException)
{
    if (static_cast<size_t>(nChildIndex) >= maMenuItems.size())
        throw IndexOutOfBoundsException();

    return mpWindow->isMenuItemSelected(static_cast<size_t>(nChildIndex));
}

void ScAccessibleFilterMenu::clearAccessibleSelection() throw (RuntimeException)
{
    mpWindow->clearSelectedMenuItem();
}

void ScAccessibleFilterMenu::selectAllAccessibleChildren() throw (RuntimeException)
{
    // not suported - this is a menu, you can't select all menu items.
}

sal_Int32 ScAccessibleFilterMenu::getSelectedAccessibleChildCount() throw (RuntimeException)
{
    // Since this is a menu, either one menu item is selected, or none at all.
    return mpWindow->getSelectedMenuItem() == ScMenuFloatingWindow::MENU_NOT_SELECTED ? 0 : 1;
}

Reference<XAccessible> ScAccessibleFilterMenu::getSelectedAccessibleChild(sal_Int32 nChildIndex)
    throw (IndexOutOfBoundsException, RuntimeException)
{
    if (static_cast<size_t>(nChildIndex) >= maMenuItems.size())
        throw IndexOutOfBoundsException();

    return maMenuItems[nChildIndex];
}

void ScAccessibleFilterMenu::deselectAccessibleChild(sal_Int32 nChildIndex) throw (IndexOutOfBoundsException, RuntimeException)
{
    if (static_cast<size_t>(nChildIndex) >= maMenuItems.size())
        throw IndexOutOfBoundsException();

    mpWindow->selectMenuItem(nChildIndex, false, false);
}

// XInterface

uno::Any SAL_CALL ScAccessibleFilterMenu::queryInterface( uno::Type const & rType )
    throw (RuntimeException)
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
    throw (RuntimeException)
{
    Sequence<sal_Int8> aId(16);
    return aId;
}

Rectangle ScAccessibleFilterMenu::GetBoundingBoxOnScreen() const
    throw (RuntimeException)
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
    throw (RuntimeException)
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
