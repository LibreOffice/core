/*************************************************************************
 *
 *  $RCSfile: AccessibleShape.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2002-02-08 17:56:09 $
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



#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_SHAPE_HXX
#include "AccessibleShape.hxx"
#endif

#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLE_ROLE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPES_HPP_
#include <com/sun/star/drawing/XShapes.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPEDESCRIPTOR_HPP_
#include <com/sun/star/drawing/XShapeDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_INDEXOUTOFBOUNDSEXCEPTION_HPP_
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#endif

#include "svdobj.hxx"
#include "svdmodel.hxx"
#include "unoapi.hxx"
#include <com/sun/star/uno/Exception.hpp>

#include "ShapeTypeHandler.hxx"
#include "SvxShapeTypes.hxx"

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::accessibility;

namespace accessibility {

//=====  internal  ============================================================

AccessibleShape::AccessibleShape (const ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XShape>& rxShape,
        const ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::accessibility::XAccessible>& rxParent)
    :      AccessibleContextBase (rxParent,
        AccessibleRole::SHAPE),
           mxShape (rxShape)
{
    // Create a children manager if this shape has children of its own.
    uno::Reference<drawing::XShapes> xShapes (mxShape, uno::UNO_QUERY);
    if (xShapes.is() && xShapes->getCount() > 0)
    {
        maChildrenManager = new ChildrenManager (this);
        Rectangle aBBox (mxShape->getPosition().X, mxShape->getPosition().Y,
            mxShape->getSize().Width, mxShape->getSize().Height);
        maChildrenManager->update (xShapes, aBBox);
    }
    else
        maChildrenManager = NULL;
}




AccessibleShape::~AccessibleShape (void)
{
    if (maChildrenManager != NULL)
        delete maChildrenManager;
}




//=====  XAccessibleContext  ==================================================

uno::Reference<XAccessible> SAL_CALL
    AccessibleShape::getAccessibleParent (void)
    throw (uno::RuntimeException)
{
    return AccessibleContextBase::getAccessibleParent();
}




/** Return the number of children of the original shape.
*/
sal_Int32 SAL_CALL
       AccessibleShape::getAccessibleChildCount (void)
    throw ()
{
    if (maChildrenManager != NULL)
        return maChildrenManager->getChildCount ();
    else
        return 0;
}




/** Forward the request to the shape.  Return the requested shape or throw
    an exception for a wrong index.
*/
uno::Reference<XAccessible> SAL_CALL
    AccessibleShape::getAccessibleChild (long nIndex)
    throw (::com::sun::star::uno::RuntimeException)
{
    if (maChildrenManager != NULL)
        return maChildrenManager->getChild (nIndex);
    else
        throw lang::IndexOutOfBoundsException (
            ::rtl::OUString::createFromAscii ("shape has no child"),
            NULL);
}




//=====  XAccessibleComponent  ================================================

awt::Point SAL_CALL AccessibleShape::getLocation (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    awt::Point aLocation (mxShape->getPosition ());
    return aLocation;
}




awt::Point SAL_CALL AccessibleShape::getLocationOnScreen (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return getLocation();
}




::com::sun::star::awt::Size SAL_CALL AccessibleShape::getSize (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    awt::Size aSize (mxShape->getSize ());
    return aSize;
}




//=====  XAccessibleExtendedComponent  ========================================

sal_Int32 SAL_CALL AccessibleShape::getForeground (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    sal_Int32 nColor (0x0ffffffL);

    try
    {
        uno::Reference<beans::XPropertySet> aSet (mxShape, uno::UNO_QUERY);
        if (aSet.is())
        {
            uno::Any aColor;
            aColor = aSet->getPropertyValue (OUString::createFromAscii ("LineColor"));
            aColor >>= nColor;
        }
    }
    catch (::com::sun::star::beans::UnknownPropertyException)
    {
        // Ignore exception and return default color.
    }
    return nColor;
}




sal_Int32 SAL_CALL AccessibleShape::getBackground (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    sal_Int32 nColor (0x0ffffffL);

    try
    {
        uno::Reference<beans::XPropertySet> aSet (mxShape, uno::UNO_QUERY);
        if (aSet.is())
        {
            uno::Any aColor;
            aColor = aSet->getPropertyValue (OUString::createFromAscii ("FillColor"));
            aColor >>= nColor;
        }
    }
    catch (::com::sun::star::beans::UnknownPropertyException)
    {
        // Ignore exception and return default color.
    }
    return nColor;
}




//=====  XInterface  ==========================================================

com::sun::star::uno::Any SAL_CALL
    AccessibleShape::queryInterface (const com::sun::star::uno::Type & rType)
    throw (::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aReturn = AccessibleContextBase::queryInterface (rType);
    if ( ! aReturn.hasValue())
        aReturn = ::cppu::queryInterface (rType,
            static_cast<XAccessibleComponent*>(this),
            static_cast<XAccessibleExtendedComponent*>(this));
    return aReturn;
}



void SAL_CALL
    AccessibleShape::acquire (void)
    throw ()
{
    AccessibleContextBase::acquire ();
}



void SAL_CALL
    AccessibleShape::release (void)
    throw ()
{
    AccessibleContextBase::release ();
}




//=====  XServiceInfo  ========================================================

::rtl::OUString SAL_CALL
    AccessibleShape::getImplementationName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("AccessibleShape"));
}




::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL
    AccessibleShape::getSupportedServiceNames (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return AccessibleContextBase::getSupportedServiceNames();
}





//=====  XTypeProvider  ===================================================

uno::Sequence<uno::Type> SAL_CALL
    AccessibleShape::getTypes (void)
    throw (uno::RuntimeException)
{
    // Get list of types from the context base implementation...
    uno::Sequence<uno::Type> aTypeList (AccessibleContextBase::getTypes());
    // ...and add the additional type for the component.
    uno::Sequence<uno::Type> aComponentTypeList (AccessibleComponentBase::getTypes());
    sal_Int32 nTypeCount (aTypeList.getLength()),
        nComponentTypeCount (aComponentTypeList.getLength());
    aTypeList.realloc (nTypeCount + nComponentTypeCount);
    for (int i=0; i<nComponentTypeCount; i++)
    {
        aTypeList[nTypeCount + i] = aComponentTypeList[i];
    }

    return aTypeList;
}




/// Set this object's name if is different to the current name.
::rtl::OUString
    AccessibleShape::createAccessibleName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ::rtl::OUString sName;

    ShapeTypeId nShapeType = ShapeTypeHandler::Instance().getTypeId (mxShape);
    switch (nShapeType)
    {
        case DRAWING_RECTANGLE:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("RectangleShape"));
            break;
        case DRAWING_ELLIPSE:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("EllipseShape"));
            break;
        case DRAWING_CONTROL:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("ConrolShape"));
            break;
        case DRAWING_CONNECTOR:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("ConnectorShape"));
            break;
        case DRAWING_MEASURE:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("MeasureShape"));
            break;
        case DRAWING_LINE:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("LineShape"));
            break;
        case DRAWING_POLY_POLYGON:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("PolyPolygonShape"));
            break;
        case DRAWING_POLY_LINE:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("PolyLineShape"));
            break;
        case DRAWING_OPEN_BEZIER:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("OpenBezierShape"));
            break;
        case DRAWING_CLOSED_BEZIER:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("ClosedBezierShape"));
            break;
        case DRAWING_OPEN_FREEHAND:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("OpenFreehandShape"));
            break;
        case DRAWING_CLOSED_FREEHAND:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("ClosedFreehandShape"));
            break;
        case DRAWING_POLY_POLYGON_PATH:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("PolyPolygonPathShape"));
            break;
        case DRAWING_POLY_LINE_PATH:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("PolyLinePathShape"));
            break;
        case DRAWING_GROUP:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("GroupShape"));
            break;
        case DRAWING_TEXT:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("TextShape"));
            break;
        case DRAWING_PAGE:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("PageShape"));
            break;
        case DRAWING_3D_SCENE:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("3DSceneShape"));
            break;
        case DRAWING_3D_CUBE:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("3DCubeShape"));
            break;
        case DRAWING_3D_SPHERE:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("3DSphereShape"));
            break;
        case DRAWING_3D_LATHE:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("3DLatheShape"));
            break;
        case DRAWING_3D_EXTRUDE:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("3DExtrudeShape"));
            break;
        case DRAWING_3D_POLYGON:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("3DPolygonShape"));
            break;
        default:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("UnknownAccessibleShape"));
            uno::Reference<drawing::XShapeDescriptor> xDescriptor (mxShape, uno::UNO_QUERY);
            if (xDescriptor.is())
                sName += ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM (": "))
                    + xDescriptor->getShapeType();
    }

    return sName;
}



::rtl::OUString
    AccessibleShape::createAccessibleDescription (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return createAccessibleName ();
}


} // end of namespace accessibility
