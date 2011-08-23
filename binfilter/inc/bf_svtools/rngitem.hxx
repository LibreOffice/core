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

#ifndef _SFXRNGITEM_HXX

#ifndef NUMTYPE

#define NUMTYPE	USHORT
#define SfxXRangeItem SfxRangeItem
#define SfxXRangesItem SfxUShortRangesItem
#include <bf_svtools/rngitem.hxx>
#undef NUMTYPE
#undef SfxXRangeItem
#undef SfxXRangesItem

#ifndef _SFXITEMS_HXX
#define NUMTYPE	ULONG
#define SfxXRangeItem SfxULongRangeItem
#define SfxXRangesItem SfxULongRangesItem
#include <bf_svtools/rngitem.hxx>
#undef NUMTYPE
#undef SfxXRangeItem
#undef SfxXRangesItem
#endif

#define _SFXRNGITEM_HXX

#else

#ifndef INCLUDED_SVTDLLAPI_H
#include "bf_svtools/svtdllapi.h"
#endif

#ifndef _SFXPOOLITEM_HXX
#include <bf_svtools/poolitem.hxx>
#endif

class SvStream;

namespace binfilter
{

// -----------------------------------------------------------------------

class  SfxXRangeItem : public SfxPoolItem
{
private:
    NUMTYPE 					nFrom;
    NUMTYPE 					nTo;
public:
                                TYPEINFO();
                                SfxXRangeItem();
                                SfxXRangeItem( USHORT nWID, NUMTYPE nFrom, NUMTYPE nTo );
                                SfxXRangeItem( const SfxXRangeItem& rItem );
    virtual int 				operator==( const SfxPoolItem& ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    XubString &rText,
                                    const ::IntlWrapper * = 0 ) const;
    virtual SfxPoolItem*    	Clone( SfxItemPool *pPool = 0 ) const;
    inline NUMTYPE&				From() { return nFrom; }
    inline NUMTYPE				From() const { return nFrom; }
    inline NUMTYPE&				To() { return nTo; }
    inline NUMTYPE				To() const { return nTo; }
    inline BOOL					HasRange() const { return nTo>nFrom; }
    virtual SfxPoolItem*		Create( SvStream &, USHORT nVersion ) const;
    virtual SvStream&			Store( SvStream &, USHORT nItemVersion ) const;
};

// -----------------------------------------------------------------------

class  SfxXRangesItem : public SfxPoolItem
{
private:
    NUMTYPE*					_pRanges;

public:
                                TYPEINFO();
                                SfxXRangesItem();
                                SfxXRangesItem( USHORT nWID, SvStream &rStream );
                                SfxXRangesItem( const SfxXRangesItem& rItem );
    virtual 					~SfxXRangesItem();
    virtual int 				operator==( const SfxPoolItem& ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    XubString &rText,
                                    const ::IntlWrapper * = 0 ) const;
    virtual SfxPoolItem*    	Clone( SfxItemPool *pPool = 0 ) const;
    inline const NUMTYPE*		GetRanges() const { return _pRanges; }
    virtual SfxPoolItem*		Create( SvStream &, USHORT nVersion ) const;
    virtual SvStream&			Store( SvStream &, USHORT nItemVersion ) const;
};

}

#endif
#endif
