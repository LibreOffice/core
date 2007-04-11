/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xenum.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:49:58 $
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

#ifndef _XENUM_HXX
#define _XENUM_HXX

enum XLineStyle         { XLINE_NONE, XLINE_SOLID, XLINE_DASH };
enum XLineJoint
{
    XLINEJOINT_NONE,        // no rounding
    XLINEJOINT_MIDDLE,      // calc middle value between joints
    XLINEJOINT_BEVEL,       // join edges with line
    XLINEJOINT_MITER,       // extend till cut
    XLINEJOINT_ROUND        // create arc
};
enum XDashStyle         { XDASH_RECT, XDASH_ROUND, XDASH_RECTRELATIVE,
                          XDASH_ROUNDRELATIVE };
enum XFillStyle         { XFILL_NONE, XFILL_SOLID, XFILL_GRADIENT, XFILL_HATCH,
                          XFILL_BITMAP };
enum XGradientStyle     { XGRAD_LINEAR, XGRAD_AXIAL, XGRAD_RADIAL,
                          XGRAD_ELLIPTICAL, XGRAD_SQUARE, XGRAD_RECT };
enum XHatchStyle        { XHATCH_SINGLE, XHATCH_DOUBLE, XHATCH_TRIPLE };
enum XFormTextStyle     { XFT_ROTATE, XFT_UPRIGHT, XFT_SLANTX, XFT_SLANTY,
                          XFT_NONE };
enum XFormTextAdjust    { XFT_LEFT, XFT_RIGHT, XFT_AUTOSIZE, XFT_CENTER };
enum XFormTextShadow    { XFTSHADOW_NONE, XFTSHADOW_NORMAL, XFTSHADOW_SLANT};
enum XFormTextStdForm   { XFTFORM_NONE = 0, XFTFORM_TOPCIRC, XFTFORM_BOTCIRC,
                          XFTFORM_LFTCIRC, XFTFORM_RGTCIRC, XFTFORM_TOPARC,
                          XFTFORM_BOTARC, XFTFORM_LFTARC, XFTFORM_RGTARC,
                          XFTFORM_BUTTON1, XFTFORM_BUTTON2,
                          XFTFORM_BUTTON3, XFTFORM_BUTTON4};
enum XBitmapStyle       { XBITMAP_TILE, XBITMAP_STRETCH };
enum XBitmapType        { XBITMAP_IMPORT, XBITMAP_8X8, XBITMAP_NONE,
                          XBITMAP_16X16 };


#endif      // _XENUM_HXX

