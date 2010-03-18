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

#ifndef _SV_SALGEOM_HXX
#define _SV_SALGEOM_HXX

typedef struct _SalFrameGeometry {
    // screen position of upper left corner of drawable area in pixel
    long                nX, nY;
    // dimensions of the drawable area in pixel
    unsigned long       nWidth, nHeight;
    // thickness of the decoration in pixel
    unsigned long       nLeftDecoration,
                        nTopDecoration,
                        nRightDecoration,
                        nBottomDecoration;
    unsigned int        nScreenNumber;

    _SalFrameGeometry() :
    nX( 0 ), nY( 0 ), nWidth( 1 ), nHeight( 1 ),
    nLeftDecoration( 0 ), nTopDecoration( 0 ),
    nRightDecoration( 0 ), nBottomDecoration( 0 ),
    nScreenNumber( 0 )
    {}
} SalFrameGeometry;

#endif // _SV_SALGEOM_HXX
