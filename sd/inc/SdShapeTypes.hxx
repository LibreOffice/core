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



#ifndef _SD_SD_SHAPE_TYPES_HXX
#define _SD_SD_SHAPE_TYPES_HXX

#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_SHAPE_TYPE_HANDLERHXX
#include <svx/ShapeTypeHandler.hxx>
#endif
#include <svx/AccessibleShape.hxx>
#include <com/sun/star/accessibility/XAccessible.hpp>

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
    PRESENTATION_HANDOUT,
    PRESENTATION_HEADER,
    PRESENTATION_FOOTER,
    PRESENTATION_DATETIME,
    PRESENTATION_PAGENUMBER
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
