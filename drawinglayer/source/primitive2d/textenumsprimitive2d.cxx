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

#include <drawinglayer/primitive2d/textenumsprimitive2d.hxx>


namespace drawinglayer
{
    namespace primitive2d
    {
        TextLine mapFontLineStyleToTextLine(FontLineStyle eLineStyle)
        {
            switch(eLineStyle)
            {
                case LINESTYLE_SINGLE:          return TEXT_LINE_SINGLE;
                case LINESTYLE_DOUBLE:          return TEXT_LINE_DOUBLE;
                case LINESTYLE_DOTTED:          return TEXT_LINE_DOTTED;
                case LINESTYLE_DASH:            return TEXT_LINE_DASH;
                case LINESTYLE_LONGDASH:        return TEXT_LINE_LONGDASH;
                case LINESTYLE_DASHDOT:         return TEXT_LINE_DASHDOT;
                case LINESTYLE_DASHDOTDOT:      return TEXT_LINE_DASHDOTDOT;
                case LINESTYLE_SMALLWAVE:       return TEXT_LINE_SMALLWAVE;
                case LINESTYLE_WAVE:            return TEXT_LINE_WAVE;
                case LINESTYLE_DOUBLEWAVE:      return TEXT_LINE_DOUBLEWAVE;
                case LINESTYLE_BOLD:            return TEXT_LINE_BOLD;
                case LINESTYLE_BOLDDOTTED:      return TEXT_LINE_BOLDDOTTED;
                case LINESTYLE_BOLDDASH:        return TEXT_LINE_BOLDDASH;
                case LINESTYLE_BOLDLONGDASH:    return TEXT_LINE_BOLDLONGDASH;
                case LINESTYLE_BOLDDASHDOT:     return TEXT_LINE_BOLDDASHDOT;
                case LINESTYLE_BOLDDASHDOTDOT:  return TEXT_LINE_BOLDDASHDOTDOT;
                case LINESTYLE_BOLDWAVE:        return TEXT_LINE_BOLDWAVE;
                // FontLineStyle_FORCE_EQUAL_SIZE, LINESTYLE_DONTKNOW, LINESTYLE_NONE
                default:                        return TEXT_LINE_NONE;
            }
        }

        FontLineStyle mapTextLineToFontLineStyle(TextLine eLineStyle)
        {
            switch(eLineStyle)
            {
                default: /*TEXT_LINE_NONE*/   return LINESTYLE_NONE;
                case TEXT_LINE_SINGLE:        return LINESTYLE_SINGLE;
                case TEXT_LINE_DOUBLE:        return LINESTYLE_DOUBLE;
                case TEXT_LINE_DOTTED:        return LINESTYLE_DOTTED;
                case TEXT_LINE_DASH:          return LINESTYLE_DASH;
                case TEXT_LINE_LONGDASH:      return LINESTYLE_LONGDASH;
                case TEXT_LINE_DASHDOT:       return LINESTYLE_DASHDOT;
                case TEXT_LINE_DASHDOTDOT:    return LINESTYLE_DASHDOTDOT;
                case TEXT_LINE_SMALLWAVE:     return LINESTYLE_SMALLWAVE;
                case TEXT_LINE_WAVE:          return LINESTYLE_WAVE;
                case TEXT_LINE_DOUBLEWAVE:    return LINESTYLE_DOUBLEWAVE;
                case TEXT_LINE_BOLD:          return LINESTYLE_BOLD;
                case TEXT_LINE_BOLDDOTTED:    return LINESTYLE_BOLDDOTTED;
                case TEXT_LINE_BOLDDASH:      return LINESTYLE_BOLDDASH;
                case TEXT_LINE_BOLDLONGDASH:  return LINESTYLE_LONGDASH;
                case TEXT_LINE_BOLDDASHDOT:   return LINESTYLE_BOLDDASHDOT;
                case TEXT_LINE_BOLDDASHDOTDOT:return LINESTYLE_BOLDDASHDOT;
                case TEXT_LINE_BOLDWAVE:      return LINESTYLE_BOLDWAVE;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
