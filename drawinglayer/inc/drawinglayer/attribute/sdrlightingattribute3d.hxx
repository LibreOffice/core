/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRLIGHTINGATTRIBUTE3D_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRLIGHTINGATTRIBUTE3D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <sal/types.h>
#include <vector>

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace basegfx {
    class BColor;
    class B3DVector;
}

namespace drawinglayer { namespace attribute {
    class ImpSdrLightingAttribute;
    class Sdr3DLightAttribute;
}}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class DRAWINGLAYER_DLLPUBLIC SdrLightingAttribute
        {
        private:
            ImpSdrLightingAttribute*            mpSdrLightingAttribute;

        public:
            /// constructors/assignmentoperator/destructor
            SdrLightingAttribute(
                const basegfx::BColor& rAmbientLight,
                const ::std::vector< Sdr3DLightAttribute >& rLightVector);
            SdrLightingAttribute();
            SdrLightingAttribute(const SdrLightingAttribute& rCandidate);
            SdrLightingAttribute& operator=(const SdrLightingAttribute& rCandidate);
            ~SdrLightingAttribute();

            // checks if the incarnation is default constructed
            bool isDefault() const;

            // compare operator
            bool operator==(const SdrLightingAttribute& rCandidate) const;

            // data read access
            const ::std::vector< Sdr3DLightAttribute >& getLightVector() const;

            // color model solver
            basegfx::BColor solveColorModel(
                const basegfx::B3DVector& rNormalInEyeCoordinates,
                const basegfx::BColor& rColor, const basegfx::BColor& rSpecular,
                const basegfx::BColor& rEmission, sal_uInt16 nSpecularIntensity) const;
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRLIGHTINGATTRIBUTE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
