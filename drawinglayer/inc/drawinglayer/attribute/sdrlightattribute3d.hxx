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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRLIGHTATTRIBUTE3D_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRLIGHTATTRIBUTE3D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <sal/types.h>

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace basegfx {
    class BColor;
    class B3DVector;
}

namespace drawinglayer { namespace attribute {
    class ImpSdr3DLightAttribute;
}}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class DRAWINGLAYER_DLLPUBLIC Sdr3DLightAttribute
        {
        private:
            ImpSdr3DLightAttribute*             mpSdr3DLightAttribute;

        public:
            // constructors/destructor
            Sdr3DLightAttribute(
                const basegfx::BColor& rColor,
                const basegfx::B3DVector& rDirection,
                bool bSpecular);
            explicit Sdr3DLightAttribute(const basegfx::BColor& rColor);
            Sdr3DLightAttribute(const Sdr3DLightAttribute& rCandidate);
            ~Sdr3DLightAttribute();

            // assignment operator
            Sdr3DLightAttribute& operator=(const Sdr3DLightAttribute& rCandidate);

            // checks if the incarnation is default constructed
            bool isDefault() const;

            // compare operator
            bool operator==(const Sdr3DLightAttribute& rCandidate) const;

            // data read access
            const basegfx::BColor& getColor() const;
            const basegfx::B3DVector& getDirection() const;
            bool getSpecular() const;
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRLIGHTATTRIBUTE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
