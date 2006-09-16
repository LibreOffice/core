/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AccessiblePresentationOLEShape.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 18:25:35 $
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
#include "precompiled_sd.hxx"

#ifndef _SD_ACCESSIBILITY_ACCESSIBLE_PRESENTATION_OLE_SHAPE_HXX
#include "AccessiblePresentationOLEShape.hxx"
#endif

#include "SdShapeTypes.hxx"

#include <svx/DescriptionGenerator.hxx>
#ifndef _RTL_USTRING_H_
#include <rtl/ustring.h>
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

namespace accessibility {

//=====  internal  ============================================================

AccessiblePresentationOLEShape::AccessiblePresentationOLEShape (
    const AccessibleShapeInfo& rShapeInfo,
    const AccessibleShapeTreeInfo& rShapeTreeInfo)
    :   AccessibleOLEShape (rShapeInfo, rShapeTreeInfo)
{
}




AccessiblePresentationOLEShape::~AccessiblePresentationOLEShape (void)
{
}




//=====  XServiceInfo  ========================================================

::rtl::OUString SAL_CALL
    AccessiblePresentationOLEShape::getImplementationName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AccessiblePresentationOLEShape"));
}




/// Set this object's name if it is different to the current name.
::rtl::OUString
    AccessiblePresentationOLEShape::CreateAccessibleBaseName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ::rtl::OUString sName;

    ShapeTypeId nShapeType = ShapeTypeHandler::Instance().GetTypeId (mxShape);
    switch (nShapeType)
    {
        case PRESENTATION_OLE:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("ImpressOLE"));
            break;
        case PRESENTATION_CHART:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("ImpressChart"));
            break;
        case PRESENTATION_TABLE:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("ImpressTable"));
            break;
        default:
            sName = ::rtl::OUString (
                RTL_CONSTASCII_USTRINGPARAM("UnknownAccessibleImpressOLEShape"));
            uno::Reference<drawing::XShapeDescriptor> xDescriptor (mxShape, uno::UNO_QUERY);
            if (xDescriptor.is())
                sName += ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM(": "))
                    + xDescriptor->getShapeType();
    }

    return sName;
}




::rtl::OUString
    AccessiblePresentationOLEShape::CreateAccessibleDescription (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    //    return createAccessibleName();
    DescriptionGenerator aDG (mxShape);
    ShapeTypeId nShapeType = ShapeTypeHandler::Instance().GetTypeId (mxShape);
    switch (nShapeType)
    {
        case PRESENTATION_OLE:
            aDG.Initialize (::rtl::OUString::createFromAscii ("PresentationOLEShape"));
            //SVX_RESSTR(RID_SVXSTR_A11Y_ST_RECTANGLE));
            aDG.AddProperty (OUString::createFromAscii ("CLSID"),
                DescriptionGenerator::STRING);
            break;
        case PRESENTATION_CHART:
            aDG.Initialize (::rtl::OUString::createFromAscii ("PresentationChartShape"));
            //SVX_RESSTR(RID_SVXSTR_A11Y_ST_RECTANGLE));
            aDG.AddProperty (OUString::createFromAscii ("CLSID"),
                DescriptionGenerator::STRING);
            break;
        case PRESENTATION_TABLE:
            aDG.Initialize (::rtl::OUString::createFromAscii ("PresentationTableShape"));
            //SVX_RESSTR(RID_SVXSTR_A11Y_ST_RECTANGLE));
            aDG.AddProperty (OUString::createFromAscii ("CLSID"),
                DescriptionGenerator::STRING);
            break;
        default:
            aDG.Initialize (::rtl::OUString::createFromAscii ("Unknown accessible presentation OLE shape"));
            uno::Reference<drawing::XShapeDescriptor> xDescriptor (mxShape, uno::UNO_QUERY);
            if (xDescriptor.is())
            {
                aDG.AppendString (::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("service name=")));
                aDG.AppendString (xDescriptor->getShapeType());
            }
    }

    return aDG();
}

} // end of namespace accessibility
