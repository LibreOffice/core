/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * [ Copyright (C) 2009 Tzvetelina Tzeneva <tzvetelina@gmail.com> (initial developer) ]
 */
#ifndef _SVX_RSIDITEM_HXX
#define _SVX_RSIDITEM_HXX

#include <svl/intitem.hxx>
#include "editeng/editengdllapi.h"

//----------------------
// SvxRsidItem
//----------------------

class EDITENG_DLLPUBLIC SvxRsidItem : public SfxUInt32Item
{
public:
    TYPEINFO();

    SvxRsidItem( sal_uInt32 nRsid, sal_uInt16 nId ) : SfxUInt32Item( nId, nRsid ) {}
    SvxRsidItem( SvStream& rIn, sal_uInt16 nId ) : SfxUInt32Item( nId, rIn ) {}

    virtual SfxPoolItem* Clone( SfxItemPool* pPool = NULL ) const;
    virtual SfxPoolItem* Create( SvStream& rIn, sal_uInt16 nVer ) const;

    virtual bool QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );
};

#endif // _SVX_RSIDITEM_HXX
