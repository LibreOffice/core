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



#ifndef _SVX_ACCESSIBILITY_SVX_SHAPE_TYPES_HXX
#define _SVX_ACCESSIBILITY_SVX_SHAPE_TYPES_HXX

#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_SHAPE_HXX
#include <svx/ShapeTypeHandler.hxx>
#endif
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
