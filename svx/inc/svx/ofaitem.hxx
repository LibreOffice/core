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
