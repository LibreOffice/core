/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include <vcl/msgbox.hxx>
#include <vcl/field.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svl/style.hxx>

#include <sfx2/styfitem.hxx>
#include <sfx2/styledlg.hxx>
#include <sfx2/app.hxx>
#include <sfx2/mgetempl.hxx>
#include <sfx2/objsh.hxx>
#include "sfxtypes.hxx"
#include "sfxresid.hxx"
#include <sfx2/module.hxx>

#include <sfx2/sfx.hrc>
#include "dialog.hrc"
#include "mgetempl.hrc"

#include <svl/style.hrc>

// SfxManageStyleSheetPage -----------------------------------------------

SfxManageStyleSheetPage::SfxManageStyleSheetPage( Window* pParent, const SfxItemSet& rAttrSet ) :

    SfxTabPage( pParent, SfxResId( TP_MANAGE_STYLES ), rAttrSet ),

    aNameFt     ( this, SfxResId( FT_NAME ) ),
    aNameEd     ( this, SfxResId( ED_NAME ) ),
    aAutoCB     ( this, SfxResId( CB_AUTO ) ),

    aFollowFt   ( this, SfxResId( FT_NEXT ) ),
    aFollowLb   ( this, SfxResId( LB_NEXT ) ),

    aBaseFt     ( this, SfxResId( FT_BASE ) ),
    aBaseLb     ( this, SfxResId( LB_BASE ) ),

    aFilterFt   ( this, SfxResId( FT_REGION ) ),
    aFilterLb   ( this, SfxResId( LB_REGION ) ),

    aDescFt     ( this, SfxResId( FT_DESC ) ),
    aDescED     ( this, SfxResId( ED_DESC ) ),
    aDescGb     ( this, SfxResId( GB_DESC ) ),

    pStyle( &( (SfxStyleDialog*)pParent->GetParent() )->GetStyleSheet() ),

    pItem       ( 0 ),
    bModified   ( FALSE ),
    aName       ( pStyle->GetName() ),
    aFollow     ( pStyle->GetFollow() ),
    aParent     ( pStyle->GetParent() ),
    nFlags      ( pStyle->GetMask() )

/*  [Beschreibung]
    Konstruktor; initialisiert die ListBoxen mit den Vorlagen
*/

{
    FreeResource();

    if ( IsBackground() )
        aDescED.SetBackground( GetBackground() );
    FixedInfo* pInfo = new FixedInfo( this );
    aDescED.SetFont( pInfo->GetFont() );
    delete pInfo;

    aDescED.Hide();
    aDescFt.Show();

    // diese Page braucht ExchangeSupport
    SetExchangeSupport();

    ResMgr* pResMgr = SFX_APP()->GetModule_Impl()->GetResMgr();
    DBG_ASSERT( pResMgr, "kein ResMgr am Modul" );
    pFamilies = new SfxStyleFamilies( ResId( DLG_STYLE_DESIGNER, *pResMgr ) );

    SfxStyleSheetBasePool* pPool = 0;
    SfxObjectShell* pDocShell = SfxObjectShell::Current();

    if ( pDocShell )
        pPool = pDocShell->GetStyleSheetPool();
    DBG_ASSERT( pPool, "kein Pool oder keine DocShell" );

    if ( pPool )
    {
        pPool->SetSearchMask( pStyle->GetFamily() );
        pPool->First();     // fuer SW - interne Liste updaten
    }

    if ( !pStyle->GetName().Len() && pPool )
    {
        // NullString als Name -> Name generieren
        String aNoName( SfxResId( STR_NONAME ) );
        USHORT nNo = 1;
        String aNo( aNoName );
        aNoName += String::CreateFromInt32( nNo );
        while ( pPool->Find( aNoName ) )
        {
            ++nNo;
            aNoName = aNo;
            aNoName += String::CreateFromInt32( nNo );
        }
        pStyle->SetName( aNoName );
        aName = aNoName;
        aFollow = pStyle->GetFollow();
        aParent = pStyle->GetParent();
    }
    aNameEd.SetText(pStyle->GetName());

    if ( !pStyle->IsUserDefined() )
        aNameEd.SetReadOnly();

    if ( pStyle->HasFollowSupport() && pPool )
    {
        SfxStyleSheetBase* pPoolStyle = pPool->First();

        while ( pPoolStyle )
        {
            aFollowLb.InsertEntry( pPoolStyle->GetName() );
            pPoolStyle = pPool->Next();
        }

        // eine neue Vorlage ist noch nicht im Pool
        if ( LISTBOX_ENTRY_NOTFOUND == aFollowLb.GetEntryPos( pStyle->GetName() ) )
            aFollowLb.InsertEntry( pStyle->GetName() );
    }
    else
    {
        aFollowFt.Hide();
        aFollowLb.Hide();

        aFilterFt.SetPosPixel( aBaseFt.GetPosPixel() );
        aFilterLb.SetPosPixel( aBaseLb.GetPosPixel() );

        aBaseFt.SetPosPixel( aFollowFt.GetPosPixel() );
        aBaseLb.SetPosPixel( aFollowLb.GetPosPixel() );
    }

    if ( pStyle->HasParentSupport() && pPool )
    {
        if ( pStyle->HasClearParentSupport() )
            // die Basisvorlage darf auf NULL gesetzt werden
            aBaseLb.InsertEntry( String( SfxResId( STR_NONE ) ) );

        SfxStyleSheetBase* pPoolStyle = pPool->First();

        while ( pPoolStyle )
        {
            const String aStr( pPoolStyle->GetName() );
            // eigener Namen nicht als Basisvorlage
            if ( aStr != aName )
                aBaseLb.InsertEntry( aStr );
            pPoolStyle = pPool->Next();
        }
    }
    else
    {
        aBaseFt.Disable();
        aBaseLb.Disable();
    }

    size_t nCount = pFamilies->size();
    size_t i;
    for ( i = 0; i < nCount; ++i )
    {
        pItem = pFamilies->at( i );

        if ( pItem->GetFamily() == pStyle->GetFamily() )
            break;
    }

    if ( i < nCount )
    {
        USHORT nStyleFilterIdx = 0xffff;
        // Filterflags
        const SfxStyleFilter& rList = pItem->GetFilterList();
        nCount = rList.size();
        USHORT nIdx = 0;
        USHORT nMask = pStyle->GetMask() & ~SFXSTYLEBIT_USERDEF;

        if ( !nMask )   // Benutzervorlage?
            nMask = pStyle->GetMask();

        for ( i = 0; i < nCount; ++i )
        {
            SfxFilterTupel* pTupel = rList[ i ];

            if ( pTupel->nFlags != SFXSTYLEBIT_AUTO     &&
                 pTupel->nFlags != SFXSTYLEBIT_USED     &&
                 pTupel->nFlags != SFXSTYLEBIT_ALL )
            {
                aFilterLb.InsertEntry( pTupel->aName, nIdx );
                aFilterLb.SetEntryData(nIdx, (void*)(long)i);

                if ( ( pTupel->nFlags & nMask ) == nMask )
                    nStyleFilterIdx = nIdx;
                ++nIdx;
            }
        }

        if ( nStyleFilterIdx != 0xFFFF )
            aFilterLb.SelectEntryPos( nStyleFilterIdx );
    }

    if ( !aFilterLb.GetEntryCount() || !pStyle->IsUserDefined() )
    {
        pItem = 0;
        aFilterFt.Disable();
        aFilterLb.Disable();
    }
    else
        aFilterLb.SaveValue();
    SetDescriptionText_Impl();

    if ( aFollowLb.IsEnabled() || aBaseLb.IsEnabled() )
    {
        aNameEd.SetGetFocusHdl(
            LINK( this, SfxManageStyleSheetPage, GetFocusHdl ) );
        aNameEd.SetLoseFocusHdl(
            LINK( this, SfxManageStyleSheetPage, LoseFocusHdl ) );
    }
    // ist es ein Style mit automatischem Update? (nur SW)
    if(SFX_ITEM_SET == rAttrSet.GetItemState(SID_ATTR_AUTO_STYLE_UPDATE))
    {
        Size aSize = aNameEd.GetSizePixel();
        aSize.Width() /= 2;
        aNameEd.SetSizePixel(aSize);
        aAutoCB.Show();
    }
}

//-------------------------------------------------------------------------

SfxManageStyleSheetPage::~SfxManageStyleSheetPage()

/*  [Beschreibung]
    Destruktor; Freigabe der Daten
*/

{
    aNameEd.SetGetFocusHdl( Link() );
    aNameEd.SetLoseFocusHdl( Link() );
    delete pFamilies;
    pItem = 0;
    pStyle = 0;

}

//-------------------------------------------------------------------------

void SfxManageStyleSheetPage::UpdateName_Impl( ListBox* pBox,
                                               const String& rNew )

/*  [Beschreibung]
    Nach der "Anderung eines Vorlagennamens die ListBox pBox
    aktualisieren

    [Parameter]
    ListBox* pBox           ListBox, deren Eintr"age aktualisiert
                            werden sollen
    const String& rNew      der neue Name
*/

{
    if ( pBox->IsEnabled() )
    {
        // ist der aktuelle Eintrag, dessen Namen modifizieren wurde
        const BOOL bSelect = pBox->GetSelectEntry() == aBuf;
        pBox->RemoveEntry( aBuf );
        pBox->InsertEntry( rNew );

        if ( bSelect )
            pBox->SelectEntry( rNew );
    }
}

//-------------------------------------------------------------------------

void SfxManageStyleSheetPage::SetDescriptionText_Impl()

/*  [Beschreibung]
    Attributbeschreibung setzen. daf"ur die eingestellte Metrik besorgen.
*/

{
    SfxMapUnit eUnit = SFX_MAPUNIT_CM;
    FieldUnit eFieldUnit( FUNIT_CM );
    SfxModule* pModule = SfxModule::GetActiveModule();
    if ( pModule )
    {
        const SfxPoolItem* pPoolItem = pModule->GetItem( SID_ATTR_METRIC );
        if ( pPoolItem )
            eFieldUnit = (FieldUnit)( (SfxUInt16Item*)pPoolItem )->GetValue();
    }

    switch ( eFieldUnit )
    {
        case FUNIT_MM:      eUnit = SFX_MAPUNIT_MM; break;
        case FUNIT_CM:
        case FUNIT_M:
        case FUNIT_KM:      eUnit = SFX_MAPUNIT_CM; break;
        case FUNIT_POINT:
        case FUNIT_PICA:    eUnit = SFX_MAPUNIT_POINT; break;
        case FUNIT_INCH:
        case FUNIT_FOOT:
        case FUNIT_MILE:    eUnit = SFX_MAPUNIT_INCH; break;

        default:
            DBG_ERRORFILE( "not supported fieldunit" );
    }
    aDescFt.SetText( pStyle->GetDescription( eUnit ) );
}

//-------------------------------------------------------------------------

IMPL_LINK_INLINE_START( SfxManageStyleSheetPage, GetFocusHdl, Edit *, pEdit )

/*  [Beschreibung]
    StarView Handler; GetFocus-Handler des Edits mit dem Vorlagennamen.
*/

{
    aBuf = pEdit->GetText().EraseLeadingChars();
    return 0;
}
IMPL_LINK_INLINE_END( SfxManageStyleSheetPage, GetFocusHdl, Edit *, pEdit )

//-------------------------------------------------------------------------

IMPL_LINK_INLINE_START( SfxManageStyleSheetPage, LoseFocusHdl, Edit *, pEdit )

/*  [Beschreibung]

    StarView Handler; LoseFocus-Handler des Edits mit dem Vorlagennamen.
    Dieser aktualisiert ggf. die Listbox mit den Folgevorlagen.
    In der Listbox mit den Basisvorlagen ist die aktuelle Vorlage
    selbst nicht enthalten.

*/

{
    const String aStr( pEdit->GetText().EraseLeadingChars() );
    pEdit->SetText( aStr );
    // ggf. Listbox der Basisvorlagen aktualisieren
    if ( aStr != aBuf )
        UpdateName_Impl( &aFollowLb, aStr );
    return 0;
}
IMPL_LINK_INLINE_END( SfxManageStyleSheetPage, LoseFocusHdl, Edit *, pEdit )

//-------------------------------------------------------------------------

BOOL SfxManageStyleSheetPage::FillItemSet( SfxItemSet& rSet )

/*  [Beschreibung]
    Handler f"ur das Setzen der (modifizierten) Daten. Wird aus
    dem Ok des SfxTabDialog gerufen.

    [Parameter]
    SfxItemSet &rAttrSet        das Set, das die Daten entgegennimmt.

    [R"uckgabewert]
    BOOL                        TRUE:  es hat eine "Anderung der Daten
                                       stattgefunden
                                FALSE: es hat keine "Anderung der Daten
                                       stattgefunden

    [Querverweise]
    <class SfxTabDialog>
*/

{
    const USHORT nFilterIdx = aFilterLb.GetSelectEntryPos();

    // Filter setzen

    if ( LISTBOX_ENTRY_NOTFOUND  != nFilterIdx      &&
         nFilterIdx != aFilterLb.GetSavedValue()    &&
         aFilterLb.IsEnabled() )
    {
        bModified = TRUE;
        DBG_ASSERT( pItem, "kein Item" );
        // geht nur bei Benutzervorlagen
#if OSL_DEBUG_LEVEL > 1
        USHORT nIdx = (USHORT)(long)aFilterLb.GetEntryData( nFilterIdx );
        SfxFilterTupel* p = pItem->GetFilterList()[ nIdx ];
#endif
        USHORT nMask = pItem->GetFilterList()[ (size_t)aFilterLb.GetEntryData( nFilterIdx ) ]->nFlags | SFXSTYLEBIT_USERDEF;
        pStyle->SetMask( nMask );
    }
    if(aAutoCB.IsVisible() &&
        aAutoCB.IsChecked() != aAutoCB.GetSavedValue())
    {
        rSet.Put(SfxBoolItem(SID_ATTR_AUTO_STYLE_UPDATE, aAutoCB.IsChecked()));
    }

    return bModified;
}

//-------------------------------------------------------------------------

void SfxManageStyleSheetPage::Reset( const SfxItemSet& /*rAttrSet*/ )

/*  [Beschreibung]
    Handler f"ur das Initialisieren der Seite mit den initialen Daten.

    [Parameter]
    const SfxItemSet &rAttrSet      das Set mit den Daten

    [Querverweise]
    <class SfxTabDialog>
*/

{
    bModified = FALSE;
    String sCmp( pStyle->GetName() );

    if ( sCmp != aName )
        pStyle->SetName( aName );
    aNameEd.SetText( aName );

    if ( aFollowLb.IsEnabled() )
    {
        sCmp = pStyle->GetFollow();

        if ( sCmp != aFollow )
            pStyle->SetFollow( aFollow );

        if ( !aFollow.Len() )
            aFollowLb.SelectEntry( aName );
        else
            aFollowLb.SelectEntry( aFollow );
    }

    if ( aBaseLb.IsEnabled() )
    {
        sCmp = pStyle->GetParent();

        if ( sCmp != aParent )
            pStyle->SetParent( aParent );

        if ( !aParent.Len() )
            aBaseLb.SelectEntry( String( SfxResId( STR_NONE ) ) );
        else
            aBaseLb.SelectEntry( aParent );

        if ( String( SfxResId( STR_STANDARD ) ) == aName )
        {
            // die Standardvorlage kann nicht verkn"upft werden
            aBaseFt.Disable();
            aBaseLb.Disable();
        }
    }

    if ( aFilterLb.IsEnabled() )
    {
        USHORT nCmp = pStyle->GetMask();

        if ( nCmp != nFlags )
            pStyle->SetMask( nFlags );
        aFilterLb.SelectEntryPos( aFilterLb.GetSavedValue() );
    }
}

//-------------------------------------------------------------------------

SfxTabPage* SfxManageStyleSheetPage::Create( Window* pParent,
                                             const SfxItemSet &rAttrSet )

/*  [Beschreibung]
    Factory f"ur das Erzeugen der Seite

    [Querverweise]
    <class SfxTabDialog>

*/

{
    return new SfxManageStyleSheetPage( pParent, rAttrSet );
}

//-------------------------------------------------------------------------

void SfxManageStyleSheetPage::ActivatePage( const SfxItemSet& rSet)

/*  [Beschreibung]
    ActivatePage- Handler des SfxTabDialog; wird f"ur die Aktualisierung
    des beschreibenden Textes verwendet, da sich dieser durch "Anderungen
    der Daten anderer Seiten ge"andert haben kann.

    [Parameter]
    const SfxItemSet&       das Set f"ur den Datenaustausch; wird
                            hier nicht verwendet.

    [Querverweise]
    <SfxTabDialog::ActivatePage(const SfxItemSet &)>
*/

{
    SetDescriptionText_Impl();

    // ist es ein Style mit automatischem Update? (nur SW)
    const SfxPoolItem* pPoolItem;

    if ( SFX_ITEM_SET ==
         rSet.GetItemState( SID_ATTR_AUTO_STYLE_UPDATE, FALSE, &pPoolItem ) )
        aAutoCB.Check( ( (const SfxBoolItem*)pPoolItem )->GetValue() );
    aAutoCB.SaveValue();
}

//-------------------------------------------------------------------------

int SfxManageStyleSheetPage::DeactivatePage( SfxItemSet* pItemSet )

/*  [Beschreibung]
    DeactivatePage- Handler des SfxTabDialog; die Daten werden
    an der Vorlage eingestellt, damit die richtige Vererbung
    f"ur die anderen Seiten des Dialoges vorliegt.
    Im Fehlerfall wird das Verlassen der Seite unterbunden.

    [Parameter]
    SfxItemSet*         das Set f"ur den Datenaustausch; wird hier nicht verwendet.

    [Querverweise]
    <SfxTabDialog::DeactivatePage(SfxItemSet*)>
*/

{
    int nRet = SfxTabPage::LEAVE_PAGE;

    if ( aNameEd.IsModified() )
    {
        // bei <Enter> wird kein LoseFocus() durch StarView ausgel"ost
        if ( aNameEd.HasFocus() )
            LoseFocusHdl( &aNameEd );

        if ( !pStyle->SetName( aNameEd.GetText().EraseLeadingChars() ) )
        {
            InfoBox aBox( this, SfxResId( MSG_TABPAGE_INVALIDNAME ) );
            aBox.Execute();
            aNameEd.GrabFocus();
            aNameEd.SetSelection( Selection( SELECTION_MIN, SELECTION_MAX ) );
            return SfxTabPage::KEEP_PAGE;
        }
        bModified = TRUE;
    }

    if ( pStyle->HasFollowSupport() && aFollowLb.IsEnabled() )
    {
        const String aFollowEntry( aFollowLb.GetSelectEntry() );

        if ( pStyle->GetFollow() != aFollowEntry )
        {
            if ( !pStyle->SetFollow( aFollowEntry ) )
            {
                InfoBox aBox( this, SfxResId( MSG_TABPAGE_INVALIDSTYLE ) );
                aBox.Execute();
                aFollowLb.GrabFocus();
                return SfxTabPage::KEEP_PAGE;
            }
            bModified = TRUE;
        }
    }

    if ( aBaseLb.IsEnabled() )
    {
        String aParentEntry( aBaseLb.GetSelectEntry() );

        if ( String( SfxResId( STR_NONE ) ) == aParentEntry || aParentEntry == pStyle->GetName() )
            aParentEntry.Erase();

        if ( pStyle->GetParent() != aParentEntry )
        {
            if ( !pStyle->SetParent( aParentEntry ) )
            {
                InfoBox aBox( this, SfxResId( MSG_TABPAGE_INVALIDPARENT ) );
                aBox.Execute();
                aBaseLb.GrabFocus();
                return SfxTabPage::KEEP_PAGE;
            }
            bModified = TRUE;
            nRet |= (int)SfxTabPage::REFRESH_SET;
        }
    }

    if ( pItemSet )
        FillItemSet( *pItemSet );

    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
