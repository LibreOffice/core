/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tstpitem.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:42:42 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SVX_TSPTITEM_HXX
#define _SVX_TSPTITEM_HXX

// include ---------------------------------------------------------------

#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif
#ifndef _SVX_SVXENUM_HXX //autogen
#include <svx/svxenum.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

// class SvxTabStop ------------------------------------------------------

#define SVX_TAB_DEFCOUNT    10
#define SVX_TAB_DEFDIST     1134            // 2cm in twips
#define SVX_TAB_NOTFOUND    USHRT_MAX
#define cDfltDecimalChar    (sal_Unicode(0x00)) // aus IntlWrapper besorgen
#define cDfltFillChar       (sal_Unicode(' '))

class SVX_DLLPUBLIC SvxTabStop
{
private:
    long            nTabPos;

    SvxTabAdjust    eAdjustment;
    sal_Unicode     cDecimal;
    sal_Unicode     cFill;

    SVX_DLLPRIVATE friend SvStream& operator<<( SvStream&, SvxTabStop& );

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

    String          GetValueString() const;

    // das "alte" operator==()
    BOOL            IsEqual( const SvxTabStop& rTS ) const
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

SV_DECL_VARARR_SORT_VISIBILITY( SvxTabStopArr, SvxTabStop, SVX_TAB_DEFCOUNT, 1, SVX_DLLPUBLIC )

/*
[Beschreibung]
Dieses Item beschreibt eine Liste von TabStops.
*/

#ifndef MAC
class SVX_DLLPUBLIC SvxTabStopItem : public SfxPoolItem, private SvxTabStopArr
#else
class SVX_DLLPUBLIC SvxTabStopItem : public SfxPoolItem, public SvxTabStopArr
#endif
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
    USHORT          GetPos( const long nPos ) const;

    // entprivatisiert:
    USHORT          Count() const { return SvxTabStopArr::Count(); }
    BOOL            Insert( const SvxTabStop& rTab );
    void            Insert( const SvxTabStopItem* pTabs, USHORT nStart = 0,
                            USHORT nEnd = USHRT_MAX );
    void            Remove( SvxTabStop& rTab )
                        { SvxTabStopArr::Remove( rTab ); }
    void            Remove( const USHORT nPos, const USHORT nLen = 1 )
                        { SvxTabStopArr::Remove( nPos, nLen ); }

    // Zuweisungsoperator, Gleichheitsoperator (vorsicht: teuer!)
    SvxTabStopItem& operator=( const SvxTabStopItem& rTSI );

    // this is already included in SfxPoolItem declaration
    //int             operator!=( const SvxTabStopItem& rTSI ) const
    //                  { return !( operator==( rTSI ) ); }

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
    virtual int              operator==( const SfxPoolItem& ) const;
    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*     Create( SvStream&, USHORT ) const;
    virtual SvStream&        Store( SvStream& , USHORT nItemVersion ) const;

    using SvxTabStopArr::Insert;
    using SvxTabStopArr::Remove;
};

#endif

