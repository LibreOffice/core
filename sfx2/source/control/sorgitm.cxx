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

#include <sfx2/sfxsids.hrc>
#include "sorgitm.hxx"
// STATIC DATA -----------------------------------------------------------

TYPEINIT1_AUTOFACTORY(SfxScriptOrganizerItem, SfxStringItem);

//------------------------------------------------------------------------

SfxScriptOrganizerItem::SfxScriptOrganizerItem() :

    SfxStringItem()

{
}

//------------------------------------------------------------------------

SfxScriptOrganizerItem::SfxScriptOrganizerItem( const String& rLanguage ) :

     SfxStringItem( SID_SCRIPTORGANIZER, rLanguage ),

     aLanguage( rLanguage )

{
}

//------------------------------------------------------------------------

SfxScriptOrganizerItem::SfxScriptOrganizerItem( const SfxScriptOrganizerItem& rItem ) :

    SfxStringItem( rItem ),

    aLanguage( rItem.aLanguage )

{
}

//------------------------------------------------------------------------

SfxScriptOrganizerItem::~SfxScriptOrganizerItem()
{
}

//------------------------------------------------------------------------

SfxPoolItem* SfxScriptOrganizerItem::Clone( SfxItemPool * ) const
{
    return new SfxScriptOrganizerItem( *this );
}

//------------------------------------------------------------------------

int SfxScriptOrganizerItem::operator==( const SfxPoolItem& rItem) const
{
     return rItem.Type() == Type() &&
         SfxStringItem::operator==(rItem) &&
         aLanguage == ((const SfxScriptOrganizerItem &)rItem).aLanguage;
}


bool SfxScriptOrganizerItem::QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    String aValue;
    sal_Bool bIsString = sal_False;
    sal_Bool bValue = sal_False;
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case 0:
        case MID_SCRIPT_ORGANIZER_LANGUAGE:
            bIsString = sal_True;
            aValue = aLanguage;
            break;
        default:
            OSL_FAIL("Wrong MemberId!");
               return false;
     }

    if ( bIsString )
        rVal <<= ::rtl::OUString( aValue );
    else
        rVal <<= bValue;
    return true;
}

bool SfxScriptOrganizerItem::PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId )
{
    ::rtl::OUString aValue;
    sal_Bool bRet = false;
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case 0:
        case MID_SCRIPT_ORGANIZER_LANGUAGE:
            bRet = (rVal >>= aValue);
            if ( bRet )
                aLanguage = aValue;
            break;
        default:
            OSL_FAIL("Wrong MemberId!");
            return false;
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
