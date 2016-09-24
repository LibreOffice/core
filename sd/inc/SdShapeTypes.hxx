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

#ifndef INCLUDED_SD_INC_SDSHAPETYPES_HXX
#define INCLUDED_SD_INC_SDSHAPETYPES_HXX

#include <svx/ShapeTypeHandler.hxx>
#include <svx/AccessibleShape.hxx>
#include <com/sun/star/accessibility/XAccessible.hpp>

namespace accessibility {

/** Register the SD presentation shape types with the ShapeTypeHandler singleton.
    This method is usually called while loading the sd library.
*/
void RegisterImpressShapeTypes();

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

} // end of namespace accessibility

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
