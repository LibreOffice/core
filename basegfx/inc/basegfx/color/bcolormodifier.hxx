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

#ifndef _BGFX_COLOR_BCOLORMODIFIER_HXX
#define _BGFX_COLOR_BCOLORMODIFIER_HXX

#include <basegfx/color/bcolor.hxx>
#include <vector>
#include <basegfx/basegfxdllapi.h>

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    /** Descriptor for type of color modification
    */
    enum BColorModifyMode
    {
        BCOLORMODIFYMODE_REPLACE,       // replace all color with local color
        BCOLORMODIFYMODE_INTERPOLATE,   // interpolate color between given and local with local value
        BCOLORMODIFYMODE_GRAY,          // convert color to gray
        BCOLORMODIFYMODE_BLACKANDWHITE  // convert color to B&W, local value is treshhold
    };

    /** Class to hold a color, value and mode for a color modification. Color modification is
        done calling the getModifiedColor() method
    */
    class BASEGFX_DLLPUBLIC BColorModifier
    {
    protected:
        ::basegfx::BColor                           maBColor;
        double                                      mfValue;
        BColorModifyMode                            meMode;

    public:
        BColorModifier(
            const ::basegfx::BColor& rBColor,
            double fValue = 0.5,
            BColorModifyMode eMode = BCOLORMODIFYMODE_REPLACE)
        :   maBColor(rBColor),
            mfValue(fValue),
            meMode(eMode)
        {}

        // compare operator(s)
        bool operator==(const BColorModifier& rCompare) const
        {
            return (maBColor == rCompare.maBColor && mfValue == rCompare.mfValue && meMode == rCompare.meMode);
        }

        bool operator!=(const BColorModifier& rCompare) const
        {
            return !(operator==(rCompare));
        }

        // data access
        const ::basegfx::BColor& getBColor() const { return maBColor; }
        double getValue() const { return mfValue; }
        BColorModifyMode getMode() const { return meMode; }

        // compute modified color
        ::basegfx::BColor getModifiedColor(const ::basegfx::BColor& aSourceColor) const;
    };

    /** Class to hold a stack of BColorModifiers and to get the modified color with
        applying all existing entry changes
    */
    class BASEGFX_DLLPUBLIC BColorModifierStack
    {
    protected:
        ::std::vector< BColorModifier >             maBColorModifiers;

    public:
        sal_uInt32 count() const
        {
            return maBColorModifiers.size();
        }

        const BColorModifier& getBColorModifier(sal_uInt32 nIndex) const
        {
            OSL_ENSURE(nIndex < count(), "BColorModifierStack: Access out of range (!)");
            return maBColorModifiers[nIndex];
        }

        ::basegfx::BColor getModifiedColor(const ::basegfx::BColor& rSource) const
        {
            if(count())
            {
                ::basegfx::BColor aRetval(rSource);
                ::std::vector< BColorModifier >::const_iterator aEnd(maBColorModifiers.end());

                while(aEnd != maBColorModifiers.begin())
                {
                    aRetval = (--aEnd)->getModifiedColor(aRetval);
                }

                return aRetval;
            }
            else
            {
                return rSource;
            }
        }

        void push(const BColorModifier& rNew)
        {
            maBColorModifiers.push_back(rNew);
        }

        void pop()
        {
            maBColorModifiers.pop_back();
        }
    };
} // end of namespace basegfx

#endif // _BGFX_COLOR_BCOLORMODIFIER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
