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

#include <svl/style.hrc>

/*  SfxManageStyleSheetPage Constructor
 *
 *  initializes the list box with the templates
 */
SfxManageStyleSheetPage::SfxManageStyleSheetPage(Window* pParent, const SfxItemSet& rAttrSet)
    : SfxTabPage(pParent, "ManageStylePage", "sfx/ui/managestylepage.ui", rAttrSet)
    , pStyle(&((SfxStyleDialog*)GetParentDialog())->GetStyleSheet())
    , pItem(0)
    , bModified(false)
    , aName(pStyle->GetName())
    , aFollow(pStyle->GetFollow())
    , aParent(pStyle->GetParent())
    , nFlags(pStyle->GetMask())
{
    get(m_pNameRo, "namero");
    get(m_pNameRw, "namerw");
    get(m_pAutoCB, "autoupdate");
    get(m_pFollowFt, "nextstyleft");
    get(m_pFollowLb, "nextstyle");
    m_pFollowLb->SetStyle(m_pFollowLb->GetStyle() | WB_SORT);
    get(m_pBaseFt, "linkedwithft");
    get(m_pBaseLb, "linkedwith");
    m_pBaseLb->SetStyle(m_pBaseLb->GetStyle() | WB_SORT);
    get(m_pFilterFt, "categoryft");
    get(m_pFilterLb, "category");
    m_pFilterLb->SetStyle(m_pFilterLb->GetStyle() | WB_SORT);
    get(m_pDescFt, "desc");

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

    if ( pStyle->GetName().isEmpty() && pPool )
    {
        // NullString as Name -> generate Name
        String aNoName( SfxResId(STR_NONAME).toString() );
        sal_uInt16 nNo = 1;
        String aNo( aNoName );
        aNoName += OUString::number( nNo );
        while ( pPool->Find( aNoName ) )
        {
            ++nNo;
            aNoName = aNo;
            aNoName += OUString::number( nNo );
        }
        pStyle->SetName( aNoName );
        aName = aNoName;
        aFollow = pStyle->GetFollow();
        aParent = pStyle->GetParent();
    }
    m_pNameRw->SetText(pStyle->GetName());

    // Set the field read-only if it is NOT an user-defined style
    // but allow selecting and copying
    if (!pStyle->IsUserDefined())
    {
        m_pNameRo->SetText(m_pNameRw->GetText());
        m_pNameRw->Hide();
        m_pNameRo->Show();
        FixedText *pLabel = get<FixedText>("nameft");
        pLabel->set_mnemonic_widget(m_pNameRo);
    }

    if ( pStyle->HasFollowSupport() && pPool )
    {
        SfxStyleSheetBase* pPoolStyle = pPool->First();

        while ( pPoolStyle )
        {
            m_pFollowLb->InsertEntry( pPoolStyle->GetName() );
            pPoolStyle = pPool->Next();
        }

        // A new Template is not yet in the Pool
        if ( LISTBOX_ENTRY_NOTFOUND == m_pFollowLb->GetEntryPos( pStyle->GetName() ) )
            m_pFollowLb->InsertEntry( pStyle->GetName() );
    }
    else
    {
        m_pFollowFt->Hide();
        m_pFollowLb->Hide();
    }

    if ( pStyle->HasParentSupport() && pPool )
    {
        if ( pStyle->HasClearParentSupport() )
            // the base template can be set to NULL
            m_pBaseLb->InsertEntry( SfxResId(STR_NONE).toString() );

        SfxStyleSheetBase* pPoolStyle = pPool->First();

        while ( pPoolStyle )
        {
            const String aStr( pPoolStyle->GetName() );
            // own name as base template
            if ( aStr != aName )
                m_pBaseLb->InsertEntry( aStr );
            pPoolStyle = pPool->Next();
        }
    }
    else
    {
        m_pBaseFt->Disable();
        m_pBaseLb->Disable();
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
                m_pFilterLb->InsertEntry( pTupel->aName, nIdx );
                m_pFilterLb->SetEntryData(nIdx, (void*)(sal_IntPtr)i);

                if ( ( pTupel->nFlags & nMask ) == nMask )
                    nStyleFilterIdx = nIdx;
                ++nIdx;
            }
        }

        if ( nStyleFilterIdx != 0xFFFF )
            m_pFilterLb->SelectEntryPos( nStyleFilterIdx );
    }

    if ( !m_pFilterLb->GetEntryCount() || !pStyle->IsUserDefined() )
    {
        pItem = 0;
        m_pFilterFt->Disable();
        m_pFilterLb->Disable();
    }
    else
        m_pFilterLb->SaveValue();
    SetDescriptionText_Impl();

    if ( m_pFollowLb->IsEnabled() || m_pBaseLb->IsEnabled() )
    {
        m_pNameRw->SetGetFocusHdl(
            LINK( this, SfxManageStyleSheetPage, GetFocusHdl ) );
        m_pNameRw->SetLoseFocusHdl(
            LINK( this, SfxManageStyleSheetPage, LoseFocusHdl ) );
    }
    // It is a style with auto update? (SW only)
    if(SFX_ITEM_SET == rAttrSet.GetItemState(SID_ATTR_AUTO_STYLE_UPDATE))
        m_pAutoCB->Show();
}

//-------------------------------------------------------------------------

SfxManageStyleSheetPage::~SfxManageStyleSheetPage()

/*  [Description]

    Destructor, release of the data
*/

{
    m_pNameRw->SetGetFocusHdl( Link() );
    m_pNameRw->SetLoseFocusHdl( Link() );
    delete pFamilies;
    pItem = 0;
    pStyle = 0;

}

//-------------------------------------------------------------------------

void SfxManageStyleSheetPage::UpdateName_Impl( ListBox* pBox,
                                               const OUString& rNew )

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
    m_pDescFt->SetText( pStyle->GetDescription( eUnit ) );
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
        UpdateName_Impl(m_pFollowLb, aStr);
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
    const sal_uInt16 nFilterIdx = m_pFilterLb->GetSelectEntryPos();

    // Set Filter

    if ( LISTBOX_ENTRY_NOTFOUND  != nFilterIdx      &&
         nFilterIdx != m_pFilterLb->GetSavedValue()    &&
         m_pFilterLb->IsEnabled() )
    {
        bModified = sal_True;
        OSL_ENSURE( pItem, "No Item" );
        // is only possibly for user templates
        sal_uInt16 nMask = pItem->GetFilterList()[ (size_t)m_pFilterLb->GetEntryData( nFilterIdx ) ]->nFlags | SFXSTYLEBIT_USERDEF;
        pStyle->SetMask( nMask );
    }
    if(m_pAutoCB->IsVisible() &&
        m_pAutoCB->IsChecked() != m_pAutoCB->GetSavedValue())
    {
        rSet.Put(SfxBoolItem(SID_ATTR_AUTO_STYLE_UPDATE, m_pAutoCB->IsChecked()));
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
    m_pNameRw->SetText( aName );

    if ( m_pFollowLb->IsEnabled() )
    {
        sCmp = pStyle->GetFollow();

        if ( sCmp != aFollow )
            pStyle->SetFollow( aFollow );

        if ( aFollow.isEmpty() )
            m_pFollowLb->SelectEntry( aName );
        else
            m_pFollowLb->SelectEntry( aFollow );
    }

    if ( m_pBaseLb->IsEnabled() )
    {
        sCmp = pStyle->GetParent();

        if ( sCmp != aParent )
            pStyle->SetParent( aParent );

        if ( aParent.isEmpty() )
            m_pBaseLb->SelectEntry( SfxResId(STR_NONE).toString() );
        else
            m_pBaseLb->SelectEntry( aParent );

        if ( SfxResId(STR_STANDARD).toString().equals(aName) )
        {
            // the default template can not be linked
            m_pBaseFt->Disable();
            m_pBaseLb->Disable();
        }
    }

    if ( m_pFilterLb->IsEnabled() )
    {
        sal_uInt16 nCmp = pStyle->GetMask();

        if ( nCmp != nFlags )
            pStyle->SetMask( nFlags );
        m_pFilterLb->SelectEntryPos( m_pFilterLb->GetSavedValue() );
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

    ActivatePage handler of SfxTabDialog, is used for the update of the
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
        m_pAutoCB->Check( ( (const SfxBoolItem*)pPoolItem )->GetValue() );
    m_pAutoCB->SaveValue();
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

    if ( m_pNameRw->IsModified() )
    {
        // By pressing <Enter> LoseFocus() is not trigged through StarView
        if ( m_pNameRw->HasFocus() )
            LoseFocusHdl( m_pNameRw );

        if (!pStyle->SetName(comphelper::string::stripStart(m_pNameRw->GetText(), ' ')))
        {
            InfoBox aBox( this, SfxResId( MSG_TABPAGE_INVALIDNAME ) );
            aBox.Execute();
            m_pNameRw->GrabFocus();
            m_pNameRw->SetSelection( Selection( SELECTION_MIN, SELECTION_MAX ) );
            return SfxTabPage::KEEP_PAGE;
        }
        bModified = sal_True;
    }

    if ( pStyle->HasFollowSupport() && m_pFollowLb->IsEnabled() )
    {
        const OUString aFollowEntry( m_pFollowLb->GetSelectEntry() );

        if ( pStyle->GetFollow() != aFollowEntry )
        {
            if ( !pStyle->SetFollow( aFollowEntry ) )
            {
                InfoBox aBox( this, SfxResId( MSG_TABPAGE_INVALIDSTYLE ) );
                aBox.Execute();
                m_pFollowLb->GrabFocus();
                return SfxTabPage::KEEP_PAGE;
            }
            bModified = sal_True;
        }
    }

    if ( m_pBaseLb->IsEnabled() )
    {
        OUString aParentEntry( m_pBaseLb->GetSelectEntry() );

        if ( SfxResId(STR_NONE).toString().equals(aParentEntry) || aParentEntry == pStyle->GetName() )
            aParentEntry = OUString();

        if ( pStyle->GetParent() != aParentEntry )
        {
            if ( !pStyle->SetParent( aParentEntry ) )
            {
                InfoBox aBox( this, SfxResId( MSG_TABPAGE_INVALIDPARENT ) );
                aBox.Execute();
                m_pBaseLb->GrabFocus();
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
