/*************************************************************************
 *
 *  $RCSfile: mgetempl.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mba $ $Date: 2000-09-28 11:43:15 $
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

#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SV_FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif
#pragma hdrstop

#include "styfitem.hxx"
#include "styledlg.hxx"
#include "app.hxx"
#include "mgetempl.hxx"
#include "objsh.hxx"
#include "sfxtypes.hxx"
#include "sfxresid.hxx"
#include "module.hxx"
#include "saveopt.hxx"

#include "sfx.hrc"
#include "dialog.hrc"
#include "mgetempl.hrc"

#ifndef _SFX_STYLE_HRC
#include <svtools/style.hrc>
#endif

// SfxManageStyleSheetPage -----------------------------------------------

SfxManageStyleSheetPage::SfxManageStyleSheetPage( Window* pParent, const SfxItemSet& rAttrSet ) :

    SfxTabPage( pParent, SfxResId( TP_MANAGE_STYLES ), rAttrSet ),

    aNameFt     ( this, ResId( FT_NAME ) ),
    aNameEd     ( this, ResId( ED_NAME ) ),
    aAutoCB     ( this, ResId( CB_AUTO ) ),

    aFollowFt   ( this, ResId( FT_NEXT ) ),
    aFollowLb   ( this, ResId( LB_NEXT ) ),

    aBaseFt     ( this, ResId( FT_BASE ) ),
    aBaseLb     ( this, ResId( LB_BASE ) ),

    aFilterFt   ( this, ResId( FT_REGION ) ),
    aFilterLb   ( this, ResId( LB_REGION ) ),

    aDescFt     ( this, ResId( FT_DESC ) ),
    aDescED     ( this, ResId( ED_DESC ) ),
    aDescGb     ( this, ResId( GB_DESC ) ),

    pStyle( &( (SfxStyleDialog*)pParent->GetParent() )->GetStyleSheet() ),

    bModified   ( FALSE ),
    aName       ( pStyle->GetName() ),
    aFollow     ( pStyle->GetFollow() ),
    aParent     ( pStyle->GetParent() ),
    nFlags      ( pStyle->GetMask() ),
    pItem       ( 0 )

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
    pFamilies = new SfxStyleFamilies( ResId( DLG_STYLE_DESIGNER, pResMgr ) );

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
    USHORT nCount = pFamilies->Count();

    USHORT i;
    for ( i = 0; i < nCount; ++i )
    {
        pItem = pFamilies->GetObject(i);

        if ( pItem->GetFamily() == pStyle->GetFamily() )
            break;
    }

    USHORT nStyleFilterIdx = 0xffff;

    if ( i < nCount )
    {
        // Filterflags
        const SfxStyleFilter& rList = pItem->GetFilterList();
        nCount = (USHORT)rList.Count();
        USHORT nIdx = 0;
        USHORT nMask = pStyle->GetMask() & ~SFXSTYLEBIT_USERDEF;

        if ( !nMask )   // Benutzervorlage?
            nMask = pStyle->GetMask();

        for ( i = 0; i < nCount; ++i )
        {
            SfxFilterTupel* pTupel = rList.GetObject(i);

            if ( pTupel->nFlags != SFXSTYLEBIT_AUTO     &&
                 pTupel->nFlags != SFXSTYLEBIT_USED     &&
//               pTupel->nFlags != SFXSTYLEBIT_USERDEF  &&
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
    SfxApplication* pSfxApp = SFX_APP();
    SfxMapUnit eUnit = SFX_MAPUNIT_CM;
    FieldUnit eFieldUnit = pSfxApp->GetOptions().GetMetric();
    SfxModule* pModule = pSfxApp->GetActiveModule();

    if ( pModule )
    {
        const SfxPoolItem* pItem = pModule->GetItem( SID_ATTR_METRIC );

        if ( pItem )
            eFieldUnit = (FieldUnit)( (SfxUInt16Item*)pItem )->GetValue();
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
    aBuf = pEdit->GetText();
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
    const String aStr( pEdit->GetText() );
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
#ifdef DEBUG
        USHORT nIdx = (USHORT)(long)aFilterLb.GetEntryData( nFilterIdx );
        SfxFilterTupel* p = pItem->GetFilterList().GetObject( nIdx );
#endif
        USHORT nMask = pItem->GetFilterList().GetObject(
            (USHORT)(long)aFilterLb.GetEntryData( nFilterIdx ) )->nFlags |
            SFXSTYLEBIT_USERDEF;
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

void SfxManageStyleSheetPage::Reset( const SfxItemSet& rAttrSet )

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
    const SfxPoolItem* pItem;

    if ( SFX_ITEM_SET ==
         rSet.GetItemState( SID_ATTR_AUTO_STYLE_UPDATE, FALSE, &pItem ) )
        aAutoCB.Check( ( (const SfxBoolItem*)pItem )->GetValue() );
    aAutoCB.SaveValue();
}

//-------------------------------------------------------------------------

int SfxManageStyleSheetPage::DeactivatePage( SfxItemSet* pSet )

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
        const String aFollow( aFollowLb.GetSelectEntry() );

        if ( pStyle->GetFollow() != aFollow )
        {
            if ( !pStyle->SetFollow( aFollow ) )
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
        String aParent( aBaseLb.GetSelectEntry() );

        if ( String( SfxResId( STR_NONE ) ) == aParent || aParent == pStyle->GetName() )
            aParent.Erase();

        if ( pStyle->GetParent() != aParent )
        {
            if ( !pStyle->SetParent( aParent ) )
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

    if ( pSet )
        FillItemSet( *pSet );

    return nRet;
}

