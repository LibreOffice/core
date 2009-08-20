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
#include "AccessibleFilterMenuItem.hxx"
#include "dpcontrol.hxx"

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleStateSet.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleEventObject.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/TextSegment.hpp>

#include <vector>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::lang::IndexOutOfBoundsException;
using ::com::sun::star::uno::RuntimeException;
using ::rtl::OUString;
using ::std::vector;

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

ScAccessibleFilterMenuItem::ScAccessibleFilterMenuItem(
    const Reference<XAccessible>& rxParent, ScMenuFloatingWindow* pWin, const OUString& rName, size_t nMenuPos) :
    ScAccessibleContextBase(rxParent, AccessibleRole::MENU_ITEM),
    mpWindow(pWin),
    maName(rName),
    mnMenuPos(nMenuPos),
    mbSelected(false),
    mbEnabled(true)
{
    SetName(rName);
}

ScAccessibleFilterMenuItem::~ScAccessibleFilterMenuItem()
{
}

sal_Int32 ScAccessibleFilterMenuItem::getAccessibleChildCount()
    throw (RuntimeException)
{
    return 0;
}

Reference<XAccessible> ScAccessibleFilterMenuItem::getAccessibleChild(sal_Int32 nIndex)
    throw (RuntimeException, IndexOutOfBoundsException)
{
    throw IndexOutOfBoundsException();
    return Reference<XAccessible>();
}

Reference<XAccessibleStateSet> ScAccessibleFilterMenuItem::getAccessibleStateSet()
    throw (RuntimeException)
{
    fprintf(stdout, "ScAccessibleFilterMenuItem::getAccessibleStateSet:   called\n");
    return this;
}

OUString ScAccessibleFilterMenuItem::getImplementationName()
    throw (RuntimeException)
{
    return OUString::createFromAscii("ScAccessibleFilterMenuItem - implementation name");
}

// XAccessibleStateSet

sal_Bool ScAccessibleFilterMenuItem::isEmpty() throw (RuntimeException)
{
    fprintf(stdout, "ScAccessibleFilterMenuItem::isEmpty:   called\n");
    return (mbEnabled || mbSelected);
}

sal_Bool ScAccessibleFilterMenuItem::contains(sal_Int16 nState) throw (RuntimeException)
{
    using namespace ::com::sun::star::accessibility::AccessibleStateType;
    fprintf(stdout, "ScAccessibleFilterMenuItem::contains:   state = %d\n", nState);
    if (mbEnabled)
    {
        switch (nState)
        {
            case ENABLED:
            case FOCUSABLE:
            case SELECTABLE:
            case SENSITIVE:
                return true;
        }
    }

    if (mbSelected)
    {
        switch (nState)
        {
            case FOCUSED:
            case SELECTED:
                return true;
        }
    }
    return false;
}

sal_Bool ScAccessibleFilterMenuItem::containsAll(const Sequence<sal_Int16>& aStateSet)
    throw (RuntimeException)
{
    using namespace ::com::sun::star::accessibility::AccessibleStateType;
    fprintf(stdout, "ScAccessibleFilterMenuItem::containsAll:   called\n");
    sal_Int32 n = aStateSet.getLength();
    for (sal_Int32 i = 0; i < n; ++i)
    {
        sal_Int16 nState = aStateSet[i];
        if (mbEnabled)
        {
            switch (nState)
            {
                case ENABLED:
                case FOCUSABLE:
                case SELECTABLE:
                case SENSITIVE:
                    continue;
            }
        }
        if (mbSelected)
        {
            switch (nState)
            {
                case FOCUSED:
                case SELECTED:
                    continue;
            }
        }
        return false;
    }
    return true;
}

Sequence<sal_Int16> ScAccessibleFilterMenuItem::getStates() throw (RuntimeException)
{
    fprintf(stdout, "ScAccessibleFilterMenuItem::getStates:   name = '%s'  enabled = %d  selected = %d\n",
            rtl::OUStringToOString(getAccessibleName(), RTL_TEXTENCODING_UTF8).getStr(),
            mbEnabled, mbSelected);
    using namespace ::com::sun::star::accessibility::AccessibleStateType;
    vector<sal_Int16> aStates;
    if (mbEnabled)
    {
        aStates.push_back(ENABLED);
        aStates.push_back(FOCUSABLE);
        aStates.push_back(SELECTABLE);
        aStates.push_back(SENSITIVE);
    }

    if (mbSelected)
    {
        aStates.push_back(FOCUSED);
        aStates.push_back(SELECTED);
    }

    size_t n = aStates.size();
    Sequence<sal_Int16> aSeq(aStates.size());
    for (size_t i = 0; i < n; ++i)
        aSeq[i] = aStates[i];

    return aSeq;
}

// XAccessibleAction

sal_Int32 ScAccessibleFilterMenuItem::getAccessibleActionCount() throw (RuntimeException)
{
    return 1;
}

sal_Bool ScAccessibleFilterMenuItem::doAccessibleAction(sal_Int32 nIndex)
    throw (IndexOutOfBoundsException, RuntimeException)
{
    fprintf(stdout, "ScAccessibleFilterMenuItem::doAccessibleAction:   called\n");
    return false;
}

OUString ScAccessibleFilterMenuItem::getAccessibleActionDescription(sal_Int32 nIndex)
    throw (IndexOutOfBoundsException, RuntimeException)
{
    return OUString::createFromAscii("Add some action here...");
}

Reference<XAccessibleKeyBinding> ScAccessibleFilterMenuItem::getAccessibleActionKeyBinding(
    sal_Int32 nIndex) throw (IndexOutOfBoundsException, RuntimeException)
{
    return Reference<XAccessibleKeyBinding>();
}

Any SAL_CALL ScAccessibleFilterMenuItem::queryInterface( uno::Type const & rType )
    throw (RuntimeException)
{
    Any any = ScAccessibleContextBase::queryInterface(rType);
    if (any.hasValue())
        return any;

    return ScAccessibleFilterMenuItem_BASE::queryInterface(rType);
}

void SAL_CALL ScAccessibleFilterMenuItem::acquire() throw ()
{
    ScAccessibleContextBase::acquire();
}

void SAL_CALL ScAccessibleFilterMenuItem::release() throw ()
{
    ScAccessibleContextBase::release();
}

void ScAccessibleFilterMenuItem::select()
{
    mbSelected = true;
    CommitFocusGained();
}

void ScAccessibleFilterMenuItem::unselect()
{
    mbSelected = false;
    CommitFocusLost();
}

bool ScAccessibleFilterMenuItem::isSelected() const
{
    return mbSelected;
}

void ScAccessibleFilterMenuItem::setEnabled(bool bEnabled)
{
    mbEnabled = bEnabled;
}

