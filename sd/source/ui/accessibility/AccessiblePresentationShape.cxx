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
#include "AccessiblePresentationShape.hxx"

#include "SdShapeTypes.hxx"

#include <svx/DescriptionGenerator.hxx>
#include <rtl/ustring.h>

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

namespace accessibility {

//=====  internal  ============================================================

AccessiblePresentationShape::AccessiblePresentationShape (
    const AccessibleShapeInfo& rShapeInfo,
    const AccessibleShapeTreeInfo& rShapeTreeInfo)
    : AccessibleShape (rShapeInfo, rShapeTreeInfo)
{
}




AccessiblePresentationShape::~AccessiblePresentationShape (void)
{
}




//=====  XServiceInfo  ========================================================

::rtl::OUString SAL_CALL
    AccessiblePresentationShape::getImplementationName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AccessiblePresentationShape"));
}




/// Set this object's name if is different to the current name.
::rtl::OUString
    AccessiblePresentationShape::CreateAccessibleBaseName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ::rtl::OUString sName;

    ShapeTypeId nShapeType = ShapeTypeHandler::Instance().GetTypeId (mxShape);
    switch (nShapeType)
    {
        case PRESENTATION_TITLE:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("ImpressTitle"));
            break;
        case PRESENTATION_OUTLINER:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("ImpressOutliner"));
            break;
        case PRESENTATION_SUBTITLE:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("ImpressSubtitle"));
            break;
        case PRESENTATION_PAGE:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("ImpressPage"));
            break;
        case PRESENTATION_NOTES:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("ImpressNotes"));
            break;
        case PRESENTATION_HANDOUT:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("ImpressHandout"));
            break;
        case PRESENTATION_HEADER:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("ImpressHeader"));
            break;
        case PRESENTATION_FOOTER:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("ImpressFooter"));
            break;
        case PRESENTATION_DATETIME:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("ImpressDateAndTime"));
            break;
        case PRESENTATION_PAGENUMBER:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("ImpressPageNumber"));
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
    AccessiblePresentationShape::CreateAccessibleDescription (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    //    return createAccessibleName ();
    DescriptionGenerator aDG (mxShape);
    ShapeTypeId nShapeType = ShapeTypeHandler::Instance().GetTypeId (mxShape);
    switch (nShapeType)
    {
        case PRESENTATION_TITLE:
            aDG.Initialize (::rtl::OUString::createFromAscii ("PresentationTitleShape"));
            break;
        case PRESENTATION_OUTLINER:
            aDG.Initialize (::rtl::OUString::createFromAscii ("PresentationOutlinerShape"));
            break;
        case PRESENTATION_SUBTITLE:
            aDG.Initialize (::rtl::OUString::createFromAscii ("PresentationSubtitleShape"));
            break;
        case PRESENTATION_PAGE:
            aDG.Initialize (::rtl::OUString::createFromAscii ("PresentationPageShape"));
            break;
        case PRESENTATION_NOTES:
            aDG.Initialize (::rtl::OUString::createFromAscii ("PresentationNotesShape"));
            break;
        case PRESENTATION_HANDOUT:
            aDG.Initialize (::rtl::OUString::createFromAscii ("PresentationHandoutShape"));
            break;
        case PRESENTATION_HEADER:
            aDG.Initialize (::rtl::OUString::createFromAscii ("PresentationHeaderShape"));
            break;
        case PRESENTATION_FOOTER:
            aDG.Initialize (::rtl::OUString::createFromAscii ("PresentationFooterShape"));
            break;
        case PRESENTATION_DATETIME:
            aDG.Initialize (::rtl::OUString::createFromAscii ("PresentationDateAndTimeShape"));
            break;
        case PRESENTATION_PAGENUMBER:
            aDG.Initialize (::rtl::OUString::createFromAscii ("PresentationPageNumberShape"));
            break;
        default:
            aDG.Initialize (::rtl::OUString::createFromAscii ("Unknown accessible presentation shape"));
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
