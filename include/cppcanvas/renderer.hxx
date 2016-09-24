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

#ifndef INCLUDED_CPPCANVAS_RENDERER_HXX
#define INCLUDED_CPPCANVAS_RENDERER_HXX

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <boost/optional.hpp>
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
        /** Render subset of metafile to given canvas

            This method renders the given subset of the content to the
            associated canvas.

            @param nStartIndex
            The index of the first action to be rendered (the indices
            correspond roughly to the action indices of the
            originating GDIMetaFile. Note, although, that certain
            actions, e.g. text, accounts for more than one index: a
            text produces as many addressable indices as it has
            characters).

            @param nEndIndex
            The index of the first action _not_ painted anymore,
            i.e. the action after the last action rendered (the
            indices correspond roughly to the action indices of the
            originating GDIMetaFile. Note, although, that certain
            actions, e.g. text, accounts for more than one index: a
            text produces as many addressable indices as it has
            characters).

            @return whether the rendering finished successfully.
         */
        virtual bool drawSubset( sal_Int32  nStartIndex,
                                 sal_Int32  nEndIndex ) const = 0;

        /** Query bounding box of metafile subset

            This method queries the actual bounding box of the given
            subset, when rendered on the associated canvas.

            @param nStartIndex
            The index of the first action to be rendered (the indices
            correspond roughly to the action indices of the
            originating GDIMetaFile. Note, although, that certain
            actions, e.g. text, accounts for more than one index: a
            text produces as many addressable indices as it has
            characters).

            @param nEndIndex
            The index of the first action _not_ painted anymore,
            i.e. the action after the last action rendered (the
            indices correspond roughly to the action indices of the
            originating GDIMetaFile. Note, although, that certain
            actions, e.g. text, accounts for more than one index: a
            text produces as many addressable indices as it has
            characters).

            @return the bounding box of the specified subset
         */
        virtual ::basegfx::B2DRange getSubsetArea( sal_Int32    nStartIndex,
                                                   sal_Int32    nEndIndex ) const = 0;

        /** Parameters for the Renderer
         */
        struct Parameters
        {
            /// Optionally forces the fill color attribute for all actions
            ::boost::optional< Color::IntSRGBA >            maFillColor;

            /// Optionally forces the line color attribute for all actions
            ::boost::optional< Color::IntSRGBA >        maLineColor;

            /// Optionally forces the text color attribute for all actions
            ::boost::optional< Color::IntSRGBA >        maTextColor;

            /// Optionally forces the given fontname for all text actions
            ::boost::optional< OUString >        maFontName;

            /** Optionally transforms all text output actions with the
                given matrix (in addition to the overall canvas
                transformation).

                Note that the matrix given here is applied to the unit
                rect coordinate system, i.e. the metafile is assumed
                to be contained in the unit rect.
             */
            ::boost::optional< ::basegfx::B2DHomMatrix >    maTextTransformation;

            /// Optionally forces the given font weight for all text actions
            ::boost::optional< sal_Int8 >                   maFontWeight;

            /// Optionally forces the given font letter form (italics etc.) for all text actions
            ::boost::optional< sal_Int8 >                   maFontLetterForm;

            /// Optionally forces the given font proportion (condensed, monospaced etc.) for all text actions
            ::boost::optional< sal_Int8 >                   maFontProportion;

            /// Optionally forces underlining for all text actions
            ::boost::optional< bool >                       maFontUnderline;
        };
    };

    typedef std::shared_ptr< ::cppcanvas::Renderer > RendererSharedPtr;
}

#endif // INCLUDED_CPPCANVAS_RENDERER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
