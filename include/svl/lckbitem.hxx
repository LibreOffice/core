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
#ifndef INCLUDED_SVL_LCKBITEM_HXX
#define INCLUDED_SVL_LCKBITEM_HXX

#include <svl/poolitem.hxx>
#include <svl/svldllapi.h>
#include <tools/stream.hxx>

class SVL_DLLPUBLIC SfxLockBytesItem : public SfxPoolItem
{
    SvLockBytesRef          _xVal;

public:
                            static SfxPoolItem* CreateDefault();
                            SfxLockBytesItem();
                            SfxLockBytesItem( sal_uInt16 nWhich, SvStream & );
                            virtual ~SfxLockBytesItem() override;

    SfxLockBytesItem(SfxLockBytesItem const &) = default;
    SfxLockBytesItem(SfxLockBytesItem &&) = default;
    SfxLockBytesItem & operator =(SfxLockBytesItem const &) = default;
    SfxLockBytesItem & operator =(SfxLockBytesItem &&) = default;

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual SfxPoolItem*    Create(SvStream &, sal_uInt16 nItemVersion) const override;
    virtual SvStream&       Store(SvStream &, sal_uInt16 nItemVersion ) const override;

    SvLockBytes*            GetValue() const { return _xVal.get(); }

    virtual bool            PutValue  ( const css::uno::Any& rVal,
                                        sal_uInt8 nMemberId ) override;
    virtual bool            QueryValue( css::uno::Any& rVal,
                                        sal_uInt8 nMemberId = 0 ) const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
