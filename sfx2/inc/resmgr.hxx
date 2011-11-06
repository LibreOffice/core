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


#ifndef _SFX_RESMGR_HXX
#define _SFX_RESMGR_HXX

#include <sfx2/minarray.hxx>

class SfxMessageDescription;
class SfxMessageTable;
class Bitmap;
class ResMgr;

DECL_PTRARRAY(SfxResMgrArr, ResMgr *, 1, 1);

// INCLUDE ---------------------------------------------------------------

class SfxResourceManager
{
    SfxResMgrArr                aResMgrArr;
    SfxResMgrArr                aResMgrBmpArr;
    sal_uInt16                      nEnterCount;
    SfxMessageTable*            pMessageTable;

private:
    void                        ClearMsgTable_Impl();
    SfxMessageDescription*      MakeDesc_Impl(sal_uInt16);

public:
                                SfxResourceManager();
                                ~SfxResourceManager();

    sal_uInt16                      RegisterResource( const char *pFileName);
    void                        ReleaseResource( sal_uInt16 nRegisterId );

    sal_uInt16                      RegisterBitmap(const char *pMono, const char *pColor);

    sal_uInt16                      RegisterBitmap( const char *pSingleFile );
    void                        ReleaseBitmap( sal_uInt16 nRegisterId );

    Bitmap                      GetAllBitmap( sal_uInt16 nBmpsPerRow );

    void                        Enter();
    void                        Leave();
    SfxMessageDescription*      CreateDescription( sal_uInt16 nId );
};


#define SFX_RESMANAGER() SFX_APP()->GetResourceManager()

#endif

