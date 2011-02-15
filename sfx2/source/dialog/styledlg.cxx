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
#include "precompiled_sfx2.hxx"

// include ---------------------------------------------------------------
#include <svl/whiter.hxx>
#include <svl/style.hxx>
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef GCC
#endif

#include <sfx2/styledlg.hxx>
#include <sfx2/mgetempl.hxx>
#include "sfx2/sfxresid.hxx"
#include <sfx2/sfxuno.hxx>

#include "dialog.hrc"

// class SfxStyleDialog --------------------------------------------------

SfxStyleDialog::SfxStyleDialog
(
    Window* pParent,            // Parent
    const ResId& rResId,        // ResId
    SfxStyleSheetBase& rStyle,  // zu bearbeitendes StyleSheet
    sal_Bool bFreeRes,              // Flag Resourcen freigeben
    const String* pUserBtnTxt
) :

/*  [Beschreibung]

    Konstruktor: Verwalten-TabPage zuf"ugen, ExampleSet vom Style setzen.
*/

    SfxTabDialog( pParent, rResId,
                  rStyle.GetItemSet().Clone(),
                  // auch ohne ParentSupport TRUE "ubergeben, aber erweitert
                  // um den StandardButton zu unterdr"ucken
                  rStyle.HasParentSupport() ? sal_True : 2,
                  pUserBtnTxt ),

    pStyle( &rStyle )

{
    AddTabPage( ID_TABPAGE_MANAGESTYLES,
                String( SfxResId( STR_TABPAGE_MANAGESTYLES ) ),
                SfxManageStyleSheetPage::Create, 0, sal_False, 0 );

    // bei neuer Vorlage immer die Verwaltungsseite als aktuelle
    // Seite setzen

    if( !rStyle.GetName().Len() )
        SetCurPageId( ID_TABPAGE_MANAGESTYLES );
    else
    {
        String sTxt( GetText() );
        sTxt += DEFINE_CONST_UNICODE(": ") ;
        sTxt += rStyle.GetName();
        SetText( sTxt );
    }
    delete pExampleSet; // im SfxTabDialog::Ctor() schon angelegt
    pExampleSet = &pStyle->GetItemSet();

    if ( bFreeRes )
        FreeResource();
    GetCancelButton().SetClickHdl( LINK(this, SfxStyleDialog, CancelHdl) );
}

// -----------------------------------------------------------------------

SfxStyleDialog::~SfxStyleDialog()

/*  [Beschreibung]

    Destruktor: ExampleSet auf NULL setzen, damit der SfxTabDialog nicht den
    Set vom Style l"oscht.
*/

{
    pExampleSet = 0;
    pStyle = 0;
    delete GetInputSetImpl();
}

// -----------------------------------------------------------------------

const SfxItemSet* SfxStyleDialog::GetRefreshedSet()

/*  [Beschreibung]

    Diese wird gerufen, wenn <SfxTabPage::DeactivatePage(SfxItemSet *)>
    <SfxTabPage::REFRESH_SET> liefert.
*/

{
    return GetInputSetImpl();
}

// -----------------------------------------------------------------------

short SfxStyleDialog::Ok()

/*  [Beschreibung]

    "Uberladen, damit immer RET_OK zur"uckgegeben wird.
*/

{
    SfxTabDialog::Ok();
    return RET_OK;
}

// -----------------------------------------------------------------------

IMPL_LINK( SfxStyleDialog, CancelHdl, Button *, pButton )

/*  [Beschreibung]

    Wenn der Dialog abgebrochen wurde, m"ussen alle schon eingestellten
    Attribute wieder zur"uckgesetzt werden.
*/

{
    (void)pButton; //unused
    SfxTabPage* pPage = GetTabPage( ID_TABPAGE_MANAGESTYLES );

    const SfxItemSet* pInSet = GetInputSetImpl();
    SfxWhichIter aIter( *pInSet );
    sal_uInt16 nWhich = aIter.FirstWhich();

    while ( nWhich )
    {
        SfxItemState eState = pInSet->GetItemState( nWhich, sal_False );

        if ( SFX_ITEM_DEFAULT == eState )
            pExampleSet->ClearItem( nWhich );
        else
            pExampleSet->Put( pInSet->Get( nWhich ) );
        nWhich = aIter.NextWhich();
    }

    if ( pPage )
        pPage->Reset( *GetInputSetImpl() );
    EndDialog( RET_CANCEL );
    return 0;
}


