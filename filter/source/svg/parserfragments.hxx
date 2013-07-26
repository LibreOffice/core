/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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
