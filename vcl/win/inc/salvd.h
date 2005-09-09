/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salvd.h,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 14:00:37 $
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

#ifndef _SV_SV_H
#include <sv.h>
#endif

#ifndef _SV_SALVD_HXX
#include <salvd.hxx>
#endif

class WinSalGraphics;

// -----------------
// - SalVirDevData -
// -----------------

class WinSalVirtualDevice : public SalVirtualDevice
{
public:
    HDC                     mhDC;                   // HDC or 0 for Cache Device
    HBITMAP                 mhBmp;                  // Memory Bitmap
    HBITMAP                 mhDefBmp;               // Default Bitmap
    WinSalGraphics*         mpGraphics;             // current VirDev graphics
    WinSalVirtualDevice*    mpNext;                 // next VirDev
    USHORT                  mnBitCount;             // BitCount (0 or 1)
    BOOL                    mbGraphics;             // is Graphics used
    BOOL                    mbForeignDC;            // uses a foreign DC instead of a bitmap

    WinSalVirtualDevice();
    virtual ~WinSalVirtualDevice();

    virtual SalGraphics*            GetGraphics();
    virtual void                    ReleaseGraphics( SalGraphics* pGraphics );
    virtual BOOL                    SetSize( long nNewDX, long nNewDY );
};

#endif // _SV_SALVD_H
