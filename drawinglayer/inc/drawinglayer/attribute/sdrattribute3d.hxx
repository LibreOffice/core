/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdrattribute3d.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: aw $ $Date: 2008-03-13 08:21:36 $
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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRATTRIBUTE3D_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRATTRIBUTE3D_HXX

#ifndef _BGFX_VECTOR_B2DVECTOR_HXX
#include <basegfx/vector/b2dvector.hxx>
#endif

#ifndef _COM_SUN_STAR_DRAWING_NORMALSKIND_HPP_
#include <com/sun/star/drawing/NormalsKind.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_TEXTUREPROJECTIONMODE_HPP_
#include <com/sun/star/drawing/TextureProjectionMode.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_PROJECTIONMODE_HPP_
#include <com/sun/star/drawing/ProjectionMode.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_TEXTUREKIND2_HPP_
#include <com/sun/star/drawing/TextureKind2.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_TEXTUREMODE_HPP_
#include <com/sun/star/drawing/TextureMode.hpp>
#endif

#ifndef _BGFX_VECTOR_B3DVECTOR_HXX
#include <basegfx/vector/b3dvector.hxx>
#endif

#ifndef _COM_SUN_STAR_DRAWING_SHADEMODE_HPP_
#include <com/sun/star/drawing/ShadeMode.hpp>
#endif

#ifndef _BGFX_COLOR_BCOLOR_HXX
#include <basegfx/color/bcolor.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_MATERIALATTRIBUTE3D_HXX
#include <drawinglayer/attribute/materialattribute3d.hxx>
#endif

#include <vector>

//////////////////////////////////////////////////////////////////////////////
// predefines

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class Sdr3DObjectAttribute
        {
            // 3D object attribute definitions
            ::com::sun::star::drawing::NormalsKind              maNormalsKind;              // normals type (0..2)
            ::com::sun::star::drawing::TextureProjectionMode    maTextureProjectionX;       // texture projection type X (0..2)
            ::com::sun::star::drawing::TextureProjectionMode    maTextureProjectionY;       // texture projection type Y (0..2)
            ::com::sun::star::drawing::TextureKind2             maTextureKind;              // texture kind (see uno API)
            ::com::sun::star::drawing::TextureMode              maTextureMode;              // texture kind (see uno API)
            MaterialAttribute3D                                 maMaterial;                 // object, specular and emissive colors, SpecularIntensity

            // bitfield
            unsigned                                            mbNormalsInvert : 1;        // invert normals
            unsigned                                            mbDoubleSided : 1;          // surfaces are double sided
            unsigned                                            mbShadow3D : 1;             // display shadow in 3D (if on), params for that are at scene
            unsigned                                            mbTextureFilter : 1;        // filter texture to make more smooth
            unsigned                                            mbReducedLineGeometry : 1;  // use reduced line geometry (object specific)

        public:
            Sdr3DObjectAttribute(
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
                bool bReducedLineGeometry);

            // compare operator
            bool operator==(const Sdr3DObjectAttribute& rCandidate) const;

            // data access
            ::com::sun::star::drawing::NormalsKind getNormalsKind() const { return maNormalsKind; }
            ::com::sun::star::drawing::TextureProjectionMode getTextureProjectionX() const { return maTextureProjectionX; }
            ::com::sun::star::drawing::TextureProjectionMode getTextureProjectionY() const { return maTextureProjectionY; }
            ::com::sun::star::drawing::TextureKind2 getTextureKind() const { return maTextureKind; }
            ::com::sun::star::drawing::TextureMode getTextureMode() const { return maTextureMode; }
            const MaterialAttribute3D& getMaterial() const { return maMaterial; }
            bool getNormalsInvert() const { return mbNormalsInvert; }
            bool getDoubleSided() const { return mbDoubleSided; }
            bool getShadow3D() const { return mbShadow3D; }
            bool getTextureFilter() const { return mbTextureFilter; }
            bool getReducedLineGeometry() const { return mbReducedLineGeometry; }
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class Sdr3DLightAttribute
        {
            // 3D light attribute definitions
            basegfx::BColor                             maColor;
            basegfx::B3DVector                          maDirection;

            // bitfield
            unsigned                                    mbSpecular : 1;

        public:
            Sdr3DLightAttribute(
                const basegfx::BColor& rColor,
                const basegfx::B3DVector& rDirection,
                bool bSpecular);

            // compare operator
            bool operator==(const Sdr3DLightAttribute& rCandidate) const;

            // data access
            const basegfx::BColor& getColor() const { return maColor; }
            const basegfx::B3DVector& getDirection() const { return maDirection; }
            bool getSpecular() const { return mbSpecular; }
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class SdrLightingAttribute
        {
            // 3D light attribute definitions
            basegfx::BColor                             maAmbientLight;
            ::std::vector< Sdr3DLightAttribute >        maLightVector;

        public:
            SdrLightingAttribute(
                const basegfx::BColor& rAmbientLight,
                const ::std::vector< Sdr3DLightAttribute >& rLightVector);

            // compare operator
            bool operator==(const SdrLightingAttribute& rCandidate) const;

            // data access
            const basegfx::BColor& getAmbientLight() const { return maAmbientLight; }
            const ::std::vector< Sdr3DLightAttribute >& getLightVector() const { return maLightVector; }

            // color model solver
            basegfx::BColor solveColorModel(
                const basegfx::B3DVector& rNormalInEyeCoordinates,
                const basegfx::BColor& rColor, const basegfx::BColor& rSpecular,
                const basegfx::BColor& rEmission, sal_uInt16 nSpecularIntensity) const;
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class SdrSceneAttribute
        {
            // 3D scene attribute definitions
            double                                      mfDistance;
            double                                      mfShadowSlant;
            ::com::sun::star::drawing::ProjectionMode   maProjectionMode;
            ::com::sun::star::drawing::ShadeMode        maShadeMode;

            // bitfield
            unsigned                                    mbTwoSidedLighting : 1;

        public:
            SdrSceneAttribute(
                double fDistance,
                double fShadowSlant,
                ::com::sun::star::drawing::ProjectionMode aProjectionMode,
                ::com::sun::star::drawing::ShadeMode aShadeMode,
                bool bTwoSidedLighting);

            // compare operator
            bool operator==(const SdrSceneAttribute& rCandidate) const;

            // data access
            double getDistance() const { return mfDistance; }
            double getShadowSlant() const { return mfShadowSlant; }
            ::com::sun::star::drawing::ProjectionMode getProjectionMode() const { return maProjectionMode; }
            ::com::sun::star::drawing::ShadeMode getShadeMode() const { return maShadeMode; }
            bool getTwoSidedLighting() const { return mbTwoSidedLighting; }
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRATTRIBUTE3D_HXX

// eof
