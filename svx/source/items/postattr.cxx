/*************************************************************************
 *
 *  $RCSfile: postattr.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:21 $
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

// include ---------------------------------------------------------------

#pragma hdrstop

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

SvxPostItAuthorItem::SvxPostItAuthorItem( sal_uInt16 nWhich )
{
    SetWhich( nWhich );
}

// -----------------------------------------------------------------------

SvxPostItAuthorItem::SvxPostItAuthorItem( const XubString& rAuthor,
                                          sal_uInt16 nWhich ) :
    SfxStringItem( nWhich, rAuthor )
{
}

//------------------------------------------------------------------------

SfxItemPresentation SvxPostItAuthorItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
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
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

SfxPoolItem* __EXPORT SvxPostItAuthorItem::Clone( SfxItemPool * ) const
{
    return new SvxPostItAuthorItem( *this );
}

// class SvxPostItDateItem -----------------------------------------------

SvxPostItDateItem::SvxPostItDateItem( sal_uInt16 nWhich )
{
    SetWhich( nWhich );
}

// -----------------------------------------------------------------------

SvxPostItDateItem::SvxPostItDateItem( const XubString& rDate, sal_uInt16 nWhich ) :

    SfxStringItem( nWhich, rDate )
{
}

//------------------------------------------------------------------------

SfxItemPresentation SvxPostItDateItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
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
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

SfxPoolItem* __EXPORT SvxPostItDateItem::Clone( SfxItemPool * ) const
{
    return new SvxPostItDateItem( *this );
}

// class SvxPostItTextItem -----------------------------------------------

SvxPostItTextItem::SvxPostItTextItem( sal_uInt16 nWhich )
{
    SetWhich( nWhich );
}

// -----------------------------------------------------------------------

SvxPostItTextItem::SvxPostItTextItem( const XubString& rText, sal_uInt16 nWhich ) :

    SfxStringItem( nWhich, rText )
{
}

//------------------------------------------------------------------------

SfxItemPresentation SvxPostItTextItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
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
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

SfxPoolItem* __EXPORT SvxPostItTextItem::Clone( SfxItemPool * ) const
{
    return new SvxPostItTextItem( *this );
}


