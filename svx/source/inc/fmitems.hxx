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
#ifndef _SVX_FMITEMS_HXX
#define _SVX_FMITEMS_HXX


#include <svl/poolitem.hxx>
#include <comphelper/uno3.hxx>
//  FORWARD_DECLARE_INTERFACE(awt,XControlContainer)
FORWARD_DECLARE_INTERFACE(uno,Any)
//  class ::com::sun::star::uno::Any;

//==================================================================
class FmInterfaceItem : public SfxPoolItem
{
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  xInterface;

public:
    TYPEINFO();

    FmInterfaceItem( const sal_uInt16 nId, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rxInterface )
        :SfxPoolItem( nId )
        ,xInterface( rxInterface )
    {}

    inline FmInterfaceItem&  operator=( const FmInterfaceItem &rCpy );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create(SvStream &, sal_uInt16) const;
    virtual SvStream&       Store(SvStream &, sal_uInt16 nItemVersion ) const;

    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >         GetInterface() const { return xInterface; }
};

inline FmInterfaceItem& FmInterfaceItem::operator=( const FmInterfaceItem &rCpy )
{
    xInterface = rCpy.xInterface;

    return *this;
}



#endif // _SVX_FMITEMS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
