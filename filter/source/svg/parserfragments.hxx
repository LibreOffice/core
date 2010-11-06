/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Fridrich Strba  <fridrich.strba@bluewin.ch>
 *       Thorsten Behrens <tbehrens@novell.com>
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */
#ifndef INCLUDED_PARSERFRAGMENTS_HXX
#define INCLUDED_PARSERFRAGMENTS_HXX

#include <sal/config.h>
#include <vector>
#include <utility>
#include <string>

namespace basegfx
{
    class B2DHomMatrix;
    class B2DRange;
}
namespace svgi
{
    struct ARGBColor;

    /// Parse given string for one of the SVG color grammars
    bool parseColor( const char* sColor, ARGBColor& rColor );
    bool parseOpacity( const char* sOpacity, ARGBColor& rColor );

    /// Parse given string for one of the SVG transformation grammars
    bool parseTransform( const char* sTransform, basegfx::B2DHomMatrix& rTransform );

    /// Parse given string for the viewBox attribute
    bool parseViewBox( const char* sViewbox, basegfx::B2DRange& rRect );

    /// Parse given string for a list of double values, comma-delimited
    bool parseDashArray( const char* sDashArray, std::vector<double>& rOutputVector );

    /** Parse paint uri

        @param o_rPaintUri
        Start and end ptr for uri substring (within
        [sPaintUri,sPaintUri+strlen(sPaintUri)]

        @param io_rColor
        The optional paint color to use. if o_rPaintUri is empty,
        parser sets io_rColor.second to false for color="None", to
        true and keeps current io_rColor.first entry for
        "currentColor", and to true and sets io_rColor.first to parsed
        color otherwise.

        @param sPaintUri
        String to parse. Permitted to contain the optional paint
        stuff, like fallback color.

        @return true, if a paint uri was successfully parsed.
     */
    bool parsePaintUri( std::pair<const char*,const char*>& o_rPaintUri,
                        std::pair<ARGBColor,bool>&          io_rColor,
                        const char*                         sPaintUri );

    /// Parse given string for the xlink attribute
    bool parseXlinkHref( const char* xlink, std::string& data );

} // namespace svgi

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
