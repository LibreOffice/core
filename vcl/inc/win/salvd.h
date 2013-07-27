/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SV_SALVD_H
#define _SV_SALVD_H

#include <salvd.hxx>

class WinSalGraphics;

// -----------------
// - SalVirDevData -
// -----------------

class WinSalVirtualDevice : public SalVirtualDevice
{
private:
    HDC                     mhLocalDC;              // HDC or 0 for Cache Device

public:
    HDC getHDC() { return mhLocalDC; }
    void setHDC(HDC aNew) { mhLocalDC = aNew; }

public:
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
