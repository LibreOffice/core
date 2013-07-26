/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "parserfragments.hxx"
#include "spirit_supplements.hxx"
#include "gfxtypes.hxx"

#include <basegfx/tools/canvastools.hxx>
#include <com/sun/star/geometry/AffineMatrix2D.hpp>

#include <string.h>
#include <limits.h>
#include <boost/bind.hpp>
#include <boost/spirit/include/classic.hpp>
#include <boost/spirit/include/classic_while.hpp>
#include <numeric>
#include <algorithm>

#include "units.hxx"
#include "tokenmap.hxx"
#include "sal/log.hxx"

using namespace ::com::sun::star;

namespace svgi
{

inline sal_uInt8 hex2int( char val )
{
    return val <= '9' ? val-'0' : (val < 'a' ? val+10-'A' : val+10-'a');
}

void setFourBitColor( double& rChannel, char nChar )
{
    const sal_uInt8 nVal(hex2int(nChar));
    OSL_TRACE( "setFourBitCOlor %d color", nVal );
    rChannel = (nVal*16+nVal)/255.0;
}

void setEightBitColor( double& rChannel, const char* pStart, const char* )
{
    const sal_uInt8 nVal0(hex2int(pStart[0]));
    const sal_uInt8 nVal1(hex2int(pStart[1]));
    OSL_TRACE( "setEightbitCOlor %d, %d color", nVal0, nVal1 );
    rChannel = (nVal0*16+nVal1)/255.0;
}

void setIntColor( double& rChannel, sal_uInt8 nVal )
{
    OSL_TRACE( "setIntColor %d color", nVal );
    rChannel = nVal/255.0;
}

void setPercentColor( double& rChannel, double nVal )
{
    rChannel = nVal/100.0;
    SAL_INFO("svg", "setPercentColor " << nVal << " " << rChannel);
}

void calcRotation(std::vector<geometry::AffineMatrix2D>& rTransforms,
                  geometry::AffineMatrix2D&              rCurrTransform,
                  double                                 fRotationAngle)
{
    ::basegfx::B2DHomMatrix aCurr;
    aCurr.translate(-rCurrTransform.m02,-rCurrTransform.m12);
    aCurr.rotate(fRotationAngle*M_PI/180);
    aCurr.translate(rCurrTransform.m02,rCurrTransform.m12);

    OSL_TRACE("calcRotation - fRotationAngle - %f", fRotationAngle);
    rTransforms.push_back(
        basegfx::unotools::affineMatrixFromHomMatrix(
            rCurrTransform,
            aCurr));
}

void calcSkewX(std::vector<geometry::AffineMatrix2D>& rTransforms,
               double                                 fSkewAngle)
{
    geometry::AffineMatrix2D aMat(1.0,tan(fSkewAngle*M_PI/180),0.0,
                                  0.0,1.0,0.0);
    rTransforms.push_back(aMat);
}

void calcSkewY(std::vector<geometry::AffineMatrix2D>& rTransforms,
               double                                 fSkewAngle)
{
    geometry::AffineMatrix2D aMat(1.0,0.0,0.0,
                                  tan(fSkewAngle*M_PI/180),1.0,0.0);
    rTransforms.push_back(aMat);
}

void assign_twice(double& r_oVal1, double& r_oVal2, const double& rInVal )
{
    r_oVal1 = r_oVal2 = rInVal;
}

geometry::AffineMatrix2D multiplyMatrix( const geometry::AffineMatrix2D& rLHS,
                                         const geometry::AffineMatrix2D& rRHS )
{
    basegfx::B2DHomMatrix aLHS;
    basegfx::B2DHomMatrix aRHS;

    basegfx::unotools::homMatrixFromAffineMatrix(aLHS,rLHS);
    basegfx::unotools::homMatrixFromAffineMatrix(aRHS,rRHS);

    aRHS*=aLHS;

    geometry::AffineMatrix2D aRet;
    return basegfx::unotools::affineMatrixFromHomMatrix(aRet,aRHS);
}

namespace
{
    struct ColorGrammar : public ::boost::spirit::classic::grammar< ColorGrammar >
    {
    public:
        ARGBColor& m_rColor;
        explicit ColorGrammar( ARGBColor& rColor ) : m_rColor(rColor) {}
        template< typename ScannerT >
        struct definition
        {
            ::boost::spirit::classic::rule< ScannerT > colorExpression;
            definition( const ColorGrammar& self )
            {
                using namespace ::boost::spirit::classic;

                int_parser<sal_uInt8,10,1,3> byte_p;
                colorExpression =
                    (
                        // the #rrggbb form
                        ('#' >> (xdigit_p >> xdigit_p)[boost::bind(&setEightBitColor,
                                                                   boost::ref(self.m_rColor.r),_1,_2)]
                             >> (xdigit_p >> xdigit_p)[boost::bind(&setEightBitColor,
                                                                   boost::ref(self.m_rColor.g),_1,_2)]
                             >> (xdigit_p >> xdigit_p)[boost::bind(&setEightBitColor,
                                                                   boost::ref(self.m_rColor.b),_1,_2)])
                        |
                        // the #rgb form
                        ('#' >> xdigit_p[boost::bind(&setFourBitColor,
                                                     boost::ref(self.m_rColor.r),_1)]
                             >> xdigit_p[boost::bind(&setFourBitColor,
                                                     boost::ref(self.m_rColor.g),_1)]
                             >> xdigit_p[boost::bind(&setFourBitColor,
                                                     boost::ref(self.m_rColor.b),_1)])
                        |
                        // rgb() form
                        (str_p("rgb")
                            >> '(' >>
                            (
                                // rgb(int,int,int)
                                (byte_p[boost::bind(&setIntColor,
                                                    boost::ref(self.m_rColor.r),_1)] >> ',' >>
                                 byte_p[boost::bind(&setIntColor,
                                                    boost::ref(self.m_rColor.g),_1)] >> ',' >>
                                 byte_p[boost::bind(&setIntColor,
                                                    boost::ref(self.m_rColor.b),_1)])
                             |
                                // rgb(double,double,double)
                                (real_p[assign_a(self.m_rColor.r)] >> ',' >>
                                 real_p[assign_a(self.m_rColor.g)] >> ',' >>
                                 real_p[assign_a(self.m_rColor.b)])
                             |
                                // rgb(percent,percent,percent)
                                (real_p[boost::bind(&setPercentColor,
                                                    boost::ref(self.m_rColor.r),_1)] >> "%," >>
                                 real_p[boost::bind(&setPercentColor,
                                                    boost::ref(self.m_rColor.g),_1)] >> "%," >>
                                 real_p[boost::bind(&setPercentColor,
                                                    boost::ref(self.m_rColor.b),_1)] >> "%")
                             )
                         >> ')')
                     );
            }
            ::boost::spirit::classic::rule<ScannerT> const& start() const { return colorExpression; }
        };
    };
}

bool parseColor( const char* sColor, ARGBColor& rColor  )
{
    using namespace ::boost::spirit::classic;

    if( parse(sColor,
              ColorGrammar(rColor) >> end_p,
              space_p).full )
    {
        // free-form color found & parsed
        return true;
    }

    // no free-form color - maybe a color name?
    // trim white space before
    while( *sColor &&
           (*sColor==' ' || *sColor=='\t' || *sColor=='\r' || *sColor=='\n') )
        ++sColor;
    // trim white space after
    int nLen=strlen(sColor)-1;
    while( nLen &&
           (sColor[nLen]==' ' || sColor[nLen]=='\t' || sColor[nLen]=='\r' || sColor[nLen]=='\n') )
        --nLen;
    switch (getTokenId(sColor, nLen+1))
    {
        case XML_ALICEBLUE: rColor = ARGBColor(240,248,255); return true;
        case XML_ANTIQUEWHITE: rColor = ARGBColor(250,235,215); return true;
        case XML_AQUA: rColor = ARGBColor(0,255,255); return true;
        case XML_AQUAMARINE: rColor = ARGBColor(127,255,212); return true;
        case XML_AZURE: rColor = ARGBColor(240,255,255); return true;
        case XML_BEIGE: rColor = ARGBColor(245,245,220); return true;
        case XML_BISQUE: rColor = ARGBColor(255,228,196); return true;
        case XML_BLACK: rColor = ARGBColor(0,0,0); return true;
        case XML_BLANCHEDALMOND: rColor = ARGBColor(255,235,205); return true;
        case XML_BLUE: rColor = ARGBColor(0,0,255); return true;
        case XML_BLUEVIOLET: rColor = ARGBColor(138,43,226); return true;
        case XML_BROWN: rColor = ARGBColor(165,42,42); return true;
        case XML_BURLYWOOD: rColor = ARGBColor(222,184,135); return true;
        case XML_CADETBLUE: rColor = ARGBColor(95,158,160); return true;
        case XML_CHARTREUSE: rColor = ARGBColor(127,255,0); return true;
        case XML_CHOCOLATE: rColor = ARGBColor(210,105,30); return true;
        case XML_CORAL: rColor = ARGBColor(255,127,80); return true;
        case XML_CORNFLOWERBLUE: rColor = ARGBColor(100,149,237); return true;
        case XML_CORNSILK: rColor = ARGBColor(255,248,220); return true;
        case XML_CRIMSON: rColor = ARGBColor(220,20,60); return true;
        case XML_CYAN: rColor = ARGBColor(0,255,255); return true;
        case XML_DARKBLUE: rColor = ARGBColor(0,0,139); return true;
        case XML_DARKCYAN: rColor = ARGBColor(0,139,139); return true;
        case XML_DARKGOLDENROD: rColor = ARGBColor(184,134,11); return true;
        case XML_DARKGRAY: rColor = ARGBColor(169,169,169); return true;
        case XML_DARKGREEN: rColor = ARGBColor(0,100,0); return true;
        case XML_DARKGREY: rColor = ARGBColor(169,169,169); return true;
        case XML_DARKKHAKI: rColor = ARGBColor(189,183,107); return true;
        case XML_DARKMAGENTA: rColor = ARGBColor(139,0,139); return true;
        case XML_DARKOLIVEGREEN: rColor = ARGBColor(85,107,47); return true;
        case XML_DARKORANGE: rColor = ARGBColor(255,140,0); return true;
        case XML_DARKORCHID: rColor = ARGBColor(153,50,204); return true;
        case XML_DARKRED: rColor = ARGBColor(139,0,0); return true;
        case XML_DARKSALMON: rColor = ARGBColor(233,150,122); return true;
        case XML_DARKSEAGREEN: rColor = ARGBColor(143,188,143); return true;
        case XML_DARKSLATEBLUE: rColor = ARGBColor(72,61,139); return true;
        case XML_DARKSLATEGRAY: rColor = ARGBColor(47,79,79); return true;
        case XML_DARKSLATEGREY: rColor = ARGBColor(47,79,79); return true;
        case XML_DARKTURQUOISE: rColor = ARGBColor(0,206,209); return true;
        case XML_DARKVIOLET: rColor = ARGBColor(148,0,211); return true;
        case XML_DEEPPINK: rColor = ARGBColor(255,20,147); return true;
        case XML_DEEPSKYBLUE: rColor = ARGBColor(0,191,255); return true;
        case XML_DIMGRAY: rColor = ARGBColor(105,105,105); return true;
        case XML_DIMGREY: rColor = ARGBColor(105,105,105); return true;
        case XML_DODGERBLUE: rColor = ARGBColor(30,144,255); return true;
        case XML_FIREBRICK: rColor = ARGBColor(178,34,34); return true;
        case XML_FLORALWHITE: rColor = ARGBColor(255,250,240); return true;
        case XML_FORESTGREEN: rColor = ARGBColor(34,139,34); return true;
        case XML_FUCHSIA: rColor = ARGBColor(255,0,255); return true;
        case XML_GAINSBORO: rColor = ARGBColor(220,220,220); return true;
        case XML_GHOSTWHITE: rColor = ARGBColor(248,248,255); return true;
        case XML_GOLD: rColor = ARGBColor(255,215,0); return true;
        case XML_GOLDENROD: rColor = ARGBColor(218,165,32); return true;
        case XML_GRAY: rColor = ARGBColor(128,128,128); return true;
        case XML_GREY: rColor = ARGBColor(128,128,128); return true;
        case XML_GREEN: rColor = ARGBColor(0,128,0); return true;
        case XML_GREENYELLOW: rColor = ARGBColor(173,255,47); return true;
        case XML_HONEYDEW: rColor = ARGBColor(240,255,240); return true;
        case XML_HOTPINK: rColor = ARGBColor(255,105,180); return true;
        case XML_INDIANRED: rColor = ARGBColor(205,92,92); return true;
        case XML_INDIGO: rColor = ARGBColor(75,0,130); return true;
        case XML_IVORY: rColor = ARGBColor(255,255,240); return true;
        case XML_KHAKI: rColor = ARGBColor(240,230,140); return true;
        case XML_LAVENDER: rColor = ARGBColor(230,230,250); return true;
        case XML_LAVENDERBLUSH: rColor = ARGBColor(255,240,245); return true;
        case XML_LAWNGREEN: rColor = ARGBColor(124,252,0); return true;
        case XML_LEMONCHIFFON: rColor = ARGBColor(255,250,205); return true;
        case XML_LIGHTBLUE: rColor = ARGBColor(173,216,230); return true;
        case XML_LIGHTCORAL: rColor = ARGBColor(240,128,128); return true;
        case XML_LIGHTCYAN: rColor = ARGBColor(224,255,255); return true;
        case XML_LIGHTGOLDENRODYELLOW: rColor = ARGBColor(250,250,210); return true;
        case XML_LIGHTGRAY: rColor = ARGBColor(211,211,211); return true;
        case XML_LIGHTGREEN: rColor = ARGBColor(144,238,144); return true;
        case XML_LIGHTGREY: rColor = ARGBColor(211,211,211); return true;
        case XML_LIGHTPINK: rColor = ARGBColor(255,182,193); return true;
        case XML_LIGHTSALMON: rColor = ARGBColor(255,160,122); return true;
        case XML_LIGHTSEAGREEN: rColor = ARGBColor(32,178,170); return true;
        case XML_LIGHTSKYBLUE: rColor = ARGBColor(135,206,250); return true;
        case XML_LIGHTSLATEGRAY: rColor = ARGBColor(119,136,153); return true;
        case XML_LIGHTSLATEGREY: rColor = ARGBColor(119,136,153); return true;
        case XML_LIGHTSTEELBLUE: rColor = ARGBColor(176,196,222); return true;
        case XML_LIGHTYELLOW: rColor = ARGBColor(255,255,224); return true;
        case XML_LIME: rColor = ARGBColor(0,255,0); return true;
        case XML_LIMEGREEN: rColor = ARGBColor(50,205,50); return true;
        case XML_LINEN: rColor = ARGBColor(250,240,230); return true;
        case XML_MAGENTA: rColor = ARGBColor(255,0,255); return true;
        case XML_MAROON: rColor = ARGBColor(128,0,0); return true;
        case XML_MEDIUMAQUAMARINE: rColor = ARGBColor(102,205,170); return true;
        case XML_MEDIUMBLUE: rColor = ARGBColor(0,0,205); return true;
        case XML_MEDIUMORCHID: rColor = ARGBColor(186,85,211); return true;
        case XML_MEDIUMPURPLE: rColor = ARGBColor(147,112,219); return true;
        case XML_MEDIUMSEAGREEN: rColor = ARGBColor(60,179,113); return true;
        case XML_MEDIUMSLATEBLUE: rColor = ARGBColor(123,104,238); return true;
        case XML_MEDIUMSPRINGGREEN: rColor = ARGBColor(0,250,154); return true;
        case XML_MEDIUMTURQUOISE: rColor = ARGBColor(72,209,204); return true;
        case XML_MEDIUMVIOLETRED: rColor = ARGBColor(199,21,133); return true;
        case XML_MIDNIGHTBLUE: rColor = ARGBColor(25,25,112); return true;
        case XML_MINTCREAM: rColor = ARGBColor(245,255,250); return true;
        case XML_MISTYROSE: rColor = ARGBColor(255,228,225); return true;
        case XML_MOCCASIN: rColor = ARGBColor(255,228,181); return true;
        case XML_NAVAJOWHITE: rColor = ARGBColor(255,222,173); return true;
        case XML_NAVY: rColor = ARGBColor(0,0,128); return true;
        case XML_OLDLACE: rColor = ARGBColor(253,245,230); return true;
        case XML_OLIVE: rColor = ARGBColor(128,128,0); return true;
        case XML_OLIVEDRAB: rColor = ARGBColor(107,142,35); return true;
        case XML_ORANGE: rColor = ARGBColor(255,165,0); return true;
        case XML_ORANGERED: rColor = ARGBColor(255,69,0); return true;
        case XML_ORCHID: rColor = ARGBColor(218,112,214); return true;
        case XML_PALEGOLDENROD: rColor = ARGBColor(238,232,170); return true;
        case XML_PALEGREEN: rColor = ARGBColor(152,251,152); return true;
        case XML_PALETURQUOISE: rColor = ARGBColor(175,238,238); return true;
        case XML_PALEVIOLETRED: rColor = ARGBColor(219,112,147); return true;
        case XML_PAPAYAWHIP: rColor = ARGBColor(255,239,213); return true;
        case XML_PEACHPUFF: rColor = ARGBColor(255,218,185); return true;
        case XML_PERU: rColor = ARGBColor(205,133,63); return true;
        case XML_PINK: rColor = ARGBColor(255,192,203); return true;
        case XML_PLUM: rColor = ARGBColor(221,160,221); return true;
        case XML_POWDERBLUE: rColor = ARGBColor(176,224,230); return true;
        case XML_PURPLE: rColor = ARGBColor(128,0,128); return true;
        case XML_RED: rColor = ARGBColor(255,0,0); return true;
        case XML_ROSYBROWN: rColor = ARGBColor(188,143,143); return true;
        case XML_ROYALBLUE: rColor = ARGBColor(65,105,225); return true;
        case XML_SADDLEBROWN: rColor = ARGBColor(139,69,19); return true;
        case XML_SALMON: rColor = ARGBColor(250,128,114); return true;
        case XML_SANDYBROWN: rColor = ARGBColor(244,164,96); return true;
        case XML_SEAGREEN: rColor = ARGBColor(46,139,87); return true;
        case XML_SEASHELL: rColor = ARGBColor(255,245,238); return true;
        case XML_SIENNA: rColor = ARGBColor(160,82,45); return true;
        case XML_SILVER: rColor = ARGBColor(192,192,192); return true;
        case XML_SKYBLUE: rColor = ARGBColor(135,206,235); return true;
        case XML_SLATEBLUE: rColor = ARGBColor(106,90,205); return true;
        case XML_SLATEGRAY: rColor = ARGBColor(112,128,144); return true;
        case XML_SLATEGREY: rColor = ARGBColor(112,128,144); return true;
        case XML_SNOW: rColor = ARGBColor(255,250,250); return true;
        case XML_SPRINGGREEN: rColor = ARGBColor(0,255,127); return true;
        case XML_STEELBLUE: rColor = ARGBColor(70,130,180); return true;
        case XML_TAN: rColor = ARGBColor(210,180,140); return true;
        case XML_TEAL: rColor = ARGBColor(0,128,128); return true;
        case XML_THISTLE: rColor = ARGBColor(216,191,216); return true;
        case XML_TOMATO: rColor = ARGBColor(255,99,71); return true;
        case XML_TURQUOISE: rColor = ARGBColor(64,224,208); return true;
        case XML_VIOLET: rColor = ARGBColor(238,130,238); return true;
        case XML_WHEAT: rColor = ARGBColor(245,222,179); return true;
        case XML_WHITE: rColor = ARGBColor(255,255,255); return true;
        case XML_WHITESMOKE: rColor = ARGBColor(245,245,245); return true;
        case XML_YELLOW: rColor = ARGBColor(255,255,0); return true;
        case XML_YELLOWGREEN: rColor = ARGBColor(154,205,50); return true;

        default:
            return false; // no color at all, I'd guess.
    }
}

bool parseOpacity (const char* sOpacity, ARGBColor& rColor )
{
    using namespace ::boost::spirit::classic;

    if( parse(sOpacity,
              // Begin grammar
              (
                  real_p[assign_a(rColor.a)]
                  ) >> end_p,
              // End grammar
              space_p).full )
    {
        return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////

bool parseTransform( const char* sTransform, basegfx::B2DHomMatrix& rTransform )
{
    using namespace ::boost::spirit::classic;

    double fRefOffsetX(0.0);
    double fRefOffsetY(0.0);
    bool   bRefTransform(false);

    double fRotationAngle=0.0;
    double fSkewAngle=0.0;
    geometry::AffineMatrix2D aIdentityTransform;
    geometry::AffineMatrix2D aCurrTransform;
    std::vector<geometry::AffineMatrix2D> aTransforms;
    aIdentityTransform.m00 = 1.0; aIdentityTransform.m11 = 1.0;
    aCurrTransform = aIdentityTransform;

    const bool bRes = parse(sTransform,
        //  Begin grammar
        (
            // identity transform
            str_p("none")
          |
            // the ref() form
            (str_p("ref")
             >> '('
             >> str_p("svg")[assign_a(bRefTransform,true)]
             >> !(real_p[assign_a(fRefOffsetX)] >> (',' | eps_p) >>
                  real_p[assign_a(fRefOffsetY)])
             >> ')')
          |
            // the transform-list form
            (list_p(
               (
                 // matrix(a,b,c,d,e,f)
                 (str_p("matrix")
                  >> '('
                  >> real_p[assign_a(aCurrTransform.m00)] >> (',' | eps_p)
                  >> real_p[assign_a(aCurrTransform.m10)] >> (',' | eps_p)
                  >> real_p[assign_a(aCurrTransform.m01)] >> (',' | eps_p)
                  >> real_p[assign_a(aCurrTransform.m11)] >> (',' | eps_p)
                  >> real_p[assign_a(aCurrTransform.m02)] >> (',' | eps_p)
                  >> real_p[assign_a(aCurrTransform.m12)]
                  >> ')')[push_back_a(aTransforms,aCurrTransform)]
               |
                 // translate(x,[y])
                 (str_p("translate")
                  >> '('
                  >> real_p[boost::bind(&assign_twice,
                                        boost::ref(aCurrTransform.m02),
                                        boost::ref(aCurrTransform.m12),_1)]
                  >> !((',' | eps_p) >> real_p[assign_a(aCurrTransform.m12)])
                  >> ')')[push_back_a(aTransforms,aCurrTransform)]
               |
                 // scale(x,[y])
                 (str_p("scale")
                  >> '('
                  >> real_p[boost::bind(&assign_twice,
                                        boost::ref(aCurrTransform.m00),
                                        boost::ref(aCurrTransform.m11),_1)]
                  >> !((',' | eps_p) >> real_p[assign_a(aCurrTransform.m11)])
                  >> ')')[push_back_a(aTransforms,aCurrTransform)]
               |
                 // rotate(phi,[cx, cy])
                 (str_p("rotate")
                  >> '('
                  >> real_p[assign_a(fRotationAngle)]
                  >> !((',' | eps_p) >> real_p[assign_a(aCurrTransform.m02)]
                       >> real_p[assign_a(aCurrTransform.m12)])
                  >> ')')[boost::bind(&calcRotation,
                                      boost::ref(aTransforms),
                                      boost::ref(aCurrTransform),
                                      boost::cref(fRotationAngle))]
               |
                 // skewX(phi)
                 (str_p("skewX")
                  >> '('
                  >> real_p[assign_a(fSkewAngle)]
                  >> ')')[boost::bind(&calcSkewX,
                                      boost::ref(aTransforms),
                                      boost::cref(fSkewAngle))]
               |
                 // skewY(phi)
                 (str_p("skewY")
                  >> '('
                  >> real_p[assign_a(fSkewAngle)]
                  >> ')')[boost::bind(&calcSkewY,
                                      boost::ref(aTransforms),
                                      boost::cref(fSkewAngle))]
                 // reset current transform after every push
               )[assign_a(aCurrTransform,aIdentityTransform)],
                 // list delimiter is either ',' or space
               ',' | eps_p ))
        ) >> end_p,
        //  End grammar
        space_p).full;

    if( !bRes )
        return false;

    // fold all transformations into one
    const geometry::AffineMatrix2D aTotalTransform(
        std::accumulate(aTransforms.begin(),
                        aTransforms.end(),
                        aIdentityTransform,
                        &multiplyMatrix));

    basegfx::unotools::homMatrixFromAffineMatrix(
        rTransform,
        aTotalTransform);

    // TODO(F1): handle the ref case
    return bRes;
}

//////////////////////////////////////////////////////////////

bool parseViewBox( const char* sViewbox, basegfx::B2DRange& rRect )
{
    using namespace ::boost::spirit::classic;

    double x=0.0,y=0.0,w=0.0,h=0.0;

    const bool bRes = parse(sViewbox,
        //  Begin grammar
        (
            // either comma- or space-delimited list of four doubles
            real_p[assign_a(x)] >> (',' | eps_p) >>
            real_p[assign_a(y)] >> (',' | eps_p) >>
            real_p[assign_a(w)] >> (',' | eps_p) >>
            real_p[assign_a(h)] >> end_p
        ),
        //  End grammar
        space_p).full;

    if( !bRes )
        return false;

    rRect = basegfx::B2DRange(x,y,x+w,y+h);

    return true;
}

//////////////////////////////////////////////////////////////

bool parseDashArray( const char* sDashArray, std::vector<double>& rOutputVector )
{
    using namespace ::boost::spirit::classic;

    rOutputVector.clear();
    return parse(sDashArray,
        //  Begin grammar
        (
            // parse comma-delimited list of doubles (have to use the
            // 'direct' variant, as otherwise spirit refactors our
            // parser to push both real num and comma to push_back_a)
            list_p.direct
            (
                real_p[push_back_a(rOutputVector)],
                // list delimiter is either ',' or space
                ',' | eps_p
            )
        ) >> end_p,
        //  End grammar
        space_p).full;
}

//////////////////////////////////////////////////////////////

bool parsePaintUri( std::pair<const char*,const char*>& o_rPaintUri,
                    std::pair<ARGBColor,bool>&          io_rColor,
                    const char*                         sPaintUri )
{
    using namespace ::boost::spirit::classic;

    const bool bRes = parse(sPaintUri,
        //  Begin grammar
        (
            str_p("url(") >> !( str_p("'") | str_p("\"") ) >> ("#") >>
            (+alnum_p)[assign_a(o_rPaintUri)] >>
            !( str_p("'") | str_p("\"") ) >> str_p(")") >>
            *( str_p("none")[assign_a(io_rColor.second,false)] |
               str_p("currentColor")[assign_a(io_rColor.second,true)] |
               ColorGrammar(io_rColor.first)
               // TODO(F1): named color
             )
        ) >> end_p,
        //  End grammar
        space_p).full;

    return bRes;
}

//////////////////////////////////////////////////////////////

bool parseXlinkHref( const char* sXlinkHref, std::string& data )
{
    using namespace ::boost::spirit::classic;

    data.erase(data.begin(),data.end());

    std::string sLink(sXlinkHref);

    if (!sLink.compare(0,5,"data:"))
    {
        // the inplace "data" uri
        size_t position = sLink.rfind(',');
        if (position > 0 && position < std::string::npos)
        {
            data = sLink.substr(position+1);
            OSL_TRACE("%s", data.c_str());
            return true;
        }
    }

    return false;
}

} // namespace svgi

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
