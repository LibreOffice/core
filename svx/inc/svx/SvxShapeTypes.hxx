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

#ifndef _SVX_ACCESSIBILITY_SVX_SHAPE_TYPES_HXX
#define _SVX_ACCESSIBILITY_SVX_SHAPE_TYPES_HXX

#include <svx/ShapeTypeHandler.hxx>
#include <com/sun/star/accessibility/XAccessible.hpp>

namespace accessibility {

/** Register the SVX draw shape types with the ShapeTypeHandler singleton.
    This method is usually called from the shape type handler constructor.
*/
void RegisterDrawShapeTypes (void);

/** Enum describing all shape types known in the SVX project.
*/
enum SvxShapeTypes
{
    DRAWING_RECTANGLE = 1,
    DRAWING_ELLIPSE,
    DRAWING_CONTROL,
    DRAWING_CONNECTOR,
    DRAWING_MEASURE,
    DRAWING_LINE,
    DRAWING_POLY_POLYGON,
    DRAWING_POLY_LINE,
    DRAWING_OPEN_BEZIER,
    DRAWING_CLOSED_BEZIER,
    DRAWING_OPEN_FREEHAND,
    DRAWING_CLOSED_FREEHAND,
    DRAWING_POLY_POLYGON_PATH,
    DRAWING_POLY_LINE_PATH,
    DRAWING_GRAPHIC_OBJECT,
    DRAWING_GROUP,
    DRAWING_TEXT,
    DRAWING_OLE,
    DRAWING_PAGE,
    DRAWING_CAPTION,
    DRAWING_FRAME,
    DRAWING_PLUGIN,
    DRAWING_APPLET,
    DRAWING_3D_SCENE,
    DRAWING_3D_CUBE,
    DRAWING_3D_SPHERE,
    DRAWING_3D_LATHE,
    DRAWING_3D_EXTRUDE,
    DRAWING_CUSTOM,
    DRAWING_TABLE, /* = 30 */
    DRAWING_MEDIA, /* = 31 */
    DRAWING_END = DRAWING_MEDIA
};

/** List of shape type descriptors corresponding to the
    <type>SvxShapeTypes</type> enum.
*/
extern ShapeTypeDescriptor aSvxShapeTypeList[];

} // end of namespace accessibility

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
