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

#ifndef INCLUDED_OOX_DRAWINGML_SHAPE3DPROPERTIES_HXX
#define INCLUDED_OOX_DRAWINGML_SHAPE3DPROPERTIES_HXX

#include <map>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/geometry/IntegerRectangle2D.hpp>
#include <oox/core/xmlfilterbase.hxx>
#include <oox/drawingml/color.hxx>
#include <oox/helper/helper.hxx>

namespace oox { class PropertyMap; }
namespace oox { class PropertySet; }
namespace oox { namespace core { class ModelObjectContainer; } }

namespace oox {
namespace drawingml {



struct Shape3DProperties
{
    OptValue< sal_Int32 > mnPreset;
    OptValue< float > mfFieldOfVision;
    OptValue< float > mfZoom;
    OptValue< sal_Int32 > mnLightRigDirection;
    OptValue< sal_Int32 > mnLightRigType;

    /** Overwrites all members that are explicitly set in rSourceProps. */
    void                assignUsed( const Shape3DProperties& rSourceProps );
};



} // namespace drawingml
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
