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

#include <optcomp.hxx>

#include <cmdid.h>
#include <docsh.hxx>
#include <strings.hrc>
#include <uiitems.hxx>
#include <view.hxx>
#include <wrtsh.hxx>

#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include <IDocumentSettingAccess.hxx>
#include <vector>
#include <svtools/restartdialog.hxx>
#include <comphelper/processfactory.hxx>
#include <officecfg/Office/Compatibility.hxx>
#include <osl/diagnose.h>

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::document;

namespace
{
// Option ID, TranslateId
constexpr std::pair<OUString, TranslateId> options_list[]{
    { u"AddSpacing"_ustr, STR_COMPAT_OPT_ADDSPACING },
    { u"AddSpacingAtPages"_ustr, STR_COMPAT_OPT_ADDSPACINGATPAGES },
    { u"UseOurTabStopFormat"_ustr, STR_COMPAT_OPT_USEOURTABSTOPFORMAT },
    { u"NoExternalLeading"_ustr, STR_COMPAT_OPT_NOEXTERNALLEADING },
    { u"UseLineSpacing"_ustr, STR_COMPAT_OPT_USELINESPACING },
    { u"AddTableSpacing"_ustr, STR_COMPAT_OPT_ADDTABLESPACING },
    { u"UseObjectPositioning"_ustr, STR_COMPAT_OPT_USEOBJECTPOSITIONING },
    { u"UseOurTextWrapping"_ustr, STR_COMPAT_OPT_USEOURTEXTWRAPPING },
    { u"ConsiderWrappingStyle"_ustr, STR_COMPAT_OPT_CONSIDERWRAPPINGSTYLE },
    { u"ExpandWordSpace"_ustr, STR_COMPAT_OPT_EXPANDWORDSPACE },
    { u"ProtectForm"_ustr, STR_COMPAT_OPT_PROTECTFORM },
    { u"MsWordCompTrailingBlanks"_ustr, STR_COMPAT_OPT_MSWORDCOMPTRAILINGBLANKS },
    { u"SubtractFlysAnchoredAtFlys"_ustr, STR_COMPAT_OPT_SUBTRACTFLYSANCHOREDATFLYS },
    { u"EmptyDbFieldHidesPara"_ustr, STR_COMPAT_OPT_EMPTYDBFIELDHIDESPARA },
    { u"UseVariableWidthNBSP"_ustr, STR_COMPAT_OPT_USEVARIABLEWIDTHNBSP },
    { u"NoGapAfterNoteNumber"_ustr, STR_COMPAT_OPT_NOGAPAFTERNOTENUMBER },
    { u"TabsRelativeToIndent"_ustr, STR_COMPAT_OPT_TABSRELATIVETOINDENT },
    { u"TabOverMargin"_ustr, STR_COMPAT_OPT_TABOVERMARGIN },
};

// DocumentSettingId, negate?
std::pair<DocumentSettingId, bool> DocumentSettingForOption(const OUString& option)
{
    static const std::map<OUString, std::pair<DocumentSettingId, bool>> map{
        { u"AddSpacing"_ustr, { DocumentSettingId::PARA_SPACE_MAX, false } },
        { u"AddSpacingAtPages"_ustr, { DocumentSettingId::PARA_SPACE_MAX_AT_PAGES, false } },
        { u"UseOurTabStopFormat"_ustr, { DocumentSettingId::TAB_COMPAT, true } },
        { u"NoExternalLeading"_ustr, { DocumentSettingId::ADD_EXT_LEADING, true } },
        { u"UseLineSpacing"_ustr, { DocumentSettingId::OLD_LINE_SPACING, false } },
        { u"AddTableSpacing"_ustr, { DocumentSettingId::ADD_PARA_SPACING_TO_TABLE_CELLS, false } },
        { u"UseObjectPositioning"_ustr, { DocumentSettingId::USE_FORMER_OBJECT_POS, false } },
        { u"UseOurTextWrapping"_ustr, { DocumentSettingId::USE_FORMER_TEXT_WRAPPING, false } },
        { u"ConsiderWrappingStyle"_ustr, { DocumentSettingId::CONSIDER_WRAP_ON_OBJECT_POSITION, false } },
        { u"ExpandWordSpace"_ustr, { DocumentSettingId::DO_NOT_JUSTIFY_LINES_WITH_MANUAL_BREAK, true } },
        { u"ProtectForm"_ustr, { DocumentSettingId::PROTECT_FORM, false } },
        { u"MsWordCompTrailingBlanks"_ustr, { DocumentSettingId::MS_WORD_COMP_TRAILING_BLANKS, false } },
        { u"SubtractFlysAnchoredAtFlys"_ustr, { DocumentSettingId::SUBTRACT_FLYS, false } },
        { u"EmptyDbFieldHidesPara"_ustr, { DocumentSettingId::EMPTY_DB_FIELD_HIDES_PARA, false } },
        { u"UseVariableWidthNBSP"_ustr, { DocumentSettingId::USE_VARIABLE_WIDTH_NBSP, false } },
        { u"NoGapAfterNoteNumber"_ustr, { DocumentSettingId::NO_GAP_AFTER_NOTE_NUMBER, false } },
        { u"TabsRelativeToIndent"_ustr, { DocumentSettingId::TABS_RELATIVE_TO_INDENT, false } },
        { u"TabOverMargin"_ustr, { DocumentSettingId::TAB_OVER_MARGIN, false } },
//        { u"AddTableLineSpacing"_ustr, { DocumentSettingId::ADD_PARA_LINE_SPACING_TO_TABLE_CELLS, false } },
    };
    return map.at(option);
}
}

SwCompatibilityOptPage::SwCompatibilityOptPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, "modules/swriter/ui/optcompatpage.ui", "OptCompatPage", &rSet)
    , m_pWrtShell(nullptr)
    , m_xMain(m_xBuilder->weld_frame("compatframe"))
    , m_xOptionsLB(m_xBuilder->weld_tree_view("options"))
    , m_xDefaultPB(m_xBuilder->weld_button("default"))
{
    m_xOptionsLB->enable_toggle_buttons(weld::ColumnToggleType::Check);

    auto pos = m_xOptionsLB->make_iterator();
    for (const auto& [compatId, translateId] : options_list)
    {
        m_xOptionsLB->append(pos.get());
        m_xOptionsLB->set_id(*pos, compatId);
        m_xOptionsLB->set_text(*pos, SwResId(translateId), 0);
    }

    InitControls( rSet );

    // set handler
    m_xDefaultPB->connect_clicked( LINK( this, SwCompatibilityOptPage, UseAsDefaultHdl ) );
}

SwCompatibilityOptPage::~SwCompatibilityOptPage()
{
}

void SwCompatibilityOptPage::InitControls( const SfxItemSet& rSet )
{
    // init objectshell and detect document name
    OUString sDocTitle;
    SfxObjectShell* pObjShell = nullptr;
    if ( const SwPtrItem* pItem = rSet.GetItemIfSet( FN_PARAM_WRTSHELL, false ) )
        m_pWrtShell = static_cast<SwWrtShell*>(pItem->GetValue());
    if ( m_pWrtShell )
    {
        pObjShell = m_pWrtShell->GetView().GetDocShell();
        if ( pObjShell )
            sDocTitle = pObjShell->GetTitle();
    }
    else
    {
        m_xMain->set_sensitive(false);
    }
    const OUString& rText = m_xMain->get_label();
    m_xMain->set_label(rText.replaceAll("%DOCNAME", sDocTitle));
}

IMPL_LINK_NOARG(SwCompatibilityOptPage, UseAsDefaultHdl, weld::Button&, void)
{
    std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetFrameWeld(), "modules/swriter/ui/querydefaultcompatdialog.ui"));
    std::unique_ptr<weld::MessageDialog> xQueryBox(xBuilder->weld_message_dialog("QueryDefaultCompatDialog"));
    if (xQueryBox->run() != RET_YES)
        return;

    auto batch = comphelper::ConfigurationChanges::create();
    SvtCompatibilityDefault defaultCompatOptions(batch);

    const sal_Int32 nCount = m_xOptionsLB->n_children();
    for ( sal_Int32 i = 0; i < nCount; ++i )
    {
        OUString option = m_xOptionsLB->get_id(i);
        bool bChecked = m_xOptionsLB->get_toggle(i);
        defaultCompatOptions.set(option, bChecked);

        if (option == "AddTableSpacing")
        {
            bool const isLineSpacing = m_xOptionsLB->get_toggle(i) == TRISTATE_TRUE;
            defaultCompatOptions.set(u"AddTableLineSpacing"_ustr, isLineSpacing);
        }
        else
        {
            assert(m_xOptionsLB->get_toggle(i) != TRISTATE_INDET);
        }
    }

    batch->commit();
}

void SwCompatibilityOptPage::SetCurrentOptions()
{
    bool hasReadOnly = false;
    if (m_pWrtShell)
    {
        m_aSavedOptions.clear();
        // get document options
        const auto& rIDocumentSettingAccess = m_pWrtShell->getIDocumentSettingAccess();
        auto batch = comphelper::ConfigurationChanges::create(); // needed to obtain RO status
        const SvtCompatibilityDefault defaultCompatOptions(batch);
        const sal_Int32 nCount = m_xOptionsLB->n_children();
        for (sal_Int32 i = 0; i < nCount; ++i)
        {
            OUString option = m_xOptionsLB->get_id(i);
            const bool bReadOnly = defaultCompatOptions.getPropertyReadOnly(option);
            if (bReadOnly)
                hasReadOnly = true;
            const auto& [docSettingId, shouldNegate] = DocumentSettingForOption(option);
            bool bChecked = rIDocumentSettingAccess.get(docSettingId);
            if (shouldNegate)
                bChecked = !bChecked;
            TriState value = bChecked ? TRISTATE_TRUE : TRISTATE_FALSE;
            if (option == "AddTableSpacing")
            { // hack: map 2 bools to 1 tristate
                if (value == TRISTATE_TRUE)
                {
                    if (!rIDocumentSettingAccess.get(
                            DocumentSettingId::ADD_PARA_LINE_SPACING_TO_TABLE_CELLS))
                        value = TRISTATE_INDET; // 3 values possible here
                }
            }
            m_xOptionsLB->set_toggle(i, value);
            m_xOptionsLB->set_sensitive(i, !bReadOnly);
            m_aSavedOptions[option] = value;
        }
    }

    m_xDefaultPB->set_sensitive(!hasReadOnly);
}

std::unique_ptr<SfxTabPage> SwCompatibilityOptPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet)
{
    return std::make_unique<SwCompatibilityOptPage>(pPage, pController, *rAttrSet);
}

OUString SwCompatibilityOptPage::GetAllStrings()
{
    OUString sAllStrings;

    if (const auto& pString = m_xBuilder->weld_label(u"label11"_ustr))
        sAllStrings += pString->get_label() + " ";

    sAllStrings += m_xDefaultPB->get_label() + " ";

    return sAllStrings.replaceAll("_", "");
}

bool SwCompatibilityOptPage::FillItemSet( SfxItemSet*  )
{
    bool bModified = false;
    if ( m_pWrtShell )
    {
        const int nCount = m_xOptionsLB->n_children();
        for (int i = 0; i < nCount; ++i)
        {
            OUString option = m_xOptionsLB->get_id(i);
            TriState const current = m_xOptionsLB->get_toggle(i);
            TriState saved = m_aSavedOptions[option];
            if (current != saved)
            {
                bool const bChecked(current != TRISTATE_FALSE);
                assert(current != TRISTATE_INDET); // can't *change* it to that
                switch (DocumentSettingForOption(option).first)
                {
                    case DocumentSettingId::PARA_SPACE_MAX:
                        m_pWrtShell->SetParaSpaceMax( bChecked );
                        break;

                    case DocumentSettingId::PARA_SPACE_MAX_AT_PAGES:
                        m_pWrtShell->SetParaSpaceMaxAtPages( bChecked );
                        break;

                    case DocumentSettingId::TAB_COMPAT:
                        m_pWrtShell->SetTabCompat( !bChecked );
                        break;

                    case DocumentSettingId::ADD_EXT_LEADING:
                        m_pWrtShell->SetAddExtLeading( !bChecked );
                        break;

                    case DocumentSettingId::OLD_LINE_SPACING:
                        m_pWrtShell->SetUseFormerLineSpacing( bChecked );
                        break;

                    case DocumentSettingId::ADD_PARA_SPACING_TO_TABLE_CELLS:
                        m_pWrtShell->SetAddParaSpacingToTableCells( bChecked );
                        break;

                    case DocumentSettingId::USE_FORMER_OBJECT_POS:
                        m_pWrtShell->SetUseFormerObjectPositioning( bChecked );
                        break;

                    case DocumentSettingId::USE_FORMER_TEXT_WRAPPING:
                        m_pWrtShell->SetUseFormerTextWrapping( bChecked );
                        break;

                    case DocumentSettingId::CONSIDER_WRAP_ON_OBJECT_POSITION:
                        m_pWrtShell->SetConsiderWrapOnObjPos( bChecked );
                        break;

                    case DocumentSettingId::DO_NOT_JUSTIFY_LINES_WITH_MANUAL_BREAK:
                        m_pWrtShell->SetDoNotJustifyLinesWithManualBreak( !bChecked );
                        break;

                    case DocumentSettingId::PROTECT_FORM:
                        m_pWrtShell->SetProtectForm( bChecked );
                        break;

                    case DocumentSettingId::MS_WORD_COMP_TRAILING_BLANKS:
                        m_pWrtShell->SetMsWordCompTrailingBlanks( bChecked );
                        break;

                    case DocumentSettingId::SUBTRACT_FLYS:
                        m_pWrtShell->SetSubtractFlysAnchoredAtFlys(bChecked);
                        break;

                    case DocumentSettingId::EMPTY_DB_FIELD_HIDES_PARA:
                        m_pWrtShell->SetEmptyDbFieldHidesPara(bChecked);
                        break;

                    case DocumentSettingId::USE_VARIABLE_WIDTH_NBSP:
                        m_pWrtShell->GetDoc()->getIDocumentSettingAccess()
                            .set(DocumentSettingId::USE_VARIABLE_WIDTH_NBSP, bChecked);
                        break;

                    case DocumentSettingId::NO_GAP_AFTER_NOTE_NUMBER:
                        m_pWrtShell->SetNoGapAfterNoteNumber(bChecked);
                        break;

                    case DocumentSettingId::TABS_RELATIVE_TO_INDENT:
                        m_pWrtShell->SetTabsRelativeToIndent(bChecked);
                        break;

                    case DocumentSettingId::TAB_OVER_MARGIN:
                        m_pWrtShell->SetTabOverMargin(bChecked);
                        break;

                    default:
                        break;
                }
                bModified = true;
            }
        }
    }

    return bModified;
}

void SwCompatibilityOptPage::Reset( const SfxItemSet*  )
{
    SetCurrentOptions();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
