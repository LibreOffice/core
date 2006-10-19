/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: materialattribute3d.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2006-10-19 10:30:35 $
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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_MATERIALATTRIBUTE3D_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_MATERIALATTRIBUTE3D_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace drawinglayer { namespace {
    class ImpMaterialAttribute3D;
}}

namespace basegfx {
    class BColor;
}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class MaterialAttribute3D
        {
        private:
            ImpMaterialAttribute3D*                             mpMaterialAttribute3D;

        public:
            // constructors/destructor
            MaterialAttribute3D(const basegfx::BColor& rColor, const basegfx::BColor& rSpecular, const basegfx::BColor& rEmission, sal_uInt16 nSpecularIntensity);
            MaterialAttribute3D(const basegfx::BColor& rColor);
            MaterialAttribute3D();
            MaterialAttribute3D(const MaterialAttribute3D& rCandidate);
            ~MaterialAttribute3D();

            // assignment operator
            MaterialAttribute3D& operator=(const MaterialAttribute3D& rCandidate);

            // compare operator
            bool operator==(const MaterialAttribute3D& rCandidate) const;
            bool operator!=(const MaterialAttribute3D& rCandidate) const { return !operator==(rCandidate); }

            // data access
            const basegfx::BColor& getColor() const;
            const basegfx::BColor& getSpecular() const;
            const basegfx::BColor& getEmission() const;
            sal_uInt16 getSpecularIntensity() const;
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_MATERIALATTRIBUTE3D_HXX

// eof
