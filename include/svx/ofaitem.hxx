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
#ifndef _OFF_OFAITEM_HXX
#define _OFF_OFAITEM_HXX

#include <svl/poolitem.hxx>
#include <rtl/ref.hxx>
#include "svx/svxdllapi.h"

// class OfaPtrItem ------------------------------------------------------

class SVX_DLLPUBLIC OfaPtrItem : public SfxPoolItem
{
private:
    void* pPtr;

public:
                             OfaPtrItem( sal_uInt16 nWhich, void *pPtr );
                             OfaPtrItem( const OfaPtrItem& );

    virtual int              operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;

    void*                    GetValue() const { return pPtr; }
    void                     SetValue( void* pNewPtr ) { pPtr = pNewPtr; }
};

// class OfaRefItem - for ref counting items

template <class reference_type>
class OfaRefItem : public SfxPoolItem
{
 private:
    rtl::Reference<reference_type> mxRef;
public:
    OfaRefItem( sal_uInt16 _nWhich, const rtl::Reference<reference_type> &xRef )
        : SfxPoolItem( _nWhich ), mxRef( xRef )
    {}
    OfaRefItem( const OfaRefItem& rItem )
        : SfxPoolItem( rItem.Which() ), mxRef( rItem.mxRef )
    {}
    virtual int operator==( const SfxPoolItem& rItem ) const
    {
        return mxRef == ((OfaRefItem<reference_type> &)rItem).mxRef;
    }
    virtual SfxPoolItem*Clone( SfxItemPool* /*pPool = 0*/ ) const
    {
        return new OfaRefItem( *this );
    }
    inline rtl::Reference<reference_type> GetValue() const
    {
        return mxRef;
    }
    inline void SetValue( const rtl::Reference<reference_type> &xRef )
    {
        mxRef = xRef;
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
