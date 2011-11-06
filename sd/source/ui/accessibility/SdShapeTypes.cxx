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
        ::rtl::OUString::createFromAscii ("com.sun.star.presentation.OutlinerShape"),
        CreateSdAccessibleShape ),
    ShapeTypeDescriptor (
        PRESENTATION_SUBTITLE,
        ::rtl::OUString::createFromAscii ("com.sun.star.presentation.SubtitleShape"),
        CreateSdAccessibleShape ),
    ShapeTypeDescriptor (
        PRESENTATION_GRAPHIC_OBJECT,
        ::rtl::OUString::createFromAscii ("com.sun.star.presentation.GraphicObjectShape"),
        CreateSdAccessibleShape ),
    ShapeTypeDescriptor (
        PRESENTATION_PAGE,
        ::rtl::OUString::createFromAscii ("com.sun.star.presentation.PageShape"),
        CreateSdAccessibleShape ),
    ShapeTypeDescriptor (
        PRESENTATION_OLE,
        ::rtl::OUString::createFromAscii ("com.sun.star.presentation.OLE2Shape"),
        CreateSdAccessibleShape ),
    ShapeTypeDescriptor (
        PRESENTATION_CHART,
        ::rtl::OUString::createFromAscii ("com.sun.star.presentation.ChartShape"),
        CreateSdAccessibleShape ),
    ShapeTypeDescriptor (
        PRESENTATION_TABLE,
        ::rtl::OUString::createFromAscii ("com.sun.star.presentation.TableShape"),
        CreateSdAccessibleShape ),
    ShapeTypeDescriptor (
        PRESENTATION_NOTES,
        ::rtl::OUString::createFromAscii ("com.sun.star.presentation.NotesShape"),
        CreateSdAccessibleShape ),
    ShapeTypeDescriptor (
        PRESENTATION_TITLE,
        ::rtl::OUString::createFromAscii ("com.sun.star.presentation.TitleTextShape"),
        CreateSdAccessibleShape ),
    ShapeTypeDescriptor (
        PRESENTATION_HANDOUT,
        ::rtl::OUString::createFromAscii ("com.sun.star.presentation.HandoutShape"),
        CreateSdAccessibleShape ),
    ShapeTypeDescriptor (
        PRESENTATION_HEADER,
        ::rtl::OUString::createFromAscii ("com.sun.star.presentation.HeaderShape"),
        CreateSdAccessibleShape ),
    ShapeTypeDescriptor (
        PRESENTATION_FOOTER,
        ::rtl::OUString::createFromAscii ("com.sun.star.presentation.FooterShape"),
        CreateSdAccessibleShape ),
    ShapeTypeDescriptor (
        PRESENTATION_DATETIME,
        ::rtl::OUString::createFromAscii ("com.sun.star.presentation.DateTimeShape"),
        CreateSdAccessibleShape ),
    ShapeTypeDescriptor (
        PRESENTATION_PAGENUMBER,
        ::rtl::OUString::createFromAscii ("com.sun.star.presentation.SlideNumberShape"),
        CreateSdAccessibleShape )
};




void RegisterImpressShapeTypes (void)
{
    ShapeTypeHandler::Instance().AddShapeTypeList (
        PRESENTATION_PAGENUMBER - PRESENTATION_OUTLINER + 1,
        aSdShapeTypeList);
}




} // end of namespace accessibility
