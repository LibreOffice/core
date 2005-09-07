/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tplpitem.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:34:36 $
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

// INCLUDE ---------------------------------------------------------------

#ifndef GCC
#pragma hdrstop
#endif

#include "tplpitem.hxx"

#ifndef _COM_SUN_STAR_FRAME_STATUS_TEMPLATE_HPP_
#include <com/sun/star/frame/status/Template.hpp>
#endif


// STATIC DATA -----------------------------------------------------------

TYPEINIT1_AUTOFACTORY(SfxTemplateItem, SfxFlagItem);

//=========================================================================

SfxTemplateItem::SfxTemplateItem() :
    SfxFlagItem()
{
}

SfxTemplateItem::SfxTemplateItem
(
    USHORT nWhich,          // Slot-ID
    const String& rStyle,   // Name des aktuellen Styles
    USHORT nValue           // Flags f"ur das Filtern bei automatischer Anzeige
) :

    SfxFlagItem( nWhich, nValue ),

    aStyle( rStyle )

{
}

//-------------------------------------------------------------------------

// copy ctor
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
sal_Bool SfxTemplateItem::QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
{
    ::com::sun::star::frame::status::Template aTemplate;

    aTemplate.Value = GetValue();
    aTemplate.StyleName = aStyle;
    rVal <<= aTemplate;

    return sal_True;
}

//-------------------------------------------------------------------------
sal_Bool SfxTemplateItem::PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId )
{
    ::com::sun::star::frame::status::Template aTemplate;

    if ( rVal >>= aTemplate )
    {
        SetValue( aTemplate.Value );
        aStyle = aTemplate.StyleName;
        return sal_True;
    }

    return sal_False;
}

//-------------------------------------------------------------------------

BYTE SfxTemplateItem::GetFlagCount() const
{
    return sizeof(USHORT) * 8;
}


