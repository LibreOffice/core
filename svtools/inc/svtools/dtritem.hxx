/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dtritem.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:16:24 $
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

#ifndef _DTRITEM_HXX
#define _DTRITEM_HXX

#ifndef _RTTI_HXX
#include <tools/rtti.hxx>
#endif

#ifndef _DATETIME_HXX
#include <tools/datetime.hxx>
#endif

#include <svtools/poolitem.hxx>

class SvStream;

DBG_NAMEEX(SfxDateTimeRangeItem)

// class SfxDateTimeRangeItem -------------------------------------------------

class SfxDateTimeRangeItem : public SfxPoolItem
{
private:
    DateTime                aStartDateTime;
    DateTime                aEndDateTime;

public:
            TYPEINFO();

            SfxDateTimeRangeItem( const SfxDateTimeRangeItem& rCpy );
            SfxDateTimeRangeItem( USHORT nWhich );
            SfxDateTimeRangeItem( USHORT nWhich, const DateTime& rStartDT,
                                  const DateTime& rEndDT );

            ~SfxDateTimeRangeItem()
                { DBG_DTOR(SfxDateTimeRangeItem, 0); }

    virtual int             operator==( const SfxPoolItem& )            const;
    using SfxPoolItem::Compare;
    virtual int             Compare( const SfxPoolItem &rWith )         const;
    virtual SfxPoolItem*    Create( SvStream&, USHORT nItemVersion )    const;
    virtual SvStream&       Store( SvStream&, USHORT nItemVersion )     const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 )             const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    XubString &rText,
                                    const IntlWrapper * pIntlWrapper = 0 )
        const;

    virtual BOOL            PutValue  ( const com::sun::star::uno::Any& rVal,
                                         BYTE nMemberId = 0 );
    virtual BOOL            QueryValue( com::sun::star::uno::Any& rVal,
                                         BYTE nMemberId = 0 ) const;

    const DateTime&         GetStartDateTime()      const { return aStartDateTime; }
    const DateTime&         GetEndDateTime()        const { return aEndDateTime; }

    void                    SetStartDateTime( const DateTime& rDT )
                            { DBG_ASSERT( GetRefCount() == 0, "SetDateTime() with pooled item" );
                              aStartDateTime = rDT; }

    void                    SetEndDateTime( const DateTime& rDT )
                            { DBG_ASSERT( GetRefCount() == 0, "SetDateTime() with pooled item" );
                              aEndDateTime = rDT; }
};

#endif

