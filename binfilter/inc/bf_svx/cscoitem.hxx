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
#ifndef _SVX_CSCOITEM_HXX
#define _SVX_CSCOITEM_HXX

// include ---------------------------------------------------------------

#include <tools/string.hxx>
#include <bf_svx/colritem.hxx>
namespace binfilter {

// class SvxCharSetColorItem ---------------------------------------------

/*	[Beschreibung]

    PB: wird nur intern im Reader des Writers benoetigt
*/

class SvxCharSetColorItem : public SvxColorItem
{
    rtl_TextEncoding eFrom;
public:
    TYPEINFO();

    SvxCharSetColorItem( const USHORT nId = ITEMID_CHARSETCOLOR );
    SvxCharSetColorItem( const Color& aColor, const rtl_TextEncoding eFrom,
                     const USHORT nId = ITEMID_CHARSETCOLOR );

    // "pure virtual Methoden" vom SfxPoolItem

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*     Create(SvStream &, USHORT) const;
    virtual SvStream&		 Store(SvStream &, USHORT nItemVersion) const;

    inline rtl_TextEncoding&	GetCharSet() { return eFrom; }
    inline rtl_TextEncoding 	GetCharSet() const { return eFrom; }

    inline SvxCharSetColorItem& operator=(const SvxCharSetColorItem& rColor)
    {
        SetValue( rColor.GetValue() );
        eFrom = rColor.GetCharSet();
        return *this;
    }
};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
