/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

#include <drawinglayer/attribute/materialattribute3d.hxx>
#include <basegfx/color/bcolor.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class ImpMaterialAttribute3D
        {
        public:
            // materialAttribute3D definitions
            basegfx::BColor                         maColor;                // object color
            basegfx::BColor                         maSpecular;             // material specular color
            basegfx::BColor                         maEmission;             // material emissive color
            sal_uInt16                                  mnSpecularIntensity;    // material specular intensity [0..128]

            // refcounter
            sal_uInt32                                  mnRefCount;

            ImpMaterialAttribute3D(const basegfx::BColor& rColor, const basegfx::BColor& rSpecular, const basegfx::BColor& rEmission, sal_uInt16 nSpecularIntensity)
            :   maColor(rColor),
                maSpecular(rSpecular),
                maEmission(rEmission),
                mnSpecularIntensity(nSpecularIntensity),
                mnRefCount(0L)
            {
            }

            ImpMaterialAttribute3D(const basegfx::BColor& rColor)
            :   maColor(rColor),
                maSpecular(1.0, 1.0, 1.0),
                maEmission(),
                mnSpecularIntensity(15),
                mnRefCount(0L)
            {
            }

            ImpMaterialAttribute3D()
            :   mnSpecularIntensity(0),
                mnRefCount(0L)
            {
            }

            bool operator==(const ImpMaterialAttribute3D& rCandidate) const
            {
                return (maColor == rCandidate.maColor
                    && maSpecular == rCandidate.maSpecular
                    && maEmission == rCandidate.maEmission
                    && mnSpecularIntensity == rCandidate.mnSpecularIntensity);
            }

            const basegfx::BColor& getColor() const { return maColor; }
            const basegfx::BColor& getSpecular() const { return maSpecular; }
            const basegfx::BColor& getEmission() const { return maEmission; }
            sal_uInt16 getSpecularIntensity() const { return mnSpecularIntensity; }
        };
    } // end of anonymous namespace
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        MaterialAttribute3D::MaterialAttribute3D(const basegfx::BColor& rColor, const basegfx::BColor& rSpecular, const basegfx::BColor& rEmission, sal_uInt16 nSpecularIntensity)
        :   mpMaterialAttribute3D(new ImpMaterialAttribute3D(rColor, rSpecular, rEmission, nSpecularIntensity))
        {
        }

        MaterialAttribute3D::MaterialAttribute3D(const basegfx::BColor& rColor)
        :   mpMaterialAttribute3D(new ImpMaterialAttribute3D(rColor))
        {
        }

        MaterialAttribute3D::MaterialAttribute3D()
        :   mpMaterialAttribute3D(new ImpMaterialAttribute3D())
        {
        }

        MaterialAttribute3D::MaterialAttribute3D(const MaterialAttribute3D& rCandidate)
        :   mpMaterialAttribute3D(rCandidate.mpMaterialAttribute3D)
        {
            mpMaterialAttribute3D->mnRefCount++;
        }

        MaterialAttribute3D::~MaterialAttribute3D()
        {
            if(mpMaterialAttribute3D->mnRefCount)
            {
                mpMaterialAttribute3D->mnRefCount--;
            }
            else
            {
                delete mpMaterialAttribute3D;
            }
        }

        MaterialAttribute3D& MaterialAttribute3D::operator=(const MaterialAttribute3D& rCandidate)
        {
            if(rCandidate.mpMaterialAttribute3D != mpMaterialAttribute3D)
            {
                if(mpMaterialAttribute3D->mnRefCount)
                {
                    mpMaterialAttribute3D->mnRefCount--;
                }
                else
                {
                    delete mpMaterialAttribute3D;
                }

                mpMaterialAttribute3D = rCandidate.mpMaterialAttribute3D;
                mpMaterialAttribute3D->mnRefCount++;
            }

            return *this;
        }

        bool MaterialAttribute3D::operator==(const MaterialAttribute3D& rCandidate) const
        {
            if(rCandidate.mpMaterialAttribute3D == mpMaterialAttribute3D)
            {
                return true;
            }

            return (*rCandidate.mpMaterialAttribute3D == *mpMaterialAttribute3D);
        }

        const basegfx::BColor& MaterialAttribute3D::getColor() const
        {
            return mpMaterialAttribute3D->getColor();
        }

        const basegfx::BColor& MaterialAttribute3D::getSpecular() const
        {
            return mpMaterialAttribute3D->getSpecular();
        }

        const basegfx::BColor& MaterialAttribute3D::getEmission() const
        {
            return mpMaterialAttribute3D->getEmission();
        }

        sal_uInt16 MaterialAttribute3D::getSpecularIntensity() const
        {
            return mpMaterialAttribute3D->getSpecularIntensity();
        }
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
