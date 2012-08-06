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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRFILLATTRIBUTE_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRFILLATTRIBUTE_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <sal/types.h>

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace basegfx {
    class BColor;
}

namespace drawinglayer { namespace attribute {
    class ImpSdrFillAttribute;
    class FillGradientAttribute;
    class FillHatchAttribute;
    class SdrFillBitmapAttribute;
}}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class DRAWINGLAYER_DLLPUBLIC SdrFillAttribute
        {
        private:
            ImpSdrFillAttribute*            mpSdrFillAttribute;

        public:
            /// constructors/assignmentoperator/destructor
            SdrFillAttribute(
                double fTransparence,
                const basegfx::BColor& rColor,
                const FillGradientAttribute& rGradient,
                const FillHatchAttribute& rHatch,
                const SdrFillBitmapAttribute& rBitmap);
            SdrFillAttribute();
            SdrFillAttribute(const SdrFillAttribute& rCandidate);
            SdrFillAttribute& operator=(const SdrFillAttribute& rCandidate);
            ~SdrFillAttribute();

            // checks if the incarnation is default constructed
            bool isDefault() const;

            // compare operator
            bool operator==(const SdrFillAttribute& rCandidate) const;

            // data read access
            double getTransparence() const;
            const basegfx::BColor& getColor() const;
            const FillGradientAttribute& getGradient() const;
            const FillHatchAttribute& getHatch() const;
            const SdrFillBitmapAttribute& getBitmap() const;
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRFILLATTRIBUTE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
