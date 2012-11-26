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



#ifndef _SDGCOITM_HXX
#define _SDGCOITM_HXX

#include <svx/sdprcitm.hxx>
#include <svx/svddef.hxx>
#include "svx/svxdllapi.h"

//-----------------
// SdrGrafRedItem -
//-----------------

class SVX_DLLPUBLIC SdrGrafRedItem : public SdrSignedPercentItem
{
public:

                            SdrGrafRedItem( short nRedPercent = 0 ) : SdrSignedPercentItem( SDRATTR_GRAFRED, nRedPercent ) {}
                            SdrGrafRedItem( SvStream& rIn ) : SdrSignedPercentItem( SDRATTR_GRAFRED, rIn ) {}

    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = NULL ) const;
    virtual SfxPoolItem*    Create( SvStream& rIn, sal_uInt16 nVer ) const;
};

//-------------------
// SdrGrafGreenItem -
//-------------------

class SVX_DLLPUBLIC SdrGrafGreenItem : public SdrSignedPercentItem
{
public:

                            SdrGrafGreenItem( short nGreenPercent = 0 ) : SdrSignedPercentItem( SDRATTR_GRAFGREEN, nGreenPercent ) {}
                            SdrGrafGreenItem( SvStream& rIn ) : SdrSignedPercentItem( SDRATTR_GRAFGREEN, rIn ) {}

    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = NULL ) const;
    virtual SfxPoolItem*    Create( SvStream& rIn, sal_uInt16 nVer ) const;
};

//-------------------
// SdrGrafBlueItem -
//-------------------

class SVX_DLLPUBLIC SdrGrafBlueItem : public SdrSignedPercentItem
{
public:

                            SdrGrafBlueItem( short nBluePercent = 0 ) : SdrSignedPercentItem( SDRATTR_GRAFBLUE, nBluePercent ) {}
                            SdrGrafBlueItem( SvStream& rIn ) : SdrSignedPercentItem( SDRATTR_GRAFBLUE, rIn ) {}

    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = NULL ) const;
    virtual SfxPoolItem*    Create( SvStream& rIn, sal_uInt16 nVer ) const;
};

#endif // _SDGCOITM_HXX
