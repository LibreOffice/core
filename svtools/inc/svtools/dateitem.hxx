 /*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dateitem.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:15:47 $
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
#ifndef _DATETIMEITEM_HXX
#define _DATETIMEITEM_HXX

#ifndef _RTTI_HXX
#include <tools/rtti.hxx>
#endif

#ifndef _DATETIME_HXX
#include <tools/datetime.hxx>
#endif

#include <svtools/poolitem.hxx>

class SvStream;

DBG_NAMEEX(SfxDateTimeItem)

// class SfxDateTimeItem -------------------------------------------------

class SfxDateTimeItem : public SfxPoolItem
{
private:
    DateTime                aDateTime;

public:
                            TYPEINFO();

                            SfxDateTimeItem( USHORT nWhich );
                            SfxDateTimeItem( USHORT nWhich,
                                             const DateTime& rDT );
                            SfxDateTimeItem( const SfxDateTimeItem& rCpy );

                            ~SfxDateTimeItem() {
                                DBG_DTOR(SfxDateTimeItem, 0); }

    virtual int             operator==( const SfxPoolItem& ) const;
    using SfxPoolItem::Compare;
    virtual int             Compare( const SfxPoolItem &rWith ) const;
    virtual SfxPoolItem*    Create( SvStream&, USHORT nItemVersion ) const;
    virtual SvStream&       Store( SvStream&, USHORT nItemVersion ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    XubString &rText,
                                    const IntlWrapper * pIntlWrapper = 0 )
        const;

    const DateTime&         GetDateTime() const { return aDateTime; }
    void                    SetDateTime( const DateTime& rDT ) {
                                DBG_ASSERT( GetRefCount() == 0,
                                            "SetDateTime() with pooled item" );
                                aDateTime = rDT; }

    virtual BOOL PutValue  ( const com::sun::star::uno::Any& rVal,
                             BYTE nMemberId = 0 );
    virtual BOOL QueryValue( com::sun::star::uno::Any& rVal,
                             BYTE nMemberId = 0 ) const;
};

class SfxColumnDateTimeItem : public SfxDateTimeItem
{
public:
                            TYPEINFO();

                            SfxColumnDateTimeItem( USHORT nWhich );
                            SfxColumnDateTimeItem( USHORT nWhich,
                                             const DateTime& rDT );
                            SfxColumnDateTimeItem( const SfxDateTimeItem& rCpy );

                            ~SfxColumnDateTimeItem() {}

    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    XubString &rText,
                                    const IntlWrapper * pIntlWrapper = 0 )
        const;
};

#endif

