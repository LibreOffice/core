/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SdShapeTypes.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 18:27:52 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "SdShapeTypes.hxx"
#include "AccessiblePresentationShape.hxx"
#include "AccessiblePresentationGraphicShape.hxx"
#include "AccessiblePresentationOLEShape.hxx"

#ifndef _RTL_USTRING_H_
#include <rtl/ustring.h>
#endif

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
