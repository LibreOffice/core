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

#include <svx/ShapeTypeHandler.hxx>
#include <svx/SvxShapeTypes.hxx>
#include <svx/AccessibleShapeInfo.hxx>
#include <com/sun/star/drawing/XShapeDescriptor.hpp>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <svx/dialmgr.hxx>
#include "svx/svdstr.hrc"


using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

namespace accessibility {

// Pointer to the shape type handler singleton.
ShapeTypeHandler* ShapeTypeHandler::instance = NULL;


// Create an empty reference to an accessible object.
AccessibleShape*
    CreateEmptyShapeReference (
        const AccessibleShapeInfo& /*rShapeInfo*/,
        const AccessibleShapeTreeInfo& /*rShapeTreeInfo*/,
        ShapeTypeId /*nId*/)
{
    return NULL;
}




ShapeTypeHandler& ShapeTypeHandler::Instance (void)
{
    // Using double check pattern to make sure that exactly one instance of
    // the shape type handler is instantiated.
    if (instance == NULL)
    {
        SolarMutexGuard aGuard;
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
        OUString(RTL_CONSTASCII_USTRINGPARAM("UNKNOWN_SHAPE_TYPE"));
    maShapeTypeDescriptorList[0].maCreateFunction = CreateEmptyShapeReference;
    maServiceNameToSlotId[maShapeTypeDescriptorList[0].msServiceName] = 0;
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




bool ShapeTypeHandler::AddShapeTypeList (int nDescriptorCount,
    ShapeTypeDescriptor aDescriptorList[])
{
    SolarMutexGuard aGuard;

    // Determine first id of new type descriptor(s).
    int nFirstId = maShapeTypeDescriptorList.size();

    // Resize the list, if necessary, so that the types can be inserted.
    maShapeTypeDescriptorList.resize (nFirstId + nDescriptorCount);

    for (int i=0; i<nDescriptorCount; i++)
    {
    #if OSL_DEBUG_LEVEL > 0
        ShapeTypeId nId (aDescriptorList[i].mnShapeTypeId);
        (void)nId;
    #endif

        // Fill Type descriptor.
        maShapeTypeDescriptorList[nFirstId+i].mnShapeTypeId = aDescriptorList[i].mnShapeTypeId;
        maShapeTypeDescriptorList[nFirstId+i].msServiceName = aDescriptorList[i].msServiceName;
        maShapeTypeDescriptorList[nFirstId+i].maCreateFunction = aDescriptorList[i].maCreateFunction;

        // Update inverse mapping from service name to the descriptor's position.
        maServiceNameToSlotId[aDescriptorList[i].msServiceName] = nFirstId+i;
    }

    return true;
}




#include <tools/string.hxx>
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

/// get the accessible base name for an object
::rtl::OUString
    ShapeTypeHandler::CreateAccessibleBaseName (const uno::Reference<drawing::XShape>& rxShape)
    throw (::com::sun::star::uno::RuntimeException)
{
    sal_Int32 nResourceId;
    OUString sName;

    switch (ShapeTypeHandler::Instance().GetTypeId (rxShape))
    {
      // case DRAWING_3D_POLYGON: was removed in original code in
      // AccessibleShape::CreateAccessibleBaseName.  See issue 11190 for details.
      // Id can be removed from SvxShapeTypes.hxx as well.
        case DRAWING_3D_CUBE:
            nResourceId = STR_ObjNameSingulCube3d;
            break;
        case DRAWING_3D_EXTRUDE:
            nResourceId = STR_ObjNameSingulExtrude3d;
            break;
        case DRAWING_3D_LATHE:
            nResourceId = STR_ObjNameSingulLathe3d;
            break;
        case DRAWING_3D_SCENE:
            nResourceId = STR_ObjNameSingulScene3d;
            break;
        case DRAWING_3D_SPHERE:
            nResourceId = STR_ObjNameSingulSphere3d;
            break;
        case DRAWING_CAPTION:
            nResourceId = STR_ObjNameSingulCAPTION;
            break;
        case DRAWING_CLOSED_BEZIER:
            nResourceId = STR_ObjNameSingulPATHFILL;
            break;
        case DRAWING_CLOSED_FREEHAND:
            nResourceId = STR_ObjNameSingulFREEFILL;
            break;
        case DRAWING_CONNECTOR:
            nResourceId = STR_ObjNameSingulEDGE;
            break;
        case DRAWING_CONTROL:
            nResourceId = STR_ObjNameSingulUno;
            break;
        case DRAWING_ELLIPSE:
            nResourceId = STR_ObjNameSingulCIRCE;
            break;
        case DRAWING_GROUP:
            nResourceId = STR_ObjNameSingulGRUP;
            break;
        case DRAWING_LINE:
            nResourceId = STR_ObjNameSingulLINE;
            break;
        case DRAWING_MEASURE:
            nResourceId = STR_ObjNameSingulMEASURE;
            break;
        case DRAWING_OPEN_BEZIER:
            nResourceId = STR_ObjNameSingulPATHLINE;
            break;
        case DRAWING_OPEN_FREEHAND:
            nResourceId = STR_ObjNameSingulFREELINE;
            break;
        case DRAWING_PAGE:
            nResourceId = STR_ObjNameSingulPAGE;
            break;
        case DRAWING_POLY_LINE:
            nResourceId = STR_ObjNameSingulPLIN;
            break;
        case DRAWING_POLY_LINE_PATH:
            nResourceId = STR_ObjNameSingulPLIN;
            break;
        case DRAWING_POLY_POLYGON:
            nResourceId = STR_ObjNameSingulPOLY;
            break;
        case DRAWING_POLY_POLYGON_PATH:
            nResourceId = STR_ObjNameSingulPOLY;
            break;
        case DRAWING_RECTANGLE:
            nResourceId = STR_ObjNameSingulRECT;
            break;
        case DRAWING_TEXT:
            nResourceId = STR_ObjNameSingulTEXT;
            break;
        default:
            nResourceId = -1;
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("UnknownAccessibleShape"));
            uno::Reference<drawing::XShapeDescriptor> xDescriptor (rxShape, uno::UNO_QUERY);
            if (xDescriptor.is())
                sName += ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM(": "))
                    + xDescriptor->getShapeType();
            break;
    }

    if (nResourceId != -1)
    {
        SolarMutexGuard aGuard;
        sName = OUString (SVX_RESSTR((unsigned short)nResourceId));
    }

    return sName;
}

} // end of namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
