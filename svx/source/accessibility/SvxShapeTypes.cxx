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
        case DRAWING_CONNECTOR:
        case DRAWING_ELLIPSE:
        case DRAWING_GROUP:
        case DRAWING_LINE:
        case DRAWING_MEASURE:
        case DRAWING_OPEN_BEZIER:
        case DRAWING_PAGE:
        case DRAWING_POLY_POLYGON:
        case DRAWING_POLY_LINE:
        case DRAWING_POLY_POLYGON_PATH:
        case DRAWING_POLY_LINE_PATH:
        case DRAWING_RECTANGLE:
        case DRAWING_TEXT:
        // --> OD 2004-11-29 #i37790# - default accessiblility shape for
        // com::sun::star::drawing::CustomShape
        case DRAWING_CUSTOM:
        // <--
        // --> OD 2008-05-19 #i85429# - default accessiblility shape for
        // com::sun::star::drawing::MediaShape
        case DRAWING_MEDIA:
        // <--
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
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.TextShape"),
        CreateSvxAccessibleShape),
    ShapeTypeDescriptor (DRAWING_RECTANGLE,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.RectangleShape"),
        CreateSvxAccessibleShape),
    ShapeTypeDescriptor (   DRAWING_ELLIPSE,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.EllipseShape"),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_CONTROL,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.ControlShape"),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_CONNECTOR,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.ConnectorShape"),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_MEASURE,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.MeasureShape"),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_LINE,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.LineShape"),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_POLY_POLYGON,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.PolyPolygonShape"),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_POLY_LINE,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.PolyLineShape"),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_OPEN_BEZIER,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.OpenBezierShape"),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_CLOSED_BEZIER,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.ClosedBezierShape"),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_POLY_POLYGON_PATH,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.PolyPolygonPathShape"),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_POLY_LINE_PATH,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.PolyLinePathShape"),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_GRAPHIC_OBJECT,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.GraphicObjectShape"),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_GROUP,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.GroupShape"),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_OLE,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.OLE2Shape"),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_PAGE,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.PageShape"),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_CAPTION,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.CaptionShape"),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_FRAME,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.FrameShape"),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_PLUGIN,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.PluginShape"),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_APPLET,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.AppletShape"),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_3D_SCENE,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.Shape3DSceneObject"),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_3D_CUBE,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.Shape3DCubeObject"),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_3D_SPHERE,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.Shape3DSphereObject"),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_3D_LATHE,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.Shape3DLatheObject"),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_3D_EXTRUDE,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.Shape3DExtrudeObject"),
        CreateSvxAccessibleShape ),
    ShapeTypeDescriptor (   DRAWING_CUSTOM,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.CustomShape"),
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
    // --> OD 2004-11-26 #i37790#
    ShapeTypeHandler::Instance().AddShapeTypeList ( DRAWING_END, aSvxShapeTypeList);
    // <--
}


} // end of namespace accessibility
