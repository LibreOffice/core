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
#ifndef INCLUDED_SVX_OFAITEM_HXX
#define INCLUDED_SVX_OFAITEM_HXX

#include <svl/poolitem.hxx>
#include <rtl/ref.hxx>
#include <svx/svxdllapi.h>
#include <svx/xtable.hxx>

class SVX_DLLPUBLIC OfaPtrItem final : public SfxPoolItem
{
private:
    void* pPtr;

public:
                             DECLARE_ITEM_TYPE_FUNCTION(OfaPtrItem)
                             OfaPtrItem( sal_uInt16 nWhich, void *pPtr );

    virtual bool             operator==( const SfxPoolItem& ) const override;
    virtual OfaPtrItem*      Clone( SfxItemPool *pPool = nullptr ) const override;

    void*                    GetValue() const { return pPtr; }
};

class SVX_DLLPUBLIC OfaXColorListItem final : public SfxPoolItem
{
 private:
    rtl::Reference<XColorList> mxRef;
public:
    DECLARE_ITEM_TYPE_FUNCTION(OfaXColorListItem)
    OfaXColorListItem( sal_uInt16 _nWhich, rtl::Reference<XColorList> xRef );

    virtual bool operator==( const SfxPoolItem& rItem ) const override;

    virtual OfaXColorListItem* Clone( SfxItemPool *pPool = nullptr  ) const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
