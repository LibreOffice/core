/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

//////////////////////////////////////////////////////////////////////////////
// eof
