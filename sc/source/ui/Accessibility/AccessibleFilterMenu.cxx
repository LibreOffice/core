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

#include <stdio.h>
#include <string>
#include <sys/time.h>

namespace {

class StackPrinter
{
public:
    explicit StackPrinter(const char* msg) :
        msMsg(msg)
    {
        fprintf(stdout, "%s: --begin\n", msMsg.c_str());
        mfStartTime = getTime();
    }

    ~StackPrinter()
    {
        double fEndTime = getTime();
        fprintf(stdout, "%s: --end (duration: %g sec)\n", msMsg.c_str(), (fEndTime-mfStartTime));
    }

    void printTime(int line) const
    {
        double fEndTime = getTime();
        fprintf(stdout, "%s: --(%d) (duration: %g sec)\n", msMsg.c_str(), line, (fEndTime-mfStartTime));
    }

private:
    double getTime() const
    {
        timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec + tv.tv_usec / 1000000.0;
    }

    ::std::string msMsg;
    double mfStartTime;
};

}

// ============================================================================

namespace {

class SelectMenuItem : public ::std::unary_function<void, ScAccessibleFilterMenu::MenuItem>
{
public:
    explicit SelectMenuItem(bool bSelect) : mbSelect(bSelect) {}

    void operator() (ScAccessibleFilterMenu::MenuItem& rItem) const
    {
        rItem.mbSelected = mbSelect;
        ScAccessibleFilterMenuItem* p = static_cast<ScAccessibleFilterMenuItem*>(rItem.mxAccessible.get());
        if (mbSelect)
            p->select();
        else
            p->unselect();
    }
private:
    bool mbSelect;
};

class AddRemoveEventListener : public ::std::unary_function<void, ScAccessibleFilterMenu::MenuItem>
{
public:
    explicit AddRemoveEventListener(const Reference<XAccessibleEventListener>& rListener, bool bAdd) :
        mxListener(rListener), mbAdd(bAdd) {}

    void operator() (ScAccessibleFilterMenu::MenuItem& rItem) const
    {
        if (!rItem.mxAccessible.is())
            return;

        Reference<XAccessibleEventBroadcaster> xBc(rItem.mxAccessible, UNO_QUERY);
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

class CountSelectedMenuItem : public ::std::unary_function<void, ScAccessibleFilterMenu::MenuItem>
{
public:
    explicit CountSelectedMenuItem() : mnCount(0) {}

    CountSelectedMenuItem(const CountSelectedMenuItem& r) :
        mnCount(r.mnCount) {}

    void operator() (ScAccessibleFilterMenu::MenuItem& rItem)
    {
//      if (rItem.mbSelected)
//          ++mnCount;
//
        ScAccessibleFilterMenuItem* p = static_cast<ScAccessibleFilterMenuItem*>(rItem.mxAccessible.get());
//      fprintf(stdout, "CountSelectedMenuItem::():   name = '%s'  selected = %d\n",
//              rtl::OUStringToOString(p->getAccessibleName(), RTL_TEXTENCODING_UTF8).getStr(), p->isSelected());
        if (p->isSelected())
            ++mnCount;
    }

    size_t getCount() const { return mnCount; }

private:
    size_t mnCount;
};

}

// ============================================================================

ScAccessibleFilterMenu::MenuItem::MenuItem() :
    mbSelected(false)
{
}

// ============================================================================

ScAccessibleFilterMenu::ScAccessibleFilterMenu(const Reference<XAccessible>& rxParent, ScMenuFloatingWindow* pWin, const OUString& rName, ScDocument* pDoc) :
    ScAccessibleContextBase(rxParent, AccessibleRole::MENU),
    mpWindow(pWin),
    mpDoc(pDoc),
    mbEnabled(true),
    mbSelected(false)
{
    fprintf(stdout, "ScAccessibleFilterMenu::ScAccessibleFilterMenu:   ctor (%p)\n", this);
    SetName(rName);
//  SetDescription(OUString::createFromAscii("parent filter menu description"));
}

ScAccessibleFilterMenu::~ScAccessibleFilterMenu()
{
    fprintf(stdout, "ScAccessibleFilterMenu::~ScAccessibleFilterMenu:   dtor (%p)\n", this);
}

// XAccessibleComponent

Reference<XAccessible> ScAccessibleFilterMenu::getAccessibleAtPoint( const ::com::sun::star::awt::Point& rPoint )
        throw (RuntimeException)
{
    fprintf(stdout, "ScAccessibleFilterMenu::getAccessibleAtPoint:   point = (%ld,%ld)\n", rPoint.X, rPoint.Y);
    return this;
}

sal_Bool ScAccessibleFilterMenu::isVisible() throw (RuntimeException)
{
    fprintf(stdout, "ScAccessibleFilterMenu::isVisible:   called\n");
    return true;
}

void ScAccessibleFilterMenu::grabFocus()
    throw (RuntimeException)
{
    fprintf(stdout, "ScAccessibleFilterMenu::grabFocus:   called\n");
}

sal_Int32 ScAccessibleFilterMenu::getForeground()
    throw (RuntimeException)
{
    fprintf(stdout, "ScAccessibleFilterMenu::getForeground:   called\n");
    return 0;
}

sal_Int32 ScAccessibleFilterMenu::getBackground()
    throw (RuntimeException)
{
    fprintf(stdout, "ScAccessibleFilterMenu::getBackground:   called\n");
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
    if (maMenuItems.size() <= nIndex)
        throw IndexOutOfBoundsException();

    return maMenuItems[nIndex].mxAccessible;
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
    return OUString::createFromAscii("ScAccessibleFilterMenu - implementation name");
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
    updateStates();
    return maStates.empty();
}

sal_Bool ScAccessibleFilterMenu::contains(sal_Int16 nState) throw (RuntimeException)
{
    updateStates();
    return maStates.count(nState) > 0;
}

sal_Bool ScAccessibleFilterMenu::containsAll(const Sequence<sal_Int16>& aStateSet)
    throw (RuntimeException)
{
    updateStates();
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
    StackPrinter __stack_printer__("ScAccessibleFilterMenu::getStates");
    updateStates();
    Sequence<sal_Int16> aSeq(0);
    set<sal_Int16>::const_iterator itr = maStates.begin(), itrEnd = maStates.end();
    for (size_t i = 0; itr != itrEnd; ++itr, ++i)
    {
        aSeq.realloc(i+1);
        aSeq[i] = *itr;
    }
    if (maStates.count(FOCUSED))
        fprintf(stdout, "ScAccessibleFilterMenu::getStates:   focused\n");
    else
        fprintf(stdout, "ScAccessibleFilterMenu::getStates:   not focused\n");
    return aSeq;
}

// XAccessibleSelection

void ScAccessibleFilterMenu::selectAccessibleChild(sal_Int32 nChildIndex)
    throw (IndexOutOfBoundsException, RuntimeException)
{
    StackPrinter __stack_printer__("********** ScAccessibleFilterMenu::selectAccessibleChild **********");
    fprintf(stdout, "ScAccessibleFilterMenu::selectAccessibleChild:   index = %ld\n", nChildIndex);
    if (nChildIndex >= maMenuItems.size())
        throw IndexOutOfBoundsException();

    maMenuItems[nChildIndex].mbSelected = true;
    mpWindow->setSelectedMenuItem(nChildIndex, false, false);
}

sal_Bool ScAccessibleFilterMenu::isAccessibleChildSelected(sal_Int32 nChildIndex)
    throw (IndexOutOfBoundsException, RuntimeException)
{
    fprintf(stdout, "ScAccessibleFilterMenu::isAccessibleChildSelected:   index = %ld\n", nChildIndex);
    if (nChildIndex >= maMenuItems.size())
        throw IndexOutOfBoundsException();

    return maMenuItems[nChildIndex].mbSelected;
}

void ScAccessibleFilterMenu::clearAccessibleSelection() throw (RuntimeException)
{
    fprintf(stdout, "ScAccessibleFilterMenu::clearAccessibleSelection:   called\n");
    for_each(maMenuItems.begin(), maMenuItems.end(), SelectMenuItem(false));
    mpWindow->clearSelectedMenuItem(false);
}

void ScAccessibleFilterMenu::selectAllAccessibleChildren() throw (RuntimeException)
{
    // not suported - this is a menu, you can't select all menu items.

//  fprintf(stdout, "ScAccessibleFilterMenu::selectAllAccessibleChildren:   called\n");
//  for_each(maMenuItems.begin(), maMenuItems.end(), SelectMenuItem(true));
}

sal_Int32 ScAccessibleFilterMenu::getSelectedAccessibleChildCount() throw (RuntimeException)
{
    sal_Int32 n = for_each(maMenuItems.begin(), maMenuItems.end(), CountSelectedMenuItem()).getCount();
    fprintf(stdout, "ScAccessibleFilterMenu::getSelectedAccessibleChildCount:   count = %ld\n", n);
    return n;
}

Reference<XAccessible> ScAccessibleFilterMenu::getSelectedAccessibleChild(sal_Int32 nChildIndex)
    throw (IndexOutOfBoundsException, RuntimeException)
{
    fprintf(stdout, "ScAccessibleFilterMenu::getSelectedAccessibleChild:   index = %ld\n", nChildIndex);
    if (static_cast<size_t>(nChildIndex) >= maMenuItems.size())
        throw IndexOutOfBoundsException();

    return maMenuItems[nChildIndex].mxAccessible;
}

void ScAccessibleFilterMenu::deselectAccessibleChild(sal_Int32 nChildIndex) throw (IndexOutOfBoundsException, RuntimeException)
{
    fprintf(stdout, "ScAccessibleFilterMenu::deselectAccessibleChild:   index = %ld\n", nChildIndex);
    if (static_cast<size_t>(nChildIndex) >= maMenuItems.size())
        throw IndexOutOfBoundsException();

    maMenuItems[nChildIndex].mbSelected = false;
    ScAccessibleFilterMenuItem* p = static_cast<ScAccessibleFilterMenuItem*>(
        maMenuItems[nChildIndex].mxAccessible.get());
    p->unselect();

    mpWindow->selectMenuItem(nChildIndex, false, false, false);
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

void ScAccessibleFilterMenu::selectMenuItem(size_t nIndex, bool bSelect)
{
    StackPrinter __stack_printer__("ScAccessibleFilterMenu::selectMenuItem");
    fprintf(stdout, "ScAccessibleFilterMenu::selectMenuItem:   index = %d  select = %d\n", nIndex, bSelect);
    if (maMenuItems.size() <= nIndex)
        return;

    AccessibleEventObject aEvent;
    aEvent.EventId = AccessibleEventId::SELECTION_CHANGED;
    CommitChange(aEvent);

    maMenuItems[nIndex].mbSelected = bSelect;
    ScAccessibleFilterMenuItem* p = static_cast<ScAccessibleFilterMenuItem*>(
        maMenuItems[nIndex].mxAccessible.get());
    if (bSelect)
        p->select();
    else
        p->unselect();

    isSelected();
//  aEvent.EventId = AccessibleEventId::STATE_CHANGED;
//  CommitChange(aEvent);
}

void ScAccessibleFilterMenu::appendMenuItem(const OUString& rName, bool bEnabled, size_t nMenuPos)
{
    // Check weather this menu item is a sub menu or a regular menu item.
    ScMenuFloatingWindow* pSubMenu = mpWindow->getSubMenuWindow(nMenuPos);
    MenuItem aItem;
    if (pSubMenu)
    {
        aItem.mxAccessible = pSubMenu->CreateAccessible();
        ScAccessibleFilterMenu* p = static_cast<ScAccessibleFilterMenu*>(
            aItem.mxAccessible.get());
        p->setEnabled(bEnabled);
    }
    else
    {
        aItem.mxAccessible.set(new ScAccessibleFilterMenuItem(this, mpWindow, rName, nMenuPos));
        ScAccessibleFilterMenuItem* p = static_cast<ScAccessibleFilterMenuItem*>(
            aItem.mxAccessible.get());
        p->setEnabled(bEnabled);
    }
    maMenuItems.push_back(aItem);
}

void ScAccessibleFilterMenu::setEnabled(bool bEnabled)
{
    mbEnabled = bEnabled;
}

bool ScAccessibleFilterMenu::isFocused()
{
    return true;
//  return isSelected();
}

bool ScAccessibleFilterMenu::isSelected()
{
    // Check to see if any of the child menu items is selected.
    size_t nSelectCount = for_each(maMenuItems.begin(), maMenuItems.end(), CountSelectedMenuItem()).getCount();
//  fprintf(stdout, "ScAccessibleFilterMenu::isSelected:   selected count = %d\n", nSelectCount);
    return nSelectCount == 0;
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
