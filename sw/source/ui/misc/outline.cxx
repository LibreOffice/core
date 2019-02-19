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
#include <vcl/menu.hxx>
#include <vcl/settings.hxx>
#include <vcl/builderfactory.hxx>
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
#include <svtools/ctrlbox.hxx>
#include <globals.hrc>
#include <outline.hrc>
#include <strings.hrc>
#include <paratr.hxx>

#include <unomid.h>

#include <IDocumentOutlineNodes.hxx>
#include <app.hrc>

#include <com/sun/star/accessibility/AccessibleRole.hpp>

using namespace ::com::sun::star;

class SwNumNamesDlg : public weld::GenericDialogController
{
    std::unique_ptr<weld::Entry> m_xFormEdit;
    std::unique_ptr<weld::TreeView> m_xFormBox;
    std::unique_ptr<weld::Button> m_xOKBtn;

    DECL_LINK( ModifyHdl, weld::Entry&, void );
    DECL_LINK( SelectHdl, weld::TreeView&, void );
    DECL_LINK( DoubleClickHdl, weld::TreeView&, void );

public:
    explicit SwNumNamesDlg(weld::Window *pParent);
    void SetUserNames(const OUString *pList[]);
    OUString GetName() const { return m_xFormEdit->get_text(); }
    int GetCurEntryPos() const { return m_xFormBox->get_selected_index(); }
};

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
    m_xFormBox->select(nSelect);
    SelectHdl(*m_xFormBox);
}

// unlock OK-Button when text is in Edit
IMPL_LINK( SwNumNamesDlg, ModifyHdl, weld::Entry&, rBox, void )
{
    m_xOKBtn->set_sensitive(!rBox.get_text().isEmpty());
}

// DoubleClickHdl
IMPL_LINK_NOARG(SwNumNamesDlg, DoubleClickHdl, weld::TreeView&, void)
{
    m_xDialog->response(RET_OK);
}

SwNumNamesDlg::SwNumNamesDlg(weld::Window *pParent)
    : GenericDialogController(pParent,
            "modules/swriter/ui/numberingnamedialog.ui",
            "NumberingNameDialog")
    , m_xFormEdit(m_xBuilder->weld_entry("entry"))
    , m_xFormBox(m_xBuilder->weld_tree_view("form"))
    , m_xOKBtn(m_xBuilder->weld_button("ok"))
{
    for (size_t i = 0; i < SAL_N_ELEMENTS(OUTLINE_STYLE); ++i)
        m_xFormBox->append_text(SwResId(OUTLINE_STYLE[i]));

    m_xFormEdit->connect_changed(LINK(this, SwNumNamesDlg, ModifyHdl));
    m_xFormBox->connect_changed(LINK(this, SwNumNamesDlg, SelectHdl));
    m_xFormBox->connect_row_activated(LINK(this, SwNumNamesDlg, DoubleClickHdl));
    m_xFormBox->set_size_request(-1, m_xFormBox->get_height_rows(9));
}

static sal_uInt16 lcl_BitToLevel(sal_uInt16 nActLevel)
{
    sal_uInt16 nTmp = nActLevel;
    sal_uInt16 nTmpLevel = 0;
    while( 0 != (nTmp >>= 1) )
        nTmpLevel++;
    return nTmpLevel;
}

sal_uInt16 SwOutlineTabDialog::nNumLevel = 1;

SwOutlineTabDialog::SwOutlineTabDialog(weld::Window* pParent, const SfxItemSet* pSwItemSet,
    SwWrtShell &rSh)
    : SfxTabDialogController(pParent, "modules/swriter/ui/outlinenumbering.ui", "OutlineNumberingDialog", pSwItemSet)
    , rWrtSh(rSh)
    , pChapterNumRules(SW_MOD()->GetChapterNumRules())
    , bModified(rWrtSh.IsModified())
    , m_xMenuButton(m_xBuilder->weld_menu_button("format"))
{
    m_xMenuButton->connect_clicked(LINK(this, SwOutlineTabDialog, FormHdl));
    m_xMenuButton->connect_selected(LINK(this, SwOutlineTabDialog, MenuSelectHdl));

    xNumRule.reset(new SwNumRule(*rSh.GetOutlineNumRule()));
    GetCancelButton().connect_clicked(LINK(this, SwOutlineTabDialog, CancelHdl));

    AddTabPage("position", &SwNumPositionTabPage::Create, nullptr);
    AddTabPage("numbering", &SwOutlineSettingsTabPage::Create, nullptr);

    OUString sHeadline;
    sal_uInt16 i;

    for( i = 0; i < MAXLEVEL; ++i )
    {
        // if the style wasn't created yet, it's still at this position
        if( !rWrtSh.GetParaStyle( sHeadline =
            SwStyleNameMapper::GetUIName( static_cast< sal_uInt16 >(RES_POOLCOLL_HEADLINE1 + i),
                                          sHeadline )) )
            aCollNames[i] = sHeadline;
    }

    // query the text templates' outlining levels
    const sal_uInt16 nCount = rWrtSh.GetTextFormatCollCount();
    for(i = 0; i < nCount; ++i )
    {
        SwTextFormatColl &rTextColl = rWrtSh.GetTextFormatColl(i);
        if(!rTextColl.IsDefault())
        {
            if(rTextColl.IsAssignedToListLevelOfOutlineStyle())
            {
                int nOutLevel = rTextColl.GetAssignedOutlineStyleLevel();
                aCollNames[ nOutLevel ] = rTextColl.GetName();
            }
        }
    }
}

SwOutlineTabDialog::~SwOutlineTabDialog()
{
}

void SwOutlineTabDialog::PageCreated(const OString& rPageId, SfxTabPage& rPage)
{
    if (rPageId == "position")
    {
        static_cast<SwNumPositionTabPage&>(rPage).SetWrtShell(&rWrtSh);
        static_cast<SwNumPositionTabPage&>(rPage).SetOutlineTabDialog(this);
    }
    else if (rPageId == "numbering")
    {
        static_cast<SwOutlineSettingsTabPage&>(rPage).SetWrtShell(&rWrtSh);
    }
}

IMPL_LINK_NOARG(SwOutlineTabDialog, CancelHdl, weld::Button&, void)
{
    if (!bModified)
        rWrtSh.ResetModified();
    m_xDialog->response(RET_CANCEL);
}

IMPL_LINK_NOARG(SwOutlineTabDialog, FormHdl, weld::Button&, void)
{
    // fill PopupMenu
    for(sal_uInt16 i = 0; i < SwChapterNumRules::nMaxRules; ++i)
    {
        const SwNumRulesWithName *pRules = pChapterNumRules->GetRules(i);
        if (!pRules)
            continue;
        m_xMenuButton->set_item_label("form" + OString::number(i + 1), pRules->GetName());
    }

    OString sHelpId(m_xMenuButton->get_item_help_id("form1"));
    for (sal_Int32 i = 2; i <= 9; ++i)
    {
        m_xMenuButton->set_item_help_id("form" + OString::number(i), sHelpId);
    }
}

IMPL_LINK(SwOutlineTabDialog, MenuSelectHdl, const OString&, rIdent, void)
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
            const SwNumRulesWithName *pRules = pChapterNumRules->GetRules(i);
            if(pRules)
                aStrArr[i] = &pRules->GetName();
            else
                aStrArr[i] = nullptr;
        }
        aDlg.SetUserNames(aStrArr);
        if (aDlg.run() == RET_OK)
        {
            const OUString aName(aDlg.GetName());
            pChapterNumRules->ApplyNumRules( SwNumRulesWithName(
                    *xNumRule, aName ), aDlg.GetCurEntryPos() );
            m_xMenuButton->set_item_label("form" + OString::number(aDlg.GetCurEntryPos() + 1), aName);
        }
        return;
    }

    if( nLevelNo-- )
    {
        const SwNumRulesWithName *pRules = pChapterNumRules->GetRules( nLevelNo );
        if( pRules )
        {
            xNumRule.reset(pRules->MakeNumRule(rWrtSh));
            xNumRule->SetRuleType( OUTLINE_RULE );
            SfxTabPage* pOutlinePage = GetTabPage("numbering");
            assert(pOutlinePage);
            static_cast<SwOutlineSettingsTabPage*>(pOutlinePage)->SetNumRule(xNumRule.get());
        }
        else
            *xNumRule = *rWrtSh.GetOutlineNumRule();
    }

    SfxTabPage* pPage = GetCurTabPage();
    pPage->Reset(GetOutputItemSet());
}

sal_uInt16  SwOutlineTabDialog::GetLevel(const OUString &rFormatName) const
{
    for(sal_uInt16 i = 0; i < MAXLEVEL; ++i)
    {
        if(aCollNames[i] == rFormatName)
            return i;
    }
    return MAXLEVEL;
}

short SwOutlineTabDialog::Ok()
{
    SfxTabDialogController::Ok();
    // set levels for all created templates; has to be done in order to
    // delete possibly cancelled assignments again.

    // encapsulate changes into a action to avoid effects on the current cursor
    // position during the changes.
    rWrtSh.StartAction();

    const SwNumRule * pOutlineRule = rWrtSh.GetOutlineNumRule();

    sal_uInt16 i, nCount = rWrtSh.GetTextFormatCollCount();
    for( i = 0; i < nCount; ++i )
    {
        SwTextFormatColl &rTextColl = rWrtSh.GetTextFormatColl(i);
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
        SwTextFormatColl* pColl = rWrtSh.FindTextFormatCollByName( sHeadline );
        if( !pColl && aCollNames[i] != sHeadline)
        {
            SwTextFormatColl* pTextColl = rWrtSh.GetTextCollFromPool(
                static_cast< sal_uInt16 >(RES_POOLCOLL_HEADLINE1 + i) );
            pTextColl->DeleteAssignmentToListLevelOfOutlineStyle();
            pTextColl->ResetFormatAttr(RES_PARATR_NUMRULE);

            if( !aCollNames[i].isEmpty() )
            {
                pTextColl = rWrtSh.GetParaStyle(
                            aCollNames[i], SwWrtShell::GETSTYLE_CREATESOME);
                if(pTextColl)
                {
                    pTextColl->AssignToListLevelOfOutlineStyle(i);
                    SwNumRuleItem aItem(pOutlineRule->GetName());
                    pTextColl->SetFormatAttr(aItem);
                }
            }
        }
    }

    rWrtSh.SetOutlineNumRule(*xNumRule);

    // #i30443#
    rWrtSh.EndAction();

    return RET_OK;
}

SwOutlineSettingsTabPage::SwOutlineSettingsTabPage(TabPageParent pPage,
    const SfxItemSet& rSet)
    : SfxTabPage(pPage, "modules/swriter/ui/outlinenumberingpage.ui", "OutlineNumberingPage", &rSet)
    , aNoFormatName(SwResId(SW_STR_NONE))
    , pSh(nullptr)
    , pNumRule(nullptr)
    , pCollNames(nullptr)
    , nActLevel(1)
    , m_xLevelLB(m_xBuilder->weld_tree_view("level"))
    , m_xCollBox(m_xBuilder->weld_combo_box("style"))
    , m_xNumberBox(new SwNumberingTypeListBox(m_xBuilder->weld_combo_box("numbering")))
    , m_xCharFormatLB(m_xBuilder->weld_combo_box("charstyle"))
    , m_xAllLevelFT(m_xBuilder->weld_label("sublevelsft"))
    , m_xAllLevelNF(m_xBuilder->weld_spin_button("sublevelsnf"))
    , m_xPrefixED(m_xBuilder->weld_entry("prefix"))
    , m_xSuffixED(m_xBuilder->weld_entry("suffix"))
    , m_xStartEdit(m_xBuilder->weld_spin_button("startat"))
    , m_xPreviewWIN(new weld::CustomWeld(*m_xBuilder, "preview", m_aPreviewWIN))
{
    SetExchangeSupport();

    m_xNumberBox->Reload(SwInsertNumTypes::NoNumbering | SwInsertNumTypes::Extended);
    m_xCollBox->make_sorted();
    m_xCollBox->append_text(aNoFormatName);
    m_xLevelLB->connect_changed(LINK(this,    SwOutlineSettingsTabPage, LevelHdl));
    m_xAllLevelNF->connect_value_changed(LINK(this, SwOutlineSettingsTabPage, ToggleComplete));
    m_xCollBox->connect_changed(LINK(this,    SwOutlineSettingsTabPage, CollSelect));
    m_xNumberBox->connect_changed(LINK(this,  SwOutlineSettingsTabPage, NumberSelect));
    m_xPrefixED->connect_changed(LINK(this,   SwOutlineSettingsTabPage, DelimModify));
    m_xSuffixED->connect_changed(LINK(this,   SwOutlineSettingsTabPage, DelimModify));
    m_xStartEdit->connect_value_changed(LINK(this,  SwOutlineSettingsTabPage, StartModified));
    m_xCharFormatLB->connect_changed(LINK(this,  SwOutlineSettingsTabPage, CharFormatHdl));
}

void    SwOutlineSettingsTabPage::Update()
{
        // if a template was already selected for this level, select it in the ListBox
    m_xCollBox->set_sensitive(USHRT_MAX != nActLevel);
    if(USHRT_MAX == nActLevel)
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

            aNumFormatArr[ i ] = &pNumRule->Get(i);
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
        sal_uInt16 nTmpLevel = lcl_BitToLevel(nActLevel);
        OUString aColl(pCollNames[nTmpLevel]);
        if(!aColl.isEmpty())
            m_xCollBox->set_active_text(aColl);
        else
            m_xCollBox->set_active_text(aNoFormatName);
        const SwNumFormat &rFormat = pNumRule->Get(nTmpLevel);

        m_xNumberBox->SelectNumberingType( rFormat.GetNumberingType() );
        m_xPrefixED->set_text(rFormat.GetPrefix());
        m_xSuffixED->set_text(rFormat.GetSuffix());
        const SwCharFormat* pFormat = rFormat.GetCharFormat();
        if(pFormat)
            m_xCharFormatLB->set_active_text(pFormat->GetName());
        else
            m_xCharFormatLB->set_active_text(SwViewShell::GetShellRes()->aStrNone);

        if(nTmpLevel)
        {
            m_xAllLevelFT->set_sensitive(true);
            m_xAllLevelNF->set_sensitive(true);
            m_xAllLevelNF->set_max(nTmpLevel + 1);
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
    nActLevel = 0;
    auto aRows = rBox.get_selected_rows();
    if (std::find(aRows.begin(), aRows.end(), MAXLEVEL) != aRows.end())
    {
        nActLevel = 0xFFFF;
    }
    else
    {
        sal_uInt16 nMask = 1;
        for( sal_uInt16 i = 0; i < MAXLEVEL; i++ )
        {
            if (std::find(aRows.begin(), aRows.end(), i) != aRows.end())
                nActLevel |= nMask;
            nMask <<= 1;
        }
    }
    Update();
}

IMPL_LINK(SwOutlineSettingsTabPage, ToggleComplete, weld::SpinButton&, rEdit, void)
{
    sal_uInt16 nMask = 1;
    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
    {
        if(nActLevel & nMask)
        {
            SwNumFormat aNumFormat(pNumRule->Get(i));
            aNumFormat.SetIncludeUpperLevels( std::min( static_cast<sal_uInt8>(rEdit.get_value()),
                                                static_cast<sal_uInt8>(i + 1)) );
            pNumRule->Set(i, aNumFormat);
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
    sal_uInt16 nTmpLevel = lcl_BitToLevel(nActLevel);
    OUString sOldName( pCollNames[nTmpLevel] );

    for( i = 0; i < MAXLEVEL; ++i)
        pCollNames[i] = aSaveCollNames[i];

    if(aCollName == aNoFormatName)
        pCollNames[nTmpLevel].clear();
    else
    {
        pCollNames[nTmpLevel] = aCollName;
                // template already in use?
        for( i = 0; i < MAXLEVEL; ++i)
            if(i != nTmpLevel && pCollNames[i] == aCollName )
                pCollNames[i].clear();
    }

    // search the oldname and put it into the current entries
    if( !sOldName.isEmpty() )
        for( i = 0; i < MAXLEVEL; ++i)
            if( aSaveCollNames[ i ] == sOldName && i != nTmpLevel &&
                pCollNames[ i ].isEmpty() )
            {
                sal_uInt8 n;
                for( n = 0; n < MAXLEVEL; ++n )
                    if( pCollNames[ n ] == sOldName )
                        break;

                if( MAXLEVEL == n )
                    // it was an outline level name and the current entries is zero.
                    pCollNames[ i ] = sOldName;
            }

    SetModified();
    CollSave();
}

void SwOutlineSettingsTabPage::CollSave()
{
    for (sal_uInt8 i = 0; i < MAXLEVEL; ++i)
        aSaveCollNames[i] =  pCollNames[i];
}

IMPL_LINK_NOARG(SwOutlineSettingsTabPage, NumberSelect, weld::ComboBox&, void)
{
    sal_uInt16 nMask = 1;
    SvxNumType nNumberType = m_xNumberBox->GetSelectedNumberingType();
    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
    {
        if(nActLevel & nMask)
        {
            SwNumFormat aNumFormat(pNumRule->Get(i));
            aNumFormat.SetNumberingType(nNumberType);
            pNumRule->Set(i, aNumFormat);
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
        if(nActLevel & nMask)
        {
            SwNumFormat aNumFormat(pNumRule->Get(i));
            aNumFormat.SetPrefix( m_xPrefixED->get_text() );
            aNumFormat.SetSuffix( m_xSuffixED->get_text() );
            pNumRule->Set(i, aNumFormat);
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
        if(nActLevel & nMask)
        {
            SwNumFormat aNumFormat(pNumRule->Get(i));
            aNumFormat.SetStart(static_cast<sal_uInt16>(rEdit.get_value()));
            pNumRule->Set(i, aNumFormat);
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
        sal_uInt16 nChCount = pSh->GetCharFormatCount();
        for(sal_uInt16 i = 0; i < nChCount; i++)
        {
            SwCharFormat& rChFormat = pSh->GetCharFormat(i);
            if(rChFormat.GetName() == sEntry)
            {
                pFormat = &rChFormat;
                break;
            }
        }
        if(!pFormat)
        {
            SfxStyleSheetBasePool* pPool = pSh->GetView().GetDocShell()->GetStyleSheetPool();
            SfxStyleSheetBase* pBase;
            pBase = pPool->Find(sEntry, SfxStyleFamily::Char);
            if(!pBase)
                pBase = &pPool->Make(sEntry, SfxStyleFamily::Page);
            pFormat = static_cast<SwDocStyleSheet*>(pBase)->GetCharFormat();

        }
    }

    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
    {
        if(nActLevel & nMask)
        {
            SwNumFormat aNumFormat(pNumRule->Get(i));
            if(bFormatNone)
                aNumFormat.SetCharFormat(nullptr);
            else
                aNumFormat.SetCharFormat(pFormat);
            pNumRule->Set(i, aNumFormat);
        }
        nMask <<= 1;
    }
}

SwOutlineSettingsTabPage::~SwOutlineSettingsTabPage()
{
}

void SwOutlineSettingsTabPage::SetWrtShell(SwWrtShell* pShell)
{
    pSh = pShell;
    // query this document's NumRules
    pNumRule = static_cast<SwOutlineTabDialog*>(GetDialogController())->GetNumRule();
    pCollNames = static_cast<SwOutlineTabDialog*>(GetDialogController())->GetCollNames();

    CollSave();

    m_aPreviewWIN.SetNumRule(pNumRule);
    m_aPreviewWIN.SetOutlineNames(pCollNames);
    // set start value - nActLevel must be 1 here
    sal_uInt16 nTmpLevel = lcl_BitToLevel(nActLevel);
    const SwNumFormat& rNumFormat = pNumRule->Get( nTmpLevel );
    m_xStartEdit->set_value( rNumFormat.GetStart() );

    // create pool formats for headlines
    for (sal_uInt16 i = 0; i < MAXLEVEL; ++i)
    {
        m_xCollBox->append_text( SwStyleNameMapper::GetUIName(
                                    static_cast< sal_uInt16 >(RES_POOLCOLL_HEADLINE1 + i), OUString()));
        m_xLevelLB->append_text( OUString::number(i + 1) );
    }
    OUString sStr("1 - ");
    sStr += OUString::number(MAXLEVEL);
    m_xLevelLB->append_text(sStr);

    // query the texttemplates' outlining levels
    const sal_uInt16 nCount = pSh->GetTextFormatCollCount();
    for (sal_uInt16 i = 0; i < nCount; ++i)
    {
        SwTextFormatColl &rTextColl = pSh->GetTextFormatColl(i);
        if(!rTextColl.IsDefault())
        {
            sStr = rTextColl.GetName();
            if (m_xCollBox->find_text(sStr) == -1)
                m_xCollBox->append_text(sStr);
        }
    }

    m_xNumberBox->SelectNumberingType(rNumFormat.GetNumberingType());
    SwOutlineNodes::size_type nOutlinePos = pSh->GetOutlinePos(MAXLEVEL);
    int nTmp = 0;
    if(nOutlinePos != SwOutlineNodes::npos)
    {
        nTmp = static_cast<sal_uInt16>(pSh->getIDocumentOutlineNodesAccess()->getOutlineLevel(nOutlinePos));
    }
    m_xLevelLB->select(nTmp-1);

    // collect char styles
    m_xCharFormatLB->clear();
    m_xCharFormatLB->append_text(SwViewShell::GetShellRes()->aStrNone);

    // char styles
    ::FillCharStyleListBox(*m_xCharFormatLB,
                        pSh->GetView().GetDocShell());
    Update();
}

void SwOutlineSettingsTabPage::ActivatePage(const SfxItemSet& )
{
    nActLevel = SwOutlineTabDialog::GetActNumLevel();
    if(nActLevel != USHRT_MAX)
        m_xLevelLB->select(lcl_BitToLevel(nActLevel));
    else
        m_xLevelLB->select(MAXLEVEL);
    LevelHdl(*m_xLevelLB);
}

DeactivateRC SwOutlineSettingsTabPage::DeactivatePage(SfxItemSet*)
{
    SwOutlineTabDialog::SetActNumLevel(nActLevel);
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

VclPtr<SfxTabPage> SwOutlineSettingsTabPage::Create(TabPageParent pParent,
                                                    const SfxItemSet* rAttrSet)
{
    return VclPtr<SwOutlineSettingsTabPage>::Create(pParent, *rAttrSet);
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

static long lcl_DrawBullet(vcl::RenderContext* pVDev, const SwNumFormat& rFormat, long nXStart, long nYStart, const Size& rSize)
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
    OUString aText(rFormat.GetBulletChar());
    long nY = nYStart;
    nY -= ((aTmpSize.Height() - rSize.Height())/ 2);
    pVDev->DrawText( Point(nXStart, nY), aText );
    long nRet = pVDev->GetTextWidth(aText);

    pVDev->SetFont(aTmpFont);
    return nRet;
}

static long lcl_DrawGraphic(vcl::RenderContext* pVDev, const SwNumFormat &rFormat, long nXStart, long nYStart, long nDivision)
{
    const SvxBrushItem* pBrushItem = rFormat.GetBrush();
    long nRet = 0;
    if (pBrushItem)
    {
        const Graphic* pGraphic = pBrushItem->GetGraphic();
        if (pGraphic)
        {
            Size aGSize( rFormat.GetGraphicSize());
            aGSize.setWidth( aGSize.Width() / nDivision );
            nRet = aGSize.Width();
            aGSize.setHeight( aGSize.Height() / nDivision );
            pGraphic->Draw(pVDev, Point(nXStart, nYStart), pVDev->PixelToLogic(aGSize));
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

    // #101524# OJ
    pVDev->SetFillColor(rRenderContext.GetSettings().GetStyleSettings().GetWindowColor());
    pVDev->SetLineColor(rRenderContext.GetSettings().GetStyleSettings().GetButtonTextColor());
    pVDev->DrawRect(tools::Rectangle(Point(0,0), aSize));

    if (pActNum)
    {
        long nWidthRelation = 30; // chapter dialog
        if(nPageWidth)
        {
            nWidthRelation = nPageWidth / aSize.Width();
            if(bPosition)
                nWidthRelation = nWidthRelation * 2 / 3;
            else
                nWidthRelation = nWidthRelation / 4;
        }

        // height per level
        const long nXStep = aSize.Width() / (3 * MAXLEVEL * ((MAXLEVEL < 10) ? 2 : 1));
        const long nYStep = (aSize.Height() - 6)/ MAXLEVEL;
        long nYStart = 4;
        aStdFont = OutputDevice::GetDefaultFont(DefaultFontType::UI_SANS, GetAppLanguage(),
                                                GetDefaultFontFlags::OnlyOne, &rRenderContext);
        // #101524# OJ
        aStdFont.SetColor(SwViewOption::GetFontColor());

        const long nFontHeight = nYStep * ( bPosition ? 15 : 6 ) / 10;
        aStdFont.SetFontSize(Size( 0, nFontHeight ));

        long nPreNum = pActNum->Get(0).GetStart();

        if (bPosition)
        {
            const long nLineHeight = nFontHeight * 8 / 7;
            sal_uInt8 nStart = 0;
            while (!(nActLevel & (1 << nStart)))
            {
                nStart++;
            }
            if(nStart) // so that possible predecessors and successors are showed
                nStart--;

            SwNumberTree::tNumberVector aNumVector;
            sal_uInt8 nEnd = std::min(sal_uInt8(nStart + 3), MAXLEVEL);
            for (sal_uInt8 nLevel = nStart; nLevel < nEnd; ++nLevel)
            {
                const SwNumFormat &rFormat = pActNum->Get(nLevel);
                aNumVector.push_back(rFormat.GetStart());

                long nXStart( 0 );
                long nTextOffset( 0 );
                long nNumberXPos( 0 );
                if (rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION)
                {
                    nXStart = rFormat.GetAbsLSpace() / nWidthRelation;
                    nTextOffset = rFormat.GetCharTextDistance() / nWidthRelation;
                    nNumberXPos = nXStart;
                    const long nFirstLineOffset = (-rFormat.GetFirstLineOffset()) / nWidthRelation;

                    if(nFirstLineOffset <= nNumberXPos)
                        nNumberXPos -= nFirstLineOffset;
                    else
                        nNumberXPos = 0;
                }
                else if (rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT)
                {
                    const long nTmpNumberXPos((rFormat.GetIndentAt() + rFormat.GetFirstLineIndent()) / nWidthRelation);
                    nNumberXPos = (nTmpNumberXPos < 0) ? 0 : nTmpNumberXPos;
                }

                long nBulletWidth = 0;
                if (SVX_NUM_BITMAP == rFormat.GetNumberingType())
                {
                    nBulletWidth = lcl_DrawGraphic(pVDev.get(), rFormat, nNumberXPos,
                                                   nYStart, nWidthRelation);
                }
                else if (SVX_NUM_CHAR_SPECIAL == rFormat.GetNumberingType())
                {
                    nBulletWidth = lcl_DrawBullet(pVDev.get(), rFormat, nNumberXPos,
                                                  nYStart, aStdFont.GetFontSize());
                }
                else
                {
                    pVDev->SetFont(aStdFont);
                    if(pActNum->IsContinusNum())
                        aNumVector[nLevel] = nPreNum;
                    OUString aText(pActNum->MakeNumString( aNumVector ));
                    pVDev->DrawText( Point(nNumberXPos, nYStart), aText );
                    nBulletWidth = pVDev->GetTextWidth(aText);
                    nPreNum++;
                }
                if ( rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT &&
                     rFormat.GetLabelFollowedBy() == SvxNumberFormat::SPACE )
                {
                    pVDev->SetFont(aStdFont);
                    OUString aText(' ');
                    pVDev->DrawText( Point(nNumberXPos, nYStart), aText );
                    nBulletWidth += pVDev->GetTextWidth(aText);
                }

                long nTextXPos(0);
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
            const long nLineHeight = nFontHeight * 3 / 2;
            for (sal_uInt8 nLevel = 0; nLevel < MAXLEVEL; ++nLevel, nYStart = nYStart + nYStep)
            {
                const SwNumFormat &rFormat = pActNum->Get(nLevel);
                aNumVector.push_back(rFormat.GetStart());
                long nXStart(0);
                if (rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION)
                {
                    nXStart = rFormat.GetAbsLSpace() / nWidthRelation;
                }
                else if (rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT)
                {
                    const long nTmpXStart((rFormat.GetIndentAt() + rFormat.GetFirstLineIndent() ) / nWidthRelation);
                    nXStart = (nTmpXStart < 0) ? 0 : nTmpXStart;
                }
                nXStart /= 2;
                nXStart += 2;
                long nTextOffset;
                if (SVX_NUM_BITMAP == rFormat.GetNumberingType())
                {
                    lcl_DrawGraphic(pVDev.get(), rFormat, nXStart, nYStart, nWidthRelation);
                    nTextOffset = nLineHeight + nXStep;
                }
                else if (SVX_NUM_CHAR_SPECIAL == rFormat.GetNumberingType())
                {
                    nTextOffset =  lcl_DrawBullet(pVDev.get(), rFormat, nXStart, nYStart, aStdFont.GetFontSize());
                    nTextOffset += nXStep;
                }
                else
                {
                    pVDev->SetFont(aStdFont);
                    if (pActNum->IsContinusNum())
                        aNumVector[nLevel] = nPreNum;
                    OUString aText(pActNum->MakeNumString( aNumVector ));
                    pVDev->DrawText( Point(nXStart, nYStart), aText );
                    nTextOffset = pVDev->GetTextWidth(aText) + nXStep;
                    nPreNum++;
                }
                pVDev->SetFont(aStdFont);
                pVDev->DrawText(
                    Point(nXStart + nTextOffset, nYStart),
                    (pOutlineNames == nullptr
                     ? utl::ConfigManager::getProductName()
                     : pOutlineNames[nLevel]));
            }
        }
    }
    rRenderContext.DrawOutDev(Point(0,0), aSize, Point(0,0), aSize, *pVDev);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
