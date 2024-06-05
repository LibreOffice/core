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

#include <hintids.hxx>
#include <vcl/settings.hxx>
#include <vcl/virdev.hxx>
#include <sfx2/tabdlg.hxx>
#include <editeng/brushitem.hxx>
#include <unotools/configmgr.hxx>
#include <SwStyleNameMapper.hxx>
#include <num.hxx>
#include <view.hxx>
#include <docsh.hxx>
#include <uitool.hxx>
#include <wrtsh.hxx>
#include <swmodule.hxx>
#include <fmtcol.hxx>
#include <outline.hxx>
#include <uinums.hxx>
#include <poolfmt.hxx>
#include <shellres.hxx>
#include <svl/style.hxx>
#include <charfmt.hxx>
#include <docstyle.hxx>
#include <viewopt.hxx>
#include <outline.hrc>
#include <strings.hrc>
#include <paratr.hxx>
#include <svtools/colorcfg.hxx>

#include <IDocumentOutlineNodes.hxx>

using namespace ::com::sun::star;

namespace {

class SwNumNamesDlg : public weld::GenericDialogController
{
    std::unique_ptr<weld::Entry> m_xFormEdit;
    std::unique_ptr<weld::TreeView> m_xFormBox;
    std::unique_ptr<weld::Button> m_xOKBtn;

    DECL_LINK( ModifyHdl, weld::Entry&, void );
    DECL_LINK( SelectHdl, weld::TreeView&, void );
    DECL_LINK( DoubleClickHdl, weld::TreeView&, bool );

public:
    explicit SwNumNamesDlg(weld::Window *pParent);
    void SetUserNames(const OUString *pList[]);
    OUString GetName() const { return m_xFormEdit->get_text(); }
    int GetCurEntryPos() const { return m_xFormBox->get_selected_index(); }
};

}

// remember selected entry
IMPL_LINK( SwNumNamesDlg, SelectHdl, weld::TreeView&, rBox, void )
{
    m_xFormEdit->set_text(rBox.get_selected_text());
    m_xFormEdit->select_region(0, -1);
}

/** set user defined names
 *
 * @param pList list of user defined names; unknown positions for the user are 0.
 */
void SwNumNamesDlg::SetUserNames(const OUString *pList[])
{
    sal_uInt16 nSelect = 0;
    for (sal_uInt16 i = 0; i < SwChapterNumRules::nMaxRules; ++i)
    {
        if(pList[i])
        {
            m_xFormBox->remove(i);
            m_xFormBox->insert_text(i, *pList[i]);
            if (i == nSelect)
                nSelect++;
        }
    }
    m_xFormBox->select(std::min(nSelect, o3tl::narrowing<sal_uInt16>(m_xFormBox->n_children() - 1)));
    SelectHdl(*m_xFormBox);
}

// unlock OK-Button when text is in Edit
IMPL_LINK( SwNumNamesDlg, ModifyHdl, weld::Entry&, rBox, void )
{
    m_xOKBtn->set_sensitive(!rBox.get_text().isEmpty());
}

// DoubleClickHdl
IMPL_LINK_NOARG(SwNumNamesDlg, DoubleClickHdl, weld::TreeView&, bool)
{
    m_xDialog->response(RET_OK);
    return true;
}

SwNumNamesDlg::SwNumNamesDlg(weld::Window *pParent)
    : GenericDialogController(pParent,
            u"modules/swriter/ui/numberingnamedialog.ui"_ustr,
            u"NumberingNameDialog"_ustr)
    , m_xFormEdit(m_xBuilder->weld_entry(u"entry"_ustr))
    , m_xFormBox(m_xBuilder->weld_tree_view(u"form"_ustr))
    , m_xOKBtn(m_xBuilder->weld_button(u"ok"_ustr))
{
    for (auto const& aID : OUTLINE_STYLE)
        m_xFormBox->append_text(SwResId(aID));

    m_xFormEdit->connect_changed(LINK(this, SwNumNamesDlg, ModifyHdl));
    m_xFormBox->connect_changed(LINK(this, SwNumNamesDlg, SelectHdl));
    m_xFormBox->connect_row_activated(LINK(this, SwNumNamesDlg, DoubleClickHdl));
    m_xFormBox->set_size_request(-1, m_xFormBox->get_height_rows(9));
}

static sal_uInt16 lcl_BitToLevel(sal_uInt16 nActLevel)
{
    constexpr sal_uInt16 MAXLEVEL_MASK = USHRT_MAX >> (sizeof(sal_uInt16) * CHAR_BIT - MAXLEVEL);
    assert((nActLevel & MAXLEVEL_MASK) == nActLevel);
    sal_uInt16 nTmp = nActLevel & MAXLEVEL_MASK; // a safety measure
    sal_uInt16 nTmpLevel = 0;
    while( 0 != (nTmp >>= 1) )
        nTmpLevel++;
    return nTmpLevel;
}

sal_uInt16 SwOutlineTabDialog::s_nNumLevel = 1;

SwOutlineTabDialog::SwOutlineTabDialog(weld::Window* pParent, const SfxItemSet* pSwItemSet,
    SwWrtShell &rSh)
    : SfxTabDialogController(pParent, u"modules/swriter/ui/outlinenumbering.ui"_ustr, u"OutlineNumberingDialog"_ustr, pSwItemSet)
    , m_rWrtSh(rSh)
    , m_pChapterNumRules(SW_MOD()->GetChapterNumRules())
    , m_bModified(m_rWrtSh.IsModified())
    , m_xMenuButton(m_xBuilder->weld_menu_button(u"format"_ustr))
{
    m_xMenuButton->connect_toggled(LINK(this, SwOutlineTabDialog, FormHdl));
    m_xMenuButton->connect_selected(LINK(this, SwOutlineTabDialog, MenuSelectHdl));

    m_xNumRule.reset(new SwNumRule(*rSh.GetOutlineNumRule()));
    GetCancelButton().connect_clicked(LINK(this, SwOutlineTabDialog, CancelHdl));

    if (auto nOutlinePos = m_rWrtSh.GetOutlinePos(MAXLEVEL); nOutlinePos != SwOutlineNodes::npos)
    {
        int nTmp = m_rWrtSh.getIDocumentOutlineNodesAccess()->getOutlineLevel(nOutlinePos);
        assert(nTmp < MAXLEVEL);
        SetActNumLevel(nTmp < 0 ? USHRT_MAX : (1 << nTmp));
    }

    AddTabPage(u"position"_ustr, &SwNumPositionTabPage::Create, nullptr);
    AddTabPage(u"numbering"_ustr, &SwOutlineSettingsTabPage::Create, nullptr);

    OUString sHeadline;
    sal_uInt16 i;

    for( i = 0; i < MAXLEVEL; ++i )
    {
        // if the style wasn't created yet, it's still at this position
        if( !m_rWrtSh.GetParaStyle( sHeadline =
            SwStyleNameMapper::GetUIName( static_cast< sal_uInt16 >(RES_POOLCOLL_HEADLINE1 + i),
                                          sHeadline )) )
            m_aCollNames[i] = sHeadline;
    }

    // query the text templates' outlining levels
    const sal_uInt16 nCount = m_rWrtSh.GetTextFormatCollCount();
    for(i = 0; i < nCount; ++i )
    {
        SwTextFormatColl &rTextColl = m_rWrtSh.GetTextFormatColl(i);
        if(!rTextColl.IsDefault())
        {
            if(rTextColl.IsAssignedToListLevelOfOutlineStyle())
            {
                int nOutLevel = rTextColl.GetAssignedOutlineStyleLevel();
                m_aCollNames[ nOutLevel ] = rTextColl.GetName();
            }
        }
    }
}

SwOutlineTabDialog::~SwOutlineTabDialog()
{
}

void SwOutlineTabDialog::PageCreated(const OUString& rPageId, SfxTabPage& rPage)
{
    if (rPageId == "position")
    {
        static_cast<SwNumPositionTabPage&>(rPage).SetWrtShell(&m_rWrtSh);
        static_cast<SwNumPositionTabPage&>(rPage).SetOutlineTabDialog(this);
    }
    else if (rPageId == "numbering")
    {
        static_cast<SwOutlineSettingsTabPage&>(rPage).SetWrtShell(&m_rWrtSh);
    }
}

IMPL_LINK_NOARG(SwOutlineTabDialog, CancelHdl, weld::Button&, void)
{
    if (!m_bModified)
        m_rWrtSh.ResetModified();
    m_xDialog->response(RET_CANCEL);
}

IMPL_LINK_NOARG(SwOutlineTabDialog, FormHdl, weld::Toggleable&, void)
{
    if (!m_xMenuButton->get_active())
        return;

    // fill PopupMenu
    for(sal_uInt16 i = 0; i < SwChapterNumRules::nMaxRules; ++i)
    {
        const SwNumRulesWithName *pRules = m_pChapterNumRules->GetRules(i);
        if (!pRules)
            continue;
        m_xMenuButton->set_item_label("form" + OUString::number(i + 1), pRules->GetName());
    }
}

IMPL_LINK(SwOutlineTabDialog, MenuSelectHdl, const OUString&, rIdent, void)
{
    sal_uInt8 nLevelNo = 0;

    if (rIdent == "form1")
        nLevelNo = 1;
    else if (rIdent == "form2")
        nLevelNo = 2;
    else if (rIdent == "form3")
        nLevelNo = 3;
    else if (rIdent == "form4")
        nLevelNo = 4;
    else if (rIdent == "form5")
        nLevelNo = 5;
    else if (rIdent == "form6")
        nLevelNo = 6;
    else if (rIdent == "form7")
        nLevelNo = 7;
    else if (rIdent == "form8")
        nLevelNo = 8;
    else if (rIdent == "form9")
        nLevelNo = 9;
    else if (rIdent == "saveas")
    {
        SwNumNamesDlg aDlg(m_xDialog.get());
        const OUString *aStrArr[SwChapterNumRules::nMaxRules];
        for(sal_uInt16 i = 0; i < SwChapterNumRules::nMaxRules; ++i)
        {
            const SwNumRulesWithName *pRules = m_pChapterNumRules->GetRules(i);
            if(pRules)
                aStrArr[i] = &pRules->GetName();
            else
                aStrArr[i] = nullptr;
        }
        aDlg.SetUserNames(aStrArr);
        if (aDlg.run() == RET_OK)
        {
            const OUString aName(aDlg.GetName());
            m_pChapterNumRules->ApplyNumRules( SwNumRulesWithName(
                    *m_xNumRule, aName ), aDlg.GetCurEntryPos() );
            m_xMenuButton->set_item_label("form" + OUString::number(aDlg.GetCurEntryPos() + 1), aName);
        }
        return;
    }

    if( nLevelNo-- )
    {
        const SwNumRulesWithName *pRules = m_pChapterNumRules->GetRules( nLevelNo );
        if( pRules )
        {
            pRules->ResetNumRule(m_rWrtSh, *m_xNumRule);
            m_xNumRule->SetRuleType( OUTLINE_RULE );
            SfxTabPage* pOutlinePage = GetTabPage(u"numbering");
            assert(pOutlinePage);
            static_cast<SwOutlineSettingsTabPage*>(pOutlinePage)->SetNumRule(m_xNumRule.get());
        }
        else
            *m_xNumRule = *m_rWrtSh.GetOutlineNumRule();
    }

    SfxTabPage* pPage = GetCurTabPage();
    pPage->Reset(GetOutputItemSet());
}

sal_uInt16  SwOutlineTabDialog::GetLevel(std::u16string_view rFormatName) const
{
    for(sal_uInt16 i = 0; i < MAXLEVEL; ++i)
    {
        if(m_aCollNames[i] == rFormatName)
            return i;
    }
    return MAXLEVEL;
}

short SwOutlineTabDialog::Ok()
{
    SfxTabDialogController::Ok();
    // set levels for all created templates; has to be done in order to
    // delete possibly cancelled assignments again.

    // encapsulate changes into an action to avoid effects on the current cursor
    // position during the changes.
    m_rWrtSh.StartAction();

    const SwNumRule * pOutlineRule = m_rWrtSh.GetOutlineNumRule();

    sal_uInt16 i, nCount = m_rWrtSh.GetTextFormatCollCount();
    for( i = 0; i < nCount; ++i )
    {
        SwTextFormatColl &rTextColl = m_rWrtSh.GetTextFormatColl(i);
        if( !rTextColl.IsDefault() )
        {
            const SfxPoolItem & rItem =
                rTextColl.GetFormatAttr(RES_PARATR_NUMRULE, false);

            if (static_cast<sal_uInt8>(GetLevel(rTextColl.GetName())) == MAXLEVEL)
            {
                if(rTextColl.IsAssignedToListLevelOfOutlineStyle())
                {
                    rTextColl.DeleteAssignmentToListLevelOfOutlineStyle();
                }
                if (static_cast<const SwNumRuleItem &>(rItem).GetValue() ==
                    pOutlineRule->GetName())
                {
                    rTextColl.ResetFormatAttr(RES_PARATR_NUMRULE);
                }
            }
            else
            {
                rTextColl.AssignToListLevelOfOutlineStyle(GetLevel(rTextColl.GetName()));

                if (static_cast<const SwNumRuleItem &>(rItem).GetValue() !=
                    pOutlineRule->GetName())
                {
                    SwNumRuleItem aItem(pOutlineRule->GetName());
                    rTextColl.SetFormatAttr(aItem);
                }
            }
        }
    }

    for(i = 0; i < MAXLEVEL; ++i )
    {
        OUString sHeadline;
        ::SwStyleNameMapper::FillUIName( static_cast< sal_uInt16 >(RES_POOLCOLL_HEADLINE1 + i),
                                         sHeadline );
        SwTextFormatColl* pColl = m_rWrtSh.FindTextFormatCollByName( sHeadline );
        if( !pColl && m_aCollNames[i] != sHeadline)
        {
            SwTextFormatColl* pTextColl = m_rWrtSh.GetTextCollFromPool(
                static_cast< sal_uInt16 >(RES_POOLCOLL_HEADLINE1 + i) );
            pTextColl->DeleteAssignmentToListLevelOfOutlineStyle();
            pTextColl->ResetFormatAttr(RES_PARATR_NUMRULE);

            if( !m_aCollNames[i].isEmpty() )
            {
                pTextColl = m_rWrtSh.GetParaStyle(
                            m_aCollNames[i], SwWrtShell::GETSTYLE_CREATESOME);
                if(pTextColl)
                {
                    pTextColl->AssignToListLevelOfOutlineStyle(i);
                    SwNumRuleItem aItem(pOutlineRule->GetName());
                    pTextColl->SetFormatAttr(aItem);
                }
            }
        }
    }

    m_rWrtSh.SetOutlineNumRule(*m_xNumRule);

    // #i30443#
    m_rWrtSh.EndAction();

    return RET_OK;
}

SwOutlineSettingsTabPage::SwOutlineSettingsTabPage(weld::Container* pPage, weld::DialogController* pController,
    const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, u"modules/swriter/ui/outlinenumberingpage.ui"_ustr, u"OutlineNumberingPage"_ustr, &rSet)
    , m_aNoFormatName(SwResId(SW_STR_NONE))
    , m_pSh(nullptr)
    , m_pNumRule(nullptr)
    , m_pCollNames(nullptr)
    , m_nActLevel(1)
    , m_xLevelLB(m_xBuilder->weld_tree_view(u"level"_ustr))
    , m_xCollBox(m_xBuilder->weld_combo_box(u"style"_ustr))
    , m_xNumberBox(new SwNumberingTypeListBox(m_xBuilder->weld_combo_box(u"numbering"_ustr)))
    , m_xCharFormatLB(m_xBuilder->weld_combo_box(u"charstyle"_ustr))
    , m_xAllLevelFT(m_xBuilder->weld_label(u"sublevelsft"_ustr))
    , m_xAllLevelNF(m_xBuilder->weld_spin_button(u"sublevelsnf"_ustr))
    , m_xPrefixED(m_xBuilder->weld_entry(u"prefix"_ustr))
    , m_xSuffixED(m_xBuilder->weld_entry(u"suffix"_ustr))
    , m_xStartEdit(m_xBuilder->weld_spin_button(u"startat"_ustr))
    , m_xPreviewWIN(new weld::CustomWeld(*m_xBuilder, u"preview"_ustr, m_aPreviewWIN))
{
    SetExchangeSupport();

    m_xNumberBox->Reload(SwInsertNumTypes::NoNumbering | SwInsertNumTypes::Extended);
    m_xCollBox->make_sorted();
    m_xCollBox->append_text(m_aNoFormatName);
    m_xLevelLB->connect_changed(LINK(this,    SwOutlineSettingsTabPage, LevelHdl));
    m_xAllLevelNF->connect_value_changed(LINK(this, SwOutlineSettingsTabPage, ToggleComplete));
    m_xCollBox->connect_changed(LINK(this,    SwOutlineSettingsTabPage, CollSelect));
    m_xNumberBox->connect_changed(LINK(this,  SwOutlineSettingsTabPage, NumberSelect));
    m_xPrefixED->connect_changed(LINK(this,   SwOutlineSettingsTabPage, DelimModify));
    m_xSuffixED->connect_changed(LINK(this,   SwOutlineSettingsTabPage, DelimModify));
    m_xStartEdit->connect_value_changed(LINK(this,  SwOutlineSettingsTabPage, StartModified));
    m_xCharFormatLB->make_sorted();
    m_xCharFormatLB->connect_changed(LINK(this,  SwOutlineSettingsTabPage, CharFormatHdl));
}

void    SwOutlineSettingsTabPage::Update()
{
        // if a template was already selected for this level, select it in the ListBox
    m_xCollBox->set_sensitive(USHRT_MAX != m_nActLevel);
    if(USHRT_MAX == m_nActLevel)
    {
        bool bSamePrefix = true;
        bool bSameSuffix = true;
        bool bSameType = true;
        bool bSameComplete = true;
        bool bSameStart = true;
        bool bSameCharFormat = true;

        const SwNumFormat* aNumFormatArr[MAXLEVEL];
        const SwCharFormat* pFirstFormat = nullptr;

        for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
        {

            aNumFormatArr[ i ] = &m_pNumRule->Get(i);
            if(i == 0)
                pFirstFormat = aNumFormatArr[i]->GetCharFormat();
            else
            {
                bSameType   &= aNumFormatArr[i]->GetNumberingType() == aNumFormatArr[0]->GetNumberingType();
                bSameStart  &= aNumFormatArr[i]->GetStart() == aNumFormatArr[0]->GetStart();
                bSamePrefix &= aNumFormatArr[i]->GetPrefix() == aNumFormatArr[0]->GetPrefix();
                bSameSuffix &= aNumFormatArr[i]->GetSuffix() == aNumFormatArr[0]->GetSuffix();
                bSameComplete &= aNumFormatArr[i]->GetIncludeUpperLevels() == aNumFormatArr[0]->GetIncludeUpperLevels();
                const SwCharFormat* pFormat = aNumFormatArr[i]->GetCharFormat();
                bSameCharFormat &=     (!pFirstFormat && !pFormat)
                                    || (pFirstFormat && pFormat && pFormat->GetName() == pFirstFormat->GetName());
            }
        }
        CheckForStartValue_Impl(aNumFormatArr[0]->GetNumberingType());
        if (bSameType)
            m_xNumberBox->SelectNumberingType( aNumFormatArr[0]->GetNumberingType() );
        else
            m_xNumberBox->SetNoSelection();
        if(bSameStart)
            m_xStartEdit->set_value(aNumFormatArr[0]->GetStart());
        else
            m_xStartEdit->set_text(OUString());
        if(bSamePrefix)
            m_xPrefixED->set_text(aNumFormatArr[0]->GetPrefix());
        else
            m_xPrefixED->set_text(OUString());
        if(bSameSuffix)
            m_xSuffixED->set_text(aNumFormatArr[0]->GetSuffix());
        else
            m_xSuffixED->set_text(OUString());

        if (bSameCharFormat)
        {
            if (pFirstFormat)
                m_xCharFormatLB->set_active_text(pFirstFormat->GetName());
            else
                m_xCharFormatLB->set_active_text(SwViewShell::GetShellRes()->aStrNone);
        }
        else
            m_xCharFormatLB->set_active(-1);

        m_xAllLevelFT->set_sensitive(true);
        m_xAllLevelNF->set_sensitive(true);
        m_xAllLevelNF->set_max(MAXLEVEL);
        if (bSameComplete)
        {
            m_xAllLevelNF->set_value(aNumFormatArr[0]->GetIncludeUpperLevels());
        }
        else
        {
            m_xAllLevelNF->set_text(OUString());
        }
    }
    else
    {
        sal_uInt16 nTmpLevel = lcl_BitToLevel(m_nActLevel);
        OUString aColl(m_pCollNames[nTmpLevel]);
        if(!aColl.isEmpty())
            m_xCollBox->set_active_text(aColl);
        else
            m_xCollBox->set_active_text(m_aNoFormatName);
        const SwNumFormat &rFormat = m_pNumRule->Get(nTmpLevel);

        m_xNumberBox->SelectNumberingType( rFormat.GetNumberingType() );
        m_xPrefixED->set_text(rFormat.GetPrefix());
        m_xSuffixED->set_text(rFormat.GetSuffix());
        const SwCharFormat* pFormat = rFormat.GetCharFormat();
        if(pFormat)
            m_xCharFormatLB->set_active_text(pFormat->GetName());
        else
            m_xCharFormatLB->set_active_text(SwViewShell::GetShellRes()->aStrNone);

        if (nTmpLevel || rFormat.HasListFormat())
        {
            m_xAllLevelFT->set_sensitive(true);
            m_xAllLevelNF->set_sensitive(true);
            m_xAllLevelNF->set_max(nTmpLevel + 1);
            m_xAllLevelNF->set_min(rFormat.HasListFormat() ? 0 : 1);
            m_xAllLevelNF->set_value(rFormat.GetIncludeUpperLevels());
        }
        else
        {
            m_xAllLevelNF->set_text(OUString());
            m_xAllLevelNF->set_sensitive(false);
            m_xAllLevelFT->set_sensitive(false);
        }
        CheckForStartValue_Impl(rFormat.GetNumberingType());
        m_xStartEdit->set_value( rFormat.GetStart() );
    }
    SetModified();
}

IMPL_LINK( SwOutlineSettingsTabPage, LevelHdl, weld::TreeView&, rBox, void )
{
    auto aRows = rBox.get_selected_rows();
    assert(aRows.empty() || aRows.size() == 1); // Single selection only
    if (aRows.empty() || aRows[0] == MAXLEVEL)
    {
        m_nActLevel = USHRT_MAX;
    }
    else
    {
        m_nActLevel = 1 << aRows[0];
    }
    Update();
}

IMPL_LINK(SwOutlineSettingsTabPage, ToggleComplete, weld::SpinButton&, rEdit, void)
{
    sal_uInt16 nMask = 1;
    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
    {
        if(m_nActLevel & nMask)
        {
            SwNumFormat aNumFormat(m_pNumRule->Get(i));
            aNumFormat.SetIncludeUpperLevels( std::min( static_cast<sal_uInt8>(rEdit.get_value()),
                                                static_cast<sal_uInt8>(i + 1)) );
            // Set the same prefix/suffix to generate list format with changed IncludedUpperLevels
            aNumFormat.SetListFormat(aNumFormat.GetPrefix(), aNumFormat.GetSuffix(), i);
            m_pNumRule->Set(i, aNumFormat);
        }
        nMask <<= 1;
    }
    SetModified();
}

IMPL_LINK( SwOutlineSettingsTabPage, CollSelect, weld::ComboBox&, rBox, void )
{
    sal_uInt8 i;

    const OUString aCollName(rBox.get_active_text());
    //0xFFFF not allowed here (disable)
    sal_uInt16 nTmpLevel = lcl_BitToLevel(m_nActLevel);
    OUString sOldName( m_pCollNames[nTmpLevel] );

    for( i = 0; i < MAXLEVEL; ++i)
        m_pCollNames[i] = m_aSaveCollNames[i];

    m_pCollNames[nTmpLevel] = aCollName;
            // template already in use?
    for( i = 0; i < MAXLEVEL; ++i)
        if(i != nTmpLevel && m_pCollNames[i] == aCollName )
            m_pCollNames[i].clear();

    // search the oldname and put it into the current entries
    if( !sOldName.isEmpty() )
        for( i = 0; i < MAXLEVEL; ++i)
            if( m_aSaveCollNames[ i ] == sOldName && i != nTmpLevel &&
                m_pCollNames[ i ].isEmpty() )
            {
                sal_uInt8 n;
                for( n = 0; n < MAXLEVEL; ++n )
                    if( m_pCollNames[ n ] == sOldName )
                        break;

                if( MAXLEVEL == n )
                    // it was an outline level name and the current entries is zero.
                    m_pCollNames[ i ] = sOldName;
            }

    SetModified();
    CollSave();
}

void SwOutlineSettingsTabPage::CollSave()
{
    for (sal_uInt8 i = 0; i < MAXLEVEL; ++i)
        m_aSaveCollNames[i] =  m_pCollNames[i];
}

IMPL_LINK_NOARG(SwOutlineSettingsTabPage, NumberSelect, weld::ComboBox&, void)
{
    sal_uInt16 nMask = 1;
    SvxNumType nNumberType = m_xNumberBox->GetSelectedNumberingType();
    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
    {
        if(m_nActLevel & nMask)
        {
            SwNumFormat aNumFormat(m_pNumRule->Get(i));
            aNumFormat.SetNumberingType(nNumberType);
            // ensure that HasListFormat
            aNumFormat.SetListFormat(aNumFormat.GetPrefix(), aNumFormat.GetSuffix(), i);
            m_pNumRule->Set(i, aNumFormat);
            CheckForStartValue_Impl(nNumberType);
        }
        nMask <<= 1;
    }
    SetModified();
}

IMPL_LINK_NOARG(SwOutlineSettingsTabPage, DelimModify, weld::Entry&, void)
{
    sal_uInt16 nMask = 1;
    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
    {
        if(m_nActLevel & nMask)
        {
            SwNumFormat aNumFormat(m_pNumRule->Get(i));
            aNumFormat.SetListFormat( m_xPrefixED->get_text(), m_xSuffixED->get_text(), i );
            m_pNumRule->Set(i, aNumFormat);
        }
        nMask <<= 1;
    }
    SetModified();
}

IMPL_LINK( SwOutlineSettingsTabPage, StartModified, weld::SpinButton&, rEdit, void )
{
    sal_uInt16 nMask = 1;
    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
    {
        if(m_nActLevel & nMask)
        {
            SwNumFormat aNumFormat(m_pNumRule->Get(i));
            aNumFormat.SetStart(o3tl::narrowing<sal_uInt16>(rEdit.get_value()));
            m_pNumRule->Set(i, aNumFormat);
        }
        nMask <<= 1;
    }
    SetModified();
}

IMPL_LINK_NOARG(SwOutlineSettingsTabPage, CharFormatHdl, weld::ComboBox&, void)
{
    OUString sEntry = m_xCharFormatLB->get_active_text();
    sal_uInt16 nMask = 1;
    bool bFormatNone = sEntry == SwViewShell::GetShellRes()->aStrNone;
    SwCharFormat* pFormat = nullptr;
    if(!bFormatNone)
    {
        sal_uInt16 nChCount = m_pSh->GetCharFormatCount();
        for(sal_uInt16 i = 0; i < nChCount; i++)
        {
            SwCharFormat& rChFormat = m_pSh->GetCharFormat(i);
            if(rChFormat.GetName() == sEntry)
            {
                pFormat = &rChFormat;
                break;
            }
        }
        if(!pFormat)
        {
            SfxStyleSheetBasePool* pPool = m_pSh->GetView().GetDocShell()->GetStyleSheetPool();
            SfxStyleSheetBase* pBase;
            pBase = pPool->Find(sEntry, SfxStyleFamily::Char);
            if(!pBase)
                pBase = &pPool->Make(sEntry, SfxStyleFamily::Page);
            pFormat = static_cast<SwDocStyleSheet*>(pBase)->GetCharFormat();

        }
    }

    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
    {
        if(m_nActLevel & nMask)
        {
            SwNumFormat aNumFormat(m_pNumRule->Get(i));
            if(bFormatNone)
                aNumFormat.SetCharFormat(nullptr);
            else
                aNumFormat.SetCharFormat(pFormat);
            m_pNumRule->Set(i, aNumFormat);
        }
        nMask <<= 1;
    }
}

SwOutlineSettingsTabPage::~SwOutlineSettingsTabPage()
{
}

void SwOutlineSettingsTabPage::SetWrtShell(SwWrtShell* pShell)
{
    m_pSh = pShell;
    // query this document's NumRules
    m_pNumRule = static_cast<SwOutlineTabDialog*>(GetDialogController())->GetNumRule();
    m_pCollNames = static_cast<SwOutlineTabDialog*>(GetDialogController())->GetCollNames();

    CollSave();

    m_aPreviewWIN.SetNumRule(m_pNumRule);
    m_aPreviewWIN.SetOutlineNames(m_pCollNames);
    // set start value - nActLevel must be 1 here
    sal_uInt16 nTmpLevel = lcl_BitToLevel(m_nActLevel);
    const SwNumFormat& rNumFormat = m_pNumRule->Get( nTmpLevel );
    m_xStartEdit->set_value( rNumFormat.GetStart() );

    // create pool formats for headlines
    for (sal_uInt16 i = 0; i < MAXLEVEL; ++i)
    {
        m_xCollBox->append_text( SwStyleNameMapper::GetUIName(
                                    static_cast< sal_uInt16 >(RES_POOLCOLL_HEADLINE1 + i), OUString()));
        m_xLevelLB->append_text( OUString::number(i + 1) );
    }
    OUString sStr = "1 - " + OUString::number(MAXLEVEL);
    m_xLevelLB->append_text(sStr);

    // query the texttemplates' outlining levels
    const sal_uInt16 nCount = m_pSh->GetTextFormatCollCount();
    for (sal_uInt16 i = 0; i < nCount; ++i)
    {
        SwTextFormatColl &rTextColl = m_pSh->GetTextFormatColl(i);
        if(!rTextColl.IsDefault())
        {
            sStr = rTextColl.GetName();
            if (m_xCollBox->find_text(sStr) == -1)
                m_xCollBox->append_text(sStr);
        }
    }

    m_xNumberBox->SelectNumberingType(rNumFormat.GetNumberingType());

    // collect char styles
    m_xCharFormatLB->clear();
    m_xCharFormatLB->append_text(SwViewShell::GetShellRes()->aStrNone);

    // char styles
    ::FillCharStyleListBox(*m_xCharFormatLB,
                        m_pSh->GetView().GetDocShell());
    Update();
}

void SwOutlineSettingsTabPage::ActivatePage(const SfxItemSet& )
{
    m_nActLevel = SwOutlineTabDialog::GetActNumLevel();
    if(m_nActLevel != USHRT_MAX)
        m_xLevelLB->select(lcl_BitToLevel(m_nActLevel));
    else
        m_xLevelLB->select(MAXLEVEL);
    LevelHdl(*m_xLevelLB);
}

DeactivateRC SwOutlineSettingsTabPage::DeactivatePage(SfxItemSet*)
{
    SwOutlineTabDialog::SetActNumLevel(m_nActLevel);
    return DeactivateRC::LeavePage;
}

bool SwOutlineSettingsTabPage::FillItemSet( SfxItemSet*  )
{
    return true;
}

void SwOutlineSettingsTabPage::Reset( const SfxItemSet* rSet )
{
    ActivatePage(*rSet);
}

std::unique_ptr<SfxTabPage> SwOutlineSettingsTabPage::Create(weld::Container* pPage, weld::DialogController* pController,
                                                    const SfxItemSet* rAttrSet)
{
    return std::make_unique<SwOutlineSettingsTabPage>(pPage, pController, *rAttrSet);
}

void SwOutlineSettingsTabPage::CheckForStartValue_Impl(sal_uInt16 nNumberingType)
{
    bool bIsNull = m_xStartEdit->get_value() == 0;
    bool bNoZeroAllowed = nNumberingType < SVX_NUM_ARABIC ||
                        SVX_NUM_CHARS_UPPER_LETTER_N == nNumberingType ||
                        SVX_NUM_CHARS_LOWER_LETTER_N == nNumberingType;
    m_xStartEdit->set_min(bNoZeroAllowed ? 1 : 0);
    if (bIsNull && bNoZeroAllowed)
        StartModified(*m_xStartEdit);
}

static tools::Long lcl_DrawBullet(vcl::RenderContext* pVDev, const SwNumFormat& rFormat, tools::Long nXStart, tools::Long nYStart, const Size& rSize)
{
    vcl::Font aTmpFont(pVDev->GetFont());

    // via Uno it's possible that no font has been set!
    vcl::Font aFont(rFormat.GetBulletFont() ? *rFormat.GetBulletFont() : aTmpFont);
    Size aTmpSize(rSize);
    aTmpSize.setWidth( aTmpSize.Width() * ( rFormat.GetBulletRelSize()) );
    aTmpSize.setWidth( aTmpSize.Width() / 100 ) ;
    aTmpSize.setHeight( aTmpSize.Height() * ( rFormat.GetBulletRelSize()) );
    aTmpSize.setHeight( aTmpSize.Height() / 100 ) ;
    // in case of a height of zero it is drawn in original height
    if(!aTmpSize.Height())
        aTmpSize.setHeight( 1 );
    aFont.SetFontSize(aTmpSize);
    aFont.SetTransparent(true);
    Color aBulletColor = rFormat.GetBulletColor();
    if(aBulletColor == COL_AUTO)
        aBulletColor = pVDev->GetFillColor().IsDark() ? COL_WHITE : COL_BLACK;
    else if(aBulletColor == pVDev->GetFillColor())
        aBulletColor.Invert();
    aFont.SetColor(aBulletColor);
    pVDev->SetFont( aFont );
    sal_UCS4 cBullet = rFormat.GetBulletChar();
    OUString aText(&cBullet, 1);
    tools::Long nY = nYStart;
    nY -= ((aTmpSize.Height() - rSize.Height())/ 2);
    pVDev->DrawText( Point(nXStart, nY), aText );
    tools::Long nRet = pVDev->GetTextWidth(aText);

    pVDev->SetFont(aTmpFont);
    return nRet;
}

static tools::Long lcl_DrawGraphic(vcl::RenderContext& rVDev, const SwNumFormat &rFormat, tools::Long nXStart, tools::Long nYStart, tools::Long nDivision)
{
    const SvxBrushItem* pBrushItem = rFormat.GetBrush();
    tools::Long nRet = 0;
    if (pBrushItem)
    {
        const Graphic* pGraphic = pBrushItem->GetGraphic();
        if (pGraphic)
        {
            Size aGSize( rFormat.GetGraphicSize());
            aGSize.setWidth( aGSize.Width() / nDivision );
            nRet = aGSize.Width();
            aGSize.setHeight( aGSize.Height() / nDivision );
            pGraphic->Draw(rVDev, Point(nXStart, nYStart), rVDev.PixelToLogic(aGSize));
        }
    }
    return nRet;
}

void NumberingPreview::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    const Size aSize(rRenderContext.PixelToLogic(GetOutputSizePixel()));

    ScopedVclPtrInstance<VirtualDevice> pVDev(rRenderContext);
    pVDev->SetMapMode(rRenderContext.GetMapMode());
    pVDev->SetOutputSize(aSize);

    const SwViewOption& pOpt = SwViewOption::GetCurrentViewOptions();
    const Color& rDocColor = pOpt.GetDocColor();
    const Color& rDocBoundariesColor = pOpt.GetDocBoundariesColor();
    const Color& rFontColor = pOpt.GetFontColor();
    // #101524# OJ
    pVDev->SetFillColor(rDocColor);
    pVDev->SetLineColor(rDocBoundariesColor);
    pVDev->DrawRect(tools::Rectangle(Point(0,0), aSize));

    if (m_pActNum)
    {
        tools::Long nWidthRelation = 30; // chapter dialog
        if(m_nPageWidth)
        {
            nWidthRelation = m_nPageWidth / aSize.Width();
            if(m_bPosition)
                nWidthRelation = nWidthRelation * 2 / 3;
            else
                nWidthRelation = nWidthRelation / 4;
        }

        // height per level
        const tools::Long nXStep = aSize.Width() / (3 * MAXLEVEL * ((MAXLEVEL < 10) ? 2 : 1));
        const tools::Long nYStep = (aSize.Height() - 6)/ MAXLEVEL;
        tools::Long nYStart = 4;
        m_aStdFont = OutputDevice::GetDefaultFont(DefaultFontType::UI_SANS, GetAppLanguage(),
                                                GetDefaultFontFlags::OnlyOne, &rRenderContext);

        if (svtools::ColorConfig().GetColorValue(svtools::FONTCOLOR, false).nColor == COL_AUTO)
            m_aStdFont.SetColor( rDocColor.IsDark() ? COL_WHITE : COL_BLACK );
        else
            m_aStdFont.SetColor( rFontColor );

        const tools::Long nFontHeight = nYStep * ( m_bPosition ? 15 : 6 ) / 10;
        m_aStdFont.SetFontSize(Size( 0, nFontHeight ));

        tools::Long nPreNum = m_pActNum->Get(0).GetStart();

        if (m_bPosition)
        {
            const tools::Long nLineHeight = nFontHeight * 8 / 7;
            sal_uInt8 nStart = 0;
            while (!(m_nActLevel & (1 << nStart)))
            {
                nStart++;
            }
            if(nStart) // so that possible predecessors and successors are showed
                nStart--;

            SwNumberTree::tNumberVector aNumVector;
            sal_uInt8 nEnd = std::min(sal_uInt8(nStart + 3), MAXLEVEL);
            for (sal_uInt8 nLevel = nStart; nLevel < nEnd; ++nLevel)
            {
                const SwNumFormat &rFormat = m_pActNum->Get(nLevel);
                aNumVector.push_back(rFormat.GetStart());

                tools::Long nXStart( 0 );
                tools::Long nTextOffset( 0 );
                tools::Long nNumberXPos( 0 );
                if (rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION)
                {
                    nXStart = rFormat.GetAbsLSpace() / nWidthRelation;
                    nTextOffset = rFormat.GetCharTextDistance() / nWidthRelation;
                    nNumberXPos = nXStart;
                    const tools::Long nFirstLineOffset = (-rFormat.GetFirstLineOffset()) / nWidthRelation;

                    if(nFirstLineOffset <= nNumberXPos)
                        nNumberXPos -= nFirstLineOffset;
                    else
                        nNumberXPos = 0;
                }
                else if (rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT)
                {
                    const tools::Long nTmpNumberXPos((rFormat.GetIndentAt() + rFormat.GetFirstLineIndent()) / nWidthRelation);
                    nNumberXPos = (nTmpNumberXPos < 0) ? 0 : nTmpNumberXPos;
                }

                tools::Long nBulletWidth = 0;
                if (SVX_NUM_BITMAP == rFormat.GetNumberingType())
                {
                    nBulletWidth = lcl_DrawGraphic(*pVDev, rFormat, nNumberXPos,
                                                   nYStart, nWidthRelation);
                }
                else if (SVX_NUM_CHAR_SPECIAL == rFormat.GetNumberingType())
                {
                    nBulletWidth = lcl_DrawBullet(pVDev.get(), rFormat, nNumberXPos,
                                                  nYStart, m_aStdFont.GetFontSize());
                }
                else
                {
                    pVDev->SetFont(m_aStdFont);
                    if(m_pActNum->IsContinusNum())
                        aNumVector[nLevel] = nPreNum;
                    OUString aText(m_pActNum->MakeNumString( aNumVector ));
                    pVDev->DrawText( Point(nNumberXPos, nYStart), aText );
                    nBulletWidth = pVDev->GetTextWidth(aText);
                    nPreNum++;
                }
                if ( rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT &&
                     rFormat.GetLabelFollowedBy() == SvxNumberFormat::SPACE )
                {
                    pVDev->SetFont(m_aStdFont);
                    OUString aText(' ');
                    pVDev->DrawText( Point(nNumberXPos, nYStart), aText );
                    nBulletWidth += pVDev->GetTextWidth(aText);
                }

                tools::Long nTextXPos(0);
                if (rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION)
                {
                    nTextXPos = nXStart;
                    if (nTextOffset < 0)
                         nTextXPos = nTextXPos + nTextOffset;
                    if (nNumberXPos + nBulletWidth + nTextOffset > nTextXPos)
                        nTextXPos = nNumberXPos + nBulletWidth + nTextOffset;
                }
                else if (rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT)
                {
                    switch (rFormat.GetLabelFollowedBy())
                    {
                        case SvxNumberFormat::LISTTAB:
                        {
                            nTextXPos = rFormat.GetListtabPos() / nWidthRelation;
                            if (nTextXPos < nNumberXPos + nBulletWidth)
                            {
                                nTextXPos = nNumberXPos + nBulletWidth;
                            }
                        }
                        break;
                        case SvxNumberFormat::SPACE:
                        case SvxNumberFormat::NOTHING:
                        case SvxNumberFormat::NEWLINE:
                        {
                            nTextXPos = nNumberXPos + nBulletWidth;
                        }
                        break;
                    }

                    nXStart = rFormat.GetIndentAt() / nWidthRelation;
                }

                tools::Rectangle aRect1(Point(nTextXPos, nYStart + nFontHeight / 2), Size(aSize.Width() / 2, 2));
                pVDev->SetFillColor(rRenderContext.GetSettings().GetStyleSettings().GetWindowColor()); // COL_BLACK );
                pVDev->DrawRect(aRect1);

                tools::Rectangle aRect2(Point(nXStart, nYStart + nLineHeight + nFontHeight / 2), Size(aSize.Width() / 2, 2));
                pVDev->DrawRect(aRect2);
                nYStart += 2 * nLineHeight;
            }
        }
        else
        {
            SwNumberTree::tNumberVector aNumVector;
            const tools::Long nLineHeight = nFontHeight * 3 / 2;
            for (sal_uInt8 nLevel = 0; nLevel < MAXLEVEL; ++nLevel, nYStart = nYStart + nYStep)
            {
                const SwNumFormat &rFormat = m_pActNum->Get(nLevel);
                aNumVector.push_back(rFormat.GetStart());
                tools::Long nXStart(0);
                if (rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION)
                {
                    nXStart = rFormat.GetAbsLSpace() / nWidthRelation;
                }
                else if (rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT)
                {
                    const tools::Long nTmpXStart((rFormat.GetIndentAt() + rFormat.GetFirstLineIndent() ) / nWidthRelation);
                    nXStart = (nTmpXStart < 0) ? 0 : nTmpXStart;
                }
                nXStart /= 2;
                nXStart += 2;
                tools::Long nTextOffset;
                if (SVX_NUM_BITMAP == rFormat.GetNumberingType())
                {
                    lcl_DrawGraphic(*pVDev, rFormat, nXStart, nYStart, nWidthRelation);
                    nTextOffset = nLineHeight + nXStep;
                }
                else if (SVX_NUM_CHAR_SPECIAL == rFormat.GetNumberingType())
                {
                    nTextOffset =  lcl_DrawBullet(pVDev.get(), rFormat, nXStart, nYStart, m_aStdFont.GetFontSize());
                    nTextOffset += nXStep;
                }
                else
                {
                    pVDev->SetFont(m_aStdFont);
                    if (m_pActNum->IsContinusNum())
                        aNumVector[nLevel] = nPreNum;
                    OUString aText(m_pActNum->MakeNumString( aNumVector ));
                    pVDev->DrawText( Point(nXStart, nYStart), aText );
                    nTextOffset = pVDev->GetTextWidth(aText) + nXStep;
                    nPreNum++;
                }
                pVDev->SetFont(m_aStdFont);
                pVDev->DrawText(
                    Point(nXStart + nTextOffset, nYStart),
                    (m_pOutlineNames == nullptr
                     ? utl::ConfigManager::getProductName()
                     : m_pOutlineNames[nLevel]));
            }
        }
    }
    rRenderContext.DrawOutDev(Point(0,0), aSize, Point(0,0), aSize, *pVDev);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
