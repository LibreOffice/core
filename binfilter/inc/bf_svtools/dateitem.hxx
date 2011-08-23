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
#ifndef _DATETIMEITEM_HXX
#define _DATETIMEITEM_HXX

#ifndef _RTTI_HXX
#include <tools/rtti.hxx>
#endif

#ifndef _DATETIME_HXX
#include <tools/datetime.hxx>
#endif

#include <bf_svtools/poolitem.hxx>

class SvStream;

namespace binfilter {


DBG_NAMEEX(SfxDateTimeItem)

// class SfxDateTimeItem -------------------------------------------------

class SfxDateTimeItem : public SfxPoolItem
{
private:
    DateTime				aDateTime;

public:
                            TYPEINFO();

                            SfxDateTimeItem( USHORT nWhich,
                                             const DateTime& rDT );
                            SfxDateTimeItem( const SfxDateTimeItem& rCpy );

                            ~SfxDateTimeItem() {
                                DBG_DTOR(SfxDateTimeItem, 0); }

    virtual	int				operator==( const SfxPoolItem& ) const;
    using SfxPoolItem::Compare;
    virtual int				Compare( const SfxPoolItem &rWith ) const;
    virtual SfxPoolItem*	Create( SvStream&, USHORT nItemVersion ) const;
    virtual SvStream&		Store( SvStream&, USHORT nItemVersion ) const;
    virtual SfxPoolItem*	Clone( SfxItemPool* pPool = 0 ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    XubString &rText,
                                    const ::IntlWrapper * pIntlWrapper = 0 )
        const;

    const DateTime&			GetDateTime() const { return aDateTime; }
    void					SetDateTime( const DateTime& rDT ) {
                                DBG_ASSERT( GetRefCount() == 0,
                                            "SetDateTime() with pooled item" );
                                aDateTime = rDT; }

    virtual	BOOL PutValue  ( const ::com::sun::star::uno::Any& rVal,
                             BYTE nMemberId = 0 );
    virtual	BOOL QueryValue( ::com::sun::star::uno::Any& rVal,
                             BYTE nMemberId = 0 ) const;
};
/*
class SfxColumnDateTimeItem : public SfxDateTimeItem
{
public:
                            TYPEINFO();

                            ~SfxColumnDateTimeItem() {}

    virtual SfxPoolItem*	Clone( SfxItemPool* pPool = 0 ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    XubString &rText,
                                    const ::IntlWrapper * pIntlWrapper = 0 )
        const;
};
*/
}

#endif

