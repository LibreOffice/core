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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
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
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                "ChildrenManager::ChildrenManager can't create implementation object")), NULL);
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




void ChildrenManager::AddAccessibleShape (std::auto_ptr<AccessibleShape> pShape)
{
    OSL_ASSERT (mpImpl != NULL);
    mpImpl->AddAccessibleShape (pShape);
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
