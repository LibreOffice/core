/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <optional>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <cppcanvas/canvasgraphic.hxx>
#include <cppcanvas/color.hxx>
#include <memory>

namespace basegfx
{
    class B2DRange;
}

/* Definition of Renderer interface */

namespace cppcanvas
{

    class Renderer : public virtual CanvasGraphic
    {
    public:

        /** Parameters for the Renderer
         */
        struct Parameters
        {
            /// Optionally forces the fill color attribute for all actions
            ::std::optional< IntSRGBA >                  maFillColor;

            /// Optionally forces the line color attribute for all actions
            ::std::optional< IntSRGBA >                 maLineColor;

            /// Optionally forces the text color attribute for all actions
            ::std::optional< IntSRGBA >                  maTextColor;

            /// Optionally forces the given fontname for all text actions
            ::std::optional< OUString >                  maFontName;

            /** Optionally transforms all text output actions with the
                given matrix (in addition to the overall canvas
                transformation).

                Note that the matrix given here is applied to the unit
                rect coordinate system, i.e. the metafile is assumed
                to be contained in the unit rect.
             */
            ::std::optional< ::basegfx::B2DHomMatrix >    maTextTransformation;

            /// Optionally forces the given font weight for all text actions
            ::std::optional< sal_Int8 >                   maFontWeight;

            /// Optionally forces the given font letter form (italics etc.) for all text actions
            ::std::optional< sal_Int8 >                   maFontLetterForm;

            /// Optionally forces underlining for all text actions
            ::std::optional< bool >                       maFontUnderline;
        };
    };

    typedef std::shared_ptr< ::cppcanvas::Renderer > RendererSharedPtr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
