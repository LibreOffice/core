/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: charhiddenitem.cxx,v $
 * $Revision: 1.9 $
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
#include "precompiled_svx.hxx"


#include <charhiddenitem.hxx>
#include <svx/svxitems.hrc>
#include <svx/dialmgr.hxx>

TYPEINIT1_FACTORY(SvxCharHiddenItem, SfxBoolItem, new SvxCharHiddenItem(sal_False, 0));

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
    XubString&          rText, const IntlWrapper * /*pIntl*/
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

