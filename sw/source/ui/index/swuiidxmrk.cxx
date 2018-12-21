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

#include <swuiidxmrk.hxx>
#include <hintids.hxx>
#include <helpids.h>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/Bibliography.hpp>
#include <com/sun/star/i18n/IndexEntrySupplier.hpp>
#include <com/sun/star/util/SearchAlgorithms2.hpp>
#include <com/sun/star/util/SearchFlags.hpp>
#include <i18nutil/searchopt.hxx>
#include <svl/stritem.hxx>
#include <vcl/weld.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/eitem.hxx>
#include <unotools/textsearch.hxx>
#include <editeng/scripttypeitem.hxx>
#include <svl/itemset.hxx>
#include <editeng/langitem.hxx>
#include <editeng/unolingu.hxx>
#include <swtypes.hxx>
#include <idxmrk.hxx>
#include <txttxmrk.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <multmrk.hxx>
#include <swundo.hxx>
#include <cmdid.h>
#include <app.hrc>
#include <swmodule.hxx>
#include <fldmgr.hxx>
#include <fldbas.hxx>
#include <strings.hrc>
#include <swcont.hxx>
#include <svl/cjkoptions.hxx>
#include <ndtxt.hxx>
#include <breakit.hxx>
#include <SwRewriter.hxx>

#include <unomid.h>

#define POS_CONTENT 0
#define POS_INDEX   1

static sal_Int32  nTypePos = 1; // TOX_INDEX as standard
static sal_uInt16 nKey1Pos = USHRT_MAX;

static sal_uInt16 nKey2Pos = USHRT_MAX;

using namespace com::sun::star;
using namespace com::sun::star::i18n;
using namespace com::sun::star::lang;
using namespace com::sun::star::util;
using namespace ::comphelper;

// dialog to insert a directory selection
SwIndexMarkPane::SwIndexMarkPane(Dialog &rDialog, bool bNewDlg,
    SwWrtShell& rWrtShell)
    : m_rDialog(rDialog)
    , bDel(false)
    , bNewMark(bNewDlg)
    , bSelected(false)
    , bPhoneticED0_ChangedByUser(false)
    , bPhoneticED1_ChangedByUser(false)
    , bPhoneticED2_ChangedByUser(false)
    , nLangForPhoneticReading(LANGUAGE_CHINESE_SIMPLIFIED)
    , bIsPhoneticReadingEnabled(false)
    , xExtendedIndexEntrySupplier(nullptr)
    , pTOXMgr(nullptr)
    , pSh(&rWrtShell)
{
    rDialog.get(m_pFrame, "frame");
    rDialog.get(m_pTypeFT, "typeft");
    rDialog.get(m_pTypeDCB, "typecb");
    rDialog.get(m_pNewBT, "new");
    rDialog.get(m_pEntryED, "entryed");
    rDialog.get(m_pSyncED, "sync");
    m_pSyncED->Show();
    rDialog.get(m_pPhoneticFT0, "phonetic0ft");
    rDialog.get(m_pPhoneticED0, "phonetic0ed");
    rDialog.get(m_pKey1FT, "key1ft");
    rDialog.get(m_pKey1DCB, "key1cb");
    rDialog.get(m_pPhoneticFT1, "phonetic1ft");
    rDialog.get(m_pPhoneticED1, "phonetic1ed");
    rDialog.get(m_pKey2FT, "key2ft");
    rDialog.get(m_pKey2DCB, "key2cb");
    rDialog.get(m_pPhoneticFT2, "phonetic2ft");
    rDialog.get(m_pPhoneticED2, "phonetic2ed");
    rDialog.get(m_pLevelFT, "levelft");
    rDialog.get(m_pLevelNF, "levelnf");
    rDialog.get(m_pMainEntryCB, "mainentrycb");
    rDialog.get(m_pApplyToAllCB, "applytoallcb");
    rDialog.get(m_pSearchCaseSensitiveCB, "searchcasesensitivecb");
    rDialog.get(m_pSearchCaseWordOnlyCB, "searchcasewordonlycb");
    rDialog.get(m_pCloseBT, "close");
    rDialog.get(m_pDelBT, "delete");
    rDialog.get(m_pPrevSameBT, "first");
    rDialog.get(m_pNextSameBT, "last");
    rDialog.get(m_pPrevBT, "previous");
    rDialog.get(m_pNextBT, "next");

    if (SvtCJKOptions().IsCJKFontEnabled())
    {
        uno::Reference< uno::XComponentContext > xContext = getProcessComponentContext();

        xExtendedIndexEntrySupplier = i18n::IndexEntrySupplier::create(xContext);

        m_pPhoneticFT0->Show();
        m_pPhoneticED0->Show();
        m_pPhoneticFT1->Show();
        m_pPhoneticED1->Show();
        m_pPhoneticFT2->Show();
        m_pPhoneticED2->Show();
    }

    rDialog.SetText( SwResId( bNewMark ? STR_IDXMRK_INSERT : STR_IDXMRK_EDIT));

    m_pDelBT->SetClickHdl(LINK(this,SwIndexMarkPane,        DelHdl));
    m_pPrevBT->SetClickHdl(LINK(this,SwIndexMarkPane,       PrevHdl));
    m_pPrevSameBT->SetClickHdl(LINK(this,SwIndexMarkPane,   PrevSameHdl));
    m_pNextBT->SetClickHdl(LINK(this,SwIndexMarkPane,       NextHdl));
    m_pNextSameBT->SetClickHdl(LINK(this,SwIndexMarkPane,   NextSameHdl));
    m_pTypeDCB->SetSelectHdl(LINK(this,SwIndexMarkPane,     ModifyListBoxHdl));
    m_pKey1DCB->SetModifyHdl(LINK(this,SwIndexMarkPane,      KeyDCBModifyHdl));
    m_pKey2DCB->SetModifyHdl(LINK(this,SwIndexMarkPane,     KeyDCBModifyHdl));
    m_pCloseBT->SetClickHdl(LINK(this,SwIndexMarkPane,      CloseHdl));
    m_pEntryED->SetModifyHdl(LINK(this,SwIndexMarkPane,     ModifyEditHdl));
    m_pNewBT->SetClickHdl(LINK(this, SwIndexMarkPane,       NewUserIdxHdl));
    m_pApplyToAllCB->SetClickHdl(LINK(this, SwIndexMarkPane, SearchTypeHdl));
    m_pPhoneticED0->SetModifyHdl(LINK(this,SwIndexMarkPane, PhoneticEDModifyHdl));
    m_pPhoneticED1->SetModifyHdl(LINK(this,SwIndexMarkPane, PhoneticEDModifyHdl));
    m_pPhoneticED2->SetModifyHdl(LINK(this,SwIndexMarkPane, PhoneticEDModifyHdl));
    m_pSyncED->SetClickHdl(LINK(this, SwIndexMarkPane, SyncSelectionHdl));

    if(bNewMark)
    {
        m_pDelBT->Hide();
        rDialog.get(m_pOKBT, "insert");
    }
    else
    {
        m_pNewBT->Hide();
        rDialog.get(m_pOKBT, "ok");
    }
    m_pOKBT->Show();
    m_pOKBT->SetClickHdl(LINK(this, SwIndexMarkPane, InsertHdl));

    m_pEntryED->GrabFocus();
}

// Newly initialise controls with the new selection
void SwIndexMarkPane::InitControls()
{
    OSL_ENSURE(pSh && pTOXMgr, "no shell?");
    // contents index
    const SwTOXType* pType = pTOXMgr->GetTOXType(TOX_CONTENT);
    OSL_ENSURE(pType, "No directory type !!");
    OUString sTmpTypeSelection;
    if(m_pTypeDCB->GetSelectedEntryCount())
        sTmpTypeSelection = m_pTypeDCB->GetSelectedEntry();
    m_pTypeDCB->Clear();
    m_pTypeDCB->InsertEntry(pType->GetTypeName());

    // keyword index
    pType = pTOXMgr->GetTOXType(TOX_INDEX);
    OSL_ENSURE(pType, "No directory type !!");
    m_pTypeDCB->InsertEntry(pType->GetTypeName());

    // user index
    sal_uInt16 nCount = pSh->GetTOXTypeCount(TOX_USER);
    for( sal_uInt16 i = 0; i < nCount; ++i )
        m_pTypeDCB->InsertEntry( pSh->GetTOXType(TOX_USER, i)->GetTypeName() );

    // read keywords primary
    {
        std::vector<OUString> aArr;
        pSh->GetTOIKeys(TOI_PRIMARY, aArr);
        std::sort(aArr.begin(), aArr.end());
        auto last = std::unique(aArr.begin(), aArr.end());
        for (auto it = aArr.begin(); it != last; ++it)
            m_pKey1DCB->InsertEntry(*it);
    }

    // read keywords secondary
    {
        std::vector<OUString> aArr;
        pSh->GetTOIKeys( TOI_SECONDARY, aArr );
        std::sort(aArr.begin(), aArr.end());
        auto last = std::unique(aArr.begin(), aArr.end());
        for (auto it = aArr.begin(); it != last; ++it)
            m_pKey2DCB->InsertEntry(*it);
    }

    UpdateLanguageDependenciesForPhoneticReading();

    // current entry
    const SwTOXMark* pMark = pTOXMgr->GetCurTOXMark();
    if( pMark && !bNewMark)
    {
        // Controls-Handling

        // only if there are more than one
        // if equal it lands at the same entry
        pSh->SttCursorMove();

        const SwTOXMark* pMoveMark;
        bool bShow = false;

        pMoveMark = &pSh->GotoTOXMark( *pMark, TOX_PRV );
        if( pMoveMark != pMark )
        {
            pSh->GotoTOXMark( *pMoveMark, TOX_NXT );
            bShow = true;
        }
        m_pPrevBT->Enable( pMoveMark != pMark );
        pMoveMark = &pSh->GotoTOXMark( *pMark, TOX_NXT );
        if( pMoveMark != pMark )
        {
            pSh->GotoTOXMark( *pMoveMark, TOX_PRV );
            bShow = true;
        }
        m_pNextBT->Enable( pMoveMark != pMark );
        if( bShow )
        {
            m_pPrevBT->Show();
            m_pNextBT->Show();
            bShow = false;
        }

        pMoveMark = &pSh->GotoTOXMark( *pMark, TOX_SAME_PRV );
        if( pMoveMark != pMark )
        {
            pSh->GotoTOXMark( *pMoveMark, TOX_SAME_NXT );
            bShow = true;
        }
        m_pPrevSameBT->Enable( pMoveMark != pMark );
        pMoveMark = &pSh->GotoTOXMark( *pMark, TOX_SAME_NXT );
        if( pMoveMark != pMark )
        {
            pSh->GotoTOXMark( *pMoveMark, TOX_SAME_PRV );
            bShow = true;
        }
        m_pNextSameBT->Enable( pMoveMark != pMark );
        if( bShow )
        {
            m_pNextSameBT->Show();
            m_pPrevSameBT->Show();
        }
        pSh->EndCursorMove();

        m_pTypeFT->Show();

        m_pTypeDCB->Enable(false);
        m_pTypeFT->Enable(false);

        UpdateDialog();
    }
    else
    {   // display current selection (first element) ????
        if (pSh->GetCursorCnt() < 2)
        {
            bSelected = !pSh->HasSelection();
            aOrgStr = pSh->GetView().GetSelectionTextParam(true, false);
            m_pEntryED->SetText(aOrgStr);

            //to include all equal entries may only be allowed in the body and even there
            //only when a simple selection exists
            const FrameTypeFlags nFrameType = pSh->GetFrameType(nullptr,true);
            m_pApplyToAllCB->Show();
            m_pSearchCaseSensitiveCB->Show();
            m_pSearchCaseWordOnlyCB->Show();
            m_pApplyToAllCB->Enable(!aOrgStr.isEmpty() &&
                !(nFrameType & ( FrameTypeFlags::HEADER | FrameTypeFlags::FOOTER | FrameTypeFlags::FLY_ANY )));
            SearchTypeHdl(m_pApplyToAllCB);
        }

        // index type is default
        if( !sTmpTypeSelection.isEmpty() &&
            LISTBOX_ENTRY_NOTFOUND != m_pTypeDCB->GetEntryPos( sTmpTypeSelection ) )
            m_pTypeDCB->SelectEntry(sTmpTypeSelection);
        else
            m_pTypeDCB->SelectEntry(m_pTypeDCB->GetEntry(nTypePos));
        ModifyHdl(m_pTypeDCB);
    }
}

void    SwIndexMarkPane::UpdateLanguageDependenciesForPhoneticReading()
{
    //no phonetic reading if no global cjk support
    if( !xExtendedIndexEntrySupplier.is() )
    {
        bIsPhoneticReadingEnabled = false;
        return;
    }
    bIsPhoneticReadingEnabled = true;

    //get the current language
    if(!bNewMark) //if dialog is opened to iterate existing marks
    {
        OSL_ENSURE(pTOXMgr, "need TOXMgr");
        if(!pTOXMgr)
            return;
        SwTOXMark* pMark = pTOXMgr->GetCurTOXMark();
        OSL_ENSURE(pMark, "need current SwTOXMark");
        if(!pMark)
            return;
        SwTextTOXMark* pTextTOXMark = pMark->GetTextTOXMark();
        OSL_ENSURE(pTextTOXMark, "need current SwTextTOXMark");
        if(!pTextTOXMark)
            return;
        const SwTextNode* pTextNode = pTextTOXMark->GetpTextNd();
        OSL_ENSURE(pTextNode, "need current SwTextNode");
        if(!pTextNode)
            return;
        sal_Int32 nTextIndex = pTextTOXMark->GetStart();
        nLangForPhoneticReading = pTextNode->GetLang( nTextIndex );
    }
    else //if dialog is opened to create a new mark
    {
        sal_uInt16 nWhich;
        switch(pSh->GetScriptType())
        {
            case SvtScriptType::ASIAN: nWhich = RES_CHRATR_CJK_LANGUAGE; break;
            case SvtScriptType::COMPLEX:nWhich = RES_CHRATR_CTL_LANGUAGE; break;
            default:nWhich = RES_CHRATR_LANGUAGE; break;
        }
        SfxItemSet aLangSet(pSh->GetAttrPool(), {{nWhich, nWhich}});
        pSh->GetCurAttr(aLangSet);
        nLangForPhoneticReading = static_cast<const SvxLanguageItem&>(aLangSet.Get(nWhich)).GetLanguage();
    }

}

OUString SwIndexMarkPane::GetDefaultPhoneticReading( const OUString& rText )
{
    if( !bIsPhoneticReadingEnabled )
        return OUString();

    return xExtendedIndexEntrySupplier->getPhoneticCandidate(rText, LanguageTag::convertToLocale( nLangForPhoneticReading ));
}

void    SwIndexMarkPane::Activate()
{
    // display current selection (first element) ????
    if (bNewMark)
    {
        m_pSyncED->Enable(pSh->GetCursorCnt() < 2);
    }
}

IMPL_LINK_NOARG(SwIndexMarkPane, SyncSelectionHdl, Button*, void)
{
    bSelected = !pSh->HasSelection();
    aOrgStr = pSh->GetView().GetSelectionTextParam(true, false);
    m_pEntryED->SetText(aOrgStr);

    //to include all equal entries may only be allowed in the body and even there
    //only when a simple selection exists
    const FrameTypeFlags nFrameType = pSh->GetFrameType(nullptr,true);
    m_pApplyToAllCB->Show();
    m_pSearchCaseSensitiveCB->Show();
    m_pSearchCaseWordOnlyCB->Show();
    m_pApplyToAllCB->Enable(!aOrgStr.isEmpty() &&
        !(nFrameType & ( FrameTypeFlags::HEADER | FrameTypeFlags::FOOTER | FrameTypeFlags::FLY_ANY )));
    SearchTypeHdl(m_pApplyToAllCB);
    ModifyHdl(m_pEntryED);
}

// evaluate Ok-Button
void SwIndexMarkPane::Apply()
{
    InsertUpdate();
    if(bSelected)
        pSh->ResetSelect(nullptr, false);
}

// apply changes
void SwIndexMarkPane::InsertUpdate()
{
    pSh->StartUndo(bDel ? SwUndoId::INDEX_ENTRY_DELETE : SwUndoId::INDEX_ENTRY_INSERT);
    pSh->StartAllAction();
    SwRewriter aRewriter;

    if( bNewMark )
    {
        InsertMark();

        if ( pTOXMgr->GetCurTOXMark())
            aRewriter.AddRule(UndoArg1,
                    pTOXMgr->GetCurTOXMark()->GetText(pSh->GetLayout()));
    }
    else if( !pSh->HasReadonlySel() )
    {
        if ( pTOXMgr->GetCurTOXMark())
            aRewriter.AddRule(UndoArg1,
                    pTOXMgr->GetCurTOXMark()->GetText(pSh->GetLayout()));

        if( bDel )
            pTOXMgr->DeleteTOXMark();
        else if( pTOXMgr->GetCurTOXMark() )
            UpdateMark();
    }

    pSh->EndAllAction();
    pSh->EndUndo(bDel ? SwUndoId::INDEX_ENTRY_DELETE : SwUndoId::INDEX_ENTRY_INSERT);

    if((nTypePos = m_pTypeDCB->GetEntryPos(m_pTypeDCB->GetSelectedEntry())) == LISTBOX_ENTRY_NOTFOUND)
        nTypePos = 0;

    nKey1Pos = m_pKey1DCB->GetEntryPos(m_pKey1DCB->GetText());
    nKey2Pos = m_pKey2DCB->GetEntryPos(m_pKey2DCB->GetText());
}

// insert mark
static void lcl_SelectSameStrings(SwWrtShell& rSh, bool bWordOnly, bool bCaseSensitive)
{
    rSh.Push();

    i18nutil::SearchOptions2 aSearchOpt(
                        SearchAlgorithms_ABSOLUTE,
                        ( bWordOnly ? SearchFlags::NORM_WORD_ONLY : 0 ),
                        rSh.GetSelText(), OUString(),
                        GetAppLanguageTag().getLocale(),
                        0, 0, 0,
                        (bCaseSensitive
                            ? TransliterationFlags::NONE
                            : TransliterationFlags::IGNORE_CASE),
                        SearchAlgorithms2::ABSOLUTE,
                        '\\' );

    rSh.ClearMark();
    bool bCancel;

    //todo/mba: assuming that notes should not be searched
    rSh.Find_Text(aSearchOpt, false/*bSearchInNotes*/, SwDocPositions::Start, SwDocPositions::End, bCancel,
              FindRanges::InSelAll | FindRanges::InBodyOnly );
}

void SwIndexMarkPane::InsertMark()
{
    sal_Int32 nPos = m_pTypeDCB->GetEntryPos(m_pTypeDCB->GetSelectedEntry());
    TOXTypes eType = nPos == POS_CONTENT ? TOX_CONTENT :
                        nPos == POS_INDEX ? TOX_INDEX : TOX_USER;

    SwTOXMarkDescription aDesc(eType);

    const int nLevel = m_pLevelNF->Denormalize(m_pLevelNF->GetValue());
    switch(nPos)
    {
        case POS_CONTENT : break;
        case POS_INDEX:     // keyword index mark
        {
            UpdateKeyBoxes();
            aDesc.SetPrimKey(m_pKey1DCB->GetText());
            aDesc.SetSecKey(m_pKey2DCB->GetText());
            aDesc.SetMainEntry(m_pMainEntryCB->IsChecked());
            aDesc.SetPhoneticReadingOfAltStr(m_pPhoneticED0->GetText());
            aDesc.SetPhoneticReadingOfPrimKey(m_pPhoneticED1->GetText());
            aDesc.SetPhoneticReadingOfSecKey(m_pPhoneticED2->GetText());
        }
        break;
        default:            // Userdefined index mark
        {
            aDesc.SetTOUName(m_pTypeDCB->GetSelectedEntry());
        }
    }
    if (aOrgStr != m_pEntryED->GetText())
        aDesc.SetAltStr(m_pEntryED->GetText());
    bool bApplyAll = m_pApplyToAllCB->IsChecked();
    bool bWordOnly = m_pSearchCaseWordOnlyCB->IsChecked();
    bool bCaseSensitive = m_pSearchCaseSensitiveCB->IsChecked();

    pSh->StartAllAction();
    // all equal strings have to be selected here so that the
    // entry is applied to all equal strings
    if(bApplyAll)
    {
        lcl_SelectSameStrings(*pSh, bWordOnly, bCaseSensitive);
    }
    aDesc.SetLevel(nLevel);
    SwTOXMgr aMgr(pSh);
    aMgr.InsertTOXMark(aDesc);
    if(bApplyAll)
        pSh->Pop(SwCursorShell::PopMode::DeleteCurrent);

    pSh->EndAllAction();
}

// update mark
void SwIndexMarkPane::UpdateMark()
{
    OUString  aAltText(m_pEntryED->GetText());
    OUString* pAltText = aOrgStr != m_pEntryED->GetText() ? &aAltText : nullptr;
    //empty alternative texts are not allowed
    if(pAltText && pAltText->isEmpty())
        return;

    UpdateKeyBoxes();

    sal_Int32 nPos = m_pTypeDCB->GetEntryPos(m_pTypeDCB->GetSelectedEntry());
    TOXTypes eType = TOX_USER;
    if(POS_CONTENT == nPos)
        eType = TOX_CONTENT;
    else if(POS_INDEX == nPos)
        eType = TOX_INDEX;

    SwTOXMarkDescription aDesc(eType);
    aDesc.SetLevel( static_cast< int >(m_pLevelNF->GetValue()) );
    if(pAltText)
        aDesc.SetAltStr(*pAltText);

    OUString  aPrim(m_pKey1DCB->GetText());
    if(!aPrim.isEmpty())
        aDesc.SetPrimKey(aPrim);
    OUString  aSec(m_pKey2DCB->GetText());
    if(!aSec.isEmpty())
        aDesc.SetSecKey(aSec);

    if(eType == TOX_INDEX)
    {
        aDesc.SetPhoneticReadingOfAltStr(m_pPhoneticED0->GetText());
        aDesc.SetPhoneticReadingOfPrimKey(m_pPhoneticED1->GetText());
        aDesc.SetPhoneticReadingOfSecKey(m_pPhoneticED2->GetText());
    }
    aDesc.SetMainEntry(m_pMainEntryCB->IsVisible() && m_pMainEntryCB->IsChecked());
    pTOXMgr->UpdateTOXMark(aDesc);
}

// insert new keys
void SwIndexMarkPane::UpdateKeyBoxes()
{
    OUString aKey(m_pKey1DCB->GetText());
    sal_Int32 nPos = m_pKey1DCB->GetEntryPos(aKey);
    if(nPos == COMBOBOX_ENTRY_NOTFOUND && !aKey.isEmpty())
    {   // create new key
        m_pKey1DCB->InsertEntry(aKey);
    }

    aKey = m_pKey2DCB->GetText();
    nPos = m_pKey2DCB->GetEntryPos(aKey);

    if(nPos == COMBOBOX_ENTRY_NOTFOUND && !aKey.isEmpty())
    {   // create new key
        m_pKey2DCB->InsertEntry(aKey);
    }
}

class SwNewUserIdxDlg : public weld::GenericDialogController
{
    SwIndexMarkPane* m_pDlg;

    std::unique_ptr<weld::Button> m_xOKPB;
    std::unique_ptr<weld::Entry> m_xNameED;

    DECL_LINK(ModifyHdl, weld::Entry&, void);

public:
    explicit SwNewUserIdxDlg(SwIndexMarkPane* pPane)
        : GenericDialogController(pPane->GetFrameWeld(), "modules/swriter/ui/newuserindexdialog.ui", "NewUserIndexDialog")
        , m_pDlg(pPane)
        , m_xOKPB(m_xBuilder->weld_button("ok"))
        , m_xNameED(m_xBuilder->weld_entry("entry"))
    {
        m_xNameED->connect_changed(LINK(this, SwNewUserIdxDlg, ModifyHdl));
        m_xOKPB->set_sensitive(false);
        m_xNameED->grab_focus();
    }
    OUString GetName() const { return m_xNameED->get_text(); }
};

IMPL_LINK( SwNewUserIdxDlg, ModifyHdl, weld::Entry&, rEdit, void)
{
    m_xOKPB->set_sensitive(!rEdit.get_text().isEmpty() && !m_pDlg->IsTOXType(rEdit.get_text()));
}

IMPL_LINK_NOARG(SwIndexMarkPane, NewUserIdxHdl, Button*, void)
{
    SwNewUserIdxDlg aDlg(this);
    if (aDlg.run() == RET_OK)
    {
        OUString sNewName(aDlg.GetName());
        m_pTypeDCB->InsertEntry(sNewName);
        m_pTypeDCB->SelectEntry(sNewName);
    }
}

IMPL_LINK( SwIndexMarkPane, SearchTypeHdl, Button*, pBox, void)
{
    bool bEnable = static_cast<CheckBox*>(pBox)->IsChecked() && pBox->IsEnabled();
    m_pSearchCaseWordOnlyCB->Enable(bEnable);
    m_pSearchCaseSensitiveCB->Enable(bEnable);
}

IMPL_LINK( SwIndexMarkPane, InsertHdl, Button *, pButton, void )
{
    Apply();
    //close the dialog if only one entry is available
    if(!bNewMark && !m_pPrevBT->IsVisible() && !m_pNextBT->IsVisible())
        CloseHdl(pButton);
}

IMPL_LINK_NOARG(SwIndexMarkPane, CloseHdl, Button*, void)
{
    if(bNewMark)
    {
        SfxViewFrame::Current()->GetDispatcher()->Execute(FN_INSERT_IDX_ENTRY_DLG,
                    SfxCallMode::ASYNCHRON|SfxCallMode::RECORD);
    }
    else
    {
        m_rDialog.EndDialog();
    }
}

// select index type only when inserting
IMPL_LINK( SwIndexMarkPane, ModifyListBoxHdl, ListBox&, rBox, void )
{
    ModifyHdl(&rBox);
}

IMPL_LINK( SwIndexMarkPane, ModifyEditHdl, Edit&, rEdit, void )
{
    ModifyHdl(&rEdit);
}

void SwIndexMarkPane::ModifyHdl(Control const * pBox)
{
    if (m_pTypeDCB == pBox)
    {
        // set index type
        sal_Int32 nPos = m_pTypeDCB->GetEntryPos(m_pTypeDCB->GetSelectedEntry());
        bool bLevelEnable = false,
             bKeyEnable   = false,
             bSetKey2     = false,
             bKey2Enable  = false,
             bEntryHasText   = false,
             bKey1HasText    = false,
             bKey2HasText    = false;
        if(nPos == POS_INDEX)
        {
            if(!m_pEntryED->GetText().isEmpty())
                bEntryHasText = true;
            m_pPhoneticED0->SetText(GetDefaultPhoneticReading(m_pEntryED->GetText()));

            bKeyEnable = true;
            m_pKey1DCB->SetText(m_pKey1DCB->GetEntry(nKey1Pos));
            m_pPhoneticED1->SetText(GetDefaultPhoneticReading(m_pKey1DCB->GetText()));
            if(!m_pKey1DCB->GetText().isEmpty())
            {
                bKey1HasText = bSetKey2 = bKey2Enable = true;
                m_pKey2DCB->SetText(m_pKey2DCB->GetEntry(nKey2Pos));
                m_pPhoneticED2->SetText(GetDefaultPhoneticReading(m_pKey2DCB->GetText()));
                if(!m_pKey2DCB->GetText().isEmpty())
                    bKey2HasText = true;
            }
        }
        else
        {
            bLevelEnable = true;
            m_pLevelNF->SetMax(MAXLEVEL);
            m_pLevelNF->SetValue(m_pLevelNF->Normalize(0));
            bSetKey2 = true;
        }
        m_pLevelFT->Show(bLevelEnable);
        m_pLevelNF->Show(bLevelEnable);
        m_pMainEntryCB->Show(nPos == POS_INDEX);

        m_pKey1FT->Enable(bKeyEnable);
        m_pKey1DCB->Enable(bKeyEnable);
        if ( bSetKey2 )
        {
            m_pKey2DCB->Enable(bKey2Enable);
            m_pKey2FT->Enable(bKey2Enable);
        }
        m_pPhoneticFT0->Enable(bKeyEnable&&bEntryHasText&&bIsPhoneticReadingEnabled);
        m_pPhoneticED0->Enable(bKeyEnable&&bEntryHasText&&bIsPhoneticReadingEnabled);
        m_pPhoneticFT1->Enable(bKeyEnable&&bKey1HasText&&bIsPhoneticReadingEnabled);
        m_pPhoneticED1->Enable(bKeyEnable&&bKey1HasText&&bIsPhoneticReadingEnabled);
        m_pPhoneticFT2->Enable(bKeyEnable&&bKey2HasText&&bIsPhoneticReadingEnabled);
        m_pPhoneticED2->Enable(bKeyEnable&&bKey2HasText&&bIsPhoneticReadingEnabled);
    }
    else //m_pEntryED  !!m_pEntryED is not a ListBox but a Edit
    {
        bool bHasText = !m_pEntryED->GetText().isEmpty();
        if(!bHasText)
        {
            m_pPhoneticED0->SetText(OUString());
            bPhoneticED0_ChangedByUser = false;
        }
        else if(!bPhoneticED0_ChangedByUser)
            m_pPhoneticED0->SetText(GetDefaultPhoneticReading(m_pEntryED->GetText()));

        m_pPhoneticFT0->Enable(bHasText&&bIsPhoneticReadingEnabled);
        m_pPhoneticED0->Enable(bHasText&&bIsPhoneticReadingEnabled);
    }
    m_pOKBT->Enable(!pSh->HasReadonlySel() &&
        (!m_pEntryED->GetText().isEmpty() || pSh->GetCursorCnt(false)));
}

IMPL_LINK_NOARG(SwIndexMarkPane, NextHdl, Button*, void)
{
    InsertUpdate();
    pTOXMgr->NextTOXMark();
    UpdateDialog();
}

IMPL_LINK_NOARG(SwIndexMarkPane, NextSameHdl, Button*, void)
{
    InsertUpdate();
    pTOXMgr->NextTOXMark(true);
    UpdateDialog();
}

IMPL_LINK_NOARG(SwIndexMarkPane, PrevHdl, Button*, void)
{
    InsertUpdate();
    pTOXMgr->PrevTOXMark();
    UpdateDialog();
}

IMPL_LINK_NOARG(SwIndexMarkPane, PrevSameHdl, Button*, void)
{
    InsertUpdate();
    pTOXMgr->PrevTOXMark(true);
    UpdateDialog();
}

IMPL_LINK_NOARG(SwIndexMarkPane, DelHdl, Button*, void)
{
    bDel = true;
    InsertUpdate();
    bDel = false;

    if(pTOXMgr->GetCurTOXMark())
        UpdateDialog();
    else
    {
        CloseHdl(m_pCloseBT);
        SfxViewFrame::Current()->GetBindings().Invalidate(FN_EDIT_IDX_ENTRY_DLG);
    }
}

// renew dialog view
void SwIndexMarkPane::UpdateDialog()
{
    OSL_ENSURE(pSh && pTOXMgr, "no shell?");
    SwTOXMark* pMark = pTOXMgr->GetCurTOXMark();
    OSL_ENSURE(pMark, "no current marker");
    if(!pMark)
        return;

    SwViewShell::SetCareWin(&m_rDialog);

    aOrgStr = pMark->GetText(pSh->GetLayout());
    m_pEntryED->SetText(aOrgStr);

    // set index type
    bool bLevelEnable = true,
         bKeyEnable   = false,
         bKey2Enable  = false,
         bEntryHasText  = false,
         bKey1HasText   = false,
         bKey2HasText   = false;

    TOXTypes eCurType = pMark->GetTOXType()->GetType();
    if(TOX_INDEX == eCurType)
    {
        bLevelEnable = false;
        bKeyEnable = true;
        bKey1HasText = bKey2Enable = !pMark->GetPrimaryKey().isEmpty();
        bKey2HasText = !pMark->GetSecondaryKey().isEmpty();
        bEntryHasText = !pMark->GetText(pSh->GetLayout()).isEmpty();
        m_pKey1DCB->SetText( pMark->GetPrimaryKey() );
        m_pKey2DCB->SetText( pMark->GetSecondaryKey() );
        m_pPhoneticED0->SetText( pMark->GetTextReading() );
        m_pPhoneticED1->SetText( pMark->GetPrimaryKeyReading() );
        m_pPhoneticED2->SetText( pMark->GetSecondaryKeyReading() );
        m_pMainEntryCB->Check(pMark->IsMainEntry());
    }
    else if(TOX_CONTENT == eCurType || TOX_USER == eCurType)
    {
        m_pLevelNF->SetValue(m_pLevelNF->Normalize(pMark->GetLevel()));
    }
    m_pKey1FT->Enable(bKeyEnable);
    m_pKey1DCB->Enable(bKeyEnable);
    m_pLevelNF->SetMax(MAXLEVEL);
    m_pLevelFT->Show(bLevelEnable);
    m_pLevelNF->Show(bLevelEnable);
    m_pMainEntryCB->Show(!bLevelEnable);
    m_pKey2FT->Enable(bKey2Enable);
    m_pKey2DCB->Enable(bKey2Enable);

    UpdateLanguageDependenciesForPhoneticReading();
    m_pPhoneticFT0->Enable(bKeyEnable&&bEntryHasText&&bIsPhoneticReadingEnabled);
    m_pPhoneticED0->Enable(bKeyEnable&&bEntryHasText&&bIsPhoneticReadingEnabled);
    m_pPhoneticFT1->Enable(bKeyEnable&&bKey1HasText&&bIsPhoneticReadingEnabled);
    m_pPhoneticED1->Enable(bKeyEnable&&bKey1HasText&&bIsPhoneticReadingEnabled);
    m_pPhoneticFT2->Enable(bKeyEnable&&bKey2HasText&&bIsPhoneticReadingEnabled);
    m_pPhoneticED2->Enable(bKeyEnable&&bKey2HasText&&bIsPhoneticReadingEnabled);

    // set index type
    m_pTypeDCB->SelectEntry(pMark->GetTOXType()->GetTypeName());

    // set Next - Prev - Buttons
    pSh->SttCursorMove();
    if( m_pPrevBT->IsVisible() )
    {
        const SwTOXMark* pMoveMark = &pSh->GotoTOXMark( *pMark, TOX_PRV );
        if( pMoveMark != pMark )
            pSh->GotoTOXMark( *pMoveMark, TOX_NXT );
        m_pPrevBT->Enable( pMoveMark != pMark );
        pMoveMark = &pSh->GotoTOXMark( *pMark, TOX_NXT );
        if( pMoveMark != pMark )
            pSh->GotoTOXMark( *pMoveMark, TOX_PRV );
        m_pNextBT->Enable( pMoveMark != pMark );
    }

    if( m_pPrevSameBT->IsVisible() )
    {
        const SwTOXMark* pMoveMark = &pSh->GotoTOXMark( *pMark, TOX_SAME_PRV );
        if( pMoveMark != pMark )
            pSh->GotoTOXMark( *pMoveMark, TOX_SAME_NXT );
        m_pPrevSameBT->Enable( pMoveMark != pMark );
        pMoveMark = &pSh->GotoTOXMark( *pMark, TOX_SAME_NXT );
        if( pMoveMark != pMark )
            pSh->GotoTOXMark( *pMoveMark, TOX_SAME_PRV );
        m_pNextSameBT->Enable( pMoveMark != pMark );
    }

    bool bEnable = !pSh->HasReadonlySel();
    m_pOKBT->Enable( bEnable );
    m_pDelBT->Enable( bEnable );
    m_pEntryED->SetReadOnly( !bEnable );
    m_pLevelNF->SetReadOnly( !bEnable );
    m_pKey1DCB->SetReadOnly( !bEnable );
    m_pKey2DCB->SetReadOnly( !bEnable );

    pSh->SelectTextAttr( RES_TXTATR_TOXMARK, pMark->GetTextTOXMark() );
    // we need the point at the start of the attribute
    pSh->SwapPam();

    pSh->EndCursorMove();
}

// Remind whether the edit boxes for Phonetic reading are changed manually
IMPL_LINK( SwIndexMarkPane, PhoneticEDModifyHdl, Edit&, rEdit, void )
{
    if (m_pPhoneticED0 == &rEdit)
    {
        bPhoneticED0_ChangedByUser = !rEdit.GetText().isEmpty();
    }
    else if (m_pPhoneticED1 == &rEdit)
    {
        bPhoneticED1_ChangedByUser = !rEdit.GetText().isEmpty();
    }
    else if (m_pPhoneticED2 == &rEdit)
    {
        bPhoneticED2_ChangedByUser = !rEdit.GetText().isEmpty();
    }
}

// Enable Disable of the 2nd key
IMPL_LINK( SwIndexMarkPane, KeyDCBModifyHdl, Edit&, rEdit, void )
{
    ComboBox* pBox = static_cast<ComboBox*>(&rEdit);
    if (m_pKey1DCB == pBox)
    {
        bool bEnable = !pBox->GetText().isEmpty();
        if(!bEnable)
        {
            m_pKey2DCB->SetText(OUString());
            m_pPhoneticED1->SetText(OUString());
            m_pPhoneticED2->SetText(OUString());
            bPhoneticED1_ChangedByUser = false;
            bPhoneticED2_ChangedByUser = false;
        }
        else
        {
            if(pBox->IsInDropDown())
            {
                //reset bPhoneticED1_ChangedByUser if a completely new string is selected
                bPhoneticED1_ChangedByUser = false;
            }
            if(!bPhoneticED1_ChangedByUser)
                m_pPhoneticED1->SetText(GetDefaultPhoneticReading(pBox->GetText()));
        }
        m_pKey2DCB->Enable(bEnable);
        m_pKey2FT->Enable(bEnable);
    }
    else if (m_pKey2DCB == pBox)
    {
        if(pBox->GetText().isEmpty())
        {
            m_pPhoneticED2->SetText(OUString());
            bPhoneticED2_ChangedByUser = false;
        }
        else
        {
            if(pBox->IsInDropDown())
            {
                //reset bPhoneticED1_ChangedByUser if a completely new string is selected
                bPhoneticED2_ChangedByUser = false;
            }
            if(!bPhoneticED2_ChangedByUser)
                m_pPhoneticED2->SetText(GetDefaultPhoneticReading(pBox->GetText()));
        }
    }
    bool    bKey1HasText    = !m_pKey1DCB->GetText().isEmpty();
    bool    bKey2HasText    = !m_pKey2DCB->GetText().isEmpty();

    m_pPhoneticFT1->Enable(bKey1HasText && bIsPhoneticReadingEnabled);
    m_pPhoneticED1->Enable(bKey1HasText && bIsPhoneticReadingEnabled);
    m_pPhoneticFT2->Enable(bKey2HasText && bIsPhoneticReadingEnabled);
    m_pPhoneticED2->Enable(bKey2HasText && bIsPhoneticReadingEnabled);
}

SwIndexMarkPane::~SwIndexMarkPane()
{
}

void    SwIndexMarkPane::ReInitDlg(SwWrtShell& rWrtShell, SwTOXMark const * pCurTOXMark)
{
    pSh = &rWrtShell;
    pTOXMgr.reset( new SwTOXMgr(pSh) );
    if(pCurTOXMark)
    {
        for(sal_uInt16 i = 0; i < pTOXMgr->GetTOXMarkCount(); i++)
            if(pTOXMgr->GetTOXMark(i) == pCurTOXMark)
            {
                pTOXMgr->SetCurTOXMark(i);
                break;
            }
    }
    InitControls();
}

SwIndexMarkFloatDlg::SwIndexMarkFloatDlg(SfxBindings* _pBindings,
    SfxChildWindow* pChild, vcl::Window *pParent,
    SfxChildWinInfo const * pInfo, bool bNew)
    : SfxModelessDialog(_pBindings, pChild, pParent, "IndexEntryDialog", "modules/swriter/ui/indexentry.ui")
    , m_aContent(*this, bNew, *::GetActiveWrtShell())
{
    m_aContent.ReInitDlg(*::GetActiveWrtShell());
    Initialize(pInfo);
}

void SwIndexMarkFloatDlg::Activate()
{
    SfxModelessDialog::Activate();
    m_aContent.Activate();
}

void SwIndexMarkFloatDlg::ReInitDlg(SwWrtShell& rWrtShell)
{
    m_aContent.ReInitDlg( rWrtShell );
}

SwIndexMarkModalDlg::SwIndexMarkModalDlg(vcl::Window *pParent, SwWrtShell& rSh, SwTOXMark const * pCurTOXMark)
    : SvxStandardDialog(pParent, "IndexEntryDialog", "modules/swriter/ui/indexentry.ui")
    , m_aContent(*this, false, rSh)
{
    m_aContent.ReInitDlg(rSh, pCurTOXMark);
}

void SwIndexMarkModalDlg::Apply()
{
    m_aContent.Apply();
}

void SwIndexMarkModalDlg::dispose()
{
    SwViewShell::SetCareWin(nullptr);
    SvxStandardDialog::dispose();
}

class SwCreateAuthEntryDlg_Impl : public weld::GenericDialogController
{
    std::vector<std::unique_ptr<weld::Builder>> m_aBuilders;

    Link<weld::Entry&,bool>       aShortNameCheckLink;

    SwWrtShell&     rWrtSh;

    bool            m_bNewEntryMode;
    bool            m_bNameAllowed;

    std::vector<std::unique_ptr<weld::Container>> m_aOrigContainers;
    std::vector<std::unique_ptr<weld::Label>> m_aFixedTexts;
    std::unique_ptr<weld::Entry> pEdits[AUTH_FIELD_END];
    std::unique_ptr<weld::Button> m_xOKBT;
    std::unique_ptr<weld::Container> m_xBox;
    std::unique_ptr<weld::Container> m_xLeft;
    std::unique_ptr<weld::Container> m_xRight;
    std::unique_ptr<weld::ComboBoxText> m_xTypeListBox;
    std::unique_ptr<weld::ComboBoxText> m_xIdentifierBox;

    DECL_LINK(IdentifierHdl, weld::ComboBoxText&, void);
    DECL_LINK(ShortNameHdl, weld::Entry&, void);
    DECL_LINK(EnableHdl, weld::ComboBoxText&, void);

public:
    SwCreateAuthEntryDlg_Impl(weld::Window* pParent,
                              const OUString pFields[],
                              SwWrtShell& rSh,
                              bool bNewEntry,
                              bool bCreate);

    OUString        GetEntryText(ToxAuthorityField eField) const;

    void            SetCheckNameHdl(const Link<weld::Entry&,bool>& rLink) {aShortNameCheckLink = rLink;}

};

struct TextInfo
{
    ToxAuthorityField nToxField;
    const char* pHelpId;
};

static const TextInfo aTextInfoArr[] =
{
    {AUTH_FIELD_IDENTIFIER,      HID_AUTH_FIELD_IDENTIFIER      },
    {AUTH_FIELD_AUTHORITY_TYPE,  HID_AUTH_FIELD_AUTHORITY_TYPE  },
    {AUTH_FIELD_AUTHOR,          HID_AUTH_FIELD_AUTHOR          },
    {AUTH_FIELD_TITLE,           HID_AUTH_FIELD_TITLE           },
    {AUTH_FIELD_YEAR,            HID_AUTH_FIELD_YEAR            },
    {AUTH_FIELD_PUBLISHER,       HID_AUTH_FIELD_PUBLISHER       },
    {AUTH_FIELD_ADDRESS,         HID_AUTH_FIELD_ADDRESS         },
    {AUTH_FIELD_ISBN,            HID_AUTH_FIELD_ISBN            },
    {AUTH_FIELD_CHAPTER,         HID_AUTH_FIELD_CHAPTER         },
    {AUTH_FIELD_PAGES,           HID_AUTH_FIELD_PAGES           },
    {AUTH_FIELD_EDITOR,          HID_AUTH_FIELD_EDITOR          },
    {AUTH_FIELD_EDITION,         HID_AUTH_FIELD_EDITION         },
    {AUTH_FIELD_BOOKTITLE,       HID_AUTH_FIELD_BOOKTITLE       },
    {AUTH_FIELD_VOLUME,          HID_AUTH_FIELD_VOLUME          },
    {AUTH_FIELD_HOWPUBLISHED,    HID_AUTH_FIELD_HOWPUBLISHED    },
    {AUTH_FIELD_ORGANIZATIONS,   HID_AUTH_FIELD_ORGANIZATIONS   },
    {AUTH_FIELD_INSTITUTION,     HID_AUTH_FIELD_INSTITUTION     },
    {AUTH_FIELD_SCHOOL,          HID_AUTH_FIELD_SCHOOL          },
    {AUTH_FIELD_REPORT_TYPE,     HID_AUTH_FIELD_REPORT_TYPE     },
    {AUTH_FIELD_MONTH,           HID_AUTH_FIELD_MONTH           },
    {AUTH_FIELD_JOURNAL,         HID_AUTH_FIELD_JOURNAL         },
    {AUTH_FIELD_NUMBER,          HID_AUTH_FIELD_NUMBER          },
    {AUTH_FIELD_SERIES,          HID_AUTH_FIELD_SERIES          },
    {AUTH_FIELD_ANNOTE,          HID_AUTH_FIELD_ANNOTE          },
    {AUTH_FIELD_NOTE,            HID_AUTH_FIELD_NOTE            },
    {AUTH_FIELD_URL,             HID_AUTH_FIELD_URL             },
    {AUTH_FIELD_CUSTOM1,         HID_AUTH_FIELD_CUSTOM1         },
    {AUTH_FIELD_CUSTOM2,         HID_AUTH_FIELD_CUSTOM2         },
    {AUTH_FIELD_CUSTOM3,         HID_AUTH_FIELD_CUSTOM3         },
    {AUTH_FIELD_CUSTOM4,         HID_AUTH_FIELD_CUSTOM4         },
    {AUTH_FIELD_CUSTOM5,         HID_AUTH_FIELD_CUSTOM5         }
};

bool SwAuthorMarkPane::bIsFromComponent = true;

SwAuthorMarkPane::SwAuthorMarkPane(Dialog &rDialog, bool bNewDlg)
    : m_rDialog(rDialog)
    , bNewEntry(bNewDlg)
    , bBibAccessInitialized(false)
    , pSh(nullptr)
{
    m_rDialog.get(m_pFromComponentRB, "frombibliography");
    m_rDialog.get(m_pFromDocContentRB, "fromdocument");
    m_rDialog.get(m_pAuthorFI, "author");
    m_rDialog.get(m_pTitleFI, "title");
    m_rDialog.get(m_pEntryED, "entryed");
    m_rDialog.get(m_pEntryLB, "entrylb");
    m_rDialog.get(m_pActionBT,
        bNewEntry ? OString("insert") : OString("modify"));
    m_pActionBT->Show();
    m_rDialog.get(m_pCloseBT, "close");
    m_rDialog.get(m_pCreateEntryPB, "new");
    m_rDialog.get(m_pEditEntryPB, "edit");

    m_pFromComponentRB->Show(bNewEntry);
    m_pFromDocContentRB->Show(bNewEntry);
    m_pFromComponentRB->Check(bIsFromComponent);
    m_pFromDocContentRB->Check(!bIsFromComponent);

    m_pActionBT->SetClickHdl(LINK(this,SwAuthorMarkPane, InsertHdl));
    m_pCloseBT->SetClickHdl(LINK(this,SwAuthorMarkPane, CloseHdl));
    m_pCreateEntryPB->SetClickHdl(LINK(this,SwAuthorMarkPane, CreateEntryHdl));
    m_pEditEntryPB->SetClickHdl(LINK(this,SwAuthorMarkPane, CreateEntryHdl));
    m_pFromComponentRB->SetClickHdl(LINK(this,SwAuthorMarkPane, ChangeSourceHdl));
    m_pFromDocContentRB->SetClickHdl(LINK(this,SwAuthorMarkPane, ChangeSourceHdl));
    m_pEntryED->SetModifyHdl(LINK(this,SwAuthorMarkPane, EditModifyHdl));

    m_rDialog.SetText(SwResId(
                    bNewEntry ? STR_AUTHMRK_INSERT : STR_AUTHMRK_EDIT));

    m_pEntryED->Show(!bNewEntry);
    m_pEntryLB->Show(bNewEntry);
    if(bNewEntry)
    {
        m_pEntryLB->SetSelectHdl(LINK(this, SwAuthorMarkPane, CompEntryHdl));
    }
}

void    SwAuthorMarkPane::ReInitDlg(SwWrtShell& rWrtShell)
{
    pSh = &rWrtShell;
    InitControls();
}

IMPL_LINK_NOARG(SwAuthorMarkPane, CloseHdl, Button*, void)
{
    if(bNewEntry)
    {
        SfxViewFrame::Current()->GetDispatcher()->Execute(FN_INSERT_AUTH_ENTRY_DLG,
                    SfxCallMode::ASYNCHRON|SfxCallMode::RECORD);
    }
    else
    {
        m_rDialog.EndDialog();
    }
}

static OUString lcl_FindColumnEntry(const beans::PropertyValue* pFields, sal_Int32 nLen, const OUString& rColumnTitle)
{
    for(sal_Int32 i = 0; i < nLen; i++)
    {
        OUString sRet;
        if(pFields[i].Name == rColumnTitle &&
            (pFields[i].Value >>= sRet))
        {
            return sRet;
        }
    }
    return OUString();
}

IMPL_LINK( SwAuthorMarkPane, CompEntryHdl, ListBox&, rBox, void)
{
    const OUString sEntry(rBox.GetSelectedEntry());
    if(bIsFromComponent)
    {
        if(xBibAccess.is() && !sEntry.isEmpty())
        {
            if(xBibAccess->hasByName(sEntry))
            {
                uno::Any aEntry(xBibAccess->getByName(sEntry));
                uno::Sequence<beans::PropertyValue> aFieldProps;
                if(aEntry >>= aFieldProps)
                {
                    const beans::PropertyValue* pProps = aFieldProps.getConstArray();
                    for(sal_Int32 i = 0; i < AUTH_FIELD_END && i < aFieldProps.getLength(); i++)
                    {
                        m_sFields[i] = lcl_FindColumnEntry(
                                pProps, aFieldProps.getLength(), m_sColumnTitles[i]);
                    }
                }
            }
        }
    }
    else
    {
        if(!sEntry.isEmpty())
        {
            const SwAuthorityFieldType* pFType = static_cast<const SwAuthorityFieldType*>(
                                        pSh->GetFieldType(SwFieldIds::TableOfAuthorities, OUString()));
            const SwAuthEntry*  pEntry = pFType ? pFType->GetEntryByIdentifier(sEntry) : nullptr;
            for(int i = 0; i < AUTH_FIELD_END; i++)
                m_sFields[i] = pEntry ?
                            pEntry->GetAuthorField(static_cast<ToxAuthorityField>(i)) : OUString();
        }
    }
    if (rBox.GetSelectedEntry().isEmpty())
    {
        for(OUString & s : m_sFields)
            s.clear();
    }
    m_pAuthorFI->SetText(m_sFields[AUTH_FIELD_AUTHOR]);
    m_pTitleFI->SetText(m_sFields[AUTH_FIELD_TITLE]);
}

IMPL_LINK_NOARG(SwAuthorMarkPane, InsertHdl, Button*, void)
{
    //insert or update the SwAuthorityField...
    if(pSh)
    {
        bool bDifferent = false;
        OSL_ENSURE(!m_sFields[AUTH_FIELD_IDENTIFIER].isEmpty() , "No Id is set!");
        OSL_ENSURE(!m_sFields[AUTH_FIELD_AUTHORITY_TYPE].isEmpty() , "No authority type is set!");
        //check if the entry already exists with different content
        const SwAuthorityFieldType* pFType = static_cast<const SwAuthorityFieldType*>(
                                        pSh->GetFieldType(SwFieldIds::TableOfAuthorities, OUString()));
        const SwAuthEntry*  pEntry = pFType ?
                pFType->GetEntryByIdentifier( m_sFields[AUTH_FIELD_IDENTIFIER])
                : nullptr;
        if(pEntry)
        {
            for(int i = 0; i < AUTH_FIELD_END && !bDifferent; i++)
                bDifferent |= m_sFields[i] != pEntry->GetAuthorField(static_cast<ToxAuthorityField>(i));
            if(bDifferent)
            {
                std::unique_ptr<weld::MessageDialog> xQuery(Application::CreateMessageDialog(m_rDialog.GetFrameWeld(),
                                                            VclMessageType::Question, VclButtonsType::YesNo,
                                                            SwResId(STR_QUERY_CHANGE_AUTH_ENTRY)));
                if (RET_YES != xQuery->run())
                    return;
            }
        }

        SwFieldMgr aMgr(pSh);
        OUString sFields;
        for(OUString & s : m_sFields)
        {
            sFields += s + OUStringLiteral1(TOX_STYLE_DELIMITER);
        }
        if(bNewEntry)
        {
            if(bDifferent)
            {
                SwAuthEntry aNewData;
                for(int i = 0; i < AUTH_FIELD_END; i++)
                    aNewData.SetAuthorField(static_cast<ToxAuthorityField>(i), m_sFields[i]);
                pSh->ChangeAuthorityData(&aNewData);
            }
            SwInsertField_Data aData(TYP_AUTHORITY, 0, sFields, OUString(), 0 );
            aMgr.InsertField( aData );
        }
        else if(aMgr.GetCurField())
        {
            aMgr.UpdateCurField(0, sFields, OUString());
        }
    }
    if(!bNewEntry)
        CloseHdl(nullptr);
}

IMPL_LINK(SwAuthorMarkPane, CreateEntryHdl, Button*, pButton, void)
{
    bool bCreate = pButton == m_pCreateEntryPB;
    OUString sOldId = m_sCreatedEntry[0];
    for(int i = 0; i < AUTH_FIELD_END; i++)
        m_sCreatedEntry[i] = bCreate ? OUString() : m_sFields[i];
    SwCreateAuthEntryDlg_Impl aDlg(pButton->GetFrameWeld(),
                bCreate ? m_sCreatedEntry : m_sFields,
                *pSh, bNewEntry, bCreate);
    if(bNewEntry)
    {
        aDlg.SetCheckNameHdl(LINK(this, SwAuthorMarkPane, IsEntryAllowedHdl));
    }
    if(RET_OK == aDlg.run())
    {
        if(bCreate && !sOldId.isEmpty())
        {
            m_pEntryLB->RemoveEntry(sOldId);
        }
        for(int i = 0; i < AUTH_FIELD_END; i++)
        {
            m_sFields[i] = aDlg.GetEntryText(static_cast<ToxAuthorityField>(i));
            m_sCreatedEntry[i] = m_sFields[i];
        }
        if(bNewEntry && !m_pFromDocContentRB->IsChecked())
        {
            m_pFromDocContentRB->Check();
            ChangeSourceHdl(m_pFromDocContentRB);
        }
        if(bCreate)
        {
            OSL_ENSURE(LISTBOX_ENTRY_NOTFOUND ==
                        m_pEntryLB->GetEntryPos(m_sFields[AUTH_FIELD_IDENTIFIER]),
                        "entry exists!");
            m_pEntryLB->InsertEntry(m_sFields[AUTH_FIELD_IDENTIFIER]);
            m_pEntryLB->SelectEntry(m_sFields[AUTH_FIELD_IDENTIFIER]);
        }
        m_pEntryED->SetText(m_sFields[AUTH_FIELD_IDENTIFIER]);
        m_pAuthorFI->SetText(m_sFields[AUTH_FIELD_AUTHOR]);
        m_pTitleFI->SetText(m_sFields[AUTH_FIELD_TITLE]);
        m_pActionBT->Enable();
    }
}

IMPL_LINK(SwAuthorMarkPane, ChangeSourceHdl, Button*, pButton, void)
{
    bool bFromComp = (pButton == m_pFromComponentRB);
    bIsFromComponent = bFromComp;
    m_pCreateEntryPB->Enable(!bIsFromComponent);
    m_pEntryLB->Clear();
    if(bIsFromComponent)
    {
        if(!bBibAccessInitialized)
        {
            uno::Reference< uno::XComponentContext > xContext = getProcessComponentContext();
            xBibAccess = frame::Bibliography::create( xContext );
            uno::Reference< beans::XPropertySet >  xPropSet(xBibAccess, uno::UNO_QUERY);
            OUString uPropName("BibliographyDataFieldNames");
            if(xPropSet.is() && xPropSet->getPropertySetInfo()->hasPropertyByName(uPropName))
            {
                uno::Any aNames = xPropSet->getPropertyValue(uPropName);
                uno::Sequence<beans::PropertyValue> aSeq;
                if( aNames >>= aSeq)
                {
                    const beans::PropertyValue* pArr = aSeq.getConstArray();
                    for(sal_Int32 i = 0; i < aSeq.getLength(); i++)
                    {
                        OUString sTitle = pArr[i].Name;
                        sal_Int16 nField = 0;
                        pArr[i].Value >>= nField;
                        if(nField >= 0 && nField < AUTH_FIELD_END)
                            m_sColumnTitles[nField] = sTitle;
                    }
                }
            }
            bBibAccessInitialized = true;
        }
        if(xBibAccess.is())
        {
            uno::Sequence<OUString> aIdentifiers = xBibAccess->getElementNames();
            const OUString* pNames = aIdentifiers.getConstArray();
            for(sal_Int32 i = 0; i < aIdentifiers.getLength(); i++)
            {
                m_pEntryLB->InsertEntry(pNames[i]);
            }
        }
    }
    else
    {
        const SwAuthorityFieldType* pFType = static_cast<const SwAuthorityFieldType*>(
                                    pSh->GetFieldType(SwFieldIds::TableOfAuthorities, OUString()));
        if(pFType)
        {
            std::vector<OUString> aIds;
            pFType->GetAllEntryIdentifiers( aIds );
            for(const OUString & i : aIds)
                m_pEntryLB->InsertEntry(i);
        }
        if(!m_sCreatedEntry[AUTH_FIELD_IDENTIFIER].isEmpty())
            m_pEntryLB->InsertEntry(m_sCreatedEntry[AUTH_FIELD_IDENTIFIER]);
    }
    m_pEntryLB->SelectEntryPos(0);
    CompEntryHdl(*m_pEntryLB);
}

IMPL_LINK(SwAuthorMarkPane, EditModifyHdl, Edit&, rEdit, void)
{
    Link<Edit*,bool> aAllowed = LINK(this, SwAuthorMarkPane, IsEditAllowedHdl);
    bool bResult = aAllowed.Call(&rEdit);
    m_pActionBT->Enable(bResult);
    if(bResult)
    {
        OUString sEntry(rEdit.GetText());
        m_sFields[AUTH_FIELD_IDENTIFIER] = sEntry;
        m_sCreatedEntry[AUTH_FIELD_IDENTIFIER] = sEntry;
    }
};

IMPL_LINK(SwAuthorMarkPane, IsEntryAllowedHdl, weld::Entry&, rEdit, bool)
{
    OUString sEntry = rEdit.get_text();
    bool bAllowed = false;
    if(!sEntry.isEmpty())
    {
        if(m_pEntryLB->GetEntryPos(sEntry) != LISTBOX_ENTRY_NOTFOUND)
            return false;
        else if(bIsFromComponent)
        {
            const SwAuthorityFieldType* pFType = static_cast<const SwAuthorityFieldType*>(
                                        pSh->GetFieldType(SwFieldIds::TableOfAuthorities, OUString()));
            bAllowed = !pFType || !pFType->GetEntryByIdentifier(sEntry);
        }
        else
        {
            bAllowed = !xBibAccess.is() || !xBibAccess->hasByName(sEntry);
        }
    }
    return bAllowed;
}

IMPL_LINK(SwAuthorMarkPane, IsEditAllowedHdl, Edit*, pEdit, bool)
{
    OUString sEntry = pEdit->GetText();
    bool bAllowed = false;
    if(!sEntry.isEmpty())
    {
        if(m_pEntryLB->GetEntryPos(sEntry) != LISTBOX_ENTRY_NOTFOUND)
            return false;
        else if(bIsFromComponent)
        {
            const SwAuthorityFieldType* pFType = static_cast<const SwAuthorityFieldType*>(
                                        pSh->GetFieldType(SwFieldIds::TableOfAuthorities, OUString()));
            bAllowed = !pFType || !pFType->GetEntryByIdentifier(sEntry);
        }
        else
        {
            bAllowed = !xBibAccess.is() || !xBibAccess->hasByName(sEntry);
        }
    }
    return bAllowed;
}

void SwAuthorMarkPane::InitControls()
{
    OSL_ENSURE(pSh, "no shell?");
    SwField* pField = pSh->GetCurField();
    OSL_ENSURE(bNewEntry || pField, "no current marker");
    if(bNewEntry)
    {
        ChangeSourceHdl(m_pFromComponentRB->IsChecked() ? m_pFromComponentRB : m_pFromDocContentRB);
        m_pCreateEntryPB->Enable(!m_pFromComponentRB->IsChecked());
        if(!m_pFromComponentRB->IsChecked() && !m_sCreatedEntry[0].isEmpty())
            for(int i = 0; i < AUTH_FIELD_END; i++)
                m_sFields[i] = m_sCreatedEntry[i];
    }
    if(bNewEntry || !pField || pField->GetTyp()->Which() != SwFieldIds::TableOfAuthorities)
        return;

    const SwAuthEntry* pEntry = static_cast<SwAuthorityFieldType*>(pField->GetTyp())->
            GetEntryByHandle(static_cast<SwAuthorityField*>(pField)->GetHandle());

    OSL_ENSURE(pEntry, "No authority entry found");
    if(!pEntry)
        return;
    for(int i = 0; i < AUTH_FIELD_END; i++)
        m_sFields[i] = pEntry->GetAuthorField(static_cast<ToxAuthorityField>(i));

    m_pEntryED->SetText(pEntry->GetAuthorField(AUTH_FIELD_IDENTIFIER));
    m_pAuthorFI->SetText(pEntry->GetAuthorField(AUTH_FIELD_AUTHOR));
    m_pTitleFI->SetText(pEntry->GetAuthorField(AUTH_FIELD_TITLE));
}

void SwAuthorMarkPane::Activate()
{
    m_pActionBT->Enable(!pSh->HasReadonlySel());
}

namespace
{
    const char* STR_AUTH_FIELD_ARY[] =
    {
        STR_AUTH_FIELD_IDENTIFIER,
        STR_AUTH_FIELD_AUTHORITY_TYPE,
        STR_AUTH_FIELD_ADDRESS,
        STR_AUTH_FIELD_ANNOTE,
        STR_AUTH_FIELD_AUTHOR,
        STR_AUTH_FIELD_BOOKTITLE,
        STR_AUTH_FIELD_CHAPTER,
        STR_AUTH_FIELD_EDITION,
        STR_AUTH_FIELD_EDITOR,
        STR_AUTH_FIELD_HOWPUBLISHED,
        STR_AUTH_FIELD_INSTITUTION,
        STR_AUTH_FIELD_JOURNAL,
        STR_AUTH_FIELD_MONTH,
        STR_AUTH_FIELD_NOTE,
        STR_AUTH_FIELD_NUMBER,
        STR_AUTH_FIELD_ORGANIZATIONS,
        STR_AUTH_FIELD_PAGES,
        STR_AUTH_FIELD_PUBLISHER,
        STR_AUTH_FIELD_SCHOOL,
        STR_AUTH_FIELD_SERIES,
        STR_AUTH_FIELD_TITLE,
        STR_AUTH_FIELD_TYPE,
        STR_AUTH_FIELD_VOLUME,
        STR_AUTH_FIELD_YEAR,
        STR_AUTH_FIELD_URL,
        STR_AUTH_FIELD_CUSTOM1,
        STR_AUTH_FIELD_CUSTOM2,
        STR_AUTH_FIELD_CUSTOM3,
        STR_AUTH_FIELD_CUSTOM4,
        STR_AUTH_FIELD_CUSTOM5,
        STR_AUTH_FIELD_ISBN
    };
}

SwCreateAuthEntryDlg_Impl::SwCreateAuthEntryDlg_Impl(weld::Window* pParent,
        const OUString pFields[],
        SwWrtShell& rSh,
        bool bNewEntry,
        bool bCreate)
    : GenericDialogController(pParent, "modules/swriter/ui/createauthorentry.ui", "CreateAuthorEntryDialog")
    , rWrtSh(rSh)
    , m_bNewEntryMode(bNewEntry)
    , m_bNameAllowed(true)
    , m_xOKBT(m_xBuilder->weld_button("ok"))
    , m_xBox(m_xBuilder->weld_container("box"))
    , m_xLeft(m_xBuilder->weld_container("leftgrid"))
    , m_xRight(m_xBuilder->weld_container("rightgrid"))
{
    bool bLeft = true;
    sal_Int32 nLeftRow(0), nRightRow(0);
    for(int nIndex = 0; nIndex < AUTH_FIELD_END; nIndex++)
    {
        //m_xBox parent just to have some parent during setup, added contents are not directly visible under m_xBox
        m_aBuilders.emplace_back(Application::CreateBuilder(m_xBox.get(), "modules/swriter/ui/bibliofragment.ui"));
        const TextInfo aCurInfo = aTextInfoArr[nIndex];

        m_aOrigContainers.emplace_back(m_aBuilders.back()->weld_container("biblioentry"));
        m_aFixedTexts.emplace_back(m_aBuilders.back()->weld_label("label"));
        if (bLeft)
            m_aOrigContainers.back()->move(m_aFixedTexts.back().get(), m_xLeft.get());
        else
            m_aOrigContainers.back()->move(m_aFixedTexts.back().get(), m_xRight.get());
        m_aFixedTexts.back()->set_grid_left_attach(0);
        m_aFixedTexts.back()->set_grid_top_attach(bLeft ? nLeftRow : nRightRow);
        m_aFixedTexts.back()->set_label(SwResId(STR_AUTH_FIELD_ARY[aCurInfo.nToxField]));
        m_aFixedTexts.back()->show();
        if( AUTH_FIELD_AUTHORITY_TYPE == aCurInfo.nToxField )
        {
            m_xTypeListBox.reset(m_aBuilders.back()->weld_combo_box_text("listbox"));
            if (bLeft)
                m_aOrigContainers.back()->move(m_xTypeListBox.get(), m_xLeft.get());
            else
                m_aOrigContainers.back()->move(m_xTypeListBox.get(), m_xRight.get());

            for (int j = 0; j < AUTH_TYPE_END; j++)
                m_xTypeListBox->append_text(SwAuthorityFieldType::GetAuthTypeName(static_cast<ToxAuthorityType>(j)));
            if(!pFields[aCurInfo.nToxField].isEmpty())
            {
                m_xTypeListBox->set_active(pFields[aCurInfo.nToxField].toInt32());
            }
            m_xTypeListBox->set_grid_left_attach(1);
            m_xTypeListBox->set_grid_top_attach(bLeft ? nLeftRow : nRightRow);
            m_xTypeListBox->set_hexpand(true);
            m_xTypeListBox->show();
            m_xTypeListBox->connect_changed(LINK(this, SwCreateAuthEntryDlg_Impl, EnableHdl));
            m_xTypeListBox->set_help_id(aCurInfo.pHelpId);
            m_aFixedTexts.back()->set_mnemonic_widget(m_xTypeListBox.get());
        }
        else if(AUTH_FIELD_IDENTIFIER == aCurInfo.nToxField && !m_bNewEntryMode)
        {
            m_xIdentifierBox.reset(m_aBuilders.back()->weld_combo_box_text("combobox"));
            if (bLeft)
                m_aOrigContainers.back()->move(m_xIdentifierBox.get(), m_xLeft.get());
            else
                m_aOrigContainers.back()->move(m_xIdentifierBox.get(), m_xRight.get());

            m_xIdentifierBox->connect_changed(LINK(this,
                                    SwCreateAuthEntryDlg_Impl, IdentifierHdl));

            const SwAuthorityFieldType* pFType = static_cast<const SwAuthorityFieldType*>(
                                        rSh.GetFieldType(SwFieldIds::TableOfAuthorities, OUString()));
            if(pFType)
            {
                std::vector<OUString> aIds;
                pFType->GetAllEntryIdentifiers( aIds );
                for (const OUString& a : aIds)
                    m_xIdentifierBox->append_text(a);
            }
            m_xIdentifierBox->set_entry_text(pFields[aCurInfo.nToxField]);
            m_xIdentifierBox->set_grid_left_attach(1);
            m_xIdentifierBox->set_grid_top_attach(bLeft ? nLeftRow : nRightRow);
            m_xIdentifierBox->set_hexpand(true);
            m_xIdentifierBox->show();
            m_xIdentifierBox->set_help_id(aCurInfo.pHelpId);
            m_aFixedTexts.back()->set_mnemonic_widget(m_xIdentifierBox.get());
        }
        else
        {
            pEdits[nIndex].reset(m_aBuilders.back()->weld_entry("entry"));
            if (bLeft)
                m_aOrigContainers.back()->move(pEdits[nIndex].get(), m_xLeft.get());
            else
                m_aOrigContainers.back()->move(pEdits[nIndex].get(), m_xRight.get());

            pEdits[nIndex]->set_grid_left_attach(1);
            pEdits[nIndex]->set_grid_top_attach(bLeft ? nLeftRow : nRightRow);
            pEdits[nIndex]->set_hexpand(true);
            pEdits[nIndex]->set_text(pFields[aCurInfo.nToxField]);
            pEdits[nIndex]->show();
            pEdits[nIndex]->set_help_id(aCurInfo.pHelpId);
            if(AUTH_FIELD_IDENTIFIER == aCurInfo.nToxField)
            {
                pEdits[nIndex]->connect_changed(LINK(this, SwCreateAuthEntryDlg_Impl, ShortNameHdl));
                m_bNameAllowed = !pFields[nIndex].isEmpty();
                if(!bCreate)
                {
                    m_aFixedTexts.back()->set_sensitive(false);
                    pEdits[nIndex]->set_sensitive(false);
                }
            }
            m_aFixedTexts.back()->set_mnemonic_widget(pEdits[nIndex].get());
        }
        if(bLeft)
            ++nLeftRow;
        else
            ++nRightRow;
        bLeft = !bLeft;
    }
    EnableHdl(*m_xTypeListBox);
}

OUString  SwCreateAuthEntryDlg_Impl::GetEntryText(ToxAuthorityField eField) const
{
    if( AUTH_FIELD_AUTHORITY_TYPE == eField )
    {
        OSL_ENSURE(m_xTypeListBox, "No ListBox");
        return OUString::number(m_xTypeListBox->get_active());
    }

    if( AUTH_FIELD_IDENTIFIER == eField && !m_bNewEntryMode)
    {
        OSL_ENSURE(m_xIdentifierBox, "No ComboBox");
        return m_xIdentifierBox->get_active_text();
    }

    for(int nIndex = 0; nIndex < AUTH_FIELD_END; nIndex++)
    {
        const TextInfo aCurInfo = aTextInfoArr[nIndex];
        if(aCurInfo.nToxField == eField)
        {
            return pEdits[nIndex]->get_text();
        }
    }

    return OUString();
}

IMPL_LINK(SwCreateAuthEntryDlg_Impl, IdentifierHdl, weld::ComboBoxText&, rBox, void)
{
    const SwAuthorityFieldType* pFType = static_cast<const SwAuthorityFieldType*>(
                                rWrtSh.GetFieldType(SwFieldIds::TableOfAuthorities, OUString()));
    if(pFType)
    {
        const SwAuthEntry* pEntry = pFType->GetEntryByIdentifier(
                                                        rBox.get_active_text());
        if(pEntry)
        {
            for(int i = 0; i < AUTH_FIELD_END; i++)
            {
                const TextInfo aCurInfo = aTextInfoArr[i];
                if(AUTH_FIELD_IDENTIFIER == aCurInfo.nToxField)
                    continue;
                if(AUTH_FIELD_AUTHORITY_TYPE == aCurInfo.nToxField)
                    m_xTypeListBox->set_active_text(
                                pEntry->GetAuthorField(aCurInfo.nToxField));
                else
                    pEdits[i]->set_text(
                                pEntry->GetAuthorField(aCurInfo.nToxField));
            }
        }
    }
}

IMPL_LINK(SwCreateAuthEntryDlg_Impl, ShortNameHdl, weld::Entry&, rEdit, void)
{
    if (aShortNameCheckLink.IsSet())
    {
        bool bEnable = aShortNameCheckLink.Call(rEdit);
        m_bNameAllowed |= bEnable;
        m_xOKBT->set_sensitive(m_xTypeListBox->get_active() != -1 && bEnable);
    }
}

IMPL_LINK(SwCreateAuthEntryDlg_Impl, EnableHdl, weld::ComboBoxText&, rBox, void)
{
    m_xOKBT->set_sensitive(m_bNameAllowed && rBox.get_active() != -1);
};

SwAuthMarkFloatDlg::SwAuthMarkFloatDlg(SfxBindings* _pBindings,
                                   SfxChildWindow* pChild,
                                   vcl::Window *pParent,
                                   SfxChildWinInfo const * pInfo,
                                   bool bNew)
    : SfxModelessDialog(_pBindings, pChild, pParent,
        "BibliographyEntryDialog", "modules/swriter/ui/bibliographyentry.ui")
    , m_aContent(*this, bNew)
{
    Initialize(pInfo);
    SwWrtShell* pWrtShell = ::GetActiveWrtShell();
    if (pWrtShell)
        m_aContent.ReInitDlg(*pWrtShell);
}

void SwAuthMarkFloatDlg::Activate()
{
    SfxModelessDialog::Activate();
    m_aContent.Activate();
}

void SwAuthMarkFloatDlg::ReInitDlg(SwWrtShell& rWrtShell)
{
    m_aContent.ReInitDlg( rWrtShell );
}

SwAuthMarkModalDlg::SwAuthMarkModalDlg(vcl::Window *pParent, SwWrtShell& rSh)
    : SvxStandardDialog(pParent, "BibliographyEntryDialog",
        "modules/swriter/ui/bibliographyentry.ui")
    , m_aContent(*this, false)
{
    m_aContent.ReInitDlg(rSh);
}

void SwAuthMarkModalDlg::Apply()
{
    m_aContent.InsertHdl(nullptr);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
