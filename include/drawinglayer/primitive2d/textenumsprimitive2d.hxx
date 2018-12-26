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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTENUMSPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTENUMSPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <vcl/vclenum.hxx>
#include <tools/fontenum.hxx>


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
        TextLine DRAWINGLAYER_DLLPUBLIC mapFontLineStyleToTextLine(FontLineStyle eLineStyle);
        FontLineStyle DRAWINGLAYER_DLLPUBLIC mapTextLineToFontLineStyle(TextLine eLineStyle);

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
            TEXT_FONT_EMPHASIS_MARK_NONE,
            TEXT_FONT_EMPHASIS_MARK_DOT,
            TEXT_FONT_EMPHASIS_MARK_CIRCLE,
            TEXT_FONT_EMPHASIS_MARK_DISC,
            TEXT_FONT_EMPHASIS_MARK_ACCENT
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


#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTENUMSPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
