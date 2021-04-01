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

#include <memory>
#include <hintids.hxx>

#include <comphelper/fileurl.hxx>
#include <svl/urihelper.hxx>
#include <svl/stritem.hxx>
#include <editeng/flstitem.hxx>
#include <sfx2/htmlmode.hxx>
#include <svl/cjkoptions.hxx>
#include <svl/macitem.hxx>
#include <osl/diagnose.h>

#include <cmdid.h>
#include <swtypes.hxx>
#include <view.hxx>
#include <docsh.hxx>
#include <uitool.hxx>
#include <fmtinfmt.hxx>
#include <macassgn.hxx>
#include <chrdlg.hxx>
#include <swmodule.hxx>
#include <poolfmt.hxx>

#include <strings.hrc>
#include <chrdlgmodes.hxx>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker3.hpp>
#include <SwStyleNameMapper.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/frame.hxx>

#include <svx/svxdlg.hxx>
#include <svx/flagsdef.hxx>
#include <svx/dialogs.hrc>

using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::sfx2;

SwCharDlg::SwCharDlg(weld::Window* pParent, SwView& rVw, const SfxItemSet& rCoreSet,
    SwCharDlgMode nDialogMode, const OUString* pStr)
    : SfxTabDialogController(pParent, "modules/swriter/ui/characterproperties.ui",
                             "CharacterPropertiesDialog", &rCoreSet, pStr != nullptr)
    , m_rView(rVw)
    , m_nDialogMode(nDialogMode)
{
    if (pStr)
    {
        m_xDialog->set_title(m_xDialog->get_title() + SwResId(STR_TEXTCOLL_HEADER) + *pStr + ")");
    }
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    AddTabPage("font", pFact->GetTabPageCreatorFunc(RID_SVXPAGE_CHAR_NAME), nullptr);
    AddTabPage("fonteffects", pFact->GetTabPageCreatorFunc(RID_SVXPAGE_CHAR_EFFECTS), nullptr);
    AddTabPage("position", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_POSITION ), nullptr );
    AddTabPage("asianlayout", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_TWOLINES ), nullptr );
    AddTabPage("hyperlink", SwCharURLPage::Create, nullptr);
    AddTabPage("background", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BKG ), nullptr );
    AddTabPage("borders", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), nullptr );

    if (m_nDialogMode == SwCharDlgMode::Draw || m_nDialogMode == SwCharDlgMode::Ann)
    {
        RemoveTabPage("hyperlink");
        RemoveTabPage("asianlayout");
    }
    else
    {
        SvtCJKOptions aCJKOptions;
        if (!aCJKOptions.IsDoubleLinesEnabled())
            RemoveTabPage("asianlayout");
    }

    if (m_nDialogMode != SwCharDlgMode::Std)
        RemoveTabPage("borders");
}

SwCharDlg::~SwCharDlg()
{
}

// set FontList
void SwCharDlg::PageCreated(const OString& rId, SfxTabPage &rPage)
{
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    if (rId == "font")
    {
        SvxFontListItem aFontListItem( *static_cast<const SvxFontListItem*>(
           ( m_rView.GetDocShell()->GetItem( SID_ATTR_CHAR_FONTLIST ) ) ) );
        aSet.Put (SvxFontListItem( aFontListItem.GetFontList(), SID_ATTR_CHAR_FONTLIST));
        if(m_nDialogMode != SwCharDlgMode::Draw && m_nDialogMode != SwCharDlgMode::Ann)
            aSet.Put (SfxUInt32Item(SID_FLAG_TYPE,SVX_PREVIEW_CHARACTER));
        rPage.PageCreated(aSet);
    }
    else if (rId == "fonteffects")
    {
        aSet.Put(
            SfxUInt32Item(SID_FLAG_TYPE, SVX_PREVIEW_CHARACTER | SVX_ENABLE_CHAR_TRANSPARENCY));
        rPage.PageCreated(aSet);
    }
    else if (rId == "position")
    {
        aSet.Put (SfxUInt32Item(SID_FLAG_TYPE,SVX_PREVIEW_CHARACTER));
        rPage.PageCreated(aSet);
    }
    else if (rId == "asianlayout")
    {
        aSet.Put (SfxUInt32Item(SID_FLAG_TYPE,SVX_PREVIEW_CHARACTER));
        rPage.PageCreated(aSet);
    }
    else if (rId == "background")
    {
        SvxBackgroundTabFlags eFlags(SvxBackgroundTabFlags::SHOW_HIGHLIGHTING);
        if (m_nDialogMode == SwCharDlgMode::Draw || m_nDialogMode == SwCharDlgMode::Ann)
            eFlags = SvxBackgroundTabFlags::SHOW_CHAR_BKGCOLOR;
        aSet.Put (SfxUInt32Item(SID_FLAG_TYPE,static_cast<sal_uInt32>(eFlags)));
        rPage.PageCreated(aSet);
    }
}

SwCharURLPage::SwCharURLPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rCoreSet)
    : SfxTabPage(pPage, pController, "modules/swriter/ui/charurlpage.ui", "CharURLPage", &rCoreSet)
    , bModified(false)
    , m_xURLED(m_xBuilder->weld_entry("urled"))
    , m_xTextFT(m_xBuilder->weld_label("textft"))
    , m_xTextED(m_xBuilder->weld_entry("texted"))
    , m_xNameED(m_xBuilder->weld_entry("nameed"))
    , m_xTargetFrameLB(m_xBuilder->weld_combo_box("targetfrmlb"))
    , m_xURLPB(m_xBuilder->weld_button("urlpb"))
    , m_xEventPB(m_xBuilder->weld_button("eventpb"))
    , m_xVisitedLB(m_xBuilder->weld_combo_box("visitedlb"))
    , m_xNotVisitedLB(m_xBuilder->weld_combo_box("unvisitedlb"))
    , m_xCharStyleContainer(m_xBuilder->weld_widget("charstyle"))
{
    // tdf#120188 like SfxManageStyleSheetPage limit the width of the style combos
    const int nMaxWidth(m_xVisitedLB->get_approximate_digit_width() * 50);
    m_xVisitedLB->set_size_request(nMaxWidth , -1);
    m_xNotVisitedLB->set_size_request(nMaxWidth , -1);

    const SfxPoolItem* pItem;
    SfxObjectShell* pShell;
    if(SfxItemState::SET == rCoreSet.GetItemState(SID_HTML_MODE, false, &pItem) ||
        ( nullptr != ( pShell = SfxObjectShell::Current()) &&
                    nullptr != (pItem = pShell->GetItem(SID_HTML_MODE))))
    {
        sal_uInt16 nHtmlMode = static_cast<const SfxUInt16Item*>(pItem)->GetValue();
        if (HTMLMODE_ON & nHtmlMode)
            m_xCharStyleContainer->hide();
    }

    m_xURLPB->connect_clicked(LINK( this, SwCharURLPage, InsertFileHdl));
    m_xEventPB->connect_clicked(LINK( this, SwCharURLPage, EventHdl));

    SwView *pView = ::GetActiveView();
    ::FillCharStyleListBox(*m_xVisitedLB, pView->GetDocShell());
    ::FillCharStyleListBox(*m_xNotVisitedLB, pView->GetDocShell());
    m_xVisitedLB->set_active_id(OUString::number(RES_POOLCHR_INET_VISIT));
    m_xVisitedLB->save_value();
    m_xNotVisitedLB->set_active_id(OUString::number(RES_POOLCHR_INET_NORMAL));
    m_xNotVisitedLB->save_value();

    std::unique_ptr<TargetList> pList( new TargetList );
    SfxFrame::GetDefaultTargetList(*pList);

    m_xTargetFrameLB->freeze();
    size_t nCount = pList->size();
    for (size_t i = 0; i < nCount; ++i)
    {
        m_xTargetFrameLB->append_text(pList->at(i));
    }
    m_xTargetFrameLB->thaw();
}

SwCharURLPage::~SwCharURLPage()
{
    pINetItem.reset();
}

void SwCharURLPage::Reset(const SfxItemSet* rSet)
{
    const SfxPoolItem* pItem;
    if (SfxItemState::SET == rSet->GetItemState(RES_TXTATR_INETFMT, false, &pItem))
    {
        const SwFormatINetFormat* pINetFormat = static_cast<const SwFormatINetFormat*>( pItem);
        m_xURLED->set_text(INetURLObject::decode(pINetFormat->GetValue(),
            INetURLObject::DecodeMechanism::Unambiguous));
        m_xURLED->save_value();
        m_xNameED->set_text(pINetFormat->GetName());
        m_xNameED->save_value();

        OUString sEntry = pINetFormat->GetVisitedFormat();
        if (sEntry.isEmpty())
        {
            OSL_ENSURE( false, "<SwCharURLPage::Reset(..)> - missing visited character format at hyperlink attribute" );
            SwStyleNameMapper::FillUIName(RES_POOLCHR_INET_VISIT, sEntry);
        }
        m_xVisitedLB->set_active_text(sEntry);

        sEntry = pINetFormat->GetINetFormat();
        if (sEntry.isEmpty())
        {
            OSL_ENSURE( false, "<SwCharURLPage::Reset(..)> - missing unvisited character format at hyperlink attribute" );
            SwStyleNameMapper::FillUIName(RES_POOLCHR_INET_NORMAL, sEntry);
        }
        m_xNotVisitedLB->set_active_text(sEntry);

        m_xTargetFrameLB->set_entry_text(pINetFormat->GetTargetFrame());
        m_xVisitedLB->save_value();
        m_xNotVisitedLB->save_value();
        m_xTargetFrameLB->save_value();
        pINetItem.reset( new SvxMacroItem(FN_INET_FIELD_MACRO) );

        if( pINetFormat->GetMacroTable() )
            pINetItem->SetMacroTable(*pINetFormat->GetMacroTable());
    }
    if (SfxItemState::SET == rSet->GetItemState(FN_PARAM_SELECTION, false, &pItem))
    {
        m_xTextED->set_text(static_cast<const SfxStringItem*>(pItem)->GetValue());
        m_xTextFT->set_sensitive(false);
        m_xTextED->set_sensitive(false);
    }
}

bool SwCharURLPage::FillItemSet(SfxItemSet* rSet)
{
    OUString sURL = m_xURLED->get_text();
    if (!sURL.isEmpty())
    {
        sURL = URIHelper::SmartRel2Abs(INetURLObject(), sURL, Link<OUString *, bool>(), false );
        // #i100683# file URLs should be normalized in the UI
        if ( comphelper::isFileUrl(sURL) )
            sURL = URIHelper::simpleNormalizedMakeRelative(OUString(), sURL);
    }

    SwFormatINetFormat aINetFormat(sURL, m_xTargetFrameLB->get_active_text());
    aINetFormat.SetName(m_xNameED->get_text());
    bool bURLModified = m_xURLED->get_value_changed_from_saved();
    bool bNameModified = m_xNameED->get_value_changed_from_saved();
    bool bTargetModified = m_xTargetFrameLB->get_value_changed_from_saved();
    bModified = bURLModified || bNameModified || bTargetModified;

    // set valid settings first
    OUString sEntry = m_xVisitedLB->get_active_text();
    sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName( sEntry, SwGetPoolIdFromName::ChrFmt);
    aINetFormat.SetVisitedFormatAndId( sEntry, nId );

    sEntry = m_xNotVisitedLB->get_active_text();
    nId = SwStyleNameMapper::GetPoolIdFromUIName( sEntry, SwGetPoolIdFromName::ChrFmt);
    aINetFormat.SetINetFormatAndId( sEntry, nId );

    if (pINetItem && !pINetItem->GetMacroTable().empty())
        aINetFormat.SetMacroTable(&pINetItem->GetMacroTable());

    if (m_xVisitedLB->get_value_changed_from_saved())
        bModified = true;

    if (m_xNotVisitedLB->get_value_changed_from_saved())
        bModified = true;

    if (bNameModified)
    {
        bModified = true;
        rSet->Put(SfxStringItem(FN_PARAM_SELECTION, m_xTextED->get_text()));
    }
    if(bModified)
        rSet->Put(aINetFormat);
    return bModified;
}

std::unique_ptr<SfxTabPage> SwCharURLPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet)
{
    return std::make_unique<SwCharURLPage>(pPage, pController, *rAttrSet);
}

IMPL_LINK_NOARG(SwCharURLPage, InsertFileHdl, weld::Button&, void)
{
    FileDialogHelper aDlgHelper(TemplateDescription::FILEOPEN_SIMPLE,
                                FileDialogFlags::NONE, GetFrameWeld());
    if( aDlgHelper.Execute() == ERRCODE_NONE )
    {
        const Reference<XFilePicker3>& xFP = aDlgHelper.GetFilePicker();
        m_xURLED->set_text(xFP->getSelectedFiles().getConstArray()[0]);
    }
}

IMPL_LINK_NOARG(SwCharURLPage, EventHdl, weld::Button&, void)
{
    bModified |= SwMacroAssignDlg::INetFormatDlg(GetFrameWeld(),
                    ::GetActiveView()->GetWrtShell(), pINetItem);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
