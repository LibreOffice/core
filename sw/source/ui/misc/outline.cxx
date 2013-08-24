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
#include <tools/shl.hxx>
#include <vcl/menu.hxx>
#include <vcl/msgbox.hxx>
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
#include <globals.hrc>      // for template name 'none'
#include <misc.hrc>
#include <paratr.hxx>

#include <unomid.h>

#include <IDocumentOutlineNodes.hxx>
#include <app.hrc>

#include <com/sun/star/accessibility/AccessibleRole.hpp>

using namespace ::com::sun::star;

DBG_NAME(outlinehdl)

class SwNumNamesDlg : public ModalDialog
{
    Edit*     m_pFormEdit;
    ListBox*  m_pFormBox;
    OKButton* m_pOKBtn;

    DECL_LINK( ModifyHdl, Edit * );
    DECL_LINK( SelectHdl, ListBox * );
    DECL_LINK(DoubleClickHdl, void *);

public:
    SwNumNamesDlg(Window *pParent);
    void SetUserNames(const String *pList[]);
    String GetName() const { return m_pFormEdit->GetText(); }
    sal_uInt16 GetCurEntryPos() const { return m_pFormBox->GetSelectEntryPos(); }
};

/*------------------------------------------------------------------------
 Description:  remember selected entry
------------------------------------------------------------------------*/
IMPL_LINK_INLINE_START( SwNumNamesDlg, SelectHdl, ListBox *, pBox )
{
    m_pFormEdit->SetText(pBox->GetSelectEntry());
    m_pFormEdit->SetSelection(Selection(0, SELECTION_MAX));
    return 0;
}
IMPL_LINK_INLINE_END( SwNumNamesDlg, SelectHdl, ListBox *, pBox )

/*------------------------------------------------------------------------
 Description:  set user defined names
 Parameter:    list of user defined names;
               unknown positions for the user are 0.
------------------------------------------------------------------------*/
void SwNumNamesDlg::SetUserNames(const String *pList[])
{
    sal_uInt16 nSelect = 0;
    for(sal_uInt16 i = 0; i < SwBaseNumRules::nMaxRules; ++i)
    {
        if(pList[i])
        {
            m_pFormBox->RemoveEntry(i);
            m_pFormBox->InsertEntry(*pList[i], i);
            if(i == nSelect && nSelect < SwBaseNumRules::nMaxRules)
                nSelect++;
        }
    }
    m_pFormBox->SelectEntryPos(nSelect);
    SelectHdl(m_pFormBox);
}

/*------------------------------------------------------------------------
 Description:  unlock OK-Button when text is in Edit
------------------------------------------------------------------------*/
IMPL_LINK_INLINE_START( SwNumNamesDlg, ModifyHdl, Edit *, pBox )
{
    m_pOKBtn->Enable(!pBox->GetText().isEmpty());
    return 0;
}
IMPL_LINK_INLINE_END( SwNumNamesDlg, ModifyHdl, Edit *, pBox )

/*------------------------------------------------------------------------
 Description:  DoubleClickHdl
------------------------------------------------------------------------*/
IMPL_LINK_NOARG_INLINE_START(SwNumNamesDlg, DoubleClickHdl)
{
    EndDialog(RET_OK);
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwNumNamesDlg, DoubleClickHdl)

SwNumNamesDlg::SwNumNamesDlg(Window *pParent)
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
    SelectHdl(m_pFormBox);
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
SwOutlineTabDialog::SwOutlineTabDialog(Window* pParent, const SfxItemSet* pSwItemSet,
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
    const sal_uInt16 nCount = rWrtSh.GetTxtFmtCollCount();
    for(i = 0; i < nCount; ++i )
    {
        SwTxtFmtColl &rTxtColl = rWrtSh.GetTxtFmtColl(i);
        if(!rTxtColl.IsDefault())
        {
            //->added by zhaojianwei
            if(rTxtColl.IsAssignedToListLevelOfOutlineStyle())
            {
                int nOutLevel = rTxtColl.GetAssignedOutlineStyleLevel();
                aCollNames[ nOutLevel ] = rTxtColl.GetName();
            }
            //<-end
        }
    }
}

SwOutlineTabDialog::~SwOutlineTabDialog()
{
    delete pNumRule;
}

void SwOutlineTabDialog::PageCreated(sal_uInt16 nPageId, SfxTabPage& rPage)
{
    if (nPageId == m_nNumPosId)
    {
        ((SwNumPositionTabPage&)rPage).SetWrtShell(&rWrtSh);
        ((SwNumPositionTabPage&)rPage).SetOutlineTabDialog(this);
    }
    else if (nPageId == m_nOutlineId)
    {
        ((SwOutlineSettingsTabPage&)rPage).SetWrtShell(&rWrtSh);
    }
}

IMPL_LINK_NOARG(SwOutlineTabDialog, CancelHdl)
{
    if (!bModified)
        rWrtSh.ResetModified();
    EndDialog(RET_CANCEL);
    return 0;
}

IMPL_LINK( SwOutlineTabDialog, FormHdl, Button *, pBtn )
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
    pFormMenu->Execute(pBtn, Rectangle(Point(0,0), pBtn->GetSizePixel()), POPUPMENU_EXECUTE_DOWN);
    return 0;
}

IMPL_LINK( SwOutlineTabDialog, MenuSelectHdl, Menu *, pMenu )
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
        SwNumNamesDlg *pDlg = new SwNumNamesDlg(this);
        const String *aStrArr[SwChapterNumRules::nMaxRules];
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
            const String aName(pDlg->GetName());
            pChapterNumRules->ApplyNumRules( SwNumRulesWithName(
                    *pNumRule, aName ), pDlg->GetCurEntryPos() );
            pMenu->SetItemText(pMenu->GetItemId(pDlg->GetCurEntryPos()), aName);
        }
        delete pDlg;
        return 0;

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
    pPage->Reset(*GetOutputItemSet());

    return 0;
}

sal_uInt16  SwOutlineTabDialog::GetLevel(const String &rFmtName) const
{
    for(sal_uInt16 i = 0; i < MAXLEVEL; ++i)
    {
        if(aCollNames[i] == rFmtName)
            return i;
    }
    return MAXLEVEL;//NO_NUMBERING; //#outline level,zhaojianwei

}

short SwOutlineTabDialog::Ok()
{
    SfxTabDialog::Ok();
    // set levels for all created templates; has to be done in order to
    // delete possibly cancelled assignments again.

    // #130443#
    // encapsulate changes into a action to avoid effects on the current cursor
    // position during the changes.
    rWrtSh.StartAction();

    const SwNumRule * pOutlineRule = rWrtSh.GetOutlineNumRule();

    sal_uInt16 i, nCount = rWrtSh.GetTxtFmtCollCount();
    for( i = 0; i < nCount; ++i )
    {
        SwTxtFmtColl &rTxtColl = rWrtSh.GetTxtFmtColl(i);
        if( !rTxtColl.IsDefault() )
        {

            const SfxPoolItem & rItem =
                rTxtColl.GetFmtAttr(RES_PARATR_NUMRULE, sal_False);

           if ((sal_uInt8)GetLevel(rTxtColl.GetName()) == MAXLEVEL) //add by zhaojianwei
            {
                if(rTxtColl.IsAssignedToListLevelOfOutlineStyle())
                {
                    rTxtColl.DeleteAssignmentToListLevelOfOutlineStyle();
                }
                if (static_cast<const SwNumRuleItem &>(rItem).GetValue() ==
                    pOutlineRule->GetName())
                {
                    rTxtColl.ResetFmtAttr(RES_PARATR_NUMRULE);
                }
            }
            else
            {
                rTxtColl.AssignToListLevelOfOutlineStyle(GetLevel(rTxtColl.GetName()));

                if (static_cast<const SwNumRuleItem &>(rItem).GetValue() !=
                    pOutlineRule->GetName())
                {
                    SwNumRuleItem aItem(pOutlineRule->GetName());
                    rTxtColl.SetFmtAttr(aItem);
                }
            }                           //<-end,zhaojianwei
        }
    }

    for(i = 0; i < MAXLEVEL; ++i )
    {
        OUString sHeadline;
        ::SwStyleNameMapper::FillUIName( static_cast< sal_uInt16 >(RES_POOLCOLL_HEADLINE1 + i),
                                         sHeadline );
        SwTxtFmtColl* pColl = rWrtSh.FindTxtFmtCollByName( sHeadline );
        if( !pColl )
        {
            if(aCollNames[i] != sHeadline)
            {
                SwTxtFmtColl* pTxtColl = rWrtSh.GetTxtCollFromPool(
                    static_cast< sal_uInt16 >(RES_POOLCOLL_HEADLINE1 + i) );
                pTxtColl->DeleteAssignmentToListLevelOfOutlineStyle();
                pTxtColl->ResetFmtAttr(RES_PARATR_NUMRULE);

                if( !aCollNames[i].isEmpty() )
                {
                    pTxtColl = rWrtSh.GetParaStyle(
                                aCollNames[i], SwWrtShell::GETSTYLE_CREATESOME);
                    if(pTxtColl)
                    {
                        pTxtColl->AssignToListLevelOfOutlineStyle(i);
                        SwNumRuleItem aItem(pOutlineRule->GetName());
                        pTxtColl->SetFmtAttr(aItem);
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

SwOutlineSettingsTabPage::SwOutlineSettingsTabPage(Window* pParent,
    const SfxItemSet& rSet)
    : SfxTabPage(pParent, "OutlineNumberingPage",
        "modules/swriter/ui/outlinenumberingpage.ui", rSet)
    , aNoFmtName(SW_RESSTR(SW_STR_NONE))
    , pSh(0)
    , pCollNames(0)
    , nActLevel(1)
{
    get(m_pLevelLB, "level");
    get(m_pCollBox, "style");
    m_pCollBox->SetStyle(m_pCollBox->GetStyle()|WB_SORT);
    get(m_pNumberBox, "numbering");
    get(m_pCharFmtLB, "charstyle");
    get(m_pAllLevelFT, "sublevelsft");
    get(m_pAllLevelNF, "sublevelsnf");
    get(m_pPrefixED, "prefix");
    get(m_pSuffixED, "suffix");
    get(m_pStartEdit, "startat");
    get(m_pPreviewWIN, "preview");

    SetExchangeSupport();

    m_pCollBox->InsertEntry(aNoFmtName);
    m_pLevelLB->SetSelectHdl(LINK(this,    SwOutlineSettingsTabPage, LevelHdl));
    m_pAllLevelNF->SetModifyHdl(LINK(this, SwOutlineSettingsTabPage, ToggleComplete));
    m_pCollBox->SetSelectHdl(LINK(this,    SwOutlineSettingsTabPage, CollSelect));
    m_pCollBox->SetGetFocusHdl(LINK(this,  SwOutlineSettingsTabPage, CollSelectGetFocus));
    m_pNumberBox->SetSelectHdl(LINK(this,  SwOutlineSettingsTabPage, NumberSelect));
    m_pPrefixED->SetModifyHdl(LINK(this,   SwOutlineSettingsTabPage, DelimModify));
    m_pSuffixED->SetModifyHdl(LINK(this,   SwOutlineSettingsTabPage, DelimModify));
    m_pStartEdit->SetModifyHdl(LINK(this,  SwOutlineSettingsTabPage, StartModified));
    m_pCharFmtLB->SetSelectHdl(LINK(this,  SwOutlineSettingsTabPage, CharFmtHdl));
}

void    SwOutlineSettingsTabPage::Update()
{
        // if a template was already selected for this level, select it in the ListBox
    m_pCollBox->Enable(USHRT_MAX != nActLevel);
    if(USHRT_MAX == nActLevel)
    {
        sal_Bool bSamePrefix = sal_True;
        sal_Bool bSameSuffix = sal_True;
        sal_Bool bSameType = sal_True;
        sal_Bool bSameComplete = sal_True;
        sal_Bool bSameStart = sal_True;
        sal_Bool bSameCharFmt = sal_True;

        const SwNumFmt* aNumFmtArr[MAXLEVEL];
        const SwCharFmt* pFirstFmt = 0;

        for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
        {

            aNumFmtArr[ i ] = &pNumRule->Get(i);
            if(i == 0)
                pFirstFmt = aNumFmtArr[i]->GetCharFmt();
            else
            {
                bSameType   &= aNumFmtArr[i]->GetNumberingType() == aNumFmtArr[0]->GetNumberingType();
                bSameStart  &= aNumFmtArr[i]->GetStart() == aNumFmtArr[0]->GetStart();
                bSamePrefix &= aNumFmtArr[i]->GetPrefix() == aNumFmtArr[0]->GetPrefix();
                bSameSuffix &= aNumFmtArr[i]->GetSuffix() == aNumFmtArr[0]->GetSuffix();
                bSameComplete &= aNumFmtArr[i]->GetIncludeUpperLevels() == aNumFmtArr[0]->GetIncludeUpperLevels();
                const SwCharFmt* pFmt = aNumFmtArr[i]->GetCharFmt();
                bSameCharFmt &=     (!pFirstFmt && !pFmt)
                                    || (pFirstFmt && pFmt && pFmt->GetName() == pFirstFmt->GetName());
            }
        }
        CheckForStartValue_Impl(aNumFmtArr[0]->GetNumberingType());
        if(bSameType)
            m_pNumberBox->SelectNumberingType( aNumFmtArr[0]->GetNumberingType() );
        else
            m_pNumberBox->SetNoSelection();
        if(bSameStart)
            m_pStartEdit->SetValue(aNumFmtArr[0]->GetStart());
        else
            m_pStartEdit->SetText(aEmptyStr);
        if(bSamePrefix)
            m_pPrefixED->SetText(aNumFmtArr[0]->GetPrefix());
        else
            m_pPrefixED->SetText(aEmptyStr);
        if(bSameSuffix)
            m_pSuffixED->SetText(aNumFmtArr[0]->GetSuffix());
        else
            m_pSuffixED->SetText(aEmptyStr);

        if(bSameCharFmt)
        {
            if(pFirstFmt)
                m_pCharFmtLB->SelectEntry(pFirstFmt->GetName());
            else
                m_pCharFmtLB->SelectEntry( ViewShell::GetShellRes()->aStrNone );
        }
        else
            m_pCharFmtLB->SetNoSelection();

        m_pAllLevelFT->Enable(sal_True);
        m_pAllLevelNF->Enable(sal_True);
        m_pAllLevelNF->SetMax(MAXLEVEL);
        if(bSameComplete)
        {
            m_pAllLevelNF->SetValue(aNumFmtArr[0]->GetIncludeUpperLevels());
        }
        else
        {
            m_pAllLevelNF->SetText(aEmptyStr);
        }
    }
    else
    {
        sal_uInt16 nTmpLevel = lcl_BitToLevel(nActLevel);
        String aColl(pCollNames[nTmpLevel]);
        if(aColl.Len())
            m_pCollBox->SelectEntry(aColl);
        else
            m_pCollBox->SelectEntry(aNoFmtName);
        const SwNumFmt &rFmt = pNumRule->Get(nTmpLevel);

        m_pNumberBox->SelectNumberingType( rFmt.GetNumberingType() );
        m_pPrefixED->SetText(rFmt.GetPrefix());
        m_pSuffixED->SetText(rFmt.GetSuffix());
        const SwCharFmt* pFmt = rFmt.GetCharFmt();
        if(pFmt)
            m_pCharFmtLB->SelectEntry(pFmt->GetName());
        else
            m_pCharFmtLB->SelectEntry( ViewShell::GetShellRes()->aStrNone );

        if(nTmpLevel)
        {
            m_pAllLevelFT->Enable(sal_True);
            m_pAllLevelNF->Enable(sal_True);
            m_pAllLevelNF->SetMax(nTmpLevel + 1);
            m_pAllLevelNF->SetValue(rFmt.GetIncludeUpperLevels());
        }
        else
        {
            m_pAllLevelNF->SetText(aEmptyStr);
            m_pAllLevelNF->Enable(sal_False);
            m_pAllLevelFT->Enable(sal_False);
        }
        CheckForStartValue_Impl(rFmt.GetNumberingType());
        m_pStartEdit->SetValue( rFmt.GetStart() );
    }
    SetModified();
}

IMPL_LINK( SwOutlineSettingsTabPage, LevelHdl, ListBox *, pBox )
{
    nActLevel = 0;
    if(pBox->IsEntryPosSelected( MAXLEVEL ))
    {
        nActLevel = 0xFFFF;
    }
    else
    {
        sal_uInt16 nMask = 1;
        for( sal_uInt16 i = 0; i < MAXLEVEL; i++ )
        {
            if(pBox->IsEntryPosSelected( i ))
                nActLevel |= nMask;
            nMask <<= 1;
        }
    }
    Update();
    return 0;
}

IMPL_LINK( SwOutlineSettingsTabPage, ToggleComplete, NumericField *, pFld )
{
    sal_uInt16 nMask = 1;
    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
    {
        if(nActLevel & nMask)
        {
            SwNumFmt aNumFmt(pNumRule->Get(i));
            aNumFmt.SetIncludeUpperLevels( std::min( (sal_uInt8)pFld->GetValue(),
                                                (sal_uInt8)(i + 1)) );
            pNumRule->Set(i, aNumFmt);
        }
        nMask <<= 1;
    }
    SetModified();
    return 0;
}

IMPL_LINK( SwOutlineSettingsTabPage, CollSelect, ListBox *, pBox )
{
    sal_uInt8 i;

    const String aCollName(pBox->GetSelectEntry());
    //0xFFFF not allowed here (disable)
    sal_uInt16 nTmpLevel = lcl_BitToLevel(nActLevel);
    String sOldName( pCollNames[nTmpLevel] );

    for( i = 0; i < MAXLEVEL; ++i)
        pCollNames[i] = aSaveCollNames[i];

    if(aCollName == aNoFmtName)
        pCollNames[nTmpLevel] = aEmptyStr;
    else
    {
        pCollNames[nTmpLevel] = aCollName;
                // template already in use?
        for( i = 0; i < MAXLEVEL; ++i)
            if(i != nTmpLevel && pCollNames[i] == aCollName )
                pCollNames[i] = aEmptyStr;
    }

    // search the oldname and put it into the current entries
    if( sOldName.Len() )
        for( i = 0; i < MAXLEVEL; ++i)
            if( aSaveCollNames[ i ] == sOldName && i != nTmpLevel &&
                pCollNames[ i ].isEmpty() )
            {
                sal_uInt8 n;
                for( n = 0; n < MAXLEVEL; ++n )
                    if( pCollNames[ n ] == sOldName )
                        break;

                if( MAXLEVEL == n )
                    // it was a outline leveld name and the current entries is zero.
                    pCollNames[ i ] = sOldName;
            }

    SetModified();
    return 0;
}

IMPL_LINK_NOARG(SwOutlineSettingsTabPage, CollSelectGetFocus)
{
    for( sal_uInt8 i = 0; i < MAXLEVEL; ++i)
        aSaveCollNames[i] =  pCollNames[i];
    return 0;
}

IMPL_LINK( SwOutlineSettingsTabPage, NumberSelect, SwNumberingTypeListBox *, pBox )
{
    sal_uInt16 nMask = 1;
    sal_Int16 nNumberType = pBox->GetSelectedNumberingType();
    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
    {
        if(nActLevel & nMask)
        {
            SwNumFmt aNumFmt(pNumRule->Get(i));
            aNumFmt.SetNumberingType(nNumberType);
            pNumRule->Set(i, aNumFmt);
            CheckForStartValue_Impl(nNumberType);
        }
        nMask <<= 1;
    }
    SetModified();
    return 0;
}

IMPL_LINK_NOARG(SwOutlineSettingsTabPage, DelimModify)
{
    sal_uInt16 nMask = 1;
    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
    {
        if(nActLevel & nMask)
        {
            SwNumFmt aNumFmt(pNumRule->Get(i));
            aNumFmt.SetPrefix( m_pPrefixED->GetText() );
            aNumFmt.SetSuffix( m_pSuffixED->GetText() );
            pNumRule->Set(i, aNumFmt);
        }
        nMask <<= 1;
    }
    SetModified();
    return 0;
}

IMPL_LINK( SwOutlineSettingsTabPage, StartModified, NumericField *, pFld )
{
    sal_uInt16 nMask = 1;
    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
    {
        if(nActLevel & nMask)
        {
            SwNumFmt aNumFmt(pNumRule->Get(i));
            aNumFmt.SetStart( (sal_uInt16)pFld->GetValue() );
            pNumRule->Set(i, aNumFmt);
        }
        nMask <<= 1;
    }
    SetModified();
    return 0;
}

IMPL_LINK_NOARG(SwOutlineSettingsTabPage, CharFmtHdl)
{
    String sEntry = m_pCharFmtLB->GetSelectEntry();
    sal_uInt16 nMask = 1;
    bool bFormatNone = sEntry == ViewShell::GetShellRes()->aStrNone;
    SwCharFmt* pFmt = 0;
    if(!bFormatNone)
    {
        sal_uInt16 nChCount = pSh->GetCharFmtCount();
        for(sal_uInt16 i = 0; i < nChCount; i++)
        {
            SwCharFmt& rChFmt = pSh->GetCharFmt(i);
            if(rChFmt.GetName() == sEntry)
            {
                pFmt = &rChFmt;
                break;
            }
        }
        if(!pFmt)
        {
            SfxStyleSheetBasePool* pPool = pSh->GetView().GetDocShell()->GetStyleSheetPool();
            SfxStyleSheetBase* pBase;
            pBase = pPool->Find(sEntry, SFX_STYLE_FAMILY_CHAR);
            if(!pBase)
                pBase = &pPool->Make(sEntry, SFX_STYLE_FAMILY_PAGE);
            pFmt = ((SwDocStyleSheet*)pBase)->GetCharFmt();

        }
    }

    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
    {
        if(nActLevel & nMask)
        {
            SwNumFmt aNumFmt(pNumRule->Get(i));
            if(bFormatNone)
                aNumFmt.SetCharFmt(0);
            else
                aNumFmt.SetCharFmt(pFmt);
            pNumRule->Set(i, aNumFmt);
        }
        nMask <<= 1;
    }
    return RET_OK;
}

SwOutlineSettingsTabPage::~SwOutlineSettingsTabPage()
{
}

void SwOutlineSettingsTabPage::SetWrtShell(SwWrtShell* pShell)
{
    pSh = pShell;
    // query this document's NumRules
    pNumRule = ((SwOutlineTabDialog*)GetTabDialog())->GetNumRule();
    pCollNames = ((SwOutlineTabDialog*)GetTabDialog())->GetCollNames();

    m_pPreviewWIN->SetNumRule(pNumRule);
    m_pPreviewWIN->SetOutlineNames(pCollNames);
    // set start value - nActLevel must be 1 here
    sal_uInt16 nTmpLevel = lcl_BitToLevel(nActLevel);
    const SwNumFmt& rNumFmt = pNumRule->Get( nTmpLevel );
    m_pStartEdit->SetValue( rNumFmt.GetStart() );

    // create pool formats for headlines
    String sStr;
    sal_uInt16 i;
    for( i = 0; i < MAXLEVEL; ++i )
    {
        m_pCollBox->InsertEntry( SwStyleNameMapper::GetUIName(
                                    static_cast< sal_uInt16 >(RES_POOLCOLL_HEADLINE1 + i), aEmptyStr ));
        m_pLevelLB->InsertEntry( OUString::number(i + 1) );
    }
    sStr.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "1 - " ));
    sStr += OUString::number(MAXLEVEL);
    m_pLevelLB->InsertEntry( sStr );

    // query the texttemplates' outlining levels
    const sal_uInt16 nCount = pSh->GetTxtFmtCollCount();
    for( i = 0; i < nCount; ++i )
    {
        SwTxtFmtColl &rTxtColl = pSh->GetTxtFmtColl(i);
        if(!rTxtColl.IsDefault())
        {
            sStr = rTxtColl.GetName();
            if(LISTBOX_ENTRY_NOTFOUND == m_pCollBox->GetEntryPos( sStr ))
                m_pCollBox->InsertEntry( sStr );
        }
    }

    m_pNumberBox->SelectNumberingType(rNumFmt.GetNumberingType());
    sal_uInt16 nOutlinePos = pSh->GetOutlinePos(MAXLEVEL);
    sal_uInt16 nTmp = 0;
    if(nOutlinePos != USHRT_MAX)
    {
        nTmp = static_cast<sal_uInt16>(pSh->getIDocumentOutlineNodesAccess()->getOutlineLevel(nOutlinePos));
    }
    m_pLevelLB->SelectEntryPos(nTmp-1);//nTmp);//#outline level,zhaojianwei

    // collect char styles
    m_pCharFmtLB->Clear();
    m_pCharFmtLB->InsertEntry( ViewShell::GetShellRes()->aStrNone );

    // char styles
    ::FillCharStyleListBox(*m_pCharFmtLB,
                        pSh->GetView().GetDocShell());
    Update();
}

void    SwOutlineSettingsTabPage::ActivatePage(const SfxItemSet& )
{
    nActLevel = SwOutlineTabDialog::GetActNumLevel();
    if(nActLevel != USHRT_MAX)
        m_pLevelLB->SelectEntryPos(lcl_BitToLevel(nActLevel));
    else
        m_pLevelLB->SelectEntryPos(MAXLEVEL);
    LevelHdl(m_pLevelLB);
}

int     SwOutlineSettingsTabPage::DeactivatePage(SfxItemSet*)
{
    SwOutlineTabDialog::SetActNumLevel(nActLevel);
    return LEAVE_PAGE;
}

sal_Bool    SwOutlineSettingsTabPage::FillItemSet( SfxItemSet&  )
{
    return sal_True;
}

void    SwOutlineSettingsTabPage::Reset( const SfxItemSet& rSet )
{
    ActivatePage(rSet);
}

SfxTabPage* SwOutlineSettingsTabPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet)
{
    return new SwOutlineSettingsTabPage(pParent, rAttrSet);
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

static sal_uInt16 lcl_DrawBullet(VirtualDevice* pVDev,
            const SwNumFmt& rFmt, sal_uInt16 nXStart,
            sal_uInt16 nYStart, const Size& rSize)
{
    Font aTmpFont(pVDev->GetFont());

    Font aFont(*rFmt.GetBulletFont());
    aFont.SetSize(rSize);
    aFont.SetTransparent(sal_True);
    pVDev->SetFont( aFont );
    OUString aText(rFmt.GetBulletChar());
    pVDev->DrawText( Point(nXStart, nYStart), aText );
    sal_uInt16 nRet = (sal_uInt16)pVDev->GetTextWidth(aText);

    pVDev->SetFont(aTmpFont);
    return nRet;
}

static sal_uInt16 lcl_DrawGraphic(VirtualDevice* pVDev, const SwNumFmt &rFmt, sal_uInt16 nXStart,
                        sal_uInt16 nYStart, sal_uInt16 nDivision)
{
    const SvxBrushItem* pBrushItem = rFmt.GetBrush();
    sal_uInt16 nRet = 0;
    if(pBrushItem)
    {
        const Graphic* pGrf = pBrushItem->GetGraphic();
        if(pGrf)
        {
            Size aGSize( rFmt.GetGraphicSize());
            aGSize.Width() /= nDivision;
            nRet = (sal_uInt16)aGSize.Width();
            aGSize.Height() /= nDivision;
            pGrf->Draw( pVDev, Point(nXStart,nYStart),
                    pVDev->PixelToLogic( aGSize ) );
        }
    }
    return nRet;

}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeNumberingPreview(Window *pParent, VclBuilder::stringmap &)
{
    return new NumberingPreview(pParent);
}

/*--------------------------------------------------
    paint numbering's preview
--------------------------------------------------*/
void    NumberingPreview::Paint( const Rectangle& /*rRect*/ )
{
    Size aSize(PixelToLogic(GetOutputSizePixel()));
    Rectangle aRect(Point(0,0), aSize);

    VirtualDevice* pVDev = new VirtualDevice(*this);
    pVDev->SetMapMode(GetMapMode());
    pVDev->SetOutputSize( aSize );

    // #101524# OJ
    pVDev->SetFillColor( GetSettings().GetStyleSettings().GetWindowColor() );
    pVDev->SetLineColor( GetSettings().GetStyleSettings().GetButtonTextColor() );
    pVDev->DrawRect(aRect);

    if(pActNum)
    {
        sal_uInt16 nWidthRelation;
        if(nPageWidth)
        {
            nWidthRelation = sal_uInt16 (nPageWidth / aSize.Width());
            if(bPosition)
                nWidthRelation = nWidthRelation * 2 / 3;
            else
                nWidthRelation = nWidthRelation / 4;
        }
        else
            nWidthRelation = 30; // chapter dialog

        // height per level
        sal_uInt16 nXStep = sal_uInt16(aSize.Width() / (3 * MAXLEVEL));
        if(MAXLEVEL < 10)
            nXStep /= 2;
        sal_uInt16 nYStart = 4;
        sal_uInt16 nYStep = sal_uInt16((aSize.Height() - 6)/ MAXLEVEL);
        aStdFont = OutputDevice::GetDefaultFont(
                                    DEFAULTFONT_UI_SANS, GetAppLanguage(),
                                    DEFAULTFONT_FLAGS_ONLYONE, this );
        // #101524# OJ
        aStdFont.SetColor( SwViewOption::GetFontColor() );

        sal_uInt16 nFontHeight = nYStep * 6 / 10;
        if(bPosition)
            nFontHeight = nYStep * 15 / 10;
        aStdFont.SetSize(Size( 0, nFontHeight ));

        sal_uInt16 nPreNum = pActNum->Get(0).GetStart();

        if(bPosition)
        {
            sal_uInt16 nLineHeight = nFontHeight * 8 / 7;
            sal_uInt8 nStart = 0;
            while( !(nActLevel & (1<<nStart)) )
            {
                nStart++;
            }
            if(nStart) // so that possible predecessors and successors are showed
                nStart--;

            SwNumberTree::tNumberVector aNumVector;
            sal_uInt8 nEnd = std::min( (sal_uInt8)(nStart + 3), MAXLEVEL );
            for( sal_uInt8 nLevel = nStart; nLevel < nEnd; ++nLevel )
            {
                const SwNumFmt &rFmt = pActNum->Get(nLevel);
                aNumVector.push_back(rFmt.GetStart());

                sal_uInt16 nXStart( 0 );
                short nTextOffset( 0 );
                sal_uInt16 nNumberXPos( 0 );
                if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
                {
                    nXStart = rFmt.GetAbsLSpace() / nWidthRelation;
                    nTextOffset = rFmt.GetCharTextDistance() / nWidthRelation;
                    nNumberXPos = nXStart;
                    sal_uInt16 nFirstLineOffset = (-rFmt.GetFirstLineOffset()) / nWidthRelation;

                    if(nFirstLineOffset <= nNumberXPos)
                        nNumberXPos = nNumberXPos - nFirstLineOffset;
                    else
                        nNumberXPos = 0;
                }
                else if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
                {
                    const long nTmpNumberXPos( ( rFmt.GetIndentAt() +
                                                 rFmt.GetFirstLineIndent() ) /
                                               nWidthRelation );
                    if ( nTmpNumberXPos < 0 )
                    {
                        nNumberXPos = 0;
                    }
                    else
                    {
                        nNumberXPos = static_cast<sal_uInt16>(nTmpNumberXPos);
                    }
                }

                sal_uInt16 nBulletWidth = 0;
                if( SVX_NUM_BITMAP == rFmt.GetNumberingType() )
                {
                    nBulletWidth = lcl_DrawGraphic(pVDev, rFmt,
                                        nNumberXPos,
                                            nYStart, nWidthRelation);
                }
                else if( SVX_NUM_CHAR_SPECIAL == rFmt.GetNumberingType() )
                {
                    nBulletWidth =  lcl_DrawBullet(pVDev, rFmt, nNumberXPos, nYStart, aStdFont.GetSize());
                }
                else
                {
                    pVDev->SetFont(aStdFont);
                    if(pActNum->IsContinusNum())
                        aNumVector[nLevel] = nPreNum;
                    // #128041#
                    String aText(pActNum->MakeNumString( aNumVector ));
                    pVDev->DrawText( Point(nNumberXPos, nYStart), aText );
                    nBulletWidth = (sal_uInt16)pVDev->GetTextWidth(aText);
                    nPreNum++;
                }
                if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT &&
                     rFmt.GetLabelFollowedBy() == SvxNumberFormat::SPACE )
                {
                    pVDev->SetFont(aStdFont);
                    OUString aText(' ');
                    pVDev->DrawText( Point(nNumberXPos, nYStart), aText );
                    nBulletWidth = nBulletWidth + (sal_uInt16)pVDev->GetTextWidth(aText);
                }

                sal_uInt16 nTextXPos( 0 );
                if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
                {
                    nTextXPos = nXStart;
                    if(nTextOffset < 0)
                         nTextXPos = nTextXPos + nTextOffset;
                    if(nNumberXPos + nBulletWidth + nTextOffset > nTextXPos )
                        nTextXPos = nNumberXPos + nBulletWidth + nTextOffset;
                }
                else if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
                {
                    switch ( rFmt.GetLabelFollowedBy() )
                    {
                        case SvxNumberFormat::LISTTAB:
                        {
                            nTextXPos = static_cast<sal_uInt16>(
                                            rFmt.GetListtabPos() / nWidthRelation );
                            if ( nTextXPos < nNumberXPos + nBulletWidth )
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

                    nXStart = static_cast<sal_uInt16>( rFmt.GetIndentAt() / nWidthRelation );
                }


                Rectangle aRect1(Point(nTextXPos, nYStart + nFontHeight / 2), Size(aSize.Width() / 2, 2));
                pVDev->SetFillColor( GetSettings().GetStyleSettings().GetWindowColor() ); // Color( COL_BLACK ) );
                pVDev->DrawRect( aRect1 );

                Rectangle aRect2(Point(nXStart, nYStart + nLineHeight + nFontHeight / 2 ), Size(aSize.Width() / 2, 2));
                pVDev->DrawRect( aRect2 );
                nYStart += 2 * nLineHeight;
            }
        }
        else
        {
            SwNumberTree::tNumberVector aNumVector;
            sal_uInt16 nLineHeight = nFontHeight * 3 / 2;
            for( sal_uInt8 nLevel = 0; nLevel < MAXLEVEL;
                            ++nLevel, nYStart = nYStart + nYStep )
            {
                const SwNumFmt &rFmt = pActNum->Get(nLevel);
                aNumVector.push_back(rFmt.GetStart());
                sal_uInt16 nXStart( 0 );
                if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
                {
                    nXStart = rFmt.GetAbsLSpace() / nWidthRelation;
                }
                else if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
                {
                    const long nTmpXStart( ( rFmt.GetIndentAt() +
                                             rFmt.GetFirstLineIndent() ) /
                                           nWidthRelation );
                    if ( nTmpXStart < 0 )
                    {
                        nXStart = 0;
                    }
                    else
                    {
                        nXStart = static_cast<sal_uInt16>(nTmpXStart);
                    }
                }
                nXStart /= 2;
                nXStart += 2;
                sal_uInt16 nTextOffset = 2 * nXStep;
                if( SVX_NUM_BITMAP == rFmt.GetNumberingType() )
                {
                    lcl_DrawGraphic(pVDev, rFmt, nXStart, nYStart, nWidthRelation);
                    nTextOffset = nLineHeight + nXStep;
                }
                else if( SVX_NUM_CHAR_SPECIAL == rFmt.GetNumberingType() )
                {
                    nTextOffset =  lcl_DrawBullet(pVDev, rFmt, nXStart, nYStart, aStdFont.GetSize());
                    nTextOffset = nTextOffset + nXStep;
                }
                else
                {
                    pVDev->SetFont(aStdFont);
                    if(pActNum->IsContinusNum())
                        aNumVector[nLevel] = nPreNum;
                    // #128041#
                    String aText(pActNum->MakeNumString( aNumVector ));
                    pVDev->DrawText( Point(nXStart, nYStart), aText );
                    nTextOffset = (sal_uInt16)pVDev->GetTextWidth(aText);
                    nTextOffset = nTextOffset + nXStep;
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
    DrawOutDev( Point(0,0), aSize,
                Point(0,0), aSize,
                        *pVDev );
    delete pVDev;

}

NumberingPreview::~NumberingPreview()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
