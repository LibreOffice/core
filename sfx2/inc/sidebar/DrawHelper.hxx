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
#pragma once

#include <vcl/outdev.hxx>

class Color;
class SvBorder;

namespace sfx2::sidebar {

/** Some convenience functions for painting backgrounds and borders.
*/
class DrawHelper
{
public:
    static void DrawHorizontalLine(vcl::RenderContext& rRenderContext, const sal_Int32 nLeft, const sal_Int32 nRight,
                                   const sal_Int32 nY, const sal_Int32 nHeight, const Color& rColor);
    static void DrawVerticalLine(vcl::RenderContext& rRenderContext, const sal_Int32 nTop, const sal_Int32 nBottom,
                                 const sal_Int32 nX, const sal_Int32 nWidth, const Color& rColor);
};

} // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
