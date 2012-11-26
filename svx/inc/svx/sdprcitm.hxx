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


#ifndef _SDPRCITM_HXX
#define _SDPRCITM_HXX

#include <svl/intitem.hxx>
#include "svx/svxdllapi.h"


//------------------------------------------------------------
// class SdrPercentItem
// Ganze Prozente von 0
//------------------------------------------------------------

class SVX_DLLPUBLIC SdrPercentItem : public SfxUInt16Item
{
public:
    SdrPercentItem(): SfxUInt16Item() {}
    SdrPercentItem(sal_uInt16 nId, sal_uInt16 nVal=0): SfxUInt16Item(nId,nVal) {}
    SdrPercentItem(sal_uInt16 nId, SvStream& rIn):  SfxUInt16Item(nId,rIn) {}
    virtual SfxPoolItem* Clone(SfxItemPool* pPool=NULL) const;
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nVer) const;

    virtual SfxItemPresentation GetPresentation(SfxItemPresentation ePres, SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, String& rText, const IntlWrapper * = 0) const;

#ifdef SDR_ISPOOLABLE
    virtual int IsPoolable() const;
#endif
};

//------------------------------------------------------------
// class SdrSignedPercentItem
// Ganze Prozente von +/-
//------------------------------------------------------------

class SVX_DLLPUBLIC SdrSignedPercentItem : public SfxInt16Item
{
public:
    SdrSignedPercentItem(): SfxInt16Item() {}
    SdrSignedPercentItem( sal_uInt16 nId, sal_Int16 nVal = 0 ) : SfxInt16Item( nId,nVal ) {}
    SdrSignedPercentItem( sal_uInt16 nId, SvStream& rIn ) : SfxInt16Item( nId,rIn ) {}
    virtual SfxPoolItem* Clone( SfxItemPool* pPool = NULL ) const;
    virtual SfxPoolItem* Create( SvStream& rIn, sal_uInt16 nVer ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres, SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, String& rText, const IntlWrapper * = 0 ) const;

#ifdef SDR_ISPOOLABLE
    virtual int IsPoolable() const;
#endif
};


#endif
