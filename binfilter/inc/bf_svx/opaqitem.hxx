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
#ifndef _SVX_OPAQITEM_HXX
#define _SVX_OPAQITEM_HXX

// include ---------------------------------------------------------------

#ifndef _SFXENUMITEM_HXX //autogen
#include <bf_svtools/eitem.hxx>
#endif

namespace rtl
{
    class OUString;
}
namespace binfilter {
class SvXMLUnitConverter;
// class SvxOpaqueItem ---------------------------------------------------

/*
[Beschreibung]
Dieses Item beschreibt eine logische Variable "Undurchsichtig ja oder nein".
*/

class SvxOpaqueItem : public SfxBoolItem
{
public:
    TYPEINFO();

    SvxOpaqueItem( const USHORT nId = ITEMID_OPAQUE, const BOOL bOpa = TRUE );
    inline SvxOpaqueItem &operator=( const SvxOpaqueItem &rCpy );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create(SvStream &, USHORT) const;
    virtual SvStream&		Store(SvStream &, USHORT nItemVersion ) const;

};

inline SvxOpaqueItem::SvxOpaqueItem( const USHORT nId, const BOOL bOpa )
    : SfxBoolItem( nId, bOpa )
{}

inline SvxOpaqueItem &SvxOpaqueItem::operator=( const SvxOpaqueItem &rCpy )
{
    SetValue( rCpy.GetValue() );
    return *this;
}

}//end of namespace binfilter
#endif

