/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "units.hxx"
#include <basegfx/range/b2drange.hxx>
#include "gfxtypes.hxx"
#include <rtl/ustring.hxx>
#include <boost/spirit/include/classic.hpp>

namespace svgi
{

double convLength( double value, SvgUnit unit, const State& rState, char dir )
{
    // convert svg unit to internal coordinates ("pixel"). Since the
    // OOo drawing layer is still largely integer-based, the initial
    // viewport transformation includes a certain scale factor
    double fRet(value);
    switch ( unit )
    {
        case SVG_LENGTH_UNIT_CM: fRet *= 72.0/2.54; break;
        case SVG_LENGTH_UNIT_IN: fRet *= 72.0; break;
        case SVG_LENGTH_UNIT_MM: fRet *= 72.0/25.4; break;
        case SVG_LENGTH_UNIT_PC: fRet *= 72.0/6.0; break;
        case SVG_LENGTH_UNIT_USER:
        case SVG_LENGTH_UNIT_PX: // no unit defaults to PX in svg,
                                 // assume display to have 72DPI
        case SVG_LENGTH_UNIT_PT: break;
        case SVG_LENGTH_UNIT_EM: fRet *= rState.mnFontSize; break;
        case SVG_LENGTH_UNIT_EX: fRet *= rState.mnFontSize / 2.0; break;
        case SVG_LENGTH_UNIT_PERCENTAGE:
        {
            double fBoxLen;
            if (rState.maViewBox.isEmpty())
            {
                basegfx::B2DRange aDefaultBox(0, 0,
                  convLength(210, SVG_LENGTH_UNIT_MM, rState, 'h'),
                  convLength(297, SVG_LENGTH_UNIT_MM, rState, 'v'));
                fBoxLen = (dir=='h' ? aDefaultBox.getWidth() :
                          (dir=='v' ? aDefaultBox.getHeight() :
                           aDefaultBox.getRange().getLength()));
            }
            else
            {
                fBoxLen = (dir=='h' ? rState.maViewBox.getWidth() :
                          (dir=='v' ? rState.maViewBox.getHeight() :
                           rState.maViewBox.getRange().getLength()));
            }

            fRet *= fBoxLen/100.0;
        }
        break;
        default: OSL_TRACE( "Unknown length type" ); break;
    }

    return fRet;
}

double convLength( const OUString& sValue, const State& rState, char dir )
{
    //FIXME: convert deprecated spirit::classic to use spirit::qi
    using namespace ::boost::spirit::classic;

    OString aUTF8 = OUStringToOString( sValue,
                                                 RTL_TEXTENCODING_UTF8 );

    double  nVal=0.0;
    SvgUnit eUnit=SVG_LENGTH_UNIT_PX;
    const bool bRes = parse(aUTF8.getStr(),
        //  Begin grammar
        (
            real_p[assign_a(nVal)]
            >> (  str_p("cm") [assign_a(eUnit,SVG_LENGTH_UNIT_CM)]
                | str_p("em") [assign_a(eUnit,SVG_LENGTH_UNIT_EM)]
                | str_p("ex") [assign_a(eUnit,SVG_LENGTH_UNIT_EX)]
                | str_p("in") [assign_a(eUnit,SVG_LENGTH_UNIT_IN)]
                | str_p("mm") [assign_a(eUnit,SVG_LENGTH_UNIT_MM)]
                | str_p("pc") [assign_a(eUnit,SVG_LENGTH_UNIT_PC)]
                | str_p("pt") [assign_a(eUnit,SVG_LENGTH_UNIT_PT)]
                | str_p("px") [assign_a(eUnit,SVG_LENGTH_UNIT_PX)]
                | str_p("%") [assign_a(eUnit,SVG_LENGTH_UNIT_PERCENTAGE)]
                | str_p("") [assign_a(eUnit,SVG_LENGTH_UNIT_USER)]
                | end_p)
        ),
        //  End grammar
        space_p).full;

    if( !bRes )
        return 0.0;

    return convLength(nVal,eUnit,rState,dir);
}

} // namespace svgi

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
