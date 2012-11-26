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



#ifndef _SDGCPITM_HXX
#define _SDGCPITM_HXX

#include <svx/grfcrop.hxx>
#include <svx/svddef.hxx>

//------------------
// SdrGrafCropItem -
//------------------

class SVX_DLLPUBLIC SdrGrafCropItem : public SvxGrfCrop
{
public:

                            SdrGrafCropItem( sal_Int32 nLeftCrop = 0, sal_Int32 nTopCrop = 0,
                                             sal_Int32 nRightCrop = 0, sal_Int32 nBottomCrop = 0,
                                             sal_uInt16 nWhichId = SDRATTR_GRAFCROP ) :
                                SvxGrfCrop( nLeftCrop, nRightCrop, nTopCrop, nBottomCrop, nWhichId ) {}

    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = NULL ) const;
    virtual SfxPoolItem*    Create( SvStream& rIn, sal_uInt16 nVer ) const;
    virtual sal_uInt16          GetVersion( sal_uInt16 nFileVersion ) const;
};

#endif // _SDGCPITM_HXX
