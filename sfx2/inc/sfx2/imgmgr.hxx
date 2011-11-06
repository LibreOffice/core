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



#ifndef _SFXIMGMGR_HXX
#define _SFXIMGMGR_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"

#include <sal/types.h>
#include <tools/gen.hxx>
#ifndef _IMAGE_HXX //autogen
#include <vcl/image.hxx>
#endif

class ToolBox;
class SfxModule;
class SfxImageManager_Impl;
class SFX2_DLLPUBLIC SfxImageManager
{
    SfxImageManager_Impl* pImp;

public:
    static SfxImageManager*  GetImageManager( SfxModule* );

                    SfxImageManager( SfxModule* pModule = 0 );
                    ~SfxImageManager();

    void            RegisterToolBox( ToolBox *pBox, sal_uInt16 nFlags=0xFFFF);
    void            ReleaseToolBox( ToolBox *pBox );

                    // get images from resources
    void            SetImages( ToolBox& rToolBox );
    void            SetImages( ToolBox& rToolBox, sal_Bool bHiContrast, sal_Bool bLarge );
    void            SetImagesForceSize( ToolBox& rToolBox, sal_Bool bHiContrast, sal_Bool bLarge );

    Image           GetImage( sal_uInt16 nId, sal_Bool bLarge, sal_Bool bHiContrast ) const;
    Image           GetImage( sal_uInt16 nId, sal_Bool bHiContrast ) const;
    Image           SeekImage( sal_uInt16 nId, sal_Bool bLarge, sal_Bool bHiContrast ) const;
    Image           SeekImage( sal_uInt16 nId, sal_Bool bHiContrast ) const;
};

#endif
