/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: texture.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: aw $ $Date: 2008-03-05 09:15:46 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

#ifndef INCLUDED_DRAWINGLAYER_TEXTURE_TEXTURE_HXX
#include <drawinglayer/texture/texture.hxx>
#endif

#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace texture
    {
        GeoTexSvx::GeoTexSvx()
        {
        }

        GeoTexSvx::~GeoTexSvx()
        {
        }

        bool GeoTexSvx::operator==(const GeoTexSvx& /*rGeoTexSvx*/) const
        {
            // default implementation says yes (no data -> no difference)
            return true;
        }

        void GeoTexSvx::appendTransformations(::std::vector< basegfx::B2DHomMatrix >& /*rMatrices*/)
        {
            // default implementation does nothing
        }

        void GeoTexSvx::modifyBColor(const basegfx::B2DPoint& /*rUV*/, basegfx::BColor& rBColor, double& /*rfOpacity*/) const
        {
            // base implementation creates random color (for testing only, may also be pure virtual)
            rBColor.setRed((rand() & 0x7fff) / 32767.0);
            rBColor.setGreen((rand() & 0x7fff) / 32767.0);
            rBColor.setBlue((rand() & 0x7fff) / 32767.0);
        }

        void GeoTexSvx::modifyOpacity(const basegfx::B2DPoint& rUV, double& rfOpacity) const
        {
            // base implementation uses inverse of luminance of solved color
            basegfx::BColor aBaseColor;
            modifyBColor(rUV, aBaseColor, rfOpacity);
            rfOpacity = 1.0 - aBaseColor.luminance();
        }
    } // end of namespace texture
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace texture
    {
        void GeoTexSvxGradient::impAppendMatrix(::std::vector< basegfx::B2DHomMatrix >& rMatrices, const basegfx::B2DRange& rRange)
        {
            basegfx::B2DHomMatrix aNew;
            aNew.set(0, 0, rRange.getWidth());
            aNew.set(1, 1, rRange.getHeight());
            aNew.set(0, 2, rRange.getMinX());
            aNew.set(1, 2, rRange.getMinY());
            rMatrices.push_back(maTextureTransform * aNew);
        }

        void GeoTexSvxGradient::impAppendColorsRadial(::std::vector< basegfx::BColor >& rColors)
        {
            if(mnSteps)
            {
                rColors.push_back(maStart);

                for(sal_uInt32 a(1L); a < mnSteps - 1L; a++)
                {
                    rColors.push_back(interpolate(maStart, maEnd, (double)a / (double)mnSteps));
                }

                rColors.push_back(maEnd);
            }
        }

        GeoTexSvxGradient::GeoTexSvxGradient(const basegfx::B2DRange& rTargetRange, const basegfx::BColor& rStart, const basegfx::BColor& rEnd, sal_uInt32 nSteps, double fBorder)
        :   maTargetRange(rTargetRange),
            maStart(rStart),
            maEnd(rEnd),
            mnSteps(nSteps),
            mfAspect(1.0),
            mfBorder(fBorder)
        {
        }

        GeoTexSvxGradient::~GeoTexSvxGradient()
        {
        }

        bool GeoTexSvxGradient::operator==(const GeoTexSvx& rGeoTexSvx) const
        {
            const GeoTexSvxGradient* pCompare = dynamic_cast< const GeoTexSvxGradient* >(&rGeoTexSvx);
            return (pCompare
                && maTextureTransform == pCompare->maTextureTransform
                && maTargetRange == pCompare->maTargetRange
                && mnSteps == pCompare->mnSteps
                && mfAspect == pCompare->mfAspect
                && mfBorder == pCompare->mfBorder);
        }
    } // end of namespace texture
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace texture
    {
        GeoTexSvxGradientLinear::GeoTexSvxGradientLinear(const basegfx::B2DRange& rTargetRange, const basegfx::BColor& rStart, const basegfx::BColor& rEnd, sal_uInt32 nSteps, double fBorder, double fAngle)
        :   GeoTexSvxGradient(rTargetRange, rStart, rEnd, nSteps, fBorder)
        {
            double fTargetSizeX(maTargetRange.getWidth());
            double fTargetSizeY(maTargetRange.getHeight());
            double fTargetOffsetX(maTargetRange.getMinX());
            double fTargetOffsetY(maTargetRange.getMinY());

            // add object expansion
            if(0.0 != fAngle)
            {
                const double fAbsCos(fabs(cos(fAngle)));
                const double fAbsSin(fabs(sin(fAngle)));
                const double fNewX(fTargetSizeX * fAbsCos + fTargetSizeY * fAbsSin);
                const double fNewY(fTargetSizeY * fAbsCos + fTargetSizeX * fAbsSin);
                fTargetOffsetX -= (fNewX - fTargetSizeX) / 2.0;
                fTargetOffsetY -= (fNewY - fTargetSizeY) / 2.0;
                fTargetSizeX = fNewX;
                fTargetSizeY = fNewY;
            }

            // add object scale before rotate
            maTextureTransform.scale(fTargetSizeX, fTargetSizeY);

            // add texture rotate after scale to keep perpendicular angles
            if(0.0 != fAngle)
            {
                basegfx::B2DPoint aCenter(0.5, 0.5);
                aCenter *= maTextureTransform;

                maTextureTransform.translate(-aCenter.getX(), -aCenter.getY());
                maTextureTransform.rotate(fAngle);
                maTextureTransform.translate(aCenter.getX(), aCenter.getY());
            }

            // add object translate
            maTextureTransform.translate(fTargetOffsetX, fTargetOffsetY);

            // prepare aspect for texture
            mfAspect = (0.0 != fTargetSizeY) ?  fTargetSizeX / fTargetSizeY : 1.0;

            // build transform from u,v to [0.0 .. 1.0]. As base, use inverse texture transform
            maBackTextureTransform = maTextureTransform;
            maBackTextureTransform.invert();
            maBackTextureTransform.translate(0.0, -mfBorder);
            const double fSizeWithoutBorder(1.0 - mfBorder);

            if(!basegfx::fTools::equal(fSizeWithoutBorder, 0.0))
            {
                maBackTextureTransform.scale(1.0, 1.0 / fSizeWithoutBorder);
            }
        }

        GeoTexSvxGradientLinear::~GeoTexSvxGradientLinear()
        {
        }

        void GeoTexSvxGradientLinear::appendTransformations(::std::vector< basegfx::B2DHomMatrix >& rMatrices)
        {
            if(mnSteps)
            {
                const double fTop(mfBorder);
                const double fStripeWidth((1.0 - fTop) / mnSteps);

                for(sal_uInt32 a(1L); a < mnSteps; a++)
                {
                    const double fOffsetUpper(fStripeWidth * (double)a);

                    // create matrix
                    const basegfx::B2DRange aRect(0.0, fTop + fOffsetUpper, 1.0, 1.0);
                    impAppendMatrix(rMatrices, aRect);
                }
            }
        }

        void GeoTexSvxGradientLinear::appendColors(::std::vector< basegfx::BColor >& rColors)
        {
            if(mnSteps)
            {
                rColors.push_back(maStart);

                for(sal_uInt32 a(1L); a < mnSteps; a++)
                {
                    rColors.push_back(interpolate(maStart, maEnd, (double)a / (double)(mnSteps + 1L)));
                }
            }
        }

        void GeoTexSvxGradientLinear::modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& /*rfOpacity*/) const
        {
            const basegfx::B2DPoint aCoor(maBackTextureTransform * rUV);

            if(basegfx::fTools::lessOrEqual(aCoor.getY(), 0.0))
            {
                rBColor = maStart;
                return;
            }

            if(basegfx::fTools::moreOrEqual(aCoor.getY(), 1.0))
            {
                rBColor = maEnd;
                return;
            }

            double fScaler(aCoor.getY());

            if(mnSteps > 2L && mnSteps < 128L)
            {
                fScaler = floor(fScaler * (double)mnSteps) / (double)(mnSteps + 1L);
            }

            rBColor = (maStart * (1.0 - fScaler)) + (maEnd * fScaler);
        }
    } // end of namespace texture
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace texture
    {
        GeoTexSvxGradientAxial::GeoTexSvxGradientAxial(const basegfx::B2DRange& rTargetRange, const basegfx::BColor& rStart, const basegfx::BColor& rEnd, sal_uInt32 nSteps, double fBorder, double fAngle)
        :   GeoTexSvxGradient(rTargetRange, rStart, rEnd, nSteps, fBorder)
        {
            double fTargetSizeX(maTargetRange.getWidth());
            double fTargetSizeY(maTargetRange.getHeight());
            double fTargetOffsetX(maTargetRange.getMinX());
            double fTargetOffsetY(maTargetRange.getMinY());

            // add object expansion
            if(0.0 != fAngle)
            {
                const double fAbsCos(fabs(cos(fAngle)));
                const double fAbsSin(fabs(sin(fAngle)));
                const double fNewX(fTargetSizeX * fAbsCos + fTargetSizeY * fAbsSin);
                const double fNewY(fTargetSizeY * fAbsCos + fTargetSizeX * fAbsSin);
                fTargetOffsetX -= (fNewX - fTargetSizeX) / 2.0;
                fTargetOffsetY -= (fNewY - fTargetSizeY) / 2.0;
                fTargetSizeX = fNewX;
                fTargetSizeY = fNewY;
            }

            // add object scale before rotate
            maTextureTransform.scale(fTargetSizeX, fTargetSizeY);

            // add texture rotate after scale to keep perpendicular angles
            if(0.0 != fAngle)
            {
                basegfx::B2DPoint aCenter(0.5, 0.5);
                aCenter *= maTextureTransform;

                maTextureTransform.translate(-aCenter.getX(), -aCenter.getY());
                maTextureTransform.rotate(fAngle);
                maTextureTransform.translate(aCenter.getX(), aCenter.getY());
            }

            // add object translate
            maTextureTransform.translate(fTargetOffsetX, fTargetOffsetY);

            // prepare aspect for texture
            mfAspect = (0.0 != fTargetSizeY) ?  fTargetSizeX / fTargetSizeY : 1.0;

            // build transform from u,v to [0.0 .. 1.0]. As base, use inverse texture transform
            maBackTextureTransform = maTextureTransform;
            maBackTextureTransform.invert();
            maBackTextureTransform.translate(0.0, -0.5);
            const double fSizeWithoutBorder((1.0 - mfBorder) * 0.5);

            if(!basegfx::fTools::equal(fSizeWithoutBorder, 0.0))
            {
                maBackTextureTransform.scale(1.0, 1.0 / fSizeWithoutBorder);
            }

            // fill internal steps for getBColor implementation
            mfInternalSteps = (double)((mnSteps * 2L) - 1L);
        }

        GeoTexSvxGradientAxial::~GeoTexSvxGradientAxial()
        {
        }

        void GeoTexSvxGradientAxial::appendTransformations(::std::vector< basegfx::B2DHomMatrix >& rMatrices)
        {
            if(mnSteps)
            {
                const double fHalfBorder(mfBorder * 0.5);
                double fTop(fHalfBorder);
                double fBottom(1.0 - fHalfBorder);
                const double fStripeWidth((fBottom - fTop) / ((mnSteps * 2L) - 1L));

                for(sal_uInt32 a(1L); a < mnSteps; a++)
                {
                    const double fOffset(fStripeWidth * (double)a);

                    // create matrix
                    const basegfx::B2DRange aRect(0.0, fTop + fOffset, 1.0, fBottom - fOffset);
                    impAppendMatrix(rMatrices, aRect);
                }
            }
        }

        void GeoTexSvxGradientAxial::appendColors(::std::vector< basegfx::BColor >& rColors)
        {
            if(mnSteps)
            {
                rColors.push_back(maEnd);

                for(sal_uInt32 a(1L); a < mnSteps; a++)
                {
                    rColors.push_back(interpolate(maEnd, maStart, (double)a / (double)mnSteps));
                }
            }
        }

        void GeoTexSvxGradientAxial::modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& /*rfOpacity*/) const
        {
            const basegfx::B2DPoint aCoor(maBackTextureTransform * rUV);
            const double fAbsY(fabs(aCoor.getY()));

            if(basegfx::fTools::moreOrEqual(fAbsY, 1.0))
            {
                rBColor = maEnd;
                return;
            }

            double fScaler(fAbsY);

            if(mnSteps > 2L && mnSteps < 128L)
            {
                fScaler = floor(((fScaler * mfInternalSteps) + 1.0) / 2.0) / (double)(mnSteps - 1L);
            }

            rBColor = (maStart * (1.0 - fScaler)) + (maEnd * fScaler);
        }
    } // end of namespace texture
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace texture
    {
        GeoTexSvxGradientRadial::GeoTexSvxGradientRadial(const basegfx::B2DRange& rTargetRange, const basegfx::BColor& rStart, const basegfx::BColor& rEnd, sal_uInt32 nSteps, double fBorder, double fOffsetX, double fOffsetY)
        :   GeoTexSvxGradient(rTargetRange, rStart, rEnd, nSteps, fBorder)
        {
            double fTargetSizeX(maTargetRange.getWidth());
            double fTargetSizeY(maTargetRange.getHeight());
            double fTargetOffsetX(maTargetRange.getMinX());
            double fTargetOffsetY(maTargetRange.getMinY());

            // add object expansion
            const double fOriginalDiag(sqrt((fTargetSizeX * fTargetSizeX) + (fTargetSizeY * fTargetSizeY)));
            fTargetOffsetX -= (fOriginalDiag - fTargetSizeX) / 2.0;
            fTargetOffsetY -= (fOriginalDiag - fTargetSizeY) / 2.0;
            fTargetSizeX = fOriginalDiag;
            fTargetSizeY = fOriginalDiag;

            // add object scale before rotate
            maTextureTransform.scale(fTargetSizeX, fTargetSizeY);

            // add defined offsets after rotation
            if(0.5 != fOffsetX || 0.5 != fOffsetY)
            {
                // use original target size
                fTargetOffsetX += (fOffsetX - 0.5) * maTargetRange.getWidth();
                fTargetOffsetY += (fOffsetY - 0.5) * maTargetRange.getHeight();
            }

            // add object translate
            maTextureTransform.translate(fTargetOffsetX, fTargetOffsetY);

            // prepare aspect for texture
            mfAspect = (0.0 != fTargetSizeY) ?  fTargetSizeX / fTargetSizeY : 1.0;

            // build transform from u,v to [0.0 .. 1.0]. As base, use inverse texture transform
            maBackTextureTransform = maTextureTransform;
            maBackTextureTransform.invert();
            maBackTextureTransform.translate(-0.5, -0.5);
            const double fHalfBorder((1.0 - mfBorder) * 0.5);

            if(!basegfx::fTools::equal(fHalfBorder, 0.0))
            {
                const double fFactor(1.0 / fHalfBorder);
                maBackTextureTransform.scale(fFactor, fFactor);
            }
        }

        GeoTexSvxGradientRadial::~GeoTexSvxGradientRadial()
        {
        }

        void GeoTexSvxGradientRadial::appendTransformations(::std::vector< basegfx::B2DHomMatrix >& rMatrices)
        {
            if(mnSteps)
            {
                const double fHalfBorder((1.0 - mfBorder) * 0.5);
                double fLeft(0.5 - fHalfBorder);
                double fTop(0.5 - fHalfBorder);
                double fRight(0.5 + fHalfBorder);
                double fBottom(0.5 + fHalfBorder);
                double fIncrementX, fIncrementY;

                if(mfAspect > 1.0)
                {
                    fIncrementY = (fBottom - fTop) / (double)(mnSteps * 2L);
                    fIncrementX = fIncrementY / mfAspect;
                }
                else
                {
                    fIncrementX = (fRight - fLeft) / (double)(mnSteps * 2L);
                    fIncrementY = fIncrementX * mfAspect;
                }

                for(sal_uInt32 a(1L); a < mnSteps; a++)
                {
                    // next step
                    fLeft += fIncrementX;
                    fRight -= fIncrementX;
                    fTop += fIncrementY;
                    fBottom -= fIncrementY;

                    // create matrix
                    const basegfx::B2DRange aRect(fLeft, fTop, fRight, fBottom);
                    impAppendMatrix(rMatrices, aRect);
                }
            }
        }

        void GeoTexSvxGradientRadial::appendColors(::std::vector< basegfx::BColor >& rColors)
        {
            impAppendColorsRadial(rColors);
        }

        void GeoTexSvxGradientRadial::modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& /*rfOpacity*/) const
        {
            const basegfx::B2DPoint aCoor(maBackTextureTransform * rUV);
            const double fDist(aCoor.getX() * aCoor.getX() + aCoor.getY() * aCoor.getY());

            if(basegfx::fTools::moreOrEqual(fDist, 1.0))
            {
                rBColor = maStart;
                return;
            }

            double fScaler(1.0 - sqrt(fDist));

            if(mnSteps > 2L && mnSteps < 128L)
            {
                fScaler = floor(fScaler * (double)mnSteps) / (double)(mnSteps - 1L);
            }

            rBColor = (maStart * (1.0 - fScaler)) + (maEnd * fScaler);
        }
    } // end of namespace texture
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace texture
    {
        GeoTexSvxGradientElliptical::GeoTexSvxGradientElliptical(const basegfx::B2DRange& rTargetRange, const basegfx::BColor& rStart, const basegfx::BColor& rEnd, sal_uInt32 nSteps, double fBorder, double fOffsetX, double fOffsetY, double fAngle)
        :   GeoTexSvxGradient(rTargetRange, rStart, rEnd, nSteps, fBorder)
        {
            double fTargetSizeX(maTargetRange.getWidth());
            double fTargetSizeY(maTargetRange.getHeight());
            double fTargetOffsetX(maTargetRange.getMinX());
            double fTargetOffsetY(maTargetRange.getMinY());

            // add object expansion
            fTargetOffsetX -= (0.4142 / 2.0 ) * fTargetSizeX;
            fTargetOffsetY -= (0.4142 / 2.0 ) * fTargetSizeY;
            fTargetSizeX = 1.4142 * fTargetSizeX;
            fTargetSizeY = 1.4142 * fTargetSizeY;

            // add object scale before rotate
            maTextureTransform.scale(fTargetSizeX, fTargetSizeY);

            // add texture rotate after scale to keep perpendicular angles
            if(0.0 != fAngle)
            {
                basegfx::B2DPoint aCenter(0.5, 0.5);
                aCenter *= maTextureTransform;

                maTextureTransform.translate(-aCenter.getX(), -aCenter.getY());
                maTextureTransform.rotate(fAngle);
                maTextureTransform.translate(aCenter.getX(), aCenter.getY());
            }

            // add defined offsets after rotation
            if(0.5 != fOffsetX || 0.5 != fOffsetY)
            {
                // use original target size
                fTargetOffsetX += (fOffsetX - 0.5) * maTargetRange.getWidth();
                fTargetOffsetY += (fOffsetY - 0.5) * maTargetRange.getHeight();
            }

            // add object translate
            maTextureTransform.translate(fTargetOffsetX, fTargetOffsetY);

            // prepare aspect for texture
            mfAspect = (0.0 != fTargetSizeY) ?  fTargetSizeX / fTargetSizeY : 1.0;

            // build transform from u,v to [0.0 .. 1.0]. As base, use inverse texture transform
            maBackTextureTransform = maTextureTransform;
            maBackTextureTransform.invert();
            maBackTextureTransform.translate(-0.5, -0.5);
            const double fHalfBorder((1.0 - mfBorder) * 0.5);

            if(!basegfx::fTools::equal(fHalfBorder, 0.0))
            {
                const double fFactor(1.0 / fHalfBorder);
                maBackTextureTransform.scale(fFactor, fFactor);
            }
        }

        GeoTexSvxGradientElliptical::~GeoTexSvxGradientElliptical()
        {
        }

        void GeoTexSvxGradientElliptical::appendTransformations(::std::vector< basegfx::B2DHomMatrix >& rMatrices)
        {
            if(mnSteps)
            {
                const double fHalfBorder((1.0 - mfBorder) * 0.5);
                double fLeft(0.5 - fHalfBorder);
                double fTop(0.5 - fHalfBorder);
                double fRight(0.5 + fHalfBorder);
                double fBottom(0.5 + fHalfBorder);
                double fIncrementX, fIncrementY;

                if(mfAspect > 1.0)
                {
                    fIncrementY = (fBottom - fTop) / (double)(mnSteps * 2L);
                    fIncrementX = fIncrementY / mfAspect;
                }
                else
                {
                    fIncrementX = (fRight - fLeft) / (double)(mnSteps * 2L);
                    fIncrementY = fIncrementX * mfAspect;
                }

                for(sal_uInt32 a(1L); a < mnSteps; a++)
                {
                    // next step
                    fLeft += fIncrementX;
                    fRight -= fIncrementX;
                    fTop += fIncrementY;
                    fBottom -= fIncrementY;

                    // create matrix
                    const basegfx::B2DRange aRect(fLeft, fTop, fRight, fBottom);
                    impAppendMatrix(rMatrices, aRect);
                }
            }
        }

        void GeoTexSvxGradientElliptical::appendColors(::std::vector< basegfx::BColor >& rColors)
        {
            impAppendColorsRadial(rColors);
        }

        void GeoTexSvxGradientElliptical::modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& /*rfOpacity*/) const
        {
            const basegfx::B2DPoint aCoor(maBackTextureTransform * rUV);
            const double fDist(aCoor.getX() * aCoor.getX() + aCoor.getY() * aCoor.getY());

            if(basegfx::fTools::moreOrEqual(fDist, 1.0))
            {
                rBColor = maStart;
                return;
            }

            double fScaler(1.0 - sqrt(fDist));

            if(mnSteps > 2L && mnSteps < 128L)
            {
                fScaler = floor(fScaler * (double)mnSteps) / (double)(mnSteps - 1L);
            }

            rBColor = (maStart * (1.0 - fScaler)) + (maEnd * fScaler);
        }
    } // end of namespace texture
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace texture
    {
        GeoTexSvxGradientSquare::GeoTexSvxGradientSquare(const basegfx::B2DRange& rTargetRange, const basegfx::BColor& rStart, const basegfx::BColor& rEnd, sal_uInt32 nSteps, double fBorder, double fOffsetX, double fOffsetY, double fAngle)
        :   GeoTexSvxGradient(rTargetRange, rStart, rEnd, nSteps, fBorder)
        {
            double fTargetSizeX(maTargetRange.getWidth());
            double fTargetSizeY(maTargetRange.getHeight());
            double fTargetOffsetX(maTargetRange.getMinX());
            double fTargetOffsetY(maTargetRange.getMinY());

            // add object expansion
            if(0.0 != fAngle)
            {
                const double fAbsCos(fabs(cos(fAngle)));
                const double fAbsSin(fabs(sin(fAngle)));
                const double fNewX(fTargetSizeX * fAbsCos + fTargetSizeY * fAbsSin);
                const double fNewY(fTargetSizeY * fAbsCos + fTargetSizeX * fAbsSin);
                fTargetOffsetX -= (fNewX - fTargetSizeX) / 2.0;
                fTargetOffsetY -= (fNewY - fTargetSizeY) / 2.0;
                fTargetSizeX = fNewX;
                fTargetSizeY = fNewY;
            }

            // add object scale before rotate
            maTextureTransform.scale(fTargetSizeX, fTargetSizeY);

            // add texture rotate after scale to keep perpendicular angles
            if(0.0 != fAngle)
            {
                basegfx::B2DPoint aCenter(0.5, 0.5);
                aCenter *= maTextureTransform;

                maTextureTransform.translate(-aCenter.getX(), -aCenter.getY());
                maTextureTransform.rotate(fAngle);
                maTextureTransform.translate(aCenter.getX(), aCenter.getY());
            }

            // add defined offsets after rotation
            if(0.5 != fOffsetX || 0.5 != fOffsetY)
            {
                // use scaled target size
                fTargetOffsetX += (fOffsetX - 0.5) * fTargetSizeX;
                fTargetOffsetY += (fOffsetY - 0.5) * fTargetSizeY;
            }

            // add object translate
            maTextureTransform.translate(fTargetOffsetX, fTargetOffsetY);

            // prepare aspect for texture
            mfAspect = (0.0 != fTargetSizeY) ?  fTargetSizeX / fTargetSizeY : 1.0;

            // build transform from u,v to [0.0 .. 1.0]. As base, use inverse texture transform
            maBackTextureTransform = maTextureTransform;
            maBackTextureTransform.invert();
            maBackTextureTransform.translate(-0.5, -0.5);
            const double fHalfBorder((1.0 - mfBorder) * 0.5);

            if(!basegfx::fTools::equal(fHalfBorder, 0.0))
            {
                const double fFactor(1.0 / fHalfBorder);
                maBackTextureTransform.scale(fFactor, fFactor);
            }
        }

        GeoTexSvxGradientSquare::~GeoTexSvxGradientSquare()
        {
        }

        void GeoTexSvxGradientSquare::appendTransformations(::std::vector< basegfx::B2DHomMatrix >& rMatrices)
        {
            if(mnSteps)
            {
                const double fHalfBorder((1.0 - mfBorder) * 0.5);
                double fLeft(0.5 - fHalfBorder);
                double fTop(0.5 - fHalfBorder);
                double fRight(0.5 + fHalfBorder);
                double fBottom(0.5 + fHalfBorder);
                double fIncrementX, fIncrementY;

                if(mfAspect > 1.0)
                {
                    const double fWidth(fRight - fLeft);
                    const double fHalfAspectExpansion(((mfAspect - 1.0) * 0.5) * fWidth);
                    fTop -= fHalfAspectExpansion;
                    fBottom += fHalfAspectExpansion;
                    fIncrementX = fWidth / (double)(mnSteps * 2L);
                    fIncrementY = fIncrementX * mfAspect;
                }
                else
                {
                    const double fHeight(fBottom - fTop);
                    const double fHalfAspectExpansion((((1.0 / mfAspect) - 1.0) * 0.5) * fHeight);
                    fLeft -= fHalfAspectExpansion;
                    fRight += fHalfAspectExpansion;
                    fIncrementY = fHeight / (double)(mnSteps * 2L);
                    fIncrementX = fIncrementY / mfAspect;
                }

                for(sal_uInt32 a(1L); a < mnSteps; a++)
                {
                    // next step
                    fLeft += fIncrementX;
                    fRight -= fIncrementX;
                    fTop += fIncrementY;
                    fBottom -= fIncrementY;

                    // create matrix
                    const basegfx::B2DRange aRect(fLeft, fTop, fRight, fBottom);
                    impAppendMatrix(rMatrices, aRect);
                }
            }
        }

        void GeoTexSvxGradientSquare::appendColors(::std::vector< basegfx::BColor >& rColors)
        {
            impAppendColorsRadial(rColors);
        }

        void GeoTexSvxGradientSquare::modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& /*rfOpacity*/) const
        {
            const basegfx::B2DPoint aCoor(maBackTextureTransform * rUV);
            const double fAbsX(fabs(aCoor.getX()));
            const double fAbsY(fabs(aCoor.getY()));

            if(basegfx::fTools::moreOrEqual(fAbsX, 1.0) || basegfx::fTools::moreOrEqual(fAbsY, 1.0))
            {
                rBColor = maStart;
                return;
            }

            double fScaler(1.0 - (fAbsX > fAbsY ? fAbsX : fAbsY));

            if(mnSteps > 2L && mnSteps < 128L)
            {
                fScaler = floor(fScaler * (double)mnSteps) / (double)(mnSteps - 1L);
            }

            rBColor = (maStart * (1.0 - fScaler)) + (maEnd * fScaler);
        }
    } // end of namespace texture
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace texture
    {
        GeoTexSvxGradientRect::GeoTexSvxGradientRect(const basegfx::B2DRange& rTargetRange, const basegfx::BColor& rStart, const basegfx::BColor& rEnd, sal_uInt32 nSteps, double fBorder, double fOffsetX, double fOffsetY, double fAngle)
        :   GeoTexSvxGradient(rTargetRange, rStart, rEnd, nSteps, fBorder)
        {
            double fTargetSizeX(maTargetRange.getWidth());
            double fTargetSizeY(maTargetRange.getHeight());
            double fTargetOffsetX(maTargetRange.getMinX());
            double fTargetOffsetY(maTargetRange.getMinY());

            // add object expansion
            if(0.0 != fAngle)
            {
                const double fAbsCos(fabs(cos(fAngle)));
                const double fAbsSin(fabs(sin(fAngle)));
                const double fNewX(fTargetSizeX * fAbsCos + fTargetSizeY * fAbsSin);
                const double fNewY(fTargetSizeY * fAbsCos + fTargetSizeX * fAbsSin);
                fTargetOffsetX -= (fNewX - fTargetSizeX) / 2.0;
                fTargetOffsetY -= (fNewY - fTargetSizeY) / 2.0;
                fTargetSizeX = fNewX;
                fTargetSizeY = fNewY;
            }

            // add object scale before rotate
            maTextureTransform.scale(fTargetSizeX, fTargetSizeY);

            // add texture rotate after scale to keep perpendicular angles
            if(0.0 != fAngle)
            {
                basegfx::B2DPoint aCenter(0.5, 0.5);
                aCenter *= maTextureTransform;

                maTextureTransform.translate(-aCenter.getX(), -aCenter.getY());
                maTextureTransform.rotate(fAngle);
                maTextureTransform.translate(aCenter.getX(), aCenter.getY());
            }

            // add defined offsets after rotation
            if(0.5 != fOffsetX || 0.5 != fOffsetY)
            {
                // use scaled target size
                fTargetOffsetX += (fOffsetX - 0.5) * fTargetSizeX;
                fTargetOffsetY += (fOffsetY - 0.5) * fTargetSizeY;
            }

            // add object translate
            maTextureTransform.translate(fTargetOffsetX, fTargetOffsetY);

            // prepare aspect for texture
            mfAspect = (0.0 != fTargetSizeY) ?  fTargetSizeX / fTargetSizeY : 1.0;

            // build transform from u,v to [0.0 .. 1.0]. As base, use inverse texture transform
            maBackTextureTransform = maTextureTransform;
            maBackTextureTransform.invert();
            maBackTextureTransform.translate(-0.5, -0.5);
            const double fHalfBorder((1.0 - mfBorder) * 0.5);

            if(!basegfx::fTools::equal(fHalfBorder, 0.0))
            {
                const double fFactor(1.0 / fHalfBorder);
                maBackTextureTransform.scale(fFactor, fFactor);
            }
        }

        GeoTexSvxGradientRect::~GeoTexSvxGradientRect()
        {
        }

        void GeoTexSvxGradientRect::appendTransformations(::std::vector< basegfx::B2DHomMatrix >& rMatrices)
        {
            if(mnSteps)
            {
                const double fHalfBorder((1.0 - mfBorder) * 0.5);
                double fLeft(0.5 - fHalfBorder);
                double fTop(0.5 - fHalfBorder);
                double fRight(0.5 + fHalfBorder);
                double fBottom(0.5 + fHalfBorder);
                double fIncrementX, fIncrementY;

                if(mfAspect > 1.0)
                {
                    fIncrementY = (fBottom - fTop) / (double)(mnSteps * 2L);
                    fIncrementX = fIncrementY / mfAspect;
                }
                else
                {
                    fIncrementX = (fRight - fLeft) / (double)(mnSteps * 2L);
                    fIncrementY = fIncrementX * mfAspect;
                }

                for(sal_uInt32 a(1L); a < mnSteps; a++)
                {
                    // next step
                    fLeft += fIncrementX;
                    fRight -= fIncrementX;
                    fTop += fIncrementY;
                    fBottom -= fIncrementY;

                    // create matrix
                    const basegfx::B2DRange aRect(fLeft, fTop, fRight, fBottom);
                    impAppendMatrix(rMatrices, aRect);
                }
            }
        }

        void GeoTexSvxGradientRect::appendColors(::std::vector< basegfx::BColor >& rColors)
        {
            impAppendColorsRadial(rColors);
        }

        void GeoTexSvxGradientRect::modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& /*rfOpacity*/) const
        {
            const basegfx::B2DPoint aCoor(maBackTextureTransform * rUV);
            const double fAbsX(fabs(aCoor.getX()));
            const double fAbsY(fabs(aCoor.getY()));

            if(basegfx::fTools::moreOrEqual(fAbsX, 1.0) || basegfx::fTools::moreOrEqual(fAbsY, 1.0))
            {
                rBColor = maStart;
                return;
            }

            double fScaler(1.0 - (fAbsX > fAbsY ? fAbsX : fAbsY));

            if(mnSteps > 2L && mnSteps < 128L)
            {
                fScaler = floor(fScaler * (double)mnSteps) / (double)(mnSteps - 1L);
            }

            rBColor = (maStart * (1.0 - fScaler)) + (maEnd * fScaler);
        }
    } // end of namespace texture
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace texture
    {
        GeoTexSvxHatch::GeoTexSvxHatch(const basegfx::B2DRange& rTargetRange, double fDistance, double fAngle)
        :   mfDistance(0.1),
            mfAngle(fAngle),
            mnSteps(10L)
        {
            double fTargetSizeX(rTargetRange.getWidth());
            double fTargetSizeY(rTargetRange.getHeight());
            double fTargetOffsetX(rTargetRange.getMinX());
            double fTargetOffsetY(rTargetRange.getMinY());

            // add object expansion
            if(0.0 != fAngle)
            {
                const double fAbsCos(fabs(cos(fAngle)));
                const double fAbsSin(fabs(sin(fAngle)));
                const double fNewX(fTargetSizeX * fAbsCos + fTargetSizeY * fAbsSin);
                const double fNewY(fTargetSizeY * fAbsCos + fTargetSizeX * fAbsSin);
                fTargetOffsetX -= (fNewX - fTargetSizeX) / 2.0;
                fTargetOffsetY -= (fNewY - fTargetSizeY) / 2.0;
                fTargetSizeX = fNewX;
                fTargetSizeY = fNewY;
            }

            // add object scale before rotate
            maTextureTransform.scale(fTargetSizeX, fTargetSizeY);

            // add texture rotate after scale to keep perpendicular angles
            if(0.0 != fAngle)
            {
                basegfx::B2DPoint aCenter(0.5, 0.5);
                aCenter *= maTextureTransform;

                maTextureTransform.translate(-aCenter.getX(), -aCenter.getY());
                maTextureTransform.rotate(fAngle);
                maTextureTransform.translate(aCenter.getX(), aCenter.getY());
            }

            // add object translate
            maTextureTransform.translate(fTargetOffsetX, fTargetOffsetY);

            // prepare height for texture
            const double fSteps((0.0 != fDistance) ? fTargetSizeY / fDistance : 10.0);
            mnSteps = basegfx::fround(fSteps + 0.5);
            mfDistance = 1.0 / fSteps;

            // build transform from u,v to [0.0 .. 1.0]. As base, use inverse texture transform
            maBackTextureTransform = maTextureTransform;
            maBackTextureTransform.invert();
        }

        GeoTexSvxHatch::~GeoTexSvxHatch()
        {
        }

        bool GeoTexSvxHatch::operator==(const GeoTexSvx& rGeoTexSvx) const
        {
            const GeoTexSvxHatch* pCompare = dynamic_cast< const GeoTexSvxHatch* >(&rGeoTexSvx);
            return (pCompare
                && maTextureTransform == pCompare->maTextureTransform
                && mfDistance == pCompare->mfDistance
                && mfAngle == pCompare->mfAngle
                && mnSteps == pCompare->mnSteps);
        }

        void GeoTexSvxHatch::appendTransformations(::std::vector< basegfx::B2DHomMatrix >& rMatrices)
        {
            for(sal_uInt32 a(1L); a < mnSteps; a++)
            {
                // create matrix
                const double fOffset(mfDistance * (double)a);
                basegfx::B2DHomMatrix aNew;
                aNew.set(1, 2, fOffset);
                rMatrices.push_back(maTextureTransform * aNew);
            }
        }

        double GeoTexSvxHatch::getDistanceToHatch(const basegfx::B2DPoint& rUV) const
        {
            const basegfx::B2DPoint aCoor(maBackTextureTransform * rUV);
            return fmod(aCoor.getY(), mfDistance);
        }
    } // end of namespace texture
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace texture
    {
        GeoTexSvxTiled::GeoTexSvxTiled(const basegfx::B2DPoint& rTopLeft, const basegfx::B2DVector& rSize)
        :   maTopLeft(rTopLeft),
            maSize(rSize)
        {
            if(basegfx::fTools::lessOrEqual(maSize.getX(), 0.0))
            {
                maSize.setX(1.0);
            }

            if(basegfx::fTools::lessOrEqual(maSize.getY(), 0.0))
            {
                maSize.setY(1.0);
            }
        }

        GeoTexSvxTiled::~GeoTexSvxTiled()
        {
        }

        bool GeoTexSvxTiled::operator==(const GeoTexSvx& rGeoTexSvx) const
        {
            const GeoTexSvxTiled* pCompare = dynamic_cast< const GeoTexSvxTiled* >(&rGeoTexSvx);
            return (pCompare
                && maTopLeft == pCompare->maTopLeft
                && maSize == pCompare->maSize);
        }

        void GeoTexSvxTiled::appendTransformations(::std::vector< basegfx::B2DHomMatrix >& rMatrices)
        {
            double fStartX(maTopLeft.getX());
            double fStartY(maTopLeft.getY());

            if(basegfx::fTools::more(fStartX, 0.0))
            {
                fStartX -= (floor(fStartX / maSize.getX()) + 1.0) * maSize.getX();
            }

            if(basegfx::fTools::less(fStartX + maSize.getX(), 0.0))
            {
                fStartX += floor(-fStartX / maSize.getX()) * maSize.getX();
            }

            if(basegfx::fTools::more(fStartY, 0.0))
            {
                fStartY -= (floor(fStartY / maSize.getY()) + 1.0) * maSize.getY();
            }

            if(basegfx::fTools::less(fStartY + maSize.getY(), 0.0))
            {
                fStartY += floor(-fStartY / maSize.getY()) * maSize.getY();
            }

            for(double fPosY(fStartY); basegfx::fTools::less(fPosY, 1.0); fPosY += maSize.getY())
            {
                for(double fPosX(fStartX); basegfx::fTools::less(fPosX, 1.0); fPosX += maSize.getX())
                {
                    basegfx::B2DHomMatrix aNew;

                    aNew.set(0, 0, maSize.getX());
                    aNew.set(1, 1, maSize.getY());
                    aNew.set(0, 2, fPosX);
                    aNew.set(1, 2, fPosY);

                    rMatrices.push_back(aNew);
                }
            }
        }
    } // end of namespace texture
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
