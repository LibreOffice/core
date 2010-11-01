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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "SdShapeTypes.hxx"
#include "AccessiblePresentationShape.hxx"
#include "AccessiblePresentationGraphicShape.hxx"
#include "AccessiblePresentationOLEShape.hxx"
#include <rtl/ustring.h>

namespace accessibility {

AccessibleShape*
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




ShapeTypeDescriptor aSdShapeTypeList[] = {
    ShapeTypeDescriptor (
        PRESENTATION_OUTLINER,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( ("com.sun.star.presentation.OutlinerShape"))),
        CreateSdAccessibleShape ),
    ShapeTypeDescriptor (
        PRESENTATION_SUBTITLE,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( ("com.sun.star.presentation.SubtitleShape"))),
        CreateSdAccessibleShape ),
    ShapeTypeDescriptor (
        PRESENTATION_GRAPHIC_OBJECT,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( ("com.sun.star.presentation.GraphicObjectShape"))),
        CreateSdAccessibleShape ),
    ShapeTypeDescriptor (
        PRESENTATION_PAGE,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( ("com.sun.star.presentation.PageShape"))),
        CreateSdAccessibleShape ),
    ShapeTypeDescriptor (
        PRESENTATION_OLE,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( ("com.sun.star.presentation.OLE2Shape"))),
        CreateSdAccessibleShape ),
    ShapeTypeDescriptor (
        PRESENTATION_CHART,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( ("com.sun.star.presentation.ChartShape"))),
        CreateSdAccessibleShape ),
    ShapeTypeDescriptor (
        PRESENTATION_TABLE,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( ("com.sun.star.presentation.TableShape"))),
        CreateSdAccessibleShape ),
    ShapeTypeDescriptor (
        PRESENTATION_NOTES,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( ("com.sun.star.presentation.NotesShape"))),
        CreateSdAccessibleShape ),
    ShapeTypeDescriptor (
        PRESENTATION_TITLE,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.presentation.TitleTextShape")),
        CreateSdAccessibleShape ),
    ShapeTypeDescriptor (
        PRESENTATION_HANDOUT,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.presentation.HandoutShape")),
        CreateSdAccessibleShape ),
    ShapeTypeDescriptor (
        PRESENTATION_HEADER,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.presentation.HeaderShape")),
        CreateSdAccessibleShape ),
    ShapeTypeDescriptor (
        PRESENTATION_FOOTER,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.presentation.FooterShape")),
        CreateSdAccessibleShape ),
    ShapeTypeDescriptor (
        PRESENTATION_DATETIME,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.presentation.DateTimeShape")),
        CreateSdAccessibleShape ),
    ShapeTypeDescriptor (
        PRESENTATION_PAGENUMBER,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.presentation.SlideNumberShape")),
        CreateSdAccessibleShape )
};




void RegisterImpressShapeTypes (void)
{
    ShapeTypeHandler::Instance().AddShapeTypeList (
        PRESENTATION_PAGENUMBER - PRESENTATION_OUTLINER + 1,
        aSdShapeTypeList);
}




} // end of namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
