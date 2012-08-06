/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRFILLBITMAPATTRIBUTE_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRFILLBITMAPATTRIBUTE_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <sal/types.h>

//////////////////////////////////////////////////////////////////////////////
// predefines

class Bitmap;

namespace basegfx {
    class B2DRange;
    class B2DVector;
}

namespace drawinglayer { namespace attribute {
    class FillBitmapAttribute;
    class ImpSdrFillBitmapAttribute;
}}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class DRAWINGLAYER_DLLPUBLIC SdrFillBitmapAttribute
        {
        private:
            ImpSdrFillBitmapAttribute*          mpSdrFillBitmapAttribute;

        public:
            /// constructors/assignmentoperator/destructor
            SdrFillBitmapAttribute(
                const Bitmap& rBitmap,
                const basegfx::B2DVector& rSize,
                const basegfx::B2DVector& rOffset,
                const basegfx::B2DVector& rOffsetPosition,
                const basegfx::B2DVector& rRectPoint,
                bool bTiling,
                bool bStretch,
                bool bLogSize);
            SdrFillBitmapAttribute();
            SdrFillBitmapAttribute(const SdrFillBitmapAttribute& rCandidate);
            SdrFillBitmapAttribute& operator=(const SdrFillBitmapAttribute& rCandidate);
            ~SdrFillBitmapAttribute();

            // checks if the incarnation is default constructed
            bool isDefault() const;

            // compare operator
            bool operator==(const SdrFillBitmapAttribute& rCandidate) const;

            // data read access
            const Bitmap& getBitmap() const;
            const basegfx::B2DVector& getSize() const;
            const basegfx::B2DVector& getOffset() const;
            const basegfx::B2DVector& getOffsetPosition() const;
            const basegfx::B2DVector& getRectPoint() const;
            bool getTiling() const;
            bool getStretch() const;

            // FillBitmapAttribute generator
            FillBitmapAttribute getFillBitmapAttribute(const basegfx::B2DRange& rRange) const;
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRFILLBITMAPATTRIBUTE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
