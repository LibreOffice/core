/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: customshapegeometry.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:18:49 $
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

#include <com/sun/star/xml/sax/FastToken.hpp>

#include "oox/helper/propertymap.hxx"
#include "oox/core/namespaces.hxx"
#include "oox/drawingml/customshapegeometry.hxx"
#include "tokens.hxx"
#ifndef _UTL_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#include <hash_map>

using ::rtl::OUString;
using ::com::sun::star::beans::NamedValue;
using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace drawingml {

enum FormularCommand
{
    FC_MULDIV = 0,
    FC_PLUSMINUS,
    FC_PLUSDIV,
    FC_IFELSE,
    FC_ABS,
    FC_AT2,
    FC_CAT2,
    FC_COS,
    FC_MAX,
    FC_MIN,
    FC_MOD,
    FC_PIN,
    FC_SAT2,
    FC_SIN,
    FC_SQRT,
    FC_TAN,
    FC_VAL,
    FC_LAST
};
struct FormularCommandNameTable
{
    const char*     pS;
    FormularCommand pE;
};
static FormularCommandNameTable pFormularCommandNameTable[] =
{
    { "*/",     FC_MULDIV },
    { "+-",     FC_PLUSMINUS },
    { "+/",     FC_PLUSDIV },
    { "ifelse", FC_IFELSE },
    { "abs",    FC_ABS },
    { "at2",    FC_AT2 },
    { "cat2",   FC_CAT2 },
    { "cos",    FC_COS },
    { "max",    FC_MAX },
    { "min",    FC_MIN },
    { "mod",    FC_MOD },
    { "pin",    FC_PIN },
    { "sat2",   FC_SAT2 },
    { "sin",    FC_SIN },
    { "sqrt",   FC_SQRT },
    { "tan",    FC_TAN },
    { "val",    FC_VAL }

};
typedef std::hash_map< rtl::OUString, FormularCommand, comphelper::UStringHash, comphelper::UStringEqual > FormulaCommandHMap;

static const FormulaCommandHMap* pCommandHashMap;

// ---------------------------------------------------------------------
// CT_GeomGuideList
class AdjustmentValueContext : public ContextHandler
{
public:
    AdjustmentValueContext( ContextHandler& rParent, CustomShapeProperties& rCustomShapeProperties );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 aElementToken, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

protected:
    CustomShapeProperties& mrCustomShapeProperties;
};

AdjustmentValueContext::AdjustmentValueContext( ContextHandler& rParent, CustomShapeProperties& rCustomShapeProperties )
: ContextHandler( rParent )
, mrCustomShapeProperties( rCustomShapeProperties )
{
}

static rtl::OUString convertToOOEquation( const rtl::OUString& rSource )
{
    if ( !pCommandHashMap )
    {
        FormulaCommandHMap* pHM = new FormulaCommandHMap();
        for( sal_Int32 i = 0; i < FC_LAST; i++ )
            (*pHM)[ OUString::createFromAscii( pFormularCommandNameTable[ i ].pS ) ] =  pFormularCommandNameTable[ i ].pE;
        pCommandHashMap = pHM;
    }

    std::vector< rtl::OUString > aTokens;
    sal_Int32 nIndex = 0;
    do
    {
        rtl::OUString aToken( rSource.getToken( 0, ' ', nIndex ) );
        if ( aToken.getLength() )
            aTokens.push_back( aToken );
    }
    while ( nIndex >= 0 );

    rtl::OUString aEquation;
    if ( aTokens.size() )
    {
        const FormulaCommandHMap::const_iterator aIter( pCommandHashMap->find( aTokens[ 0 ] ) );
        if ( aIter != pCommandHashMap->end() )
        {
            switch( aIter->second )
            {
                case FC_MULDIV :
                case FC_PLUSMINUS :
                case FC_PLUSDIV :
                case FC_IFELSE :
                case FC_ABS :
                case FC_AT2 :
                case FC_CAT2 :
                case FC_COS :
                case FC_MAX :
                case FC_MIN :
                case FC_MOD :
                case FC_PIN :
                case FC_SAT2 :
                case FC_SIN :
                case FC_SQRT :
                case FC_TAN :
                case FC_VAL :
                default :
                    break;
            }
        }
    }
    return aEquation;
}

Reference< XFastContextHandler > AdjustmentValueContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    if ( aElementToken == ( NMSP_DRAWINGML | XML_gd ) ) // CT_GeomGuide
    {
        CustomShapeGuide aGuide;
        aGuide.maName = xAttribs->getOptionalValue( XML_name );
        aGuide.maFormula = convertToOOEquation( xAttribs->getOptionalValue( XML_fmla ) );
        std::vector< CustomShapeGuide >& rAdjustmentValues( mrCustomShapeProperties.getAdjustmentValues() );
        rAdjustmentValues.push_back( aGuide );
    }
    return this;
}

// ---------------------------------------------------------------------

OUString GetShapeType( sal_Int32 nType )
{
    OUString sType;
     switch( nType )
    {
        case XML_lineInv:   // TODO
        case XML_line: {
            static const OUString sLine = CREATE_OUSTRING( "mso-spt20" );
            sType = sLine;
            } break;
        case XML_triangle: {
            static const OUString sTriangle = CREATE_OUSTRING( "isosceles-triangle" );
            sType = sTriangle;
            } break;
        case XML_rtTriangle: {
            static const OUString sRtTriangle = CREATE_OUSTRING( "right-triangle" );
            sType = sRtTriangle;
            } break;
        case XML_rect: {
            static const OUString sRectangle = CREATE_OUSTRING( "rectangle" );
            sType = sRectangle;
            } break;
        case XML_diamond: {
            static const OUString sDiamond = CREATE_OUSTRING( "diamond" );
            sType = sDiamond;
            } break;
        case XML_parallelogram: {
            static const OUString sParallelogram = CREATE_OUSTRING( "parallelogram" );
            sType = sParallelogram;
            } break;
        case XML_nonIsoscelesTrapezoid:     // TODO
        case XML_trapezoid: {
            static const OUString sTrapezoid = CREATE_OUSTRING( "trapezoid" );
            sType = sTrapezoid;
            } break;
        case XML_pentagon: {
            static const OUString sPentagon = CREATE_OUSTRING( "pentagon" );
            sType = sPentagon;
            } break;
        case XML_heptagon:                  // TODO
        case XML_hexagon: {
            static const OUString sHexagon = CREATE_OUSTRING( "hexagon" );
            sType = sHexagon;
            } break;
        case XML_decagon:                   // TODO
        case XML_dodecagon:                 // TODO
        case XML_octagon: {
            static const OUString sOctagon = CREATE_OUSTRING( "octagon" );
            sType = sOctagon;
            } break;
        case XML_star4: {
            static const OUString sStar4 = CREATE_OUSTRING( "star4" );
            sType = sStar4;
            } break;
        case XML_star6:                     // TODO
        case XML_star7:                     // TODO
        case XML_star5: {
            static const OUString sStar5 = CREATE_OUSTRING( "star5" );
            sType = sStar5;
            } break;
        case XML_star10:                    // TODO
        case XML_star12:                    // TODO
        case XML_star16:                    // TODO
        case XML_star8: {
            static const OUString sStar8 = CREATE_OUSTRING( "star8" );
            sType = sStar8;
            } break;
        case XML_star32:                    // TODO
        case XML_star24: {
            static const OUString sStar24 = CREATE_OUSTRING( "star24" );
            sType = sStar24;
            } break;
        case XML_round1Rect:                // TODO
        case XML_round2SameRect:            // TODO
        case XML_round2DiagRect:            // TODO
        case XML_snipRoundRect:             // TODO
        case XML_snip1Rect:                 // TODO
        case XML_snip2SameRect:             // TODO
        case XML_snip2DiagRect:             // TODO
        case XML_roundRect: {
            static const OUString sRoundRect = CREATE_OUSTRING( "round-rectangle" );
            sType = sRoundRect;
            } break;
        case XML_plaque: {
            static const OUString sPlaque = CREATE_OUSTRING( "mso-spt21" );
            sType = sPlaque;
            } break;
        case XML_teardrop:                  // TODO
        case XML_ellipse: {
            static const OUString sEllipse = CREATE_OUSTRING( "ellipse" );
            sType = sEllipse;
            } break;
        case XML_homePlate: {
            static const OUString sHomePlate = CREATE_OUSTRING( "pentagon-right" );
            sType = sHomePlate;
            } break;
        case XML_chevron: {
            static const OUString sChevron = CREATE_OUSTRING( "chevron" );
            sType = sChevron;
            } break;
        case XML_pieWedge:                  // TODO
        case XML_pie:                       // TODO
        case XML_blockArc: {
            static const OUString sBlockArc = CREATE_OUSTRING( "block-arc" );
            sType = sBlockArc;
            } break;
        case XML_donut: {
            static const OUString sDonut = CREATE_OUSTRING( "ring" );
            sType = sDonut;
            } break;
        case XML_noSmoking: {
            static const OUString sNoSmoking = CREATE_OUSTRING( "forbidden" );
            sType = sNoSmoking;
            } break;
        case XML_rightArrow: {
            static const OUString sRightArrow = CREATE_OUSTRING( "right-arrow" );
            sType = sRightArrow;
            } break;
        case XML_leftArrow: {
            static const OUString sLeftArrow = CREATE_OUSTRING( "left-arrow" );
            sType = sLeftArrow;
            } break;
        case XML_upArrow: {
            static const OUString sUpArrow = CREATE_OUSTRING( "up-arrow" );
            sType = sUpArrow;
            } break;
        case XML_downArrow: {
            static const OUString sDownArrow = CREATE_OUSTRING( "down-arrow" );
            sType = sDownArrow;
            } break;
        case XML_stripedRightArrow: {
            static const OUString sStripedRightArrow = CREATE_OUSTRING( "striped-right-arrow" );
            sType = sStripedRightArrow;
            } break;
        case XML_notchedRightArrow: {
            static const OUString sNotchedRightArrow = CREATE_OUSTRING( "notched-right-arrow" );
            sType = sNotchedRightArrow;
            } break;
        case XML_bentUpArrow: {
            static const OUString sBentUpArrow = CREATE_OUSTRING( "mso-spt90" );
            sType = sBentUpArrow;
            } break;
        case XML_leftRightArrow: {
            static const OUString sLeftRightArrow = CREATE_OUSTRING( "left-right-arrow" );
            sType = sLeftRightArrow;
            } break;
        case XML_upDownArrow: {
            static const OUString sUpDownArrow = CREATE_OUSTRING( "up-down-arrow" );
            sType = sUpDownArrow;
            } break;
        case XML_leftUpArrow: {
            static const OUString sLeftUpArrow = CREATE_OUSTRING( "mso-spt89" );
            sType = sLeftUpArrow;
            } break;
        case XML_leftRightUpArrow: {
            static const OUString sLeftRightUpArrow = CREATE_OUSTRING( "mso-spt182" );
            sType = sLeftRightUpArrow;
            } break;
        case XML_quadArrow: {
            static const OUString sQuadArrow = CREATE_OUSTRING( "quad-arrow" );
            sType = sQuadArrow;
            } break;
        case XML_leftArrowCallout: {
            static const OUString sLeftArrowCallout = CREATE_OUSTRING( "left-arrow-callout" );
            sType = sLeftArrowCallout;
            } break;
        case XML_rightArrowCallout: {
            static const OUString sRightArrowCallout = CREATE_OUSTRING( "right-arrow-callout" );
            sType = sRightArrowCallout;
            } break;
        case XML_upArrowCallout: {
            static const OUString sUpArrowCallout = CREATE_OUSTRING( "up-arrow-callout" );
            sType = sUpArrowCallout;
            } break;
        case XML_downArrowCallout: {
            static const OUString sDownArrowCallout = CREATE_OUSTRING( "down-arrow-callout" );
            sType = sDownArrowCallout;
            } break;
        case XML_leftRightArrowCallout: {
            static const OUString sLeftRightArrowCallout = CREATE_OUSTRING( "left-right-arrow-callout" );
            sType = sLeftRightArrowCallout;
            } break;
        case XML_upDownArrowCallout: {
            static const OUString sUpDownArrowCallout = CREATE_OUSTRING( "up-down-arrow-callout" );
            sType = sUpDownArrowCallout;
            } break;
        case XML_quadArrowCallout: {
            static const OUString sQuadArrowCallout = CREATE_OUSTRING( "quad-arrow-callout" );
            sType = sQuadArrowCallout;
            } break;
        case XML_bentArrow: {
            static const OUString sBentArrow = CREATE_OUSTRING( "mso-spt91" );
            sType = sBentArrow;
            } break;
        case XML_uturnArrow: {
            static const OUString sUTurnArrow = CREATE_OUSTRING( "mso-spt101" );
            sType = sUTurnArrow;
            } break;
        case XML_leftCircularArrow:         // TODO
        case XML_leftRightCircularArrow:    // TODO
        case XML_circularArrow: {
            static const OUString sCircularArrow = CREATE_OUSTRING( "circular-arrow" );
            sType = sCircularArrow;
            } break;
        case XML_curvedRightArrow: {
            static const OUString sCurvedRightArrow = CREATE_OUSTRING( "mso-spt102" );
            sType = sCurvedRightArrow;
            } break;
        case XML_curvedLeftArrow: {
            static const OUString sCurvedLeftArrow = CREATE_OUSTRING( "mso-spt103" );
            sType = sCurvedLeftArrow;
            } break;
        case XML_curvedUpArrow: {
            static const OUString sCurvedUpArrow = CREATE_OUSTRING( "mso-spt104" );
            sType = sCurvedUpArrow;
            } break;
        case XML_swooshArrow:               // TODO
        case XML_curvedDownArrow: {
            static const OUString sCurvedDownArrow = CREATE_OUSTRING( "mso-spt105" );
            sType = sCurvedDownArrow;
            } break;
        case XML_cube: {
            static const OUString sCube = CREATE_OUSTRING( "cube" );
            sType = sCube;
            } break;
        case XML_can: {
            static const OUString sCan = CREATE_OUSTRING( "can" );
            sType = sCan;
            } break;
        case XML_lightningBolt: {
            static const OUString sLightningBolt = CREATE_OUSTRING( "lightning" );
            sType = sLightningBolt;
            } break;
        case XML_heart: {
            static const OUString sHeart = CREATE_OUSTRING( "heart" );
            sType = sHeart;
            } break;
        case XML_sun: {
            static const OUString sSun = CREATE_OUSTRING( "sun" );
            sType = sSun;
            } break;
        case XML_moon: {
            static const OUString sMoon = CREATE_OUSTRING( "moon" );
            sType = sMoon;
            } break;
        case XML_smileyFace: {
            static const OUString sSmileyFace = CREATE_OUSTRING( "smiley" );
            sType = sSmileyFace;
            } break;
        case XML_irregularSeal1: {
            static const OUString sIrregularSeal1 = CREATE_OUSTRING( "mso-spt71" );
            sType = sIrregularSeal1;
            } break;
        case XML_irregularSeal2: {
            static const OUString sIrregularSeal2 = CREATE_OUSTRING( "bang" );
            sType = sIrregularSeal2;
            } break;
        case XML_foldedCorner: {
            static const OUString sFoldedCorner = CREATE_OUSTRING( "paper" );
            sType = sFoldedCorner;
            } break;
        case XML_bevel: {
            static const OUString sBevel = CREATE_OUSTRING( "quad-bevel" );
            sType = sBevel;
            } break;
        case XML_halfFrame:                 // TODO
        case XML_corner:                    // TODO
        case XML_diagStripe:                // TODO
        case XML_chord:                     // TODO
        case XML_frame: {
            static const OUString sFrame = CREATE_OUSTRING( "mso-spt75" );
            sType = sFrame;
            } break;
        case XML_arc: {
            static const OUString sArc = CREATE_OUSTRING( "mso-spt19" );
            sType = sArc;
            } break;
        case XML_leftBracket: {
            static const OUString sLeftBracket = CREATE_OUSTRING( "left-bracket" );
            sType = sLeftBracket;
            } break;
        case XML_rightBracket: {
            static const OUString sRightBracket = CREATE_OUSTRING( "right-bracket" );
            sType = sRightBracket;
            } break;
        case XML_leftBrace: {
            static const OUString sLeftBrace = CREATE_OUSTRING( "left-brace" );
            sType = sLeftBrace;
            } break;
        case XML_rightBrace: {
            static const OUString sRightBrace = CREATE_OUSTRING( "right-brace" );
            sType = sRightBrace;
            } break;
        case XML_bracketPair: {
            static const OUString sBracketPair = CREATE_OUSTRING( "bracket-pair" );
            sType = sBracketPair;
            } break;
        case XML_bracePair: {
            static const OUString sBracePair = CREATE_OUSTRING( "brace-pair" );
            sType = sBracePair;
            } break;
        case XML_straightConnector1: {
            static const OUString sStraightConnector1 = CREATE_OUSTRING( "mso-spt32" );
            sType = sStraightConnector1;
            } break;
        case XML_bentConnector2: {
            static const OUString sBentConnector2 = CREATE_OUSTRING( "mso-spt33" );
            sType = sBentConnector2;
            } break;
        case XML_bentConnector3: {
            static const OUString sBentConnector3 = CREATE_OUSTRING( "mso-spt34" );
            sType = sBentConnector3;
            } break;
        case XML_bentConnector4: {
            static const OUString sBentConnector4 = CREATE_OUSTRING( "mso-spt35" );
            sType = sBentConnector4;
            } break;
        case XML_bentConnector5: {
            static const OUString sBentConnector5 = CREATE_OUSTRING( "mso-spt36" );
            sType = sBentConnector5;
            } break;
        case XML_curvedConnector2: {
            static const OUString sCurvedConnector2 = CREATE_OUSTRING( "mso-spt37" );
            sType = sCurvedConnector2;
            } break;
        case XML_curvedConnector3: {
            static const OUString sCurvedConnector3 = CREATE_OUSTRING( "mso-spt38" );
            sType = sCurvedConnector3;
            } break;
        case XML_curvedConnector4: {
            static const OUString sCurvedConnector4 = CREATE_OUSTRING( "mso-spt39" );
            sType = sCurvedConnector4;
            } break;
        case XML_curvedConnector5: {
            static const OUString sCurvedConnector5 = CREATE_OUSTRING( "mso-spt40" );
            sType = sCurvedConnector5;
            } break;
        case XML_callout1: {
            static const OUString sCallout1 = CREATE_OUSTRING( "mso-spt41" );
            sType = sCallout1;
            } break;
        case XML_callout2: {
            static const OUString sCallout2 = CREATE_OUSTRING( "mso-spt42" );
            sType = sCallout2;
            } break;
        case XML_callout3: {
            static const OUString sCallout3 = CREATE_OUSTRING( "mso-spt43" );
            sType = sCallout3;
            } break;
        case XML_accentCallout1: {
            static const OUString sAccentCallout1 = CREATE_OUSTRING( "mso-spt44" );
            sType = sAccentCallout1;
            } break;
        case XML_accentCallout2: {
            static const OUString sAccentCallout2 = CREATE_OUSTRING( "mso-spt45" );
            sType = sAccentCallout2;
            } break;
        case XML_accentCallout3: {
            static const OUString sAccentCallout3 = CREATE_OUSTRING( "mso-spt46" );
            sType = sAccentCallout3;
            } break;
        case XML_borderCallout1: {
            static const OUString sBorderCallout1 = CREATE_OUSTRING( "line-callout-1" );
            sType = sBorderCallout1;
            } break;
        case XML_borderCallout2: {
            static const OUString sBorderCallout2 = CREATE_OUSTRING( "line-callout-2" );
            sType = sBorderCallout2;
            } break;
        case XML_borderCallout3: {
            static const OUString sBorderCallout3 = CREATE_OUSTRING( "mso-spt49" );
            sType = sBorderCallout3;
            } break;
        case XML_accentBorderCallout1: {
            static const OUString sAccentBorderCallout1 = CREATE_OUSTRING( "mso-spt50" );
            sType = sAccentBorderCallout1;
            } break;
        case XML_accentBorderCallout2: {
            static const OUString sAccentBorderCallout2 = CREATE_OUSTRING( "mso-spt51" );
            sType = sAccentBorderCallout2;
            } break;
        case XML_accentBorderCallout3: {
            static const OUString sAccentBorderCallout3 = CREATE_OUSTRING( "mso-spt52" );
            sType = sAccentBorderCallout3;
            } break;
        case XML_wedgeRectCallout: {
            static const OUString sWedgeRectCallout = CREATE_OUSTRING( "rectangular-callout" );
            sType = sWedgeRectCallout;
            } break;
        case XML_wedgeRoundRectCallout: {
            static const OUString sWedgeRoundRectCallout = CREATE_OUSTRING( "round-rectangular-callout" );
            sType = sWedgeRoundRectCallout;
            } break;
        case XML_wedgeEllipseCallout: {
            static const OUString sWedgeEllipseCallout = CREATE_OUSTRING( "round-callout" );
            sType = sWedgeEllipseCallout;
            } break;
        case XML_cloud:                     // TODO
        case XML_cloudCallout: {
            static const OUString sCloudCallout = CREATE_OUSTRING( "cloud-callout" );
            sType = sCloudCallout;
            } break;
        case XML_ribbon: {
            static const OUString sRibbon = CREATE_OUSTRING( "mso-spt53" );
            sType = sRibbon;
            } break;
        case XML_ribbon2: {
            static const OUString sRibbon2 = CREATE_OUSTRING( "mso-spt54" );
            sType = sRibbon2;
            } break;
        case XML_ellipseRibbon: {
            static const OUString sEllipseRibbon = CREATE_OUSTRING( "mso-spt107" );
            sType = sEllipseRibbon;
            } break;
        case XML_leftRightRibbon:           // TODO
        case XML_ellipseRibbon2: {
            static const OUString sEllipseRibbon2 = CREATE_OUSTRING( "mso-spt108" );
            sType = sEllipseRibbon2;
            } break;
        case XML_verticalScroll: {
            static const OUString sVerticalScroll = CREATE_OUSTRING( "vertical-scroll" );
            sType = sVerticalScroll;
            } break;
        case XML_horizontalScroll: {
            static const OUString sHorizontalScroll = CREATE_OUSTRING( "horizontal-scroll" );
            sType = sHorizontalScroll;
            } break;
        case XML_wave: {
            static const OUString sWave = CREATE_OUSTRING( "mso-spt64" );
            sType = sWave;
            } break;
        case XML_doubleWave: {
            static const OUString sDoubleWave = CREATE_OUSTRING( "mso-spt188" );
            sType = sDoubleWave;
            } break;
        case XML_plus: {
            static const OUString sPlus = CREATE_OUSTRING( "cross" );
            sType = sPlus;
            } break;
        case XML_flowChartProcess: {
            static const OUString sFlowChartProcess = CREATE_OUSTRING( "flowchart-process" );
            sType = sFlowChartProcess;
            } break;
        case XML_flowChartDecision: {
            static const OUString sFlowChartDecision = CREATE_OUSTRING( "flowchart-decision" );
            sType = sFlowChartDecision;
            } break;
        case XML_flowChartInputOutput: {
            static const OUString sFlowChartInputOutput = CREATE_OUSTRING( "flowchart-data" );
            sType = sFlowChartInputOutput;
            } break;
        case XML_flowChartPredefinedProcess: {
            static const OUString sFlowChartPredefinedProcess = CREATE_OUSTRING( "flowchart-predefined-process" );
            sType = sFlowChartPredefinedProcess;
            } break;
        case XML_flowChartInternalStorage: {
            static const OUString sFlowChartInternalStorage = CREATE_OUSTRING( "flowchart-internal-storage" );
            sType = sFlowChartInternalStorage;
            } break;
        case XML_flowChartDocument: {
            static const OUString sFlowChartDocument = CREATE_OUSTRING( "flowchart-document" );
            sType = sFlowChartDocument;
            } break;
        case XML_flowChartMultidocument: {
            static const OUString sFlowChartMultidocument = CREATE_OUSTRING( "flowchart-multidocument" );
            sType = sFlowChartMultidocument;
            } break;
        case XML_flowChartTerminator: {
            static const OUString sFlowChartTerminator = CREATE_OUSTRING( "flowchart-terminator" );
            sType = sFlowChartTerminator;
            } break;
        case XML_flowChartPreparation : {
            static const OUString sFlowChartPreparation = CREATE_OUSTRING( "flowchart-preparation" );
            sType = sFlowChartPreparation;
            } break;
        case XML_flowChartManualInput: {
            static const OUString sFlowChartManualInput = CREATE_OUSTRING( "flowchart-manual-input" );
            sType = sFlowChartManualInput;
            } break;
        case XML_flowChartManualOperation: {
            static const OUString sFlowChartManualOperation = CREATE_OUSTRING( "flowchart-manual-operation" );
            sType = sFlowChartManualOperation;
            } break;
        case XML_flowChartConnector: {
            static const OUString sFlowChartConnector = CREATE_OUSTRING( "flowchart-connector" );
            sType = sFlowChartConnector;
            } break;
        case XML_flowChartPunchedCard: {
            static const OUString sFlowChartPunchedCard = CREATE_OUSTRING( "flowchart-card" );
            sType = sFlowChartPunchedCard;
            } break;
        case XML_flowChartPunchedTape: {
            static const OUString sFlowChartPunchedTape = CREATE_OUSTRING( "flowchart-punched-tape" );
            sType = sFlowChartPunchedTape;
            } break;
        case XML_flowChartSummingJunction: {
            static const OUString sFlowChartSummingJunction = CREATE_OUSTRING( "flowchart-summing-junction" );
            sType = sFlowChartSummingJunction;
            } break;
        case XML_flowChartOr: {
            static const OUString sFlowChartOr = CREATE_OUSTRING( "flowchart-or" );
            sType = sFlowChartOr;
            } break;
        case XML_flowChartCollate: {
            static const OUString sFlowChartCollate = CREATE_OUSTRING( "flowchart-collate" );
            sType = sFlowChartCollate;
            } break;
        case XML_flowChartSort: {
            static const OUString sFlowChartSort = CREATE_OUSTRING( "flowchart-sort" );
            sType = sFlowChartSort;
            } break;
        case XML_flowChartExtract: {
            static const OUString sFlowChartExtract = CREATE_OUSTRING( "flowchart-extract" );
            sType = sFlowChartExtract;
            } break;
        case XML_flowChartMerge: {
            static const OUString sFlowChartMerge = CREATE_OUSTRING( "flowchart-merge" );
            sType = sFlowChartMerge;
            } break;
        case XML_flowChartOfflineStorage: {
            static const OUString sFlowChartOfflineStorage = CREATE_OUSTRING( "mso-spt129" );
            sType = sFlowChartOfflineStorage;
            } break;
        case XML_flowChartOnlineStorage: {
            static const OUString sFlowChartOnlineStorage = CREATE_OUSTRING( "flowchart-stored-data" );
            sType = sFlowChartOnlineStorage;
            } break;
        case XML_flowChartMagneticTape: {
            static const OUString sFlowChartMagneticTape = CREATE_OUSTRING( "flowchart-sequential-access" );
            sType = sFlowChartMagneticTape;
            } break;
        case XML_flowChartMagneticDisk: {
            static const OUString sFlowChartMagneticDisk = CREATE_OUSTRING( "flowchart-magnetic-disk" );
            sType = sFlowChartMagneticDisk;
            } break;
        case XML_flowChartMagneticDrum: {
            static const OUString sFlowChartMagneticDrum = CREATE_OUSTRING( "flowchart-direct-access-storage" );
            sType = sFlowChartMagneticDrum;
            } break;
        case XML_flowChartDisplay: {
            static const OUString sFlowChartDisplay = CREATE_OUSTRING( "flowchart-display" );
            sType = sFlowChartDisplay;
            } break;
        case XML_flowChartDelay: {
            static const OUString sFlowChartDelay = CREATE_OUSTRING( "flowchart-delay" );
            sType = sFlowChartDelay;
            } break;
        case XML_flowChartAlternateProcess: {
            static const OUString sFlowChartAlternateProcess = CREATE_OUSTRING( "flowchart-alternate-process" );
            sType = sFlowChartAlternateProcess;
            } break;
        case XML_flowChartOffpageConnector: {
            static const OUString sFlowChartOffpageConnector = CREATE_OUSTRING( "flowchart-off-page-connector" );
            sType = sFlowChartOffpageConnector;
            } break;
        case XML_actionButtonBlank: {
            static const OUString sActionButtonBlank = CREATE_OUSTRING( "mso-spt189" );
            sType = sActionButtonBlank;
            } break;
        case XML_actionButtonHome: {
            static const OUString sActionButtonHome = CREATE_OUSTRING( "mso-spt190" );
            sType = sActionButtonHome;
            } break;
        case XML_actionButtonHelp: {
            static const OUString sActionButtonHelp = CREATE_OUSTRING( "mso-spt191" );
            sType = sActionButtonHelp;
            } break;
        case XML_actionButtonInformation: {
            static const OUString sActionButtonInformation = CREATE_OUSTRING( "mso-spt192" );
            sType = sActionButtonInformation;
            } break;
        case XML_actionButtonForwardNext: {
            static const OUString sActionButtonForwardNext = CREATE_OUSTRING( "mso-spt193" );
            sType = sActionButtonForwardNext;
            } break;
        case XML_actionButtonBackPrevious: {
            static const OUString sActionButtonBackPrevious = CREATE_OUSTRING( "mso-spt194" );
            sType = sActionButtonBackPrevious;
            } break;
        case XML_actionButtonEnd: {
            static const OUString sActionButtonEnd = CREATE_OUSTRING( "mso-spt195" );
            sType = sActionButtonEnd;
            } break;
        case XML_actionButtonBeginning: {
            static const OUString sActionButtonBeginning = CREATE_OUSTRING( "mso-spt196" );
            sType = sActionButtonBeginning;
            } break;
        case XML_actionButtonReturn: {
            static const OUString sActionButtonReturn = CREATE_OUSTRING( "mso-spt197" );
            sType = sActionButtonReturn;
            } break;
        case XML_actionButtonDocument: {
            static const OUString sActionButtonDocument = CREATE_OUSTRING( "mso-spt198" );
            sType = sActionButtonDocument;
            } break;
        case XML_actionButtonSound: {
            static const OUString sActionButtonSound = CREATE_OUSTRING( "mso-spt199" );
            sType = sActionButtonSound;
            } break;
        case XML_actionButtonMovie: {
            static const OUString sActionButtonMovie = CREATE_OUSTRING( "mso-spt200" );
            sType = sActionButtonMovie;
            } break;
        case XML_gear6:                     // TODO
        case XML_gear9:                     // TODO
        case XML_funnel:                    // TODO
        case XML_mathPlus:                  // TODO
        case XML_mathMinus:                 // TODO
        case XML_mathMultiply:              // TODO
        case XML_mathDivide:                // TODO
        case XML_mathEqual:                 // TODO
        case XML_mathNotEqual:              // TODO
        case XML_cornerTabs:                // TODO
        case XML_squareTabs:                // TODO
        case XML_plaqueTabs:                // TODO
        case XML_chartX:                    // TODO
        case XML_chartStar:                 // TODO
        case XML_chartPlus: {               // TODO
            static const OUString sRectangle = CREATE_OUSTRING( "rectangle" );
            sType = sRectangle;
            } break;
        default:
            break;
    }
    return sType;
}

static OUString GetTextShapeType( sal_Int32 nType )
{
    OUString sType;
    switch( nType )
    {
        case XML_textNoShape:               // TODO
        case XML_textPlain: {
            static const OUString sTextPlain = CREATE_OUSTRING( "fontwork-plain-text" );
            sType = sTextPlain;
            } break;
        case XML_textStop: {
            static const OUString sTextStop = CREATE_OUSTRING( "fontwork-stop" );
            sType = sTextStop;
            } break;
        case XML_textTriangle: {
            static const OUString sTextTriangle = CREATE_OUSTRING( "fontwork-triangle-up" );
            sType = sTextTriangle;
            } break;
        case XML_textTriangleInverted: {
            static const OUString sTextTriangleInverted = CREATE_OUSTRING( "fontwork-triangle-down" );
            sType = sTextTriangleInverted;
            } break;
        case XML_textChevron: {
            static const OUString sTextChevron = CREATE_OUSTRING( "fontwork-chevron-up" );
            sType = sTextChevron;
            } break;
        case XML_textChevronInverted: {
            static const OUString sTextChevronInverted = CREATE_OUSTRING( "fontwork-chevron-down" );
            sType = sTextChevronInverted;
            } break;
        case XML_textRingInside: {
            static const OUString sTextRingInside = CREATE_OUSTRING( "mso-spt142" );
            sType = sTextRingInside;
            } break;
        case XML_textRingOutside: {
            static const OUString sTextRingOutside = CREATE_OUSTRING( "mso-spt143" );
            sType = sTextRingOutside;
            } break;
        case XML_textArchUp: {
            static const OUString sTextArchUp = CREATE_OUSTRING( "fontwork-arch-up-curve" );
            sType = sTextArchUp;
            } break;
        case XML_textArchDown: {
            static const OUString sTextArchDown = CREATE_OUSTRING( "fontwork-arch-down-curve" );
            sType = sTextArchDown;
            } break;
        case XML_textCircle: {
            static const OUString sTextCircle = CREATE_OUSTRING( "fontwork-circle-curve" );
            sType = sTextCircle;
            } break;
        case XML_textButton: {
            static const OUString sTextButton = CREATE_OUSTRING( "fontwork-open-circle-curve" );
            sType = sTextButton;
            } break;
        case XML_textArchUpPour: {
            static const OUString sTextArchUpPour = CREATE_OUSTRING( "fontwork-arch-up-pour" );
            sType = sTextArchUpPour;
            } break;
        case XML_textArchDownPour: {
            static const OUString sTextArchDownPour = CREATE_OUSTRING( "fontwork-arch-down-pour" );
            sType = sTextArchDownPour;
            } break;
        case XML_textCirclePour: {
            static const OUString sTextCirclePour = CREATE_OUSTRING( "fontwork-circle-pour" );
            sType = sTextCirclePour;
            } break;
        case XML_textButtonPour: {
            static const OUString sTextButtonPour = CREATE_OUSTRING( "fontwork-open-circle-pour" );
            sType = sTextButtonPour;
            } break;
        case XML_textCurveUp: {
            static const OUString sTextCurveUp = CREATE_OUSTRING( "fontwork-curve-up" );
            sType = sTextCurveUp;
            } break;
        case XML_textCurveDown: {
            static const OUString sTextCurveDown = CREATE_OUSTRING( "fontwork-curve-down" );
            sType = sTextCurveDown;
            } break;
        case XML_textCanUp: {
            static const OUString sTextCanUp = CREATE_OUSTRING( "mso-spt174" );
            sType = sTextCanUp;
            } break;
        case XML_textCanDown: {
            static const OUString sTextCanDown = CREATE_OUSTRING( "mso-spt175" );
            sType = sTextCanDown;
            } break;
        case XML_textWave1: {
            static const OUString sTextWave1 = CREATE_OUSTRING( "fontwork-wave" );
            sType = sTextWave1;
            } break;
        case XML_textWave2: {
            static const OUString sTextWave2 = CREATE_OUSTRING( "mso-spt157" );
            sType = sTextWave2;
            } break;
        case XML_textDoubleWave1: {
            static const OUString sTextDoubleWave1 = CREATE_OUSTRING( "mso-spt158" );
            sType = sTextDoubleWave1;
            } break;
        case XML_textWave4: {
            static const OUString sTextWave4 = CREATE_OUSTRING( "mso-spt159" );
            sType = sTextWave4;
            } break;
        case XML_textInflate: {
            static const OUString sTextInflate = CREATE_OUSTRING( "fontwork-inflate" );
            sType = sTextInflate;
            } break;
        case XML_textDeflate: {
            static const OUString sTextDeflate = CREATE_OUSTRING( "mso-spt161" );
            sType = sTextDeflate;
            } break;
        case XML_textInflateBottom: {
            static const OUString sTextInflateBottom = CREATE_OUSTRING( "mso-spt162" );
            sType = sTextInflateBottom;
            } break;
        case XML_textDeflateBottom: {
            static const OUString sTextDeflateBottom = CREATE_OUSTRING( "mso-spt163" );
            sType = sTextDeflateBottom;
            } break;
        case XML_textInflateTop: {
            static const OUString sTextInflateTop = CREATE_OUSTRING( "mso-spt164" );
            sType = sTextInflateTop;
            } break;
        case XML_textDeflateTop: {
            static const OUString sTextDeflateTop = CREATE_OUSTRING( "mso-spt165" );
            sType = sTextDeflateTop;
            } break;
        case XML_textDeflateInflate: {
            static const OUString sTextDeflateInflate = CREATE_OUSTRING( "mso-spt166" );
            sType = sTextDeflateInflate;
            } break;
        case XML_textDeflateInflateDeflate: {
            static const OUString sTextDeflateInflateDeflate = CREATE_OUSTRING( "mso-spt167" );
            sType = sTextDeflateInflateDeflate;
            } break;
        case XML_textFadeRight: {
            static const OUString sTextFadeRight = CREATE_OUSTRING( "fontwork-fade-right" );
            sType = sTextFadeRight;
            } break;
        case XML_textFadeLeft: {
            static const OUString sTextFadeLeft = CREATE_OUSTRING( "fontwork-fade-left" );
            sType = sTextFadeLeft;
            } break;
        case XML_textFadeUp: {
            static const OUString sTextFadeUp = CREATE_OUSTRING( "fontwork-fade-up" );
            sType = sTextFadeUp;
            } break;
        case XML_textFadeDown: {
            static const OUString sTextFadeDown = CREATE_OUSTRING( "fontwork-fade-down" );
            sType = sTextFadeDown;
            } break;
        case XML_textSlantUp: {
            static const OUString sTextSlantUp = CREATE_OUSTRING( "fontwork-slant-up" );
            sType = sTextSlantUp;
            } break;
        case XML_textSlantDown: {
            static const OUString sTextSlantDown = CREATE_OUSTRING( "fontwork-slant-down" );
            sType = sTextSlantDown;
            } break;
        case XML_textCascadeUp: {
            static const OUString sTextCascadeUp = CREATE_OUSTRING( "fontwork-fade-up-and-right" );
            sType = sTextCascadeUp;
            } break;
        case XML_textCascadeDown: {
            static const OUString sTextCascadeDown = CREATE_OUSTRING( "fontwork-fade-up-and-left" );
            sType = sTextCascadeDown;
            } break;
        default:
        break;
    }
    return sType;
}

// ---------------------------------------------------------------------
// CT_CustomGeometry2D
CustomShapeGeometryContext::CustomShapeGeometryContext( ContextHandler& rParent, const Reference< XFastAttributeList >& /* xAttribs */, CustomShapeProperties& rCustomShapeProperties )
: ContextHandler( rParent )
, mrCustomShapeProperties( rCustomShapeProperties )
{
}

Reference< XFastContextHandler > CustomShapeGeometryContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& ) throw (SAXException, RuntimeException)
{
    switch( aElementToken )
    {
    // todo
    case NMSP_DRAWINGML|XML_avLst:      // CT_GeomGuideList adjust value list
    case NMSP_DRAWINGML|XML_gdLst:      // CT_GeomGuideList guide list
    case NMSP_DRAWINGML|XML_ahLst:      // CT_AdjustHandleList adjust handle list
    case NMSP_DRAWINGML|XML_cxnLst: // CT_ConnectionSiteList connection site list
    case NMSP_DRAWINGML|XML_rect:   // CT_GeomRectList geometry rect list
    case NMSP_DRAWINGML|XML_pathLst:    // CT_Path2DList 2d path list
        break;
    }

    Reference< XFastContextHandler > xEmpty;
    return xEmpty;
}

// ---------------------------------------------------------------------
// CT_PresetGeometry2D
PresetShapeGeometryContext::PresetShapeGeometryContext( ContextHandler& rParent, const Reference< XFastAttributeList >& xAttribs, CustomShapeProperties& rCustomShapeProperties )
: ContextHandler( rParent )
, mrCustomShapeProperties( rCustomShapeProperties )
{
    OUString sShapeType;
    sal_Int32 nShapeType = xAttribs->getOptionalValueToken( XML_prst, FastToken::DONTKNOW );
    if ( nShapeType != FastToken::DONTKNOW )
        sShapeType = GetShapeType( nShapeType );
    OSL_ENSURE( sShapeType.getLength(), "oox::drawingml::CustomShapeCustomGeometryContext::CustomShapeCustomGeometryContext(), unknown shape type" );
    mrCustomShapeProperties.setShapePresetType( sShapeType );
}

Reference< XFastContextHandler > PresetShapeGeometryContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& ) throw (SAXException, RuntimeException)
{
    if ( aElementToken == ( NMSP_DRAWINGML | XML_avLst ) )
        return new AdjustmentValueContext( *this, mrCustomShapeProperties );
    else
        return this;
}

// ---------------------------------------------------------------------
// CT_PresetTextShape
PresetTextShapeContext::PresetTextShapeContext( ContextHandler& rParent, const Reference< XFastAttributeList >& xAttribs, CustomShapeProperties& rCustomShapeProperties )
: ContextHandler( rParent )
, mrCustomShapeProperties( rCustomShapeProperties )
{
    OUString sShapeType;
    sal_Int32 nShapeType = xAttribs->getOptionalValueToken( XML_prst, FastToken::DONTKNOW );
    if ( nShapeType != FastToken::DONTKNOW )
        sShapeType = GetTextShapeType( nShapeType );
    OSL_ENSURE( sShapeType.getLength(), "oox::drawingml::CustomShapeCustomGeometryContext::CustomShapeCustomGeometryContext(), unknown shape type" );
    mrCustomShapeProperties.setShapePresetType( sShapeType );
}

Reference< XFastContextHandler > PresetTextShapeContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& ) throw (SAXException, RuntimeException)
{
    switch( aElementToken )
    {
    // todo
    case NMSP_DRAWINGML|XML_avLst:      // CT_GeomGuideList adjust value list
    case NMSP_DRAWINGML|XML_gdLst:      // CT_GeomGuideList guide list
    case NMSP_DRAWINGML|XML_ahLst:      // CT_AdjustHandleList adjust handle list
    case NMSP_DRAWINGML|XML_cxnLst: // CT_ConnectionSiteList connection site list
    case NMSP_DRAWINGML|XML_rect:   // CT_GeomRectList geometry rect list
    case NMSP_DRAWINGML|XML_pathLst:    // CT_Path2DList 2d path list
        break;
    }

    Reference< XFastContextHandler > xEmpty;
    return xEmpty;
}

} }
