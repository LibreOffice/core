/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salvd.h,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 10:37:00 $
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

#ifndef _SV_SALVD_H
#define _SV_SALVD_H

#include <premac.h>
#include <ApplicationServices/ApplicationServices.h>
#include <postmac.h>

#ifndef _SV_SV_H
    #include <sv.h>
#endif

#ifndef _SV_SALGDI_HXX
    #include <salgdi.hxx>
#endif

#ifndef _SV_SALCONST_H
    #include <salconst.h>
#endif

#ifndef _SV_SALCOLORUTILS_HXX
    #include <salcolorutils.hxx>
#endif

#if PRAGMA_ONCE
    #pragma once
#endif

// =======================================================================

// =======================================================================

// -----------------
// - SalVirDevData -
// -----------------

struct SalVirDevData
{
    SalGraphics  *mpGraphics;    // current VirDev graphics
    USHORT        mnBitCount;    // GWorld pixel depth
    long          mnWidth;       // GWorld width
    long          mnHeight;      // GWorld height
    BOOL          mbGraphics;    // is Graphics used?
};

typedef struct SalVirDevData   SalVirDevData;
typedef SalVirDevData         *SalVirDevDataPtr;
typedef SalVirDevDataPtr      *SalVirDevDataHandle;

// =======================================================================

// =======================================================================

#endif // _SV_SALVD_H
