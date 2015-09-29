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
#include <svl/itempool.hxx>
#include <svtools/stdmenu.hxx>
#include <svtools/ctrltool.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>

#include <svx/fntszctl.hxx>
#include <svx/dialogs.hrc>
#include <editeng/fhgtitem.hxx>
#include "editeng/fontitem.hxx"
#include "editeng/flstitem.hxx"

#include "svx/dlgutil.hxx"
#include <svx/dialmgr.hxx>

#define LOGIC   OutputDevice::LogicToLogic

SFX_IMPL_MENU_CONTROL(SvxFontSizeMenuControl, SvxFontHeightItem);



/*  [Beschreibung]

    Select-Handler des Men"us; die aktuelle Fontgr"o\se
    wird in einem SvxFontHeightItem verschickt.
*/

IMPL_LINK_TYPED( SvxFontSizeMenuControl, MenuSelect, FontSizeMenu*, pMen, void )
{
    SfxViewFrame* pFrm = SfxViewFrame::Current();
    SfxShell* pSh = pFrm ? pFrm->GetDispatcher()->GetShell( 0 ) : NULL;

    if ( !pSh )
        return;

    const SfxItemPool& rPool = pSh->GetPool();
    sal_uInt16 nWhich = rPool.GetWhich( SID_ATTR_CHAR_FONTHEIGHT );
    const SfxMapUnit eUnit = rPool.GetMetric( nWhich );
    long nH = LOGIC( pMen->GetCurHeight(), MAP_POINT, (MapUnit)eUnit ) / 10;
    SvxFontHeightItem aItem( nH, 100, GetId() );
    GetBindings().GetDispatcher()->Execute( GetId(), SfxCallMode::RECORD, &aItem, 0L );
}



/*  [Beschreibung]

    Statusbenachrichtigung;
    Ist die Funktionalit"at disabled, wird der entsprechende
    Men"ueintrag im Parentmenu disabled, andernfalls wird er enabled.
    die aktuelle Fontgr"o\se wird mit einer Checkmark versehen.
*/

void SvxFontSizeMenuControl::StateChanged(

    sal_uInt16, SfxItemState eState, const SfxPoolItem* pState )

{
    rParent.EnableItem( GetId(), SfxItemState::DISABLED != eState );

    if ( SfxItemState::DEFAULT == eState )
    {
        if ( dynamic_cast<const SvxFontHeightItem*>( pState) !=  nullptr )
        {
            const SvxFontHeightItem* pItem =
                dynamic_cast<const SvxFontHeightItem*>( pState  );
            long nVal = 0;

            if ( pItem )
            {
                SfxViewFrame* pFrm = SfxViewFrame::Current();
                SfxShell* pSh = pFrm ? pFrm->GetDispatcher()->GetShell( 0 )
                                     : NULL;

                if ( !pSh )
                    return;

                const SfxItemPool& rPool = pSh->GetPool();
                sal_uInt16 nWhich = rPool.GetWhich( SID_ATTR_CHAR_FONTHEIGHT );
                const SfxMapUnit eUnit = rPool.GetMetric( nWhich );
                long nH = pItem->GetHeight() * 10;
                nVal = LOGIC( nH, (MapUnit)eUnit, MAP_POINT );
            }
            pMenu->SetCurHeight( nVal );
        }
        else if ( dynamic_cast<const SvxFontItem*>( pState) !=  nullptr )
        {
            const SvxFontItem* pItem = dynamic_cast<const SvxFontItem*>( pState  );

            if ( pItem )
            {
                SfxObjectShell *pDoc = SfxObjectShell::Current();

                if ( pDoc )
                {
                    const SvxFontListItem* pFonts = static_cast<const SvxFontListItem*>(
                        pDoc->GetItem( SID_ATTR_CHAR_FONTLIST ));
                    const FontList* pList = pFonts ? pFonts->GetFontList(): 0;

                    if ( pList )
                    {
                        vcl::FontInfo aFntInf = pList->Get( pItem->GetFamilyName(),
                                                       pItem->GetStyleName() );
                        pMenu->Fill( aFntInf, pList );
                    }
                    // else manche Shells haben keine Fontliste (z.B. Image)
                }
            }
        }
    }
    else
    {
        // irgendwie muss man ja das Men"u f"ullen
        SfxObjectShell* pSh = SfxObjectShell::Current();

        if ( pSh )
        {
            // daf"ur von der Shell eine Fontliste besorgen
            const SvxFontListItem* pFonts =
                static_cast<const SvxFontListItem*>(pSh->GetItem( SID_ATTR_CHAR_FONTLIST ));
            const FontList* pList = pFonts ? pFonts->GetFontList(): NULL;
            if ( pList )
                pMenu->Fill( pList->GetFontName(0), pList );
        }
    }
}



/*  [Beschreibung]

    Ctor; setzt den Select-Handler am Men"u und tr"agt Men"u
    in seinen Parent ein.
*/

SvxFontSizeMenuControl::SvxFontSizeMenuControl
(
    sal_uInt16          _nId,
    Menu&           rMenu,
    SfxBindings&    rBindings
) :
    SfxMenuControl( _nId, rBindings ),

    pMenu   ( new FontSizeMenu ),
    rParent ( rMenu ),
    aFontNameForwarder( SID_ATTR_CHAR_FONT, *this )

{
    rMenu.SetPopupMenu( _nId, pMenu );
    pMenu->SetSelectHdl( LINK( this, SvxFontSizeMenuControl, MenuSelect ) );
}



/*  [Beschreibung]

    Dtor; gibt das Men"u frei.
*/

SvxFontSizeMenuControl::~SvxFontSizeMenuControl()
{
    delete pMenu;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
