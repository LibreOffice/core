/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sorgitm.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:08:20 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#pragma hdrstop

#include "sfxsids.hrc"
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


sal_Bool SfxScriptOrganizerItem::QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
{
    String aValue;
    BOOL bIsString = FALSE;
    sal_Bool bValue = sal_False;
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case 0:
        case MID_SCRIPT_ORGANIZER_LANGUAGE:
            bIsString = TRUE;
            aValue = aLanguage;
            break;
        default:
            DBG_ERROR("Wrong MemberId!");
               return sal_False;
     }

    if ( bIsString )
        rVal <<= ::rtl::OUString( aValue );
    else
        rVal <<= bValue;
    return sal_True;
}

sal_Bool SfxScriptOrganizerItem::PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId )
{
    ::rtl::OUString aValue;
    sal_Bool bRet = sal_False;
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
            DBG_ERROR("Wrong MemberId!");
            return sal_False;
    }

    return bRet;
}

