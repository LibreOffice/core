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
#include <vcl/weld.hxx>
#include <vcl/field.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svl/style.hxx>
#include <osl/diagnose.h>

#include <sfx2/styfitem.hxx>
#include <sfx2/styledlg.hxx>
#include <sfx2/tabdlg.hxx>
#include <sfx2/app.hxx>
#include <sfx2/mgetempl.hxx>
#include <sfx2/objsh.hxx>
#include <sfxtypes.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/module.hxx>

#include <sfx2/templdlg.hxx>
#include <templdgi.hxx>
#include <sfx2/strings.hrc>

#include <svl/stritem.hxx>
#include <sfx2/dispatch.hxx>

/*  SfxManageStyleSheetPage Constructor
 *
 *  initializes the list box with the templates
 */
SfxManageStyleSheetPage::SfxManageStyleSheetPage(TabPageParent pParent, const SfxItemSet& rAttrSet)
    : SfxTabPage(pParent, "sfx/ui/managestylepage.ui", "ManageStylePage", &rAttrSet)
    , pStyle(&static_cast<SfxStyleDialogController*>(pParent.pController)->GetStyleSheet())
    , pItem(nullptr)
    , bModified(false)
    , aName(pStyle->GetName())
    , aFollow(pStyle->GetFollow())
    , aParent(pStyle->GetParent())
    , nFlags(pStyle->GetMask())
    , m_xName(m_xBuilder->weld_entry("name"))
    , m_xAutoCB(m_xBuilder->weld_check_button("autoupdate"))
    , m_xFollowFt(m_xBuilder->weld_label("nextstyleft"))
    , m_xFollowLb(m_xBuilder->weld_combo_box("nextstyle"))
    , m_xEditStyleBtn(m_xBuilder->weld_button("editstyle"))
    , m_xBaseFt(m_xBuilder->weld_label("linkedwithft"))
    , m_xBaseLb(m_xBuilder->weld_combo_box("linkedwith"))
    , m_xEditLinkStyleBtn(m_xBuilder->weld_button("editlinkstyle"))
    , m_xFilterFt(m_xBuilder->weld_label("categoryft"))
    , m_xFilterLb(m_xBuilder->weld_combo_box("category"))
    , m_xDescFt(m_xBuilder->weld_label("desc"))
    , m_xNameFt(m_xBuilder->weld_label("nameft"))
{
    m_xFollowLb->make_sorted();
    // tdf#120188 like SwCharURLPage limit the width of the style combos
    const int nMaxWidth(m_xFollowLb->get_approximate_digit_width() * 50);
    m_xFollowLb->set_size_request(nMaxWidth , -1);
    m_xBaseLb->make_sorted();
    m_xBaseLb->set_size_request(nMaxWidth , -1);
    //note that the code depends on categories not being lexically
    //sorted, so if its changed to sorted, the code needs to
    //be adapted to be position unaware
    m_xFilterLb->set_size_request(nMaxWidth , -1);

    // this Page needs ExchangeSupport
    SetExchangeSupport();

    if ( aFollow == aName )
        m_xEditStyleBtn->set_sensitive(false);
    else
        m_xEditStyleBtn->set_sensitive(true);

    int linkSelectPos = m_xBaseLb->get_active();
    if ( linkSelectPos == 0 )
        m_xEditLinkStyleBtn->set_sensitive(false);
    else
        m_xEditLinkStyleBtn->set_sensitive(true);

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
        OUString aNoName(SfxStyleDialogController::GenerateUnusedName(*pPool));
        pStyle->SetName( aNoName );
        aName = aNoName;
        aFollow = pStyle->GetFollow();
        aParent = pStyle->GetParent();
    }
    m_xName->set_text(pStyle->GetName());

    // Set the field read-only if it is NOT an user-defined style
    // but allow selecting and copying
    if (pStyle->IsUserDefined())
    {
        m_xName->set_can_focus(true);
        m_xName->set_editable(true);
    }

    if ( pStyle->HasFollowSupport() && pPool )
    {
        SfxStyleSheetBase* pPoolStyle = pPool->First();

        m_xFollowLb->freeze();

        while ( pPoolStyle )
        {
            m_xFollowLb->append_text(pPoolStyle->GetName());
            pPoolStyle = pPool->Next();
        }

        // A new Template is not yet in the Pool
        if (m_xFollowLb->find_text(pStyle->GetName()) == -1)
            m_xFollowLb->append_text(pStyle->GetName());

        m_xFollowLb->thaw();
    }
    else
    {
        m_xFollowFt->set_sensitive(false);
        m_xFollowFt->hide();
        m_xFollowLb->set_sensitive(false);
        m_xFollowLb->hide();
        m_xEditStyleBtn->hide();
    }

    if ( pStyle->HasParentSupport() && pPool )
    {
        m_xBaseLb->freeze();

        if ( pStyle->HasClearParentSupport() )
            // the base template can be set to NULL
            m_xBaseLb->append_text(SfxResId(STR_NONE));

        SfxStyleSheetBase* pPoolStyle = pPool->First();

        while ( pPoolStyle )
        {
            const OUString aStr( pPoolStyle->GetName() );
            // own name as base template
            if ( aStr != aName )
                m_xBaseLb->append_text(aStr);
            pPoolStyle = pPool->Next();
        }

        m_xBaseLb->thaw();
    }
    else
    {
        m_xBaseFt->set_sensitive(false);
        m_xBaseLb->set_sensitive(false);
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
        SfxStyleSearchBits nMask = pStyle->GetMask() & ~SfxStyleSearchBits::UserDefined;

        if ( nMask == SfxStyleSearchBits::Auto )   // User Template?
            nMask = pStyle->GetMask();

        for ( i = 0; i < nCount; ++i )
        {
            const SfxFilterTuple& rTupel = rList[ i ];

            if ( rTupel.nFlags != SfxStyleSearchBits::Auto     &&
                 rTupel.nFlags != SfxStyleSearchBits::Used     &&
                 rTupel.nFlags != SfxStyleSearchBits::AllVisible &&
                 rTupel.nFlags != SfxStyleSearchBits::All )
            {
                OUString sId(OUString::number(i));
                m_xFilterLb->insert(nIdx, rTupel.aName, &sId, nullptr, nullptr);
                if ( ( rTupel.nFlags & nMask ) == nMask )
                    nStyleFilterIdx = nIdx;
                ++nIdx;
            }
        }

        if ( nStyleFilterIdx != 0xFFFF )
            m_xFilterLb->set_active(nStyleFilterIdx);
    }

    if ( !m_xFilterLb->get_count() || !pStyle->IsUserDefined() )
    {
        pItem = nullptr;
        m_xFilterFt->set_sensitive(false);
        m_xFilterLb->set_sensitive(false);
    }
    else
        m_xFilterLb->save_value();
    SetDescriptionText_Impl();

    if (m_xFollowLb->get_sensitive() || m_xBaseLb->get_sensitive())
    {
        m_xName->connect_focus_in(
            LINK( this, SfxManageStyleSheetPage, GetFocusHdl ) );
        m_xName->connect_focus_out(
            LINK( this, SfxManageStyleSheetPage, LoseFocusHdl ) );
    }
    // It is a style with auto update? (SW only)
    if(SfxItemState::SET == rAttrSet.GetItemState(SID_ATTR_AUTO_STYLE_UPDATE))
        m_xAutoCB->show();
    m_xFollowLb->connect_changed(LINK(this, SfxManageStyleSheetPage, EditStyleSelectHdl_Impl));
    m_xBaseLb->connect_changed(LINK(this, SfxManageStyleSheetPage, EditLinkStyleSelectHdl_Impl));
    m_xEditStyleBtn->connect_clicked(LINK(this, SfxManageStyleSheetPage, EditStyleHdl_Impl));
    m_xEditLinkStyleBtn->connect_clicked(LINK(this, SfxManageStyleSheetPage, EditLinkStyleHdl_Impl));
}

SfxManageStyleSheetPage::~SfxManageStyleSheetPage()
{
    disposeOnce();
}

void SfxManageStyleSheetPage::dispose()
{
    pFamilies.reset();
    pItem = nullptr;
    pStyle = nullptr;
    SfxTabPage::dispose();
}

void SfxManageStyleSheetPage::UpdateName_Impl( weld::ComboBox* pBox,
                                               const OUString& rNew )

/*  [Description]

    After the change of a template name update the ListBox pBox

    [Parameter]

    ListBox* pBox         ListBox, whose entries are to be updated
    const String& rNew    the new Name
*/

{
    if (pBox->get_sensitive())
    {
        // it is the current entry, which name was modified
        const bool bSelect = pBox->get_active_text() == aBuf;
        pBox->remove_text(aBuf);
        pBox->append_text(rNew);

        if (bSelect)
            pBox->set_active_text(rNew);
    }
}

void SfxManageStyleSheetPage::SetDescriptionText_Impl()

/*  [Description]

    Set attribute description. Get the set metric for this.
*/

{
    MapUnit eUnit = MapUnit::MapCM;
    FieldUnit eFieldUnit( FieldUnit::CM );
    SfxModule* pModule = SfxModule::GetActiveModule();
    if ( pModule )
    {
        const SfxPoolItem* pPoolItem = pModule->GetItem( SID_ATTR_METRIC );
        if ( pPoolItem )
            eFieldUnit = static_cast<FieldUnit>(static_cast<const SfxUInt16Item*>( pPoolItem )->GetValue());
    }

    switch ( eFieldUnit )
    {
        case FieldUnit::MM:      eUnit = MapUnit::MapMM; break;
        case FieldUnit::CM:
        case FieldUnit::M:
        case FieldUnit::KM:      eUnit = MapUnit::MapCM; break;
        case FieldUnit::POINT:
        case FieldUnit::PICA:    eUnit = MapUnit::MapPoint; break;
        case FieldUnit::INCH:
        case FieldUnit::FOOT:
        case FieldUnit::MILE:    eUnit = MapUnit::MapInch; break;

        default:
            OSL_FAIL( "non supported field unit" );
    }
    m_xDescFt->set_label(pStyle->GetDescription(eUnit));
}

IMPL_LINK_NOARG(SfxManageStyleSheetPage, EditStyleSelectHdl_Impl, weld::ComboBox&, void)
{
    OUString aTemplName(m_xFollowLb->get_active_text());
    OUString aEditTemplName(m_xName->get_text());
    m_xEditStyleBtn->set_sensitive(aTemplName != aEditTemplName);
}

IMPL_LINK_NOARG(SfxManageStyleSheetPage, EditStyleHdl_Impl, weld::Button&, void)
{
    OUString aTemplName(m_xFollowLb->get_active_text());
    Execute_Impl(SID_STYLE_EDIT, aTemplName, static_cast<sal_uInt16>(pStyle->GetFamily()));
}

IMPL_LINK_NOARG(SfxManageStyleSheetPage, EditLinkStyleSelectHdl_Impl, weld::ComboBox&, void)
{
    int linkSelectPos = m_xBaseLb->get_active();
    if ( linkSelectPos == 0 )
        m_xEditLinkStyleBtn->set_sensitive(false);
    else
        m_xEditLinkStyleBtn->set_sensitive(true);
}

IMPL_LINK_NOARG(SfxManageStyleSheetPage, EditLinkStyleHdl_Impl, weld::Button&, void)
{
    OUString aTemplName(m_xBaseLb->get_active_text());
    if (aTemplName != SfxResId(STR_NONE))
        Execute_Impl( SID_STYLE_EDIT, aTemplName, static_cast<sal_uInt16>(pStyle->GetFamily()) );
}

// Internal: Perform functions through the Dispatcher
bool SfxManageStyleSheetPage::Execute_Impl(
    sal_uInt16 nId, const OUString &rStr, sal_uInt16 nFamily)
{

    SfxDispatcher &rDispatcher = *SfxGetpApp()->GetDispatcher_Impl();
    SfxStringItem aItem(nId, rStr);
    SfxUInt16Item aFamily(SID_STYLE_FAMILY, nFamily);
    const SfxPoolItem* pItems[ 6 ];
    sal_uInt16 nCount = 0;
    if( !rStr.isEmpty() )
        pItems[ nCount++ ] = &aItem;
    pItems[ nCount++ ] = &aFamily;

    pItems[ nCount++ ] = nullptr;

    const SfxPoolItem* pItem = rDispatcher.Execute(
        nId, SfxCallMode::SYNCHRON | SfxCallMode::RECORD,
        pItems );

    return pItem != nullptr;

}

IMPL_LINK(SfxManageStyleSheetPage, GetFocusHdl, weld::Widget&, rControl, void)

/*  [Description]

    StarView Handler; GetFocus-Handler of the Edits with the template name.
*/

{
    weld::Entry& rEdit = dynamic_cast<weld::Entry&>(rControl);
    aBuf = comphelper::string::stripStart(rEdit.get_text(), ' ');
}

IMPL_LINK(SfxManageStyleSheetPage, LoseFocusHdl, weld::Widget&, rControl, void)

/*  [Description]

    StarView Handler; lose-focus-handler of the edits of the template name.
    This will update the listbox with the subsequent templates. The current
    template itself is not returned in the listbox of the base templates.
*/

{
    weld::Entry& rEdit = dynamic_cast<weld::Entry&>(rControl);
    const OUString aStr(comphelper::string::stripStart(rEdit.get_text(), ' '));
    rEdit.set_text(aStr);
    // Update the Listbox of the base template if possible
    if ( aStr != aBuf )
        UpdateName_Impl(m_xFollowLb.get(), aStr);
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
    const int nFilterIdx = m_xFilterLb->get_active();

    // Set Filter

    if ( nFilterIdx != -1 &&
         m_xFilterLb->get_value_changed_from_saved() &&
         m_xFilterLb->get_sensitive() )
    {
        bModified = true;
        OSL_ENSURE( pItem, "No Item" );
        // is only possibly for user templates
        SfxStyleSearchBits nMask = pItem->GetFilterList()[m_xFilterLb->get_id(nFilterIdx).toUInt32()].nFlags | SfxStyleSearchBits::UserDefined;
        pStyle->SetMask( nMask );
    }
    if (m_xAutoCB->get_visible() && m_xAutoCB->get_state_changed_from_saved())
    {
        rSet->Put(SfxBoolItem(SID_ATTR_AUTO_STYLE_UPDATE, m_xAutoCB->get_active()));
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
    m_xName->set_text( aName );
    if (m_xName->get_editable())
        m_xName->select_region(0, -1);

    if ( m_xFollowLb->get_sensitive() )
    {
        sCmp = pStyle->GetFollow();

        if ( sCmp != aFollow )
            pStyle->SetFollow( aFollow );

        if ( aFollow.isEmpty() )
            m_xFollowLb->set_active_text( aName );
        else
            m_xFollowLb->set_active_text( aFollow );
    }

    if (m_xBaseLb->get_sensitive())
    {
        sCmp = pStyle->GetParent();

        if ( sCmp != aParent )
            pStyle->SetParent( aParent );

        if ( aParent.isEmpty() )
            m_xBaseLb->set_active_text( SfxResId(STR_NONE) );
        else
            m_xBaseLb->set_active_text( aParent );

        if ( SfxResId(STR_STANDARD) == aName )
        {
            // the default template can not be linked
            m_xBaseFt->set_sensitive(false);
            m_xBaseLb->set_sensitive(false);
        }
    }

    if (m_xFilterLb->get_sensitive())
    {
        SfxStyleSearchBits nCmp = pStyle->GetMask();

        if ( nCmp != nFlags )
            pStyle->SetMask( nFlags );
        m_xFilterLb->set_active_text(m_xFilterLb->get_saved_value());
    }
}

VclPtr<SfxTabPage> SfxManageStyleSheetPage::Create( TabPageParent pParent,
                                                    const SfxItemSet *rAttrSet )
{
    return VclPtr<SfxManageStyleSheetPage>::Create(pParent, *rAttrSet);
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
        m_xAutoCB->set_active(static_cast<const SfxBoolItem*>(pPoolItem)->GetValue());
    m_xAutoCB->save_state();
    m_xName->save_value();
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

    if (m_xName->get_value_changed_from_saved())
    {
        // By pressing <Enter> LoseFocus() is not triggered through StarView
        if (m_xName->has_focus())
            LoseFocusHdl( *m_xName );

        if (!pStyle->SetName(comphelper::string::stripStart(m_xName->get_text(), ' ')))
        {
            std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                                     VclMessageType::Info, VclButtonsType::Ok,
                                                                     SfxResId(STR_TABPAGE_INVALIDNAME)));
            xBox->run();
            m_xName->grab_focus();
            m_xName->select_region(0, -1);
            return DeactivateRC::KeepPage;
        }
        bModified = true;
    }

    if (pStyle->HasFollowSupport() && m_xFollowLb->get_sensitive())
    {
        const OUString aFollowEntry( m_xFollowLb->get_active_text() );

        if ( pStyle->GetFollow() != aFollowEntry )
        {
            if ( !pStyle->SetFollow( aFollowEntry ) )
            {
                std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                                         VclMessageType::Info, VclButtonsType::Ok,
                                                                         SfxResId(STR_TABPAGE_INVALIDSTYLE)));
                xBox->run();
                m_xFollowLb->grab_focus();
                return DeactivateRC::KeepPage;
            }
            bModified = true;
        }
    }

    if (m_xBaseLb->get_sensitive())
    {
        OUString aParentEntry( m_xBaseLb->get_active_text() );

        if ( SfxResId(STR_NONE) == aParentEntry || aParentEntry == pStyle->GetName() )
            aParentEntry.clear();

        if ( pStyle->GetParent() != aParentEntry )
        {
            if ( !pStyle->SetParent( aParentEntry ) )
            {
                std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                                         VclMessageType::Info, VclButtonsType::Ok,
                                                                         SfxResId(STR_TABPAGE_INVALIDPARENT)));
                xBox->run();
                m_xBaseLb->grab_focus();
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
