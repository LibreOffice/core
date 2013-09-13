/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef _SVX_TSPTITEM_HXX
#define _SVX_TSPTITEM_HXX

#include <svl/poolitem.hxx>
#include <editeng/svxenum.hxx>
#include <editeng/editengdllapi.h>
#include <o3tl/sorted_vector.hxx>

// class SvxTabStop ------------------------------------------------------

#define SVX_TAB_DEFCOUNT    10
#define SVX_TAB_DEFDIST     1134            // 2cm in twips
#define SVX_TAB_NOTFOUND    USHRT_MAX
#define cDfltDecimalChar    (sal_Unicode(0x00)) // Get from IntlWrapper
#define cDfltFillChar       (sal_Unicode(' '))

class EDITENG_DLLPUBLIC SvxTabStop
{
private:
    sal_Int32 nTabPos;

    SvxTabAdjust    eAdjustment;
    mutable sal_Unicode     m_cDecimal;
    sal_Unicode     cFill;

    EDITENG_DLLPRIVATE friend SvStream& operator<<( SvStream&, SvxTabStop& );

    void fillDecimal() const;

public:
    SvxTabStop();
    explicit SvxTabStop( const sal_Int32 nPos,
                const SvxTabAdjust eAdjst = SVX_TAB_ADJUST_LEFT,
                const sal_Unicode cDec = cDfltDecimalChar,
                const sal_Unicode cFil = cDfltFillChar );

    sal_Int32& GetTabPos() { return nTabPos; }
    sal_Int32 GetTabPos() const { return nTabPos; }

    SvxTabAdjust&   GetAdjustment() { return eAdjustment; }
    SvxTabAdjust    GetAdjustment() const { return eAdjustment; }

    sal_Unicode&  GetDecimal() { fillDecimal(); return m_cDecimal; }
    sal_Unicode   GetDecimal() const { fillDecimal(); return m_cDecimal; }

    sal_Unicode&  GetFill() { return cFill; }
    sal_Unicode   GetFill() const { return cFill; }

    OUString      GetValueString() const;

    // the "old" operator==()
    sal_Bool            IsEqual( const SvxTabStop& rTS ) const
                        {
                            return ( nTabPos     == rTS.nTabPos     &&
                                     eAdjustment == rTS.eAdjustment &&
                                     m_cDecimal    == rTS.m_cDecimal    &&
                                     cFill       == rTS.cFill );
                        }

    // For the SortedArray:
    sal_Bool            operator==( const SvxTabStop& rTS ) const
                        { return nTabPos == rTS.nTabPos; }
    sal_Bool            operator <( const SvxTabStop& rTS ) const
                        { return nTabPos < rTS.nTabPos; }

    SvxTabStop&     operator=( const SvxTabStop& rTS )
                        {
                            nTabPos = rTS.nTabPos;
                            eAdjustment = rTS.eAdjustment;
                            m_cDecimal = rTS.m_cDecimal;
                            cFill = rTS.cFill;
                            return *this;
                        }
};

// class SvxTabStopItem --------------------------------------------------

typedef o3tl::sorted_vector<SvxTabStop> SvxTabStopArr;

/*  [Description]

    This item describes a list of TabStops.
*/

class EDITENG_DLLPUBLIC SvxTabStopItem : public SfxPoolItem
{
    SvxTabStopArr maTabStops;

public:
    TYPEINFO();

    explicit SvxTabStopItem( sal_uInt16 nWhich  );
    SvxTabStopItem( const sal_uInt16 nTabs,
                    const sal_uInt16 nDist,
                    const SvxTabAdjust eAdjst /*= SVX_TAB_ADJUST_DEFAULT*/,
                    sal_uInt16 nWhich  );
    SvxTabStopItem( const SvxTabStopItem& rTSI );

    // Returns index of the tab or TAB_NOTFOUND
    sal_uInt16          GetPos( const SvxTabStop& rTab ) const;

    // Returns index of the tab at nPos, or TAB_NOTFOUND
    sal_uInt16          GetPos( const sal_Int32 nPos ) const;

    // unprivatized:
    sal_uInt16          Count() const { return maTabStops.size(); }
    bool                Insert( const SvxTabStop& rTab );
    void                Insert( const SvxTabStopItem* pTabs, sal_uInt16 nStart = 0,
                            sal_uInt16 nEnd = USHRT_MAX );
    void                Remove( SvxTabStop& rTab )
                        { maTabStops.erase( rTab ); }
    void                Remove( const sal_uInt16 nPos, const sal_uInt16 nLen = 1 )
                        { maTabStops.erase( maTabStops.begin() + nPos, maTabStops.begin() + nPos + nLen ); }

    // Assignment operator, equality operator (caution: expensive!)
    SvxTabStopItem& operator=( const SvxTabStopItem& rTSI );

    // this is already included in SfxPoolItem declaration
    //int             operator!=( const SvxTabStopItem& rTSI ) const
    //                  { return !( operator==( rTSI ) ); }

    const SvxTabStop& operator[]( const sal_uInt16 nPos ) const
                        { return maTabStops[nPos]; }

    const SvxTabStop& At( const sal_uInt16 nPos ) const
    {
        return maTabStops[nPos];
    }

    // "pure virtual Methods" from SfxPoolItem
    virtual int              operator==( const SfxPoolItem& ) const;
    virtual bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = 0 ) const;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*     Create( SvStream&, sal_uInt16 ) const;
    virtual SvStream&        Store( SvStream& , sal_uInt16 nItemVersion ) const;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
