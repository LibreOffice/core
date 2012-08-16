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

#ifndef OOX_DRAWINGML_TEXTBODYPROPERTIES_HXX
#define OOX_DRAWINGML_TEXTBODYPROPERTIES_HXX

#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include "oox/helper/helper.hxx"
#include "oox/helper/propertymap.hxx"
#include <boost/optional.hpp>

namespace oox {
namespace drawingml {

// ============================================================================

struct TextBodyProperties
{
    PropertyMap                                     maPropertyMap;
    OptValue< sal_Int32 >                           moRotation;
    OptValue< sal_Int32 >                           moVert;
    boost::optional< sal_Int32 >                    moInsets[4];
    ::com::sun::star::drawing::TextVerticalAdjust   meVA;

    explicit            TextBodyProperties();

    void                pushRotationAdjustments( sal_Int32 nRotation );
    void                pushVertSimulation();
};

// ============================================================================

} // namespace drawingml
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
