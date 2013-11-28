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

#include "accessibility.hrc"
#include "sdresid.hxx"
#include <tools/string.hxx>
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
            //sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("ImpressTitle"));
            sName = ::rtl::OUString (String(SdResId(SID_SD_A11Y_P_TITLE_N)));
            break;
        case PRESENTATION_OUTLINER:
            //sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("ImpressOutliner"));
            sName = ::rtl::OUString (String(SdResId(SID_SD_A11Y_P_OUTLINER_N)));
            break;
        case PRESENTATION_SUBTITLE:
            //sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("ImpressSubtitle"));
            sName = ::rtl::OUString (String(SdResId(SID_SD_A11Y_P_SUBTITLE_N)));
            break;
        case PRESENTATION_PAGE:
            //sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("ImpressPage"));
            sName = ::rtl::OUString (String(SdResId(SID_SD_A11Y_P_PAGE_N)));
            break;
        case PRESENTATION_NOTES:
            //sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("ImpressNotes"));
            sName = ::rtl::OUString (String(SdResId(SID_SD_A11Y_P_NOTES_N)));
            break;
        case PRESENTATION_HANDOUT:
            //sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("ImpressHandout"));
            sName = ::rtl::OUString (String(SdResId(SID_SD_A11Y_P_HANDOUT_N)));
            break;
        case PRESENTATION_HEADER:
            //sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("ImpressHeader"));
        sName = ::rtl::OUString ( String(SdResId(SID_SD_A11Y_P_HEADER_N)) );
            break;
        case PRESENTATION_FOOTER:
            //sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("ImpressFooter"));
        sName = ::rtl::OUString ( String(SdResId(SID_SD_A11Y_P_FOOTER_N)) );
            break;
        case PRESENTATION_DATETIME:
            //sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("ImpressDateAndTime"));
        sName = ::rtl::OUString ( String(SdResId(SID_SD_A11Y_P_DATE_N)) );
            break;
        case PRESENTATION_PAGENUMBER:
            //sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("ImpressPageNumber"));
        sName = ::rtl::OUString ( String(SdResId(SID_SD_A11Y_P_NUMBER_N)) );
            break;
        default:
            //sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("UnknownAccessibleImpressShape"));
            sName = ::rtl::OUString (String(SdResId(SID_SD_A11Y_P_UNKNOWN_N)));
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
    ::rtl::OUString sDescription;
    DescriptionGenerator aDG (mxShape);
    ShapeTypeId nShapeType = ShapeTypeHandler::Instance().GetTypeId (mxShape);
    switch (nShapeType)
    {
        case PRESENTATION_TITLE:
            //aDG.Initialize (::rtl::OUString::createFromAscii ("PresentationTitleShape"));
        sDescription = ::rtl::OUString ( String(SdResId(SID_SD_A11Y_P_TITLE_D)) );
        aDG.Initialize (sDescription);
            break;
        case PRESENTATION_OUTLINER:
            //aDG.Initialize (::rtl::OUString::createFromAscii ("PresentationOutlinerShape"));
        sDescription = ::rtl::OUString ( String(SdResId(SID_SD_A11Y_P_OUTLINER_D)) );
        aDG.Initialize (sDescription);              //PresentationOutlinerShape
            break;
        case PRESENTATION_SUBTITLE:
            aDG.Initialize (::rtl::OUString::createFromAscii ("PresentationSubtitleShape"));
         sDescription = ::rtl::OUString ( String(SdResId(SID_SD_A11Y_P_SUBTITLE_D)) );
        aDG.Initialize (sDescription);              //PresentationSubtitleShape
            break;
        case PRESENTATION_PAGE:
            aDG.Initialize (::rtl::OUString::createFromAscii ("PresentationPageShape"));
            sDescription = ::rtl::OUString ( String(SdResId(SID_SD_A11Y_P_PAGE_D)) );
        aDG.Initialize (sDescription);              //PresentationPageShape
            break;
        case PRESENTATION_NOTES:
            aDG.Initialize (::rtl::OUString::createFromAscii ("PresentationNotesShape"));
        sDescription = ::rtl::OUString ( String(SdResId(SID_SD_A11Y_P_NOTES_D)) );
        aDG.Initialize (sDescription);              //PresentationNotesShape
            break;
        case PRESENTATION_HANDOUT:
            aDG.Initialize (::rtl::OUString::createFromAscii ("PresentationHandoutShape"));
        sDescription = ::rtl::OUString ( String(SdResId(SID_SD_A11Y_P_HANDOUT_D)) );
        aDG.Initialize (sDescription);              //PresentationHandoutShape
            break;
        case PRESENTATION_HEADER:
            //aDG.Initialize (::rtl::OUString::createFromAscii ("PresentationHeaderShape"));
        sDescription = ::rtl::OUString ( String(SdResId(SID_SD_A11Y_P_HEADER_D)) );
        aDG.Initialize (sDescription);              //PresentationHeaderShape
            break;
        case PRESENTATION_FOOTER:
            //aDG.Initialize (::rtl::OUString::createFromAscii ("PresentationFooterShape"));
        sDescription = ::rtl::OUString ( String(SdResId(SID_SD_A11Y_P_FOOTER_D)) );
        aDG.Initialize (sDescription);              //PresentationFooterShape
            break;
        case PRESENTATION_DATETIME:
            //aDG.Initialize (::rtl::OUString::createFromAscii ("PresentationDateAndTimeShape"));
        sDescription = ::rtl::OUString ( String(SdResId(SID_SD_A11Y_P_DATE_D)) );
        aDG.Initialize (sDescription);              //PresentationDateShape
            break;
        case PRESENTATION_PAGENUMBER:
            //aDG.Initialize (::rtl::OUString::createFromAscii ("PresentationPageNumberShape"));
        sDescription = ::rtl::OUString ( String(SdResId(SID_SD_A11Y_P_NUMBER_D)) );
        aDG.Initialize (sDescription);              //PresentationNumberShape
            break;
        default:
            //aDG.Initialize (::rtl::OUString::createFromAscii ("Unknown accessible presentation shape"));
        sDescription = ::rtl::OUString ( String(SdResId(SID_SD_A11Y_P_UNKNOWN_D)) );
        aDG.Initialize (sDescription);              //Unknown accessible presentation shape
            uno::Reference<drawing::XShapeDescriptor> xDescriptor (mxShape, uno::UNO_QUERY);
            if (xDescriptor.is())
            {
                aDG.AppendString (::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("service name=")));
                aDG.AppendString (xDescriptor->getShapeType());
            }
    }

    return aDG();
}
::rtl::OUString AccessiblePresentationShape::GetStyle()
{
    ::rtl::OUString sName;

    ShapeTypeId nShapeType = ShapeTypeHandler::Instance().GetTypeId (mxShape);
    switch (nShapeType)
    {
        case PRESENTATION_TITLE:
            sName = ::rtl::OUString (String(SdResId(SID_SD_A11Y_P_TITLE_N_STYLE)));
            break;
        case PRESENTATION_OUTLINER:
            sName = ::rtl::OUString (String(SdResId(SID_SD_A11Y_P_OUTLINER_N_STYLE)));
            break;
        case PRESENTATION_SUBTITLE:
            sName = ::rtl::OUString (String(SdResId(SID_SD_A11Y_P_SUBTITLE_N_STYLE)));
            break;
        case PRESENTATION_PAGE:
            sName = ::rtl::OUString (String(SdResId(SID_SD_A11Y_P_PAGE_N_STYLE)));
            break;
        case PRESENTATION_NOTES:
            sName = ::rtl::OUString (String(SdResId(SID_SD_A11Y_P_NOTES_N_STYLE)));
            break;
        case PRESENTATION_HANDOUT:
            sName = ::rtl::OUString (String(SdResId(SID_SD_A11Y_P_HANDOUT_N_STYLE)));
            break;
        case PRESENTATION_FOOTER:
        sName = ::rtl::OUString ( String(SdResId(SID_SD_A11Y_P_FOOTER_N_STYLE)) );
            break;
    case PRESENTATION_HEADER:
        sName = ::rtl::OUString ( String(SdResId(SID_SD_A11Y_P_HEADER_N_STYLE)) );
            break;
        case PRESENTATION_DATETIME:
            sName = ::rtl::OUString ( String(SdResId(SID_SD_A11Y_P_DATE_N_STYLE)) );
            break;
        case PRESENTATION_PAGENUMBER:
            sName = ::rtl::OUString ( String(SdResId(SID_SD_A11Y_P_NUMBER_N_STYLE)) );
            break;
        default:
            sName = ::rtl::OUString (String(SdResId(SID_SD_A11Y_P_UNKNOWN_N_STYLE)));
            uno::Reference<drawing::XShapeDescriptor> xDescriptor (mxShape, uno::UNO_QUERY);
            if (xDescriptor.is())
                sName += ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM(": "))
                    + xDescriptor->getShapeType();
    }

    return sName;

}
} // end of namespace accessibility
