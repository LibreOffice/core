/*************************************************************************
 *
 *  $RCSfile: ShapeTypeHandler.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 15:00:31 $
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

#include "ShapeTypeHandler.hxx"
#include "SvxShapeTypes.hxx"

#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_SHAPE_INFO_HXX
#include "AccessibleShapeInfo.hxx"
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPEDESCRIPTOR_HPP_
#include <com/sun/star/drawing/XShapeDescriptor.hpp>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif


using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::accessibility;

namespace accessibility {

// Pointer to the shape type handler singleton.
ShapeTypeHandler* ShapeTypeHandler::instance = NULL;


// Create an empty reference to an accessible object.
AccessibleShape*
    CreateEmptyShapeReference (
        const AccessibleShapeInfo& rShapeInfo,
        const AccessibleShapeTreeInfo& rShapeTreeInfo,
        ShapeTypeId nId)
{
    return NULL;
}




ShapeTypeHandler& ShapeTypeHandler::Instance (void)
{
    // Using double check pattern to make sure that exactly one instance of
    // the shape type handler is instantiated.
    if (instance == NULL)
    {
        ::vos::OGuard aGuard (::Application::GetSolarMutex());
        if (instance == NULL)
        {
            // Create the single instance of the shape type handler.
            instance = new ShapeTypeHandler;

            // Register the basic SVX shape types.
            RegisterDrawShapeTypes ();
        }
    }

    return *instance;
}




/** The given service name is first transformed into a slot id that
    identifies the place of the type descriptor.  From that descriptor the
    shape type id is returned.
*/
ShapeTypeId ShapeTypeHandler::GetTypeId (const OUString& aServiceName) const
{
    tServiceNameToSlotId::iterator I (maServiceNameToSlotId.find (aServiceName));
    if (I != maServiceNameToSlotId.end())
    {
        //  long nSlotId = maServiceNameToSlotId[aServiceName];
        return maShapeTypeDescriptorList[I->second].mnShapeTypeId;
    }
    else
        return -1;
}



/** Extract the specified shape's service name and forward the request to
    the appropriate method.
*/
ShapeTypeId ShapeTypeHandler::GetTypeId (const uno::Reference<drawing::XShape>& rxShape) const
{
    uno::Reference<drawing::XShapeDescriptor> xDescriptor (rxShape, uno::UNO_QUERY);
    if (xDescriptor.is())
        return GetTypeId (xDescriptor->getShapeType());
    else
        return -1;
}




const OUString& ShapeTypeHandler::GetServiceName (ShapeTypeId aTypeId) const
{
    return maShapeTypeDescriptorList[aTypeId].msServiceName;
}




/** This factory method determines the type descriptor for the type of the
    given shape, then calls the descriptor's create function, and finally
    initializes the new object.
*/
AccessibleShape*
    ShapeTypeHandler::CreateAccessibleObject (
        const AccessibleShapeInfo& rShapeInfo,
        const AccessibleShapeTreeInfo& rShapeTreeInfo) const
{
    ShapeTypeId nSlotId (GetSlotId (rShapeInfo.mxShape));
    AccessibleShape* pShape =
        maShapeTypeDescriptorList[nSlotId].maCreateFunction (
            rShapeInfo,
            rShapeTreeInfo,
            maShapeTypeDescriptorList[nSlotId].mnShapeTypeId);
    return pShape;
}




/** Create the single instance of this class and initialize its list of
    type descriptors with an entry of an unknown type.
*/
ShapeTypeHandler::ShapeTypeHandler (void)
    : maShapeTypeDescriptorList (1)
{
    // Make sure that at least the UNKNOWN entry is present.
    // Resize the list, if necessary, so that the new type can be inserted.
    maShapeTypeDescriptorList[0].mnShapeTypeId = UNKNOWN_SHAPE_TYPE;
    maShapeTypeDescriptorList[0].msServiceName =
        OUString::createFromAscii ("UNKNOWN_SHAPE_TYPE");
    maShapeTypeDescriptorList[0].maCreateFunction = CreateEmptyShapeReference;
    maServiceNameToSlotId[maShapeTypeDescriptorList[0].msServiceName] = 0;
}




ShapeTypeHandler::ShapeTypeHandler (const ShapeTypeHandler& aHandler)
{
    // Don't call this constructor.  This class is a singleton.
    OSL_ENSURE (sal_False, "Wrong (copy-) constructor of singleton ShapeTypeHandler called."
        "  Don't do that again.");
}




ShapeTypeHandler::~ShapeTypeHandler (void)
{
    //  Because this class is a singleton and the only instance, whose
    //  destructor has just been called, is pointed to from instance,
    //  we reset the static variable instance, so that further calls to
    //  getInstance do not return an undefined object but create a new
    //  singleton.
    instance = NULL;
}




ShapeTypeHandler& ShapeTypeHandler::operator= (const ShapeTypeHandler& aHandler)
{
    // Don't call this operator.  This class is a singleton.
    OSL_ENSURE (sal_False, "Assignment operator of singleton ShapeTypeHandler called."
        "  Don't do that again.");
    return *this;
}




bool ShapeTypeHandler::AddShapeTypeList (int nDescriptorCount,
    ShapeTypeDescriptor aDescriptorList[])
{
    ::vos::OGuard aGuard (::Application::GetSolarMutex());

    // Determine first id of new type descriptor(s).
    int nFirstId = maShapeTypeDescriptorList.size();

    // Resize the list, if necessary, so that the types can be inserted.
    maShapeTypeDescriptorList.resize (nFirstId + nDescriptorCount);

    for (int i=0; i<nDescriptorCount; i++)
    {
        ShapeTypeId nId (aDescriptorList[i].mnShapeTypeId);

        // Fill Type descriptor.
        maShapeTypeDescriptorList[nFirstId+i].mnShapeTypeId = aDescriptorList[i].mnShapeTypeId;
        maShapeTypeDescriptorList[nFirstId+i].msServiceName = aDescriptorList[i].msServiceName;
        maShapeTypeDescriptorList[nFirstId+i].maCreateFunction = aDescriptorList[i].maCreateFunction;

        // Update inverse mapping from service name to the descriptor's position.
        maServiceNameToSlotId[aDescriptorList[i].msServiceName] = nFirstId+i;
    }

    return true;
}




#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
long ShapeTypeHandler::GetSlotId (const OUString& aServiceName) const
{
    tServiceNameToSlotId::iterator I (maServiceNameToSlotId.find (aServiceName));
    if (I != maServiceNameToSlotId.end())
        return I->second;
    else
        return 0;
}




// Extract the given shape's service name and forward request to appropriate
// method.
long ShapeTypeHandler::GetSlotId (const uno::Reference<drawing::XShape>& rxShape) const
{
    uno::Reference<drawing::XShapeDescriptor> xDescriptor (rxShape, uno::UNO_QUERY);
    if (xDescriptor.is())
        return GetSlotId (xDescriptor->getShapeType());
    else
        return 0;
}

} // end of namespace accessibility
