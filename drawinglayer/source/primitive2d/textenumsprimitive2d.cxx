/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textprimitive2d.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: aw $ $Date: 2008-05-27 14:11:20 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

#include <drawinglayer/primitive2d/textenumsprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        TextLine mapFontUnderlineToTextLine(FontUnderline eLineStyle)
        {
            switch(eLineStyle)
            {
                case UNDERLINE_SINGLE:          return TEXT_LINE_SINGLE;
                case UNDERLINE_DOUBLE:          return TEXT_LINE_DOUBLE;
                case UNDERLINE_DOTTED:          return TEXT_LINE_DOTTED;
                case UNDERLINE_DASH:            return TEXT_LINE_DASH;
                case UNDERLINE_LONGDASH:        return TEXT_LINE_LONGDASH;
                case UNDERLINE_DASHDOT:         return TEXT_LINE_DASHDOT;
                case UNDERLINE_DASHDOTDOT:      return TEXT_LINE_DASHDOTDOT;
                case UNDERLINE_SMALLWAVE:       return TEXT_LINE_SMALLWAVE;
                case UNDERLINE_WAVE:            return TEXT_LINE_WAVE;
                case UNDERLINE_DOUBLEWAVE:      return TEXT_LINE_DOUBLEWAVE;
                case UNDERLINE_BOLD:            return TEXT_LINE_BOLD;
                case UNDERLINE_BOLDDOTTED:      return TEXT_LINE_BOLDDOTTED;
                case UNDERLINE_BOLDDASH:        return TEXT_LINE_BOLDDASH;
                case UNDERLINE_BOLDLONGDASH:    return TEXT_LINE_BOLDLONGDASH;
                case UNDERLINE_BOLDDASHDOT:     return TEXT_LINE_BOLDDASHDOT;
                case UNDERLINE_BOLDDASHDOTDOT:  return TEXT_LINE_BOLDDASHDOTDOT;
                case UNDERLINE_BOLDWAVE:        return TEXT_LINE_BOLDWAVE;
                // FontUnderline_FORCE_EQUAL_SIZE, UNDERLINE_DONTKNOW, UNDERLINE_NONE
                default:                        return TEXT_LINE_NONE;
            }
        }

        FontUnderline mapTextLineToFontUnderline(TextLine eLineStyle)
        {
            switch(eLineStyle)
            {
                default: /*TEXT_LINE_NONE*/   return UNDERLINE_NONE;
                case TEXT_LINE_SINGLE:        return UNDERLINE_SINGLE;
                case TEXT_LINE_DOUBLE:        return UNDERLINE_DOUBLE;
                case TEXT_LINE_DOTTED:        return UNDERLINE_DOTTED;
                case TEXT_LINE_DASH:          return UNDERLINE_DASH;
                case TEXT_LINE_LONGDASH:      return UNDERLINE_LONGDASH;
                case TEXT_LINE_DASHDOT:       return UNDERLINE_DASHDOT;
                case TEXT_LINE_DASHDOTDOT:    return UNDERLINE_DASHDOTDOT;
                case TEXT_LINE_SMALLWAVE:     return UNDERLINE_SMALLWAVE;
                case TEXT_LINE_WAVE:          return UNDERLINE_WAVE;
                case TEXT_LINE_DOUBLEWAVE:    return UNDERLINE_DOUBLEWAVE;
                case TEXT_LINE_BOLD:          return UNDERLINE_BOLD;
                case TEXT_LINE_BOLDDOTTED:    return UNDERLINE_BOLDDOTTED;
                case TEXT_LINE_BOLDDASH:      return UNDERLINE_BOLDDASH;
                case TEXT_LINE_BOLDLONGDASH:  return UNDERLINE_LONGDASH;
                case TEXT_LINE_BOLDDASHDOT:   return UNDERLINE_BOLDDASHDOT;
                case TEXT_LINE_BOLDDASHDOTDOT:return UNDERLINE_BOLDDASHDOT;
                case TEXT_LINE_BOLDWAVE:      return UNDERLINE_BOLDWAVE;
            }
        }

        TextStrikeout mapFontStrikeoutToTextStrikeout(FontStrikeout eFontStrikeout)
        {
            switch(eFontStrikeout)
            {
                case STRIKEOUT_SINGLE:  return TEXT_STRIKEOUT_SINGLE;
                case STRIKEOUT_DOUBLE:  return TEXT_STRIKEOUT_DOUBLE;
                case STRIKEOUT_BOLD:    return TEXT_STRIKEOUT_BOLD;
                case STRIKEOUT_SLASH:   return TEXT_STRIKEOUT_SLASH;
                case STRIKEOUT_X:       return TEXT_STRIKEOUT_X;
                // FontStrikeout_FORCE_EQUAL_SIZE, STRIKEOUT_NONE, STRIKEOUT_DONTKNOW
                default:                return TEXT_STRIKEOUT_NONE;
            }
        }

        FontStrikeout mapTextStrikeoutToFontStrikeout(TextStrikeout eTextStrikeout)
        {
            switch(eTextStrikeout)
            {
                default: /*case primitive2d::TEXT_STRIKEOUT_NONE*/  return STRIKEOUT_NONE;
                case TEXT_STRIKEOUT_SINGLE:     return STRIKEOUT_SINGLE;
                case TEXT_STRIKEOUT_DOUBLE:     return STRIKEOUT_DOUBLE;
                case TEXT_STRIKEOUT_BOLD:       return STRIKEOUT_BOLD;
                case TEXT_STRIKEOUT_SLASH:      return STRIKEOUT_SLASH;
                case TEXT_STRIKEOUT_X:          return STRIKEOUT_X;
            }
        }

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
