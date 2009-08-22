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
#include "AccessibleFilterMenu.hxx"
#include "AccessibleFilterMenuItem.hxx"
#include "unoguard.hxx"
#include "global.hxx"
#include "document.hxx"
#include "docpool.hxx"

#include "tools/gen.hxx"
#include "svx/unoedsrc.hxx"
#include "svx/editdata.hxx"
#include "svx/outliner.hxx"
#include "svtools/itemset.hxx"
#include "vcl/unohelp.hxx"
#include "dpcontrol.hxx"

#include <vector>

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
using ::std::set;

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

ScAccessibleFilterMenu::ScAccessibleFilterMenu(const Reference<XAccessible>& rxParent, ScMenuFloatingWindow* pWin, const OUString& rName, size_t nMenuPos, ScDocument* pDoc) :
    ScAccessibleContextBase(rxParent, AccessibleRole::MENU),
    mnMenuPos(nMenuPos),
    mpWindow(pWin),
    mpDoc(pDoc),
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
    return maMenuItems.size();
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
    return this;
}

OUString ScAccessibleFilterMenu::getImplementationName()
    throw (RuntimeException)
{
    return OUString::createFromAscii("ScAccessibleFilterMenu");
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

// XAccessibleStateSet

sal_Bool ScAccessibleFilterMenu::isEmpty() throw (RuntimeException)
{
    return maStates.empty();
}

sal_Bool ScAccessibleFilterMenu::contains(sal_Int16 nState) throw (RuntimeException)
{
    return maStates.count(nState) > 0;
}

sal_Bool ScAccessibleFilterMenu::containsAll(const Sequence<sal_Int16>& aStateSet)
    throw (RuntimeException)
{
    sal_Int32 n = aStateSet.getLength();
    for (sal_Int32 i = 0; i < n; ++i)
    {
        if (!maStates.count(aStateSet[i]))
            // This state is not set.
            return false;
    }
    // All specified states are set.
    return true;
}

Sequence<sal_Int16> ScAccessibleFilterMenu::getStates() throw (RuntimeException)
{
    updateStates();
    Sequence<sal_Int16> aSeq(0);
    set<sal_Int16>::const_iterator itr = maStates.begin(), itrEnd = maStates.end();
    for (size_t i = 0; itr != itrEnd; ++itr, ++i)
    {
        aSeq.realloc(i+1);
        aSeq[i] = *itr;
    }
    return aSeq;
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

void ScAccessibleFilterMenu::appendMenuItem(const OUString& rName, bool bEnabled, size_t nMenuPos)
{
    // Check weather this menu item is a sub menu or a regular menu item.
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

bool ScAccessibleFilterMenu::isFocused()
{
    return isSelected();
}

bool ScAccessibleFilterMenu::isSelected()
{
    // Check to see if any of the child menu items is selected.
    return mpWindow->isMenuItemSelected(mnMenuPos);
}

void ScAccessibleFilterMenu::updateStates()
{
    maStates.clear();
    maStates.insert(ENABLED);
    maStates.insert(FOCUSABLE);
    maStates.insert(SELECTABLE);
    maStates.insert(SENSITIVE);
    maStates.insert(OPAQUE);

    if (isFocused())
        maStates.insert(FOCUSED);
    if (isSelected())
        maStates.insert(SELECTED);
}
