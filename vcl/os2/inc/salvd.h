/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salvd.h,v $
 * $Revision: 1.6 $
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

#ifndef _SV_SALVD_H
#define _SV_SALVD_H

#include <vcl/sv.h>
#include <vcl/salvd.hxx>

class Os2SalGraphics;

// -----------------
// - SalVirDevData -
// -----------------

//class SalVirDevData
class Os2SalVirtualDevice : public SalVirtualDevice
{
public:
    HPS                     mhPS;                   // HPS
    HDC                     mhDC;                   // HDC
    HBITMAP                 mhBmp;                  // Memory Bitmap
    HBITMAP                 mhDefBmp;               // Default Bitmap
    Os2SalGraphics*         mpGraphics;             // current VirDev graphics
    USHORT                  mnBitCount;             // BitCount (0 or 1)
    BOOL                    mbGraphics;             // is Graphics used

    Os2SalVirtualDevice();
    virtual ~Os2SalVirtualDevice();

    virtual SalGraphics*            GetGraphics();
    virtual void                    ReleaseGraphics( SalGraphics* pGraphics );
    virtual BOOL                    SetSize( long nNewDX, long nNewDY );
    virtual void                    GetSize( long& rWidth, long& rHeight );
};

// Help-Functions
HBITMAP ImplCreateVirDevBitmap( HDC hDC, HPS hPS, long nDX, long nDY,
                                USHORT nBitCount );

#endif // _SV_SALVD_H
