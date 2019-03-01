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

#include <svx/ShapeTypeHandler.hxx>
#include <SdShapeTypes.hxx>
#include <AccessiblePresentationShape.hxx>
#include <AccessiblePresentationGraphicShape.hxx>
#include <AccessiblePresentationOLEShape.hxx>

namespace accessibility {

static AccessibleShape*
    CreateSdAccessibleShape (
        const AccessibleShapeInfo& rShapeInfo,
        const AccessibleShapeTreeInfo& rShapeTreeInfo,
        ShapeTypeId nId)
{
    switch (nId)
    {
        case PRESENTATION_TITLE:
        case PRESENTATION_OUTLINER:
        case PRESENTATION_SUBTITLE:
        case PRESENTATION_PAGE:
        case PRESENTATION_NOTES:
        case PRESENTATION_HANDOUT:
        case PRESENTATION_HEADER:
        case PRESENTATION_FOOTER:
        case PRESENTATION_DATETIME:
        case PRESENTATION_PAGENUMBER:
            return new AccessiblePresentationShape (rShapeInfo, rShapeTreeInfo);

        case PRESENTATION_GRAPHIC_OBJECT:
            return new AccessiblePresentationGraphicShape (rShapeInfo, rShapeTreeInfo);

        case PRESENTATION_OLE:
        case PRESENTATION_CHART:
        case PRESENTATION_TABLE:
            return new AccessiblePresentationOLEShape (rShapeInfo, rShapeTreeInfo);

        default:
            return new AccessibleShape (rShapeInfo, rShapeTreeInfo);
    }
}

void RegisterImpressShapeTypes()
{
    /** List of shape type descriptors corresponding to the
        <type>SdShapeTypes</type> enum.
    */
    ShapeTypeDescriptor aSdShapeTypeList[] = {
        ShapeTypeDescriptor (
            PRESENTATION_OUTLINER,
            "com.sun.star.presentation.OutlinerShape",
            CreateSdAccessibleShape ),
        ShapeTypeDescriptor (
            PRESENTATION_SUBTITLE,
            "com.sun.star.presentation.SubtitleShape",
            CreateSdAccessibleShape ),
        ShapeTypeDescriptor (
            PRESENTATION_GRAPHIC_OBJECT,
            "com.sun.star.presentation.GraphicObjectShape",
            CreateSdAccessibleShape ),
        ShapeTypeDescriptor (
            PRESENTATION_PAGE,
            "com.sun.star.presentation.PageShape",
            CreateSdAccessibleShape ),
        ShapeTypeDescriptor (
            PRESENTATION_OLE,
            "com.sun.star.presentation.OLE2Shape",
            CreateSdAccessibleShape ),
        ShapeTypeDescriptor (
            PRESENTATION_CHART,
            "com.sun.star.presentation.ChartShape",
            CreateSdAccessibleShape ),
        ShapeTypeDescriptor (
            PRESENTATION_TABLE,
            "com.sun.star.presentation.TableShape",
            CreateSdAccessibleShape ),
        ShapeTypeDescriptor (
            PRESENTATION_NOTES,
            "com.sun.star.presentation.NotesShape",
            CreateSdAccessibleShape ),
        ShapeTypeDescriptor (
            PRESENTATION_TITLE,
            "com.sun.star.presentation.TitleTextShape",
            CreateSdAccessibleShape ),
        ShapeTypeDescriptor (
            PRESENTATION_HANDOUT,
            "com.sun.star.presentation.HandoutShape",
            CreateSdAccessibleShape ),
        ShapeTypeDescriptor (
            PRESENTATION_HEADER,
            "com.sun.star.presentation.HeaderShape",
            CreateSdAccessibleShape ),
        ShapeTypeDescriptor (
            PRESENTATION_FOOTER,
            "com.sun.star.presentation.FooterShape",
            CreateSdAccessibleShape ),
        ShapeTypeDescriptor (
            PRESENTATION_DATETIME,
            "com.sun.star.presentation.DateTimeShape",
            CreateSdAccessibleShape ),
        ShapeTypeDescriptor (
            PRESENTATION_PAGENUMBER,
            "com.sun.star.presentation.SlideNumberShape",
            CreateSdAccessibleShape )
    };

    ShapeTypeHandler::Instance().AddShapeTypeList (
        PRESENTATION_PAGENUMBER - PRESENTATION_OUTLINER + 1,
        aSdShapeTypeList);
}

} // end of namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
