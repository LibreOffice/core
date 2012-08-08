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
#ifndef _SVX_PROTITEM_HXX
#define _SVX_PROTITEM_HXX

#include <svl/poolitem.hxx>
#include "editeng/editengdllapi.h"

class SvXMLUnitConverter;
namespace rtl
{
    class OUString;
}

// class SvxProtectItem --------------------------------------------------


/*  [Description]

    This item describes, if content, size or position should be protected.
*/

class EDITENG_DLLPUBLIC SvxProtectItem : public SfxPoolItem
{
    sal_Bool bCntnt :1;     // Content protected
    sal_Bool bSize  :1;     // Size protected
    sal_Bool bPos   :1;     // Position protected

public:
    TYPEINFO();

    inline SvxProtectItem( const sal_uInt16 nId  );
    inline SvxProtectItem &operator=( const SvxProtectItem &rCpy );

    // "pure virtual Methods" from SfxPoolItem
    virtual int              operator==( const SfxPoolItem& ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;


    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*     Create(SvStream &, sal_uInt16) const;
    virtual SvStream&        Store(SvStream &, sal_uInt16 nItemVersion ) const;

    sal_Bool IsCntntProtected() const { return bCntnt; }
    sal_Bool IsSizeProtected()  const { return bSize;  }
    sal_Bool IsPosProtected()   const { return bPos;   }
    void SetCntntProtect( sal_Bool bNew ) { bCntnt = bNew; }
    void SetSizeProtect ( sal_Bool bNew ) { bSize  = bNew; }
    void SetPosProtect  ( sal_Bool bNew ) { bPos   = bNew; }

    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );
};

inline SvxProtectItem::SvxProtectItem( const sal_uInt16 nId )
    : SfxPoolItem( nId )
{
    bCntnt = bSize = bPos = sal_False;
}

inline SvxProtectItem &SvxProtectItem::operator=( const SvxProtectItem &rCpy )
{
    bCntnt = rCpy.IsCntntProtected();
    bSize  = rCpy.IsSizeProtected();
    bPos   = rCpy.IsPosProtected();
    return *this;
}




#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
