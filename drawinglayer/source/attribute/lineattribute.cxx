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

#include <drawinglayer/attribute/lineattribute.hxx>
#include <basegfx/color/bcolor.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class ImpLineAttribute
        {
        public:
            // refcounter
            sal_uInt32                              mnRefCount;

            // data definitions
            basegfx::BColor                         maColor;                // color
            double                                  mfWidth;                // absolute line width
            basegfx::B2DLineJoin                    meLineJoin;             // type of LineJoin

            ImpLineAttribute(
                const basegfx::BColor& rColor,
                double fWidth,
                basegfx::B2DLineJoin aB2DLineJoin)
            :   mnRefCount(0),
                maColor(rColor),
                mfWidth(fWidth),
                meLineJoin(aB2DLineJoin)
            {
            }

            // data read access
            const basegfx::BColor& getColor() const { return maColor; }
            double getWidth() const { return mfWidth; }
            basegfx::B2DLineJoin getLineJoin() const { return meLineJoin; }

            bool operator==(const ImpLineAttribute& rCandidate) const
            {
                return (getColor() == rCandidate.getColor()
                    && getWidth() == rCandidate.getWidth()
                    && getLineJoin() == rCandidate.getLineJoin());
            }

            static ImpLineAttribute* get_global_default()
            {
                static ImpLineAttribute* pDefault = 0;

                if(!pDefault)
                {
                    pDefault = new ImpLineAttribute(
                        basegfx::BColor(),
                        0.0,
                        basegfx::B2DLINEJOIN_ROUND);

                    // never delete; start with RefCount 1, not 0
                    pDefault->mnRefCount++;
                }

                return pDefault;
            }
        };

        LineAttribute::LineAttribute(
            const basegfx::BColor& rColor,
            double fWidth,
            basegfx::B2DLineJoin aB2DLineJoin)
        :   mpLineAttribute(new ImpLineAttribute(
                rColor, fWidth, aB2DLineJoin))
        {
        }

        LineAttribute::LineAttribute()
        :   mpLineAttribute(ImpLineAttribute::get_global_default())
        {
            mpLineAttribute->mnRefCount++;
        }

        LineAttribute::LineAttribute(const LineAttribute& rCandidate)
        :   mpLineAttribute(rCandidate.mpLineAttribute)
        {
            mpLineAttribute->mnRefCount++;
        }

        LineAttribute::~LineAttribute()
        {
            if(mpLineAttribute->mnRefCount)
            {
                mpLineAttribute->mnRefCount--;
            }
            else
            {
                delete mpLineAttribute;
            }
        }

        bool LineAttribute::isDefault() const
        {
            return mpLineAttribute == ImpLineAttribute::get_global_default();
        }

        LineAttribute& LineAttribute::operator=(const LineAttribute& rCandidate)
        {
            if(rCandidate.mpLineAttribute != mpLineAttribute)
            {
                if(mpLineAttribute->mnRefCount)
                {
                    mpLineAttribute->mnRefCount--;
                }
                else
                {
                    delete mpLineAttribute;
                }

                mpLineAttribute = rCandidate.mpLineAttribute;
                mpLineAttribute->mnRefCount++;
            }

            return *this;
        }

        bool LineAttribute::operator==(const LineAttribute& rCandidate) const
        {
            if(rCandidate.mpLineAttribute == mpLineAttribute)
            {
                return true;
            }

            if(rCandidate.isDefault() != isDefault())
            {
                return false;
            }

            return (*rCandidate.mpLineAttribute == *mpLineAttribute);
        }

        const basegfx::BColor& LineAttribute::getColor() const
        {
            return mpLineAttribute->getColor();
        }

        double LineAttribute::getWidth() const
        {
            return mpLineAttribute->getWidth();
        }

        basegfx::B2DLineJoin LineAttribute::getLineJoin() const
        {
            return mpLineAttribute->getLineJoin();
        }

    } // end of namespace attribute
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
