/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cstitem.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:15:07 $
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

#ifndef _CSTITEM_HXX
#define _CSTITEM_HXX

#ifndef _RTTI_HXX
#include <tools/rtti.hxx>
#endif

#include <svtools/poolitem.hxx>

class SvStream;

enum CrawlStatus
{
    CSTAT_NEVER_UPD         = 0,        /* noch nie ueberprueft                         */
    CSTAT_IN_UPD            = 1,        /* Ueberpruefung laeuft                         */
    CSTAT_UPD_NEWER         = 2,        /* wurde ueberprueft und ist neuer              */
    CSTAT_UPD_NOT_NEWER     = 3,        /* wurde ueberprueft und ist nicht neuer        */
    CSTAT_UPD_CANCEL        = 4,        /* Ueberpruefung vom Benutzer abgebrochen       */
    CSTAT_ERR_GENERAL       = 5,        /* allgemeiner Fehler                           */
    CSTAT_ERR_NOTEXISTS     = 6,        /* Server existiert nicht                       */
    CSTAT_ERR_NOTREACHED    = 7,        /* Server nicht ereicht                         */
    CSTAT_UPD_IMMEDIATELY   = 8,        /* es wird gleich ueberprueftt                  */
    CSTAT_ERR_OFFLINE       = 9         /* Ueberpruefung nicht m�glich, da Offline        */
};

DBG_NAMEEX(SfxCrawlStatusItem)

// class SfxDateTimeRangeItem -------------------------------------------------

class SfxCrawlStatusItem : public SfxPoolItem
{
private:
            CrawlStatus     eStatus;
public:
            TYPEINFO();

            SfxCrawlStatusItem( const SfxCrawlStatusItem& rCpy );
            SfxCrawlStatusItem( USHORT nWhich );
            SfxCrawlStatusItem( USHORT nWhich, CrawlStatus eStat );
            ~SfxCrawlStatusItem() { DBG_DTOR(SfxCrawlStatusItem, 0); }

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

    CrawlStatus             GetStatus() const { return eStatus; }
    void                    SetStatus(CrawlStatus eNew) { eStatus = eNew; }

    virtual BOOL PutValue  ( const com::sun::star::uno::Any& rVal,
                             BYTE nMemberId = 0 );
    virtual BOOL QueryValue( com::sun::star::uno::Any& rVal,
                             BYTE nMemberId = 0 ) const;
};

#endif

