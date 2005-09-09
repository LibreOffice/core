/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salpixmaputils.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 10:35:41 $
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

#ifndef _SV_SALPIXMAPUTILS_HXX
#define _SV_SALPIXMAPUTILS_HXX

#include <premac.h>
#include <ApplicationServices/ApplicationServices.h>
#include <postmac.h>

#ifndef _GEN_HXX
    #include <tools/gen.hxx>
#endif

#ifndef _SV_SALBTYPE_HXX
    #include <salbtype.hxx>
#endif

#ifndef _SV_SALGTYPE_HXX
    #include <salgtype.hxx>
#endif

#ifndef _SV_SALCONST_H
    #include <salconst.h>
#endif

#ifndef _SV_SALCOLORUTILS_HXX
    #include <salcolorutils.hxx>
#endif

// ------------------------------------------------------------------


PixMapHandle GetNewPixMap ( const Size           &rPixMapSize,
                            const USHORT          nPixMapBits,
                            const BitmapPalette  &rBitmapPalette
                          );

PixMapHandle CopyPixMap ( PixMapHandle  hPixMap );

PixMapHandle GetCGrafPortPixMap ( const Size           &rPixMapSize,
                                  const USHORT          nPixMapBits,
                                  const BitmapPalette  &rBitmapPalette,
                                  const CGrafPtr        pCGraf
                                );

// ------------------------------------------------------------------

#endif  // _SV_SALPIXMAPUTILS_HXX
