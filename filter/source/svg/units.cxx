/*************************************************************************
 *
 *    OpenOffice.org - a multi-platform office productivity suite
 *
 *    Author:
 *      Jan Holesovsky   <kendy@suse.cz>
 *      Fridrich Strba   <fridrich.strba@bluewin.ch>
 *      Thorsten Behrens <tbehrens@novell.com>
 *
 *      Copyright (C) 2008, Novell Inc.
 *
 *   The Contents of this file are made available subject to
 *   the terms of GNU Lesser General Public License Version 3.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"

#include "units.hxx"
#include "gfxtypes.hxx"
#include "spirit_supplements.hxx"

#include <string.h>
#include <rtl/ustring.hxx>

#include <boost/bind.hpp>
// workaround. spirit uses INT_MAX.
#include <limits.h>
#include <boost/spirit.hpp>


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

double convLength( const rtl::OUString& sValue, const State& rState, char dir )
{
    using namespace ::boost::spirit;

    rtl::OString aUTF8 = rtl::OUStringToOString( sValue,
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
