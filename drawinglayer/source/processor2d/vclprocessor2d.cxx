/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vclprocessor2d.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: aw $ $Date: 2006-10-19 10:35:37 $
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

#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_FILLATTRIBUTE_HXX
#include <drawinglayer/attribute/fillattribute.hxx>
#endif

#ifndef _BGFX_POLYPOLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_TEXTURE_TEXTURE_HXX
#include <drawinglayer/texture/texture.hxx>
#endif

#ifndef _SV_BMPACC_HXX
#include <vcl/bmpacc.hxx>
#endif

#ifndef _SV_BITMAPEX_HXX
#include <vcl/bitmapex.hxx>
#endif

#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif

#ifndef _GRFMGR_HXX
#include <goodies/grfmgr.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE_TEXTPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POLYGONPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BITMAPPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRFILLBITMAPATTRIBUTE_HXX
#include <drawinglayer/attribute/sdrfillbitmapattribute.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_FILLBITMAPPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/fillbitmapprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POLYPOLYGONPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_METAFILEPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/metafileprimitive2d.hxx>
#endif

#ifndef _BGFX_TOOLS_CANVASTOOLS_HXX
#include <basegfx/tools/canvastools.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MASKPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_ALPHAPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/alphaprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TRANSFORMPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MARKERPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/markerprimitive2d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////
// support methods for vcl direct gradeint renderering

namespace drawinglayer
{
    namespace
    {
        sal_uInt32 impCalcGradientSteps(OutputDevice& rOutDev, sal_uInt32 nSteps, const basegfx::B2DRange& rRange, sal_uInt32 nMaxDist)
        {
            if(nSteps == 0L)
            {
                const Size aSize(rOutDev.LogicToPixel(Size(basegfx::fround(rRange.getWidth()), basegfx::fround(rRange.getHeight()))));
                nSteps = (aSize.getWidth() + aSize.getHeight()) >> 3L;
            }

            if(nSteps < 2L)
            {
                nSteps = 2L;
            }

            if(nSteps > nMaxDist)
            {
                nSteps = nMaxDist;
            }

            return nSteps;
        }

        void impDrawGradientToOutDevSimple(
            OutputDevice& rOutDev,
            const basegfx::B2DPolyPolygon& rTargetForm,
            const ::std::vector< basegfx::B2DHomMatrix >& rMatrices,
            const ::std::vector< basegfx::BColor >& rColors,
            const basegfx::B2DPolygon& rUnitPolygon)
        {
            rOutDev.SetLineColor();

            for(sal_uInt32 a(0L); a < rColors.size(); a++)
            {
                // set correct color
                const basegfx::BColor aFillColor(rColors[a]);
                rOutDev.SetFillColor(Color(aFillColor));

                if(a)
                {
                    if(a - 1L < rMatrices.size())
                    {
                        basegfx::B2DPolygon aNewPoly(rUnitPolygon);
                        aNewPoly.transform(rMatrices[a - 1L]);
                        rOutDev.DrawPolygon(Polygon(aNewPoly));
                    }
                }
                else
                {
                    rOutDev.DrawPolyPolygon(PolyPolygon(rTargetForm));
                }
            }
        }

        void impDrawGradientToOutDevComplex(
            OutputDevice& rOutDev,
            const basegfx::B2DPolyPolygon& rTargetForm,
            const ::std::vector< basegfx::B2DHomMatrix >& rMatrices,
            const ::std::vector< basegfx::BColor >& rColors,
            const basegfx::B2DPolygon& rUnitPolygon)
        {
            PolyPolygon aVclTargetForm(rTargetForm);
            ::std::vector< Polygon > aVclPolygons;
            sal_uInt32 a;

            // remember and set to XOR
            rOutDev.SetLineColor();
            rOutDev.Push(PUSH_RASTEROP);
            rOutDev.SetRasterOp(ROP_XOR);

            // draw gradient PolyPolygons
            for(a = 0L; a < rMatrices.size(); a++)
            {
                // create polygon and remember
                basegfx::B2DPolygon aNewPoly(rUnitPolygon);
                aNewPoly.transform(rMatrices[a]);
                aVclPolygons.push_back(Polygon(aNewPoly));

                // set correct color
                if(rColors.size() > a)
                {
                    const basegfx::BColor aFillColor(rColors[a]);
                    rOutDev.SetFillColor(Color(aFillColor));
                }

                // create vcl PolyPolygon and draw it
                if(a)
                {
                    PolyPolygon aVclPolyPoly(aVclPolygons[a - 1L]);
                    aVclPolyPoly.Insert(aVclPolygons[a]);
                    rOutDev.DrawPolyPolygon(aVclPolyPoly);
                }
                else
                {
                    PolyPolygon aVclPolyPoly(aVclTargetForm);
                    aVclPolyPoly.Insert(aVclPolygons[0L]);
                    rOutDev.DrawPolyPolygon(aVclPolyPoly);
                }
            }

            // draw last poly in last color
            if(rColors.size())
            {
                const basegfx::BColor aFillColor(rColors[rColors.size() - 1L]);
                rOutDev.SetFillColor(Color(aFillColor));
                rOutDev.DrawPolygon(aVclPolygons[aVclPolygons.size() - 1L]);
            }

            // draw object form in black and go back to XOR
            rOutDev.SetFillColor(COL_BLACK);
            rOutDev.SetRasterOp(ROP_0);
            rOutDev.DrawPolyPolygon(aVclTargetForm);
            rOutDev.SetRasterOp(ROP_XOR);

            // draw gradient PolyPolygons again
            for(a = 0L; a < rMatrices.size(); a++)
            {
                // set correct color
                if(rColors.size() > a)
                {
                    const basegfx::BColor aFillColor(rColors[a]);
                    rOutDev.SetFillColor(Color(aFillColor));
                }

                // create vcl PolyPolygon and draw it
                if(a)
                {
                    PolyPolygon aVclPolyPoly(aVclPolygons[a - 1L]);
                    aVclPolyPoly.Insert(aVclPolygons[a]);
                    rOutDev.DrawPolyPolygon(aVclPolyPoly);
                }
                else
                {
                    PolyPolygon aVclPolyPoly(aVclTargetForm);
                    aVclPolyPoly.Insert(aVclPolygons[0L]);
                    rOutDev.DrawPolyPolygon(aVclPolyPoly);
                }
            }

            // draw last poly in last color
            if(rColors.size())
            {
                const basegfx::BColor aFillColor(rColors[rColors.size() - 1L]);
                rOutDev.SetFillColor(Color(aFillColor));
                rOutDev.DrawPolygon(aVclPolygons[aVclPolygons.size() - 1L]);
            }

            // reset drawmode
            rOutDev.Pop();
        }

        void impDrawGradientToOutDev(
            OutputDevice& rOutDev,
            const basegfx::B2DPolyPolygon& rTargetForm,
            attribute::GradientStyle eGradientStyle,
            sal_uInt32 nSteps,
            const basegfx::BColor& rStart,
            const basegfx::BColor& rEnd,
            double fBorder, double fAngle, double fOffsetX, double fOffsetY, bool bSimple)
        {
            const basegfx::B2DRange aOutlineRange(basegfx::tools::getRange(rTargetForm));
            ::std::vector< basegfx::B2DHomMatrix > aMatrices;
            ::std::vector< basegfx::BColor > aColors;
            basegfx::B2DPolygon aUnitPolygon;

            if(attribute::GRADIENTSTYLE_RADIAL == eGradientStyle || attribute::GRADIENTSTYLE_ELLIPTICAL == eGradientStyle)
            {
                const basegfx::B2DPoint aCircleCenter(0.5, 0.5);
                aUnitPolygon = basegfx::tools::createPolygonFromEllipse(aCircleCenter, 0.5, 0.5);
                // aUnitPolygon = basegfx::tools::adaptiveSubdivideByAngle(aUnitPolygon);
            }
            else
            {
                aUnitPolygon = basegfx::tools::createPolygonFromRect(basegfx::B2DRange(0.0, 0.0, 1.0, 1.0));
            }

            // make sure steps is not too high/low
            nSteps = impCalcGradientSteps(rOutDev, nSteps, aOutlineRange, sal_uInt32((rStart.getMaximumDistance(rEnd) * 127.5) + 0.5));

            // create geometries
            switch(eGradientStyle)
            {
                case attribute::GRADIENTSTYLE_LINEAR:
                {
                    texture::GeoTexSvxGradientLinear aGradient(aOutlineRange, rStart, rEnd, nSteps, fBorder, fAngle);
                    aGradient.appendTransformations(aMatrices);
                    aGradient.appendColors(aColors);
                    break;
                }
                case attribute::GRADIENTSTYLE_AXIAL:
                {
                    texture::GeoTexSvxGradientAxial aGradient(aOutlineRange, rStart, rEnd, nSteps, fBorder, fAngle);
                    aGradient.appendTransformations(aMatrices);
                    aGradient.appendColors(aColors);
                    break;
                }
                case attribute::GRADIENTSTYLE_RADIAL:
                {
                    texture::GeoTexSvxGradientRadial aGradient(aOutlineRange, rStart, rEnd, nSteps, fBorder, fOffsetX, fOffsetY);
                    aGradient.appendTransformations(aMatrices);
                    aGradient.appendColors(aColors);
                    break;
                }
                case attribute::GRADIENTSTYLE_ELLIPTICAL:
                {
                    texture::GeoTexSvxGradientElliptical aGradient(aOutlineRange, rStart, rEnd, nSteps, fBorder, fOffsetX, fOffsetX, fAngle);
                    aGradient.appendTransformations(aMatrices);
                    aGradient.appendColors(aColors);
                    break;
                }
                case attribute::GRADIENTSTYLE_SQUARE:
                {
                    texture::GeoTexSvxGradientSquare aGradient(aOutlineRange, rStart, rEnd, nSteps, fBorder, fOffsetX, fOffsetX, fAngle);
                    aGradient.appendTransformations(aMatrices);
                    aGradient.appendColors(aColors);
                    break;
                }
                case attribute::GRADIENTSTYLE_RECT:
                {
                    texture::GeoTexSvxGradientRect aGradient(aOutlineRange, rStart, rEnd, nSteps, fBorder, fOffsetX, fOffsetX, fAngle);
                    aGradient.appendTransformations(aMatrices);
                    aGradient.appendColors(aColors);
                    break;
                }
            }

            // paint them with mask using the XOR method
            if(aMatrices.size())
            {
                if(bSimple)
                {
                    impDrawGradientToOutDevSimple(rOutDev, rTargetForm, aMatrices, aColors, aUnitPolygon);
                }
                else
                {
                    impDrawGradientToOutDevComplex(rOutDev, rTargetForm, aMatrices, aColors, aUnitPolygon);
                }
            }
        }
    } // end of anonymous namespace
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// support for rendering Bitmap and BitmapEx contents

namespace drawinglayer
{
    namespace
    {
        void impSmoothPoint(BitmapColor& rValue, const basegfx::B2DPoint& rSource, sal_Int32 nIntX, sal_Int32 nIntY, BitmapReadAccess& rRead)
        {
            double fDeltaX(rSource.getX() - nIntX);
            double fDeltaY(rSource.getY() - nIntY);
            sal_Int32 nIndX(0L);
            sal_Int32 nIndY(0L);

            if(fDeltaX > 0.0 && nIntX + 1L < rRead.Width())
            {
                nIndX++;
            }
            else if(fDeltaX < 0.0 && nIntX >= 1L)
            {
                fDeltaX = -fDeltaX;
                nIndX--;
            }

            if(fDeltaY > 0.0 && nIntY + 1L < rRead.Height())
            {
                nIndY++;
            }
            else if(fDeltaY < 0.0 && nIntY >= 1L)
            {
                fDeltaY = -fDeltaY;
                nIndY--;
            }

            if(nIndX || nIndY)
            {
                const double fColorToReal(1.0 / 255.0);
                double fR(rValue.GetRed() * fColorToReal);
                double fG(rValue.GetGreen() * fColorToReal);
                double fB(rValue.GetBlue() * fColorToReal);
                double fRBottom(0.0), fGBottom(0.0), fBBottom(0.0);

                if(nIndX)
                {
                    const double fMulA(fDeltaX * fColorToReal);
                    double fMulB(1.0 - fDeltaX);
                    const BitmapColor aTopPartner(rRead.GetColor(nIntY, nIntX + nIndX));

                    fR = (fR * fMulB) + (aTopPartner.GetRed() * fMulA);
                    fG = (fG * fMulB) + (aTopPartner.GetGreen() * fMulA);
                    fB = (fB * fMulB) + (aTopPartner.GetBlue() * fMulA);

                    if(nIndY)
                    {
                        fMulB *= fColorToReal;
                        const BitmapColor aBottom(rRead.GetColor(nIntY + nIndY, nIntX));
                        const BitmapColor aBottomPartner(rRead.GetColor(nIntY + nIndY, nIntX + nIndX));

                        fRBottom = (aBottom.GetRed() * fMulB) + (aBottomPartner.GetRed() * fMulA);
                        fGBottom = (aBottom.GetGreen() * fMulB) + (aBottomPartner.GetGreen() * fMulA);
                        fBBottom = (aBottom.GetBlue() * fMulB) + (aBottomPartner.GetBlue() * fMulA);
                    }
                }

                if(nIndY)
                {
                    if(!nIndX)
                    {
                        const BitmapColor aBottom(rRead.GetColor(nIntY + nIndY, nIntX));

                        fRBottom = aBottom.GetRed() * fColorToReal;
                        fGBottom = aBottom.GetGreen() * fColorToReal;
                        fBBottom = aBottom.GetBlue() * fColorToReal;
                    }

                    const double fMulB(1.0 - fDeltaY);

                    fR = (fR * fMulB) + (fRBottom * fDeltaY);
                    fG = (fG * fMulB) + (fGBottom * fDeltaY);
                    fB = (fB * fMulB) + (fBBottom * fDeltaY);
                }

                rValue.SetRed((sal_uInt8)(fR * 255.0));
                rValue.SetGreen((sal_uInt8)(fG * 255.0));
                rValue.SetBlue((sal_uInt8)(fB * 255.0));
            }
        }

        void impSmoothIndex(BitmapColor& rValue, const basegfx::B2DPoint& rSource, sal_Int32 nIntX, sal_Int32 nIntY, BitmapReadAccess& rRead)
        {
            double fDeltaX(rSource.getX() - nIntX);
            double fDeltaY(rSource.getY() - nIntY);
            sal_Int32 nIndX(0L);
            sal_Int32 nIndY(0L);

            if(fDeltaX > 0.0 && nIntX + 1L < rRead.Width())
            {
                nIndX++;
            }
            else if(fDeltaX < 0.0 && nIntX >= 1L)
            {
                fDeltaX = -fDeltaX;
                nIndX--;
            }

            if(fDeltaY > 0.0 && nIntY + 1L < rRead.Height())
            {
                nIndY++;
            }
            else if(fDeltaY < 0.0 && nIntY >= 1L)
            {
                fDeltaY = -fDeltaY;
                nIndY--;
            }

            if(nIndX || nIndY)
            {
                const double fColorToReal(1.0 / 255.0);
                double fVal(rValue.GetIndex() * fColorToReal);
                double fValBottom(0.0);

                if(nIndX)
                {
                    const double fMulA(fDeltaX * fColorToReal);
                    double fMulB(1.0 - fDeltaX);
                    const BitmapColor aTopPartner(rRead.GetPixel(nIntY, nIntX + nIndX));

                    fVal = (fVal * fMulB) + (aTopPartner.GetIndex() * fMulA);

                    if(nIndY)
                    {
                        fMulB *= fColorToReal;
                        const BitmapColor aBottom(rRead.GetPixel(nIntY + nIndY, nIntX));
                        const BitmapColor aBottomPartner(rRead.GetPixel(nIntY + nIndY, nIntX + nIndX));

                        fValBottom = (aBottom.GetIndex() * fMulB) + (aBottomPartner.GetIndex() * fMulA);
                    }
                }

                if(nIndY)
                {
                    if(!nIndX)
                    {
                        const BitmapColor aBottom(rRead.GetPixel(nIntY + nIndY, nIntX));

                        fValBottom = aBottom.GetIndex() * fColorToReal;
                    }

                    const double fMulB(1.0 - fDeltaY);

                    fVal = (fVal * fMulB) + (fValBottom * fDeltaY);
                }

                rValue.SetIndex((sal_uInt8)(fVal * 255.0));
            }
        }

        void impTransformBitmap(const Bitmap& rSource, Bitmap& rDestination, const basegfx::B2DHomMatrix& rTransform, bool bSmooth)
        {
            BitmapWriteAccess* pWrite = rDestination.AcquireWriteAccess();

            if(pWrite)
            {
                const Size aContentSizePixel(rSource.GetSizePixel());
                BitmapReadAccess* pRead = (const_cast< Bitmap& >(rSource)).AcquireReadAccess();

                if(pRead)
                {
                    const Size aDestinationSizePixel(rDestination.GetSizePixel());
                    bool bWorkWithIndex(rDestination.GetBitCount() <= 8);
                    BitmapColor aOutside(pRead->GetBestMatchingColor(BitmapColor(0xff, 0xff, 0xff)));

                    for(sal_Int32 y(0L); y < aDestinationSizePixel.getHeight(); y++)
                    {
                        for(sal_Int32 x(0L); x < aDestinationSizePixel.getWidth(); x++)
                        {
                            const basegfx::B2DPoint aSourceCoor(rTransform * basegfx::B2DPoint(x, y));
                            const sal_Int32 nIntX(basegfx::fround(aSourceCoor.getX()));

                            if(nIntX >= 0L && nIntX < aContentSizePixel.getWidth())
                            {
                                const sal_Int32 nIntY(basegfx::fround(aSourceCoor.getY()));

                                if(nIntY >= 0L && nIntY < aContentSizePixel.getHeight())
                                {
                                    if(bWorkWithIndex)
                                    {
                                        BitmapColor aValue(pRead->GetPixel(nIntY, nIntX));

                                        if(bSmooth)
                                        {
                                            impSmoothIndex(aValue, aSourceCoor, nIntX, nIntY, *pRead);
                                        }

                                        pWrite->SetPixel(y, x, aValue);
                                    }
                                    else
                                    {
                                        BitmapColor aValue(pRead->GetColor(nIntY, nIntX));

                                        if(bSmooth)
                                        {
                                            impSmoothPoint(aValue, aSourceCoor, nIntX, nIntY, *pRead);
                                        }

                                        pWrite->SetPixel(y, x, aValue.IsIndex() ? aValue : pWrite->GetBestMatchingColor(aValue));
                                    }

                                    continue;
                                }
                            }

                            // here are outside pixels. Complete mask
                            if(bWorkWithIndex)
                            {
                                pWrite->SetPixel(y, x, aOutside);
                            }
                        }
                    }

                    delete pRead;
                }

                delete pWrite;
            }
        }

        Bitmap impCreateEmptyBitmapWithPattern(const Bitmap& rSource, const Size& aTargetSizePixel)
        {
            Bitmap aRetval;
            BitmapReadAccess* pReadAccess = (const_cast< Bitmap& >(rSource)).AcquireReadAccess();

            if(pReadAccess)
            {
                if(rSource.GetBitCount() <= 8)
                {
                    BitmapPalette aPalette(pReadAccess->GetPalette());
                    aRetval = Bitmap(aTargetSizePixel, rSource.GetBitCount(), &aPalette);
                }
                else
                {
                    aRetval = Bitmap(aTargetSizePixel, rSource.GetBitCount());
                }

                delete pReadAccess;
            }

            return aRetval;
        }

        BitmapEx impTransformBitmapEx(const BitmapEx& rSource, const Rectangle &rCroppedRectPixel, const basegfx::B2DHomMatrix& rTransform)
        {
            // force destination to 24 bit, we want to smooth output
            const Size aDestinationSize(rCroppedRectPixel.GetSize());
            Bitmap aDestination(impCreateEmptyBitmapWithPattern(rSource.GetBitmap(), aDestinationSize));
            static bool bDoSmoothAtAll(true);
            impTransformBitmap(rSource.GetBitmap(), aDestination, rTransform, bDoSmoothAtAll);

            // create mask
            if(rSource.IsTransparent())
            {
                if(rSource.IsAlpha())
                {
                    Bitmap aAlpha(impCreateEmptyBitmapWithPattern(rSource.GetAlpha().GetBitmap(), aDestinationSize));
                    impTransformBitmap(rSource.GetAlpha().GetBitmap(), aAlpha, rTransform, bDoSmoothAtAll);
                    return BitmapEx(aDestination, AlphaMask(aAlpha));
                }
                else
                {
                    Bitmap aMask(impCreateEmptyBitmapWithPattern(rSource.GetMask(), aDestinationSize));
                    impTransformBitmap(rSource.GetMask(), aMask, rTransform, false);
                    return BitmapEx(aDestination, aMask);
                }
            }

            return BitmapEx(aDestination);
        }

    } // end of anonymous namespace
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// support class for OutputDevice output with mask

namespace drawinglayer
{
    namespace
    {
        class impBufferDevice
        {
            OutputDevice&                       mrOutDev;
            VirtualDevice                       maContent;
            VirtualDevice*                      mpMask;
            VirtualDevice*                      mpAlpha;
            Rectangle                           maDestPixel;

        public:
            impBufferDevice(OutputDevice& rOutDev, const basegfx::B2DRange& rRange);
            ~impBufferDevice();

            void paint(double fTrans = 0.0);
            bool isVisible() const { return !maDestPixel.IsEmpty(); }
            VirtualDevice& getContent() { return maContent; }
            VirtualDevice& getMask();
            VirtualDevice& getAlpha();
        };

        impBufferDevice::impBufferDevice(OutputDevice& rOutDev, const basegfx::B2DRange& rRange)
        :   mrOutDev(rOutDev),
            maContent(rOutDev),
            mpMask(0L),
            mpAlpha(0L)
        {
            const Rectangle aRectLogic(
                (sal_Int32)floor(rRange.getMinX()), (sal_Int32)floor(rRange.getMinY()),
                (sal_Int32)floor(rRange.getMaxX()) + 1L, (sal_Int32)floor(rRange.getMaxY()) + 1L);
            const Rectangle aRectPixel(rOutDev.LogicToPixel(aRectLogic));
            const Point aEmptyPoint;
            maDestPixel = Rectangle(aEmptyPoint, rOutDev.GetOutputSizePixel());
            maDestPixel.Intersection(aRectPixel);

            if(isVisible())
            {
                maContent.SetOutputSizePixel(maDestPixel.GetSize(), false);

                const bool bWasEnabledSrc(rOutDev.IsMapModeEnabled());
                rOutDev.EnableMapMode(false);
                maContent.DrawOutDev(aEmptyPoint, maDestPixel.GetSize(), maDestPixel.TopLeft(), maDestPixel.GetSize(), rOutDev);
                rOutDev.EnableMapMode(bWasEnabledSrc);

                MapMode aNewMapMode(rOutDev.GetMapMode());
                const Point aLogicTopLeft(rOutDev.PixelToLogic(maDestPixel.TopLeft()));
                aNewMapMode.SetOrigin(Point(-aLogicTopLeft.X(), -aLogicTopLeft.Y()));

                maContent.SetMapMode(aNewMapMode);
            }
        }

        impBufferDevice::~impBufferDevice()
        {
            delete mpMask;
            delete mpAlpha;
        }

        void impBufferDevice::paint(double fTrans)
        {
            const Point aEmptyPoint;
            const Size aSizePixel(maContent.GetOutputSizePixel());
            const bool bWasEnabledDst(mrOutDev.IsMapModeEnabled());

            mrOutDev.EnableMapMode(false);
            maContent.EnableMapMode(false);
            Bitmap aContent(maContent.GetBitmap(aEmptyPoint, aSizePixel));

            if(mpAlpha)
            {
                mpAlpha->EnableMapMode(false);
                AlphaMask aAlphaMask(mpAlpha->GetBitmap(aEmptyPoint, aSizePixel));
                mrOutDev.DrawBitmapEx(maDestPixel.TopLeft(), BitmapEx(aContent, aAlphaMask));
            }
            else if(mpMask)
            {
                mpMask->EnableMapMode(false);
                Bitmap aMask(mpMask->GetBitmap(aEmptyPoint, aSizePixel));
                mrOutDev.DrawBitmapEx(maDestPixel.TopLeft(), BitmapEx(aContent, aMask));
            }
            else if(0.0 != fTrans)
            {
                sal_uInt8 nMaskValue((sal_uInt8)basegfx::fround(fTrans * 255.0));
                AlphaMask aAlphaMask(aSizePixel, &nMaskValue);
                mrOutDev.DrawBitmapEx(maDestPixel.TopLeft(), BitmapEx(aContent, aAlphaMask));
            }
            else
            {
                mrOutDev.DrawBitmap(maDestPixel.TopLeft(), aContent);
            }

            mrOutDev.EnableMapMode(bWasEnabledDst);
        }

        VirtualDevice& impBufferDevice::getMask()
        {
            if(!mpMask)
            {
                mpMask = new VirtualDevice(mrOutDev, 1);
                mpMask->SetOutputSizePixel(maDestPixel.GetSize(), true);
                mpMask->SetMapMode(maContent.GetMapMode());
            }

            return *mpMask;
        }

        VirtualDevice& impBufferDevice::getAlpha()
        {
            if(!mpAlpha)
            {
                mpAlpha = new VirtualDevice();
                mpAlpha->SetOutputSizePixel(maDestPixel.GetSize(), true);
                mpAlpha->SetMapMode(maContent.GetMapMode());
            }

            return *mpAlpha;
        }
    } // end of anonymous namespace
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// support for different kinds of bitmap rendering using vcl

namespace drawinglayer
{
    namespace
    {
        void RenderBitmapPrimitive2D_GraphicManager(OutputDevice& rOutDev, const BitmapEx& rBitmapEx, const basegfx::B2DHomMatrix& rTransform)
        {
            // prepare attributes
            GraphicAttr aAttributes;

            // decompose matrix to check for shear, rotate and mirroring
            basegfx::B2DVector aScale, aTranslate;
            double fRotate, fShearX;
            rTransform.decompose(aScale, aTranslate, fRotate, fShearX);

            // mirror flags
            aAttributes.SetMirrorFlags(
                (basegfx::fTools::less(aScale.getX(), 0.0) ? BMP_MIRROR_HORZ : 0)|
                (basegfx::fTools::less(aScale.getY(), 0.0) ? BMP_MIRROR_VERT : 0));

            // rotation
            if(!basegfx::fTools::equalZero(fRotate))
            {
                double fRotation(fmod(3600.0 - (fRotate * (10.0 / F_PI180)), 3600.0));
                aAttributes.SetRotation((sal_uInt16)(fRotation));
            }

            // prepare Bitmap
            basegfx::B2DRange aOutlineRange(0.0, 0.0, 1.0, 1.0);

            if(basegfx::fTools::equalZero(fRotate))
            {
                aOutlineRange.transform(rTransform);
            }
            else
            {
                // if rotated, create the unrotated output rectangle for the GraphicManager paint
                basegfx::B2DHomMatrix aSimpleObjectMatrix;

                aSimpleObjectMatrix.scale(fabs(aScale.getX()), fabs(aScale.getY()));
                aSimpleObjectMatrix.translate(aTranslate.getX(), aTranslate.getY());

                aOutlineRange.transform(aSimpleObjectMatrix);
            }

            // prepare dest coor
            const Rectangle aDestRectPixel(
                basegfx::fround(aOutlineRange.getMinX()), basegfx::fround(aOutlineRange.getMinY()),
                basegfx::fround(aOutlineRange.getMaxX()), basegfx::fround(aOutlineRange.getMaxY()));

            // paint it using GraphicManager
            Graphic aGraphic(rBitmapEx);
            GraphicObject aGraphicObject(aGraphic);
            aGraphicObject.Draw(&rOutDev, aDestRectPixel.TopLeft(), aDestRectPixel.GetSize(), &aAttributes);
        }

        void RenderBitmapPrimitive2D_BitmapEx(OutputDevice& rOutDev, const BitmapEx& rBitmapEx, const basegfx::B2DHomMatrix& rTransform)
        {
            // only translate and scale, use vcl's DrawBitmapEx().
            BitmapEx aContent(rBitmapEx);

            // prepare dest coor. Necessary to expand since vcl's DrawBitmapEx draws one pix less
            basegfx::B2DRange aOutlineRange(0.0, 0.0, 1.0, 1.0);
            aOutlineRange.transform(rTransform);
            const Rectangle aDestRectPixel(
                basegfx::fround(aOutlineRange.getMinX()), basegfx::fround(aOutlineRange.getMinY()),
                basegfx::fround(aOutlineRange.getMaxX()), basegfx::fround(aOutlineRange.getMaxY()));

            // decompose matrix to check for shear, rotate and mirroring
            basegfx::B2DVector aScale, aTranslate;
            double fRotate, fShearX;
            rTransform.decompose(aScale, aTranslate, fRotate, fShearX);

            // Check mirroring.
            sal_uInt32 nMirrorFlags(BMP_MIRROR_NONE);

            if(basegfx::fTools::less(aScale.getX(), 0.0))
            {
                nMirrorFlags |= BMP_MIRROR_HORZ;
            }

            if(basegfx::fTools::less(aScale.getY(), 0.0))
            {
                nMirrorFlags |= BMP_MIRROR_VERT;
            }

            if(BMP_MIRROR_NONE != nMirrorFlags)
            {
                aContent.Mirror(nMirrorFlags);
            }

            // draw bitmap
            rOutDev.DrawBitmapEx(aDestRectPixel.TopLeft(), aDestRectPixel.GetSize(), aContent);
        }

        void RenderBitmapPrimitive2D_self(OutputDevice& rOutDev, const BitmapEx& rBitmapEx, const basegfx::B2DHomMatrix& rTransform)
        {
            // process self with free transformation (containing shear and rotate). Get dest rect in pixels.
            basegfx::B2DRange aOutlineRange(0.0, 0.0, 1.0, 1.0);
            aOutlineRange.transform(rTransform);
            const Rectangle aDestRectLogic(
                basegfx::fround(aOutlineRange.getMinX()), basegfx::fround(aOutlineRange.getMinY()),
                basegfx::fround(aOutlineRange.getMaxX()), basegfx::fround(aOutlineRange.getMaxY()));
            const Rectangle aDestRectPixel(rOutDev.LogicToPixel(aDestRectLogic));

            // intersect with output pixel size
            const Rectangle aOutputRectPixel(Point(), rOutDev.GetOutputSizePixel());
            const Rectangle aCroppedRectPixel(aDestRectPixel.GetIntersection(aOutputRectPixel));

            if(!aCroppedRectPixel.IsEmpty())
            {
                // build transform from pixel in aDestination to pixel in rBitmapEx
                basegfx::B2DHomMatrix aTransform;

                // from relative in aCroppedRectPixel to relative in aDestRectPixel
                aTransform.translate(aCroppedRectPixel.Left() - aDestRectPixel.Left(), aCroppedRectPixel.Top() - aDestRectPixel.Top());

                // from relative in aDestRectPixel to absolute Logic
                aTransform.scale((double)aDestRectLogic.getWidth() / (double)aDestRectPixel.getWidth(), (double)aDestRectLogic.getHeight() / (double)aDestRectPixel.getHeight());
                aTransform.translate(aDestRectLogic.Left(), aDestRectLogic.Top());

                // from absolute in Logic to unified object coordinates (0.0 .. 1.0 in x and y)
                basegfx::B2DHomMatrix aInvBitmapTransform(rTransform);
                aInvBitmapTransform.invert();
                aTransform = aInvBitmapTransform * aTransform;

                // from unit object coordinates to rBitmapEx pixel coordintes
                const Size aSourceSizePixel(rBitmapEx.GetSizePixel());
                aTransform.scale(aSourceSizePixel.getWidth() - 1L, aSourceSizePixel.getHeight() - 1L);

                // create bitmap using source, destination and linear back-transformation
                BitmapEx aDestination = impTransformBitmapEx(rBitmapEx, aCroppedRectPixel, aTransform);

                // paint
                const bool bWasEnabled(rOutDev.IsMapModeEnabled());
                rOutDev.EnableMapMode(false);
                rOutDev.DrawBitmapEx(aCroppedRectPixel.TopLeft(), aDestination);
                rOutDev.EnableMapMode(bWasEnabled);
            }
        }

        BitmapEx impModifyBitmapEx(const basegfx::BColorModifierStack& rBColorModifierStack, const BitmapEx& rSource)
        {
            Bitmap aChangedBitmap(rSource.GetBitmap());
            bool bDone(false);

            for(sal_uInt32 a(rBColorModifierStack.count()); a && !bDone; )
            {
                const basegfx::BColorModifier& rModifier = rBColorModifierStack.getBColorModifier(--a);

                switch(rModifier.getMode())
                {
                    case basegfx::BCOLORMODIFYMODE_REPLACE :
                    {
                        // complete replace
                        if(rSource.IsTransparent())
                        {
                            // clear bitmap with dest color
                            aChangedBitmap.Erase(Color(rModifier.getBColor()));
                        }
                        else
                        {
                            // erase bitmap, caller will know to paint direct
                            aChangedBitmap.SetEmpty();
                        }

                        bDone = true;
                        break;
                    }

                    default : // BCOLORMODIFYMODE_INTERPOLATE, BCOLORMODIFYMODE_GRAY, BCOLORMODIFYMODE_BLACKANDWHITE
                    {
                        BitmapWriteAccess* pContent = aChangedBitmap.AcquireWriteAccess();

                        if(pContent)
                        {
                            for(sal_uInt32 y(0L); y < (sal_uInt32)pContent->Height(); y++)
                            {
                                for(sal_uInt32 x(0L); x < (sal_uInt32)pContent->Width(); x++)
                                {
                                    const basegfx::BColor aBColor(rModifier.getModifiedColor(Color(pContent->GetPixel(y, x)).getBColor()));
                                    pContent->SetPixel(y, x, BitmapColor(Color(aBColor)));
                                }
                            }

                            delete pContent;
                        }

                        break;
                    }
                }
            }

            if(aChangedBitmap.IsEmpty())
            {
                return BitmapEx();
            }
            else
            {
                if(rSource.IsTransparent())
                {
                    if(rSource.IsAlpha())
                    {
                        return BitmapEx(aChangedBitmap, rSource.GetAlpha());
                    }
                    else
                    {
                        return BitmapEx(aChangedBitmap, rSource.GetMask());
                    }
                }
                else
                {
                    return BitmapEx(aChangedBitmap);
                }
            }
        }
    } // end of anonymous namespace
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor2d
    {
        //////////////////////////////////////////////////////////////////////////////
        // rendering support

        // directdraw of text simple portion
        void VclProcessor2D::RenderTextSimplePortionPrimitive2D(const primitive2d::TextSimplePortionPrimitive2D& rTextCandidate)
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
                    fRotate += (180.0 * F_PI180);
                }

                if(basegfx::fTools::more(aScale.getX(), 0.0) && basegfx::fTools::more(aScale.getY(), 0.0))
                {
                    // handle, there is no shear and no mirror
                    bPrimitiveAccepted = true;
                    const Font aFont(getVclFontFromFontAttributes(rTextCandidate.getFontAttributes(), aScale, fRotate));
                    const basegfx::B2DPoint aPoint(aLocalTransform * basegfx::B2DPoint(0.0, 0.0));
                    const Point aStartPoint(basegfx::fround(aPoint.getX()), basegfx::fround(aPoint.getY()));

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

                    // set parameters and paint
                    const basegfx::BColor aRGBFontColor(maBColorModifierStack.getModifiedColor(rTextCandidate.getFontColor()));
                    mpOutputDevice->SetFont(aFont);
                    mpOutputDevice->SetTextColor(Color(aRGBFontColor));
                    mpOutputDevice->DrawTextArray(aStartPoint, rTextCandidate.getText(), &(aTransformedDXArray[0]));
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

            mpOutputDevice->DrawPolyLine(Polygon(aLocalPolygon));
        }

        // direct draw of transformed BitmapEx primitive
        void VclProcessor2D::RenderBitmapPrimitive2D(const primitive2d::BitmapPrimitive2D& rBitmapCandidate)
        {
            // decompose matrix to check for shear, rotate and mirroring
            basegfx::B2DHomMatrix aLocalTransform(maCurrentTransformation * rBitmapCandidate.getTransform());
            basegfx::B2DVector aScale, aTranslate;
            double fRotate, fShearX;
            aLocalTransform.decompose(aScale, aTranslate, fRotate, fShearX);
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
                    mpOutputDevice->DrawPolygon(Polygon(aPolygon));

                    bPainted = true;
                }
            }

            if(!bPainted)
            {
                static bool bForceUseOfOwnTransformer(false);
                static bool bUseGraphicManager(true);

                if(!bForceUseOfOwnTransformer && basegfx::fTools::equalZero(fShearX))
                {
                    if(basegfx::fTools::equalZero(fRotate) && !bUseGraphicManager)
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
                            mpOutputDevice->DrawPolygon(Polygon(aPolygon));

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

            mpOutputDevice->DrawPolyPolygon(PolyPolygon(aLocalPolyPolygon));
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
        void VclProcessor2D::RenderMaskPrimitive2D(const primitive2d::MaskPrimitive2D& rMaskCandidate)
        {
            if(rMaskCandidate.getChildren().hasElements())
            {
                basegfx::B2DPolyPolygon aMask(rMaskCandidate.getMask());
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
                    VirtualDevice& rMask = aBufferDevice.getMask();
                    rMask.SetLineColor();
                    rMask.SetFillColor(COL_BLACK);
                    rMask.DrawPolyPolygon(PolyPolygon(aMask));

                    // dump buffer to outdev
                    aBufferDevice.paint();
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

                    // paint mask to it
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
            // remember current transformation
            basegfx::B2DHomMatrix aLastCurrentTransformation(maCurrentTransformation);

            // create new transformations
            maCurrentTransformation = maCurrentTransformation * rTransformCandidate.getTransformation();

            // let break down
            process(rTransformCandidate.getChildren());

            // restore transformations
            maCurrentTransformation = aLastCurrentTransformation;
        }

        // marker
        void VclProcessor2D::RenderMarkerPrimitive2D(const primitive2d::MarkerPrimitive2D& rMarkCandidate)
        {
            const basegfx::BColor aRGBColor(maBColorModifierStack.getModifiedColor(rMarkCandidate.getRGBColor()));
            const Color aColor(aRGBColor);

            // evtl. just test markers
            static bool bForceMarkersToRangeTest(false);

            if(bForceMarkersToRangeTest)
            {
                // ATM just test marker range
                const basegfx::B2DRange aRange(rMarkCandidate.getB2DRange(getViewInformation2D()));
                basegfx::B2DPolygon aPolygon(basegfx::tools::createPolygonFromRect(aRange));
                aPolygon.transform(maCurrentTransformation);

                mpOutputDevice->SetFillColor();
                mpOutputDevice->SetLineColor(aColor);
                mpOutputDevice->DrawPolygon(Polygon(aPolygon));
            }
            else
            {
                const basegfx::B2DPoint aViewPosition(maCurrentTransformation * rMarkCandidate.getPosition());
                Point aPos(basegfx::fround(aViewPosition.getX()), basegfx::fround(aViewPosition.getY()));

                switch(rMarkCandidate.getStyle())
                {
                    default : // MARKERSTYLE2D_POINT
                    {
                        mpOutputDevice->DrawPixel(aPos, aColor);
                        break;
                    }
                    case primitive2d::MARKERSTYLE2D_CROSS :
                    {
                        basegfx::B2DRange aRange;
                        rMarkCandidate.getRealtiveViewRange(aRange);
                        const basegfx::B2DPoint aCenter(aRange.getCenter());

                        aPos = mpOutputDevice->LogicToPixel(aPos);
                        aPos.X() += basegfx::fround(aCenter.getX());
                        aPos.Y() += basegfx::fround(aCenter.getY());

                        const bool bWasEnabled(mpOutputDevice->IsMapModeEnabled());
                        mpOutputDevice->EnableMapMode(false);

                        mpOutputDevice->DrawPixel(aPos, aColor);
                        mpOutputDevice->DrawPixel(Point(aPos.X() - 1L, aPos.Y()), aColor);
                        mpOutputDevice->DrawPixel(Point(aPos.X() + 1L, aPos.Y()), aColor);
                        mpOutputDevice->DrawPixel(Point(aPos.X(), aPos.Y() - 1L), aColor);
                        mpOutputDevice->DrawPixel(Point(aPos.X(), aPos.Y() + 1L), aColor);

                        mpOutputDevice->EnableMapMode(bWasEnabled);
                        break;
                    }
                    case primitive2d::MARKERSTYLE2D_GLUEPOINT :
                    {
                        basegfx::B2DRange aRange;
                        rMarkCandidate.getRealtiveViewRange(aRange);
                        const basegfx::B2DPoint aCenter(aRange.getCenter());

                        aPos = mpOutputDevice->LogicToPixel(aPos);
                        aPos.X() += basegfx::fround(aCenter.getX());
                        aPos.Y() += basegfx::fround(aCenter.getY());

                        const bool bWasEnabled(mpOutputDevice->IsMapModeEnabled());
                        mpOutputDevice->EnableMapMode(false);

                        // backpen
                        mpOutputDevice->SetLineColor(aColor);
                        mpOutputDevice->DrawLine(aPos + Point(-2, -3), aPos + Point(+3, +2));
                        mpOutputDevice->DrawLine(aPos + Point(-3, -2), aPos + Point(+2, +3));
                        mpOutputDevice->DrawLine(aPos + Point(-3, +2), aPos + Point(+2, -3));
                        mpOutputDevice->DrawLine(aPos + Point(-2, +3), aPos + Point(+3, -2));

                        // frontpen (hard coded)
                        mpOutputDevice->SetLineColor(COL_LIGHTBLUE);
                        mpOutputDevice->DrawLine(aPos + Point(-2, -2), aPos + Point(+2, +2));
                        mpOutputDevice->DrawLine(aPos + Point(-2, +2), aPos + Point(+2, -2));

                        mpOutputDevice->EnableMapMode(bWasEnabled);
                        break;
                    }
                }
            }
        }

        //////////////////////////////////////////////////////////////////////////////
        // internal processing support

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
                            // it is a BasePrimitive2D implementation, use getPrimitiveID() call for switch
                            switch(pBasePrimitive->getPrimitiveID())
                            {
                                case Create2DPrimitiveID('2','T','S','i') :
                                {
                                    // directdraw of text simple portion
                                    RenderTextSimplePortionPrimitive2D(static_cast< const primitive2d::TextSimplePortionPrimitive2D& >(*pBasePrimitive));
                                    break;
                                }
                                case Create2DPrimitiveID('2','P','H','a') :
                                {
                                    // direct draw of hairline
                                    RenderPolygonHairlinePrimitive2D(static_cast< const primitive2d::PolygonHairlinePrimitive2D& >(*pBasePrimitive));
                                    break;
                                }
                                case Create2DPrimitiveID('2','B','i','t') :
                                {
                                    // direct draw of transformed BitmapEx primitive
                                    RenderBitmapPrimitive2D(static_cast< const primitive2d::BitmapPrimitive2D& >(*pBasePrimitive));
                                    break;
                                }
                                case Create2DPrimitiveID('2','F','B','i') :
                                {
                                    // direct draw of fillBitmapPrimitive
                                    RenderFillBitmapPrimitive2D(static_cast< const primitive2d::FillBitmapPrimitive2D& >(*pBasePrimitive));
                                    break;
                                }
                                case Create2DPrimitiveID('2','P','P','G') :
                                {
                                    // direct draw of gradient
                                    RenderPolyPolygonGradientPrimitive2D(static_cast< const primitive2d::PolyPolygonGradientPrimitive2D& >(*pBasePrimitive));
                                    break;
                                }
                                case Create2DPrimitiveID('2','P','P','C') :
                                {
                                    // direct draw of PolyPolygon with color
                                    RenderPolyPolygonColorPrimitive2D(static_cast< const primitive2d::PolyPolygonColorPrimitive2D& >(*pBasePrimitive));
                                    break;
                                }
                                case Create2DPrimitiveID('2','M','e','t') :
                                {
                                    // direct draw of MetaFile
                                    RenderMetafilePrimitive2D(static_cast< const primitive2d::MetafilePrimitive2D& >(*pBasePrimitive));
                                    break;
                                }
                                case Create2DPrimitiveID('2','M','a','s') :
                                {
                                    // mask group. Force output to VDev and create mask from given mask
                                    RenderMaskPrimitive2D(static_cast< const primitive2d::MaskPrimitive2D& >(*pBasePrimitive));
                                    break;
                                }
                                case Create2DPrimitiveID('2','M','C','o') :
                                {
                                    // modified color group. Force output to unified color.
                                    RenderModifiedColorPrimitive2D(static_cast< const primitive2d::ModifiedColorPrimitive2D& >(*pBasePrimitive));
                                    break;
                                }
                                case Create2DPrimitiveID('2','A','l','p') :
                                {
                                    // sub-transparence group. Draw to VDev first.
                                    RenderAlphaPrimitive2D(static_cast< const primitive2d::AlphaPrimitive2D& >(*pBasePrimitive));
                                    break;
                                }
                                case Create2DPrimitiveID('2','T','r','a') :
                                {
                                    // transform group.
                                    RenderTransformPrimitive2D(static_cast< const primitive2d::TransformPrimitive2D& >(*pBasePrimitive));
                                    break;
                                }
                                case Create2DPrimitiveID('2','M','a','r') :
                                {
                                    // marker
                                    RenderMarkerPrimitive2D(static_cast< const primitive2d::MarkerPrimitive2D& >(*pBasePrimitive));
                                    break;
                                }
                                default :
                                {
                                    // process recursively
                                    process(pBasePrimitive->get2DDecomposition(getViewInformation2D()));
                                    break;
                                }
                            }
                        }
                        else
                        {
                            // unknown implementation, use UNO API call instead and process recursively
                            com::sun::star::graphic::Primitive2DParameters aPrimitive2DParameters;

                            basegfx::unotools::affineMatrixFromHomMatrix(aPrimitive2DParameters.ViewTransformation, getViewInformation2D().getViewTransformation());
                            aPrimitive2DParameters.Viewport = basegfx::unotools::rectangle2DFromB2DRectangle(getViewInformation2D().getViewport());
                            aPrimitive2DParameters.Time = getViewInformation2D().getViewTime();

                            process(xReference->getDecomposition(aPrimitive2DParameters));
                        }
                    }
                }
            }
        }

        VclProcessor2D::VclProcessor2D(
            const geometry::ViewInformation2D& rViewInformation,
            OutputDevice& rOutDev)
        :   BaseProcessor2D(rViewInformation),
            mbOutputToRecordingMetaFile(false)
        {
            // initialize destination OutDev
            mpOutputDevice = &rOutDev;

            // check if output is recorded to metafile
            const GDIMetaFile* pMetaFile = mpOutputDevice->GetConnectMetaFile();
            mbOutputToRecordingMetaFile = (pMetaFile && pMetaFile->IsRecord() && !pMetaFile->IsPause());

            if(mbOutputToRecordingMetaFile)
            {
                // draw to logic coordinates, do not initialize maCurrentTransformation to viewTransformation,
                // do not change MapMode of destination
            }
            else
            {
                // prepare maCurrentTransformation matrix with viewTransformation to go directly to pixels
                maCurrentTransformation = rViewInformation.getViewTransformation();

                // prepare output to pixels
                mpOutputDevice->Push(PUSH_MAPMODE);
                mpOutputDevice->SetMapMode();
            }
        }

        VclProcessor2D::~VclProcessor2D()
        {
            if(mbOutputToRecordingMetaFile)
            {
            }
            else
            {
                // restore MapMode
                mpOutputDevice->Pop();
            }
        }
    } // end of namespace processor2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
