/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: charhiddenitem.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 05:18:20 $
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

#define ITEMID_CHARHIDDEN            0

#ifndef _SVX_CHARHIDDENITEM_HXX
#include <charhiddenitem.hxx>
#endif
#include "svxitems.hrc"
#include "dialmgr.hxx"

TYPEINIT1_AUTOFACTORY(SvxCharHiddenItem, SfxBoolItem);

/*-- 16.12.2003 15:24:25---------------------------------------------------

  -----------------------------------------------------------------------*/
SvxCharHiddenItem::SvxCharHiddenItem( const sal_Bool bHidden, const USHORT nId ) :
    SfxBoolItem( nId, bHidden )
{
}
/*-- 16.12.2003 15:24:25---------------------------------------------------

  -----------------------------------------------------------------------*/
SfxPoolItem* SvxCharHiddenItem::Clone( SfxItemPool * ) const
{
    return new SvxCharHiddenItem( *this );
}
/*-- 16.12.2003 15:24:25---------------------------------------------------

  -----------------------------------------------------------------------*/
SfxItemPresentation SvxCharHiddenItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper */*pIntl*/
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            USHORT nId = RID_SVXITEMS_CHARHIDDEN_FALSE;

            if ( GetValue() )
                nId = RID_SVXITEMS_CHARHIDDEN_TRUE;
            rText = SVX_RESSTR(nId);
            return ePres;
        }
        default: ; //prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

