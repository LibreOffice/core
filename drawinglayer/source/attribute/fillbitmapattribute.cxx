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

#include <drawinglayer/attribute/fillbitmapattribute.hxx>
#include <vcl/bitmapex.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class ImpFillBitmapAttribute
        {
        public:
            // refcounter
            sal_uInt32                              mnRefCount;

            // data definitions
            BitmapEx                                maBitmapEx;
            basegfx::B2DPoint                       maTopLeft;
            basegfx::B2DVector                      maSize;

            // bitfield
            unsigned                                mbTiling : 1;

            ImpFillBitmapAttribute(
                const BitmapEx& rBitmapEx,
                const basegfx::B2DPoint& rTopLeft,
                const basegfx::B2DVector& rSize,
                bool bTiling)
            :   mnRefCount(0),
                maBitmapEx(rBitmapEx),
                maTopLeft(rTopLeft),
                maSize(rSize),
                mbTiling(bTiling)
            {
            }

            bool operator==(const ImpFillBitmapAttribute& rCandidate) const
            {
                return (maBitmapEx == rCandidate.maBitmapEx
                    && maTopLeft == rCandidate.maTopLeft
                    && maSize == rCandidate.maSize
                    && mbTiling == rCandidate.mbTiling);
            }

            // data read access
            const BitmapEx& getBitmapEx() const { return maBitmapEx; }
            const basegfx::B2DPoint& getTopLeft() const { return maTopLeft; }
            const basegfx::B2DVector& getSize() const { return maSize; }
            bool getTiling() const { return mbTiling; }

            static ImpFillBitmapAttribute* get_global_default()
            {
                static ImpFillBitmapAttribute* pDefault = 0;

                if(!pDefault)
                {
                    pDefault = new ImpFillBitmapAttribute(
                        BitmapEx(),
                        basegfx::B2DPoint(),
                        basegfx::B2DVector(),
                        false);

                    // never delete; start with RefCount 1, not 0
                    pDefault->mnRefCount++;
                }

                return pDefault;
            }
        };

        FillBitmapAttribute::FillBitmapAttribute(
            const BitmapEx& rBitmapEx,
            const basegfx::B2DPoint& rTopLeft,
            const basegfx::B2DVector& rSize,
            bool bTiling)
        :   mpFillBitmapAttribute(new ImpFillBitmapAttribute(
                rBitmapEx, rTopLeft, rSize, bTiling))
        {
        }

        FillBitmapAttribute::FillBitmapAttribute(const FillBitmapAttribute& rCandidate)
        :   mpFillBitmapAttribute(rCandidate.mpFillBitmapAttribute)
        {
            mpFillBitmapAttribute->mnRefCount++;
        }

        FillBitmapAttribute::~FillBitmapAttribute()
        {
            if(mpFillBitmapAttribute->mnRefCount)
            {
                mpFillBitmapAttribute->mnRefCount--;
            }
            else
            {
                delete mpFillBitmapAttribute;
            }
        }

        bool FillBitmapAttribute::isDefault() const
        {
            return mpFillBitmapAttribute == ImpFillBitmapAttribute::get_global_default();
        }

        FillBitmapAttribute& FillBitmapAttribute::operator=(const FillBitmapAttribute& rCandidate)
        {
            if(rCandidate.mpFillBitmapAttribute != mpFillBitmapAttribute)
            {
                if(mpFillBitmapAttribute->mnRefCount)
                {
                    mpFillBitmapAttribute->mnRefCount--;
                }
                else
                {
                    delete mpFillBitmapAttribute;
                }

                mpFillBitmapAttribute = rCandidate.mpFillBitmapAttribute;
                mpFillBitmapAttribute->mnRefCount++;
            }

            return *this;
        }

        bool FillBitmapAttribute::operator==(const FillBitmapAttribute& rCandidate) const
        {
            if(rCandidate.mpFillBitmapAttribute == mpFillBitmapAttribute)
            {
                return true;
            }

            if(rCandidate.isDefault() != isDefault())
            {
                return false;
            }

            return (*rCandidate.mpFillBitmapAttribute == *mpFillBitmapAttribute);
        }

        const BitmapEx& FillBitmapAttribute::getBitmapEx() const
        {
            return mpFillBitmapAttribute->getBitmapEx();
        }

        const basegfx::B2DPoint& FillBitmapAttribute::getTopLeft() const
        {
            return mpFillBitmapAttribute->getTopLeft();
        }

        const basegfx::B2DVector& FillBitmapAttribute::getSize() const
        {
            return mpFillBitmapAttribute->getSize();
        }

        bool FillBitmapAttribute::getTiling() const
        {
            return mpFillBitmapAttribute->getTiling();
        }

    } // end of namespace attribute
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
