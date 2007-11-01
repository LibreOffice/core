/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChildrenManager.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-01 13:01:02 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifndef _SVX_ACCESSIBILITY_CHILDREN_MANAGER_HXX
#include <svx/ChildrenManager.hxx>
#endif
#ifndef _SVX_ACCESSIBILITY_CHILDREN_MANAGER_IMPL_HXX
#include "ChildrenManagerImpl.hxx"
#endif
#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_SHAPE_HXX
#include <svx/AccessibleShape.hxx>
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using ::com::sun::star::uno::Reference;

namespace accessibility {


//=====  AccessibleChildrenManager  ===========================================

ChildrenManager::ChildrenManager (
    const Reference<XAccessible>& rxParent,
    const Reference<drawing::XShapes>& rxShapeList,
    const AccessibleShapeTreeInfo& rShapeTreeInfo,
    AccessibleContextBase& rContext)
    : mpImpl (NULL)
{
    mpImpl = new ChildrenManagerImpl (rxParent, rxShapeList, rShapeTreeInfo, rContext);
    if (mpImpl != NULL)
        mpImpl->Init ();
    else
        throw uno::RuntimeException(
            ::rtl::OUString::createFromAscii(
                "ChildrenManager::ChildrenManager can't create implementation object"), NULL);
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




Reference<XAccessible> ChildrenManager::GetChild (long nIndex)
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
