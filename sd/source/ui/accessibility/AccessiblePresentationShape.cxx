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
    return ::rtl::OUString("AccessiblePresentationShape");
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
            sName = "ImpressTitle";
            break;
        case PRESENTATION_OUTLINER:
            sName = "ImpressOutliner";
            break;
        case PRESENTATION_SUBTITLE:
            sName = "ImpressSubtitle";
            break;
        case PRESENTATION_PAGE:
            sName = "ImpressPage";
            break;
        case PRESENTATION_NOTES:
            sName = "ImpressNotes";
            break;
        case PRESENTATION_HANDOUT:
            sName = "ImpressHandout";
            break;
        case PRESENTATION_HEADER:
            sName = "ImpressHeader";
            break;
        case PRESENTATION_FOOTER:
            sName = "ImpressFooter";
            break;
        case PRESENTATION_DATETIME:
            sName = "ImpressDateAndTime";
            break;
        case PRESENTATION_PAGENUMBER:
            sName = "ImpressPageNumber";
            break;
        default:
            sName = "UnknownAccessibleImpressShape";
            uno::Reference<drawing::XShapeDescriptor> xDescriptor (mxShape, uno::UNO_QUERY);
            if (xDescriptor.is())
                sName += ": " + xDescriptor->getShapeType();
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
            aDG.Initialize ("PresentationTitleShape");
            break;
        case PRESENTATION_OUTLINER:
            aDG.Initialize ("PresentationOutlinerShape");
            break;
        case PRESENTATION_SUBTITLE:
            aDG.Initialize ("PresentationSubtitleShape");
            break;
        case PRESENTATION_PAGE:
            aDG.Initialize ("PresentationPageShape");
            break;
        case PRESENTATION_NOTES:
            aDG.Initialize ("PresentationNotesShape");
            break;
        case PRESENTATION_HANDOUT:
            aDG.Initialize ("PresentationHandoutShape");
            break;
        case PRESENTATION_HEADER:
            aDG.Initialize ("PresentationHeaderShape");
            break;
        case PRESENTATION_FOOTER:
            aDG.Initialize ("PresentationFooterShape");
            break;
        case PRESENTATION_DATETIME:
            aDG.Initialize ("PresentationDateAndTimeShape");
            break;
        case PRESENTATION_PAGENUMBER:
            aDG.Initialize ("PresentationPageNumberShape");
            break;
        default:
            aDG.Initialize ("Unknown accessible presentation shape");
            uno::Reference<drawing::XShapeDescriptor> xDescriptor (mxShape, uno::UNO_QUERY);
            if (xDescriptor.is())
            {
                aDG.AppendString ("service name=");
                aDG.AppendString (xDescriptor->getShapeType());
            }
    }

    return aDG();
}

} // end of namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
