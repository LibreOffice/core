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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include "vcl/salbtype.hxx"

#include "aqua/salcolorutils.hxx"

// =======================================================================

SalColor GetSalColor( const float* pQuartzColor )
{
    return MAKE_SALCOLOR( sal_uInt8( pQuartzColor[0] * 255.0), sal_uInt8(  pQuartzColor[1] * 255.0 ), sal_uInt8( pQuartzColor[2] * 255.0 ) );
}

void SetSalColor( const SalColor& rColor, float* pQuartzColor )
{
    pQuartzColor[0] = (float) SALCOLOR_RED(rColor) / 255.0;
    pQuartzColor[1] = (float) SALCOLOR_GREEN(rColor) / 255.0;
    pQuartzColor[2] = (float) SALCOLOR_BLUE(rColor) / 255.0;
    pQuartzColor[3] = 1.0;
}

// =======================================================================

