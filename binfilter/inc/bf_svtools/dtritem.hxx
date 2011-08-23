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

#ifndef _DTRITEM_HXX
#define _DTRITEM_HXX

#include <tools/rtti.hxx>

#include <tools/datetime.hxx>

#include <bf_svtools/poolitem.hxx>

class SvStream;

namespace binfilter {

DBG_NAMEEX(SfxDateTimeRangeItem)

// class SfxDateTimeRangeItem -------------------------------------------------

class SfxDateTimeRangeItem : public SfxPoolItem
{
private:
    DateTime				aStartDateTime;
    DateTime				aEndDateTime;

public:
            TYPEINFO();

            SfxDateTimeRangeItem( const SfxDateTimeRangeItem& rCpy );
            SfxDateTimeRangeItem( USHORT nWhich, const DateTime& rStartDT,
                                  const DateTime& rEndDT );

            ~SfxDateTimeRangeItem()
                { DBG_DTOR(SfxDateTimeRangeItem, 0); }

    virtual	int				operator==( const SfxPoolItem& )			const;
    using SfxPoolItem::Compare;
    virtual int				Compare( const SfxPoolItem &rWith )			const;
    virtual SfxPoolItem*	Create( SvStream&, USHORT nItemVersion )	const;
    virtual SvStream&		Store( SvStream&, USHORT nItemVersion )		const;
    virtual SfxPoolItem*	Clone( SfxItemPool* pPool = 0 )				const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    XubString &rText,
                                    const ::IntlWrapper * pIntlWrapper = 0 )
        const;

    virtual	bool            PutValue  ( const ::com::sun::star::uno::Any& rVal,
                                         BYTE nMemberId = 0 );
    virtual	bool            QueryValue( ::com::sun::star::uno::Any& rVal,
                                         BYTE nMemberId = 0 ) const;

    const DateTime&			GetStartDateTime()		const { return aStartDateTime; }
    const DateTime&			GetEndDateTime()		const { return aEndDateTime; }

    void					SetStartDateTime( const DateTime& rDT )
                            { DBG_ASSERT( GetRefCount() == 0, "SetDateTime() with pooled item" );
                              aStartDateTime = rDT; }

    void					SetEndDateTime( const DateTime& rDT )
                            { DBG_ASSERT( GetRefCount() == 0, "SetDateTime() with pooled item" );
                              aEndDateTime = rDT; }
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
