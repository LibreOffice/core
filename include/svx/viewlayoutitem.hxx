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
#ifndef INCLUDED_SVX_VIEWLAYOUTITEM_HXX
#define INCLUDED_SVX_VIEWLAYOUTITEM_HXX

#include <svl/intitem.hxx>
#include <svx/svxids.hrc>
#include <svx/svxdllapi.h>



class SVX_DLLPUBLIC SvxViewLayoutItem: public SfxUInt16Item
{
    bool                    mbBookMode;

public:
    static SfxPoolItem* CreateDefault();

    SvxViewLayoutItem( sal_uInt16 nColumns = 0, bool bBookMode = false, sal_uInt16 nWhich = SID_ATTR_VIEWLAYOUT );
    SvxViewLayoutItem( const SvxViewLayoutItem& );
    virtual ~SvxViewLayoutItem();

    void                    SetBookMode( bool bNew ) {mbBookMode = bNew; }
    bool                    IsBookMode() const {return mbBookMode; }

    // "pure virtual Methoden" vom SfxPoolItem
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const override;
    virtual SfxPoolItem*    Create( SvStream& rStrm, sal_uInt16 nVersion ) const override;                       // leer
    virtual SvStream&       Store( SvStream& rStrm , sal_uInt16 nItemVersion ) const override;                   // leer
    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override; // leer
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;   // leer
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
