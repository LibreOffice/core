/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vclprocessor2d.cxx,v $
 *
 *  $Revision: 1.25 $
 *
 *  last change: $Author: aw $ $Date: 2008-02-07 13:41:59 $
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

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR2D_VCLPROCESSOR2D_HXX
#include <drawinglayer/processor2d/vclprocessor2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE_TEXTPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE_TEXTDECORATEDPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/textdecoratedprimitive2d.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POLYGONPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BITMAPPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR2D_VCLHELPERBITMAPTRANSFORM_HXX
#include <vclhelperbitmaptransform.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR2D_VCLHELPERBITMAPRENDER_HXX
#include <vclhelperbitmaprender.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRFILLBITMAPATTRIBUTE_HXX
#include <drawinglayer/attribute/sdrfillbitmapattribute.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_FILLBITMAPPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/fillbitmapprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_FILLATTRIBUTE_HXX
#include <drawinglayer/attribute/fillattribute.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POLYPOLYGONPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR2D_VCLHELPERGRADIENT_HXX
#include <vclhelpergradient.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_METAFILEPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/metafileprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MASKPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#endif

#ifndef _BGFX_POLYPOLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR2D_VCLHELPERBUFFERDEVICE_HXX
#include <vclhelperbufferdevice.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MODIFIEDCOLORPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/modifiedcolorprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_ALPHAPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/alphaprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TRANSFORMPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MARKERARRAYPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/markerarrayprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POINTRARRAYPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/pointarrayprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_WRONGSPELLPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/wrongspellprimitive2d.hxx>
#endif

#ifndef _SVTOOLS_CTLOPTIONS_HXX
#include <svtools/ctloptions.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_PAGEPREVIEWPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/pagepreviewprimitive2d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////
// control support

#ifndef _COM_SUN_STAR_AWT_XWINDOW2_HPP_
#include <com/sun/star/awt/XWindow2.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XVIEW_HPP_
#include <com/sun/star/awt/XView.hpp>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_CONTROLPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/controlprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_TEXTLAYOUTDEVICE_HXX
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////
// for test, can be removed again

#ifndef _BGFX_POLYPOLYGON_B2DPOLYGONCLIPPER_HXX
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor2d
    {
        //////////////////////////////////////////////////////////////////////////////
        // UNO class usages
        using ::com::sun::star::uno::Reference;
        using ::com::sun::star::uno::UNO_QUERY;
        using ::com::sun::star::uno::UNO_QUERY_THROW;
        using ::com::sun::star::awt::XView;
        using ::com::sun::star::awt::XGraphics;
        using ::com::sun::star::awt::XWindow;
        using ::com::sun::star::awt::PosSize::POSSIZE;

        //////////////////////////////////////////////////////////////////////////////
        // rendering support

        // directdraw of text simple portion or decorated portion primitive. When decorated, all the extra
        // information is translated to VCL parameters and set at the font.
        // Acceptance is restricted to no shearing and positive scaling in X and Y (no font mirroring
        // for VCL)
        void VclProcessor2D::RenderTextSimpleOrDecoratedPortionPrimitive2D(const primitive2d::TextSimplePortionPrimitive2D& rTextCandidate)
        {
            // decompose matrix to have position and size of text
            basegfx::B2DHomMatrix aLocalTransform(maCurrentTransformation * rTextCandidate.getTextTransform());
            basegfx::B2DVector aScale, aTranslate;
            double fRotate, fShearX;
            aLocalTransform.decompose(aScale, aTranslate, fRotate, fShearX);
            bool bPrimitiveAccepted(false);

            if(basegfx::fTools::equalZero(fShearX))
            {
                if(basegfx::fTools::less(aScale.getX(), 0.0) && basegfx::fTools::less(aScale.getY(), 0.0))
                {
                    // handle special case: If scale is negative in (x,y) (3rd quadrant), it can
                    // be expressed as rotation by PI
                    aScale = basegfx::absolute(aScale);
                    fRotate += F_PI;
                }

                if(basegfx::fTools::more(aScale.getX(), 0.0) && basegfx::fTools::more(aScale.getY(), 0.0))
                {
                    // prepare everything that is not sheared and mirrored
                    Font aFont(primitive2d::getVclFontFromFontAttributes(
                        rTextCandidate.getFontAttributes(), aScale.getX(), aScale.getY(), fRotate, *mpOutputDevice));

                    // handle additional font attributes
                    const primitive2d::TextDecoratedPortionPrimitive2D* pTCPP =
                        dynamic_cast<const primitive2d::TextDecoratedPortionPrimitive2D*>( &rTextCandidate );

                    if( pTCPP != NULL )
                    {
                        // set Underline attribute
                        FontUnderline eFontUnderline = UNDERLINE_NONE;
                        switch( pTCPP->getFontUnderline() )
                        {
                            default:
                                DBG_WARNING1( "DrawingLayer: Unknown underline attribute (%d)!", pTCPP->getFontUnderline() );
                                // fall through
                            case primitive2d::FONT_UNDERLINE_NONE:      eFontUnderline = UNDERLINE_NONE; break;
                            case primitive2d::FONT_UNDERLINE_SINGLE:    eFontUnderline = UNDERLINE_SINGLE; break;
                            case primitive2d::FONT_UNDERLINE_DOUBLE:    eFontUnderline = UNDERLINE_DOUBLE; break;
                            case primitive2d::FONT_UNDERLINE_DOTTED:    eFontUnderline = UNDERLINE_DOTTED; break;
                            case primitive2d::FONT_UNDERLINE_DASH:      eFontUnderline = UNDERLINE_DASH; break;
                            case primitive2d::FONT_UNDERLINE_LONGDASH:  eFontUnderline = UNDERLINE_LONGDASH; break;
                            case primitive2d::FONT_UNDERLINE_DASHDOT:   eFontUnderline = UNDERLINE_DASHDOT; break;
                            case primitive2d::FONT_UNDERLINE_DASHDOTDOT:eFontUnderline = UNDERLINE_DASHDOTDOT; break;
                            case primitive2d::FONT_UNDERLINE_SMALLWAVE: eFontUnderline = UNDERLINE_SMALLWAVE; break;
                            case primitive2d::FONT_UNDERLINE_WAVE:      eFontUnderline = UNDERLINE_WAVE; break;
                            case primitive2d::FONT_UNDERLINE_DOUBLEWAVE:eFontUnderline = UNDERLINE_DOUBLEWAVE; break;
                            case primitive2d::FONT_UNDERLINE_BOLD:      eFontUnderline = UNDERLINE_BOLD; break;
                            case primitive2d::FONT_UNDERLINE_BOLDDOTTED:eFontUnderline = UNDERLINE_BOLDDOTTED; break;
                            case primitive2d::FONT_UNDERLINE_BOLDDASH:  eFontUnderline = UNDERLINE_BOLDDASH; break;
                            case primitive2d::FONT_UNDERLINE_BOLDLONGDASH:eFontUnderline = UNDERLINE_LONGDASH; break;
                            case primitive2d::FONT_UNDERLINE_BOLDDASHDOT:eFontUnderline = UNDERLINE_BOLDDASHDOT; break;
                            case primitive2d::FONT_UNDERLINE_BOLDDASHDOTDOT:eFontUnderline = UNDERLINE_BOLDDASHDOT; break;
                            case primitive2d::FONT_UNDERLINE_BOLDWAVE:  eFontUnderline = UNDERLINE_BOLDWAVE; break;
                        }

                        if( eFontUnderline != UNDERLINE_NONE )
                        {
                            aFont.SetUnderline( eFontUnderline );
                            if( pTCPP->getWordLineMode() )
                                aFont.SetWordLineMode( true );
//TODO: ???                 if( pTCPP->getUnderlineAbove() )
//                              aFont.SetUnderlineAbove( true );
                        }

                        // set Strikeout attribute
                        FontStrikeout eFontStrikeout = STRIKEOUT_NONE;
                        switch( pTCPP->getFontStrikeout() )
                        {
                            default:
                                DBG_WARNING1( "DrawingLayer: Unknown strikeout attribute (%d)!", pTCPP->getFontUnderline() );
                                // fall through
                            case primitive2d::FONT_STRIKEOUT_NONE:      eFontStrikeout = STRIKEOUT_NONE; break;
                            case primitive2d::FONT_STRIKEOUT_SINGLE:    eFontStrikeout = STRIKEOUT_SINGLE; break;
                            case primitive2d::FONT_STRIKEOUT_DOUBLE:    eFontStrikeout = STRIKEOUT_DOUBLE; break;
                            case primitive2d::FONT_STRIKEOUT_BOLD:      eFontStrikeout = STRIKEOUT_BOLD; break;
                            case primitive2d::FONT_STRIKEOUT_SLASH:     eFontStrikeout = STRIKEOUT_SLASH; break;
                            case primitive2d::FONT_STRIKEOUT_X:         eFontStrikeout = STRIKEOUT_X; break;
                        }

                        if( eFontStrikeout != STRIKEOUT_NONE )
                            aFont.SetStrikeout( eFontStrikeout );

                        // set EmphasisMark attribute
                        FontEmphasisMark eFontEmphasisMark = EMPHASISMARK_NONE;
                        switch( pTCPP->getFontEmphasisMark() )
                        {
                            default:
                                DBG_WARNING1( "DrawingLayer: Unknown EmphasisMark style (%d)!", pTCPP->getFontEmphasisMark() );
                                // fall through
                            case primitive2d::FONT_EMPHASISMARK_NONE:   eFontEmphasisMark = EMPHASISMARK_NONE; break;
                            case primitive2d::FONT_EMPHASISMARK_DOT:    eFontEmphasisMark = EMPHASISMARK_DOT; break;
                            case primitive2d::FONT_EMPHASISMARK_CIRCLE: eFontEmphasisMark = EMPHASISMARK_CIRCLE; break;
                            case primitive2d::FONT_EMPHASISMARK_DISC:   eFontEmphasisMark = EMPHASISMARK_DISC; break;
                            case primitive2d::FONT_EMPHASISMARK_ACCENT: eFontEmphasisMark = EMPHASISMARK_ACCENT; break;
                        }

                        if( eFontEmphasisMark != EMPHASISMARK_NONE )
                        {
                            DBG_ASSERT( (pTCPP->getEmphasisMarkAbove() != pTCPP->getEmphasisMarkBelow()),
                                "DrawingLayer: Bad EmphasisMark position!" );
                            if( pTCPP->getEmphasisMarkAbove() )
                                eFontEmphasisMark |= EMPHASISMARK_POS_ABOVE;
                            else
                                eFontEmphasisMark |= EMPHASISMARK_POS_BELOW;
                            aFont.SetEmphasisMark( eFontEmphasisMark );
                        }

                        // set Relief attribute
                        FontRelief eFontRelief = RELIEF_NONE;
                        switch( pTCPP->getFontRelief() )
                        {
                            default:
                                DBG_WARNING1( "DrawingLayer: Unknown Relief style (%d)!", pTCPP->getFontRelief() );
                                // fall through
                            case primitive2d::FONT_RELIEF_NONE:     eFontRelief = RELIEF_NONE; break;
                            case primitive2d::FONT_RELIEF_EMBOSSED: eFontRelief = RELIEF_EMBOSSED; break;
                            case primitive2d::FONT_RELIEF_ENGRAVED: eFontRelief = RELIEF_ENGRAVED; break;
                        }

                        if( eFontRelief != RELIEF_NONE )
                            aFont.SetRelief( eFontRelief );

                        // set Shadow attribute
                        if( pTCPP->getShadow() )
                            aFont.SetShadow( true );
                    }

                    // create transformed integer DXArray in view coordinate system
                    ::std::vector< sal_Int32 > aTransformedDXArray;

                    if(rTextCandidate.getDXArray().size())
                    {
                        aTransformedDXArray.reserve(rTextCandidate.getDXArray().size());
                        const basegfx::B2DVector aPixelVector(aLocalTransform * basegfx::B2DVector(1.0, 0.0));
                        const double fPixelVectorLength(aPixelVector.getLength());

                        for(::std::vector< double >::const_iterator aStart(rTextCandidate.getDXArray().begin()); aStart != rTextCandidate.getDXArray().end(); aStart++)
                        {
                            aTransformedDXArray.push_back(basegfx::fround((*aStart) * fPixelVectorLength));
                        }
                    }

                    // set parameters and paint text snippet
                    const basegfx::BColor aRGBFontColor(maBColorModifierStack.getModifiedColor(rTextCandidate.getFontColor()));
                    const basegfx::B2DPoint aPoint(aLocalTransform * basegfx::B2DPoint(0.0, 0.0));
                    const Point aStartPoint(basegfx::fround(aPoint.getX()), basegfx::fround(aPoint.getY()));
                    const sal_uInt32 nOldLayoutMode(mpOutputDevice->GetLayoutMode());

                    if(rTextCandidate.getFontAttributes().getRTL())
                    {
                        sal_uInt32 nRTLLayoutMode(nOldLayoutMode & ~(TEXT_LAYOUT_COMPLEX_DISABLED|TEXT_LAYOUT_BIDI_STRONG));
                        nRTLLayoutMode |= TEXT_LAYOUT_BIDI_RTL|TEXT_LAYOUT_TEXTORIGIN_LEFT;
                        mpOutputDevice->SetLayoutMode(nRTLLayoutMode);
                    }

                    mpOutputDevice->SetFont(aFont);
                    mpOutputDevice->SetTextColor(Color(aRGBFontColor));

                    mpOutputDevice->DrawTextArray(
                        aStartPoint,
                        rTextCandidate.getText(),
                        aTransformedDXArray.size() ? &(aTransformedDXArray[0]) : NULL,
                        rTextCandidate.getTextPosition(),
                        rTextCandidate.getTextLength());

                    if(rTextCandidate.getFontAttributes().getRTL())
                    {
                        mpOutputDevice->SetLayoutMode(nOldLayoutMode);
                    }

                    bPrimitiveAccepted = true;
                }
            }

            if(!bPrimitiveAccepted)
            {
                // let break down
                process(rTextCandidate.get2DDecomposition(getViewInformation2D()));
            }
        }

        // direct draw of hairline
        void VclProcessor2D::RenderPolygonHairlinePrimitive2D(const primitive2d::PolygonHairlinePrimitive2D& rPolygonCandidate)
        {
            const basegfx::BColor aHairlineColor(maBColorModifierStack.getModifiedColor(rPolygonCandidate.getBColor()));
            mpOutputDevice->SetLineColor(Color(aHairlineColor));
            mpOutputDevice->SetFillColor();

            basegfx::B2DPolygon aLocalPolygon(rPolygonCandidate.getB2DPolygon());
            aLocalPolygon.transform(maCurrentTransformation);
            mpOutputDevice->DrawPolyLine(aLocalPolygon, 0.0);
        }

        // direct draw of transformed BitmapEx primitive
        void VclProcessor2D::RenderBitmapPrimitive2D(const primitive2d::BitmapPrimitive2D& rBitmapCandidate)
        {
            // create local transform
            basegfx::B2DHomMatrix aLocalTransform(maCurrentTransformation * rBitmapCandidate.getTransform());
            BitmapEx aBitmapEx(rBitmapCandidate.getBitmapEx());
            bool bPainted(false);

            if(maBColorModifierStack.count())
            {
                aBitmapEx = impModifyBitmapEx(maBColorModifierStack, aBitmapEx);

                if(aBitmapEx.IsEmpty())
                {
                    // color gets completely replaced, get it
                    const basegfx::BColor aModifiedColor(maBColorModifierStack.getModifiedColor(basegfx::BColor()));
                    basegfx::B2DPolygon aPolygon(basegfx::tools::createPolygonFromRect(basegfx::B2DRange(0.0, 0.0, 1.0, 1.0)));
                    aPolygon.transform(aLocalTransform);

                    mpOutputDevice->SetFillColor(Color(aModifiedColor));
                    mpOutputDevice->SetLineColor();
                    mpOutputDevice->DrawPolygon(aPolygon);

                    bPainted = true;
                }
            }

            if(!bPainted)
            {
                static bool bForceUseOfOwnTransformer(false);
                static bool bUseGraphicManager(true);

                // decompose matrix to check for shear, rotate and mirroring
                basegfx::B2DVector aScale, aTranslate;
                double fRotate, fShearX;
                aLocalTransform.decompose(aScale, aTranslate, fRotate, fShearX);

                if(!bForceUseOfOwnTransformer && basegfx::fTools::equalZero(fShearX))
                {
                    if(!bUseGraphicManager && basegfx::fTools::equalZero(fRotate))
                    {
                        RenderBitmapPrimitive2D_BitmapEx(*mpOutputDevice, aBitmapEx, aLocalTransform);
                    }
                    else
                    {
                        RenderBitmapPrimitive2D_GraphicManager(*mpOutputDevice, aBitmapEx, aLocalTransform);
                    }
                }
                else
                {
                    if(!aBitmapEx.IsTransparent() && (!basegfx::fTools::equalZero(fShearX) || !basegfx::fTools::equalZero(fRotate)))
                    {
                        // parts will be uncovered, extend aBitmapEx with a mask bitmap
                        const Bitmap aContent(aBitmapEx.GetBitmap());
                        aBitmapEx = BitmapEx(aContent, Bitmap(aContent.GetSizePixel(), 1));
                    }

                    RenderBitmapPrimitive2D_self(*mpOutputDevice, aBitmapEx, aLocalTransform);
                }
            }
        }

        void VclProcessor2D::RenderFillBitmapPrimitive2D(const primitive2d::FillBitmapPrimitive2D& rFillBitmapCandidate)
        {
            const attribute::FillBitmapAttribute& rFillBitmapAttribute(rFillBitmapCandidate.getFillBitmap());
            bool bPrimitiveAccepted(false);

            if(rFillBitmapAttribute.getTiling())
            {
                // decompose matrix to check for shear, rotate and mirroring
                basegfx::B2DHomMatrix aLocalTransform(maCurrentTransformation * rFillBitmapCandidate.getTransformation());
                basegfx::B2DVector aScale, aTranslate;
                double fRotate, fShearX;
                aLocalTransform.decompose(aScale, aTranslate, fRotate, fShearX);

                if(basegfx::fTools::equalZero(fRotate) && basegfx::fTools::equalZero(fShearX))
                {
                    // no shear or rotate, draw direct in pixel coordinates
                    bPrimitiveAccepted = true;
                    BitmapEx aBitmapEx(rFillBitmapAttribute.getBitmap());
                    bool bPainted(false);

                    if(maBColorModifierStack.count())
                    {
                        aBitmapEx = impModifyBitmapEx(maBColorModifierStack, aBitmapEx);

                        if(aBitmapEx.IsEmpty())
                        {
                            // color gets completely replaced, get it
                            const basegfx::BColor aModifiedColor(maBColorModifierStack.getModifiedColor(basegfx::BColor()));
                            basegfx::B2DPolygon aPolygon(basegfx::tools::createPolygonFromRect(basegfx::B2DRange(0.0, 0.0, 1.0, 1.0)));
                            aPolygon.transform(aLocalTransform);

                            mpOutputDevice->SetFillColor(Color(aModifiedColor));
                            mpOutputDevice->SetLineColor();
                            mpOutputDevice->DrawPolygon(aPolygon);

                            bPainted = true;
                        }
                    }

                    if(!bPainted)
                    {
                        const basegfx::B2DPoint aObjTopLeft(aTranslate.getX(), aTranslate.getY());
                        const basegfx::B2DPoint aObjBottomRight(aTranslate.getX() + aScale.getX(), aTranslate.getY() + aScale.getY());
                        const Point aObjTL(mpOutputDevice->LogicToPixel(Point((sal_Int32)aObjTopLeft.getX(), (sal_Int32)aObjTopLeft.getY())));
                        const Point aObjBR(mpOutputDevice->LogicToPixel(Point((sal_Int32)aObjBottomRight.getX(), (sal_Int32)aObjBottomRight.getY())));

                        const basegfx::B2DPoint aBmpTopLeft(aLocalTransform * rFillBitmapAttribute.getTopLeft());
                        const basegfx::B2DPoint aBmpBottomRight(aLocalTransform * basegfx::B2DPoint(rFillBitmapAttribute.getTopLeft() + rFillBitmapAttribute.getSize()));
                        const Point aBmpTL(mpOutputDevice->LogicToPixel(Point((sal_Int32)aBmpTopLeft.getX(), (sal_Int32)aBmpTopLeft.getY())));
                        const Point aBmpBR(mpOutputDevice->LogicToPixel(Point((sal_Int32)aBmpBottomRight.getX(), (sal_Int32)aBmpBottomRight.getY())));

                        sal_Int32 nOWidth(aObjBR.X() - aObjTL.X());
                        sal_Int32 nOHeight(aObjBR.Y() - aObjTL.Y());

                        if(nOWidth < 0L)
                        {
                            nOWidth = 1L;
                        }

                        if(nOHeight < 0L)
                        {
                            nOHeight = 1L;
                        }

                        sal_Int32 nBWidth(aBmpBR.X() - aBmpTL.X());
                        sal_Int32 nBHeight(aBmpBR.Y() - aBmpTL.Y());

                        if(nBWidth < 0L)
                        {
                            nBWidth = 1L;
                        }

                        if(nBHeight < 0L)
                        {
                            nBHeight = 1L;
                        }

                        sal_Int32 nBLeft(aBmpTL.X());
                        sal_Int32 nBTop(aBmpTL.Y());

                        if(nBLeft > aObjTL.X())
                        {
                            nBLeft -= ((nBLeft / nBWidth) + 1L) * nBWidth;
                        }

                        if(nBLeft + nBWidth <= aObjTL.X())
                        {
                            nBLeft -= (nBLeft / nBWidth) * nBWidth;
                        }

                        if(nBTop > aObjTL.Y())
                        {
                            nBTop -= ((nBTop / nBHeight) + 1L) * nBHeight;
                        }

                        if(nBTop + nBHeight <= aObjTL.Y())
                        {
                            nBTop -= (nBTop / nBHeight) * nBHeight;
                        }

                        // nBWidth, nBHeight is the pixel size of the neede bitmap. To not need to scale it
                        // in vcl many times, create a size-optimized version
                        const Size aNeededBitmapSizePixel(nBWidth, nBHeight);

                        if(aNeededBitmapSizePixel != aBitmapEx.GetSizePixel())
                        {
                            aBitmapEx.Scale(aNeededBitmapSizePixel);
                        }

                        // prepare OutDev
                        const Point aEmptyPoint(0, 0);
                        const Rectangle aVisiblePixel(aEmptyPoint, mpOutputDevice->GetOutputSizePixel());
                        const bool bWasEnabled(mpOutputDevice->IsMapModeEnabled());
                        mpOutputDevice->EnableMapMode(false);

                        for(sal_Int32 nXPos(nBLeft); nXPos < aObjTL.X() + nOWidth; nXPos += nBWidth)
                        {
                            for(sal_Int32 nYPos(nBTop); nYPos < aObjTL.Y() + nOHeight; nYPos += nBHeight)
                            {
                                const Rectangle aOutRectPixel(Point(nXPos, nYPos), aNeededBitmapSizePixel);

                                if(aOutRectPixel.IsOver(aVisiblePixel))
                                {
                                    mpOutputDevice->DrawBitmapEx(aOutRectPixel.TopLeft(), aBitmapEx);
                                }
                            }
                        }

                        // restore OutDev
                        mpOutputDevice->EnableMapMode(bWasEnabled);
                    }
                }
            }

            if(!bPrimitiveAccepted)
            {
                // do not accept, use decomposition
                process(rFillBitmapCandidate.get2DDecomposition(getViewInformation2D()));
            }
        }

        // direct draw of gradient
        void VclProcessor2D::RenderPolyPolygonGradientPrimitive2D(const primitive2d::PolyPolygonGradientPrimitive2D& rPolygonCandidate)
        {
            const attribute::FillGradientAttribute& rGradient(rPolygonCandidate.getFillGradient());
            basegfx::BColor aStartColor(maBColorModifierStack.getModifiedColor(rGradient.getStartColor()));
            basegfx::BColor aEndColor(maBColorModifierStack.getModifiedColor(rGradient.getEndColor()));
            basegfx::B2DPolyPolygon aLocalPolyPolygon(rPolygonCandidate.getB2DPolyPolygon());
            aLocalPolyPolygon.transform(maCurrentTransformation);

            if(aStartColor == aEndColor)
            {
                // no gradient at all, draw as polygon
                mpOutputDevice->SetLineColor();
                mpOutputDevice->SetFillColor(Color(aStartColor));
                mpOutputDevice->DrawPolyPolygon(aLocalPolyPolygon);
            }
            else
            {
                impDrawGradientToOutDev(
                    *mpOutputDevice, aLocalPolyPolygon, rGradient.getStyle(), rGradient.getSteps(),
                    aStartColor, aEndColor, rGradient.getBorder(),
                    -rGradient.getAngle(), rGradient.getOffsetX(), rGradient.getOffsetY(), false);
            }
        }

        // direct draw of PolyPolygon with color
        void VclProcessor2D::RenderPolyPolygonColorPrimitive2D(const primitive2d::PolyPolygonColorPrimitive2D& rPolygonCandidate)
        {
            const basegfx::BColor aPolygonColor(maBColorModifierStack.getModifiedColor(rPolygonCandidate.getBColor()));
            mpOutputDevice->SetFillColor(Color(aPolygonColor));
            mpOutputDevice->SetLineColor();

            basegfx::B2DPolyPolygon aLocalPolyPolygon(rPolygonCandidate.getB2DPolyPolygon());
            aLocalPolyPolygon.transform(maCurrentTransformation);
            mpOutputDevice->DrawPolyPolygon(aLocalPolyPolygon);

            if(mnPolygonStrokePrimitive2D && getOptionsDrawinglayer().IsAntiAliasing())
            {
                // when AA is on and this filled polygons are the result of stroked line geometry,
                // draw the geometry once extra as lines to avoid AA 'gaps' between partial polygons
                mpOutputDevice->SetFillColor();
                mpOutputDevice->SetLineColor(Color(aPolygonColor));
                const sal_uInt32 nCount(aLocalPolyPolygon.count());

                for(sal_uInt32 a(0); a < nCount; a++)
                {
                    mpOutputDevice->DrawPolyLine(aLocalPolyPolygon.getB2DPolygon(a), 0.0);
                }
            }

            static bool bTestPolygonClipping(false);
            if(bTestPolygonClipping)
            {
                static bool bInside(true);
                static bool bFilled(false);
                static bool bLine(false);

                basegfx::B2DRange aRange(aLocalPolyPolygon.getB2DRange());
                aRange.grow(aRange.getWidth() * -0.1);

                if(bFilled)
                {
                    basegfx::B2DPolyPolygon aFilledClipped(basegfx::tools::clipPolyPolygonOnRange(aLocalPolyPolygon, aRange, bInside, false));
                    basegfx::BColor aRand(rand() / 32767.0, rand() / 32767.0, rand() / 32767.0);
                    mpOutputDevice->SetFillColor(Color(aRand));
                    mpOutputDevice->SetLineColor();
                    mpOutputDevice->DrawPolyPolygon(aFilledClipped);
                }

                if(bLine)
                {
                    basegfx::B2DPolyPolygon aLineClipped(basegfx::tools::clipPolyPolygonOnRange(aLocalPolyPolygon, aRange, bInside, true));
                    basegfx::BColor aRand(rand() / 32767.0, rand() / 32767.0, rand() / 32767.0);
                    mpOutputDevice->SetFillColor();
                    mpOutputDevice->SetLineColor(Color(aRand));

                    for(sal_uInt32 a(0); a < aLineClipped.count(); a++)
                    {
                        mpOutputDevice->DrawPolyLine(aLineClipped.getB2DPolygon(a), 0.0);
                    }
                }
            }
        }

        // direct draw of MetaFile
        void VclProcessor2D::RenderMetafilePrimitive2D(const primitive2d::MetafilePrimitive2D& rMetaCandidate)
        {
            // decompose matrix to check for shear, rotate and mirroring
            basegfx::B2DHomMatrix aLocalTransform(maCurrentTransformation * rMetaCandidate.getTransform());
            basegfx::B2DVector aScale, aTranslate;
            double fRotate, fShearX;
            aLocalTransform.decompose(aScale, aTranslate, fRotate, fShearX);

            // get BoundRect
            basegfx::B2DRange aOutlineRange(rMetaCandidate.getB2DRange(getViewInformation2D()));
            aOutlineRange.transform(maCurrentTransformation);
            const Rectangle aDestRectView(
                basegfx::fround(aOutlineRange.getMinX()), basegfx::fround(aOutlineRange.getMinY()),
                basegfx::fround(aOutlineRange.getMaxX()), basegfx::fround(aOutlineRange.getMaxY()));

            // get metafile (copy it)
            GDIMetaFile aMetaFile;

            if(maBColorModifierStack.count())
            {
                const basegfx::BColor aRGBBaseColor(0, 0, 0);
                const basegfx::BColor aRGBColor(maBColorModifierStack.getModifiedColor(aRGBBaseColor));
                aMetaFile = rMetaCandidate.getMetaFile().GetMonochromeMtf(Color(aRGBColor));
            }
            else
            {
                aMetaFile = rMetaCandidate.getMetaFile();
            }

            // rotation
            if(!basegfx::fTools::equalZero(fRotate))
            {
                double fRotation((fRotate / F_PI180) * -10.0);
                aMetaFile.Rotate((sal_uInt16)(fRotation));
            }

            // paint it
            aMetaFile.WindStart();
            aMetaFile.Play(mpOutputDevice, aDestRectView.TopLeft(), aDestRectView.GetSize());
        }

        // mask group. Force output to VDev and create mask from given mask
        void VclProcessor2D::RenderMaskPrimitive2DPixel(const primitive2d::MaskPrimitive2D& rMaskCandidate)
        {
            if(rMaskCandidate.getChildren().hasElements())
            {
                basegfx::B2DPolyPolygon aMask(rMaskCandidate.getMask());

                if(aMask.count())
                {
                    aMask.transform(maCurrentTransformation);
                    const basegfx::B2DRange aRange(basegfx::tools::getRange(aMask));
                    impBufferDevice aBufferDevice(*mpOutputDevice, aRange);

                    if(aBufferDevice.isVisible())
                    {
                        // remember last OutDev and set to content
                        OutputDevice* pLastOutputDevice = mpOutputDevice;
                        mpOutputDevice = &aBufferDevice.getContent();

                        // paint to it
                        process(rMaskCandidate.getChildren());

                        // back to old OutDev
                        mpOutputDevice = pLastOutputDevice;

                        // draw mask
                        if(getOptionsDrawinglayer().IsAntiAliasing())
                        {
                            // with AA, use 8bit AlphaMask to get nice borders
                            VirtualDevice& rAlpha = aBufferDevice.getAlpha();
                            rAlpha.SetLineColor();
                            rAlpha.SetFillColor(COL_BLACK);
                            rAlpha.DrawPolyPolygon(aMask);

                            // dump buffer to outdev
                            aBufferDevice.paint();
                        }
                        else
                        {
                            // No AA, use 1bit mask
                            VirtualDevice& rMask = aBufferDevice.getMask();
                            rMask.SetLineColor();
                            rMask.SetFillColor(COL_BLACK);
                            rMask.DrawPolyPolygon(aMask);

                            // dump buffer to outdev
                            aBufferDevice.paint();
                        }
                    }
                }
            }
        }

        // modified color group. Force output to unified color.
        void VclProcessor2D::RenderModifiedColorPrimitive2D(const primitive2d::ModifiedColorPrimitive2D& rModifiedCandidate)
        {
            if(rModifiedCandidate.getChildren().hasElements())
            {
                maBColorModifierStack.push(rModifiedCandidate.getColorModifier());
                process(rModifiedCandidate.getChildren());
                maBColorModifierStack.pop();
            }
        }

        // sub-transparence group. Draw to VDev first.
        void VclProcessor2D::RenderAlphaPrimitive2D(const primitive2d::AlphaPrimitive2D& rTransCandidate)
        {
            if(rTransCandidate.getChildren().hasElements())
            {
                basegfx::B2DRange aRange(primitive2d::getB2DRangeFromPrimitive2DSequence(rTransCandidate.getChildren(), getViewInformation2D()));
                aRange.transform(maCurrentTransformation);
                impBufferDevice aBufferDevice(*mpOutputDevice, aRange);

                if(aBufferDevice.isVisible())
                {
                    // remember last OutDev and set to content
                    OutputDevice* pLastOutputDevice = mpOutputDevice;
                    mpOutputDevice = &aBufferDevice.getContent();

                    // paint content to it
                    process(rTransCandidate.getChildren());

                    // set to mask
                    mpOutputDevice = &aBufferDevice.getAlpha();

                    // when painting alpha masks, reset the color stack
                    basegfx::BColorModifierStack aLastBColorModifierStack(maBColorModifierStack);
                    maBColorModifierStack = basegfx::BColorModifierStack();

                    // paint mask to it (always with alpha intensities, evtl. with AA)
                    process(rTransCandidate.getAlpha());

                    // back to old color stack
                    maBColorModifierStack = aLastBColorModifierStack;

                    // back to old OutDev
                    mpOutputDevice = pLastOutputDevice;

                    // dump buffer to outdev
                    aBufferDevice.paint();
                }
            }
        }

        // transform group.
        void VclProcessor2D::RenderTransformPrimitive2D(const primitive2d::TransformPrimitive2D& rTransformCandidate)
        {
            // remember current transformation and ViewInformation
            const basegfx::B2DHomMatrix aLastCurrentTransformation(maCurrentTransformation);
            const geometry::ViewInformation2D aLastViewInformation2D(getViewInformation2D());

            // create new transformations for CurrentTransformation
            // and for local ViewInformation2D
            maCurrentTransformation = maCurrentTransformation * rTransformCandidate.getTransformation();
            maViewInformation2D = geometry::ViewInformation2D(
                getViewInformation2D().getViewTransformation() * rTransformCandidate.getTransformation(),
                getViewInformation2D().getViewport(),
                getViewInformation2D().getVisualizedPage(),
                getViewInformation2D().getViewTime(),
                getViewInformation2D().getExtendedInformationSequence());

            // proccess content
            process(rTransformCandidate.getChildren());

            // restore transformations
            maCurrentTransformation = aLastCurrentTransformation;
            maViewInformation2D = aLastViewInformation2D;
        }

        // new XDrawPage for ViewInformation2D
        void VclProcessor2D::RenderPagePreviewPrimitive2D(const primitive2d::PagePreviewPrimitive2D& rPagePreviewCandidate)
        {
            // remember current transformation and ViewInformation
            const geometry::ViewInformation2D aLastViewInformation2D(getViewInformation2D());

            // create new local ViewInformation2D
            maViewInformation2D = geometry::ViewInformation2D(
                getViewInformation2D().getViewTransformation(),
                getViewInformation2D().getViewport(),
                rPagePreviewCandidate.getXDrawPage(),
                getViewInformation2D().getViewTime(),
                getViewInformation2D().getExtendedInformationSequence());

            // proccess decomposed content
            process(rPagePreviewCandidate.get2DDecomposition(getViewInformation2D()));

            // restore transformations
            maViewInformation2D = aLastViewInformation2D;
        }

        // marker
        void VclProcessor2D::RenderMarkerArrayPrimitive2D(const primitive2d::MarkerArrayPrimitive2D& rMarkArrayCandidate)
        {
            static bool bCheckCompleteMarkerDecompose(false);
            if(bCheckCompleteMarkerDecompose)
            {
                process(rMarkArrayCandidate.get2DDecomposition(getViewInformation2D()));
                return;
            }

            switch(rMarkArrayCandidate.getStyle())
            {
                default :
                {
                    // not handled/unknown MarkerArrayPrimitive2D, use decomposition
                    process(rMarkArrayCandidate.get2DDecomposition(getViewInformation2D()));
                    break;
                }
                case primitive2d::MARKERSTYLE2D_CROSS :
                case primitive2d::MARKERSTYLE2D_GLUEPOINT :
                {
                    // directly supported markers
                    const std::vector< basegfx::B2DPoint >& rPositions = rMarkArrayCandidate.getPositions();
                    const basegfx::BColor aRGBColor(maBColorModifierStack.getModifiedColor(rMarkArrayCandidate.getRGBColor()));
                    const Color aVCLColor(aRGBColor);
                    const bool bWasEnabled(mpOutputDevice->IsMapModeEnabled());

                    for(std::vector< basegfx::B2DPoint >::const_iterator aIter(rPositions.begin()); aIter != rPositions.end(); aIter++)
                    {
                        const basegfx::B2DPoint aViewPosition(maCurrentTransformation * (*aIter));
                        Point aPos(basegfx::fround(aViewPosition.getX()), basegfx::fround(aViewPosition.getY()));

                        aPos = mpOutputDevice->LogicToPixel(aPos);
                        mpOutputDevice->EnableMapMode(false);

                        switch(rMarkArrayCandidate.getStyle())
                        {
                            default :
                            {
                                // this would be an error, ther cases here need to be consistent with the initially
                                // accepted ones
                                OSL_ENSURE(false, "Inconsistent RenderMarkerArrayPrimitive2D implementation (!)");
                                break;
                            }
                            case primitive2d::MARKERSTYLE2D_CROSS :
                            {
                                mpOutputDevice->DrawPixel(aPos, aVCLColor);
                                mpOutputDevice->DrawPixel(Point(aPos.X() - 1L, aPos.Y()), aVCLColor);
                                mpOutputDevice->DrawPixel(Point(aPos.X() + 1L, aPos.Y()), aVCLColor);
                                mpOutputDevice->DrawPixel(Point(aPos.X(), aPos.Y() - 1L), aVCLColor);
                                mpOutputDevice->DrawPixel(Point(aPos.X(), aPos.Y() + 1L), aVCLColor);

                                break;
                            }
                            case primitive2d::MARKERSTYLE2D_GLUEPOINT :
                            {
                                // backpen
                                mpOutputDevice->SetLineColor(aVCLColor);
                                mpOutputDevice->DrawLine(aPos + Point(-2, -3), aPos + Point(+3, +2));
                                mpOutputDevice->DrawLine(aPos + Point(-3, -2), aPos + Point(+2, +3));
                                mpOutputDevice->DrawLine(aPos + Point(-3, +2), aPos + Point(+2, -3));
                                mpOutputDevice->DrawLine(aPos + Point(-2, +3), aPos + Point(+3, -2));

                                // frontpen (hard coded)
                                const basegfx::BColor aRGBFrontColor(maBColorModifierStack.getModifiedColor(Color(COL_LIGHTBLUE).getBColor()));
                                mpOutputDevice->SetLineColor(Color(aRGBFrontColor));
                                mpOutputDevice->DrawLine(aPos + Point(-2, -2), aPos + Point(+2, +2));
                                mpOutputDevice->DrawLine(aPos + Point(-2, +2), aPos + Point(+2, -2));

                                break;
                            }
                        }

                        mpOutputDevice->EnableMapMode(bWasEnabled);
                    }

                    break;
                }
            }
        }

        // point
        void VclProcessor2D::RenderPointArrayPrimitive2D(const primitive2d::PointArrayPrimitive2D& rPointArrayCandidate)
        {
            const std::vector< basegfx::B2DPoint >& rPositions = rPointArrayCandidate.getPositions();
            const basegfx::BColor aRGBColor(maBColorModifierStack.getModifiedColor(rPointArrayCandidate.getRGBColor()));
            const Color aVCLColor(aRGBColor);

            for(std::vector< basegfx::B2DPoint >::const_iterator aIter(rPositions.begin()); aIter != rPositions.end(); aIter++)
            {
                const basegfx::B2DPoint aViewPosition(maCurrentTransformation * (*aIter));
                const Point aPos(basegfx::fround(aViewPosition.getX()), basegfx::fround(aViewPosition.getY()));

                mpOutputDevice->DrawPixel(aPos, aVCLColor);
            }
        }

        // wrong spell primitive
        void VclProcessor2D::RenderWrongSpellPrimitive2D(const primitive2d::WrongSpellPrimitive2D& rWrongSpellCandidate)
        {
            const basegfx::B2DHomMatrix aLocalTransform(maCurrentTransformation * rWrongSpellCandidate.getTransformation());
            const basegfx::B2DVector aFontVectorPixel(aLocalTransform * basegfx::B2DVector(0.0, 1.0));
            const sal_uInt32 nFontPixelHeight(basegfx::fround(aFontVectorPixel.getLength()));

            static const sal_uInt32 nMinimumFontHeight(5); // #define WRONG_SHOW_MIN         5
            static const sal_uInt32 nSmallFontHeight(11);  // #define WRONG_SHOW_SMALL      11
            static const sal_uInt32 nMediumFontHeight(15); // #define WRONG_SHOW_MEDIUM     15

            if(nFontPixelHeight > nMinimumFontHeight)
            {
                const basegfx::B2DPoint aStart(aLocalTransform * basegfx::B2DPoint(rWrongSpellCandidate.getStart(), 0.0));
                const basegfx::B2DPoint aStop(aLocalTransform * basegfx::B2DPoint(rWrongSpellCandidate.getStop(), 0.0));
                const Point aVclStart(basegfx::fround(aStart.getX()), basegfx::fround(aStart.getY()));
                const Point aVclStop(basegfx::fround(aStop.getX()), basegfx::fround(aStop.getY()));
                sal_uInt16 nWaveStyle(WAVE_FLAT);

                if(nFontPixelHeight > nMediumFontHeight)
                {
                    nWaveStyle = WAVE_NORMAL;
                }
                else if(nFontPixelHeight > nSmallFontHeight)
                {
                    nWaveStyle = WAVE_SMALL;
                }

                const basegfx::BColor aProcessedColor(maBColorModifierStack.getModifiedColor(rWrongSpellCandidate.getColor()));
                mpOutputDevice->SetLineColor(Color(aProcessedColor));
                mpOutputDevice->SetFillColor();
                mpOutputDevice->DrawWaveLine(aVclStart, aVclStop, nWaveStyle);
            }
        }

        void VclProcessor2D::RenderPolygonStrokePrimitive2D(const primitive2d::PolygonStrokePrimitive2D& rPolygonStrokeCandidate)
        {
            const attribute::LineAttribute& rLineAttribute = rPolygonStrokeCandidate.getLineAttribute();
            const double fLineWidth(rLineAttribute.getWidth());
            bool bDone(false);

            if(basegfx::fTools::more(fLineWidth, 0.0))
            {
                const basegfx::B2DVector aDiscreteUnit(maCurrentTransformation * basegfx::B2DVector(1.0, 1.0));
                const double fDiscreteLineWidth((fLineWidth * aDiscreteUnit.getX() + fLineWidth * aDiscreteUnit.getY()) * 0.5);

                if(basegfx::fTools::lessOrEqual(fDiscreteLineWidth, 2.5))
                {
                    // force to hairline
                    const attribute::StrokeAttribute& rStrokeAttribute = rPolygonStrokeCandidate.getStrokeAttribute();
                    const basegfx::BColor aHairlineColor(maBColorModifierStack.getModifiedColor(rLineAttribute.getColor()));
                    basegfx::B2DPolyPolygon aHairlinePolyPolygon;

                    mpOutputDevice->SetLineColor(Color(aHairlineColor));
                    mpOutputDevice->SetFillColor();

                    if(0.0 == rStrokeAttribute.getFullDotDashLen())
                    {
                        // no line dashing, just copy
                        aHairlinePolyPolygon.append(rPolygonStrokeCandidate.getB2DPolygon());
                    }
                    else
                    {
                        // else apply LineStyle
                        basegfx::tools::applyLineDashing(rPolygonStrokeCandidate.getB2DPolygon(),
                            rStrokeAttribute.getDotDashArray(),
                            &aHairlinePolyPolygon, 0, rStrokeAttribute.getFullDotDashLen());
                    }

                    const sal_uInt32 nCount(aHairlinePolyPolygon.count());

                    if(nCount)
                    {
                        const bool bNeedQuadro(basegfx::fTools::more(fDiscreteLineWidth, 1.5));
                        aHairlinePolyPolygon.transform(maCurrentTransformation);

                        for(sal_uInt32 a(0); a < nCount; a++)
                        {
                            // draw the basic hairline polygon
                            basegfx::B2DPolygon aCandidate(aHairlinePolyPolygon.getB2DPolygon(a));
                            mpOutputDevice->DrawPolyLine(aCandidate, 0.0);

                            if(bNeedQuadro)
                            {
                                // line width is in range ]1.5 .. 2.5], use four hairlines
                                // drawn in a square. Create the three other ones
                                basegfx::B2DHomMatrix aMat;

                                aMat.set(0, 2, 1);
                                aCandidate.transform(aMat);
                                mpOutputDevice->DrawPolyLine(aCandidate, 0.0);

                                aMat.set(0, 2, 0);
                                aMat.set(1, 2, 1);
                                aCandidate.transform(aMat);
                                mpOutputDevice->DrawPolyLine(aCandidate, 0.0);

                                aMat.set(0, 2, -1);
                                aMat.set(1, 2, 0);
                                aCandidate.transform(aMat);
                                mpOutputDevice->DrawPolyLine(aCandidate, 0.0);
                            }
                        }
                    }

                    bDone = true;
                }
            }

            if(!bDone)
            {
                // line width is big enough for standard filled polygon visualisation or zero
                process(rPolygonStrokeCandidate.get2DDecomposition(getViewInformation2D()));
            }
        }

        void VclProcessor2D::adaptLineToFillDrawMode() const
        {
            const sal_uInt32 nOriginalDrawMode(mpOutputDevice->GetDrawMode());

            if(nOriginalDrawMode & (DRAWMODE_BLACKLINE|DRAWMODE_GRAYLINE|DRAWMODE_GHOSTEDLINE|DRAWMODE_WHITELINE|DRAWMODE_SETTINGSLINE))
            {
                sal_uInt32 nAdaptedDrawMode(nOriginalDrawMode);

                if(nOriginalDrawMode & DRAWMODE_BLACKLINE)
                {
                    nAdaptedDrawMode |= DRAWMODE_BLACKFILL;
                }
                else
                {
                    nAdaptedDrawMode &= ~DRAWMODE_BLACKFILL;
                }

                if(nOriginalDrawMode & DRAWMODE_GRAYLINE)
                {
                    nAdaptedDrawMode |= DRAWMODE_GRAYFILL;
                }
                else
                {
                    nAdaptedDrawMode &= ~DRAWMODE_GRAYFILL;
                }

                if(nOriginalDrawMode & DRAWMODE_GHOSTEDLINE)
                {
                    nAdaptedDrawMode |= DRAWMODE_GHOSTEDFILL;
                }
                else
                {
                    nAdaptedDrawMode &= ~DRAWMODE_GHOSTEDFILL;
                }

                if(nOriginalDrawMode & DRAWMODE_WHITELINE)
                {
                    nAdaptedDrawMode |= DRAWMODE_WHITEFILL;
                }
                else
                {
                    nAdaptedDrawMode &= ~DRAWMODE_WHITEFILL;
                }

                if(nOriginalDrawMode & DRAWMODE_SETTINGSLINE)
                {
                    nAdaptedDrawMode |= DRAWMODE_SETTINGSFILL;
                }
                else
                {
                    nAdaptedDrawMode &= ~DRAWMODE_SETTINGSFILL;
                }

                mpOutputDevice->SetDrawMode(nAdaptedDrawMode);
            }
        }

        void VclProcessor2D::adaptTextToFillDrawMode() const
        {
            const sal_uInt32 nOriginalDrawMode(mpOutputDevice->GetDrawMode());
            if(nOriginalDrawMode & (DRAWMODE_BLACKTEXT|DRAWMODE_GRAYTEXT|DRAWMODE_GHOSTEDTEXT|DRAWMODE_WHITETEXT|DRAWMODE_SETTINGSTEXT))
            {
                sal_uInt32 nAdaptedDrawMode(nOriginalDrawMode);

                if(nOriginalDrawMode & DRAWMODE_BLACKTEXT)
                {
                    nAdaptedDrawMode |= DRAWMODE_BLACKFILL;
                }
                else
                {
                    nAdaptedDrawMode &= ~DRAWMODE_BLACKFILL;
                }

                if(nOriginalDrawMode & DRAWMODE_GRAYTEXT)
                {
                    nAdaptedDrawMode |= DRAWMODE_GRAYFILL;
                }
                else
                {
                    nAdaptedDrawMode &= ~DRAWMODE_GRAYFILL;
                }

                if(nOriginalDrawMode & DRAWMODE_GHOSTEDTEXT)
                {
                    nAdaptedDrawMode |= DRAWMODE_GHOSTEDFILL;
                }
                else
                {
                    nAdaptedDrawMode &= ~DRAWMODE_GHOSTEDFILL;
                }

                if(nOriginalDrawMode & DRAWMODE_WHITETEXT)
                {
                    nAdaptedDrawMode |= DRAWMODE_WHITEFILL;
                }
                else
                {
                    nAdaptedDrawMode &= ~DRAWMODE_WHITEFILL;
                }

                if(nOriginalDrawMode & DRAWMODE_SETTINGSTEXT)
                {
                    nAdaptedDrawMode |= DRAWMODE_SETTINGSFILL;
                }
                else
                {
                    nAdaptedDrawMode &= ~DRAWMODE_SETTINGSFILL;
                }

                mpOutputDevice->SetDrawMode(nAdaptedDrawMode);
            }
        }

        basegfx::B2DPoint VclProcessor2D::PositionAndSizeControl(const primitive2d::ControlPrimitive2D& rControlPrimitive2D)
        {
            // prepare output for given device
            Reference< XGraphics > xGraphics(mpOutputDevice->CreateUnoGraphics());
            Reference< XView > xControlView(rControlPrimitive2D.getXControl(), UNO_QUERY_THROW);
            xControlView->setGraphics(xGraphics);

            // set position and size (in pixel)
            const basegfx::B2DHomMatrix aObjectToPixel(maCurrentTransformation * rControlPrimitive2D.getTransform());
            const basegfx::B2DPoint aTopLeftPixel(aObjectToPixel * basegfx::B2DPoint(0.0, 0.0));
            Reference< XWindow > xControlWindow(rControlPrimitive2D.getXControl(), UNO_QUERY);

            if(xControlWindow.is())
            {
                const basegfx::B2DPoint aBottomRightPixel(aObjectToPixel * basegfx::B2DPoint(1.0, 1.0));

                xControlWindow->setPosSize(
                    basegfx::fround(aTopLeftPixel.getX()), basegfx::fround(aTopLeftPixel.getY()),
                    basegfx::fround(aBottomRightPixel.getX() - aTopLeftPixel.getX()),
                    basegfx::fround(aBottomRightPixel.getY() - aTopLeftPixel.getY()),
                    POSSIZE);
            }

            return aTopLeftPixel;
        }

        //////////////////////////////////////////////////////////////////////////////
        // process support

        void VclProcessor2D::process(const primitive2d::Primitive2DSequence& rSource)
        {
            if(rSource.hasElements())
            {
                const sal_Int32 nCount(rSource.getLength());

                for(sal_Int32 a(0L); a < nCount; a++)
                {
                    // get reference
                    const primitive2d::Primitive2DReference xReference(rSource[a]);

                    if(xReference.is())
                    {
                        // try to cast to BasePrimitive2D implementation
                        const primitive2d::BasePrimitive2D* pBasePrimitive = dynamic_cast< const primitive2d::BasePrimitive2D* >(xReference.get());

                        if(pBasePrimitive)
                        {
                            // it is a BasePrimitive2D implementation, use local processor
                            processBasePrimitive2D(*pBasePrimitive);
                        }
                        else
                        {
                            // unknown implementation, use UNO API call instead and process recursively
                            const uno::Sequence< beans::PropertyValue >& rViewParameters(getViewInformation2D().getViewInformationSequence());
                            process(xReference->getDecomposition(rViewParameters));
                        }
                    }
                }
            }
        }

        VclProcessor2D::VclProcessor2D(
            const geometry::ViewInformation2D& rViewInformation,
            OutputDevice& rOutDev)
        :   BaseProcessor2D(rViewInformation),
            mpOutputDevice(&rOutDev),
            maBColorModifierStack(),
            maCurrentTransformation(),
            maDrawinglayerOpt(),
            mnPolygonStrokePrimitive2D(0)
        {
            // set digit language, derived from SvtCTLOptions to have the correct
            // number display for arabic/hindi numerals
            const SvtCTLOptions aSvtCTLOptions;
            LanguageType eLang(LANGUAGE_SYSTEM);

            if(SvtCTLOptions::NUMERALS_HINDI == aSvtCTLOptions.GetCTLTextNumerals())
            {
                eLang = LANGUAGE_ARABIC;
            }
            else if(SvtCTLOptions::NUMERALS_ARABIC == aSvtCTLOptions.GetCTLTextNumerals())
            {
                eLang = LANGUAGE_ENGLISH;
            }
            else
            {
                eLang = (LanguageType)Application::GetSettings().GetLanguage();
            }

            rOutDev.SetDigitLanguage(eLang);
        }

        VclProcessor2D::~VclProcessor2D()
        {
        }
    } // end of namespace processor2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
