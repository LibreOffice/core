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
#include <svx/AccessibleShape.hxx>

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
    : mpImpl (nullptr)
{
    mpImpl = new ChildrenManagerImpl (rxParent, rxShapeList, rShapeTreeInfo, rContext);
    if (mpImpl != nullptr)
        mpImpl->Init ();
    else
        throw uno::RuntimeException(
            "ChildrenManager::ChildrenManager can't create implementation object", nullptr);
}




ChildrenManager::~ChildrenManager()
{
    if (mpImpl != nullptr)
        mpImpl->dispose();

    // emtpy
    OSL_TRACE ("~ChildrenManager");
}

long ChildrenManager::GetChildCount() const throw ()
{
    assert(mpImpl != nullptr);
    return mpImpl->GetChildCount();
}

css::uno::Reference<XAccessible> ChildrenManager::GetChild (long nIndex)
    throw (css::uno::RuntimeException,
           css::lang::IndexOutOfBoundsException)
{
    assert(mpImpl != nullptr);
    return mpImpl->GetChild (nIndex);
}

Reference<XAccessible> ChildrenManager::GetChild (const Reference<drawing::XShape>& xShape)
    throw (css::uno::RuntimeException)
{
    assert(mpImpl != nullptr);
    return mpImpl->GetChild (xShape);
}

css::uno::Reference<css::drawing::XShape> ChildrenManager::GetChildShape(long nIndex)
    throw (css::uno::RuntimeException,
           css::lang::IndexOutOfBoundsException)
{
    assert(mpImpl != nullptr);
    return mpImpl->GetChildShape(nIndex);
}

void ChildrenManager::Update (bool bCreateNewObjectsOnDemand)
{
    assert(mpImpl != nullptr);
    mpImpl->Update (bCreateNewObjectsOnDemand);
}

void ChildrenManager::SetShapeList (const css::uno::Reference<css::drawing::XShapes>& xShapeList)
{
    assert(mpImpl != nullptr);
    mpImpl->SetShapeList (xShapeList);
}

void ChildrenManager::AddAccessibleShape (css::uno::Reference<css::accessibility::XAccessible> const & shape)
{
    assert(mpImpl != nullptr);
    mpImpl->AddAccessibleShape (shape);
}

void ChildrenManager::ClearAccessibleShapeList()
{
    assert(mpImpl != nullptr);
    mpImpl->ClearAccessibleShapeList ();
}

void ChildrenManager::SetInfo (AccessibleShapeTreeInfo& rShapeTreeInfo)
{
    assert(mpImpl != nullptr);
    mpImpl->SetInfo (rShapeTreeInfo);
}

void ChildrenManager::UpdateSelection()
{
    assert(mpImpl != nullptr);
    mpImpl->UpdateSelection ();
}

bool ChildrenManager::HasFocus()
{
    assert(mpImpl != nullptr);
    return mpImpl->HasFocus ();
}

void ChildrenManager::RemoveFocus()
{
    assert(mpImpl != nullptr);
    mpImpl->RemoveFocus ();
}

// IAccessibleViewForwarderListener
void ChildrenManager::ViewForwarderChanged (ChangeType aChangeType,
        const IAccessibleViewForwarder* pViewForwarder)
{
    assert(mpImpl != nullptr);
    mpImpl->ViewForwarderChanged (aChangeType, pViewForwarder);
}

} // end of namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
