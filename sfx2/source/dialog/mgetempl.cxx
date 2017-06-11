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
#include <vcl/layout.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/field.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svl/style.hxx>

#include <sfx2/styfitem.hxx>
#include <sfx2/styledlg.hxx>
#include <sfx2/tabdlg.hxx>
#include <sfx2/app.hxx>
#include <sfx2/mgetempl.hxx>
#include <sfx2/objsh.hxx>
#include "sfxtypes.hxx"
#include <sfx2/sfxresid.hxx>
#include <sfx2/module.hxx>

#include <sfx2/templdlg.hxx>
#include "templdgi.hxx"
#include <sfx2/sfx.hrc>
#include "sfx2/strings.hrc"

#include <svl/style.hrc>
#include <svl/stritem.hxx>
#include <sfx2/dispatch.hxx>

/*  SfxManageStyleSheetPage Constructor
 *
 *  initializes the list box with the templates
 */
SfxManageStyleSheetPage::SfxManageStyleSheetPage(vcl::Window* pParent, const SfxItemSet& rAttrSet)
    : SfxTabPage(pParent, "ManageStylePage", "sfx/ui/managestylepage.ui", &rAttrSet)
    , pStyle(&static_cast<SfxStyleDialog*>(GetParentDialog())->GetStyleSheet())
    , pItem(nullptr)
    , bModified(false)
    , aName(pStyle->GetName())
    , aFollow(pStyle->GetFollow())
    , aParent(pStyle->GetParent())
    , nFlags(pStyle->GetMask())
{
    get(m_pNameRo, "namero");
    get(m_pNameRw, "namerw");
    m_pNameRo->set_width_request(m_pNameRw->get_preferred_size().Width());
    get(m_pAutoCB, "autoupdate");
    get(m_pFollowFt, "nextstyleft");
    get(m_pFollowLb, "nextstyle");
    m_pFollowLb->SetStyle(m_pFollowLb->GetStyle() | WB_SORT);
    const sal_Int32 nMaxWidth(62);
    m_pFollowLb->setMaxWidthChars(nMaxWidth);
    get(m_pEditStyleBtn, "editstyle");
    get(m_pBaseFt, "linkedwithft");
    get(m_pBaseLb, "linkedwith");
    get(m_pEditLinkStyleBtn, "editlinkstyle");
    m_pBaseLb->SetStyle(m_pBaseLb->GetStyle() | WB_SORT);
    m_pBaseLb->setMaxWidthChars(nMaxWidth);
    get(m_pFilterFt, "categoryft");
    get(m_pFilterLb, "category");
    //note that the code depends on categories not being lexically
    //sorted, so if its changed to sorted, the code needs to
    //be adapted to be position unaware
    m_pFilterLb->setMaxWidthChars(nMaxWidth);
    get(m_pDescFt, "desc");

    // this Page needs ExchangeSupport
    SetExchangeSupport();

    if ( aFollow == aName )
        m_pEditStyleBtn->Disable();
    else
        m_pEditStyleBtn->Enable();

    sal_Int32 linkSelectPos = m_pBaseLb->GetSelectEntryPos();
    if ( linkSelectPos == 0 )
        m_pEditLinkStyleBtn->Disable();
    else
        m_pEditLinkStyleBtn->Enable();

    pFamilies = SfxApplication::GetModule_Impl()->CreateStyleFamilies();

    SfxStyleSheetBasePool* pPool = nullptr;
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
        OUString aNoName(SfxStyleDialog::GenerateUnusedName(*pPool));
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
            m_pBaseLb->InsertEntry( SfxResId(STR_NONE) );

        SfxStyleSheetBase* pPoolStyle = pPool->First();

        while ( pPoolStyle )
        {
            const OUString aStr( pPoolStyle->GetName() );
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
        pItem = &(pFamilies->at(i));

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
            const SfxFilterTupel& rTupel = rList[ i ];

            if ( rTupel.nFlags != SFXSTYLEBIT_AUTO     &&
                 rTupel.nFlags != SFXSTYLEBIT_USED     &&
                 rTupel.nFlags != SFXSTYLEBIT_ALL_VISIBLE &&
                 rTupel.nFlags != SFXSTYLEBIT_ALL )
            {
                m_pFilterLb->InsertEntry( rTupel.aName, nIdx );
                m_pFilterLb->SetEntryData(nIdx, reinterpret_cast<void*>(i));

                if ( ( rTupel.nFlags & nMask ) == nMask )
                    nStyleFilterIdx = nIdx;
                ++nIdx;
            }
        }

        if ( nStyleFilterIdx != 0xFFFF )
            m_pFilterLb->SelectEntryPos( nStyleFilterIdx );
    }

    if ( !m_pFilterLb->GetEntryCount() || !pStyle->IsUserDefined() )
    {
        pItem = nullptr;
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
    if(SfxItemState::SET == rAttrSet.GetItemState(SID_ATTR_AUTO_STYLE_UPDATE))
        m_pAutoCB->Show();
    m_pFollowLb->SetSelectHdl( LINK( this, SfxManageStyleSheetPage, EditStyleSelectHdl_Impl ) );
    m_pBaseLb->SetSelectHdl( LINK( this, SfxManageStyleSheetPage, EditLinkStyleSelectHdl_Impl ) );
    m_pEditStyleBtn->SetClickHdl( LINK( this, SfxManageStyleSheetPage, EditStyleHdl_Impl ) );
    m_pEditLinkStyleBtn->SetClickHdl( LINK( this, SfxManageStyleSheetPage, EditLinkStyleHdl_Impl ) );
}


SfxManageStyleSheetPage::~SfxManageStyleSheetPage()
{
    disposeOnce();
}

void SfxManageStyleSheetPage::dispose()
{
    m_pNameRw->SetGetFocusHdl( Link<Control&,void>() );
    m_pNameRw->SetLoseFocusHdl( Link<Control&,void>() );
    delete pFamilies;
    pItem = nullptr;
    pStyle = nullptr;
    m_pNameRo.clear();
    m_pNameRw.clear();
    m_pAutoCB.clear();
    m_pFollowFt.clear();
    m_pFollowLb.clear();
    m_pEditStyleBtn.clear();
    m_pBaseFt.clear();
    m_pBaseLb.clear();
    m_pEditLinkStyleBtn.clear();
    m_pFilterFt.clear();
    m_pFilterLb.clear();
    m_pDescFt.clear();
    SfxTabPage::dispose();
}


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
        const bool bSelect = pBox->GetSelectEntry() == aBuf;
        pBox->RemoveEntry( aBuf );
        pBox->InsertEntry( rNew );

        if ( bSelect )
            pBox->SelectEntry( rNew );
    }
}


void SfxManageStyleSheetPage::SetDescriptionText_Impl()

/*  [Description]

    Set attribute description. Get the set metric for this.
*/

{
    MapUnit eUnit = MapUnit::MapCM;
    FieldUnit eFieldUnit( FUNIT_CM );
    SfxModule* pModule = SfxModule::GetActiveModule();
    if ( pModule )
    {
        const SfxPoolItem* pPoolItem = pModule->GetItem( SID_ATTR_METRIC );
        if ( pPoolItem )
            eFieldUnit = (FieldUnit) static_cast<const SfxUInt16Item*>( pPoolItem )->GetValue();
    }

    switch ( eFieldUnit )
    {
        case FUNIT_MM:      eUnit = MapUnit::MapMM; break;
        case FUNIT_CM:
        case FUNIT_M:
        case FUNIT_KM:      eUnit = MapUnit::MapCM; break;
        case FUNIT_POINT:
        case FUNIT_PICA:    eUnit = MapUnit::MapPoint; break;
        case FUNIT_INCH:
        case FUNIT_FOOT:
        case FUNIT_MILE:    eUnit = MapUnit::MapInch; break;

        default:
            OSL_FAIL( "non supported field unit" );
    }
    m_pDescFt->SetText( pStyle->GetDescription( eUnit ) );
}

IMPL_LINK_NOARG( SfxManageStyleSheetPage, EditStyleSelectHdl_Impl, ListBox&, void )
{
    OUString aTemplName(m_pFollowLb->GetSelectEntry());
    OUString aEditTemplName(m_pNameRo->GetText());
    if (!( aTemplName == aEditTemplName))
        m_pEditStyleBtn->Enable();
    else
        m_pEditStyleBtn->Disable();
}

IMPL_LINK_NOARG( SfxManageStyleSheetPage, EditStyleHdl_Impl, Button*, void )
{
    OUString aTemplName(m_pFollowLb->GetSelectEntry());
    if (Execute_Impl( SID_STYLE_EDIT, aTemplName, OUString(),(sal_uInt16)pStyle->GetFamily() ))
    {
    }
}

IMPL_LINK_NOARG( SfxManageStyleSheetPage, EditLinkStyleSelectHdl_Impl, ListBox&, void )
{
    sal_Int32 linkSelectPos = m_pBaseLb->GetSelectEntryPos();
    if ( linkSelectPos == 0 )
        m_pEditLinkStyleBtn->Disable();
    else
        m_pEditLinkStyleBtn->Enable();
}

IMPL_LINK_NOARG( SfxManageStyleSheetPage, EditLinkStyleHdl_Impl, Button*, void )
{
    OUString aTemplName(m_pBaseLb->GetSelectEntry());
    if (aTemplName != SfxResId(STR_NONE))
        Execute_Impl( SID_STYLE_EDIT, aTemplName, OUString(),(sal_uInt16)pStyle->GetFamily() );
}

// Internal: Perform functions through the Dispatcher
bool SfxManageStyleSheetPage::Execute_Impl(
    sal_uInt16 nId, const OUString &rStr, const OUString& rRefStr, sal_uInt16 nFamily,
    sal_uInt16 nMask)
{

    SfxDispatcher &rDispatcher = *SfxGetpApp()->GetDispatcher_Impl();
    SfxStringItem aItem(nId, rStr);
    SfxUInt16Item aFamily(SID_STYLE_FAMILY, nFamily);
    SfxUInt16Item aMask( SID_STYLE_MASK, nMask );
    SfxStringItem aRefName( SID_STYLE_REFERENCE, rRefStr );
    const SfxPoolItem* pItems[ 6 ];
    sal_uInt16 nCount = 0;
    if( !rStr.isEmpty() )
        pItems[ nCount++ ] = &aItem;
    pItems[ nCount++ ] = &aFamily;
    if( nMask )
        pItems[ nCount++ ] = &aMask;
    if ( !rRefStr.isEmpty() )
        pItems[ nCount++ ] = &aRefName;

    pItems[ nCount++ ] = nullptr;

    const SfxPoolItem* pItem = rDispatcher.Execute(
        nId, SfxCallMode::SYNCHRON | SfxCallMode::RECORD | SfxCallMode::MODAL,
        pItems );

    return pItem != nullptr;

}

IMPL_LINK( SfxManageStyleSheetPage, GetFocusHdl, Control&, rControl, void )

/*  [Description]

    StarView Handler; GetFocus-Handler of the Edits with the template name.
*/

{
    Edit* pEdit = static_cast<Edit*>(&rControl);
    aBuf = comphelper::string::stripStart(pEdit->GetText(), ' ');
}

IMPL_LINK( SfxManageStyleSheetPage, LoseFocusHdl, Control&, rControl, void )

/*  [Description]

    StarView Handler; lose-focus-handler of the edits of the template name.
    This will update the listbox with the subsequent templates. The current
    template itself is not returned in the listbox of the base templates.
*/

{
    Edit* pEdit = static_cast<Edit*>(&rControl);
    const OUString aStr(comphelper::string::stripStart(pEdit->GetText(), ' '));
    pEdit->SetText( aStr );
    // Update the Listbox of the base template if possible
    if ( aStr != aBuf )
        UpdateName_Impl(m_pFollowLb, aStr);
}

bool SfxManageStyleSheetPage::FillItemSet( SfxItemSet* rSet )

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
    const sal_Int32 nFilterIdx = m_pFilterLb->GetSelectEntryPos();

    // Set Filter

    if ( LISTBOX_ENTRY_NOTFOUND  != nFilterIdx      &&
         m_pFilterLb->IsValueChangedFromSaved()    &&
         m_pFilterLb->IsEnabled() )
    {
        bModified = true;
        OSL_ENSURE( pItem, "No Item" );
        // is only possibly for user templates
        sal_uInt16 nMask = pItem->GetFilterList()[ reinterpret_cast<size_t>(m_pFilterLb->GetEntryData( nFilterIdx )) ].nFlags | SFXSTYLEBIT_USERDEF;
        pStyle->SetMask( nMask );
    }
    if(m_pAutoCB->IsVisible() &&
       m_pAutoCB->IsValueChangedFromSaved())
    {
        rSet->Put(SfxBoolItem(SID_ATTR_AUTO_STYLE_UPDATE, m_pAutoCB->IsChecked()));
    }

    return bModified;
}


void SfxManageStyleSheetPage::Reset( const SfxItemSet* /*rAttrSet*/ )

/*  [Description]

    Handler to initialize the page with the initial data.

    [Parameter]

    const SfxItemSet &rAttrSet          The data set

    [Cross-reference]

    <class SfxTabDialog>
*/

{
    bModified = false;
    OUString sCmp( pStyle->GetName() );

    if ( sCmp != aName )
        pStyle->SetName( aName );
    m_pNameRw->SetText( aName );
    m_pNameRw->SetSelection( Selection( SELECTION_MIN, SELECTION_MAX ) );

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
            m_pBaseLb->SelectEntry( SfxResId(STR_NONE) );
        else
            m_pBaseLb->SelectEntry( aParent );

        if ( SfxResId(STR_STANDARD).equals(aName) )
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


VclPtr<SfxTabPage> SfxManageStyleSheetPage::Create( vcl::Window* pParent,
                                                    const SfxItemSet *rAttrSet )
{
    return VclPtr<SfxManageStyleSheetPage>::Create( pParent, *rAttrSet );
}


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

    if ( SfxItemState::SET ==
         rSet.GetItemState( SID_ATTR_AUTO_STYLE_UPDATE, false, &pPoolItem ) )
        m_pAutoCB->Check( static_cast<const SfxBoolItem*>(pPoolItem)->GetValue() );
    m_pAutoCB->SaveValue();
}


DeactivateRC SfxManageStyleSheetPage::DeactivatePage( SfxItemSet* pItemSet )

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
    DeactivateRC nRet = DeactivateRC::LeavePage;

    if ( m_pNameRw->IsModified() )
    {
        // By pressing <Enter> LoseFocus() is not trigged through StarView
        if ( m_pNameRw->HasFocus() )
            LoseFocusHdl( *m_pNameRw );

        if (!pStyle->SetName(comphelper::string::stripStart(m_pNameRw->GetText(), ' ')))
        {
            ScopedVclPtrInstance< MessageDialog > aBox(this, SfxResId( STR_TABPAGE_INVALIDNAME ), VclMessageType::Info);
            aBox->Execute();
            m_pNameRw->GrabFocus();
            m_pNameRw->SetSelection( Selection( SELECTION_MIN, SELECTION_MAX ) );
            return DeactivateRC::KeepPage;
        }
        bModified = true;
    }

    if ( pStyle->HasFollowSupport() && m_pFollowLb->IsEnabled() )
    {
        const OUString aFollowEntry( m_pFollowLb->GetSelectEntry() );

        if ( pStyle->GetFollow() != aFollowEntry )
        {
            if ( !pStyle->SetFollow( aFollowEntry ) )
            {
                ScopedVclPtrInstance< MessageDialog > aBox(this, SfxResId( STR_TABPAGE_INVALIDSTYLE ), VclMessageType::Info);
                aBox->Execute();
                m_pFollowLb->GrabFocus();
                return DeactivateRC::KeepPage;
            }
            bModified = true;
        }
    }

    if ( m_pBaseLb->IsEnabled() )
    {
        OUString aParentEntry( m_pBaseLb->GetSelectEntry() );

        if ( SfxResId(STR_NONE).equals(aParentEntry) || aParentEntry == pStyle->GetName() )
            aParentEntry.clear();

        if ( pStyle->GetParent() != aParentEntry )
        {
            if ( !pStyle->SetParent( aParentEntry ) )
            {
                ScopedVclPtrInstance< MessageDialog > aBox(this, SfxResId( STR_TABPAGE_INVALIDPARENT ), VclMessageType::Info);
                aBox->Execute();
                m_pBaseLb->GrabFocus();
                return DeactivateRC::KeepPage;
            }
            bModified = true;
            nRet = nRet | DeactivateRC::RefreshSet;
        }
    }

    if ( pItemSet )
        FillItemSet( pItemSet );

    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
