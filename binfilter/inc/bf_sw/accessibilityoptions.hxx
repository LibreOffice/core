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
#ifndef _ACCESSIBILITYOPTIONS_HXX
#define _ACCESSIBILITYOPTIONS_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
namespace binfilter {

struct SwAccessibilityOptions
{
    BOOL bIsAlwaysAutoColor         :1;
    BOOL bIsStopAnimatedText        :1;
    BOOL bIsStopAnimatedGraphics    :1;

    SwAccessibilityOptions() :
        bIsAlwaysAutoColor(FALSE),
        bIsStopAnimatedText(FALSE),
        bIsStopAnimatedGraphics(FALSE) {}

    inline BOOL IsAlwaysAutoColor() const       { return bIsAlwaysAutoColor; }
    inline void SetAlwaysAutoColor( BOOL b )    { bIsAlwaysAutoColor = b; }

    inline BOOL IsStopAnimatedGraphics() const       { return bIsStopAnimatedText;}
    inline void SetStopAnimatedGraphics( BOOL b )    { bIsStopAnimatedText = b; }

    inline BOOL IsStopAnimatedText() const       { return bIsStopAnimatedGraphics; }
    inline void SetStopAnimatedText( BOOL b )    { bIsStopAnimatedGraphics = b;}
};
} //namespace binfilter
#endif

