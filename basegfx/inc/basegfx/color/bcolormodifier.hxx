/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
        BCOLORMODIFYMODE_REPLACE,               // replace all color with local color
        BCOLORMODIFYMODE_INTERPOLATE,           // interpolate color between given and local with local value
        BCOLORMODIFYMODE_GRAY,                  // convert color to gray
        BCOLORMODIFYMODE_BLACKANDWHITE,         // convert color to B&W, local value is treshhold
        BCOLORMODIFYMODE_INVERT,                // invert color
        BCOLORMODIFYMODE_LUMINANCE_TO_ALPHA     // convert color to alpha value (used for Svg Mask)
    };

    /** Class to hold a color, value and mode for a color modification. Color modification is
        done calling the getModifiedColor() method
    */
    class BASEGFX_DLLPUBLIC SAL_WARN_UNUSED BColorModifier
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
