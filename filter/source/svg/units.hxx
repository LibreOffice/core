/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_FILTER_SOURCE_SVG_UNITS_HXX
#define INCLUDED_FILTER_SOURCE_SVG_UNITS_HXX

#include <sal/config.h>
#include <rtl/ustring.hxx>

namespace svgi
{
    // recommended value for this device dependent unit, see CSS2 section 4.3.2 Lengths
    // Same as in svgio
    #define F_SVG_PIXEL_PER_INCH  90.0

    struct State;
    enum SvgUnit
    {
        SVG_LENGTH_UNIT_CM,
        SVG_LENGTH_UNIT_EM,
        SVG_LENGTH_UNIT_EX,
        SVG_LENGTH_UNIT_IN,
        SVG_LENGTH_UNIT_MM,
        SVG_LENGTH_UNIT_PC,
        SVG_LENGTH_UNIT_PT,
        SVG_LENGTH_UNIT_PX,
        SVG_LENGTH_UNIT_PERCENTAGE,
        SVG_LENGTH_UNIT_USER
    };

    /** return svg_length_t in 100th's of mm
         @param fVal value to convert
         @param unit unit the value is in
         @param rState current state (needed for viewport dimensions etc.)
         @param dir direction - either 'h' or 'v' for horizonal or vertical, resp.
     */
    double convLength( double fVal, SvgUnit unit, const State& rState, char dir );

    /** return svg_length_t in 100th's of mm
         @param sValue value to convert
         @param rState current state (needed for viewport dimensions etc.)
         @param dir direction - either 'h' or 'v' for horizonal or vertical, resp.
     */
    double convLength( const OUString& sValue, const State& rState, char dir );

    inline double pt2mm(double fVal) { return fVal*25.4/72.0; }
    inline double pt100thmm(double fVal) { return fVal*2540.0/72.0; }

} // namespace svgi

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
