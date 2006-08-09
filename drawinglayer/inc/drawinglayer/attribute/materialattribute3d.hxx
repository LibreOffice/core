/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: materialattribute3d.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: aw $ $Date: 2006-08-09 16:36:38 $
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

#ifndef _DRAWINGLAYER_ATTRIBUTE_MATERIALATTRIBUTE3D_HXX
#define _DRAWINGLAYER_ATTRIBUTE_MATERIALATTRIBUTE3D_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace drawinglayer { namespace {
    class impMaterialAttribute3D;
}}

namespace basegfx {
    class BColor;
}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class materialAttribute3D
        {
        private:
            impMaterialAttribute3D*                             mpMaterialAttribute3D;

        public:
            // constructors/destructor
            materialAttribute3D(const basegfx::BColor& rColor, const basegfx::BColor& rSpecular, const basegfx::BColor& rEmission, sal_uInt16 nSpecularIntensity);
            materialAttribute3D(const basegfx::BColor& rColor);
            materialAttribute3D();
            materialAttribute3D(const materialAttribute3D& rCandidate);
            ~materialAttribute3D();

            // assignment operator
            materialAttribute3D& operator=(const materialAttribute3D& rCandidate);

            // compare operator
            bool operator==(const materialAttribute3D& rCandidate) const;
            bool operator!=(const materialAttribute3D& rCandidate) const { return !operator==(rCandidate); }

            // data access
            const basegfx::BColor& getColor() const;
            const basegfx::BColor& getSpecular() const;
            const basegfx::BColor& getEmission() const;
            sal_uInt16 getSpecularIntensity() const;
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //_DRAWINGLAYER_ATTRIBUTE_MATERIALATTRIBUTE3D_HXX

// eof
