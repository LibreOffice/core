/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdrattribute3d.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: aw $ $Date: 2008-03-13 08:22:00 $
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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRATTRIBUTE3D_HXX
#include <drawinglayer/attribute/sdrattribute3d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        Sdr3DObjectAttribute::Sdr3DObjectAttribute(
            ::com::sun::star::drawing::NormalsKind  aNormalsKind,
            ::com::sun::star::drawing::TextureProjectionMode aTextureProjectionX,
            ::com::sun::star::drawing::TextureProjectionMode aTextureProjectionY,
            ::com::sun::star::drawing::TextureKind2 aTextureKind,
            ::com::sun::star::drawing::TextureMode aTextureMode,
            const MaterialAttribute3D& rMaterial,
            bool bNormalsInvert,
            bool bDoubleSided,
            bool bShadow3D,
            bool bTextureFilter,
            bool bReducedLineGeometry)
        :   maNormalsKind(aNormalsKind),
            maTextureProjectionX(aTextureProjectionX),
            maTextureProjectionY(aTextureProjectionY),
            maTextureKind(aTextureKind),
            maTextureMode(aTextureMode),
            maMaterial(rMaterial),
            mbNormalsInvert(bNormalsInvert),
            mbDoubleSided(bDoubleSided),
            mbShadow3D(bShadow3D),
            mbTextureFilter(bTextureFilter),
            mbReducedLineGeometry(bReducedLineGeometry)
        {
        }

        bool Sdr3DObjectAttribute::operator==(const Sdr3DObjectAttribute& rCandidate) const
        {
            return (maNormalsKind == rCandidate.maNormalsKind
                && maTextureProjectionX == rCandidate.maTextureProjectionX
                && maTextureProjectionY == rCandidate.maTextureProjectionY
                && maTextureKind == rCandidate.maTextureKind
                && maTextureMode == rCandidate.maTextureMode
                && maMaterial == rCandidate.maMaterial
                && mbNormalsInvert == rCandidate.mbNormalsInvert
                && mbDoubleSided == rCandidate.mbDoubleSided
                && mbShadow3D == rCandidate.mbShadow3D
                && mbTextureFilter == rCandidate.mbTextureFilter
                && mbReducedLineGeometry == rCandidate.mbReducedLineGeometry);
        }
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        Sdr3DLightAttribute::Sdr3DLightAttribute(const basegfx::BColor& rColor, const basegfx::B3DVector& rDirection, bool bSpecular)
        :   maColor(rColor),
            maDirection(rDirection),
            mbSpecular(bSpecular)
        {
        }

        bool Sdr3DLightAttribute::operator==(const Sdr3DLightAttribute& rCandidate) const
        {
            return (maColor == rCandidate.maColor
                && maDirection == rCandidate.maDirection
                && mbSpecular == rCandidate.mbSpecular);
        }
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        SdrLightingAttribute::SdrLightingAttribute(
            const basegfx::BColor& rAmbientLight,
            const ::std::vector< Sdr3DLightAttribute >& rLightVector)
        :   maAmbientLight(rAmbientLight),
            maLightVector(rLightVector)
        {
        }

        bool SdrLightingAttribute::operator==(const SdrLightingAttribute& rCandidate) const
        {
            return (maAmbientLight == rCandidate.maAmbientLight
                && maLightVector == rCandidate.maLightVector);
        }

        // color model solver
        basegfx::BColor SdrLightingAttribute::solveColorModel(
            const basegfx::B3DVector& rNormalInEyeCoordinates,
            const basegfx::BColor& rColor, const basegfx::BColor& rSpecular,
            const basegfx::BColor& rEmission, sal_uInt16 nSpecularIntensity) const
        {
            // initialize with emissive color
            basegfx::BColor aRetval(rEmission);

            // take care of global ambient light
            aRetval += getAmbientLight() * rColor;

            // prepare light access. Is there a light?
            const sal_uInt32 nLightCount(maLightVector.size());

            if(nLightCount && !rNormalInEyeCoordinates.equalZero())
            {
                // prepare normal
                basegfx::B3DVector aEyeNormal(rNormalInEyeCoordinates);
                aEyeNormal.normalize();

                for(sal_uInt32 a(0L); a < nLightCount; a++)
                {
                    const Sdr3DLightAttribute& rLight(maLightVector[a]);
                    const double fCosFac(rLight.getDirection().scalar(aEyeNormal));

                    if(basegfx::fTools::more(fCosFac, 0.0))
                    {
                        aRetval += ((rLight.getColor() * rColor) * fCosFac);

                        if(rLight.getSpecular())
                        {
                            // expand by (0.0, 0.0, 1.0) in Z
                            basegfx::B3DVector aSpecularNormal(rLight.getDirection().getX(), rLight.getDirection().getY(), rLight.getDirection().getZ() + 1.0);
                            aSpecularNormal.normalize();
                            double fCosFac2(aSpecularNormal.scalar(aEyeNormal));

                            if(basegfx::fTools::more(fCosFac2, 0.0))
                            {
                                fCosFac2 = pow(fCosFac2, (double)nSpecularIntensity);
                                aRetval += (rSpecular * fCosFac2);
                            }
                        }
                    }
                }
            }

            // clamp to color space before usage
            aRetval.clamp();

            return aRetval;
        }
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        SdrSceneAttribute::SdrSceneAttribute(
            double fDistance,
            double fShadowSlant,
            ::com::sun::star::drawing::ProjectionMode aProjectionMode,
            ::com::sun::star::drawing::ShadeMode aShadeMode,
            bool bTwoSidedLighting)
        :   mfDistance(fDistance),
            mfShadowSlant(fShadowSlant),
            maProjectionMode(aProjectionMode),
            maShadeMode(aShadeMode),
            mbTwoSidedLighting(bTwoSidedLighting)
        {
        }

        bool SdrSceneAttribute::operator==(const SdrSceneAttribute& rCandidate) const
        {
            return (mfDistance == rCandidate.mfDistance
                && mfShadowSlant == rCandidate.mfShadowSlant
                && maProjectionMode == rCandidate.maProjectionMode
                && maShadeMode == rCandidate.maShadeMode
                && mbTwoSidedLighting == rCandidate.mbTwoSidedLighting);
        }
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
