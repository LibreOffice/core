/*************************************************************************
 *
 *  $RCSfile: SvxShapeTypes.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: af $ $Date: 2002-02-07 16:26:50 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "SvxShapeTypes.hxx"

#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_SHAPE_HXX
#include "AccessibleShape.hxx"
#endif
#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_GRAPHIC_SHAPE_HXX
#include "AccessibleGraphicShape.hxx"
#endif
#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_OLE_SHAPE_HXX
#include "AccessibleOLEShape.hxx"
#endif

namespace accessibility {

::com::sun::star::uno::Reference<
    ::drafts::com::sun::star::accessibility::XAccessible>
    createSvxAccessibleShape (const ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::accessibility::XAccessible>& rxParent,
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XShape>& rxShape,
        ShapeTypeId nId)
{
    switch (nId)
    {
        case DRAWING_RECTANGLE:
        case DRAWING_ELLIPSE:
        case DRAWING_CONTROL:
        case DRAWING_CONNECTOR:
        case DRAWING_MEASURE:
        case DRAWING_LINE:
        case DRAWING_POLY_POLYGON:
        case DRAWING_POLY_LINE:
        case DRAWING_OPEN_BEZIER:
        case DRAWING_CLOSED_BEZIER:
        case DRAWING_OPEN_FREEHAND:
        case DRAWING_CLOSED_FREEHAND:
        case DRAWING_POLY_POLYGON_PATH:
        case DRAWING_POLY_LINE_PATH:
        case DRAWING_GROUP:
        case DRAWING_TEXT:
        case DRAWING_PAGE:
        case DRAWING_3D_SCENE:
        case DRAWING_3D_CUBE:
        case DRAWING_3D_SPHERE:
        case DRAWING_3D_LATHE:
        case DRAWING_3D_EXTRUDE:
        case DRAWING_3D_POLYGON:
            return new AccessibleShape (rxShape, rxParent);

        case DRAWING_GRAPHIC_OBJECT:
            return new AccessibleGraphicShape (rxShape, rxParent);

        case DRAWING_OLE:
        case DRAWING_CAPTION:
        case DRAWING_FRAME:
        case DRAWING_PLUGIN:
        case DRAWING_APPLET:
            return new AccessibleOLEShape (rxShape, rxParent);
    }
}



ShapeTypeDescriptor aSvxShapeTypeList[] = {
    {   DRAWING_TEXT,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.TextShape"),
        createSvxAccessibleShape },
    {   DRAWING_RECTANGLE,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.RectangleShape"),
        createSvxAccessibleShape   },
    {   DRAWING_ELLIPSE,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.EllipseShape"),
        createSvxAccessibleShape },
    {   DRAWING_CONTROL,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.ControlShape"),
        createSvxAccessibleShape },
    {   DRAWING_CONNECTOR,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.ConnectorShape"),
        createSvxAccessibleShape },
    {   DRAWING_MEASURE,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.MeasureShape"),
        createSvxAccessibleShape },
    {   DRAWING_LINE,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.LineShape"),
        createSvxAccessibleShape },
    {   DRAWING_POLY_POLYGON,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.PolyPolygonShape"),
        createSvxAccessibleShape },
    {   DRAWING_POLY_LINE,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.PolyLineShape"),
        createSvxAccessibleShape },
    {   DRAWING_OPEN_BEZIER,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.OpenBezierShape"),
        createSvxAccessibleShape },
    {   DRAWING_CLOSED_BEZIER,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.ClosedBezierShape"),
        createSvxAccessibleShape },
    {   DRAWING_OPEN_FREEHAND,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.OpenFreeHandShape"),
        createSvxAccessibleShape },
    {   DRAWING_CLOSED_FREEHAND,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.ClosedFreeHandShape"),
        createSvxAccessibleShape },
    {   DRAWING_POLY_POLYGON_PATH,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.PolyPolygonPathShape"),
        createSvxAccessibleShape },
    {   DRAWING_POLY_LINE_PATH,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.PolyLinePathShape"),
        createSvxAccessibleShape },
    {   DRAWING_GRAPHIC_OBJECT,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.GraphicObjectShape"),
        createSvxAccessibleShape },
    {   DRAWING_GROUP,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.GroupShape"),
        createSvxAccessibleShape },
    {   DRAWING_OLE,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.OLE2Shape"),
        createSvxAccessibleShape },
    {   DRAWING_PAGE,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.PageShape"),
        createSvxAccessibleShape },
    {   DRAWING_CAPTION,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.CaptionShape"),
        createSvxAccessibleShape },
    {   DRAWING_FRAME,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.FrameShape"),
        createSvxAccessibleShape },
    {   DRAWING_PLUGIN,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.PluginShape"),
        createSvxAccessibleShape },
    {   DRAWING_APPLET,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.AppletShape"),
        createSvxAccessibleShape },
    {   DRAWING_3D_SCENE,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.Shape3DSceneObject"),
        createSvxAccessibleShape },
    {   DRAWING_3D_CUBE,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.Shape3DCubeObject"),
        createSvxAccessibleShape },
    {   DRAWING_3D_SPHERE,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.Shape3DSphereObject"),
        createSvxAccessibleShape },
    {   DRAWING_3D_LATHE,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.Shape3DLatheObject"),
        createSvxAccessibleShape },
    {   DRAWING_3D_EXTRUDE,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.Shape3DExtrudeObject"),
        createSvxAccessibleShape },
    {   DRAWING_3D_POLYGON,
        ::rtl::OUString::createFromAscii ("com.sun.star.drawing.Shape3DPolygonObject"),
        createSvxAccessibleShape }
};


void RegisterDrawShapeTypes (void)
{
    ShapeTypeHandler::Instance().addShapeTypeList (
        DRAWING_3D_POLYGON - DRAWING_RECTANGLE + 1,
        aSvxShapeTypeList);
}


} // end of namespace accessibility
