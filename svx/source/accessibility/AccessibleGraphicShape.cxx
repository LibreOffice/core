/*************************************************************************
 *
 *  $RCSfile: AccessibleGraphicShape.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 16:53:35 $
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



#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_GRAPHIC_SHAPE_HXX
#include "AccessibleGraphicShape.hxx"
#endif

#include "ShapeTypeHandler.hxx"
#include "SvxShapeTypes.hxx"

using namespace ::accessibility;
using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

//=====  internal  ============================================================

AccessibleGraphicShape::AccessibleGraphicShape (
    const AccessibleShapeInfo& rShapeInfo,
    const AccessibleShapeTreeInfo& rShapeTreeInfo)
    : AccessibleShape (rShapeInfo, rShapeTreeInfo)
{
}




AccessibleGraphicShape::~AccessibleGraphicShape (void)
{
}




//=====  XAccessibleImage  ====================================================

::rtl::OUString SAL_CALL AccessibleGraphicShape::getAccessibleImageDescription (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return AccessibleShape::getAccessibleDescription ();
}




sal_Int32 SAL_CALL AccessibleGraphicShape::getAccessibleImageHeight (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return AccessibleShape::getSize().Height;
}




sal_Int32 SAL_CALL AccessibleGraphicShape::getAccessibleImageWidth (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return AccessibleShape::getSize().Width;
}




//=====  XInterface  ==========================================================

com::sun::star::uno::Any SAL_CALL
    AccessibleGraphicShape::queryInterface (const com::sun::star::uno::Type & rType)
    throw (::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aReturn = AccessibleShape::queryInterface (rType);
    if ( ! aReturn.hasValue())
        aReturn = ::cppu::queryInterface (rType,
            static_cast<XAccessibleImage*>(this));
    return aReturn;
}



void SAL_CALL
    AccessibleGraphicShape::acquire (void)
    throw ()
{
    AccessibleShape::acquire ();
}



void SAL_CALL
    AccessibleGraphicShape::release (void)
    throw ()
{
    AccessibleShape::release ();
}




//=====  XServiceInfo  ========================================================

::rtl::OUString SAL_CALL
    AccessibleGraphicShape::getImplementationName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("AccessibleGraphicShape"));
}




::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL
    AccessibleGraphicShape::getSupportedServiceNames (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();
    // Get list of supported service names from base class...
    uno::Sequence<OUString> aServiceNames =
        AccessibleShape::getSupportedServiceNames();
    sal_Int32 nCount (aServiceNames.getLength());

    // ...and add additional names.
    aServiceNames.realloc (nCount + 1);
    static const OUString sAdditionalServiceName (RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.drawing.AccessibleGraphicShape"));
    aServiceNames[nCount] = sAdditionalServiceName;

    return aServiceNames;
}




//=====  XTypeProvider  ===================================================

uno::Sequence<uno::Type> SAL_CALL
    AccessibleGraphicShape::getTypes (void)
    throw (uno::RuntimeException)
{
    // Get list of types from the context base implementation...
    uno::Sequence<uno::Type> aTypeList (AccessibleShape::getTypes());
    // ...and add the additional type for the component.
    long nTypeCount = aTypeList.getLength();
    aTypeList.realloc (nTypeCount + 1);
    const uno::Type aImageType =
        ::getCppuType((const uno::Reference<XAccessibleImage>*)0);
    aTypeList[nTypeCount] = aImageType;

    return aTypeList;
}




/// Create the base name of this object, i.e. the name without appended number.
::rtl::OUString
    AccessibleGraphicShape::CreateAccessibleBaseName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ::rtl::OUString sName;

    ShapeTypeId nShapeType = ShapeTypeHandler::Instance().GetTypeId (mxShape);
    switch (nShapeType)
    {
        case DRAWING_GRAPHIC_OBJECT:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("GraphicObjectShape"));
            break;

        default:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("UnknownAccessibleGraphicShape"));
            uno::Reference<drawing::XShapeDescriptor> xDescriptor (mxShape, uno::UNO_QUERY);
            if (xDescriptor.is())
                sName += ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM(": "))
                    + xDescriptor->getShapeType();
    }

    return sName;
}



::rtl::OUString
    AccessibleGraphicShape::CreateAccessibleDescription (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return CreateAccessibleName ();
}
