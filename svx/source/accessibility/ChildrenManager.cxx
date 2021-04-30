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

#include <svx/ChildrenManager.hxx>
#include "ChildrenManagerImpl.hxx"
#include <sal/log.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using ::com::sun::star::uno::Reference;

namespace accessibility {

// AccessibleChildrenManager
ChildrenManager::ChildrenManager (
    const css::uno::Reference<XAccessible>& rxParent,
    const css::uno::Reference<drawing::XShapes>& rxShapeList,
    const AccessibleShapeTreeInfo& rShapeTreeInfo,
    AccessibleContextBase& rContext)
    : mpImpl(
        new ChildrenManagerImpl(
            rxParent, rxShapeList, rShapeTreeInfo, rContext))
{
    mpImpl->Init ();
}


ChildrenManager::~ChildrenManager()
{
    mpImpl->dispose();

    // empty
    SAL_INFO("svx", "~ChildrenManager");
}

tools::Long ChildrenManager::GetChildCount() const noexcept
{
    return mpImpl->GetChildCount();
}

css::uno::Reference<XAccessible> ChildrenManager::GetChild (tools::Long nIndex)
{
    return mpImpl->GetChild (nIndex);
}

css::uno::Reference<css::drawing::XShape> ChildrenManager::GetChildShape(tools::Long nIndex)
{
    return mpImpl->GetChildShape(nIndex);
}

void ChildrenManager::Update (bool bCreateNewObjectsOnDemand)
{
    mpImpl->Update (bCreateNewObjectsOnDemand);
}

void ChildrenManager::SetShapeList (const css::uno::Reference<css::drawing::XShapes>& xShapeList)
{
    mpImpl->SetShapeList (xShapeList);
}

void ChildrenManager::AddAccessibleShape (css::uno::Reference<css::accessibility::XAccessible> const & shape)
{
    mpImpl->AddAccessibleShape (shape);
}

void ChildrenManager::ClearAccessibleShapeList()
{
    mpImpl->ClearAccessibleShapeList ();
}

void ChildrenManager::SetInfo (AccessibleShapeTreeInfo const & rShapeTreeInfo)
{
    mpImpl->SetInfo (rShapeTreeInfo);
}

void ChildrenManager::UpdateSelection()
{
    mpImpl->UpdateSelection ();
}

bool ChildrenManager::HasFocus() const
{
    return mpImpl->HasFocus ();
}

void ChildrenManager::RemoveFocus()
{
    mpImpl->RemoveFocus ();
}

// IAccessibleViewForwarderListener
void ChildrenManager::ViewForwarderChanged()
{
    mpImpl->ViewForwarderChanged();
}

} // end of namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
