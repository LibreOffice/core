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

#include <basegfx/color/bcolormodifier.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    ::basegfx::BColor BColorModifier::getModifiedColor(const ::basegfx::BColor& aSourceColor) const
    {
        switch(meMode)
        {
            case BCOLORMODIFYMODE_INTERPOLATE :
            {
                return interpolate(maBColor, aSourceColor, mfValue);
            }
            case BCOLORMODIFYMODE_GRAY :
            {
                const double fLuminance(aSourceColor.luminance());
                return ::basegfx::BColor(fLuminance, fLuminance, fLuminance);
            }
            case BCOLORMODIFYMODE_BLACKANDWHITE :
            {
                const double fLuminance(aSourceColor.luminance());

                if(fLuminance < mfValue)
                {
                    return ::basegfx::BColor::getEmptyBColor();
                }
                else
                {
                    return ::basegfx::BColor(1.0, 1.0, 1.0);
                }
            }
            default : // BCOLORMODIFYMODE_REPLACE
            {
                return maBColor;
            }
        }
    }
} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
