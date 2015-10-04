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
#include <vcl/msgbox.hxx>
#include <vcl/settings.hxx>
#include <vcl/builderfactory.hxx>

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
#include <helpid.h>
#include <globals.hrc>
#include <misc.hrc>
#include <paratr.hxx>

#include <unomid.h>

#include <IDocumentOutlineNodes.hxx>
#include <app.hrc>

#include <com/sun/star/accessibility/AccessibleRole.hpp>

using namespace ::com::sun::star;

class SwNumNamesDlg : public ModalDialog
{
    VclPtr<Edit>     m_pFormEdit;
    VclPtr<ListBox>  m_pFormBox;
    VclPtr<OKButton> m_pOKBtn;

    DECL_LINK( ModifyHdl, Edit * );
    DECL_LINK_TYPED( SelectHdl, ListBox&, void );
    DECL_LINK_TYPED( DoubleClickHdl, ListBox&, void );

public:
    explicit SwNumNamesDlg(vcl::Window *pParent);
    virtual ~SwNumNamesDlg();
    virtual void dispose() SAL_OVERRIDE;
    void SetUserNames(const OUString *pList[]);
    OUString GetName() const { return m_pFormEdit->GetText(); }
    sal_Int32 GetCurEntryPos() const { return m_pFormBox->GetSelectEntryPos(); }
};

SwNumNamesDlg::~SwNumNamesDlg()
{
    disposeOnce();
}

void SwNumNamesDlg::dispose()
{
    m_pFormEdit.clear();
    m_pFormBox.clear();
    m_pOKBtn.clear();
    ModalDialog::dispose();
}


// remember selected entry
IMPL_LINK_TYPED( SwNumNamesDlg, SelectHdl, ListBox&, rBox, void )
{
    m_pFormEdit->SetText(rBox.GetSelectEntry());
    m_pFormEdit->SetSelection(Selection(0, SELECTION_MAX));
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
            m_pFormBox->RemoveEntry(i);
            m_pFormBox->InsertEntry(*pList[i], i);
            if (i == nSelect && nSelect < SwChapterNumRules::nMaxRules)
                nSelect++;
        }
    }
    m_pFormBox->SelectEntryPos(nSelect);
    SelectHdl(*m_pFormBox);
}

// unlock OK-Button when text is in Edit
IMPL_LINK( SwNumNamesDlg, ModifyHdl, Edit *, pBox )
{
    m_pOKBtn->Enable(!pBox->GetText().isEmpty());
    return 0;
}

// DoubleClickHdl
IMPL_LINK_NOARG_TYPED(SwNumNamesDlg, DoubleClickHdl, ListBox&, void)
{
    EndDialog(RET_OK);
}

SwNumNamesDlg::SwNumNamesDlg(vcl::Window *pParent)
    : ModalDialog(pParent, "NumberingNameDialog",
        "modules/swriter/ui/numberingnamedialog.ui")
{
    get(m_pFormEdit, "entry");
    get(m_pFormBox, "form");
    m_pFormBox->SetDropDownLineCount(5);
    get(m_pOKBtn, "ok");
    m_pFormEdit->SetModifyHdl(LINK(this, SwNumNamesDlg, ModifyHdl));
    m_pFormBox->SetSelectHdl(LINK(this, SwNumNamesDlg, SelectHdl));
    m_pFormBox->SetDoubleClickHdl(LINK(this, SwNumNamesDlg, DoubleClickHdl));
    SelectHdl(*m_pFormBox);
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

SwOutlineTabDialog::SwOutlineTabDialog(vcl::Window* pParent, const SfxItemSet* pSwItemSet,
    SwWrtShell &rSh)
    : SfxTabDialog(pParent, "OutlineNumberingDialog",
        "modules/swriter/ui/outlinenumbering.ui", pSwItemSet)
    , rWrtSh(rSh)
    , pChapterNumRules(SW_MOD()->GetChapterNumRules())
    , bModified(rWrtSh.IsModified())
{
    PushButton* pUserButton = GetUserButton();
    pUserButton->SetClickHdl(LINK(this, SwOutlineTabDialog, FormHdl));
    pUserButton->SetAccessibleRole( com::sun::star::accessibility::AccessibleRole::BUTTON_MENU );

    pNumRule = new SwNumRule( *rSh.GetOutlineNumRule() );
    GetCancelButton().SetClickHdl(LINK(this, SwOutlineTabDialog, CancelHdl));

    m_nNumPosId = AddTabPage("position", &SwNumPositionTabPage::Create, 0);
    m_nOutlineId = AddTabPage("numbering", &SwOutlineSettingsTabPage::Create, 0);

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
    disposeOnce();
}

void SwOutlineTabDialog::dispose()
{
    delete pNumRule;
    SfxTabDialog::dispose();
}

void SwOutlineTabDialog::PageCreated(sal_uInt16 nPageId, SfxTabPage& rPage)
{
    if (nPageId == m_nNumPosId)
    {
        static_cast<SwNumPositionTabPage&>(rPage).SetWrtShell(&rWrtSh);
        static_cast<SwNumPositionTabPage&>(rPage).SetOutlineTabDialog(this);
    }
    else if (nPageId == m_nOutlineId)
    {
        static_cast<SwOutlineSettingsTabPage&>(rPage).SetWrtShell(&rWrtSh);
    }
}

IMPL_LINK_NOARG_TYPED(SwOutlineTabDialog, CancelHdl, Button*, void)
{
    if (!bModified)
        rWrtSh.ResetModified();
    EndDialog();
}

IMPL_LINK_TYPED( SwOutlineTabDialog, FormHdl, Button *, pBtn, void )
{
    PopupMenu *pFormMenu = get_menu("form");
    // fill PopupMenu
    for( sal_uInt16 i = 0; i < SwChapterNumRules::nMaxRules; ++i )
    {
        const SwNumRulesWithName *pRules = pChapterNumRules->GetRules(i);
        if( pRules )
            pFormMenu->SetItemText(pFormMenu->GetItemId(i), pRules->GetName());
    }

    OString sHelpId(pFormMenu->GetHelpId(pFormMenu->GetItemId("form1")));
    for (sal_Int32 i = 2; i <= 9; ++i)
    {
        OString sForm("form");
        pFormMenu->SetHelpId(pFormMenu->GetItemId(sForm + OString::number(i)), sHelpId);
    }

    pFormMenu->SetSelectHdl(LINK(this, SwOutlineTabDialog, MenuSelectHdl));
    pFormMenu->Execute(pBtn, Rectangle(Point(0,0), pBtn->GetSizePixel()), PopupMenuFlags::ExecuteDown);
}

IMPL_LINK_TYPED( SwOutlineTabDialog, MenuSelectHdl, Menu *, pMenu, bool )
{
    sal_uInt8 nLevelNo = 0;
    OString sIdent = pMenu->GetCurItemIdent();

    if (sIdent == "form1")
        nLevelNo = 1;
    else if (sIdent == "form2")
        nLevelNo = 2;
    else if (sIdent == "form3")
        nLevelNo = 3;
    else if (sIdent == "form4")
        nLevelNo = 4;
    else if (sIdent == "form5")
        nLevelNo = 5;
    else if (sIdent == "form6")
        nLevelNo = 6;
    else if (sIdent == "form7")
        nLevelNo = 7;
    else if (sIdent == "form8")
        nLevelNo = 8;
    else if (sIdent == "form9")
        nLevelNo = 9;
    else if (sIdent == "saveas")
    {
        VclPtrInstance< SwNumNamesDlg > pDlg(this);
        const OUString *aStrArr[SwChapterNumRules::nMaxRules];
        for(sal_uInt16 i = 0; i < SwChapterNumRules::nMaxRules; ++i)
        {
            const SwNumRulesWithName *pRules = pChapterNumRules->GetRules(i);
            if(pRules)
                aStrArr[i] = &pRules->GetName();
            else
                aStrArr[i] = 0;
        }
        pDlg->SetUserNames(aStrArr);
        if(RET_OK == pDlg->Execute())
        {
            const OUString aName(pDlg->GetName());
            pChapterNumRules->ApplyNumRules( SwNumRulesWithName(
                    *pNumRule, aName ), pDlg->GetCurEntryPos() );
            pMenu->SetItemText(pMenu->GetItemId(pDlg->GetCurEntryPos()), aName);
        }
        return false;
    }

    if( nLevelNo-- )
    {
        const SwNumRulesWithName *pRules = pChapterNumRules->GetRules( nLevelNo );
        if( pRules )
        {
            pRules->MakeNumRule( rWrtSh, *pNumRule );
            pNumRule->SetRuleType( OUTLINE_RULE );
        }
        else
            *pNumRule = *rWrtSh.GetOutlineNumRule();
    }

    sal_uInt16  nPageId = GetCurPageId();
    SfxTabPage* pPage = GetTabPage( nPageId );
    pPage->Reset(GetOutputItemSet());

    return false;
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
    SfxTabDialog::Ok();
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

           if ((sal_uInt8)GetLevel(rTextColl.GetName()) == MAXLEVEL)
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
        if( !pColl )
        {
            if(aCollNames[i] != sHeadline)
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
    }

    rWrtSh.SetOutlineNumRule( *pNumRule);

    // #i30443#
    rWrtSh.EndAction();

    return RET_OK;
}

SwOutlineSettingsTabPage::SwOutlineSettingsTabPage(vcl::Window* pParent,
    const SfxItemSet& rSet)
    : SfxTabPage(pParent, "OutlineNumberingPage",
        "modules/swriter/ui/outlinenumberingpage.ui", &rSet)
    , aNoFormatName(SW_RESSTR(SW_STR_NONE))
    , pSh(NULL)
    , pNumRule(NULL)
    , pCollNames(NULL)
    , nActLevel(1)
{
    get(m_pLevelLB, "level");
    get(m_pCollBox, "style");
    m_pCollBox->SetStyle(m_pCollBox->GetStyle()|WB_SORT);
    get(m_pNumberBox, "numbering");
    get(m_pCharFormatLB, "charstyle");
    get(m_pAllLevelFT, "sublevelsft");
    get(m_pAllLevelNF, "sublevelsnf");
    get(m_pPrefixED, "prefix");
    get(m_pSuffixED, "suffix");
    get(m_pStartEdit, "startat");
    get(m_pPreviewWIN, "preview");

    SetExchangeSupport();

    m_pCollBox->InsertEntry(aNoFormatName);
    m_pLevelLB->SetSelectHdl(LINK(this,    SwOutlineSettingsTabPage, LevelHdl));
    m_pAllLevelNF->SetModifyHdl(LINK(this, SwOutlineSettingsTabPage, ToggleComplete));
    m_pCollBox->SetSelectHdl(LINK(this,    SwOutlineSettingsTabPage, CollSelect));
    m_pCollBox->SetGetFocusHdl(LINK(this,  SwOutlineSettingsTabPage, CollSelectGetFocus));
    m_pNumberBox->SetSelectHdl(LINK(this,  SwOutlineSettingsTabPage, NumberSelect));
    m_pPrefixED->SetModifyHdl(LINK(this,   SwOutlineSettingsTabPage, DelimModify));
    m_pSuffixED->SetModifyHdl(LINK(this,   SwOutlineSettingsTabPage, DelimModify));
    m_pStartEdit->SetModifyHdl(LINK(this,  SwOutlineSettingsTabPage, StartModified));
    m_pCharFormatLB->SetSelectHdl(LINK(this,  SwOutlineSettingsTabPage, CharFormatHdl));
}

void    SwOutlineSettingsTabPage::Update()
{
        // if a template was already selected for this level, select it in the ListBox
    m_pCollBox->Enable(USHRT_MAX != nActLevel);
    if(USHRT_MAX == nActLevel)
    {
        bool bSamePrefix = true;
        bool bSameSuffix = true;
        bool bSameType = true;
        bool bSameComplete = true;
        bool bSameStart = true;
        bool bSameCharFormat = true;

        const SwNumFormat* aNumFormatArr[MAXLEVEL];
        const SwCharFormat* pFirstFormat = 0;

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
        if(bSameType)
            m_pNumberBox->SelectNumberingType( aNumFormatArr[0]->GetNumberingType() );
        else
            m_pNumberBox->SetNoSelection();
        if(bSameStart)
            m_pStartEdit->SetValue(aNumFormatArr[0]->GetStart());
        else
            m_pStartEdit->SetText(OUString());
        if(bSamePrefix)
            m_pPrefixED->SetText(aNumFormatArr[0]->GetPrefix());
        else
            m_pPrefixED->SetText(OUString());
        if(bSameSuffix)
            m_pSuffixED->SetText(aNumFormatArr[0]->GetSuffix());
        else
            m_pSuffixED->SetText(OUString());

        if(bSameCharFormat)
        {
            if(pFirstFormat)
                m_pCharFormatLB->SelectEntry(pFirstFormat->GetName());
            else
                m_pCharFormatLB->SelectEntry( SwViewShell::GetShellRes()->aStrNone );
        }
        else
            m_pCharFormatLB->SetNoSelection();

        m_pAllLevelFT->Enable(true);
        m_pAllLevelNF->Enable(true);
        m_pAllLevelNF->SetMax(MAXLEVEL);
        if(bSameComplete)
        {
            m_pAllLevelNF->SetValue(aNumFormatArr[0]->GetIncludeUpperLevels());
        }
        else
        {
            m_pAllLevelNF->SetText(OUString());
        }
    }
    else
    {
        sal_uInt16 nTmpLevel = lcl_BitToLevel(nActLevel);
        OUString aColl(pCollNames[nTmpLevel]);
        if(!aColl.isEmpty())
            m_pCollBox->SelectEntry(aColl);
        else
            m_pCollBox->SelectEntry(aNoFormatName);
        const SwNumFormat &rFormat = pNumRule->Get(nTmpLevel);

        m_pNumberBox->SelectNumberingType( rFormat.GetNumberingType() );
        m_pPrefixED->SetText(rFormat.GetPrefix());
        m_pSuffixED->SetText(rFormat.GetSuffix());
        const SwCharFormat* pFormat = rFormat.GetCharFormat();
        if(pFormat)
            m_pCharFormatLB->SelectEntry(pFormat->GetName());
        else
            m_pCharFormatLB->SelectEntry( SwViewShell::GetShellRes()->aStrNone );

        if(nTmpLevel)
        {
            m_pAllLevelFT->Enable(true);
            m_pAllLevelNF->Enable(true);
            m_pAllLevelNF->SetMax(nTmpLevel + 1);
            m_pAllLevelNF->SetValue(rFormat.GetIncludeUpperLevels());
        }
        else
        {
            m_pAllLevelNF->SetText(OUString());
            m_pAllLevelNF->Enable(false);
            m_pAllLevelFT->Enable(false);
        }
        CheckForStartValue_Impl(rFormat.GetNumberingType());
        m_pStartEdit->SetValue( rFormat.GetStart() );
    }
    SetModified();
}

IMPL_LINK_TYPED( SwOutlineSettingsTabPage, LevelHdl, ListBox&, rBox, void )
{
    nActLevel = 0;
    if(rBox.IsEntryPosSelected( MAXLEVEL ))
    {
        nActLevel = 0xFFFF;
    }
    else
    {
        sal_uInt16 nMask = 1;
        for( sal_uInt16 i = 0; i < MAXLEVEL; i++ )
        {
            if(rBox.IsEntryPosSelected( i ))
                nActLevel |= nMask;
            nMask <<= 1;
        }
    }
    Update();
}

IMPL_LINK( SwOutlineSettingsTabPage, ToggleComplete, NumericField *, pField )
{
    sal_uInt16 nMask = 1;
    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
    {
        if(nActLevel & nMask)
        {
            SwNumFormat aNumFormat(pNumRule->Get(i));
            aNumFormat.SetIncludeUpperLevels( std::min( (sal_uInt8)pField->GetValue(),
                                                (sal_uInt8)(i + 1)) );
            pNumRule->Set(i, aNumFormat);
        }
        nMask <<= 1;
    }
    SetModified();
    return 0;
}

IMPL_LINK_TYPED( SwOutlineSettingsTabPage, CollSelect, ListBox&, rBox, void )
{
    sal_uInt8 i;

    const OUString aCollName(rBox.GetSelectEntry());
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
}

IMPL_LINK_NOARG_TYPED(SwOutlineSettingsTabPage, CollSelectGetFocus, Control&, void)
{
    for( sal_uInt8 i = 0; i < MAXLEVEL; ++i)
        aSaveCollNames[i] =  pCollNames[i];
}

IMPL_LINK_TYPED( SwOutlineSettingsTabPage, NumberSelect, ListBox&, rBox, void )
{
    sal_uInt16 nMask = 1;
    sal_Int16 nNumberType = static_cast<SwNumberingTypeListBox&>(rBox).GetSelectedNumberingType();
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

IMPL_LINK_NOARG(SwOutlineSettingsTabPage, DelimModify)
{
    sal_uInt16 nMask = 1;
    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
    {
        if(nActLevel & nMask)
        {
            SwNumFormat aNumFormat(pNumRule->Get(i));
            aNumFormat.SetPrefix( m_pPrefixED->GetText() );
            aNumFormat.SetSuffix( m_pSuffixED->GetText() );
            pNumRule->Set(i, aNumFormat);
        }
        nMask <<= 1;
    }
    SetModified();
    return 0;
}

IMPL_LINK( SwOutlineSettingsTabPage, StartModified, NumericField *, pField )
{
    sal_uInt16 nMask = 1;
    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
    {
        if(nActLevel & nMask)
        {
            SwNumFormat aNumFormat(pNumRule->Get(i));
            aNumFormat.SetStart( (sal_uInt16)pField->GetValue() );
            pNumRule->Set(i, aNumFormat);
        }
        nMask <<= 1;
    }
    SetModified();
    return 0;
}

IMPL_LINK_NOARG_TYPED(SwOutlineSettingsTabPage, CharFormatHdl, ListBox&, void)
{
    OUString sEntry = m_pCharFormatLB->GetSelectEntry();
    sal_uInt16 nMask = 1;
    bool bFormatNone = sEntry == SwViewShell::GetShellRes()->aStrNone;
    SwCharFormat* pFormat = 0;
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
            pBase = pPool->Find(sEntry, SFX_STYLE_FAMILY_CHAR);
            if(!pBase)
                pBase = &pPool->Make(sEntry, SFX_STYLE_FAMILY_PAGE);
            pFormat = static_cast<SwDocStyleSheet*>(pBase)->GetCharFormat();

        }
    }

    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
    {
        if(nActLevel & nMask)
        {
            SwNumFormat aNumFormat(pNumRule->Get(i));
            if(bFormatNone)
                aNumFormat.SetCharFormat(0);
            else
                aNumFormat.SetCharFormat(pFormat);
            pNumRule->Set(i, aNumFormat);
        }
        nMask <<= 1;
    }
}

SwOutlineSettingsTabPage::~SwOutlineSettingsTabPage()
{
    disposeOnce();
}

void SwOutlineSettingsTabPage::dispose()
{
    m_pLevelLB.clear();
    m_pCollBox.clear();
    m_pNumberBox.clear();
    m_pCharFormatLB.clear();
    m_pAllLevelFT.clear();
    m_pAllLevelNF.clear();
    m_pPrefixED.clear();
    m_pSuffixED.clear();
    m_pStartEdit.clear();
    m_pPreviewWIN.clear();
    SfxTabPage::dispose();
}


void SwOutlineSettingsTabPage::SetWrtShell(SwWrtShell* pShell)
{
    pSh = pShell;
    // query this document's NumRules
    pNumRule = static_cast<SwOutlineTabDialog*>(GetTabDialog())->GetNumRule();
    pCollNames = static_cast<SwOutlineTabDialog*>(GetTabDialog())->GetCollNames();

    m_pPreviewWIN->SetNumRule(pNumRule);
    m_pPreviewWIN->SetOutlineNames(pCollNames);
    // set start value - nActLevel must be 1 here
    sal_uInt16 nTmpLevel = lcl_BitToLevel(nActLevel);
    const SwNumFormat& rNumFormat = pNumRule->Get( nTmpLevel );
    m_pStartEdit->SetValue( rNumFormat.GetStart() );

    // create pool formats for headlines
    sal_uInt16 i;
    for( i = 0; i < MAXLEVEL; ++i )
    {
        m_pCollBox->InsertEntry( SwStyleNameMapper::GetUIName(
                                    static_cast< sal_uInt16 >(RES_POOLCOLL_HEADLINE1 + i), OUString()));
        m_pLevelLB->InsertEntry( OUString::number(i + 1) );
    }
    OUString sStr("1 - ");
    sStr += OUString::number(MAXLEVEL);
    m_pLevelLB->InsertEntry( sStr );

    // query the texttemplates' outlining levels
    const sal_uInt16 nCount = pSh->GetTextFormatCollCount();
    for( i = 0; i < nCount; ++i )
    {
        SwTextFormatColl &rTextColl = pSh->GetTextFormatColl(i);
        if(!rTextColl.IsDefault())
        {
            sStr = rTextColl.GetName();
            if(LISTBOX_ENTRY_NOTFOUND == m_pCollBox->GetEntryPos( sStr ))
                m_pCollBox->InsertEntry( sStr );
        }
    }

    m_pNumberBox->SelectNumberingType(rNumFormat.GetNumberingType());
    sal_uInt16 nOutlinePos = pSh->GetOutlinePos(MAXLEVEL);
    sal_uInt16 nTmp = 0;
    if(nOutlinePos != USHRT_MAX)
    {
        nTmp = static_cast<sal_uInt16>(pSh->getIDocumentOutlineNodesAccess()->getOutlineLevel(nOutlinePos));
    }
    m_pLevelLB->SelectEntryPos(nTmp-1);

    // collect char styles
    m_pCharFormatLB->Clear();
    m_pCharFormatLB->InsertEntry( SwViewShell::GetShellRes()->aStrNone );

    // char styles
    ::FillCharStyleListBox(*m_pCharFormatLB,
                        pSh->GetView().GetDocShell());
    Update();
}

void SwOutlineSettingsTabPage::ActivatePage(const SfxItemSet& )
{
    nActLevel = SwOutlineTabDialog::GetActNumLevel();
    if(nActLevel != USHRT_MAX)
        m_pLevelLB->SelectEntryPos(lcl_BitToLevel(nActLevel));
    else
        m_pLevelLB->SelectEntryPos(MAXLEVEL);
    LevelHdl(*m_pLevelLB);
}

SfxTabPage::sfxpg SwOutlineSettingsTabPage::DeactivatePage(SfxItemSet*)
{
    SwOutlineTabDialog::SetActNumLevel(nActLevel);
    return LEAVE_PAGE;
}

bool SwOutlineSettingsTabPage::FillItemSet( SfxItemSet*  )
{
    return true;
}

void SwOutlineSettingsTabPage::Reset( const SfxItemSet* rSet )
{
    ActivatePage(*rSet);
}

VclPtr<SfxTabPage> SwOutlineSettingsTabPage::Create( vcl::Window* pParent,
                                                     const SfxItemSet* rAttrSet)
{
    return VclPtr<SwOutlineSettingsTabPage>::Create(pParent, *rAttrSet);
}

void SwOutlineSettingsTabPage::CheckForStartValue_Impl(sal_uInt16 nNumberingType)
{
    bool bIsNull = m_pStartEdit->GetValue() == 0;
    bool bNoZeroAllowed = nNumberingType < SVX_NUM_ARABIC ||
                        SVX_NUM_CHARS_UPPER_LETTER_N == nNumberingType ||
                        SVX_NUM_CHARS_LOWER_LETTER_N == nNumberingType;
    m_pStartEdit->SetMin(bNoZeroAllowed ? 1 : 0);
    if(bIsNull && bNoZeroAllowed)
        m_pStartEdit->GetModifyHdl().Call(m_pStartEdit);
}

static long lcl_DrawBullet(vcl::RenderContext* pVDev, const SwNumFormat& rFormat, long nXStart, long nYStart, const Size& rSize)
{
    vcl::Font aTmpFont(pVDev->GetFont());

    vcl::Font aFont(*rFormat.GetBulletFont());
    aFont.SetSize(rSize);
    aFont.SetTransparent(true);
    pVDev->SetFont(aFont);
    OUString aText(rFormat.GetBulletChar());
    pVDev->DrawText(Point(nXStart, nYStart), aText);
    const long nRet = pVDev->GetTextWidth(aText);

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
            aGSize.Width() /= nDivision;
            nRet = aGSize.Width();
            aGSize.Height() /= nDivision;
            pGraphic->Draw(pVDev, Point(nXStart, nYStart), pVDev->PixelToLogic(aGSize));
        }
    }
    return nRet;
}

VCL_BUILDER_FACTORY(NumberingPreview)

// paint numbering's preview
void NumberingPreview::Paint(vcl::RenderContext& rRenderContext, const Rectangle& /*rRect*/)
{
    const Size aSize(rRenderContext.PixelToLogic(GetOutputSizePixel()));

    ScopedVclPtrInstance<VirtualDevice> pVDev(rRenderContext);
    pVDev->SetMapMode(rRenderContext.GetMapMode());
    pVDev->SetOutputSize(aSize);

    // #101524# OJ
    pVDev->SetFillColor(rRenderContext.GetSettings().GetStyleSettings().GetWindowColor());
    pVDev->SetLineColor(rRenderContext.GetSettings().GetStyleSettings().GetButtonTextColor());
    pVDev->DrawRect(Rectangle(Point(0,0), aSize));

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
        aStdFont.SetSize(Size( 0, nFontHeight ));

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
                                                  nYStart, aStdFont.GetSize());
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
                        {
                            nTextXPos = nNumberXPos + nBulletWidth;
                        }
                        break;
                    }

                    nXStart = rFormat.GetIndentAt() / nWidthRelation;
                }

                Rectangle aRect1(Point(nTextXPos, nYStart + nFontHeight / 2), Size(aSize.Width() / 2, 2));
                pVDev->SetFillColor(rRenderContext.GetSettings().GetStyleSettings().GetWindowColor()); // Color( COL_BLACK ) );
                pVDev->DrawRect(aRect1);

                Rectangle aRect2(Point(nXStart, nYStart + nLineHeight + nFontHeight / 2), Size(aSize.Width() / 2, 2));
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
                    nTextOffset =  lcl_DrawBullet(pVDev.get(), rFormat, nXStart, nYStart, aStdFont.GetSize());
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
                    (pOutlineNames == 0
                     ? utl::ConfigManager::getProductName()
                     : pOutlineNames[nLevel]));
            }
        }
    }
    rRenderContext.DrawOutDev(Point(0,0), aSize, Point(0,0), aSize, *pVDev);
}

NumberingPreview::~NumberingPreview()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
