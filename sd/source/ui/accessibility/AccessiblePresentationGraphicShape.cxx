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
#include "AccessiblePresentationGraphicShape.hxx"

#include "SdShapeTypes.hxx"

#include <svx/DescriptionGenerator.hxx>
#include <rtl/ustring.h>

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

namespace accessibility {

//=====  internal  ============================================================

AccessiblePresentationGraphicShape::AccessiblePresentationGraphicShape (
    const AccessibleShapeInfo& rShapeInfo,
    const AccessibleShapeTreeInfo& rShapeTreeInfo)
    : AccessibleGraphicShape (rShapeInfo, rShapeTreeInfo)
{
}




AccessiblePresentationGraphicShape::~AccessiblePresentationGraphicShape (void)
{
}




//=====  XServiceInfo  ========================================================

::rtl::OUString SAL_CALL
    AccessiblePresentationGraphicShape::getImplementationName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AccessiblePresentationGraphicShape"));
}




/// Set this object's name if is different to the current name.
::rtl::OUString
    AccessiblePresentationGraphicShape::CreateAccessibleBaseName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ::rtl::OUString sName;

    ShapeTypeId nShapeType = ShapeTypeHandler::Instance().GetTypeId (mxShape);
    switch (nShapeType)
    {
        case PRESENTATION_GRAPHIC_OBJECT:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("ImpressGraphicObject"));
            break;
        default:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("UnknownAccessibleImpressShape"));
            uno::Reference<drawing::XShapeDescriptor> xDescriptor (mxShape, uno::UNO_QUERY);
            if (xDescriptor.is())
                sName += ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM(": "))
                    + xDescriptor->getShapeType();
    }

    return sName;
}




::rtl::OUString
    AccessiblePresentationGraphicShape::CreateAccessibleDescription (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    //    return createAccessibleName ();
    DescriptionGenerator aDG (mxShape);
    ShapeTypeId nShapeType = ShapeTypeHandler::Instance().GetTypeId (mxShape);
    switch (nShapeType)
    {
        case PRESENTATION_GRAPHIC_OBJECT:
            aDG.Initialize (::rtl::OUString::createFromAscii ("PresentationGraphicShape"));
            break;
        default:
            aDG.Initialize (
                ::rtl::OUString::createFromAscii ("Unknown accessible presentation graphic shape"));
            uno::Reference<drawing::XShapeDescriptor> xDescriptor (mxShape, uno::UNO_QUERY);
            if (xDescriptor.is())
            {
                aDG.AppendString (::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("service name=")));
                aDG.AppendString (xDescriptor->getShapeType());
            }
    }

    return aDG();
}
sal_Int16 SAL_CALL AccessiblePresentationGraphicShape::getAccessibleRole ()
    throw (::com::sun::star::uno::RuntimeException)
{

    return  AccessibleRole::GRAPHIC ;
}
} // end of namespace accessibility
