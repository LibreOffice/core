/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _SV_SALVD_H
#define _SV_SALVD_H

#include <vcl/sv.h>
#include <vcl/salvd.hxx>

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
    sal_uInt16                  mnBitCount;             // BitCount (0 or 1)
    sal_Bool                    mbGraphics;             // is Graphics used
    sal_Bool                    mbForeignDC;            // uses a foreign DC instead of a bitmap

    WinSalVirtualDevice();
    virtual ~WinSalVirtualDevice();

    virtual SalGraphics*            GetGraphics();
    virtual void                    ReleaseGraphics( SalGraphics* pGraphics );
    virtual sal_Bool                    SetSize( long nNewDX, long nNewDY );
    virtual void                    GetSize( long& rWidth, long& rHeight );
};

#endif // _SV_SALVD_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
