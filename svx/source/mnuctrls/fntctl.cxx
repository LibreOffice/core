/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <string>
#include <svtools/stdmenu.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/dispatch.hxx>

#include <svx/fntctl.hxx>
#include <svx/svxids.hrc>
#include "editeng/flstitem.hxx"
#include "editeng/fontitem.hxx"

SFX_IMPL_MENU_CONTROL(SvxFontMenuControl, SvxFontItem);



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
            static_cast<const SvxFontListItem*>(pDoc->GetItem( SID_ATTR_CHAR_FONTLIST ));
        const FontList* pList = pFonts ? pFonts->GetFontList(): 0;
        DBG_ASSERT( pList, "Kein Fonts gefunden" );
        pMenu->Fill( pList );
    }
}



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
    rParent.EnableItem( GetId(), SfxItemState::DISABLED != eState );

    if ( SfxItemState::DEFAULT == eState )
    {
        if ( !pMenu->GetItemCount() )
            FillMenu();
        const SvxFontItem* pFontItem = dynamic_cast<const SvxFontItem*>( pState  );
        OUString aFont;

        if ( pFontItem )
            aFont = pFontItem->GetFamilyName();
        pMenu->SetCurName( aFont );
    }
}



/*  [Beschreibung]

    Statusbenachrichtigung "uber Bindings;  bei DOCCHANGED
    wird das Men"u mit den aktuellen Fonts aus der Fontlist
    der DocumentShell gef"ullt.
*/

void SvxFontMenuControl::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    const SfxSimpleHint* pSimpleHint = dynamic_cast<const SfxSimpleHint*>( &rHint );
    if ( pSimpleHint &&
         pSimpleHint->GetId() == SFX_HINT_DOCCHANGED )
        FillMenu();
}



/*  [Beschreibung]

    Select-Handler des Men"us; der Name des selektierten Fonts
    wird in einem SvxFontItem verschickt. Das F"ullen mit den
    weiteren Fontinformationen mu\s durch die Applikation geschehen.
*/

IMPL_LINK_TYPED( SvxFontMenuControl, MenuSelect, FontNameMenu *, pMen, void )
{
    SvxFontItem aItem( GetId() );
    aItem.SetFamilyName(pMen->GetCurName());
    GetBindings().GetDispatcher()->Execute( GetId(), SfxCallMode::RECORD, &aItem, 0L );
}

/*  [Beschreibung]

    Dtor; gibt das Men"u frei.
*/

SvxFontMenuControl::~SvxFontMenuControl()
{
    delete pMenu;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
