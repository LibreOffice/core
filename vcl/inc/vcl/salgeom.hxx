/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salgeom.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 15:47:24 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SV_SALGEOM_HXX
#define _SV_SALGEOM_HXX

typedef struct _SalFrameGeometry {
    // screen position of upper left corner of drawable area in pixel
    int                 nX, nY;
    // dimensions of the drawable area in pixel
    unsigned int        nWidth, nHeight;
    // thickness of the decoration in pixel
    unsigned int        nLeftDecoration,
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
