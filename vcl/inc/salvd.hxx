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



#ifndef _SV_SALVD_HXX
#define _SV_SALVD_HXX

#include <vcl/sv.h>
#include <vcl/dllapi.h>

class SalGraphics;

// --------------------
// - SalVirtualDevice -
// --------------------

class VCL_PLUGIN_PUBLIC SalVirtualDevice
{
public:                     // public for Sal Implementation
    SalVirtualDevice() {}
    virtual ~SalVirtualDevice();

    // SalGraphics or NULL, but two Graphics for all SalVirtualDevices
    // must be returned
    virtual SalGraphics*            GetGraphics() = 0;
    virtual void                    ReleaseGraphics( SalGraphics* pGraphics ) = 0;

                            // Set new size, without saving the old contents
    virtual sal_Bool                    SetSize( long nNewDX, long nNewDY ) = 0;

    /// Get actual VDev size in pixel
    virtual void                    GetSize( long& rWidth, long& rHeight ) = 0;
};

#endif // _SV_SALVD_HXX
