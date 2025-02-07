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

#include "accessibleeditbrowseboxcell.hxx"

#include <comphelper/accessiblecontexthelper.hxx>

using namespace com::sun::star::accessibility;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::comphelper;

EditBrowseBoxTableCell::EditBrowseBoxTableCell(svt::ControlBase* pControl)
    : ImplInheritanceHelper(pControl)
{
}

Reference<css::accessibility::XAccessibleContext>
    SAL_CALL EditBrowseBoxTableCell::getAccessibleContext()
{
    OExternalLockGuard aGuard(this);
    ensureAlive();
    return this;
}

OUString SAL_CALL EditBrowseBoxTableCell::getImplementationName()
{
    return u"com.sun.star.comp.svtools.TableCellProxy"_ustr;
}

css::uno::Reference<css::accessibility::XAccessible>
    SAL_CALL EditBrowseBoxTableCell::getAccessibleParent()
{
    OExternalLockGuard aGuard(this);

    // AccessibleBrowseBox reports this as direct child (s. AccessibleBrowseBox::getAccessibleChild),
    // so report BrowseBox's accessible object as parent as well

    // The control's parent is a BrowserDataWin (see ControlBase ctor)
    BrowserDataWin* pBrowserDataWin = dynamic_cast<BrowserDataWin*>(GetWindow()->GetParent());
    assert(pBrowserDataWin);
    BrowseBox* pBrowseBox = pBrowserDataWin->GetParent();
    assert(pBrowseBox);
    return pBrowseBox->GetAccessible();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
