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

#include <AccessibleGlobal.hxx>
#include <AccessibleFilterMenuItem.hxx>
#include <checklistmenu.hxx>

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::accessibility::AccessibleStateType;

using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::lang::IndexOutOfBoundsException;
using ::com::sun::star::uno::RuntimeException;

ScAccessibleFilterMenuItem::ScAccessibleFilterMenuItem(
    const Reference<XAccessible>& rxParent, ScMenuFloatingWindow* pWin, const OUString& rName, size_t nMenuPos) :
    ScAccessibleContextBase(rxParent, AccessibleRole::MENU_ITEM),
    mpWindow(pWin),
    mnMenuPos(nMenuPos)
{
    SetName(rName);
}

ScAccessibleFilterMenuItem::~ScAccessibleFilterMenuItem()
{
}

sal_Int32 ScAccessibleFilterMenuItem::getAccessibleChildCount()
{
    return 0;
}

Reference<XAccessible> ScAccessibleFilterMenuItem::getAccessibleChild(sal_Int32 /*nIndex*/)
{
    throw IndexOutOfBoundsException();
}

Reference<XAccessibleStateSet> ScAccessibleFilterMenuItem::getAccessibleStateSet()
{
    updateStateSet();
    return mxStateSet;
}

OUString ScAccessibleFilterMenuItem::getImplementationName()
{
    return OUString("ScAccessibleFilterMenuItem");
}

// XAccessibleAction

sal_Int32 ScAccessibleFilterMenuItem::getAccessibleActionCount()
{
    return 1;
}

sal_Bool ScAccessibleFilterMenuItem::doAccessibleAction(sal_Int32 /*nIndex*/)
{
    mpWindow->executeMenuItem(mnMenuPos);
    return true;
}

OUString ScAccessibleFilterMenuItem::getAccessibleActionDescription(sal_Int32 /*nIndex*/)
{
    return OUString("click");
}

Reference<XAccessibleKeyBinding> ScAccessibleFilterMenuItem::getAccessibleActionKeyBinding(
    sal_Int32 /*nIndex*/)
{
    return Reference<XAccessibleKeyBinding>();
}

Any SAL_CALL ScAccessibleFilterMenuItem::queryInterface( uno::Type const & rType )
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

bool ScAccessibleFilterMenuItem::isSelected() const
{
    return mpWindow->isMenuItemSelected(mnMenuPos);
}

tools::Rectangle ScAccessibleFilterMenuItem::GetBoundingBoxOnScreen() const
{
    if (!mpWindow->IsVisible())
        return tools::Rectangle();

    Point aPos = mpWindow->OutputToAbsoluteScreenPixel(Point(0,0));
    Point aMenuPos;
    Size aMenuSize;
    mpWindow->getMenuItemPosSize(mnMenuPos, aMenuPos, aMenuSize);
    tools::Rectangle aRect(aPos + aMenuPos, aMenuSize);
    return aRect;
}

tools::Rectangle ScAccessibleFilterMenuItem::GetBoundingBox() const
{
    if (!mpWindow->IsVisible())
        return tools::Rectangle();

    Point aMenuPos;
    Size aMenuSize;
    mpWindow->getMenuItemPosSize(mnMenuPos, aMenuPos, aMenuSize);
    tools::Rectangle aRect(aMenuPos, aMenuSize);
    return aRect;
}

void ScAccessibleFilterMenuItem::updateStateSet()
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

    if (isSelected())
        p->insert(FOCUSED);

    if (isSelected())
        p->insert(SELECTED);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
