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
#ifndef _CNTWALL_HXX
#define _CNTWALL_HXX

#include <tools/shl.hxx>
#include <tools/rtti.hxx>
#include <tools/color.hxx>

#include <bf_svtools/poolitem.hxx>

class SvStream;

namespace binfilter {

class  CntWallpaperItem : public SfxPoolItem
{
private:
    UniString				_aURL;
    Color					_nColor;
    USHORT					_nStyle;

public:
                            TYPEINFO();

                            CntWallpaperItem( USHORT nWhich, SvStream& rStream, USHORT nVersion );
                            CntWallpaperItem( const CntWallpaperItem& rCpy );
                            ~CntWallpaperItem();

    virtual USHORT GetVersion(USHORT) const;

    virtual	int				operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*	Create( SvStream&, USHORT nItemVersion ) const;
    virtual SvStream&		Store( SvStream&, USHORT nItemVersion ) const;
    virtual SfxPoolItem*	Clone( SfxItemPool* pPool = 0 ) const;

    virtual	bool 			QueryValue( ::com::sun::star::uno::Any& rVal,
                                         BYTE nMemberId = 0 ) const;
    virtual	bool 			PutValue  ( const ::com::sun::star::uno::Any& rVal,
                                         BYTE nMemberId = 0 );

    void					SetBitmapURL( const UniString& rURL ) { _aURL = rURL; }
    void					SetColor( Color nColor ) { _nColor = nColor; }
    void					SetStyle( USHORT nStyle ) { _nStyle = nStyle; }

    const UniString&		GetBitmapURL() const { return _aURL; }
    Color					GetColor() const { return _nColor; }
    USHORT					GetStyle() const { return _nStyle; }
};

}

////////////////////////////////////////////////////////////////////////////////

#endif // _CNTWALL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
