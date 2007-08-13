/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textdecoratedprimitive2d.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: aw $ $Date: 2007-08-13 15:30:25 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE_TEXTDECORATEDPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/textdecoratedprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_TEXTLAYOUTDEVICE_HXX
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POLYGONPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_STROKEATTRIBUTE_HXX
#include <drawinglayer/attribute/strokeattribute.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_PRIMITIVETYPES2D_HXX
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#endif

#include <numeric>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence TextDecoratedPortionPrimitive2D::createLocalDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            std::vector< BasePrimitive2D* > aNewPrimitives;

            // First create a simple text primitive and ignore other attributes
            aNewPrimitives.push_back(new TextSimplePortionPrimitive2D(getTextTransform(), getText(), getDXArray(), getFontAttributes(), getLocale(), getFontColor()));

            // more to be done?
            const bool bNeedFontUnderline(getFontUnderline() != FONT_UNDERLINE_NONE);
            const bool bNeedFontStrikeout(getFontStrikeout() != FONT_STRIKEOUT_NONE);
            const bool bNeedEmphasisMarkAbove(getEmphasisMarkAbove() != FONT_EMPHASISMARK_NONE);
            const bool bNeedEmphasisMarkBelow(getEmphasisMarkBelow() != FONT_EMPHASISMARK_NONE);
            const sal_uInt32 nSpellVectorSize(maWrongSpellVector.size());

            if(bNeedFontUnderline
                || bNeedFontStrikeout
                || bNeedEmphasisMarkAbove
                || bNeedEmphasisMarkBelow
                || 0 != nSpellVectorSize)
            {
                // prepare transformations
                basegfx::B2DVector aScale, aTranslate;
                double fRotate, fShearX;
                getTextTransform().decompose(aScale, aTranslate, fRotate, fShearX);

                // unscaled transform is needed since the scale in the text transform describes the font size
                basegfx::B2DHomMatrix aUnscaledTransform;
                aUnscaledTransform.rotate( fRotate );
                aUnscaledTransform.shearX( fShearX );
                aUnscaledTransform.translate( aTranslate.getX(), aTranslate.getY() );

                // prepare TextLayouterDevice to get metrics for text decorations like
                // underline/strikeout/emphasis marks from it
                TextLayouterDevice aTextLayouter;

                {
                    // unrotated transform is needed for TextLayouterDevice setup
                    basegfx::B2DHomMatrix aUnrotatedTransform = getTextTransform();
                    aUnrotatedTransform.rotate( -fRotate );
                    aTextLayouter.setFontAttributes(getFontAttributes(), aUnrotatedTransform );
                }

                // init metrics to defaults
//              const double fLineHeight(aTextLayouter.getTextHeight());
                double fUnderlineOffset(aTextLayouter.getUnderlineOffset());
                double fUnderlineHeight(aTextLayouter.getUnderlineHeight());
                basegfx::tools::B2DLineJoin eLineJoin(basegfx::tools::B2DLINEJOIN_NONE);
                bool bDoubleLine(false);
                bool bWaveLine(false);
                double fTextWidth(0.0);

                if(getDXArray().empty())
                {
                    fTextWidth = aTextLayouter.getTextWidth( getText(), 0/*TODO*/, getText().Len()/*TODO*/ );
                }
                else
                {
                    fTextWidth = getDXArray().back() * aScale.getX();
                }

                // prepare line styles for text decoration lines
                const int* pDashDotArray(0);

                static const int aDottedArray[]     = { 1, 1, 0};               // DOTTED LINE
                static const int aDashDotArray[]    = { 1, 1, 4, 1, 0};         // DASHDOT
                static const int aDashDotDotArray[] = { 1, 1, 1, 1, 4, 1, 0};   // DASHDOTDOT
                static const int aDashedArray[]     = { 5, 2, 0};               // DASHED LINE
                static const int aLongDashArray[]   = { 7, 2, 0};               // LONGDASH

                // set Underline attribute
                switch( getFontUnderline() )
                {
                    default:
                        DBG_WARNING1( "DrawingLayer: Unknown underline attribute (%d)!", getFontUnderline() );
                        // fall through
                    case primitive2d::FONT_UNDERLINE_NONE:
                        fUnderlineHeight = 0;
                        break;
                    case primitive2d::FONT_UNDERLINE_BOLD:
                        fUnderlineHeight *= 2;
                        // fall through
                    case primitive2d::FONT_UNDERLINE_SINGLE:
                        break;
                    case primitive2d::FONT_UNDERLINE_DOUBLE:
                        bDoubleLine = true;
                        break;
                    case primitive2d::FONT_UNDERLINE_BOLDDOTTED:
                        fUnderlineHeight *= 2;
                        // fall through
                    case primitive2d::FONT_UNDERLINE_DOTTED:
                        eLineJoin = basegfx::tools::B2DLINEJOIN_ROUND;
                        pDashDotArray = aDottedArray;
                        break;
                    case primitive2d::FONT_UNDERLINE_BOLDDASH:
                        fUnderlineHeight *= 2;
                        // fall through
                    case primitive2d::FONT_UNDERLINE_DASH:
                        pDashDotArray = aDashedArray;
                        break;
                    case primitive2d::FONT_UNDERLINE_BOLDLONGDASH:
                        fUnderlineHeight *= 2;
                        // fall through
                    case primitive2d::FONT_UNDERLINE_LONGDASH:
                        pDashDotArray = aLongDashArray;
                        break;
                    case primitive2d::FONT_UNDERLINE_BOLDDASHDOT:
                        fUnderlineHeight *= 2;
                        // fall through
                    case primitive2d::FONT_UNDERLINE_DASHDOT:
                        pDashDotArray = aDashDotArray;
                        break;
                    case primitive2d::FONT_UNDERLINE_BOLDDASHDOTDOT:
                        fUnderlineHeight *= 2;
                        // fall through
                    case primitive2d::FONT_UNDERLINE_DASHDOTDOT:
                        eLineJoin = basegfx::tools::B2DLINEJOIN_ROUND;
                        pDashDotArray = aDashDotDotArray;
                        break;
                    case primitive2d::FONT_UNDERLINE_SMALLWAVE:
                        // TODO
                        bWaveLine = true;
                        break;
                    case primitive2d::FONT_UNDERLINE_BOLDWAVE:
                        fUnderlineHeight *= 2;
                        // fall through
                    case primitive2d::FONT_UNDERLINE_WAVE:
                        // TODO
                        bWaveLine = true;
                        break;
                    case primitive2d::FONT_UNDERLINE_DOUBLEWAVE:
                        bWaveLine = true;
                        bDoubleLine = true;
                        break;
                }

                if(fUnderlineHeight > 0.0)
                {
                    if(bDoubleLine)
                    {
                        fUnderlineOffset -= 0.50 * fUnderlineHeight;
                        fUnderlineHeight *= 0.64;
                    }

                    basegfx::B2DPolygon aUnderline;
                    ::basegfx::B2DPoint aPoint( 0.0, fUnderlineOffset );
                    aUnderline.append( aPoint );

                    if(!bWaveLine)
                    {
                        // straight underline
                        aUnderline.append( aPoint + ::basegfx::B2DPoint( fTextWidth, 0.0 ) );
                    }
                    else
                    {
                        // wavy underline
                        basegfx::B2DPolygon& aWavePoly = aUnderline;
                        double fWaveWidth(4.0 * fUnderlineHeight);

                        if(primitive2d::FONT_UNDERLINE_SMALLWAVE == getFontUnderline())
                        {
                            fWaveWidth *= 0.7;
                        }

                        const double fWaveHeight(0.5 * fWaveWidth);
                        const ::basegfx::B2DPoint aCtrlOffset( fWaveWidth * 0.467308, fWaveHeight );

                        for(double fPos = fWaveWidth; fPos < fTextWidth; fPos += fWaveWidth)
                        {
                            // create a symmetrical wave using one cubic bezier curve
                            // with y==0 for {x==0, x==0.5*fW or x==1.0*fW}
                            // and ymin/ymax at {x=0.25*fW or 0.75*fW}
                            const int n = aWavePoly.count();

                            aWavePoly.setNextControlPoint(  n-1, aPoint + aCtrlOffset );
                            aWavePoly.append(aPoint += ::basegfx::B2DPoint( fWaveWidth, 0.0 ) );
                            aWavePoly.setPrevControlPoint(  n-1, aPoint - aCtrlOffset );
                        }

                        // adjust stroke style
                        eLineJoin = basegfx::tools::B2DLINEJOIN_ROUND;
                        fUnderlineHeight *= 0.5;
                    }

                    const basegfx::BColor& rLineColor = getTextlineColor();
                    attribute::StrokeAttribute aStrokeAttr(rLineColor, fUnderlineHeight, eLineJoin);

                    if(pDashDotArray)
                    {
                        ::std::vector< double > aDoubleArray;

                        for( const int* p = pDashDotArray; *p; ++p )
                        {
                            aDoubleArray.push_back( *p * fUnderlineHeight);
                        }

                        const double fFullDashDotLen(::std::accumulate(aDoubleArray.begin(), aDoubleArray.end(), 0.0));
                        aStrokeAttr = attribute::StrokeAttribute(rLineColor, fUnderlineHeight, eLineJoin, aDoubleArray, fFullDashDotLen);
                    }

                    aUnderline.transform( aUnscaledTransform );
                    aNewPrimitives.push_back(new PolygonStrokePrimitive2D( aUnderline, aStrokeAttr ));

                    if( bDoubleLine )
                    {
                        // add another underline below the first underline
                        const double fLineDist((bWaveLine ? 3.0 : 2.0) * fUnderlineHeight);
                        ::basegfx::B2DVector aOffsetVector( 0.0, fLineDist );
                        basegfx::B2DHomMatrix aOffsetTransform;

                        aOffsetVector = aUnscaledTransform * aOffsetVector;
                        aOffsetTransform.translate( aOffsetVector.getX(), aOffsetVector.getY() );
                        aUnderline.transform( aOffsetTransform );
                        aNewPrimitives.push_back(new PolygonStrokePrimitive2D( aUnderline, aStrokeAttr ));
                    }
                }

                double fStrikeoutHeight(aTextLayouter.getUnderlineHeight());
                double fStrikeoutOffset(aTextLayouter.getStrikeoutOffset());
                eLineJoin = basegfx::tools::B2DLINEJOIN_NONE;
                bDoubleLine = false;
                sal_Unicode aStrikeoutChar('\0');

                // set Underline attribute
                switch( getFontStrikeout() )
                {
                    default:
                        DBG_WARNING1( "DrawingLayer: Unknown underline attribute (%d)!", getFontUnderline() );
                        // fall through
                    case primitive2d::FONT_STRIKEOUT_NONE:
                        fStrikeoutHeight = 0;
                        break;
                    case primitive2d::FONT_STRIKEOUT_SINGLE:
                        break;
                    case primitive2d::FONT_STRIKEOUT_DOUBLE:
                        bDoubleLine = true;
                        break;
                    case primitive2d::FONT_STRIKEOUT_BOLD:
                        fStrikeoutHeight *= 2;
                        break;
                    case primitive2d::FONT_STRIKEOUT_SLASH:
                        aStrikeoutChar = '/';
                        fStrikeoutHeight = 0;
                        break;
                    case primitive2d::FONT_STRIKEOUT_X:
                        aStrikeoutChar = 'X';
                        fStrikeoutHeight = 0;
                        break;
                }

                if(fStrikeoutHeight > 0.0)
                {
                    if( bDoubleLine )
                    {
                        fStrikeoutOffset -= 0.50 * fStrikeoutHeight;
                        fStrikeoutHeight *= 0.64;
                    }

                    basegfx::B2DPolygon aStrikeoutLine;
                    basegfx::B2DPoint aPoint( 0.0, -fStrikeoutOffset );
                    aStrikeoutLine.append( aPoint );

                    if( 1/*####*/ )
                    {
                        // straight underline
                        aStrikeoutLine.append( aPoint + ::basegfx::B2DPoint( fTextWidth, 0.0 ) );
                    }

                    const basegfx::BColor& rStrikeoutColor = getTextlineColor();
                    attribute::StrokeAttribute aStrokeAttr( rStrikeoutColor, fStrikeoutHeight, eLineJoin );

                    aStrikeoutLine.transform( aUnscaledTransform );
                    aNewPrimitives.push_back(new PolygonStrokePrimitive2D( aStrikeoutLine, aStrokeAttr ));

                    if( bDoubleLine )
                    {
                        // add another strikeout below the first strikeout
                        const double fLineDist(2.0 * fStrikeoutHeight);
                        ::basegfx::B2DVector aOffsetVector( 0.0, -fLineDist );
                        basegfx::B2DHomMatrix aOffsetTransform;

                        aOffsetVector = aUnscaledTransform * aOffsetVector;
                        aOffsetTransform.translate( aOffsetVector.getX(), aOffsetVector.getY() );
                        aStrikeoutLine.transform( aOffsetTransform );
                        aNewPrimitives.push_back(new PolygonStrokePrimitive2D( aStrikeoutLine, aStrokeAttr ));
                    }
                }

                if( aStrikeoutChar != '\0' )
                {
                    const String aSingleCharString( &aStrikeoutChar, 1 );
                    const double fStrikeCharWidth(aTextLayouter.getTextWidth( aSingleCharString, 0, 1 ));
                    const double fStrikeCharCount(fabs(fTextWidth / fStrikeCharWidth));
                    const sal_uInt32 nStrikeCharCount(static_cast< sal_uInt32 >(fStrikeCharCount + 0.9));
                    const double fStrikeCharWidthUnscaled(aScale.getX() == 0.0 ? fStrikeCharWidth : fStrikeCharWidth / aScale.getX());
                    const basegfx::BColor& rStrikeoutColor = getFontColor();

                    std::vector<double> aDXArray(nStrikeCharCount);
                    String aStrikeoutString;

                    for(sal_uInt32 a(0); a < nStrikeCharCount; a++)
                    {
                        aStrikeoutString += aStrikeoutChar;
                        aDXArray[a] = (a + 1) * fStrikeCharWidthUnscaled;
                    }

                    aNewPrimitives.push_back(new TextSimplePortionPrimitive2D(getTextTransform(), aStrikeoutString, aDXArray, getFontAttributes(), getLocale(), rStrikeoutColor ));
                }

                // TODO: need to take care of
                // -emphasis mark
                // -relief (embosses/engraved)
                // -shadow
                // if( getWordLineMode() )
                // if( getUnderlineAbove() )

                if(nSpellVectorSize && !getDXArray().empty())
                {
                    // TODO: take care of WrongSpellVector; create redlining (red wavelines) accordingly.
                    // For test purposes, create single lines as long as no waveline primitive is created
                    const ::std::vector< double >& rDXArray = getDXArray();
                    const sal_uInt32 nDXCount(rDXArray.size());
                    const basegfx::BColor aSpellColor(1.0, 0.0, 0.0); // red

                    for(sal_uInt32 a(0); a < nSpellVectorSize; a++)
                    {
                        const WrongSpellEntry& rCandidate = maWrongSpellVector[a];

                        if(rCandidate.getStart() < rCandidate.getEnd())
                        {
                            ::basegfx::B2DPoint aStart;
                            ::basegfx::B2DPoint aEnd;

                            if(rCandidate.getStart() > 0 && rCandidate.getStart() - 1 < nDXCount)
                            {
                                aStart.setX(rDXArray[rCandidate.getStart() - 1] * aScale.getX());
                            }

                            if(rCandidate.getEnd() > 0 && rCandidate.getEnd() - 1 < nDXCount)
                            {
                                aEnd.setX(rDXArray[rCandidate.getEnd() - 1] * aScale.getX());
                            }

                            if(aStart != aEnd)
                            {
                                   basegfx::B2DPolygon aPolygon;

                                aPolygon.append(aStart);
                                aPolygon.append(aEnd);
                                aPolygon.transform(aUnscaledTransform);

                                aNewPrimitives.push_back(new PolygonHairlinePrimitive2D(aPolygon, aSpellColor));
                            }
                        }
                    }
                }
            }

            // prepare return sequence
            Primitive2DSequence aRetval(aNewPrimitives.size());

            for(sal_uInt32 a(0); a < aNewPrimitives.size(); a++)
            {
                aRetval[a] = Primitive2DReference(aNewPrimitives[a]);
            }

            return aRetval;
        }

        TextDecoratedPortionPrimitive2D::TextDecoratedPortionPrimitive2D(

            // TextSimplePortionPrimitive2D parameters
            const basegfx::B2DHomMatrix& rNewTransform,
            const String& rText,
            const ::std::vector< double >& rDXArray,
            const FontAttributes& rFontAttributes,
            const ::com::sun::star::lang::Locale& rLocale,
            const basegfx::BColor& rFontColor,

            // local parameters
            const basegfx::BColor& rTextlineColor,
            FontUnderline eFontUnderline,
            bool bUnderlineAbove,
            FontStrikeout eFontStrikeout,
            bool bWordLineMode,
            FontEmphasisMark eFontEmphasisMark,
            bool bEmphasisMarkAbove,
            bool bEmphasisMarkBelow,
            FontRelief eFontRelief,
            bool bShadow,
            const WrongSpellVector& rWrongSpellVector)
        :   TextSimplePortionPrimitive2D(rNewTransform, rText, rDXArray, rFontAttributes, rLocale, rFontColor),
            maTextlineColor(rTextlineColor),
            meFontUnderline(eFontUnderline),
            meFontStrikeout(eFontStrikeout),
            meFontEmphasisMark(eFontEmphasisMark),
            meFontRelief(eFontRelief),
            maWrongSpellVector(rWrongSpellVector),
            mbUnderlineAbove(bUnderlineAbove),
            mbWordLineMode(bWordLineMode),
            mbEmphasisMarkAbove(bEmphasisMarkAbove),
            mbEmphasisMarkBelow(bEmphasisMarkBelow),
            mbShadow(bShadow)
        {
        }

        bool TextDecoratedPortionPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(TextSimplePortionPrimitive2D::operator==(rPrimitive))
            {
                const TextDecoratedPortionPrimitive2D& rCompare = (TextDecoratedPortionPrimitive2D&)rPrimitive;

                return (getTextlineColor() == rCompare.getTextlineColor()
                    && getFontUnderline() == rCompare.getFontUnderline()
                    && getFontStrikeout() == rCompare.getFontStrikeout()
                    && getFontEmphasisMark() == rCompare.getFontEmphasisMark()
                    && getFontRelief() == rCompare.getFontRelief()
                    && getWrongSpellVector() == rCompare.getWrongSpellVector()
                    && getUnderlineAbove() == rCompare.getUnderlineAbove()
                    && getWordLineMode() == rCompare.getWordLineMode()
                    && getEmphasisMarkAbove() == rCompare.getEmphasisMarkAbove()
                    && getEmphasisMarkBelow() == rCompare.getEmphasisMarkBelow()
                    && getShadow() == rCompare.getShadow());
            }

            return false;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(TextDecoratedPortionPrimitive2D, PRIMITIVE2D_ID_TEXTDECORATEDPORTIONPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
