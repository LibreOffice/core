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
 *       Jan Holesovsky   <kendy@suse.cz>
 *       Fridrich Strba   <fridrich.strba@bluewin.ch>
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
#ifndef INCLUDED_UNITS_HXX
#define INCLUDED_UNITS_HXX

#include <sal/config.h>

namespace rtl{ class OUString; }
namespace svgi
{
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
    double convLength( const rtl::OUString& sValue, const State& rState, char dir );

    inline double pt2mm(double fVal) { return fVal*25.4/72.0; }
    inline double pt100thmm(double fVal) { return fVal*2540.0/72.0; }

} // namespace svgi

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
