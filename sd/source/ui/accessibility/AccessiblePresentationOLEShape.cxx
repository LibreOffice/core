/*************************************************************************
 *
 *  $RCSfile: AccessiblePresentationOLEShape.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 17:03:54 $
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
