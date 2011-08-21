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
#ifndef _ACCESSIBILITYOPTIONS_HXX
#define _ACCESSIBILITYOPTIONS_HXX

#include <tools/solar.h>

struct SwAccessibilityOptions
{
    sal_Bool bIsAlwaysAutoColor         :1;
    sal_Bool bIsStopAnimatedText        :1;
    sal_Bool bIsStopAnimatedGraphics    :1;

    SwAccessibilityOptions() :
        bIsAlwaysAutoColor(sal_False),
        bIsStopAnimatedText(sal_False),
        bIsStopAnimatedGraphics(sal_False) {}

    inline sal_Bool IsAlwaysAutoColor() const       { return bIsAlwaysAutoColor; }
    inline void SetAlwaysAutoColor( sal_Bool b )    { bIsAlwaysAutoColor = b; }

    inline sal_Bool IsStopAnimatedGraphics() const       { return bIsStopAnimatedText;}
    inline void SetStopAnimatedGraphics( sal_Bool b )    { bIsStopAnimatedText = b; }

    inline sal_Bool IsStopAnimatedText() const       { return bIsStopAnimatedGraphics; }
    inline void SetStopAnimatedText( sal_Bool b )    { bIsStopAnimatedGraphics = b;}
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
