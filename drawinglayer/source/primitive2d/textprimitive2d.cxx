/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textprimitive2d.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: hdu $ $Date: 2007-02-22 15:11:38 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE_TEXTPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_TEXTLAYOUTDEVICE_HXX
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#endif

#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif

#ifndef _BGFX_COLOR_BCOLOR_HXX
#include <basegfx/color/bcolor.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POLYPOLYGONPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POLYGONPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_STROKEATTRIBUTE_HXX
#include <drawinglayer/attribute/strokeattribute.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DLINEGEOMETRY_HXX
#include <basegfx/polygon/b2dlinegeometry.hxx>
#endif

#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
#endif

#ifndef _BGFX_TOOLS_CANVASTOOLS_HXX
#include <basegfx/tools/canvastools.hxx>
#endif

#include <numeric>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Font getVclFontFromFontAttributes(const FontAttributes& rFontAttributes, const basegfx::B2DHomMatrix& rTransform)
        {
            // decompose matrix to have position and size of text
            basegfx::B2DVector aScale, aTranslate;
            double fRotate, fShearX;
            rTransform.decompose(aScale, aTranslate, fRotate, fShearX);
            return getVclFontFromFontAttributes(rFontAttributes, aScale, fRotate);
        }

        Font getVclFontFromFontAttributes(const FontAttributes& rFontAttributes, const basegfx::B2DVector& rFontSize, double fFontRotation)
        {
            sal_uInt32 nWidth(basegfx::fround(fabs(rFontSize.getX())));
            sal_uInt32 nHeight(basegfx::fround(fabs(rFontSize.getY())));

            if(nWidth == nHeight)
            {
                nWidth = 0L;
            }

            Font aRetval(
                rFontAttributes.maFamilyName,
                rFontAttributes.maStyleName,
                Size(nWidth, nHeight));

            if(!basegfx::fTools::equalZero(fFontRotation))
            {
                sal_Int16 aRotate10th((sal_Int16)(fFontRotation * (-1800.0/F_PI)));
                aRetval.SetOrientation(aRotate10th % 3600);
            }

            aRetval.SetAlign(ALIGN_BASELINE);
            aRetval.SetCharSet(rFontAttributes.mbSymbol ? RTL_TEXTENCODING_SYMBOL : RTL_TEXTENCODING_UNICODE);
            aRetval.SetVertical(rFontAttributes.mbVertical ? TRUE : FALSE);
            aRetval.SetWeight(static_cast<FontWeight>(rFontAttributes.mnWeight));
            aRetval.SetItalic(rFontAttributes.mbItalic ? ITALIC_NORMAL : ITALIC_NONE);
            aRetval.SetOutline(rFontAttributes.mbOutline);

            return aRetval;
        }

        FontAttributes getFontAttributesFromVclFont(basegfx::B2DVector& rSize, const Font& rFont)
        {
            FontAttributes aRetval;

            aRetval.maFamilyName = rFont.GetName();
            aRetval.maStyleName = rFont.GetStyleName();
            aRetval.mbSymbol = (RTL_TEXTENCODING_SYMBOL == rFont.GetCharSet());
            aRetval.mbVertical = rFont.IsVertical();
            aRetval.mnWeight = static_cast<sal_uInt16>(rFont.GetWeight());
            aRetval.mbItalic = (rFont.GetItalic() != ITALIC_NONE);
            aRetval.mbOutline = rFont.IsOutline();
            // TODO: eKerning

            const sal_Int32 nWidth(rFont.GetSize().getWidth());
            const sal_Int32 nHeight(rFont.GetSize().getHeight());
            rSize.setX(nWidth ? nWidth : nHeight);
            rSize.setY(nHeight);

            return aRetval;
        }
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence TextSimplePortionPrimitive2D::createLocalDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            // get integer DXArray for getTextOutlines call (ATM uses vcl)
            ::std::vector< sal_Int32 > aNewIntegerDXArray;
            getIntegerDXArray(aNewIntegerDXArray);

            // prepare transformation matrices
            basegfx::B2DVector aScale, aTranslate;
            double fRotate, fShearX;
            getTextTransform().decompose(aScale, aTranslate, fRotate, fShearX);
            basegfx::B2DHomMatrix aUnscaledTransform;
            aUnscaledTransform.rotate( fRotate );
            aUnscaledTransform.shearX( fShearX );
            aUnscaledTransform.translate( aTranslate.getX(), aTranslate.getY() );
            basegfx::B2DHomMatrix aUnrotatedTransform = getTextTransform();
            aUnrotatedTransform.rotate( -fRotate );

            // prepare textlayoutdevice
            TextLayouterDevice aTextLayouter;
            aTextLayouter.setFontAttributes(getFontAttributes(), aUnrotatedTransform );

            // get the text outlines
            basegfx::B2DPolyPolygonVector aB2DPolyPolyVector;
            aTextLayouter.getTextOutlines( aB2DPolyPolyVector,
                getText(), 0L, getText().Len(), aNewIntegerDXArray);

            // create primitives for the outlines
            const sal_uInt32 nCount = aB2DPolyPolyVector.size();
            Primitive2DSequence aRetval( nCount );

            if( !nCount )
            {
                // for invisible glyphs
                return aRetval;
            }
            else if( !getFontAttributes().mbOutline )
            {
                // for the glyph shapes as color-filled polypolygons
                for(sal_uInt32 a(0L); a < nCount; a++)
                {
                    // prepare polypolygon
                    basegfx::B2DPolyPolygon& rPolyPolygon = aB2DPolyPolyVector[a];
                    rPolyPolygon.transform(aUnscaledTransform);
                    aRetval[a] = new PolyPolygonColorPrimitive2D(rPolyPolygon, getFontColor());
                }
            }
            else
            {
                // for the glyph shapes as outline-only polypolygons
                double fStrokeWidth = 1.0 + aScale.getY() * 0.02;
                if( getFontAttributes().mnWeight > WEIGHT_SEMIBOLD )
                    fStrokeWidth *= 1.4;
                else if( getFontAttributes().mnWeight < WEIGHT_SEMILIGHT )
                    fStrokeWidth *= 0.7;
                const drawinglayer::attribute::StrokeAttribute aStrokeAttr( getFontColor(),
                    fStrokeWidth, basegfx::tools::B2DLINEJOIN_NONE );
                for(sal_uInt32 a(0L); a < nCount; a++)
                {
                    basegfx::B2DPolyPolygon& rPolyPolygon = aB2DPolyPolyVector[a];
                    rPolyPolygon.transform(aUnscaledTransform);
                    aRetval[a] = new PolyPolygonStrokePrimitive2D(rPolyPolygon, aStrokeAttr);
                }
            }

            return aRetval;
        }

        TextSimplePortionPrimitive2D::TextSimplePortionPrimitive2D(
            const basegfx::B2DHomMatrix& rNewTransform,
            const String& rText,
            const ::std::vector< double >& rDXArray,
            const FontAttributes& rFontAttributes,
            const basegfx::BColor& rFontColor)
        :   BasePrimitive2D(),
            maTextTransform(rNewTransform),
            maText(rText),
            maDXArray(rDXArray),
            maFontAttributes(rFontAttributes),
            maFontColor(rFontColor)
        {
        }

        void TextSimplePortionPrimitive2D::getIntegerDXArray(::std::vector< sal_Int32 >& rDXArray) const
        {
            rDXArray.clear();

            if(getDXArray().size())
            {
                rDXArray.reserve(getDXArray().size());
                const basegfx::B2DVector aPixelVector(getTextTransform() * basegfx::B2DVector(1.0, 0.0));
                const double fPixelVectorLength(aPixelVector.getLength());

                for(::std::vector< double >::const_iterator aStart(getDXArray().begin()); aStart != getDXArray().end(); aStart++)
                {
                    rDXArray.push_back(basegfx::fround((*aStart) * fPixelVectorLength));
                }
            }
        }

        bool TextSimplePortionPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BasePrimitive2D::operator==(rPrimitive))
            {
                const TextSimplePortionPrimitive2D& rCompare = (TextSimplePortionPrimitive2D&)rPrimitive;

                return (getTextTransform() == rCompare.getTextTransform()
                    && getText() == rCompare.getText()
                    && getDXArray() == rCompare.getDXArray()
                    && getFontAttributes() == rCompare.getFontAttributes()
                    && getFontColor() == rCompare.getFontColor());
            }

            return false;
        }

        basegfx::B2DRange TextSimplePortionPrimitive2D::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            const xub_StrLen aStrLen(getText().Len());
            basegfx::B2DRange aRetval;

            if(aStrLen)
            {
                // get TextBoundRect as base size
                TextLayouterDevice aTextLayouter;
                aTextLayouter.setFontAttributes(getFontAttributes(), getTextTransform());
                aRetval = aTextLayouter.getTextBoundRect(getText(), 0L, aStrLen);

                // apply textTransform to it, but without scaling. The scale defines the font size
                // which is already part of the fetched textRange
                basegfx::B2DVector aScale, aTranslate;
                double fRotate, fShearX;
                basegfx::B2DHomMatrix aTextTransformWithoutScale;

                getTextTransform().decompose(aScale, aTranslate, fRotate, fShearX);
                aTextTransformWithoutScale.shearX(fShearX);
                aTextTransformWithoutScale.rotate(fRotate);
                aTextTransformWithoutScale.translate(aTranslate.getX(), aTranslate.getY());
                aRetval.transform(aTextTransformWithoutScale);
            }

            return aRetval;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(TextSimplePortionPrimitive2D, '2','T','S','i')

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence TextDecoratedPortionPrimitive2D::createLocalDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            Primitive2DSequence aRetval(6);

            // First create a simple text primitive and ignore other attributes
            aRetval[0] = new TextSimplePortionPrimitive2D(getTextTransform(), getText(), getDXArray(), getFontAttributes(), getFontColor());

            if( getFontUnderline() == FONT_UNDERLINE_NONE
            &&  getFontStrikeout() == FONT_STRIKEOUT_NONE
            &&  getEmphasisMarkAbove() == FONT_EMPHASISMARK_NONE
            &&  getEmphasisMarkBelow() == FONT_EMPHASISMARK_NONE )
                return aRetval;

            // prepare transformation
            basegfx::B2DVector aScale, aTranslate;
            double fRotate, fShearX;
            getTextTransform().decompose(aScale, aTranslate, fRotate, fShearX);
            basegfx::B2DHomMatrix aUnscaledTransform;
            aUnscaledTransform.rotate( fRotate );
            aUnscaledTransform.shearX( fShearX );
            aUnscaledTransform.translate( aTranslate.getX(), aTranslate.getY() );

            basegfx::B2DHomMatrix aUnrotatedTransform = getTextTransform();
            aUnrotatedTransform.rotate( -fRotate );

            // get metrics for text decorations like underline/strikeout/emphasis marks
            TextLayouterDevice aTextLayouter;
            aTextLayouter.setFontAttributes(getFontAttributes(), aUnrotatedTransform );

//          const double fLineHeight = aTextLayouter.getTextHeight();
            double fUnderlineOffset = aTextLayouter.getUnderlineOffset();
            double fUnderlineHeight = aTextLayouter.getUnderlineHeight();
            basegfx::tools::B2DLineJoin eLineJoin = basegfx::tools::B2DLINEJOIN_NONE;
            bool bDoubleLine = false;
            bool bWaveLine = false;

            double fTextWidth = 0.0;
            if( getDXArray().empty() )
                fTextWidth = aTextLayouter.getTextWidth( getText(), 0/*TODO*/, getText().Len()/*TODO*/ );
            else
                fTextWidth = getDXArray().back() * aScale.getX();

            // prepare line styles for text decoration lines
            const int* pDashDotArray = NULL;
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

            if( fUnderlineHeight > 0 )
            {
                if( bDoubleLine )
                {
                    fUnderlineOffset -= 0.50 * fUnderlineHeight;
                    fUnderlineHeight *= 0.64;
                }

                basegfx::B2DPolygon aUnderline;
                ::basegfx::B2DPoint aPoint( 0.0, fUnderlineOffset );
                aUnderline.append( aPoint );
                if( !bWaveLine )
                {
                    // straight underline
                    aUnderline.append( aPoint + ::basegfx::B2DPoint( fTextWidth, 0.0 ) );
                }
                else
                {
                    // wavy underline
                    basegfx::B2DPolygon& aWavePoly = aUnderline;
                    double fWaveWidth = 4 * fUnderlineHeight;
                    if( getFontUnderline() == primitive2d::FONT_UNDERLINE_SMALLWAVE )
                        fWaveWidth *= 0.7;
                    const double fWaveHeight = 0.5 * fWaveWidth;
                    const ::basegfx::B2DPoint aCtrlOffset( fWaveWidth * 0.467308, fWaveHeight );
                    for( double fPos = fWaveWidth; fPos < fTextWidth; fPos += fWaveWidth ) {
                        // create a symmetrical wave using one cubic bezier curve
                        // with y==0 for {x==0, x==0.5*fW or x==1.0*fW}
                        // and ymin/ymax at {x=0.25*fW or 0.75*fW}
                        const int n = aWavePoly.count();
                        aWavePoly.setControlPointA( n-1, aPoint + aCtrlOffset );
                        aWavePoly.append(               aPoint += ::basegfx::B2DPoint( fWaveWidth, 0.0 ) );
                        aWavePoly.setControlPointB( n-1, aPoint - aCtrlOffset );
                    }
                    // adjust stroke style
                    eLineJoin = basegfx::tools::B2DLINEJOIN_ROUND;
                    fUnderlineHeight *= 0.5;
                }

                const basegfx::BColor& rLineColor = getFontColor();
                attribute::StrokeAttribute aStrokeAttr( rLineColor, fUnderlineHeight, eLineJoin );
                if( pDashDotArray != NULL )
                {
                    ::std::vector< double > aDoubleArray;
                    for( const int* p = pDashDotArray; *p; ++p )
                        aDoubleArray.push_back( *p * fUnderlineHeight);
                    const double fFullDashDotLen = ::std::accumulate(aDoubleArray.begin(), aDoubleArray.end(), 0.0);
                    aStrokeAttr = attribute::StrokeAttribute( rLineColor,
                        fUnderlineHeight, eLineJoin, aDoubleArray, fFullDashDotLen );
                }
                aUnderline.transform( aUnscaledTransform );
                aRetval[1] = new PolygonStrokePrimitive2D( aUnderline, aStrokeAttr );

                if( bDoubleLine )
                {
                    // add another underline below the first underline
                    const double fLineDist = (bWaveLine ? 3 : 2) * fUnderlineHeight;
                    ::basegfx::B2DVector aOffsetVector( 0.0, fLineDist );
                    aOffsetVector = aUnscaledTransform * aOffsetVector;
                    basegfx::B2DHomMatrix aOffsetTransform;
                    aOffsetTransform.translate( aOffsetVector.getX(), aOffsetVector.getY() );
                    aUnderline.transform( aOffsetTransform );
                    aRetval[2] = new PolygonStrokePrimitive2D( aUnderline, aStrokeAttr );
                }
            }

            double fStrikeoutHeight = aTextLayouter.getUnderlineHeight();
            double fStrikeoutOffset = aTextLayouter.getStrikeoutOffset();
            eLineJoin = basegfx::tools::B2DLINEJOIN_NONE;
            bDoubleLine = false;
            sal_Unicode aStrikeoutChar = '\0';

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
            };

            if( fStrikeoutHeight > 0 )
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

                const basegfx::BColor& rStrikeoutColor = getFontColor();
                attribute::StrokeAttribute aStrokeAttr( rStrikeoutColor, fStrikeoutHeight, eLineJoin );
                aStrikeoutLine.transform( aUnscaledTransform );
                aRetval[3] = new PolygonStrokePrimitive2D( aStrikeoutLine, aStrokeAttr );

                if( bDoubleLine )
                {
                    // add another strikeout below the first strikeout
                    const double fLineDist = 2 * fStrikeoutHeight;
                    ::basegfx::B2DVector aOffsetVector( 0.0, -fLineDist );
                    aOffsetVector = aUnscaledTransform * aOffsetVector;
                    basegfx::B2DHomMatrix aOffsetTransform;
                    aOffsetTransform.translate( aOffsetVector.getX(), aOffsetVector.getY() );
                    aStrikeoutLine.transform( aOffsetTransform );
                    aRetval[4] = new PolygonStrokePrimitive2D( aStrikeoutLine, aStrokeAttr );
                }
            }

            if( aStrikeoutChar != '\0' )
            {
                String aString( &aStrikeoutChar, 1 );
                double fStrikeCharWidth = aTextLayouter.getTextWidth( aString, 0, 1 );
                double fStrikeCharCount = fTextWidth / fStrikeCharWidth;
                int nStrikeCharCount = static_cast<int>(fStrikeCharCount + 0.9);
                for( int i = 1; i < nStrikeCharCount; ++i )
                    aString += aStrikeoutChar;
                std::vector<double> aDXArray( nStrikeCharCount );
                fStrikeCharWidth /= aScale.getX();
                for( int i = 0; i < nStrikeCharCount; ++i )
                    aDXArray[i] = (i+1) * fStrikeCharWidth;
                const basegfx::BColor& rStrikeoutColor = getFontColor();
                aRetval[5] = new TextSimplePortionPrimitive2D(getTextTransform(), aString, aDXArray, getFontAttributes(), rStrikeoutColor );
            }

            // TODO: need to take care of
            // -emphasis mark
            // -relief (embosses/engraved)
            // -shadow
            // if( getWordLineMode() )
            // if( getUnderlineAbove() )

            return aRetval;
        }

        TextDecoratedPortionPrimitive2D::TextDecoratedPortionPrimitive2D(
            const basegfx::B2DHomMatrix& rNewTransform,
            const String& rText,
            const ::std::vector< double >& rDXArray,
            const FontAttributes& rFontAttributes,
            const basegfx::BColor& rFontColor,
            FontUnderline eFontUnderline,
            bool bUnderlineAbove,
            FontStrikeout eFontStrikeout,
            bool bWordLineMode,
            FontEmphasisMark eFontEmphasisMark,
            bool bEmphasisMarkAbove,
            bool bEmphasisMarkBelow,
            FontRelief eFontRelief,
            bool bShadow)
        :   TextSimplePortionPrimitive2D(rNewTransform, rText, rDXArray, rFontAttributes, rFontColor),
            meFontUnderline(eFontUnderline),
            meFontStrikeout(eFontStrikeout),
            meFontEmphasisMark(eFontEmphasisMark),
            meFontRelief(eFontRelief),
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

                return (getFontUnderline() == rCompare.getFontUnderline()
                    && getFontStrikeout() == rCompare.getFontStrikeout()
                    && getUnderlineAbove() == rCompare.getUnderlineAbove()
                    && getWordLineMode() == rCompare.getWordLineMode()
                    && getFontEmphasisMark() == rCompare.getFontEmphasisMark()
                    && getEmphasisMarkAbove() == rCompare.getEmphasisMarkAbove()
                    && getEmphasisMarkBelow() == rCompare.getEmphasisMarkBelow()
                    && getFontRelief() == rCompare.getFontRelief()
                    && getShadow() == rCompare.getShadow());
            }

            return false;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(TextDecoratedPortionPrimitive2D, '2','T','D','o')

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
