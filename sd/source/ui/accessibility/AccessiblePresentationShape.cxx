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

#include <AccessiblePresentationShape.hxx>

#include <SdShapeTypes.hxx>

#include <strings.hrc>
#include <sdresid.hxx>
#include <svx/ShapeTypeHandler.hxx>

#include <com/sun/star/drawing/XShape.hpp>

using namespace ::com::sun::star;

namespace accessibility
{
//=====  internal  ============================================================

AccessiblePresentationShape::AccessiblePresentationShape(
    const AccessibleShapeInfo& rShapeInfo, const AccessibleShapeTreeInfo& rShapeTreeInfo)
    : AccessibleShape(rShapeInfo, rShapeTreeInfo)
{
}

AccessiblePresentationShape::~AccessiblePresentationShape() {}

// XServiceInfo

OUString SAL_CALL AccessiblePresentationShape::getImplementationName()
{
    return u"AccessiblePresentationShape"_ustr;
}

/// Set this object's name if is different to the current name.
OUString AccessiblePresentationShape::CreateAccessibleBaseName()
{
    OUString sName;

    ShapeTypeId nShapeType = ShapeTypeHandler::Instance().GetTypeId(mxShape);
    switch (nShapeType)
    {
        case PRESENTATION_TITLE:
            sName = SdResId(SID_SD_A11Y_P_TITLE_N);
            break;
        case PRESENTATION_OUTLINER:
            sName = SdResId(SID_SD_A11Y_P_OUTLINER_N);
            break;
        case PRESENTATION_SUBTITLE:
            sName = SdResId(SID_SD_A11Y_P_SUBTITLE_N);
            break;
        case PRESENTATION_PAGE:
            sName = SdResId(SID_SD_A11Y_P_PAGE_N);
            break;
        case PRESENTATION_NOTES:
            sName = SdResId(SID_SD_A11Y_P_NOTES_N);
            break;
        case PRESENTATION_HANDOUT:
            sName = SdResId(SID_SD_A11Y_P_HANDOUT_N);
            break;
        case PRESENTATION_HEADER:
            sName = SdResId(SID_SD_A11Y_P_HEADER_N);
            break;
        case PRESENTATION_FOOTER:
            sName = SdResId(SID_SD_A11Y_P_FOOTER_N);
            break;
        case PRESENTATION_DATETIME:
            sName = SdResId(SID_SD_A11Y_P_DATE_N);
            break;
        case PRESENTATION_PAGENUMBER:
            sName = SdResId(SID_SD_A11Y_P_NUMBER_N);
            break;
        default:
            sName = SdResId(SID_SD_A11Y_P_UNKNOWN_N);
            if (mxShape.is())
                sName += ": " + mxShape->getShapeType();
    }

    return sName;
}

OUString AccessiblePresentationShape::GetStyle() const
{
    OUString sName;

    ShapeTypeId nShapeType = ShapeTypeHandler::Instance().GetTypeId(mxShape);
    switch (nShapeType)
    {
        case PRESENTATION_TITLE:
            sName = SdResId(SID_SD_A11Y_P_TITLE_N_STYLE);
            break;
        case PRESENTATION_OUTLINER:
            sName = SdResId(SID_SD_A11Y_P_OUTLINER_N_STYLE);
            break;
        case PRESENTATION_SUBTITLE:
            sName = SdResId(SID_SD_A11Y_P_SUBTITLE_N_STYLE);
            break;
        case PRESENTATION_PAGE:
            sName = SdResId(SID_SD_A11Y_P_PAGE_N_STYLE);
            break;
        case PRESENTATION_NOTES:
            sName = SdResId(SID_SD_A11Y_P_NOTES_N_STYLE);
            break;
        case PRESENTATION_HANDOUT:
            sName = SdResId(SID_SD_A11Y_P_HANDOUT_N_STYLE);
            break;
        case PRESENTATION_FOOTER:
            sName = SdResId(SID_SD_A11Y_P_FOOTER_N_STYLE);
            break;
        case PRESENTATION_HEADER:
            sName = SdResId(SID_SD_A11Y_P_HEADER_N_STYLE);
            break;
        case PRESENTATION_DATETIME:
            sName = SdResId(SID_SD_A11Y_P_DATE_N_STYLE);
            break;
        case PRESENTATION_PAGENUMBER:
            sName = SdResId(SID_SD_A11Y_P_NUMBER_N_STYLE);
            break;
        default:
            sName = SdResId(SID_SD_A11Y_P_UNKNOWN_N_STYLE);
            if (mxShape.is())
                sName += ": " + mxShape->getShapeType();
    }

    return sName;
}
} // end of namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
