/*************************************************************************
 *
 *  $RCSfile: tplpitem.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: svesik $ $Date: 2004-04-21 13:14:42 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// INCLUDE ---------------------------------------------------------------

#ifndef GCC
#pragma hdrstop
#endif

#include "tplpitem.hxx"

#ifndef _DRAFTS_COM_SUN_STAR_FRAME_STATUS_TEMPLATE_HPP_
#include <drafts/com/sun/star/frame/status/Template.hpp>
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
    drafts::com::sun::star::frame::status::Template aTemplate;

    aTemplate.Value = GetValue();
    aTemplate.StyleName = aStyle;
    rVal <<= aTemplate;

    return sal_True;
}

//-------------------------------------------------------------------------
sal_Bool SfxTemplateItem::PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId )
{
    drafts::com::sun::star::frame::status::Template aTemplate;

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


