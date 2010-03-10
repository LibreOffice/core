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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_FILLBITMAPATTRIBUTE_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_FILLBITMAPATTRIBUTE_HXX

#include <vcl/bitmapex.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2dvector.hxx>

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace basegfx {
    class B2DRange;
    class BColor;
}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class FillBitmapAttribute
        {
            BitmapEx                                    maBitmapEx;
            basegfx::B2DPoint                           maTopLeft;
            basegfx::B2DVector                          maSize;

            // bitfield
            unsigned                                    mbTiling : 1;

        public:
            FillBitmapAttribute(
                const BitmapEx& rBitmapEx,
                const basegfx::B2DPoint& rTopLeft,
                const basegfx::B2DVector& rSize,
                bool bTiling);
            bool operator==(const FillBitmapAttribute& rCandidate) const;

            // data access
            const BitmapEx& getBitmapEx() const { return maBitmapEx; }
            const basegfx::B2DPoint& getTopLeft() const { return maTopLeft; }
            const basegfx::B2DVector& getSize() const { return maSize; }
            bool getTiling() const { return mbTiling; }
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_FILLBITMAPATTRIBUTE_HXX

// eof
