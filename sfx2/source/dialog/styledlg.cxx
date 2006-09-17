/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: styledlg.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 16:36:41 $
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
#include "precompiled_sfx2.hxx"

// include ---------------------------------------------------------------

#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef GCC
#endif

#include "styledlg.hxx"
#include "mgetempl.hxx"
#include "sfxresid.hxx"
#include "sfxuno.hxx"

#include "dialog.hrc"

// class SfxStyleDialog --------------------------------------------------

SfxStyleDialog::SfxStyleDialog
(
    Window* pParent,            // Parent
    const ResId& rResId,        // ResId
    SfxStyleSheetBase& rStyle,  // zu bearbeitendes StyleSheet
    BOOL bFreeRes,              // Flag Resourcen freigeben
    const String* pUserBtnTxt
) :

/*  [Beschreibung]

    Konstruktor: Verwalten-TabPage zuf"ugen, ExampleSet vom Style setzen.
*/

    SfxTabDialog( pParent, rResId,
                  rStyle.GetItemSet().Clone(),
                  // auch ohne ParentSupport TRUE "ubergeben, aber erweitert
                  // um den StandardButton zu unterdr"ucken
                  rStyle.HasParentSupport() ? TRUE : 2,
                  pUserBtnTxt ),

    pStyle( &rStyle )

{
    AddTabPage( ID_TABPAGE_MANAGESTYLES,
                String( SfxResId( STR_TABPAGE_MANAGESTYLES ) ),
                SfxManageStyleSheetPage::Create, 0, FALSE, 0 );

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
    USHORT nWhich = aIter.FirstWhich();

    while ( nWhich )
    {
        SfxItemState eState = pInSet->GetItemState( nWhich, FALSE );

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


