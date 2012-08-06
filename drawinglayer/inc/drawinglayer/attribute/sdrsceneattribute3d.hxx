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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRSCENEATTRIBUTE3D_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRSCENEATTRIBUTE3D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <sal/types.h>
#include <com/sun/star/drawing/ProjectionMode.hpp>
#include <com/sun/star/drawing/ShadeMode.hpp>

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace drawinglayer { namespace attribute {
    class ImpSdrSceneAttribute;
}}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class DRAWINGLAYER_DLLPUBLIC SdrSceneAttribute
        {
        private:
            ImpSdrSceneAttribute*               mpSdrSceneAttribute;

        public:
            /// constructors/assignmentoperator/destructor
            SdrSceneAttribute(
                double fDistance,
                double fShadowSlant,
                ::com::sun::star::drawing::ProjectionMode aProjectionMode,
                ::com::sun::star::drawing::ShadeMode aShadeMode,
                bool bTwoSidedLighting);
            SdrSceneAttribute();
            SdrSceneAttribute(const SdrSceneAttribute& rCandidate);
            SdrSceneAttribute& operator=(const SdrSceneAttribute& rCandidate);
            ~SdrSceneAttribute();

            // checks if the incarnation is default constructed
            bool isDefault() const;

            // compare operator
            bool operator==(const SdrSceneAttribute& rCandidate) const;

            // data read access
            double getShadowSlant() const;
            ::com::sun::star::drawing::ProjectionMode getProjectionMode() const;
            ::com::sun::star::drawing::ShadeMode getShadeMode() const;
            bool getTwoSidedLighting() const;
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRSCENEATTRIBUTE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
