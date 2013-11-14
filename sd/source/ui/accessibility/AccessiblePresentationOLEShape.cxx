/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"
#include "AccessiblePresentationOLEShape.hxx"

#include "SdShapeTypes.hxx"

#include <svx/DescriptionGenerator.hxx>
#include <rtl/ustring.h>

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
//IAccessibility2 Implementation 2009-----
//  Return this object's role.
sal_Int16 SAL_CALL AccessiblePresentationOLEShape::getAccessibleRole ()
    throw (::com::sun::star::uno::RuntimeException)
{

    return  AccessibleRole::EMBEDDED_OBJECT ;
}
//-----IAccessibility2 Implementation 2009
} // end of namespace accessibility
