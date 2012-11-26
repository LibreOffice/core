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



#ifndef _SDGLUITM_HXX
#define _SDGLUITM_HXX

#include <svx/sdprcitm.hxx>
#include <svx/svddef.hxx>
#include "svx/svxdllapi.h"

//-----------------------
// SdrGrafLuminanceItem -
//-----------------------

class SVX_DLLPUBLIC SdrGrafLuminanceItem : public SdrSignedPercentItem
{
public:

                            SdrGrafLuminanceItem( short nLuminancePercent = 0 ) : SdrSignedPercentItem( SDRATTR_GRAFLUMINANCE, nLuminancePercent ) {}
                            SdrGrafLuminanceItem( SvStream& rIn ) : SdrSignedPercentItem( SDRATTR_GRAFLUMINANCE, rIn ) {}

    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = NULL ) const;
    virtual SfxPoolItem*    Create( SvStream& rIn, sal_uInt16 nVer ) const;
};

//----------------------
// SdrGrafContrastItem -
//----------------------

class SVX_DLLPUBLIC SdrGrafContrastItem : public SdrSignedPercentItem
{
public:

                            SdrGrafContrastItem( short nContrastPercent = 0 ) : SdrSignedPercentItem( SDRATTR_GRAFCONTRAST, nContrastPercent ) {}
                            SdrGrafContrastItem( SvStream& rIn ) : SdrSignedPercentItem( SDRATTR_GRAFCONTRAST, rIn ) {}

    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = NULL ) const;
    virtual SfxPoolItem*    Create( SvStream& rIn, sal_uInt16 nVer ) const;
};

#endif // _SDGLUITM_HXX
