/*************************************************************************
 *
 *  $RCSfile: SdShapeTypes.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 17:02:49 $
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

#ifndef _SD_SD_SHAPE_TYPES_HXX
#define _SD_SD_SHAPE_TYPES_HXX

#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_SHAPE_TYPE_HANDLERHXX
#include <svx/ShapeTypeHandler.hxx>
#endif
#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_SHAPE_HXX
#include <svx/AccessibleShape.hxx>
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HPP_
#include <com/sun/star/accessibility/XAccessible.hpp>
#endif

namespace accessibility {

/** Register the SD presentation shape types with the ShapeTypeHandler singleton.
    This method is usually called while loading the sd library.
*/
void RegisterImpressShapeTypes (void);

/** Enum describing all shape types known in the SD project.
*/
enum SdShapeTypes
{
    PRESENTATION_OUTLINER,
    PRESENTATION_SUBTITLE,
    PRESENTATION_GRAPHIC_OBJECT,
    PRESENTATION_PAGE,
    PRESENTATION_OLE,
    PRESENTATION_CHART,
    PRESENTATION_TABLE,
    PRESENTATION_NOTES,
    PRESENTATION_TITLE,
    PRESENTATION_HANDOUT
};

/** Factory method that creates accessibility objects corresponding to the
    type of the given shape.
*/
AccessibleShape*
    CreateSdAccessibleShape (const ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible>& rxParent,
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XShape>& rxShape
        );


/** List of shape type descriptors corresponding to the
    <type>SdShapeTypes</type> enum.
*/
extern ShapeTypeDescriptor aSdShapeTypeList[];


} // end of namespace accessibility

#endif
