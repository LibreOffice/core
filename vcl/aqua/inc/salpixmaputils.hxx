/*************************************************************************
 *
 *  $RCSfile: salpixmaputils.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: bmahbod $ $Date: 2001-03-12 23:15:31 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SV_SALPIXMAPUTILS_HXX
#define _SV_SALPIXMAPUTILS_HXX

#include <premac.h>

    #ifndef __FIXMATH__
        #include <CarbonCore/FixMath.h>
    #endif

    #ifndef __MACERRORS__      #include <CarbonCore/MacErrors.h>
    #endif

    #ifndef __MACTYPES__
        #include <CarbonCore/MacTypes.h>
    #endif

    #ifndef __MACMEMORY__
        #include <CarbonCore/MacMemory.h>
    #endif

    #ifndef __QUICKDRAW__
        #include <QD/Quickdraw.h>
    #endif

    #ifndef __QDOFFSCREEN__
        #include <QD/QDOffscreen.h>
    #endif

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

#if PRAGMA_ONCE    #pragma once#endif// ------------------------------------------------------------------


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

// ------------------------------------------------------------------#endif   // _SV_SALPIXMAPUTILS_HXX
