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


#include <svx/SvxShapeTypes.hxx>
#include <svx/AccessibleShape.hxx>
#include <svx/AccessibleGraphicShape.hxx>
#include <svx/AccessibleOLEShape.hxx>
#include <svx/AccessibleControlShape.hxx>
#include <AccessibleTableShape.hxx>

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
        // css::drawing::CustomShape (#i37790#)
        case DRAWING_CUSTOM:
        // Default accessiblility shape for
        // css::drawing::MediaShape (#i85429#)
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

void RegisterDrawShapeTypes()
{
    /** List of shape type descriptors corresponding to the
        <type>SvxShapeTypes</type> enum.
    */
    ShapeTypeDescriptor aSvxShapeTypeList[] = {
        ShapeTypeDescriptor ( DRAWING_TEXT, "com.sun.star.drawing.TextShape",
            CreateSvxAccessibleShape),
        ShapeTypeDescriptor (DRAWING_RECTANGLE, "com.sun.star.drawing.RectangleShape",
            CreateSvxAccessibleShape),
        ShapeTypeDescriptor ( DRAWING_ELLIPSE, "com.sun.star.drawing.EllipseShape",
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_CONTROL, "com.sun.star.drawing.ControlShape",
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_CONNECTOR, "com.sun.star.drawing.ConnectorShape",
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_MEASURE, "com.sun.star.drawing.MeasureShape",
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_LINE, "com.sun.star.drawing.LineShape",
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_POLY_POLYGON, "com.sun.star.drawing.PolyPolygonShape",
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_POLY_LINE, "com.sun.star.drawing.PolyLineShape",
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_OPEN_BEZIER, "com.sun.star.drawing.OpenBezierShape",
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_CLOSED_BEZIER, "com.sun.star.drawing.ClosedBezierShape",
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_OPEN_FREEHAND, "com.sun.star.drawing.OpenFreeHandShape",
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_CLOSED_FREEHAND, "com.sun.star.drawing.ClosedFreeHandShape",
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_POLY_POLYGON_PATH, "com.sun.star.drawing.PolyPolygonPathShape",
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_POLY_LINE_PATH, "com.sun.star.drawing.PolyLinePathShape",
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_GRAPHIC_OBJECT, "com.sun.star.drawing.GraphicObjectShape",
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_GROUP, "com.sun.star.drawing.GroupShape",
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_OLE, "com.sun.star.drawing.OLE2Shape",
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_PAGE, "com.sun.star.drawing.PageShape",
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_CAPTION, "com.sun.star.drawing.CaptionShape",
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_FRAME, "com.sun.star.drawing.FrameShape",
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_PLUGIN, "com.sun.star.drawing.PluginShape",
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_APPLET, "com.sun.star.drawing.AppletShape",
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_3D_SCENE, "com.sun.star.drawing.Shape3DSceneObject",
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_3D_CUBE, "com.sun.star.drawing.Shape3DCubeObject",
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_3D_SPHERE, "com.sun.star.drawing.Shape3DSphereObject",
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_3D_LATHE, "com.sun.star.drawing.Shape3DLatheObject",
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_3D_EXTRUDE, "com.sun.star.drawing.Shape3DExtrudeObject",
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_CUSTOM, "com.sun.star.drawing.CustomShape",
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_TABLE, "com.sun.star.drawing.TableShape",
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_MEDIA, "com.sun.star.drawing.MediaShape",
            CreateSvxAccessibleShape ),

    };

    // Crash while inserting callout with activated accesibility (#i37790#)
    ShapeTypeHandler::Instance().AddShapeTypeList ( DRAWING_END, aSvxShapeTypeList);
}

} // end of namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
