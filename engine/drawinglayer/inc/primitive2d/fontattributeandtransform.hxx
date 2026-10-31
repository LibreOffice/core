/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
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

#pragma once

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <vcl/rendercontext/State.hxx>

namespace vcl
{
class Font;
}

namespace drawinglayer::attribute
{
class FontAttribute;
}

namespace drawinglayer::primitive2d
{
/// The font attribute of rFont, and the scale, baseline offset and rotation of a text drawn in
/// it, in rTextTransform. rAlignmentOffset is the offset from the alignment point to the
/// baseline for a font that is not aligned at its baseline.
void createFontAttributeAndTransform(const vcl::Font& rFont,
                                     vcl::text::ComplexTextLayoutFlags eLayoutMode,
                                     attribute::FontAttribute& rFontAttribute,
                                     basegfx::B2DHomMatrix& rTextTransform,
                                     basegfx::B2DVector& rAlignmentOffset);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
