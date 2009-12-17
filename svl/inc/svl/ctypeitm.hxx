/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ctypeitm.hxx,v $
 * $Revision: 1.3.136.1 $
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
#ifndef _SVTOOLS_CTYPEITM_HXX
#define _SVTOOLS_CTYPEITM_HXX

#include <svl/inettype.hxx>
#include <svl/custritm.hxx>

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

    INetContentType GetEnumValue() const;

    void                  SetValue( const INetContentType eType );
};

#endif /* !_SVTOOLS_CTYPEITM_HXX */

