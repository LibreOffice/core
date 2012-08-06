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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTENUMSPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTENUMSPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <vcl/vclenum.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** TextLine definition

            This is used for both underline and overline
         */
        enum TextLine
        {
            TEXT_LINE_NONE,
            TEXT_LINE_SINGLE,
            TEXT_LINE_DOUBLE,
            TEXT_LINE_DOTTED,
            TEXT_LINE_DASH,
            TEXT_LINE_LONGDASH,
            TEXT_LINE_DASHDOT,
            TEXT_LINE_DASHDOTDOT,
            TEXT_LINE_SMALLWAVE,
            TEXT_LINE_WAVE,
            TEXT_LINE_DOUBLEWAVE,
            TEXT_LINE_BOLD,
            TEXT_LINE_BOLDDOTTED,
            TEXT_LINE_BOLDDASH,
            TEXT_LINE_BOLDLONGDASH,
            TEXT_LINE_BOLDDASHDOT,
            TEXT_LINE_BOLDDASHDOTDOT,
            TEXT_LINE_BOLDWAVE
        };

        /** helper to convert LineStyle */
        TextLine DRAWINGLAYER_DLLPUBLIC mapFontUnderlineToTextLine(FontUnderline eLineStyle);
        FontUnderline DRAWINGLAYER_DLLPUBLIC mapTextLineToFontUnderline(TextLine eLineStyle);

        /** FontStrikeout definition */
        enum TextStrikeout
        {
            TEXT_STRIKEOUT_NONE,
            TEXT_STRIKEOUT_SINGLE,
            TEXT_STRIKEOUT_DOUBLE,
            TEXT_STRIKEOUT_BOLD,
            TEXT_STRIKEOUT_SLASH,
            TEXT_STRIKEOUT_X
        };

        /** helper to convert FontStrikeout */
        TextStrikeout DRAWINGLAYER_DLLPUBLIC mapFontStrikeoutToTextStrikeout(::FontStrikeout eFontStrikeout);
        ::FontStrikeout DRAWINGLAYER_DLLPUBLIC mapTextStrikeoutToFontStrikeout(TextStrikeout eFontStrikeout);

        /** TextEmphasisMark definition */
        enum TextEmphasisMark
        {
            TEXT_EMPHASISMARK_NONE,
            TEXT_EMPHASISMARK_DOT,
            TEXT_EMPHASISMARK_CIRCLE,
            TEXT_EMPHASISMARK_DISC,
            TEXT_EMPHASISMARK_ACCENT
        };

        /** TextRelief definition */
        enum TextRelief
        {
            TEXT_RELIEF_NONE,
            TEXT_RELIEF_EMBOSSED,
            TEXT_RELIEF_ENGRAVED
        };

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTENUMSPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
