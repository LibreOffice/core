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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDROBJECTATTRIBUTE3D_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDROBJECTATTRIBUTE3D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <sal/types.h>
#include <com/sun/star/drawing/NormalsKind.hpp>
#include <com/sun/star/drawing/TextureProjectionMode.hpp>
#include <com/sun/star/drawing/TextureKind2.hpp>
#include <com/sun/star/drawing/TextureMode.hpp>

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace drawinglayer { namespace attribute {
    class ImpSdr3DObjectAttribute;
    class MaterialAttribute3D;
}}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class DRAWINGLAYER_DLLPUBLIC Sdr3DObjectAttribute
        {
        private:
            ImpSdr3DObjectAttribute*            mpSdr3DObjectAttribute;

        public:
            // constructors/destructor
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
            Sdr3DObjectAttribute(const Sdr3DObjectAttribute& rCandidate);
            ~Sdr3DObjectAttribute();

            // assignment operator
            Sdr3DObjectAttribute& operator=(const Sdr3DObjectAttribute& rCandidate);

            // checks if the incarnation is default constructed
            bool isDefault() const;

            // compare operator
            bool operator==(const Sdr3DObjectAttribute& rCandidate) const;

            // data read access
            ::com::sun::star::drawing::NormalsKind getNormalsKind() const;
            ::com::sun::star::drawing::TextureProjectionMode getTextureProjectionX() const;
            ::com::sun::star::drawing::TextureProjectionMode getTextureProjectionY() const;
            ::com::sun::star::drawing::TextureKind2 getTextureKind() const;
            ::com::sun::star::drawing::TextureMode getTextureMode() const;
            const MaterialAttribute3D& getMaterial() const;
            bool getNormalsInvert() const;
            bool getDoubleSided() const;
            bool getShadow3D() const;
            bool getTextureFilter() const;
            bool getReducedLineGeometry() const;
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDROBJECTATTRIBUTE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
