/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _SV_SALVD_HXX
#define _SV_SALVD_HXX

#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <basebmp/bitmapdevice.hxx>

class SalGraphics;

class VCL_PLUGIN_PUBLIC SalVirtualDevice
{
public:                     // public for Sal Implementation
    SalVirtualDevice() {}
    virtual ~SalVirtualDevice();

    // SalGraphics or NULL, but two Graphics for all SalVirtualDevices
    // must be returned
    virtual SalGraphics*    GetGraphics() = 0;
    virtual void            ReleaseGraphics( SalGraphics* pGraphics ) = 0;

    // Set new size, without saving the old contents
    virtual sal_Bool        SetSize( long nNewDX, long nNewDY ) = 0;

    // Set new size using a buffer at the given address
    virtual sal_Bool        SetSizeUsingBuffer( long nNewDX, long nNewDY, const basebmp::RawMemorySharedArray & /* pBuffer */ )
        {
            // Only the headless virtual device has an implementation that uses pBuffer.
            return SetSize( nNewDX, nNewDY );
        }

    /// Get actual VDev size in pixel
    virtual void            GetSize( long& rWidth, long& rHeight ) = 0;
};

#endif // _SV_SALVD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
