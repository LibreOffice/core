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

#include <salhelper/simplereferenceobject.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/rendering/StringContext.hpp>
#include <com/sun/star/geometry/RealRectangle2D.hpp>

#include "canvasfont.hxx"
#include "impltools.hxx"

/* Definition of TextLayout class */

namespace vclcanvas
{
    /** This is the central interface for text layouting.<p>

        This is the central interface for text-related tasks more
        complicated than simple string rendering. Note that all query
        methods are subject to the current layout state of this
        object. That is, calls to XTextLayout::justify()
        or XTextLayout::applyLogicalAdvancements() are
        likely to change subsequent output of those query methods.<p>

        Similar to XCanvasFont, all measurements and
        coordinates accepted and returned by this interface are relative
        to the font's local coordinate system (which only equals device
        coordinate space, if the combined render transformation used
        during text output is the identity transformation). Conversely, if
        the combined transformation used during text output is
        <em>not</em> the identity transformation, all measurements
        returned by this interface should be subjected to that
        transformation, to yield values in device coordinate space.
        Depending on the underlying font technology, actual device output
        might be off by up to one device pixel from the transformed
        metrics.<p>
     */
    class TextLayout : public salhelper::SimpleReferenceObject
    {
    public:
        /// make noncopyable
        TextLayout(const TextLayout&) = delete;
        const TextLayout& operator=(const TextLayout&) = delete;

        TextLayout( css::rendering::StringContext                 aText,
                    sal_Int8                                      nDirection,
                    rtl::Reference<CanvasFont>                    rFont,
                    const VclPtr<OutputDevice> &                  xOutDev );

        /** Query the advancements for every character in the input string.<p>

            This method returns a sequence of advancements, one for each
            character in the input string (<em>not</em> for every
            glyph. There might be multiple glyphs per input character, or
            multiple input characters per glyph).

            An advancement value is the distance of the glyph to the beginning
            edge, which is left for LTR text and is right for RTL text. The
            maximum of the advancements can be deemed as the width of the whole
            text layout.

            This method can be used to query for the layout's default
            advancements, which can subsequently be changed and applied to
            the layout via
            XTextLayout::applyLogicalAdvancements().<p>

            @returns a sequence of double specifying the
            advancements per character in font coordinate space.

            @see XTextLayout::applyLogicalAdvancements()
         */
        cpo::uno::Sequence< double > queryLogicalAdvancements(  );
        /** Apply explicit advancements for every character in the layout
            string.<p>

            This method applies the specified advancements to every
            logical character in the input string (<em>not</em> for every
            glyph. There might be multiple glyphs per input character, or
            multiple input characters per glyph). This is useful to
            explicitly manipulate the exact output positions of
            characters, e.g. relative to a reference output device.<p>

            @param aAdvancements
            A sequence of character advancements, in font coordinate
            space.

            @see XTextLayout::queryLogicalAdvancements()

            @throws com::sun::star::lang::IllegalArgumentException
            if the size of aAdvancements does not match the number of
            characters in the text.
         */
        void applyLogicalAdvancements( const cpo::uno::Sequence< double >& aAdvancements );
        /** Query the Kashida insertion positions in the input string.<p>

            This method returns a sequence of Kashida insertion positions, one for
            each character in the input string (<em>not</em> for every
            glyph. There might be multiple glyphs per input character, or
            multiple input characters per glyph).<p>

            A Kashida insertion position is a boolean indicating if Kashida should
            inserted after this character.<p>

            This method can be used to query for the layout's default Kashida
            insertion positions, which can subsequently be changed and applied to
            the layout via
            XTextLayout::applyKashidaPositions().<p>

            @returns a sequence of booleans specifying the Kashida insertion
            positions per character.

            @see XTextLayout::applyKashidaPositions)
         */
        cpo::uno::Sequence< bool > queryKashidaPositions(  );
        /** Apply Kashida insertion positions for the layout string.<p>

            This method applies the specified Kashida insertion positions to every
            logical character in the input string (<em>not</em> for every
            glyph. There might be multiple glyphs per input character, or
            multiple input characters per glyph).<p>

            @param aPositions
            A sequence of booleans specifying Kashida insertion positions.

            @see XTextLayout::queryKashidaPositions()

            @throws com::sun::star::lang::IllegalArgumentException
            if the size of aPositions is not zero and does not match the number of
            characters in the text.
         */
        void applyKashidaPositions( const cpo::uno::Sequence< bool >& aPositions );
        /** Query the overall bounding box of the text.<p>

            This method is similar to
            XTextLayout::queryTextMeasures(), only that the
            overall bounds are returned by this method.<p>

            @return the overall bounding box for the given layout, in font
            coordinate space.
         */
        css::geometry::RealRectangle2D queryTextBounds();
        /** This method returns the main writing direction.<p>

            This method returns the main writing direction of this layout,
            i.e. either LEFT_TO_RIGHT or RIGHT_TO_LEFT.<p>

            @returns the main text direction of this layout.
         */
        sal_Int8 getMainTextDirection();
        /** Request the associated font for this layout.

            @returns the associated font for this layout.
         */
        rtl::Reference< vclcanvas::CanvasFont > getFont();
        /** Request the text this layout contains.

            @returns the text this layout contains.
         */
        css::rendering::StringContext getText();

        void draw( OutputDevice&                                   rOutDev,
                   const Point&                                    rOutpos,
                   const ::vclcanvas::ViewState&                viewState,
                   const ::vclcanvas::RenderState&              renderState ) const;

    private:
        KernArray setupTextOffsets(
                               const cpo::uno::Sequence< double >& inputOffsets,
                               const ::vclcanvas::ViewState&    viewState,
                               const ::vclcanvas::RenderState&  renderState     ) const;

        css::rendering::StringContext                    maText;
        cpo::uno::Sequence< double >                     maLogicalAdvancements;
        cpo::uno::Sequence< bool >                   maKashidaPositions;
        rtl::Reference<CanvasFont>                       mpFont;
        VclPtr<OutputDevice>                             mxOutDev;
        sal_Int8                                         mnTextDirection;
    };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
