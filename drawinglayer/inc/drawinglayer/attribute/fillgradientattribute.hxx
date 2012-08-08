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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_FILLGRADIENTATTRIBUTE_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_FILLGRADIENTATTRIBUTE_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <sal/types.h>

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace basegfx {
    class BColor;
}

namespace drawinglayer { namespace attribute {
    class ImpFillGradientAttribute;
}}


namespace drawinglayer
{
    namespace attribute
    {
        enum GradientStyle
        {
            GRADIENTSTYLE_LINEAR,
            GRADIENTSTYLE_AXIAL,
            GRADIENTSTYLE_RADIAL,
            GRADIENTSTYLE_ELLIPTICAL,
            GRADIENTSTYLE_SQUARE,
            GRADIENTSTYLE_RECT
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class DRAWINGLAYER_DLLPUBLIC FillGradientAttribute
        {
        private:
            ImpFillGradientAttribute*           mpFillGradientAttribute;

        public:
            /// constructors/assignmentoperator/destructor
            FillGradientAttribute(
                GradientStyle eStyle,
                double fBorder,
                double fOffsetX,
                double fOffsetY,
                double fAngle,
                const basegfx::BColor& rStartColor,
                const basegfx::BColor& rEndColor,
                sal_uInt16 nSteps);
            FillGradientAttribute();
            FillGradientAttribute(const FillGradientAttribute& rCandidate);
            FillGradientAttribute& operator=(const FillGradientAttribute& rCandidate);
            ~FillGradientAttribute();

            // checks if the incarnation is default constructed
            bool isDefault() const;

            // compare operator
            bool operator==(const FillGradientAttribute& rCandidate) const;

            // data read access
            GradientStyle getStyle() const;
            double getBorder() const;
            double getOffsetX() const;
            double getOffsetY() const;
            double getAngle() const;
            const basegfx::BColor& getStartColor() const;
            const basegfx::BColor& getEndColor() const;
            sal_uInt16 getSteps() const;
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_FILLGRADIENTATTRIBUTE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
