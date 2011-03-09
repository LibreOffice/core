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
#include "precompiled_svx.hxx"

#include <string> // HACK: prevent conflict between STLPORT and Workshop headern
#include <svtools/stdmenu.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/dispatch.hxx>

#include <svx/fntctl.hxx>       //
#include <svx/svxids.hrc>
#include "editeng/flstitem.hxx"
#include "editeng/fontitem.hxx"

// STATIC DATA -----------------------------------------------------------

SFX_IMPL_MENU_CONTROL(SvxFontMenuControl, SvxFontItem);

//--------------------------------------------------------------------

/*  [Beschreibung]

    Ctor; setzt den Select-Handler am Men"u und tr"agt das Men"u
    in seinen Parent ein.
*/

SvxFontMenuControl::SvxFontMenuControl
(
    sal_uInt16          _nId,
    Menu&           rMenu,
    SfxBindings&    rBindings
) :
    pMenu   ( new FontNameMenu ),
    rParent ( rMenu )
{
    rMenu.SetPopupMenu( _nId, pMenu );
    pMenu->SetSelectHdl( LINK( this, SvxFontMenuControl, MenuSelect ) );
    StartListening( rBindings );
    FillMenu();
}

//--------------------------------------------------------------------

/*  [Beschreibung]

    F"ullt das Men"u mit den aktuellen Fonts aus der Fontlist
    der DocumentShell.
*/

void SvxFontMenuControl::FillMenu()
{
    SfxObjectShell *pDoc = SfxObjectShell::Current();

    if ( pDoc )
    {
        const SvxFontListItem* pFonts =
            (const SvxFontListItem*)pDoc->GetItem( SID_ATTR_CHAR_FONTLIST );
        const FontList* pList = pFonts ? pFonts->GetFontList(): 0;
        DBG_ASSERT( pList, "Kein Fonts gefunden" );
        pMenu->Fill( pList );
    }
}

//--------------------------------------------------------------------

/*  [Beschreibung]

    Statusbenachrichtigung;
    f"ullt ggf. das Men"u mit den aktuellen Fonts aus der Fontlist
    der DocumentShell.
    Ist die Funktionalit"at disabled, wird der entsprechende
    Men"ueintrag im Parentmen"u disabled, andernfalls wird er enabled.
    Der aktuelle Font wird mit einer Checkmark versehen.
*/

void SvxFontMenuControl::StateChanged(

    sal_uInt16, SfxItemState eState, const SfxPoolItem* pState )

{
    rParent.EnableItem( GetId(), SFX_ITEM_DISABLED != eState );

    if ( SFX_ITEM_AVAILABLE == eState )
    {
        if ( !pMenu->GetItemCount() )
            FillMenu();
        const SvxFontItem* pFontItem = PTR_CAST( SvxFontItem, pState );
        String aFont;

        if ( pFontItem )
            aFont = pFontItem->GetFamilyName();
        pMenu->SetCurName( aFont );
    }
}

//--------------------------------------------------------------------

/*  [Beschreibung]

    Statusbenachrichtigung "uber Bindings;  bei DOCCHANGED
    wird das Men"u mit den aktuellen Fonts aus der Fontlist
    der DocumentShell gef"ullt.
*/

void SvxFontMenuControl::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( rHint.Type() != TYPE(SfxSimpleHint) &&
         ( (SfxSimpleHint&)rHint ).GetId() == SFX_HINT_DOCCHANGED )
        FillMenu();
}

//--------------------------------------------------------------------

/*  [Beschreibung]

    Select-Handler des Men"us; der Name des selektierten Fonts
    wird in einem SvxFontItem verschickt. Das F"ullen mit den
    weiteren Fontinformationen mu\s durch die Applikation geschehen.
*/

IMPL_LINK_INLINE_START( SvxFontMenuControl, MenuSelect, FontNameMenu *, pMen )
{
    SvxFontItem aItem( GetId() );
    aItem.SetFamilyName(pMen->GetCurName());
    GetBindings().GetDispatcher()->Execute( GetId(), SFX_CALLMODE_RECORD, &aItem, 0L );
    return 0;
}
IMPL_LINK_INLINE_END( SvxFontMenuControl, MenuSelect, FontNameMenu *, pMen )

//--------------------------------------------------------------------

/*  [Beschreibung]

    Dtor; gibt das Men"u frei.
*/

SvxFontMenuControl::~SvxFontMenuControl()
{
    delete pMenu;
}

//--------------------------------------------------------------------

/*  [Beschreibung]

    Gibt das Men"u zur"uck
*/

PopupMenu* SvxFontMenuControl::GetPopup() const
{
    return pMenu;
}



