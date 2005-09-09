/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salcolorutils.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 10:32:59 $
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

#ifndef _SV_SALCOLORUTILS_HXX
#define _SV_SALCOLORUTILS_HXX

#ifndef _LIMITS_H
    #include <limits.h>
#endif

#include <premac.h>
#include <ApplicationServices/ApplicationServices.h>
#include <postmac.h>

#ifndef _SV_SALBTYPE_HXX
    #include <salbtype.hxx>
#endif

#ifndef _SV_SALGTYPE_HXX
    #include <salgtype.hxx>
#endif

#ifndef _SV_SALCONST_H
    #include <salconst.h>
#endif

#ifndef _SV_SALMATHUTILS_HXX
    #include <salmathutils.hxx>
#endif

// ------------------------------------------------------------------

SalColor RGBColor2SALColor ( const RGBColor *pRGBColor );

SalColor RGB8BitColor2SALColor ( const RGBColor *pRGBColor );

SalColor RGB16BitColor2SALColor ( const RGBColor *pRGBColor );

SalColor RGB32BitColor2SALColor ( const RGBColor *pRGBColor );

// ------------------------------------------------------------------

RGBColor SALColor2RGBColor ( const SalColor nSalColor );

RGBColor SALColor2RGB32bitColor ( const SalColor nSalColor );

RGBColor SALColor2RGB18bitColor ( const SalColor nSalColor );

RGBColor SALColor2RGB8bitColor ( const SalColor nSalColor );

// ------------------------------------------------------------------

SalColor GetROPSalColor ( SalROPColor nROPColor );

// ------------------------------------------------------------------

RGBColor BitmapColor2RGBColor ( const BitmapColor &rBitmapColor );

void RGBColor2BitmapColor ( const RGBColor  *rRGBColor,
                            BitmapColor     &rBitmapColor
                          );

// ------------------------------------------------------------------

short GetMinColorCount ( const short           nPixMapColorDepth,
                         const BitmapPalette  &rBitmapPalette
                       );

// ------------------------------------------------------------------

void SetBlackForeColor ( );

void SetWhiteBackColor ( );

RGBColor GetBlackColor ( );

RGBColor GetWhiteColor ( );

// ------------------------------------------------------------------

CTabHandle CopyGDeviceCTab ( );

CTabHandle GetCTabFromStdCLUT ( const short nBitDepth );

CTabHandle CopyCTabIndexed ( CTabHandle hCTab );

CTabHandle CopyCTabRGBDirect ( CTabHandle hCTab );

// ------------------------------------------------------------------

CTabHandle CopyPixMapCTab ( PixMapHandle hPixMap );

// ------------------------------------------------------------------

void SetBitmapBufferColorFormat ( const PixMapHandle   mhPixMap,
                                  BitmapBuffer        *rBuffer
                                );

// ------------------------------------------------------------------

#endif  // _SV_SALCOLORUTILS_HXX
