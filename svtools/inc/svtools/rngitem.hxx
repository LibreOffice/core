/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: rngitem.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:33:01 $
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

#ifndef _SFXRNGITEM_HXX

#ifndef NUMTYPE

#define NUMTYPE USHORT
#define SfxXRangeItem SfxRangeItem
#define SfxXRangesItem SfxUShortRangesItem
#include <svtools/rngitem.hxx>
#undef NUMTYPE
#undef SfxXRangeItem
#undef SfxXRangesItem

#ifndef _SFXITEMS_HXX
#define NUMTYPE ULONG
#define SfxXRangeItem SfxULongRangeItem
#define SfxXRangesItem SfxULongRangesItem
#include <svtools/rngitem.hxx>
#undef NUMTYPE
#undef SfxXRangeItem
#undef SfxXRangesItem
#endif

#define _SFXRNGITEM_HXX

#else

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _SFXPOOLITEM_HXX
#include <svtools/poolitem.hxx>
#endif

class SvStream;

// -----------------------------------------------------------------------

class SVT_DLLPUBLIC SfxXRangeItem : public SfxPoolItem
{
private:
    NUMTYPE                     nFrom;
    NUMTYPE                     nTo;
public:
                                TYPEINFO();
                                SfxXRangeItem();
                                SfxXRangeItem( USHORT nWID, NUMTYPE nFrom, NUMTYPE nTo );
                                SfxXRangeItem( USHORT nWID, SvStream &rStream );
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
    inline BOOL                 HasRange() const { return nTo>nFrom; }
    virtual SfxPoolItem*        Create( SvStream &, USHORT nVersion ) const;
    virtual SvStream&           Store( SvStream &, USHORT nItemVersion ) const;
};

// -----------------------------------------------------------------------

class SVT_DLLPUBLIC SfxXRangesItem : public SfxPoolItem
{
private:
    NUMTYPE*                    _pRanges;

public:
                                TYPEINFO();
                                SfxXRangesItem();
                                SfxXRangesItem( USHORT nWID, const NUMTYPE *pRanges );
                                SfxXRangesItem( USHORT nWID, SvStream &rStream );
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
    virtual SfxPoolItem*        Create( SvStream &, USHORT nVersion ) const;
    virtual SvStream&           Store( SvStream &, USHORT nItemVersion ) const;
};

#endif
#endif
