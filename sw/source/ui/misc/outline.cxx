/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif
#include <hintids.hxx>
#include <tools/shl.hxx>
#include <vcl/menu.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/tabdlg.hxx>
#include <editeng/brshitem.hxx>
#include <unotools/configmgr.hxx>
#include <SwStyleNameMapper.hxx>

#ifndef _NUM_HXX //autogen
#define USE_NUMTABPAGES
#include <num.hxx>
#endif
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
#include <globals.hrc>      // fuer Vorlagenname 'keins'
#include <misc.hrc>
#include <outline.hrc>
#include <paratr.hxx>

#include <unomid.h>

#include <IDocumentOutlineNodes.hxx>
#include <app.hrc>

using namespace ::com::sun::star;

DBG_NAME(outlinehdl)

class SwNumNamesDlg: public ModalDialog
{
    Edit         aFormEdit;
    ListBox      aFormBox;
    FixedLine    aFormFL;
    OKButton     aOKBtn;
    CancelButton aCancelBtn;
    HelpButton   aHelpBtn;

    DECL_LINK( ModifyHdl, Edit * );
    DECL_LINK( SelectHdl, ListBox * );
    DECL_LINK( DoubleClickHdl, ListBox * );

public:
    SwNumNamesDlg(Window *pParent);
    ~SwNumNamesDlg();
    void SetUserNames(const String *pList[]);
    String GetName() const { return aFormEdit.GetText(); }
    USHORT GetCurEntryPos() const { return aFormBox.GetSelectEntryPos(); }
};

/*------------------------------------------------------------------------
 Beschreibung:  selektierten Eintrag merken
------------------------------------------------------------------------*/
IMPL_LINK_INLINE_START( SwNumNamesDlg, SelectHdl, ListBox *, pBox )
{
    aFormEdit.SetText(pBox->GetSelectEntry());
    aFormEdit.SetSelection(Selection(0, SELECTION_MAX));
    return 0;
}
IMPL_LINK_INLINE_END( SwNumNamesDlg, SelectHdl, ListBox *, pBox )

/*------------------------------------------------------------------------
 Beschreibung:  Setzen der vom Benutzer vergebenen Namen
 Parameter:     Liste der vom Benutzer vergebenen Namen;
                nicht vom Benutzer benannte Positionen sind 0.
------------------------------------------------------------------------*/
void SwNumNamesDlg::SetUserNames(const String *pList[])
{
    USHORT nSelect = 0;
    for(USHORT i = 0; i < SwBaseNumRules::nMaxRules; ++i)
    {
        if(pList[i])
        {
            aFormBox.RemoveEntry(i);
            aFormBox.InsertEntry(*pList[i], i);
            if(i == nSelect && nSelect < SwBaseNumRules::nMaxRules)
                nSelect++;
        }
    }
    aFormBox.SelectEntryPos(nSelect);
    SelectHdl(&aFormBox);
}

/*------------------------------------------------------------------------
 Beschreibung:  OK-Button freischalten, wenn Text im Edit steht.
------------------------------------------------------------------------*/
IMPL_LINK_INLINE_START( SwNumNamesDlg, ModifyHdl, Edit *, pBox )
{
    aOKBtn.Enable(0 != pBox->GetText().Len());
    return 0;
}
IMPL_LINK_INLINE_END( SwNumNamesDlg, ModifyHdl, Edit *, pBox )

/*------------------------------------------------------------------------
 Beschreibung:  DoubleClickHdl
------------------------------------------------------------------------*/
IMPL_LINK_INLINE_START( SwNumNamesDlg, DoubleClickHdl, ListBox *, EMPTYARG )
{
    EndDialog(RET_OK);
    return 0;
}
IMPL_LINK_INLINE_END( SwNumNamesDlg, DoubleClickHdl, ListBox *, EMPTYARG )

SwNumNamesDlg::SwNumNamesDlg(Window *pParent)
    : ModalDialog(pParent, SW_RES(DLG_NUM_NAMES)),
    aFormEdit(this, SW_RES(ED_FORM)),
    aFormBox(this, SW_RES(LB_FORM)),
    aFormFL(this, SW_RES(FL_FORM)),
    aOKBtn(this, SW_RES(BT_OK)),
    aCancelBtn(this, SW_RES(BT_CANCEL)),
    aHelpBtn(this, SW_RES(BT_HELP))
{
    FreeResource();
    aFormEdit.SetModifyHdl(LINK(this, SwNumNamesDlg, ModifyHdl));
    aFormBox.SetSelectHdl(LINK(this, SwNumNamesDlg, SelectHdl));
    aFormBox.SetDoubleClickHdl(LINK(this, SwNumNamesDlg, DoubleClickHdl));
    SelectHdl(&aFormBox);
}

SwNumNamesDlg::~SwNumNamesDlg() {}

USHORT lcl_BitToLevel(USHORT nActLevel)
{
    USHORT nTmp = nActLevel;
    USHORT nTmpLevel = 0;
    while( 0 != (nTmp >>= 1) )
        nTmpLevel++;
    return nTmpLevel;
}

USHORT SwOutlineTabDialog::nNumLevel = 1;
SwOutlineTabDialog::SwOutlineTabDialog(Window* pParent,
                    const SfxItemSet* pSwItemSet,
                    SwWrtShell &rSh) :
                                    //der UserString wird danach richtig gesetzt
        SfxTabDialog(pParent, SW_RES(DLG_TAB_OUTLINE), pSwItemSet, FALSE, &aEmptyStr),
        aNullStr(C2S("____")),
        aFormMenu(SW_RES(MN_FORM)),
        rWrtSh(rSh),
        pChapterNumRules(SW_MOD()->GetChapterNumRules()),
        bModified(rWrtSh.IsModified())
{
    // --> OD 2008-04-14 #outlinelevel#
    SetText( SW_RES( STR_OUTLINE_NUMBERING ) );
    // <--
    PushButton* pUserButton = GetUserButton();
    pUserButton->SetText(SW_RES(ST_FORM));
    pUserButton->SetHelpId(HID_OUTLINE_FORM);
    pUserButton->SetClickHdl(LINK(this, SwOutlineTabDialog, FormHdl));

    FreeResource();
    pNumRule = new SwNumRule( *rSh.GetOutlineNumRule() );
    GetCancelButton().SetClickHdl(LINK(this, SwOutlineTabDialog, CancelHdl));

    AddTabPage(TP_NUM_POSITION   , &SwNumPositionTabPage::Create, 0);
    AddTabPage(TP_OUTLINE_NUM    , &SwOutlineSettingsTabPage::Create, 0);

    String sHeadline;
    USHORT i;

    for( i = 0; i < MAXLEVEL; ++i )
    {
        // wurde die Vorlage noch nicht angelegt, dann ist sie noch an dieserPosition
        if( !rWrtSh.GetParaStyle( sHeadline =
            SwStyleNameMapper::GetUIName( static_cast< sal_uInt16 >(RES_POOLCOLL_HEADLINE1 + i),
                                          sHeadline )) )
            aCollNames[i] = sHeadline;
    }

    // Erfragen der Gliederungsebenen der Textvorlagen
    const USHORT nCount = rWrtSh.GetTxtFmtCollCount();
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

void    SwOutlineTabDialog::PageCreated(USHORT nPageId, SfxTabPage& rPage)
{
    switch ( nPageId )
    {
        case TP_NUM_POSITION:
                ((SwNumPositionTabPage&)rPage).SetWrtShell(&rWrtSh);
                ((SwNumPositionTabPage&)rPage).SetOutlineTabDialog(this);
        break;
        case TP_OUTLINE_NUM:
                ((SwOutlineSettingsTabPage&)rPage).SetWrtShell(&rWrtSh);
        break;
    }
}

IMPL_LINK( SwOutlineTabDialog, CancelHdl, Button *, EMPTYARG )
{
    if (!bModified)
        rWrtSh.ResetModified();
    EndDialog(RET_CANCEL);
    return 0;
}

IMPL_LINK( SwOutlineTabDialog, FormHdl, Button *, pBtn )
{
    //PopupMenu auffuellen
    for( USHORT i = 0; i < SwChapterNumRules::nMaxRules; ++i )
    {
        const SwNumRulesWithName *pRules = pChapterNumRules->GetRules(i);
        if( pRules )
            aFormMenu.SetItemText(i + MN_FORMBASE, pRules->GetName());
    }
    aFormMenu.SetSelectHdl(LINK(this, SwOutlineTabDialog, MenuSelectHdl));
    aFormMenu.Execute(pBtn, Rectangle(Point(0,0), pBtn->GetSizePixel()), POPUPMENU_EXECUTE_DOWN);
    return 0;
}

IMPL_LINK( SwOutlineTabDialog, MenuSelectHdl, Menu *, pMenu )
{
    BYTE nLevelNo = 0;
    switch(pMenu->GetCurItemId())
    {
        case MN_FORM1: nLevelNo = 1;    break;
        case MN_FORM2: nLevelNo = 2;    break;
        case MN_FORM3: nLevelNo = 3;    break;
        case MN_FORM4: nLevelNo = 4;    break;
        case MN_FORM5: nLevelNo = 5;    break;
        case MN_FORM6: nLevelNo = 6;    break;
        case MN_FORM7: nLevelNo = 7;    break;
        case MN_FORM8: nLevelNo = 8;    break;
        case MN_FORM9: nLevelNo = 9;    break;

        case MN_SAVE:
        {
            SwNumNamesDlg *pDlg = new SwNumNamesDlg(this);
            const String *aStrArr[SwChapterNumRules::nMaxRules];
            for(USHORT i = 0; i < SwChapterNumRules::nMaxRules; ++i)
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
                pMenu->SetItemText(
                        pDlg->GetCurEntryPos() + MN_FORMBASE, aName);
            }
            delete pDlg;
            return 0;

        }

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

    USHORT  nPageId = GetCurPageId();
    SfxTabPage* pPage = GetTabPage( nPageId );
    pPage->Reset(*GetOutputItemSet());

    return 0;
}

USHORT  SwOutlineTabDialog::GetLevel(const String &rFmtName) const
{
    for(USHORT i = 0; i < MAXLEVEL; ++i)
    {
        if(aCollNames[i] == rFmtName)
            return i;
    }
    return MAXLEVEL;//NO_NUMBERING; //#outline level,zhaojianwei

}

short SwOutlineTabDialog::Ok()
{
    SfxTabDialog::Ok();
    // bei allen erzeugten Vorlagen die Ebenen setzen, muss
    // geschehen, um evtl. aufgehobene Zuordnungen
    // auch wieder zu loeschen

    // --> OD 2006-12-11 #130443#
    // encapsulate changes into a action to avoid effects on the current cursor
    // position during the changes.
    rWrtSh.StartAction();
    // <--

    const SwNumRule * pOutlineRule = rWrtSh.GetOutlineNumRule();

    USHORT i, nCount = rWrtSh.GetTxtFmtCollCount();
    for( i = 0; i < nCount; ++i )
    {
        SwTxtFmtColl &rTxtColl = rWrtSh.GetTxtFmtColl(i);
        if( !rTxtColl.IsDefault() )
        {

            const SfxPoolItem & rItem =
                rTxtColl.GetFmtAttr(RES_PARATR_NUMRULE, FALSE);

           if ((BYTE)GetLevel(rTxtColl.GetName()) == MAXLEVEL) //add by zhaojianwei
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
        String sHeadline;
        ::SwStyleNameMapper::FillUIName( static_cast< sal_uInt16 >(RES_POOLCOLL_HEADLINE1 + i),
                                         sHeadline );
        SwTxtFmtColl* pColl = rWrtSh.FindTxtFmtCollByName( sHeadline );
        if( !pColl )
        {
            if(aCollNames[i] != sHeadline)//->added by zhaojianwei
            {
                SwTxtFmtColl* pTxtColl = rWrtSh.GetTxtCollFromPool(
                    static_cast< USHORT >(RES_POOLCOLL_HEADLINE1 + i) );
                pTxtColl->DeleteAssignmentToListLevelOfOutlineStyle();
                pTxtColl->ResetFmtAttr(RES_PARATR_NUMRULE);

                if( aCollNames[i].Len() )
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
            }//<--end,zhaojianwei
        }
    }

    rWrtSh.SetOutlineNumRule( *pNumRule);

    // --> OD 2006-12-11 #130443#
    rWrtSh.EndAction();
    // <--

    return RET_OK;
}

SwOutlineSettingsTabPage::SwOutlineSettingsTabPage(Window* pParent, const SfxItemSet& rSet) :
    SfxTabPage(pParent, SW_RES(TP_OUTLINE_NUM), rSet),
    aLevelLB(       this, SW_RES(LB_LEVEL   )),
    aLevelFL(       this, SW_RES(FL_LEVEL    )),
    aCollLbl(this, SW_RES(FT_COLL)),
    aCollBox(this, SW_RES(LB_COLL)),
    aNumberLbl(this, SW_RES(FT_NUMBER)),
    aNumberBox(this, SW_RES(LB_NUMBER), INSERT_NUM_TYPE_NO_NUMBERING|INSERT_NUM_EXTENDED_TYPES),
    aCharFmtFT(this, SW_RES(FT_CHARFMT)),
    aCharFmtLB(this, SW_RES(LB_CHARFMT)),
    aAllLevelFT(    this, SW_RES(FT_ALL_LEVEL)),
    aAllLevelNF(    this, SW_RES(NF_ALL_LEVEL)),
    aDelim(this, SW_RES(FT_DELIM)),
    aPrefixFT(this, SW_RES(FT_PREFIX)),
    aPrefixED(this, SW_RES(ED_PREFIX)),
    aSuffixFT(this, SW_RES(FT_SUFFIX)),
    aSuffixED(this, SW_RES(ED_SUFFIX)),
    aStartLbl(this, SW_RES(FT_START)),
    aStartEdit(this, SW_RES(ED_START)),
    aNumberFL(this, SW_RES(FL_NUMBER)),
    aPreviewWIN(    this, SW_RES(WIN_PREVIEW )),

    aNoFmtName(SW_RES(ST_NO_COLL)),
    pSh(0),
    pCollNames(0),
    nActLevel(1)
{
    FreeResource();
    SetExchangeSupport();

    aCollBox.InsertEntry(aNoFmtName);
    aLevelLB.SetSelectHdl(LINK(this,    SwOutlineSettingsTabPage, LevelHdl));
    aAllLevelNF.SetModifyHdl(LINK(this, SwOutlineSettingsTabPage, ToggleComplete));
    aCollBox.SetSelectHdl(LINK(this,    SwOutlineSettingsTabPage, CollSelect));
    aCollBox.SetGetFocusHdl(LINK(this,  SwOutlineSettingsTabPage, CollSelectGetFocus));
    aNumberBox.SetSelectHdl(LINK(this,  SwOutlineSettingsTabPage, NumberSelect));
    aPrefixED.SetModifyHdl(LINK(this,   SwOutlineSettingsTabPage, DelimModify));
    aSuffixED.SetModifyHdl(LINK(this,   SwOutlineSettingsTabPage, DelimModify));
    aStartEdit.SetModifyHdl(LINK(this,  SwOutlineSettingsTabPage, StartModified));
    aCharFmtLB.SetSelectHdl(LINK(this,  SwOutlineSettingsTabPage, CharFmtHdl));

}

void    SwOutlineSettingsTabPage::Update()
{
        // falls eine Vorlage fuer diese Ebene bereits selektiert wurde,
        // diese in der ListBox auswaehlean
    aCollBox.Enable(USHRT_MAX != nActLevel);
    if(USHRT_MAX == nActLevel)
    {
        BOOL bSamePrefix = TRUE;
        BOOL bSameSuffix = TRUE;
        BOOL bSameType = TRUE;
        BOOL bSameComplete = TRUE;
        BOOL bSameStart = TRUE;
        BOOL bSameCharFmt = TRUE;

        const SwNumFmt* aNumFmtArr[MAXLEVEL];
        const SwCharFmt* pFirstFmt = 0;

        for(USHORT i = 0; i < MAXLEVEL; i++)
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
            aNumberBox.SelectNumberingType( aNumFmtArr[0]->GetNumberingType() );
        else
            aNumberBox.SetNoSelection();
        if(bSameStart)
            aStartEdit.SetValue(aNumFmtArr[0]->GetStart());
        else
            aStartEdit.SetText(aEmptyStr);
        if(bSamePrefix)
            aPrefixED.SetText(aNumFmtArr[0]->GetPrefix());
        else
            aPrefixED.SetText(aEmptyStr);
        if(bSameSuffix)
            aSuffixED.SetText(aNumFmtArr[0]->GetSuffix());
        else
            aSuffixED.SetText(aEmptyStr);

        if(bSameCharFmt)
        {
            if(pFirstFmt)
                aCharFmtLB.SelectEntry(pFirstFmt->GetName());
            else
                aCharFmtLB.SelectEntry( ViewShell::GetShellRes()->aStrNone );
        }
        else
            aCharFmtLB.SetNoSelection();

        aAllLevelFT.Enable(TRUE);
        aAllLevelNF.Enable(TRUE);
        aAllLevelNF.SetMax(MAXLEVEL);
        if(bSameComplete)
        {
            aAllLevelNF.SetValue(aNumFmtArr[0]->GetIncludeUpperLevels());
        }
        else
        {
            aAllLevelNF.SetText(aEmptyStr);
        }
    }
    else
    {
        USHORT nTmpLevel = lcl_BitToLevel(nActLevel);
        String aColl(pCollNames[nTmpLevel]);
        if(aColl.Len())
            aCollBox.SelectEntry(aColl);
        else
            aCollBox.SelectEntry(aNoFmtName);
        const SwNumFmt &rFmt = pNumRule->Get(nTmpLevel);

        aNumberBox.SelectNumberingType( rFmt.GetNumberingType() );
        aPrefixED.SetText(rFmt.GetPrefix());
        aSuffixED.SetText(rFmt.GetSuffix());
        const SwCharFmt* pFmt = rFmt.GetCharFmt();
        if(pFmt)
            aCharFmtLB.SelectEntry(pFmt->GetName());
        else
            aCharFmtLB.SelectEntry( ViewShell::GetShellRes()->aStrNone );

        if(nTmpLevel)
        {
            aAllLevelFT.Enable(TRUE);
            aAllLevelNF.Enable(TRUE);
            aAllLevelNF.SetMax(nTmpLevel + 1);
            aAllLevelNF.SetValue(rFmt.GetIncludeUpperLevels());
        }
        else
        {
            aAllLevelNF.SetText(aEmptyStr);
            aAllLevelNF.Enable(FALSE);
            aAllLevelFT.Enable(FALSE);
        }
        CheckForStartValue_Impl(rFmt.GetNumberingType());
        aStartEdit.SetValue( rFmt.GetStart() );
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
        USHORT nMask = 1;
        for( USHORT i = 0; i < MAXLEVEL; i++ )
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
    USHORT nMask = 1;
    for(USHORT i = 0; i < MAXLEVEL; i++)
    {
        if(nActLevel & nMask)
        {
            SwNumFmt aNumFmt(pNumRule->Get(i));
            aNumFmt.SetIncludeUpperLevels( Min( (BYTE)pFld->GetValue(),
                                                (BYTE)(i + 1)) );
            pNumRule->Set(i, aNumFmt);
        }
        nMask <<= 1;
    }
    SetModified();
    return 0;
}

IMPL_LINK( SwOutlineSettingsTabPage, CollSelect, ListBox *, pBox )
{
    BYTE i;

    const String aCollName(pBox->GetSelectEntry());
    //0xFFFF darf hier nicht sein (disable)
    USHORT nTmpLevel = lcl_BitToLevel(nActLevel);
    String sOldName( pCollNames[nTmpLevel] );

    for( i = 0; i < MAXLEVEL; ++i)
        pCollNames[i] = aSaveCollNames[i];

    if(aCollName == aNoFmtName)
        pCollNames[nTmpLevel] = aEmptyStr;
    else
    {
        pCollNames[nTmpLevel] = aCollName;
                // wird die Vorlage bereits verwendet ?
        for( i = 0; i < MAXLEVEL; ++i)
            if(i != nTmpLevel && pCollNames[i] == aCollName )
                pCollNames[i] = aEmptyStr;
    }

    // search the oldname and put it into the current entries
    if( sOldName.Len() )
        for( i = 0; i < MAXLEVEL; ++i)
            if( aSaveCollNames[ i ] == sOldName && i != nTmpLevel &&
                !pCollNames[ i ].Len() )
            {
                BYTE n;
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

IMPL_LINK( SwOutlineSettingsTabPage, CollSelectGetFocus, ListBox *, EMPTYARG )
{
    for( BYTE i = 0; i < MAXLEVEL; ++i)
        aSaveCollNames[i] =  pCollNames[i];
    return 0;
}

IMPL_LINK( SwOutlineSettingsTabPage, NumberSelect, SwNumberingTypeListBox *, pBox )
{
    USHORT nMask = 1;
    sal_Int16 nNumberType = pBox->GetSelectedNumberingType();
    for(USHORT i = 0; i < MAXLEVEL; i++)
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

IMPL_LINK( SwOutlineSettingsTabPage, DelimModify, Edit *, EMPTYARG )
{
    USHORT nMask = 1;
    for(USHORT i = 0; i < MAXLEVEL; i++)
    {
        if(nActLevel & nMask)
        {
            SwNumFmt aNumFmt(pNumRule->Get(i));
            aNumFmt.SetPrefix( aPrefixED.GetText() );
            aNumFmt.SetSuffix( aSuffixED.GetText() );
            pNumRule->Set(i, aNumFmt);
        }
        nMask <<= 1;
    }
    SetModified();
    return 0;
}

IMPL_LINK( SwOutlineSettingsTabPage, StartModified, NumericField *, pFld )
{
    USHORT nMask = 1;
    for(USHORT i = 0; i < MAXLEVEL; i++)
    {
        if(nActLevel & nMask)
        {
            SwNumFmt aNumFmt(pNumRule->Get(i));
            aNumFmt.SetStart( (USHORT)pFld->GetValue() );
            pNumRule->Set(i, aNumFmt);
        }
        nMask <<= 1;
    }
    SetModified();
    return 0;
}

IMPL_LINK( SwOutlineSettingsTabPage, CharFmtHdl, ListBox *, EMPTYARG )
{
    String sEntry = aCharFmtLB.GetSelectEntry();
    USHORT nMask = 1;
    BOOL bFormatNone = sEntry == ViewShell::GetShellRes()->aStrNone;
    SwCharFmt* pFmt = 0;
    if(!bFormatNone)
    {
        USHORT nChCount = pSh->GetCharFmtCount();
        for(USHORT i = 0; i < nChCount; i++)
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

    for(USHORT i = 0; i < MAXLEVEL; i++)
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
    // Erfragen der NumRules dieses Dokumentes
    pNumRule = ((SwOutlineTabDialog*)GetTabDialog())->GetNumRule();
    pCollNames = ((SwOutlineTabDialog*)GetTabDialog())->GetCollNames();

    aPreviewWIN.SetNumRule(pNumRule);
    aPreviewWIN.SetOutlineNames(pCollNames);
    // Startwert setzen - nActLevel muss hier 1 sein
    USHORT nTmpLevel = lcl_BitToLevel(nActLevel);
    const SwNumFmt& rNumFmt = pNumRule->Get( nTmpLevel );
    aStartEdit.SetValue( rNumFmt.GetStart() );

    // Poolformate fuer Ueberschriften anlegen
    String sStr;
    USHORT i;
    for( i = 0; i < MAXLEVEL; ++i )
    {
        aCollBox.InsertEntry( SwStyleNameMapper::GetUIName(
                                    static_cast< sal_uInt16 >(RES_POOLCOLL_HEADLINE1 + i), aEmptyStr ));
        aLevelLB.InsertEntry( String::CreateFromInt32(i + 1) );
    }
    sStr.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "1 - " ));
    sStr += String::CreateFromInt32(MAXLEVEL);
    aLevelLB.InsertEntry( sStr );

    // Erfragen der Gliederungsebenen der Textvorlagen
    const USHORT nCount = pSh->GetTxtFmtCollCount();
    for( i = 0; i < nCount; ++i )
    {
        SwTxtFmtColl &rTxtColl = pSh->GetTxtFmtColl(i);
        if(!rTxtColl.IsDefault())
        {
            sStr = rTxtColl.GetName();
            if(LISTBOX_ENTRY_NOTFOUND == aCollBox.GetEntryPos( sStr ))
                aCollBox.InsertEntry( sStr );
        }
    }

    aNumberBox.SelectNumberingType(rNumFmt.GetNumberingType());
    USHORT nOutlinePos = pSh->GetOutlinePos(MAXLEVEL);
    USHORT nTmp = 0;
    if(nOutlinePos != USHRT_MAX)
    {
        nTmp = static_cast<USHORT>(pSh->getIDocumentOutlineNodesAccess()->getOutlineLevel(nOutlinePos));
    }
    aLevelLB.SelectEntryPos(nTmp-1);//nTmp);//#outline level,zhaojianwei

    // Zeichenvorlagen sammeln
    aCharFmtLB.Clear();
    aCharFmtLB.InsertEntry( ViewShell::GetShellRes()->aStrNone );

    // Zeichenvorlagen
    ::FillCharStyleListBox(aCharFmtLB,
                        pSh->GetView().GetDocShell());
    Update();
}

void    SwOutlineSettingsTabPage::ActivatePage(const SfxItemSet& )
{
    nActLevel = SwOutlineTabDialog::GetActNumLevel();
    if(nActLevel != USHRT_MAX)
        aLevelLB.SelectEntryPos(lcl_BitToLevel(nActLevel));
    else
        aLevelLB.SelectEntryPos(MAXLEVEL);
    LevelHdl(&aLevelLB);
}

int     SwOutlineSettingsTabPage::DeactivatePage(SfxItemSet*)
{
    SwOutlineTabDialog::SetActNumLevel(nActLevel);
    return LEAVE_PAGE;
}

BOOL    SwOutlineSettingsTabPage::FillItemSet( SfxItemSet&  )
{
    return TRUE;
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
    BOOL bIsNull = aStartEdit.GetValue() == 0;
    BOOL bNoZeroAllowed = nNumberingType < SVX_NUM_ARABIC ||
                        SVX_NUM_CHARS_UPPER_LETTER_N == nNumberingType ||
                        SVX_NUM_CHARS_LOWER_LETTER_N == nNumberingType;
    aStartEdit.SetMin(bNoZeroAllowed ? 1 : 0);
    if(bIsNull && bNoZeroAllowed)
        aStartEdit.GetModifyHdl().Call(&aStartEdit);
}

USHORT lcl_DrawBullet(VirtualDevice* pVDev,
            const SwNumFmt& rFmt, USHORT nXStart,
            USHORT nYStart, const Size& rSize)
{
    Font aTmpFont(pVDev->GetFont());

    Font aFont(*rFmt.GetBulletFont());
    aFont.SetSize(rSize);
    aFont.SetTransparent(TRUE);
    pVDev->SetFont( aFont );
    String aText(rFmt.GetBulletChar());
    pVDev->DrawText( Point(nXStart, nYStart), aText );
    USHORT nRet = (USHORT)pVDev->GetTextWidth(aText);

    pVDev->SetFont(aTmpFont);
    return nRet;
}

USHORT lcl_DrawGraphic(VirtualDevice* pVDev, const SwNumFmt &rFmt, USHORT nXStart,
                        USHORT nYStart, USHORT nDivision)
{
    const SvxBrushItem* pBrushItem = rFmt.GetBrush();
    USHORT nRet = 0;
    if(pBrushItem)
    {
        const Graphic* pGrf = pBrushItem->GetGraphic();
        if(pGrf)
        {
            Size aGSize( rFmt.GetGraphicSize());
            aGSize.Width() /= nDivision;
            nRet = (USHORT)aGSize.Width();
            aGSize.Height() /= nDivision;
            pGrf->Draw( pVDev, Point(nXStart,nYStart),
                    pVDev->PixelToLogic( aGSize ) );
        }
    }
    return nRet;

}

/*--------------------------------------------------
    Vorschau der Numerierung painten
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
        USHORT nWidthRelation;
        if(nPageWidth)
        {
            nWidthRelation = USHORT (nPageWidth / aSize.Width());
            if(bPosition)
                nWidthRelation = nWidthRelation * 2 / 3;
            else
                nWidthRelation = nWidthRelation / 4;
        }
        else
            nWidthRelation = 30; // Kapiteldialog

        //Hoehe pro Ebene
        USHORT nXStep = USHORT(aSize.Width() / (3 * MAXLEVEL));
        if(MAXLEVEL < 10)
            nXStep /= 2;
        USHORT nYStart = 4;
        USHORT nYStep = USHORT((aSize.Height() - 6)/ MAXLEVEL);
        aStdFont = OutputDevice::GetDefaultFont(
                                    DEFAULTFONT_UI_SANS, (LanguageType)GetAppLanguage(),
                                    DEFAULTFONT_FLAGS_ONLYONE, this );
        // #101524# OJ
        aStdFont.SetColor( SwViewOption::GetFontColor() );

        USHORT nFontHeight = nYStep * 6 / 10;
        if(bPosition)
            nFontHeight = nYStep * 15 / 10;
        aStdFont.SetSize(Size( 0, nFontHeight ));

        USHORT nPreNum = pActNum->Get(0).GetStart();

        if(bPosition)
        {
            USHORT nLineHeight = nFontHeight * 8 / 7;
            BYTE nStart = 0;
            while( !(nActLevel & (1<<nStart)) )
            {
                nStart++;
            }
            if(nStart) // damit moeglichs Vorgaenger und Nachfolger gezeigt werden
                nStart--;

            SwNumberTree::tNumberVector aNumVector;
            BYTE nEnd = Min( (BYTE)(nStart + 3), MAXLEVEL );
            for( BYTE nLevel = nStart; nLevel < nEnd; ++nLevel )
            {
                const SwNumFmt &rFmt = pActNum->Get(nLevel);
                aNumVector.push_back(rFmt.GetStart());

                USHORT nXStart( 0 );
                short nTextOffset( 0 );
                USHORT nNumberXPos( 0 );
                if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
                {
                    nXStart = rFmt.GetAbsLSpace() / nWidthRelation;
                    nTextOffset = rFmt.GetCharTextDistance() / nWidthRelation;
                    nNumberXPos = nXStart;
                    USHORT nFirstLineOffset = (-rFmt.GetFirstLineOffset()) / nWidthRelation;

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
                        nNumberXPos = static_cast<USHORT>(nTmpNumberXPos);
                    }
                }

                USHORT nBulletWidth = 0;
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
                    // --> OD 2005-11-17 #128041#
                    String aText(pActNum->MakeNumString( aNumVector ));
                    // <--
                    pVDev->DrawText( Point(nNumberXPos, nYStart), aText );
                    nBulletWidth = (USHORT)pVDev->GetTextWidth(aText);
                    nPreNum++;
                }
                if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT &&
                     rFmt.GetLabelFollowedBy() == SvxNumberFormat::SPACE )
                {
                    pVDev->SetFont(aStdFont);
                    String aText(' ');
                    pVDev->DrawText( Point(nNumberXPos, nYStart), aText );
                    nBulletWidth = nBulletWidth + (USHORT)pVDev->GetTextWidth(aText);
                }

                USHORT nTextXPos( 0 );
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
                            nTextXPos = static_cast<USHORT>(
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

                    nXStart = static_cast<USHORT>( rFmt.GetIndentAt() / nWidthRelation );
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
            USHORT nLineHeight = nFontHeight * 3 / 2;
            for( BYTE nLevel = 0; nLevel < MAXLEVEL;
                            ++nLevel, nYStart = nYStart + nYStep )
            {
                const SwNumFmt &rFmt = pActNum->Get(nLevel);
                aNumVector.push_back(rFmt.GetStart());
                USHORT nXStart( 0 );
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
                        nXStart = static_cast<USHORT>(nTmpXStart);
                    }
                }
                nXStart /= 2;
                nXStart += 2;
                USHORT nTextOffset = 2 * nXStep;
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
                    // --> OD 2005-11-17 #128041#
                    String aText(pActNum->MakeNumString( aNumVector ));
                    // <--
                    pVDev->DrawText( Point(nXStart, nYStart), aText );
                    nTextOffset = (USHORT)pVDev->GetTextWidth(aText);
                    nTextOffset = nTextOffset + nXStep;
                    nPreNum++;
                }
                pVDev->SetFont(aStdFont);

                // Changed as per BugID 79541 Branding/Configuration
                uno::Any MyAny = ::utl::ConfigManager::GetConfigManager().
                        GetDirectConfigProperty( ::utl::ConfigManager::PRODUCTNAME );
                ::rtl::OUString aProductName;

                MyAny >>= aProductName;
                String sMsg(C2S("%PRODUCTNAME"));
                sMsg.SearchAndReplaceAscii( "%PRODUCTNAME" , aProductName );

                if(pOutlineNames)
                    sMsg = pOutlineNames[nLevel];
                pVDev->DrawText( Point(nXStart + nTextOffset, nYStart), sMsg );
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
