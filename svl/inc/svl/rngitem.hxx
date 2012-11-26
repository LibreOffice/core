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



#ifndef _SFXRNGITEM_HXX

#ifndef NUMTYPE

#define NUMTYPE sal_uInt16
#define SfxXRangeItem SfxRangeItem
#define SfxXRangesItem SfxUShortRangesItem
#include <svl/rngitem.hxx>
#undef NUMTYPE
#undef SfxXRangeItem
#undef SfxXRangesItem

#ifndef _SFXITEMS_HXX
#define NUMTYPE sal_uLong
#define SfxXRangeItem SfxULongRangeItem
#define SfxXRangesItem SfxULongRangesItem
#include <svl/rngitem.hxx>
#undef NUMTYPE
#undef SfxXRangeItem
#undef SfxXRangesItem
#endif

#define _SFXRNGITEM_HXX

#else
#include "svl/svldllapi.h"
#include <svl/poolitem.hxx>

class SvStream;

// -----------------------------------------------------------------------

class SVL_DLLPUBLIC SfxXRangeItem : public SfxPoolItem
{
private:
    NUMTYPE                     nFrom;
    NUMTYPE                     nTo;
public:
                                SfxXRangeItem();
                                SfxXRangeItem( sal_uInt16 nWID, NUMTYPE nFrom, NUMTYPE nTo );
                                SfxXRangeItem( sal_uInt16 nWID, SvStream &rStream );
                                SfxXRangeItem( const SfxXRangeItem& rItem );
    virtual int                 operator==( const SfxPoolItem& ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    XubString &rText,
                                    const IntlWrapper * = 0 ) const;
    virtual SfxPoolItem*        Clone( SfxItemPool *pPool = 0 ) const;
    inline NUMTYPE&             From() { return nFrom; }
    inline NUMTYPE              From() const { return nFrom; }
    inline NUMTYPE&             To() { return nTo; }
    inline NUMTYPE              To() const { return nTo; }
    inline sal_Bool                 HasRange() const { return nTo>nFrom; }
    virtual SfxPoolItem*        Create( SvStream &, sal_uInt16 nVersion ) const;
    virtual SvStream&           Store( SvStream &, sal_uInt16 nItemVersion ) const;
};

// -----------------------------------------------------------------------

class SVL_DLLPUBLIC SfxXRangesItem : public SfxPoolItem
{
private:
    NUMTYPE*                    _pRanges;

public:
                                SfxXRangesItem();
                                SfxXRangesItem( sal_uInt16 nWID, const NUMTYPE *pRanges );
                                SfxXRangesItem( sal_uInt16 nWID, SvStream &rStream );
                                SfxXRangesItem( const SfxXRangesItem& rItem );
    virtual                     ~SfxXRangesItem();
    virtual int                 operator==( const SfxPoolItem& ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    XubString &rText,
                                    const IntlWrapper * = 0 ) const;
    virtual SfxPoolItem*        Clone( SfxItemPool *pPool = 0 ) const;
    inline const NUMTYPE*       GetRanges() const { return _pRanges; }
    virtual SfxPoolItem*        Create( SvStream &, sal_uInt16 nVersion ) const;
    virtual SvStream&           Store( SvStream &, sal_uInt16 nItemVersion ) const;
};

#endif
#endif
