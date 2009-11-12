/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: wrongspellprimitive2d.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2008-05-27 14:11:17 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTENUMSPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTENUMSPRIMITIVE2D_HXX

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>

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

        /** FontStrikeout definition */
        enum FontStrikeout
        {
            FONT_STRIKEOUT_NONE,
            FONT_STRIKEOUT_SINGLE,
            FONT_STRIKEOUT_DOUBLE,
            FONT_STRIKEOUT_BOLD,
            FONT_STRIKEOUT_SLASH,
            FONT_STRIKEOUT_X
        };

        /** FontEmphasisMark definition */
        enum FontEmphasisMark
        {
            FONT_EMPHASISMARK_NONE,
            FONT_EMPHASISMARK_DOT,
            FONT_EMPHASISMARK_CIRCLE,
            FONT_EMPHASISMARK_DISC,
            FONT_EMPHASISMARK_ACCENT
        };

        /** FontRelief definition */
        enum FontRelief
        {
            FONT_RELIEF_NONE,
            FONT_RELIEF_EMBOSSED,
            FONT_RELIEF_ENGRAVED
        };

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTENUMSPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
