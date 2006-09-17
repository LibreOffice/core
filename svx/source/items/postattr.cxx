/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: postattr.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 05:22:38 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------

#define _SVX_POSTATTR_CXX
#define ITEMID_AUTHOR   0
#define ITEMID_DATE     0
#define ITEMID_TEXT     0

#include "postattr.hxx"
#include "itemtype.hxx"

// -----------------------------------------------------------------------

TYPEINIT1_AUTOFACTORY(SvxPostItAuthorItem, SfxStringItem);
TYPEINIT1_AUTOFACTORY(SvxPostItDateItem, SfxStringItem);
TYPEINIT1_AUTOFACTORY(SvxPostItTextItem, SfxStringItem);

// class SvxPostItAuthorItem ---------------------------------------------

SvxPostItAuthorItem::SvxPostItAuthorItem( sal_uInt16 _nWhich )
{
    SetWhich( _nWhich );
}

// -----------------------------------------------------------------------

SvxPostItAuthorItem::SvxPostItAuthorItem( const XubString& rAuthor,
                                          sal_uInt16 _nWhich ) :
    SfxStringItem( _nWhich, rAuthor )
{
}

//------------------------------------------------------------------------

SfxItemPresentation SvxPostItAuthorItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return SFX_ITEM_PRESENTATION_NONE;
        case SFX_ITEM_PRESENTATION_NAMELESS:
            rText = GetValue();
            return SFX_ITEM_PRESENTATION_NAMELESS;
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = SVX_RESSTR(RID_SVXITEMS_AUTHOR_COMPLETE);
            rText += GetValue();
            return SFX_ITEM_PRESENTATION_COMPLETE;
        default: ;//prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

SfxPoolItem* __EXPORT SvxPostItAuthorItem::Clone( SfxItemPool * ) const
{
    return new SvxPostItAuthorItem( *this );
}

// class SvxPostItDateItem -----------------------------------------------

SvxPostItDateItem::SvxPostItDateItem( sal_uInt16 _nWhich )
{
    SetWhich( _nWhich );
}

// -----------------------------------------------------------------------

SvxPostItDateItem::SvxPostItDateItem( const XubString& rDate, sal_uInt16 _nWhich ) :

    SfxStringItem( _nWhich, rDate )
{
}

//------------------------------------------------------------------------

SfxItemPresentation SvxPostItDateItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return SFX_ITEM_PRESENTATION_NONE;
        case SFX_ITEM_PRESENTATION_NAMELESS:
            rText = GetValue();
            return SFX_ITEM_PRESENTATION_NAMELESS;
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = SVX_RESSTR(RID_SVXITEMS_DATE_COMPLETE);
            rText += GetValue();
            return SFX_ITEM_PRESENTATION_COMPLETE;
        default: ;//prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

SfxPoolItem* __EXPORT SvxPostItDateItem::Clone( SfxItemPool * ) const
{
    return new SvxPostItDateItem( *this );
}

// class SvxPostItTextItem -----------------------------------------------

SvxPostItTextItem::SvxPostItTextItem( sal_uInt16 _nWhich )
{
    SetWhich( _nWhich );
}

// -----------------------------------------------------------------------

SvxPostItTextItem::SvxPostItTextItem( const XubString& rText, sal_uInt16 _nWhich ) :

    SfxStringItem( _nWhich, rText )
{
}

//------------------------------------------------------------------------

SfxItemPresentation SvxPostItTextItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return SFX_ITEM_PRESENTATION_NONE;
        case SFX_ITEM_PRESENTATION_NAMELESS:
            rText = GetValue();
            return SFX_ITEM_PRESENTATION_NAMELESS;
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = SVX_RESSTR(RID_SVXITEMS_TEXT_COMPLETE);
            rText += GetValue();
            return SFX_ITEM_PRESENTATION_COMPLETE;
        default: ;//prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

SfxPoolItem* __EXPORT SvxPostItTextItem::Clone( SfxItemPool * ) const
{
    return new SvxPostItTextItem( *this );
}


