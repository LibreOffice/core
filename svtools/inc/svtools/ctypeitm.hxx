/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ctypeitm.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:15:27 $
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
#ifndef _SVTOOLS_CTYPEITM_HXX
#define _SVTOOLS_CTYPEITM_HXX

#ifndef _INETTYPE_HXX
#include <svtools/inettype.hxx>
#endif
#ifndef _SVTOOLS_CUSTRITM_HXX
#include <svtools/custritm.hxx>
#endif

//=========================================================================

class CntContentTypeItem : public CntUnencodedStringItem
{
private:
    INetContentType _eType;
    XubString       _aPresentation;

public:
    TYPEINFO();

    CntContentTypeItem();
    CntContentTypeItem( USHORT nWhich, const XubString& rType );
    CntContentTypeItem( USHORT nWhich, const INetContentType eType );
    CntContentTypeItem( const CntContentTypeItem& rOrig );

    virtual SfxPoolItem* Create( SvStream& rStream,
                                 USHORT nItemVersion ) const;
    virtual SvStream & Store(SvStream & rStream, USHORT) const;

    virtual int          operator==( const SfxPoolItem& rOrig ) const;

    virtual USHORT GetVersion(USHORT) const;

    virtual SfxPoolItem* Clone( SfxItemPool *pPool = NULL ) const;

    void SetValue( const XubString& rNewVal );
    void SetPresentation( const XubString& rNewVal );

    using SfxPoolItem::Compare;
    virtual int Compare( const SfxPoolItem &rWith, const IntlWrapper& rIntlWrapper ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                                 SfxMapUnit eCoreMetric,
                                                 SfxMapUnit ePresMetric,
                                                 XubString &rText,
                                                 const IntlWrapper* pIntlWrapper = 0 ) const;

    virtual BOOL QueryValue( com::sun::star::uno::Any& rVal,
                             BYTE nMemberId = 0 ) const;
    virtual BOOL PutValue  ( const com::sun::star::uno::Any& rVal,
                             BYTE nMemberId = 0);

    const INetContentType GetEnumValue() const;

    void                  SetValue( const INetContentType eType );
};

#endif /* !_SVTOOLS_CTYPEITM_HXX */

