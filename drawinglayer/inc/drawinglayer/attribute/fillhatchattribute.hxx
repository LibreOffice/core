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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_FILLHATCHATTRIBUTE_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_FILLHATCHATTRIBUTE_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <sal/types.h>

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace basegfx {
    class BColor;
}

namespace drawinglayer { namespace attribute {
    class ImpFillHatchAttribute;
}}


namespace drawinglayer
{
    namespace attribute
    {
        enum HatchStyle
        {
            HATCHSTYLE_SINGLE,
            HATCHSTYLE_DOUBLE,
            HATCHSTYLE_TRIPLE
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class DRAWINGLAYER_DLLPUBLIC FillHatchAttribute
        {
        private:
            ImpFillHatchAttribute*              mpFillHatchAttribute;

        public:
            /// constructors/assignmentoperator/destructor
            FillHatchAttribute(
                HatchStyle eStyle,
                double fDistance,
                double fAngle,
                const basegfx::BColor& rColor,
                bool bFillBackground);
            FillHatchAttribute();
            FillHatchAttribute(const FillHatchAttribute& rCandidate);
            FillHatchAttribute& operator=(const FillHatchAttribute& rCandidate);
            ~FillHatchAttribute();

            // checks if the incarnation is default constructed
            bool isDefault() const;

            // compare operator
            bool operator==(const FillHatchAttribute& rCandidate) const;

            // data read access
            HatchStyle getStyle() const;
            double getDistance() const;
            double getAngle() const;
            const basegfx::BColor& getColor() const;
            bool isFillBackground() const;
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_FILLHATCHATTRIBUTE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
