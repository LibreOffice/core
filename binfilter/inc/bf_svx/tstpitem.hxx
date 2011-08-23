/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <bf_svtools/poolitem.hxx>
#include <bf_svx/svxenum.hxx>
#include <bf_svx/svxids.hrc>
namespace binfilter {

// class SvxTabStop ------------------------------------------------------

#define SVX_TAB_DEFCOUNT    10
#define SVX_TAB_DEFDIST     1134            // 2cm in twips
#define SVX_TAB_NOTFOUND    USHRT_MAX
#define cDfltDecimalChar    (sal_Unicode(0x00)) // aus IntlWrapper besorgen
#define cDfltFillChar       (sal_Unicode(' '))

class SvxTabStop
{
private:
    long            nTabPos;

    SvxTabAdjust    eAdjustment;
    sal_Unicode		cDecimal;
    sal_Unicode		cFill;

    friend SvStream& operator<<( SvStream&, SvxTabStop& );

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

    sal_Unicode&  GetDecimal() { return cDecimal; }
    sal_Unicode   GetDecimal() const { return cDecimal; }

    sal_Unicode&  GetFill() { return cFill; }
    sal_Unicode   GetFill() const { return cFill; }


    // das "alte" operator==()
    BOOL			IsEqual( const SvxTabStop& rTS ) const
                        {
                            return ( nTabPos     == rTS.nTabPos     &&
                                     eAdjustment == rTS.eAdjustment &&
                                     cDecimal    == rTS.cDecimal    &&
                                     cFill       == rTS.cFill );
                        }

    // Fuer das SortedArray:
    BOOL            operator==( const SvxTabStop& rTS ) const
                        { return nTabPos == rTS.nTabPos; }
    BOOL            operator <( const SvxTabStop& rTS ) const
                        { return nTabPos < rTS.nTabPos; }

    SvxTabStop&     operator=( const SvxTabStop& rTS )
                        {
                            nTabPos = rTS.nTabPos;
                            eAdjustment = rTS.eAdjustment;
                            cDecimal = rTS.cDecimal;
                            cFill = rTS.cFill;
                            return *this;
                        }
};

// class SvxTabStopItem --------------------------------------------------

SV_DECL_VARARR_SORT( SvxTabStopArr, SvxTabStop, SVX_TAB_DEFCOUNT, 1 )

/*
[Beschreibung]
Dieses Item beschreibt eine Liste von TabStops.
*/

class SvxTabStopItem : public SfxPoolItem, private SvxTabStopArr
{
//friend class SvxTabStopObject_Impl;

public:
    TYPEINFO();

    SvxTabStopItem( USHORT nWhich = ITEMID_TABSTOP );
    SvxTabStopItem( const USHORT nTabs,
                    const USHORT nDist,
                    const SvxTabAdjust eAdjst = SVX_TAB_ADJUST_DEFAULT,
                    USHORT nWhich = ITEMID_TABSTOP );
    SvxTabStopItem( const SvxTabStopItem& rTSI );

    // Liefert Index-Position des Tabs zurueck oder TAB_NOTFOUND
    USHORT          GetPos( const SvxTabStop& rTab ) const;

    // Liefert Index-Position des Tabs an nPos zurueck oder TAB_NOTFOUND

    // entprivatisiert:
    USHORT          Count() const { return SvxTabStopArr::Count(); }
    BOOL            Insert( const SvxTabStop& rTab );
    void            Remove( SvxTabStop& rTab )
                        { SvxTabStopArr::Remove( rTab ); }
    void            Remove( const USHORT nPos, const USHORT nLen = 1 )
                        { SvxTabStopArr::Remove( nPos, nLen ); }

    // Zuweisungsoperator, Gleichheitsoperator (vorsicht: teuer!)

    int             operator!=( const SvxTabStopItem& rTSI ) const
                        { return !( operator==( rTSI ) ); }

    // SortedArrays liefern nur Stackobjekte zurueck!
    const SvxTabStop& operator[]( const USHORT nPos ) const
                        {
                            DBG_ASSERT( GetStart() &&
                                        nPos < Count(), "op[]" );
                            return *( GetStart() + nPos );
                        }
    const SvxTabStop*  GetStart() const
                        {   return SvxTabStopArr::GetData(); }

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int 			 operator==( const SfxPoolItem& ) const;
    virtual	bool             QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual	bool             PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );


    virtual SfxPoolItem*	 Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*	 Create( SvStream&, USHORT ) const;
    virtual SvStream&		 Store( SvStream& , USHORT nItemVersion ) const;
};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
