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
    sal_Bool                    mbGraphics;             // is Graphics used

    Os2SalVirtualDevice();
    virtual ~Os2SalVirtualDevice();

    virtual SalGraphics*            GetGraphics();
    virtual void                    ReleaseGraphics( SalGraphics* pGraphics );
    virtual sal_Bool                    SetSize( long nNewDX, long nNewDY );
    virtual void                    GetSize( long& rWidth, long& rHeight );
};

// Help-Functions
HBITMAP ImplCreateVirDevBitmap( HDC hDC, HPS hPS, long nDX, long nDY,
                                USHORT nBitCount );

#endif // _SV_SALVD_H
