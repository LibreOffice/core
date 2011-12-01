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

#include <comphelper/string.hxx>
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
#include "sfx2/sfxresid.hxx"
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
    aNameMLE    ( this, SfxResId( MLE_NAME ) ),
    aAutoCB     ( this, SfxResId( CB_AUTO ) ),

    aFollowFt   ( this, SfxResId( FT_NEXT ) ),
    aFollowLb   ( this, SfxResId( LB_NEXT ) ),

    aBaseFt     ( this, SfxResId( FT_BASE ) ),
    aBaseLb     ( this, SfxResId( LB_BASE ) ),

    aFilterFt   ( this, SfxResId( FT_REGION ) ),
    aFilterLb   ( this, SfxResId( LB_REGION ) ),

    aDescGb     ( this, SfxResId( GB_DESC ) ),
    aDescFt     ( this, SfxResId( FT_DESC ) ),
    aDescED     ( this, SfxResId( ED_DESC ) ),

    pStyle( &( (SfxStyleDialog*)GetParentDialog() )->GetStyleSheet() ),

    pItem       ( 0 ),
    bModified   ( sal_False ),
    aName       ( pStyle->GetName() ),
    aFollow     ( pStyle->GetFollow() ),
    aParent     ( pStyle->GetParent() ),
    nFlags      ( pStyle->GetMask() )

/*  [Description]

    Constructor, initializes the list box with the templates
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

    // this Page needs ExchangeSupport
    SetExchangeSupport();

    ResMgr* pResMgr = SFX_APP()->GetModule_Impl()->GetResMgr();
    OSL_ENSURE( pResMgr, "No ResMgr in Module" );
    pFamilies = new SfxStyleFamilies( ResId( DLG_STYLE_DESIGNER, *pResMgr ) );

    SfxStyleSheetBasePool* pPool = 0;
    SfxObjectShell* pDocShell = SfxObjectShell::Current();

    if ( pDocShell )
        pPool = pDocShell->GetStyleSheetPool();
    OSL_ENSURE( pPool, "no Pool or no DocShell" );

    if ( pPool )
    {
        pPool->SetSearchMask( pStyle->GetFamily() );
        pPool->First();     // for SW - update internal list
    }

    if ( !pStyle->GetName().Len() && pPool )
    {
        // NullString as Name -> generate Name
        String aNoName( SfxResId(STR_NONAME).toString() );
        sal_uInt16 nNo = 1;
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

    // Set the field read-only if it is NOT an user-defined style
    // but allow selecting and copying
    if ( !pStyle->IsUserDefined() ) {
        aNameEd.SetReadOnly();
        aNameEd.Hide();

        aNameMLE.SetControlBackground( GetSettings().GetStyleSettings().GetDialogColor() );
        aNameMLE.SetText( pStyle->GetName() );
        aNameMLE.EnableCursor( sal_False );
        aNameMLE.Show();
    }

    if ( pStyle->HasFollowSupport() && pPool )
    {
        SfxStyleSheetBase* pPoolStyle = pPool->First();

        while ( pPoolStyle )
        {
            aFollowLb.InsertEntry( pPoolStyle->GetName() );
            pPoolStyle = pPool->Next();
        }

        // A new Template is not jet in the Pool
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
            // the base template can be set to NULL
            aBaseLb.InsertEntry( SfxResId(STR_NONE).toString() );

        SfxStyleSheetBase* pPoolStyle = pPool->First();

        while ( pPoolStyle )
        {
            const String aStr( pPoolStyle->GetName() );
            // own name as base template
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
        sal_uInt16 nStyleFilterIdx = 0xffff;
        // Filter flags
        const SfxStyleFilter& rList = pItem->GetFilterList();
        nCount = rList.size();
        sal_uInt16 nIdx = 0;
        sal_uInt16 nMask = pStyle->GetMask() & ~SFXSTYLEBIT_USERDEF;

        if ( !nMask )   // User Template?
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
    // It is a style with auto update? (SW only)
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

/*  [Description]

    Destructor, release of the data
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

/*  [Description]

    After the change of a template name update the ListBox pBox

    [Parameter]

    ListBox* pBox         ListBox, whose entries are to be updated
    const String& rNew    the new Name
*/

{
    if ( pBox->IsEnabled() )
    {
        // it is the current entry, which name was modified
        const sal_Bool bSelect = pBox->GetSelectEntry() == aBuf;
        pBox->RemoveEntry( aBuf );
        pBox->InsertEntry( rNew );

        if ( bSelect )
            pBox->SelectEntry( rNew );
    }
}

//-------------------------------------------------------------------------

void SfxManageStyleSheetPage::SetDescriptionText_Impl()

/*  [Description]

    Set attribute description. Get the set metric for this.
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
            OSL_FAIL( "non supported field unit" );
    }
    aDescFt.SetText( pStyle->GetDescription( eUnit ) );
}

//-------------------------------------------------------------------------

IMPL_LINK_INLINE_START( SfxManageStyleSheetPage, GetFocusHdl, Edit *, pEdit )

/*  [Description]

    StarView Handler; GetFocus-Handler of the Edits with the template name.
*/

{
    aBuf = comphelper::string::stripStart(pEdit->GetText(), ' ');
    return 0;
}
IMPL_LINK_INLINE_END( SfxManageStyleSheetPage, GetFocusHdl, Edit *, pEdit )

//-------------------------------------------------------------------------

IMPL_LINK_INLINE_START( SfxManageStyleSheetPage, LoseFocusHdl, Edit *, pEdit )

/*  [Description]

    StarView Handler; loose-focus-handler of the edits of the template name.
    This will update the listbox with the subsequent templates. The current
    template itself is not returned in the listbox of the base templates.
*/

{
    const String aStr(comphelper::string::stripStart(pEdit->GetText(), ' '));
    pEdit->SetText( aStr );
    // Update the Listbox of the base template if possible
    if ( aStr != aBuf )
        UpdateName_Impl( &aFollowLb, aStr );
    return 0;
}
IMPL_LINK_INLINE_END( SfxManageStyleSheetPage, LoseFocusHdl, Edit *, pEdit )

//-------------------------------------------------------------------------

sal_Bool SfxManageStyleSheetPage::FillItemSet( SfxItemSet& rSet )

/*  [Description]

    Handler for setting the (modified) data. I called from the OK of the
    SfxTabDialog.

    [Parameter]

    SfxItemSet &rAttrSet    The set, which receives the data.

    [Return value]

    sal_Bool                sal_True:  The data had been changed
                            sal_False: The data had not been changed

    [Cross-reference]

    <class SfxTabDialog>
*/

{
    const sal_uInt16 nFilterIdx = aFilterLb.GetSelectEntryPos();

    // Set Filter

    if ( LISTBOX_ENTRY_NOTFOUND  != nFilterIdx      &&
         nFilterIdx != aFilterLb.GetSavedValue()    &&
         aFilterLb.IsEnabled() )
    {
        bModified = sal_True;
        OSL_ENSURE( pItem, "No Item" );
        // is only possibly for user templates
        sal_uInt16 nMask = pItem->GetFilterList()[ (size_t)aFilterLb.GetEntryData( nFilterIdx ) ]->nFlags | SFXSTYLEBIT_USERDEF;
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

/*  [Description]

    Handler to initialize the page with the initial data.

    [Parameter]

    const SfxItemSet &rAttrSet          The data set

    [Cross-reference]

    <class SfxTabDialog>
*/

{
    bModified = sal_False;
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
            aBaseLb.SelectEntry( SfxResId(STR_NONE).toString() );
        else
            aBaseLb.SelectEntry( aParent );

        if ( SfxResId(STR_STANDARD).toString().equals(aName) )
        {
            // the default template can not be linked
            aBaseFt.Disable();
            aBaseLb.Disable();
        }
    }

    if ( aFilterLb.IsEnabled() )
    {
        sal_uInt16 nCmp = pStyle->GetMask();

        if ( nCmp != nFlags )
            pStyle->SetMask( nFlags );
        aFilterLb.SelectEntryPos( aFilterLb.GetSavedValue() );
    }
}

//-------------------------------------------------------------------------

SfxTabPage* SfxManageStyleSheetPage::Create( Window* pParent,
                                             const SfxItemSet &rAttrSet )

/*  [Description]

    Factory for the creation of the page.

    [Cross-reference]

    <class SfxTabDialog>
*/

{
    return new SfxManageStyleSheetPage( pParent, rAttrSet );
}

//-------------------------------------------------------------------------

void SfxManageStyleSheetPage::ActivatePage( const SfxItemSet& rSet)

/*  [Description]

    ActivatePage handler of SfxTabDialog, is used for the the update of the
    descriptive text, since this might have changed through changes of data on
    other pages.

    [Parameter]

    const SfxItemSet&    the set for the exchange of data; is not used here.

    [Cross-reference]

    <SfxTabDialog::ActivatePage(const SfxItemSet &)>
*/

{
    SetDescriptionText_Impl();

    // It is a style with auto update? (SW only)
    const SfxPoolItem* pPoolItem;

    if ( SFX_ITEM_SET ==
         rSet.GetItemState( SID_ATTR_AUTO_STYLE_UPDATE, sal_False, &pPoolItem ) )
        aAutoCB.Check( ( (const SfxBoolItem*)pPoolItem )->GetValue() );
    aAutoCB.SaveValue();
}

//-------------------------------------------------------------------------

int SfxManageStyleSheetPage::DeactivatePage( SfxItemSet* pItemSet )

/*  [Description]

    DeactivatePage-handler of SfxTabDialog; data is set on the template, so
    that the correct inheritance on the other pages of the dialog is made.
    If an error occurs, leaving the page is prevented.
    [Parameter]

    SfxItemSet*    the set for the exchange of data; is not used here.

    [Cross-reference]

    <SfxTabDialog::DeactivatePage(SfxItemSet*)>
*/

{
    int nRet = SfxTabPage::LEAVE_PAGE;

    if ( aNameEd.IsModified() )
    {
        // By pressing <Enter> LoseFocus() is not trigged through StarView
        if ( aNameEd.HasFocus() )
            LoseFocusHdl( &aNameEd );

        if (!pStyle->SetName(comphelper::string::stripStart(aNameEd.GetText(), ' ')))
        {
            InfoBox aBox( this, SfxResId( MSG_TABPAGE_INVALIDNAME ) );
            aBox.Execute();
            aNameEd.GrabFocus();
            aNameEd.SetSelection( Selection( SELECTION_MIN, SELECTION_MAX ) );
            return SfxTabPage::KEEP_PAGE;
        }
        bModified = sal_True;
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
            bModified = sal_True;
        }
    }

    if ( aBaseLb.IsEnabled() )
    {
        String aParentEntry( aBaseLb.GetSelectEntry() );

        if ( SfxResId(STR_NONE).toString().equals(aParentEntry) || aParentEntry == pStyle->GetName() )
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
            bModified = sal_True;
            nRet |= (int)SfxTabPage::REFRESH_SET;
        }
    }

    if ( pItemSet )
        FillItemSet( *pItemSet );

    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
