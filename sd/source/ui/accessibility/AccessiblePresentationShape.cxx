/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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

OUString SAL_CALL
    AccessiblePresentationShape::getImplementationName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return OUString("AccessiblePresentationShape");
}




/// Set this object's name if is different to the current name.
OUString
    AccessiblePresentationShape::CreateAccessibleBaseName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    OUString sName;

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




OUString
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
