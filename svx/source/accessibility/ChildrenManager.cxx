/*************************************************************************
 *
 *  $RCSfile: ChildrenManager.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: af $ $Date: 2002-02-08 16:59:32 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SVX_ACCESSIBILITY_CHILDREN_MANAGER_HXX
#include "ChildrenManager.hxx"
#endif
#ifndef _SVX_ACCESSIBILITY_SHAPE_TYPE_HANDLER_HXX
#include "ShapeTypeHandler.hxx"
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XIndexAccess_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_INDEXOUTOFBOUNDSEXCEPTION_HPP_
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::accessibility;

namespace accessibility {


//=====  AccessibleChildrenManager  ===========================================

ChildrenManager::ChildrenManager (
    const ::com::sun::star::uno::Reference<
    ::drafts::com::sun::star::accessibility::XAccessible>& rxParent)
    : mxParent (rxParent)
{
    // emtpy
}




ChildrenManager::~ChildrenManager (void)
{
    // emtpy
}




long ChildrenManager::getChildCount (void) const throw ()
{
    return maChildDescriptorList.size();
}




/** Return the requested accessible child object.  Create it if it is not
    yet in the cache.
*/
::com::sun::star::uno::Reference<
    ::drafts::com::sun::star::accessibility::XAccessible>
    ChildrenManager::getChild (long nIndex)
    throw (::com::sun::star::uno::RuntimeException)
{
    // Check wether the given index is valid.
    if (nIndex < 0 || (unsigned long)nIndex >= maChildDescriptorList.size())
        throw lang::IndexOutOfBoundsException (
            ::rtl::OUString::createFromAscii ("no accessible child with index " + nIndex),
            mxParent);

    ChildDescriptor& aChildDescriptor = maChildDescriptorList[nIndex];
    if ( ! aChildDescriptor.mxAccessibleShape.is())
    {
        ::osl::Guard< ::osl::Mutex> aGuard (::osl::Mutex::getGlobalMutex());
        // Make sure that the requested accessible object has not been
        // created while locking the global mutex.
        if ( ! aChildDescriptor.mxAccessibleShape.is())
        {
            // Create corresponding accessible object.
            aChildDescriptor.mxAccessibleShape =
                ShapeTypeHandler::Instance().createAccessibleObject (
                    aChildDescriptor.mxShape,
                    mxParent);
        }
    }

    return aChildDescriptor.mxAccessibleShape;
}




/** For now forward the update of the shape list to the general update
    method.
*/
void ChildrenManager::update (const ::com::sun::star::uno::Reference<
    ::com::sun::star::drawing::XShapes>& xShapeList)
{
    update (xShapeList, maVisibleArea);
}




/** For now forward the update of the visible area to the general update
    method.
*/
void ChildrenManager::update (const Rectangle& aVisibleArea)
{
    update (mxOriginalShapeList, aVisibleArea);
}




/** Find all shapes among the specified shapes that lie fully or partially inside the
    visible area.  Put those shapes into the cleared cache. The
    corresponding accessible objects will be created on demand.
*/
void ChildrenManager::update (const ::com::sun::star::uno::Reference<
    ::com::sun::star::drawing::XShapes>& xShapeList,
    const Rectangle& aVisibleArea)
{
    ::osl::Guard< ::osl::Mutex> aGuard (::osl::Mutex::getGlobalMutex());

    // Clear the list of children.
    maChildDescriptorList.clear ();

    // Remember the new list and visible area for later use.
    mxOriginalShapeList = xShapeList;
    maVisibleArea = aVisibleArea;

    // Determine all fully and partially visible shapes and add those to the list
    // of managed children.
    uno::Reference<container::XIndexAccess> xIA (mxOriginalShapeList, uno::UNO_QUERY);
    long n = xIA->getCount();
    for (long i=0; i<n; i++)
    {
        uno::Reference<drawing::XShape> xShape (xIA->getByIndex(i), uno::UNO_QUERY);
        Rectangle aBoundingBox (xShape->getPosition().X, xShape->getPosition().Y,
            xShape->getSize().Width, xShape->getSize().Height);
        if (aBoundingBox.IsOver (maVisibleArea))
        {
            // Add shape to list of visible children.
            maChildDescriptorList.push_back (ChildDescriptor (xShape));
        }
    }
}




// This method is experimental.  Use with care.
long int ChildrenManager::getChildIndex (const ::com::sun::star::uno::Reference<
    ::drafts::com::sun::star::accessibility::XAccessible>& xChild) const
    throw (::com::sun::star::uno::RuntimeException)
{
    for (long i=0; i<maChildDescriptorList.size(); i++)
    {
        // Is this euqality comparison valid?
        if (maChildDescriptorList[i].mxAccessibleShape == xChild)
            return i;
    }

    return -1;
}




//=====  AccessibleChildDescriptor  ===========================================

ChildDescriptor::ChildDescriptor (const uno::Reference<
        drawing::XShape>& xShape)
    : mxShape (xShape)
{
    // Empty.
}


} // end of namespace accessibility
