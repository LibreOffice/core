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
#include "precompiled_svx.hxx"

#include <svx/SvxShapeTypes.hxx>
#include <svx/AccessibleShape.hxx>
#include <svx/AccessibleGraphicShape.hxx>
#include <svx/AccessibleOLEShape.hxx>
#include <svx/AccessibleControlShape.hxx>
#include <svx/AccessibleTableShape.hxx>

namespace accessibility {

AccessibleShape* CreateSvxAccessibleShape (
    const AccessibleShapeInfo& rShapeInfo,
    const AccessibleShapeTreeInfo& rShapeTreeInfo,
    ShapeTypeId nId)
{
    switch (nId)
    {
        case DRAWING_3D_CUBE:
        case DRAWING_3D_EXTRUDE:
        case DRAWING_3D_LATHE:
        case DRAWING_3D_SCENE:
        case DRAWING_3D_SPHERE:
        case DRAWING_CAPTION:
        case DRAWING_CLOSED_BEZIER:
        case DRAWING_CLOSED_FREEHAND:
        case DRAWING_CONNECTOR:
        case DRAWING_ELLIPSE:
        case DRAWING_GROUP:
        case DRAWING_LINE:
        case DRAWING_MEASURE:
        case DRAWING_OPEN_BEZIER:
        case DRAWING_OPEN_FREEHAND:
        case DRAWING_PAGE:
        case DRAWING_POLY_POLYGON:
        case DRAWING_POLY_LINE:
        case DRAWING_POLY_POLYGON_PATH:
        case DRAWING_POLY_LINE_PATH:
        case DRAWING_RECTANGLE:
        case DRAWING_TEXT:
        // Default accessiblility shape for
        // com::sun::star::drawing::CustomShape (#i37790#)
        case DRAWING_CUSTOM:
        // Default accessiblility shape for
        // com::sun::star::drawing::MediaShape (#i85429#)
        case DRAWING_MEDIA:
            return new AccessibleShape (rShapeInfo, rShapeTreeInfo);

        case DRAWING_CONTROL:
            return new AccessibleControlShape (rShapeInfo, rShapeTreeInfo);

        case DRAWING_GRAPHIC_OBJECT:
            return new AccessibleGraphicShape (rShapeInfo, rShapeTreeInfo);

        case DRAWING_APPLET:
        case DRAWING_FRAME:
        case DRAWING_OLE:
        case DRAWING_PLUGIN:
            return new AccessibleOLEShape (rShapeInfo, rShapeTreeInfo);

        case DRAWING_TABLE:
            return new AccessibleTableShape( rShapeInfo, rShapeTreeInfo );

        default:
            return NULL;
    }
}



ShapeTypeDescriptor aSvxShapeTypeList[] = {
    ShapeTypeDescriptor (DRAWING_TEXT,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.TextShape")),
        CreateSvxAccessibleShape),
    ShapeTypeDescriptor (DRAWING_RECTANGLE,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.RectangleShape")),
        CreateSvxAccessibleShape),
    ShapeTypeDescriptor (   DRAWING_ELLIPSE,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.EllipseShape")),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_CONTROL,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.ControlShape")),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_CONNECTOR,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.ConnectorShape")),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_MEASURE,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.MeasureShape")),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_LINE,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.LineShape")),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_POLY_POLYGON,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.PolyPolygonShape")),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_POLY_LINE,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.PolyLineShape")),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_OPEN_BEZIER,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.OpenBezierShape")),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_CLOSED_BEZIER,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.ClosedBezierShape")),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_OPEN_FREEHAND,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.OpenFreeHandShape")),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_CLOSED_FREEHAND,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.ClosedFreeHandShape")),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_POLY_POLYGON_PATH,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.PolyPolygonPathShape")),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_POLY_LINE_PATH,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.PolyLinePathShape")),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_GRAPHIC_OBJECT,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.GraphicObjectShape")),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_GROUP,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.GroupShape")),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_OLE,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.OLE2Shape")),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_PAGE,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.PageShape")),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_CAPTION,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.CaptionShape")),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_FRAME,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.FrameShape")),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_PLUGIN,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.PluginShape")),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_APPLET,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.AppletShape")),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_3D_SCENE,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.Shape3DSceneObject")),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_3D_CUBE,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.Shape3DCubeObject")),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_3D_SPHERE,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.Shape3DSphereObject")),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_3D_LATHE,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.Shape3DLatheObject")),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_3D_EXTRUDE,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.Shape3DExtrudeObject")),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_CUSTOM,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.CustomShape")),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_TABLE,
        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.TableShape" ) ),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_MEDIA,
        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.MediaShape" ) ),
        CreateSvxAccessibleShape ),

};


void RegisterDrawShapeTypes (void)
{
    // Crash while inserting callout with activated accesibility (#i37790#)
    ShapeTypeHandler::Instance().AddShapeTypeList ( DRAWING_END, aSvxShapeTypeList);
}


} // end of namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
