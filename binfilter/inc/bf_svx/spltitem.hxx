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
#ifndef _SVX_SPLTITEM_HXX
#define _SVX_SPLTITEM_HXX

// include ---------------------------------------------------------------

#include <bf_svtools/eitem.hxx>
#include <bf_svx/svxids.hrc>

namespace rtl
{
    class OUString;
}
namespace binfilter {
class SvXMLUnitConverter;

// class SvxFmtSplitItem -------------------------------------------------

/*
[Beschreibung]
Dieses Item beschreibt eine logische Variable "Absatz trennen ja oder nein".
*/

class SvxFmtSplitItem : public SfxBoolItem
{
public:
    TYPEINFO();
    ~SvxFmtSplitItem();

    inline SvxFmtSplitItem( const BOOL bSplit = TRUE,
                            const USHORT nWhich = ITEMID_FMTSPLIT );
    inline SvxFmtSplitItem& operator=( const SvxFmtSplitItem& rSplit );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual SfxPoolItem*	Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*	Create( SvStream&, USHORT ) const;
    virtual SvStream&		Store( SvStream& , USHORT nItemVersion ) const;

};

inline SvxFmtSplitItem::SvxFmtSplitItem( const BOOL bSplit, const USHORT nWhich ) :
    SfxBoolItem( nWhich, bSplit )
{}

inline SvxFmtSplitItem& SvxFmtSplitItem::operator=(
    const SvxFmtSplitItem& rSplit )
{
    SetValue( rSplit.GetValue() );
    return *this;
}

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
