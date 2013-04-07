/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef _SVX_PROTITEM_HXX
#define _SVX_PROTITEM_HXX

#include <svl/poolitem.hxx>
#include "editeng/editengdllapi.h"

class SvXMLUnitConverter;

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

    explicit inline SvxProtectItem( const sal_uInt16 nId  );
    inline SvxProtectItem &operator=( const SvxProtectItem &rCpy );

    // "pure virtual Methods" from SfxPoolItem
    virtual int              operator==( const SfxPoolItem& ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = 0 ) const;


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
