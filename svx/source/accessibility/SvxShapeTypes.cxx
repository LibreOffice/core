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
#include <svx/ShapeTypeHandler.hxx>
#include <AccessibleTableShape.hxx>

namespace accessibility {

static rtl::Reference<AccessibleShape> CreateSvxAccessibleShape (
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
        // Default accessibility shape for
        // css::drawing::CustomShape (#i37790#)
        case DRAWING_CUSTOM:
        // Default accessibility shape for
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
            return nullptr;
    }
}

void RegisterDrawShapeTypes()
{
    /** List of shape type descriptors corresponding to the
        <type>SvxShapeTypes</type> enum.
    */
    static ShapeTypeDescriptor const aSvxShapeTypeList[] = {
        ShapeTypeDescriptor ( DRAWING_TEXT, u"com.sun.star.drawing.TextShape"_ustr,
            CreateSvxAccessibleShape),
        ShapeTypeDescriptor (DRAWING_RECTANGLE, u"com.sun.star.drawing.RectangleShape"_ustr,
            CreateSvxAccessibleShape),
        ShapeTypeDescriptor ( DRAWING_ELLIPSE, u"com.sun.star.drawing.EllipseShape"_ustr,
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_CONTROL, u"com.sun.star.drawing.ControlShape"_ustr,
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_CONNECTOR, u"com.sun.star.drawing.ConnectorShape"_ustr,
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_MEASURE, u"com.sun.star.drawing.MeasureShape"_ustr,
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_LINE, u"com.sun.star.drawing.LineShape"_ustr,
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_POLY_POLYGON, u"com.sun.star.drawing.PolyPolygonShape"_ustr,
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_POLY_LINE, u"com.sun.star.drawing.PolyLineShape"_ustr,
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_OPEN_BEZIER, u"com.sun.star.drawing.OpenBezierShape"_ustr,
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_CLOSED_BEZIER, u"com.sun.star.drawing.ClosedBezierShape"_ustr,
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_OPEN_FREEHAND, u"com.sun.star.drawing.OpenFreeHandShape"_ustr,
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_CLOSED_FREEHAND, u"com.sun.star.drawing.ClosedFreeHandShape"_ustr,
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_POLY_POLYGON_PATH, u"com.sun.star.drawing.PolyPolygonPathShape"_ustr,
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_POLY_LINE_PATH, u"com.sun.star.drawing.PolyLinePathShape"_ustr,
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_GRAPHIC_OBJECT, u"com.sun.star.drawing.GraphicObjectShape"_ustr,
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_GROUP, u"com.sun.star.drawing.GroupShape"_ustr,
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_OLE, u"com.sun.star.drawing.OLE2Shape"_ustr,
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_PAGE, u"com.sun.star.drawing.PageShape"_ustr,
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_CAPTION, u"com.sun.star.drawing.CaptionShape"_ustr,
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_FRAME, u"com.sun.star.drawing.FrameShape"_ustr,
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_PLUGIN, u"com.sun.star.drawing.PluginShape"_ustr,
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_APPLET, u"com.sun.star.drawing.AppletShape"_ustr,
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_3D_SCENE, u"com.sun.star.drawing.Shape3DSceneObject"_ustr,
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_3D_CUBE, u"com.sun.star.drawing.Shape3DCubeObject"_ustr,
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_3D_SPHERE, u"com.sun.star.drawing.Shape3DSphereObject"_ustr,
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_3D_LATHE, u"com.sun.star.drawing.Shape3DLatheObject"_ustr,
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_3D_EXTRUDE, u"com.sun.star.drawing.Shape3DExtrudeObject"_ustr,
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_CUSTOM, u"com.sun.star.drawing.CustomShape"_ustr,
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_TABLE, u"com.sun.star.drawing.TableShape"_ustr,
            CreateSvxAccessibleShape ),
        ShapeTypeDescriptor ( DRAWING_MEDIA, u"com.sun.star.drawing.MediaShape"_ustr,
            CreateSvxAccessibleShape ),

    };

    // Crash while inserting callout with activated accessibility (#i37790#)
    ShapeTypeHandler::Instance().AddShapeTypeList ( DRAWING_END, aSvxShapeTypeList);
}

} // end of namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
