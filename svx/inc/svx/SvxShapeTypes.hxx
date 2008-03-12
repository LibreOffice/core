/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SvxShapeTypes.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 09:24:57 $
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

#ifndef _SVX_ACCESSIBILITY_SVX_SHAPE_TYPES_HXX
#define _SVX_ACCESSIBILITY_SVX_SHAPE_TYPES_HXX

#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_SHAPE_HXX
#include <svx/ShapeTypeHandler.hxx>
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HPP_
#include <com/sun/star/accessibility/XAccessible.hpp>
#endif

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
    DRAWING_END = DRAWING_TABLE
};

/** List of shape type descriptors corresponding to the
    <type>SvxShapeTypes</type> enum.
*/
extern ShapeTypeDescriptor aSvxShapeTypeList[];

} // end of namespace accessibility

#endif
