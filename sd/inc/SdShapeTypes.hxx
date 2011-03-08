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

#ifndef _SD_SD_SHAPE_TYPES_HXX
#define _SD_SD_SHAPE_TYPES_HXX

#include <svx/ShapeTypeHandler.hxx>
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
