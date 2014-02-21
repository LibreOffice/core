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


//=====  AccessibleChildrenManager  ===========================================

ChildrenManager::ChildrenManager (
    const ::com::sun::star::uno::Reference<XAccessible>& rxParent,
    const ::com::sun::star::uno::Reference<drawing::XShapes>& rxShapeList,
    const AccessibleShapeTreeInfo& rShapeTreeInfo,
    AccessibleContextBase& rContext)
    : mpImpl (NULL)
{
    mpImpl = new ChildrenManagerImpl (rxParent, rxShapeList, rShapeTreeInfo, rContext);
    if (mpImpl != NULL)
        mpImpl->Init ();
    else
        throw uno::RuntimeException(
            "ChildrenManager::ChildrenManager can't create implementation object", NULL);
}




ChildrenManager::~ChildrenManager (void)
{
    if (mpImpl != NULL)
        mpImpl->dispose();

    // emtpy
    OSL_TRACE ("~ChildrenManager");
}




long ChildrenManager::GetChildCount (void) const throw ()
{
    OSL_ASSERT (mpImpl != NULL);
    return mpImpl->GetChildCount();
}




::com::sun::star::uno::Reference<XAccessible> ChildrenManager::GetChild (long nIndex)
    throw (::com::sun::star::uno::RuntimeException,
           ::com::sun::star::lang::IndexOutOfBoundsException)
{
    OSL_ASSERT (mpImpl != NULL);
    return mpImpl->GetChild (nIndex);
}

Reference<XAccessible> ChildrenManager::GetChild (const Reference<drawing::XShape>& xShape)
    throw (::com::sun::star::uno::RuntimeException)
{
    OSL_ASSERT (mpImpl != NULL);
    return mpImpl->GetChild (xShape);
}

::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XShape> ChildrenManager::GetChildShape(long nIndex)
    throw (::com::sun::star::uno::RuntimeException,
           ::com::sun::star::lang::IndexOutOfBoundsException)
{
    OSL_ASSERT (mpImpl != NULL);
    return mpImpl->GetChildShape(nIndex);
}

void ChildrenManager::Update (bool bCreateNewObjectsOnDemand)
{
    OSL_ASSERT (mpImpl != NULL);
    mpImpl->Update (bCreateNewObjectsOnDemand);
}




void ChildrenManager::SetShapeList (const ::com::sun::star::uno::Reference<
    ::com::sun::star::drawing::XShapes>& xShapeList)
{
    OSL_ASSERT (mpImpl != NULL);
    mpImpl->SetShapeList (xShapeList);
}




void ChildrenManager::AddAccessibleShape (css::uno::Reference<css::accessibility::XAccessible> const & shape)
{
    OSL_ASSERT (mpImpl != NULL);
    mpImpl->AddAccessibleShape (shape);
}




void ChildrenManager::ClearAccessibleShapeList (void)
{
    OSL_ASSERT (mpImpl != NULL);
    mpImpl->ClearAccessibleShapeList ();
}




void ChildrenManager::SetInfo (AccessibleShapeTreeInfo& rShapeTreeInfo)
{
    OSL_ASSERT (mpImpl != NULL);
    mpImpl->SetInfo (rShapeTreeInfo);
}




void ChildrenManager::UpdateSelection (void)
{
    OSL_ASSERT (mpImpl != NULL);
    mpImpl->UpdateSelection ();
}




bool ChildrenManager::HasFocus (void)
{
    OSL_ASSERT (mpImpl != NULL);
    return mpImpl->HasFocus ();
}




void ChildrenManager::RemoveFocus (void)
{
    OSL_ASSERT (mpImpl != NULL);
    mpImpl->RemoveFocus ();
}




//=====  IAccessibleViewForwarderListener  ====================================
void ChildrenManager::ViewForwarderChanged (ChangeType aChangeType,
        const IAccessibleViewForwarder* pViewForwarder)
{
    OSL_ASSERT (mpImpl != NULL);
    mpImpl->ViewForwarderChanged (aChangeType, pViewForwarder);
}



} // end of namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
