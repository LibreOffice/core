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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

// INCLUDE ---------------------------------------------------------------

#include "sfx2/tplpitem.hxx"
#include <com/sun/star/frame/status/Template.hpp>


// STATIC DATA -----------------------------------------------------------

TYPEINIT1_AUTOFACTORY(SfxTemplateItem, SfxFlagItem);

//=========================================================================

SfxTemplateItem::SfxTemplateItem() :
    SfxFlagItem()
{
}

SfxTemplateItem::SfxTemplateItem
(
    sal_uInt16 nWhichId,      // Slot-ID
    const String& rStyle, // Name of the current Styles
    sal_uInt16 nValue         // Flags for the filters of the automatic display
) : SfxFlagItem( nWhichId, nValue ),
    aStyle( rStyle )
{
}

//-------------------------------------------------------------------------

// copy constuctor
SfxTemplateItem::SfxTemplateItem( const SfxTemplateItem& rCopy ) :

    SfxFlagItem( rCopy ),

    aStyle( rCopy.aStyle )
{
}

//-------------------------------------------------------------------------

// op ==

int SfxTemplateItem::operator==( const SfxPoolItem& rCmp ) const
{
    return ( SfxFlagItem::operator==( rCmp ) &&
             aStyle == ( (const SfxTemplateItem&)rCmp ).aStyle );
}

//-------------------------------------------------------------------------

SfxPoolItem* SfxTemplateItem::Clone( SfxItemPool *) const
{
    return new SfxTemplateItem(*this);
}

//-------------------------------------------------------------------------
bool SfxTemplateItem::QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    ::com::sun::star::frame::status::Template aTemplate;

    aTemplate.Value = GetValue();
    aTemplate.StyleName = aStyle;
    rVal <<= aTemplate;

    return true;
}

//-------------------------------------------------------------------------
bool SfxTemplateItem::PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    ::com::sun::star::frame::status::Template aTemplate;

    if ( rVal >>= aTemplate )
    {
        SetValue( sal::static_int_cast< sal_uInt16 >( aTemplate.Value ) );
        aStyle = aTemplate.StyleName;
        return true;
    }

    return false;
}

//-------------------------------------------------------------------------

sal_uInt8 SfxTemplateItem::GetFlagCount() const
{
    return sizeof(sal_uInt16) * 8;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
