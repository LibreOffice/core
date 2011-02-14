/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SVX_TSPTITEM_HXX
#define _SVX_TSPTITEM_HXX

// include ---------------------------------------------------------------

#include <svl/poolitem.hxx>
#include <editeng/svxenum.hxx>
#include <editeng/editengdllapi.h>

// class SvxTabStop ------------------------------------------------------

#define SVX_TAB_DEFCOUNT    10
#define SVX_TAB_DEFDIST     1134            // 2cm in twips
#define SVX_TAB_NOTFOUND    USHRT_MAX
#define cDfltDecimalChar    (sal_Unicode(0x00)) // aus IntlWrapper besorgen
#define cDfltFillChar       (sal_Unicode(' '))

class EDITENG_DLLPUBLIC SvxTabStop
{
private:
    long            nTabPos;

    SvxTabAdjust    eAdjustment;
    mutable sal_Unicode     m_cDecimal;
    sal_Unicode     cFill;

    EDITENG_DLLPRIVATE friend SvStream& operator<<( SvStream&, SvxTabStop& );

    void fillDecimal() const;

public:
    SvxTabStop();
#if (_MSC_VER < 1300)
    SvxTabStop( const long nPos,
                const SvxTabAdjust eAdjst = SVX_TAB_ADJUST_LEFT,
                const sal_Unicode cDec = cDfltDecimalChar,
                const sal_Unicode cFil = cDfltFillChar );
#else
    SvxTabStop::SvxTabStop( const long nPos,
                const SvxTabAdjust eAdjst = SVX_TAB_ADJUST_LEFT,
                const sal_Unicode cDec = cDfltDecimalChar,
                const sal_Unicode cFil = cDfltFillChar );
#endif

    long&           GetTabPos() { return nTabPos; }
    long            GetTabPos() const { return nTabPos; }

    SvxTabAdjust&   GetAdjustment() { return eAdjustment; }
    SvxTabAdjust    GetAdjustment() const { return eAdjustment; }

    sal_Unicode&  GetDecimal() { fillDecimal(); return m_cDecimal; }
    sal_Unicode   GetDecimal() const { fillDecimal(); return m_cDecimal; }

    sal_Unicode&  GetFill() { return cFill; }
    sal_Unicode   GetFill() const { return cFill; }

    String          GetValueString() const;

    // das "alte" operator==()
    sal_Bool            IsEqual( const SvxTabStop& rTS ) const
                        {
                            return ( nTabPos     == rTS.nTabPos     &&
                                     eAdjustment == rTS.eAdjustment &&
                                     m_cDecimal    == rTS.m_cDecimal    &&
                                     cFill       == rTS.cFill );
                        }

    // Fuer das SortedArray:
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

SV_DECL_VARARR_SORT_VISIBILITY( SvxTabStopArr, SvxTabStop, SVX_TAB_DEFCOUNT, 1, EDITENG_DLLPUBLIC )

/*
[Beschreibung]
Dieses Item beschreibt eine Liste von TabStops.
*/

class EDITENG_DLLPUBLIC SvxTabStopItem : public SfxPoolItem, private SvxTabStopArr
{
//friend class SvxTabStopObject_Impl;

public:
    TYPEINFO();

    SvxTabStopItem( sal_uInt16 nWhich  );
    SvxTabStopItem( const sal_uInt16 nTabs,
                    const sal_uInt16 nDist,
                    const SvxTabAdjust eAdjst /*= SVX_TAB_ADJUST_DEFAULT*/,
                    sal_uInt16 nWhich  );
    SvxTabStopItem( const SvxTabStopItem& rTSI );

    // Liefert Index-Position des Tabs zurueck oder TAB_NOTFOUND
    sal_uInt16          GetPos( const SvxTabStop& rTab ) const;

    // Liefert Index-Position des Tabs an nPos zurueck oder TAB_NOTFOUND
    sal_uInt16          GetPos( const long nPos ) const;

    // entprivatisiert:
    sal_uInt16          Count() const { return SvxTabStopArr::Count(); }
    sal_Bool            Insert( const SvxTabStop& rTab );
    void            Insert( const SvxTabStopItem* pTabs, sal_uInt16 nStart = 0,
                            sal_uInt16 nEnd = USHRT_MAX );
    void            Remove( SvxTabStop& rTab )
                        { SvxTabStopArr::Remove( rTab ); }
    void            Remove( const sal_uInt16 nPos, const sal_uInt16 nLen = 1 )
                        { SvxTabStopArr::Remove( nPos, nLen ); }

    // Zuweisungsoperator, Gleichheitsoperator (vorsicht: teuer!)
    SvxTabStopItem& operator=( const SvxTabStopItem& rTSI );

    // this is already included in SfxPoolItem declaration
    //int             operator!=( const SvxTabStopItem& rTSI ) const
    //                  { return !( operator==( rTSI ) ); }

    // SortedArrays liefern nur Stackobjekte zurueck!
    const SvxTabStop& operator[]( const sal_uInt16 nPos ) const
                        {
                            DBG_ASSERT( GetStart() &&
                                        nPos < Count(), "op[]" );
                            return *( GetStart() + nPos );
                        }
    const SvxTabStop*  GetStart() const
                        {   return SvxTabStopArr::GetData(); }

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int              operator==( const SfxPoolItem& ) const;
    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*     Create( SvStream&, sal_uInt16 ) const;
    virtual SvStream&        Store( SvStream& , sal_uInt16 nItemVersion ) const;

    using SvxTabStopArr::Insert;
    using SvxTabStopArr::Remove;
};

#endif

