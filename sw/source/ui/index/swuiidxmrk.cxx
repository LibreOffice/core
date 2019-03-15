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
SwIndexMarkPane::SwIndexMarkPane(const std::shared_ptr<weld::Dialog>& rDialog, weld::Builder& rBuilder, bool bNewDlg,
    SwWrtShell& rWrtShell)
    : m_xDialog(rDialog)
    , m_bDel(false)
    , m_bNewMark(bNewDlg)
    , m_bSelected(false)
    , m_bPhoneticED0_ChangedByUser(false)
    , m_bPhoneticED1_ChangedByUser(false)
    , m_bPhoneticED2_ChangedByUser(false)
    , m_nLangForPhoneticReading(LANGUAGE_CHINESE_SIMPLIFIED)
    , m_bIsPhoneticReadingEnabled(false)
    , m_pSh(&rWrtShell)
    , m_xTypeFT(rBuilder.weld_label("typeft"))
    , m_xTypeDCB(rBuilder.weld_combo_box("typecb"))
    , m_xNewBT(rBuilder.weld_button("new"))
    , m_xEntryED(rBuilder.weld_entry("entryed"))
    , m_xSyncED(rBuilder.weld_button("sync"))
    , m_xPhoneticFT0(rBuilder.weld_label("phonetic0ft"))
    , m_xPhoneticED0(rBuilder.weld_entry("phonetic0ed"))
    , m_xKey1FT(rBuilder.weld_label("key1ft"))
    , m_xKey1DCB(rBuilder.weld_combo_box("key1cb"))
    , m_xPhoneticFT1(rBuilder.weld_label("phonetic1ft"))
    , m_xPhoneticED1(rBuilder.weld_entry("phonetic1ed"))
    , m_xKey2FT(rBuilder.weld_label("key2ft"))
    , m_xKey2DCB(rBuilder.weld_combo_box("key2cb"))
    , m_xPhoneticFT2(rBuilder.weld_label("phonetic2ft"))
    , m_xPhoneticED2(rBuilder.weld_entry("phonetic2ed"))
    , m_xLevelFT(rBuilder.weld_label("levelft"))
    , m_xLevelNF(rBuilder.weld_spin_button("levelnf"))
    , m_xMainEntryCB(rBuilder.weld_check_button("mainentrycb"))
    , m_xApplyToAllCB(rBuilder.weld_check_button("applytoallcb"))
    , m_xSearchCaseSensitiveCB(rBuilder.weld_check_button("searchcasesensitivecb"))
    , m_xSearchCaseWordOnlyCB(rBuilder.weld_check_button("searchcasewordonlycb"))
    , m_xOKBT(bNewDlg ? rBuilder.weld_button("insert") : rBuilder.weld_button("ok"))
    , m_xCloseBT(rBuilder.weld_button("close"))
    , m_xDelBT(rBuilder.weld_button("delete"))
    , m_xPrevSameBT(rBuilder.weld_button("first"))
    , m_xNextSameBT(rBuilder.weld_button("last"))
    , m_xPrevBT(rBuilder.weld_button("previous"))
    , m_xNextBT(rBuilder.weld_button("next"))
{
    m_xSyncED->show();

    if (SvtCJKOptions().IsCJKFontEnabled())
    {
        uno::Reference< uno::XComponentContext > xContext = getProcessComponentContext();

        m_xExtendedIndexEntrySupplier = i18n::IndexEntrySupplier::create(xContext);

        m_xPhoneticFT0->show();
        m_xPhoneticED0->show();
        m_xPhoneticFT1->show();
        m_xPhoneticED1->show();
        m_xPhoneticFT2->show();
        m_xPhoneticED2->show();
    }

    m_xDialog->set_title(SwResId(m_bNewMark ? STR_IDXMRK_INSERT : STR_IDXMRK_EDIT));

    m_xDelBT->connect_clicked(LINK(this,SwIndexMarkPane,        DelHdl));
    m_xPrevBT->connect_clicked(LINK(this,SwIndexMarkPane,       PrevHdl));
    m_xPrevSameBT->connect_clicked(LINK(this,SwIndexMarkPane,   PrevSameHdl));
    m_xNextBT->connect_clicked(LINK(this,SwIndexMarkPane,       NextHdl));
    m_xNextSameBT->connect_clicked(LINK(this,SwIndexMarkPane,   NextSameHdl));
    m_xTypeDCB->connect_changed(LINK(this,SwIndexMarkPane,     ModifyListBoxHdl));
    m_xKey1DCB->connect_changed(LINK(this,SwIndexMarkPane,      KeyDCBModifyHdl));
    m_xKey2DCB->connect_changed(LINK(this,SwIndexMarkPane,     KeyDCBModifyHdl));
    m_xCloseBT->connect_clicked(LINK(this,SwIndexMarkPane,      CloseHdl));
    m_xEntryED->connect_changed(LINK(this,SwIndexMarkPane,     ModifyEditHdl));
    m_xNewBT->connect_clicked(LINK(this, SwIndexMarkPane,       NewUserIdxHdl));
    m_xApplyToAllCB->connect_toggled(LINK(this, SwIndexMarkPane, SearchTypeHdl));
    m_xPhoneticED0->connect_changed(LINK(this,SwIndexMarkPane, PhoneticEDModifyHdl));
    m_xPhoneticED1->connect_changed(LINK(this,SwIndexMarkPane, PhoneticEDModifyHdl));
    m_xPhoneticED2->connect_changed(LINK(this,SwIndexMarkPane, PhoneticEDModifyHdl));
    m_xSyncED->connect_clicked(LINK(this, SwIndexMarkPane, SyncSelectionHdl));

    if (m_bNewMark)
        m_xDelBT->hide();
    else
        m_xNewBT->hide();
    m_xOKBT->show();
    m_xOKBT->connect_clicked(LINK(this, SwIndexMarkPane, InsertHdl));

    m_xEntryED->grab_focus();
}

// Newly initialise controls with the new selection
void SwIndexMarkPane::InitControls()
{
    assert(m_pSh && m_pTOXMgr && "no shell?");
    // contents index
    const SwTOXType* pType = m_pTOXMgr->GetTOXType(TOX_CONTENT);
    assert(pType && "No directory type !!");
    OUString sTmpTypeSelection;
    if (m_xTypeDCB->get_active() != -1)
        sTmpTypeSelection = m_xTypeDCB->get_active_text();
    m_xTypeDCB->clear();
    m_xTypeDCB->append_text(pType->GetTypeName());

    // keyword index
    pType = m_pTOXMgr->GetTOXType(TOX_INDEX);
    assert(pType && "No directory type !!");
    m_xTypeDCB->append_text(pType->GetTypeName());

    // user index
    sal_uInt16 nCount = m_pSh->GetTOXTypeCount(TOX_USER);
    for (sal_uInt16 i = 0; i < nCount; ++i)
        m_xTypeDCB->append_text(m_pSh->GetTOXType(TOX_USER, i)->GetTypeName());

    // read keywords primary
    {
        std::vector<OUString> aArr;
        m_pSh->GetTOIKeys(TOI_PRIMARY, aArr);
        std::sort(aArr.begin(), aArr.end());
        auto last = std::unique(aArr.begin(), aArr.end());
        for (auto it = aArr.begin(); it != last; ++it)
            m_xKey1DCB->append_text(*it);
    }

    // read keywords secondary
    {
        std::vector<OUString> aArr;
        m_pSh->GetTOIKeys( TOI_SECONDARY, aArr );
        std::sort(aArr.begin(), aArr.end());
        auto last = std::unique(aArr.begin(), aArr.end());
        for (auto it = aArr.begin(); it != last; ++it)
            m_xKey2DCB->append_text(*it);
    }

    UpdateLanguageDependenciesForPhoneticReading();

    // current entry
    const SwTOXMark* pMark = m_pTOXMgr->GetCurTOXMark();
    if( pMark && !m_bNewMark)
    {
        // Controls-Handling

        // only if there are more than one
        // if equal it lands at the same entry
        m_pSh->SttCursorMove();

        const SwTOXMark* pMoveMark;
        bool bShow = false;

        pMoveMark = &m_pSh->GotoTOXMark( *pMark, TOX_PRV );
        if( pMoveMark != pMark )
        {
            m_pSh->GotoTOXMark( *pMoveMark, TOX_NXT );
            bShow = true;
        }
        m_xPrevBT->set_sensitive(pMoveMark != pMark);
        pMoveMark = &m_pSh->GotoTOXMark( *pMark, TOX_NXT );
        if( pMoveMark != pMark )
        {
            m_pSh->GotoTOXMark( *pMoveMark, TOX_PRV );
            bShow = true;
        }
        m_xNextBT->set_sensitive(pMoveMark != pMark);
        if( bShow )
        {
            m_xPrevBT->show();
            m_xNextBT->show();
            bShow = false;
        }

        pMoveMark = &m_pSh->GotoTOXMark( *pMark, TOX_SAME_PRV );
        if( pMoveMark != pMark )
        {
            m_pSh->GotoTOXMark( *pMoveMark, TOX_SAME_NXT );
            bShow = true;
        }
        m_xPrevSameBT->set_sensitive(pMoveMark != pMark);
        pMoveMark = &m_pSh->GotoTOXMark( *pMark, TOX_SAME_NXT );
        if( pMoveMark != pMark )
        {
            m_pSh->GotoTOXMark( *pMoveMark, TOX_SAME_PRV );
            bShow = true;
        }
        m_xNextSameBT->set_sensitive(pMoveMark != pMark);
        if( bShow )
        {
            m_xNextSameBT->show();
            m_xPrevSameBT->show();
        }
        m_pSh->EndCursorMove();

        m_xTypeFT->show();

        m_xTypeDCB->set_sensitive(false);
        m_xTypeFT->set_sensitive(false);

        UpdateDialog();
    }
    else
    {   // display current selection (first element) ????
        if (m_pSh->GetCursorCnt() < 2)
        {
            m_bSelected = !m_pSh->HasSelection();
            m_aOrgStr = m_pSh->GetView().GetSelectionTextParam(true, false);
            m_xEntryED->set_text(m_aOrgStr);

            //to include all equal entries may only be allowed in the body and even there
            //only when a simple selection exists
            const FrameTypeFlags nFrameType = m_pSh->GetFrameType(nullptr,true);
            m_xApplyToAllCB->show();
            m_xSearchCaseSensitiveCB->show();
            m_xSearchCaseWordOnlyCB->show();
            m_xApplyToAllCB->set_sensitive(!m_aOrgStr.isEmpty() &&
                !(nFrameType & ( FrameTypeFlags::HEADER | FrameTypeFlags::FOOTER | FrameTypeFlags::FLY_ANY )));
            SearchTypeHdl(*m_xApplyToAllCB);
        }

        // index type is default
        if (!sTmpTypeSelection.isEmpty() && m_xTypeDCB->find_text(sTmpTypeSelection) != -1)
            m_xTypeDCB->set_active_text(sTmpTypeSelection);
        else
            m_xTypeDCB->set_active_text(m_xTypeDCB->get_text(nTypePos));
        ModifyHdl(*m_xTypeDCB);
    }
}

void    SwIndexMarkPane::UpdateLanguageDependenciesForPhoneticReading()
{
    //no phonetic reading if no global cjk support
    if( !m_xExtendedIndexEntrySupplier.is() )
    {
        m_bIsPhoneticReadingEnabled = false;
        return;
    }
    m_bIsPhoneticReadingEnabled = true;

    //get the current language
    if(!m_bNewMark) //if dialog is opened to iterate existing marks
    {
        OSL_ENSURE(m_pTOXMgr, "need TOXMgr");
        if(!m_pTOXMgr)
            return;
        SwTOXMark* pMark = m_pTOXMgr->GetCurTOXMark();
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
        m_nLangForPhoneticReading = pTextNode->GetLang( nTextIndex );
    }
    else //if dialog is opened to create a new mark
    {
        sal_uInt16 nWhich;
        switch(m_pSh->GetScriptType())
        {
            case SvtScriptType::ASIAN: nWhich = RES_CHRATR_CJK_LANGUAGE; break;
            case SvtScriptType::COMPLEX:nWhich = RES_CHRATR_CTL_LANGUAGE; break;
            default:nWhich = RES_CHRATR_LANGUAGE; break;
        }
        SfxItemSet aLangSet(m_pSh->GetAttrPool(), {{nWhich, nWhich}});
        m_pSh->GetCurAttr(aLangSet);
        m_nLangForPhoneticReading = static_cast<const SvxLanguageItem&>(aLangSet.Get(nWhich)).GetLanguage();
    }

}

OUString SwIndexMarkPane::GetDefaultPhoneticReading( const OUString& rText )
{
    if( !m_bIsPhoneticReadingEnabled )
        return OUString();

    return m_xExtendedIndexEntrySupplier->getPhoneticCandidate(rText, LanguageTag::convertToLocale( m_nLangForPhoneticReading ));
}

void    SwIndexMarkPane::Activate()
{
    // display current selection (first element) ????
    if (m_bNewMark)
    {
        m_xSyncED->set_sensitive(m_pSh->GetCursorCnt() < 2);
    }
}

IMPL_LINK_NOARG(SwIndexMarkPane, SyncSelectionHdl, weld::Button&, void)
{
    m_bSelected = !m_pSh->HasSelection();
    m_aOrgStr = m_pSh->GetView().GetSelectionTextParam(true, false);
    m_xEntryED->set_text(m_aOrgStr);

    //to include all equal entries may only be allowed in the body and even there
    //only when a simple selection exists
    const FrameTypeFlags nFrameType = m_pSh->GetFrameType(nullptr,true);
    m_xApplyToAllCB->show();
    m_xSearchCaseSensitiveCB->show();
    m_xSearchCaseWordOnlyCB->show();
    m_xApplyToAllCB->set_sensitive(!m_aOrgStr.isEmpty() &&
        !(nFrameType & ( FrameTypeFlags::HEADER | FrameTypeFlags::FOOTER | FrameTypeFlags::FLY_ANY )));
    SearchTypeHdl(*m_xApplyToAllCB);
    ModifyHdl(*m_xEntryED);
}

// evaluate Ok-Button
void SwIndexMarkPane::Apply()
{
    InsertUpdate();
    if(m_bSelected)
        m_pSh->ResetSelect(nullptr, false);
}

// apply changes
void SwIndexMarkPane::InsertUpdate()
{
    m_pSh->StartUndo(m_bDel ? SwUndoId::INDEX_ENTRY_DELETE : SwUndoId::INDEX_ENTRY_INSERT);
    m_pSh->StartAllAction();
    SwRewriter aRewriter;

    if( m_bNewMark )
    {
        InsertMark();

        if ( m_pTOXMgr->GetCurTOXMark())
            aRewriter.AddRule(UndoArg1,
                    m_pTOXMgr->GetCurTOXMark()->GetText(m_pSh->GetLayout()));
    }
    else if( !m_pSh->HasReadonlySel() )
    {
        if ( m_pTOXMgr->GetCurTOXMark())
            aRewriter.AddRule(UndoArg1,
                    m_pTOXMgr->GetCurTOXMark()->GetText(m_pSh->GetLayout()));

        if( m_bDel )
            m_pTOXMgr->DeleteTOXMark();
        else if( m_pTOXMgr->GetCurTOXMark() )
            UpdateMark();
    }

    m_pSh->EndAllAction();
    m_pSh->EndUndo(m_bDel ? SwUndoId::INDEX_ENTRY_DELETE : SwUndoId::INDEX_ENTRY_INSERT);

    if((nTypePos = m_xTypeDCB->find_text(m_xTypeDCB->get_active_text())) == -1)
        nTypePos = 0;

    nKey1Pos = m_xKey1DCB->find_text(m_xKey1DCB->get_active_text());
    nKey2Pos = m_xKey2DCB->find_text(m_xKey2DCB->get_active_text());
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
    auto nPos = m_xTypeDCB->find_text(m_xTypeDCB->get_active_text());
    TOXTypes eType = nPos == POS_CONTENT ? TOX_CONTENT :
                        nPos == POS_INDEX ? TOX_INDEX : TOX_USER;

    SwTOXMarkDescription aDesc(eType);

    const int nLevel = m_xLevelNF->denormalize(m_xLevelNF->get_value());
    switch( nPos)
    {
        case POS_CONTENT : break;
        case POS_INDEX:     // keyword index mark
        {
            UpdateKeyBoxes();
            aDesc.SetPrimKey(m_xKey1DCB->get_active_text());
            aDesc.SetSecKey(m_xKey2DCB->get_active_text());
            aDesc.SetMainEntry(m_xMainEntryCB->get_active());
            aDesc.SetPhoneticReadingOfAltStr(m_xPhoneticED0->get_text());
            aDesc.SetPhoneticReadingOfPrimKey(m_xPhoneticED1->get_text());
            aDesc.SetPhoneticReadingOfSecKey(m_xPhoneticED2->get_text());
        }
        break;
        default:            // Userdefined index mark
        {
            aDesc.SetTOUName(m_xTypeDCB->get_active_text());
        }
    }
    if (m_aOrgStr != m_xEntryED->get_text())
        aDesc.SetAltStr(m_xEntryED->get_text());
    bool bApplyAll = m_xApplyToAllCB->get_active();
    bool bWordOnly = m_xSearchCaseWordOnlyCB->get_active();
    bool bCaseSensitive = m_xSearchCaseSensitiveCB->get_active();

    m_pSh->StartAllAction();
    // all equal strings have to be selected here so that the
    // entry is applied to all equal strings
    if(bApplyAll)
    {
        lcl_SelectSameStrings(*m_pSh, bWordOnly, bCaseSensitive);
    }
    aDesc.SetLevel(nLevel);
    SwTOXMgr aMgr(m_pSh);
    aMgr.InsertTOXMark(aDesc);
    if(bApplyAll)
        m_pSh->Pop(SwCursorShell::PopMode::DeleteCurrent);

    m_pSh->EndAllAction();
}

// update mark
void SwIndexMarkPane::UpdateMark()
{
    OUString  aAltText(m_xEntryED->get_text());
    OUString* pAltText = m_aOrgStr != m_xEntryED->get_text() ? &aAltText : nullptr;
    //empty alternative texts are not allowed
    if(pAltText && pAltText->isEmpty())
        return;

    UpdateKeyBoxes();

    auto nPos = m_xTypeDCB->find_text(m_xTypeDCB->get_active_text());
    TOXTypes eType = TOX_USER;
    if(POS_CONTENT == nPos)
        eType = TOX_CONTENT;
    else if(POS_INDEX == nPos)
        eType = TOX_INDEX;

    SwTOXMarkDescription aDesc(eType);
    aDesc.SetLevel(m_xLevelNF->get_value());
    if(pAltText)
        aDesc.SetAltStr(*pAltText);

    OUString  aPrim(m_xKey1DCB->get_active_text());
    if(!aPrim.isEmpty())
        aDesc.SetPrimKey(aPrim);
    OUString  aSec(m_xKey2DCB->get_active_text());
    if(!aSec.isEmpty())
        aDesc.SetSecKey(aSec);

    if(eType == TOX_INDEX)
    {
        aDesc.SetPhoneticReadingOfAltStr(m_xPhoneticED0->get_text());
        aDesc.SetPhoneticReadingOfPrimKey(m_xPhoneticED1->get_text());
        aDesc.SetPhoneticReadingOfSecKey(m_xPhoneticED2->get_text());
    }
    aDesc.SetMainEntry(m_xMainEntryCB->get_visible() && m_xMainEntryCB->get_active());
    m_pTOXMgr->UpdateTOXMark(aDesc);
}

// insert new keys
void SwIndexMarkPane::UpdateKeyBoxes()
{
    OUString aKey(m_xKey1DCB->get_active_text());
    auto nPos = m_xKey1DCB->find_text(aKey);
    if(nPos == -1 && !aKey.isEmpty())
    {   // create new key
        m_xKey1DCB->append_text(aKey);
    }

    aKey = m_xKey2DCB->get_active_text();
    nPos = m_xKey2DCB->find_text(aKey);

    if(nPos == -1 && !aKey.isEmpty())
    {   // create new key
        m_xKey2DCB->append_text(aKey);
    }
}

class SwNewUserIdxDlg : public weld::GenericDialogController
{
    SwIndexMarkPane* m_pDlg;

    std::unique_ptr<weld::Button> m_xOKPB;
    std::unique_ptr<weld::Entry> m_xNameED;

    DECL_LINK(ModifyHdl, weld::Entry&, void);

public:
    explicit SwNewUserIdxDlg(SwIndexMarkPane* pPane, weld::Window* pParent)
        : GenericDialogController(pParent, "modules/swriter/ui/newuserindexdialog.ui", "NewUserIndexDialog")
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

IMPL_LINK_NOARG(SwIndexMarkPane, NewUserIdxHdl, weld::Button&, void)
{
    SwNewUserIdxDlg aDlg(this, m_xDialog.get());
    if (aDlg.run() == RET_OK)
    {
        OUString sNewName(aDlg.GetName());
        m_xTypeDCB->append_text(sNewName);
        m_xTypeDCB->set_active_text(sNewName);
    }
}

IMPL_LINK( SwIndexMarkPane, SearchTypeHdl, weld::ToggleButton&, rBox, void)
{
    const bool bEnable = rBox.get_active() && rBox.get_sensitive();
    m_xSearchCaseWordOnlyCB->set_sensitive(bEnable);
    m_xSearchCaseSensitiveCB->set_sensitive(bEnable);
}

IMPL_LINK(SwIndexMarkPane, InsertHdl, weld::Button&, rButton, void)
{
    Apply();
    //close the dialog if only one entry is available
    if(!m_bNewMark && !m_xPrevBT->get_visible() && !m_xNextBT->get_visible())
        CloseHdl(rButton);
}

IMPL_LINK_NOARG(SwIndexMarkPane, CloseHdl, weld::Button&, void)
{
    if (m_bNewMark)
    {
        SfxViewFrame::Current()->GetDispatcher()->Execute(FN_INSERT_IDX_ENTRY_DLG,
                    SfxCallMode::ASYNCHRON|SfxCallMode::RECORD);
    }
    else
    {
        m_xDialog->response(RET_CLOSE);
    }
}

// select index type only when inserting
IMPL_LINK(SwIndexMarkPane, ModifyListBoxHdl, weld::ComboBox&, rBox, void)
{
    ModifyHdl(rBox);
}

IMPL_LINK(SwIndexMarkPane, ModifyEditHdl, weld::Entry&, rEdit, void)
{
    ModifyHdl(rEdit);
}

void SwIndexMarkPane::ModifyHdl(const weld::Widget& rBox)
{
    if (m_xTypeDCB.get() == &rBox)
    {
        // set index type
        auto nPos = m_xTypeDCB->find_text(m_xTypeDCB->get_active_text());
        bool bLevelEnable = false,
             bKeyEnable   = false,
             bSetKey2     = false,
             bKey2Enable  = false,
             bEntryHasText   = false,
             bKey1HasText    = false,
             bKey2HasText    = false;
        if(nPos == POS_INDEX)
        {
            if (!m_xEntryED->get_text().isEmpty())
                bEntryHasText = true;
            m_xPhoneticED0->set_text(GetDefaultPhoneticReading(m_xEntryED->get_text()));

            bKeyEnable = true;
            m_xKey1DCB->set_active_text(m_xKey1DCB->get_text(nKey1Pos));
            m_xPhoneticED1->set_text(GetDefaultPhoneticReading(m_xKey1DCB->get_active_text()));
            if (!m_xKey1DCB->get_active_text().isEmpty())
            {
                bKey1HasText = bSetKey2 = bKey2Enable = true;
                m_xKey2DCB->set_active_text(m_xKey2DCB->get_text(nKey2Pos));
                m_xPhoneticED2->set_text(GetDefaultPhoneticReading(m_xKey2DCB->get_active_text()));
                if(!m_xKey2DCB->get_active_text().isEmpty())
                    bKey2HasText = true;
            }
        }
        else
        {
            bLevelEnable = true;
            m_xLevelNF->set_max(MAXLEVEL);
            m_xLevelNF->set_value(m_xLevelNF->normalize(0));
            bSetKey2 = true;
        }
        m_xLevelFT->set_visible(bLevelEnable);
        m_xLevelNF->set_visible(bLevelEnable);
        m_xMainEntryCB->set_visible(nPos == POS_INDEX);

        m_xKey1FT->set_sensitive(bKeyEnable);
        m_xKey1DCB->set_sensitive(bKeyEnable);
        if ( bSetKey2 )
        {
            m_xKey2DCB->set_sensitive(bKey2Enable);
            m_xKey2FT->set_sensitive(bKey2Enable);
        }
        m_xPhoneticFT0->set_sensitive(bKeyEnable&&bEntryHasText&&m_bIsPhoneticReadingEnabled);
        m_xPhoneticED0->set_sensitive(bKeyEnable&&bEntryHasText&&m_bIsPhoneticReadingEnabled);
        m_xPhoneticFT1->set_sensitive(bKeyEnable&&bKey1HasText&&m_bIsPhoneticReadingEnabled);
        m_xPhoneticED1->set_sensitive(bKeyEnable&&bKey1HasText&&m_bIsPhoneticReadingEnabled);
        m_xPhoneticFT2->set_sensitive(bKeyEnable&&bKey2HasText&&m_bIsPhoneticReadingEnabled);
        m_xPhoneticED2->set_sensitive(bKeyEnable&&bKey2HasText&&m_bIsPhoneticReadingEnabled);
    }
    else //m_xEntryED  !!m_xEntryED is not a ListBox but a Edit
    {
        bool bHasText = !m_xEntryED->get_text().isEmpty();
        if(!bHasText)
        {
            m_xPhoneticED0->set_text(OUString());
            m_bPhoneticED0_ChangedByUser = false;
        }
        else if(!m_bPhoneticED0_ChangedByUser)
            m_xPhoneticED0->set_text(GetDefaultPhoneticReading(m_xEntryED->get_text()));

        m_xPhoneticFT0->set_sensitive(bHasText&&m_bIsPhoneticReadingEnabled);
        m_xPhoneticED0->set_sensitive(bHasText&&m_bIsPhoneticReadingEnabled);
    }
    m_xOKBT->set_sensitive(!m_pSh->HasReadonlySel() &&
        (!m_xEntryED->get_text().isEmpty() || m_pSh->GetCursorCnt(false)));
}

IMPL_LINK_NOARG(SwIndexMarkPane, NextHdl, weld::Button&, void)
{
    InsertUpdate();
    m_pTOXMgr->NextTOXMark();
    UpdateDialog();
}

IMPL_LINK_NOARG(SwIndexMarkPane, NextSameHdl, weld::Button&, void)
{
    InsertUpdate();
    m_pTOXMgr->NextTOXMark(true);
    UpdateDialog();
}

IMPL_LINK_NOARG(SwIndexMarkPane, PrevHdl, weld::Button&, void)
{
    InsertUpdate();
    m_pTOXMgr->PrevTOXMark();
    UpdateDialog();
}

IMPL_LINK_NOARG(SwIndexMarkPane, PrevSameHdl, weld::Button&, void)
{
    InsertUpdate();
    m_pTOXMgr->PrevTOXMark(true);
    UpdateDialog();
}

IMPL_LINK_NOARG(SwIndexMarkPane, DelHdl, weld::Button&, void)
{
    m_bDel = true;
    InsertUpdate();
    m_bDel = false;

    if(m_pTOXMgr->GetCurTOXMark())
        UpdateDialog();
    else
    {
        CloseHdl(*m_xCloseBT);
        SfxViewFrame::Current()->GetBindings().Invalidate(FN_EDIT_IDX_ENTRY_DLG);
    }
}

// renew dialog view
void SwIndexMarkPane::UpdateDialog()
{
    OSL_ENSURE(m_pSh && m_pTOXMgr, "no shell?");
    SwTOXMark* pMark = m_pTOXMgr->GetCurTOXMark();
    OSL_ENSURE(pMark, "no current marker");
    if(!pMark)
        return;

    SwViewShell::SetCareDialog(m_xDialog);

    m_aOrgStr = pMark->GetText(m_pSh->GetLayout());
    m_xEntryED->set_text(m_aOrgStr);

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
        bEntryHasText = !pMark->GetText(m_pSh->GetLayout()).isEmpty();
        m_xKey1DCB->set_entry_text( pMark->GetPrimaryKey() );
        m_xKey2DCB->set_entry_text( pMark->GetSecondaryKey() );
        m_xPhoneticED0->set_text( pMark->GetTextReading() );
        m_xPhoneticED1->set_text( pMark->GetPrimaryKeyReading() );
        m_xPhoneticED2->set_text( pMark->GetSecondaryKeyReading() );
        m_xMainEntryCB->set_active(pMark->IsMainEntry());
    }
    else if(TOX_CONTENT == eCurType || TOX_USER == eCurType)
    {
        m_xLevelNF->set_value(m_xLevelNF->normalize(pMark->GetLevel()));
    }
    m_xKey1FT->set_sensitive(bKeyEnable);
    m_xKey1DCB->set_sensitive(bKeyEnable);
    m_xLevelNF->set_max(MAXLEVEL);
    m_xLevelFT->set_visible(bLevelEnable);
    m_xLevelNF->set_visible(bLevelEnable);
    m_xMainEntryCB->set_visible(!bLevelEnable);
    m_xKey2FT->set_sensitive(bKey2Enable);
    m_xKey2DCB->set_sensitive(bKey2Enable);

    UpdateLanguageDependenciesForPhoneticReading();
    m_xPhoneticFT0->set_sensitive(bKeyEnable&&bEntryHasText&&m_bIsPhoneticReadingEnabled);
    m_xPhoneticED0->set_sensitive(bKeyEnable&&bEntryHasText&&m_bIsPhoneticReadingEnabled);
    m_xPhoneticFT1->set_sensitive(bKeyEnable&&bKey1HasText&&m_bIsPhoneticReadingEnabled);
    m_xPhoneticED1->set_sensitive(bKeyEnable&&bKey1HasText&&m_bIsPhoneticReadingEnabled);
    m_xPhoneticFT2->set_sensitive(bKeyEnable&&bKey2HasText&&m_bIsPhoneticReadingEnabled);
    m_xPhoneticED2->set_sensitive(bKeyEnable&&bKey2HasText&&m_bIsPhoneticReadingEnabled);

    // set index type
    m_xTypeDCB->set_active_text(pMark->GetTOXType()->GetTypeName());

    // set Next - Prev - Buttons
    m_pSh->SttCursorMove();
    if( m_xPrevBT->get_visible() )
    {
        const SwTOXMark* pMoveMark = &m_pSh->GotoTOXMark( *pMark, TOX_PRV );
        if( pMoveMark != pMark )
            m_pSh->GotoTOXMark( *pMoveMark, TOX_NXT );
        m_xPrevBT->set_sensitive( pMoveMark != pMark );
        pMoveMark = &m_pSh->GotoTOXMark( *pMark, TOX_NXT );
        if( pMoveMark != pMark )
            m_pSh->GotoTOXMark( *pMoveMark, TOX_PRV );
        m_xNextBT->set_sensitive( pMoveMark != pMark );
    }

    if (m_xPrevSameBT->get_visible())
    {
        const SwTOXMark* pMoveMark = &m_pSh->GotoTOXMark( *pMark, TOX_SAME_PRV );
        if( pMoveMark != pMark )
            m_pSh->GotoTOXMark( *pMoveMark, TOX_SAME_NXT );
        m_xPrevSameBT->set_sensitive( pMoveMark != pMark );
        pMoveMark = &m_pSh->GotoTOXMark( *pMark, TOX_SAME_NXT );
        if( pMoveMark != pMark )
            m_pSh->GotoTOXMark( *pMoveMark, TOX_SAME_PRV );
        m_xNextSameBT->set_sensitive( pMoveMark != pMark );
    }

    const bool bEnable = !m_pSh->HasReadonlySel();
    m_xOKBT->set_sensitive(bEnable);
    m_xDelBT->set_sensitive(bEnable);
    m_xEntryED->set_sensitive(bEnable);
    m_xLevelNF->set_sensitive(bEnable);
    m_xKey1DCB->set_sensitive(bEnable);
    m_xKey2DCB->set_sensitive(bEnable);

    m_pSh->SelectTextAttr( RES_TXTATR_TOXMARK, pMark->GetTextTOXMark() );
    // we need the point at the start of the attribute
    m_pSh->SwapPam();

    m_pSh->EndCursorMove();
}

// Remind whether the edit boxes for Phonetic reading are changed manually
IMPL_LINK(SwIndexMarkPane, PhoneticEDModifyHdl, weld::Entry&, rEdit, void)
{
    if (m_xPhoneticED0.get() == &rEdit)
    {
        m_bPhoneticED0_ChangedByUser = !rEdit.get_text().isEmpty();
    }
    else if (m_xPhoneticED1.get() == &rEdit)
    {
        m_bPhoneticED1_ChangedByUser = !rEdit.get_text().isEmpty();
    }
    else if (m_xPhoneticED2.get() == &rEdit)
    {
        m_bPhoneticED2_ChangedByUser = !rEdit.get_text().isEmpty();
    }
}

// Enable Disable of the 2nd key
IMPL_LINK( SwIndexMarkPane, KeyDCBModifyHdl, weld::ComboBox&, rBox, void )
{
    if (m_xKey1DCB.get() == &rBox)
    {
        bool bEnable = !rBox.get_active_text().isEmpty();
        if(!bEnable)
        {
            m_xKey2DCB->set_entry_text(OUString());
            m_xPhoneticED1->set_text(OUString());
            m_xPhoneticED2->set_text(OUString());
            m_bPhoneticED1_ChangedByUser = false;
            m_bPhoneticED2_ChangedByUser = false;
        }
        else
        {
            if (rBox.get_popup_shown())
            {
                //reset bPhoneticED1_ChangedByUser if a completely new string is selected
                m_bPhoneticED1_ChangedByUser = false;
            }
            if (!m_bPhoneticED1_ChangedByUser)
                m_xPhoneticED1->set_text(GetDefaultPhoneticReading(rBox.get_active_text()));
        }
        m_xKey2DCB->set_sensitive(bEnable);
        m_xKey2FT->set_sensitive(bEnable);
    }
    else if (m_xKey2DCB.get() == &rBox)
    {
        if (rBox.get_active_text().isEmpty())
        {
            m_xPhoneticED2->set_text(OUString());
            m_bPhoneticED2_ChangedByUser = false;
        }
        else
        {
            if (rBox.get_popup_shown())
            {
                //reset bPhoneticED1_ChangedByUser if a completely new string is selected
                m_bPhoneticED2_ChangedByUser = false;
            }
            if(!m_bPhoneticED2_ChangedByUser)
                m_xPhoneticED2->set_text(GetDefaultPhoneticReading(rBox.get_active_text()));
        }
    }

    bool bKey1HasText = !m_xKey1DCB->get_active_text().isEmpty();
    bool bKey2HasText = !m_xKey2DCB->get_active_text().isEmpty();

    m_xPhoneticFT1->set_sensitive(bKey1HasText && m_bIsPhoneticReadingEnabled);
    m_xPhoneticED1->set_sensitive(bKey1HasText && m_bIsPhoneticReadingEnabled);
    m_xPhoneticFT2->set_sensitive(bKey2HasText && m_bIsPhoneticReadingEnabled);
    m_xPhoneticED2->set_sensitive(bKey2HasText && m_bIsPhoneticReadingEnabled);
}

SwIndexMarkPane::~SwIndexMarkPane()
{
}

void    SwIndexMarkPane::ReInitDlg(SwWrtShell& rWrtShell, SwTOXMark const * pCurTOXMark)
{
    m_pSh = &rWrtShell;
    m_pTOXMgr.reset( new SwTOXMgr(m_pSh) );
    if(pCurTOXMark)
    {
        for(sal_uInt16 i = 0; i < m_pTOXMgr->GetTOXMarkCount(); i++)
            if(m_pTOXMgr->GetTOXMark(i) == pCurTOXMark)
            {
                m_pTOXMgr->SetCurTOXMark(i);
                break;
            }
    }
    InitControls();
}

SwIndexMarkFloatDlg::SwIndexMarkFloatDlg(SfxBindings* _pBindings,
    SfxChildWindow* pChild, weld::Window *pParent,
    SfxChildWinInfo const * pInfo, bool bNew)
    : SfxModelessDialogController(_pBindings, pChild, pParent,
        "modules/swriter/ui/indexentry.ui", "IndexEntryDialog")
    , m_aContent(m_xDialog, *m_xBuilder, bNew, *::GetActiveWrtShell())
{
    m_aContent.ReInitDlg(*::GetActiveWrtShell());
    Initialize(pInfo);
}

void SwIndexMarkFloatDlg::Activate()
{
    SfxModelessDialogController::Activate();
    m_aContent.Activate();
}

void SwIndexMarkFloatDlg::ReInitDlg(SwWrtShell& rWrtShell)
{
    m_aContent.ReInitDlg( rWrtShell );
}

SwIndexMarkModalDlg::SwIndexMarkModalDlg(weld::Window *pParent, SwWrtShell& rSh, SwTOXMark const * pCurTOXMark)
    : SfxDialogController(pParent, "modules/swriter/ui/indexentry.ui",
                          "IndexEntryDialog")
    , m_aContent(m_xDialog, *m_xBuilder, false, rSh)
{
    m_aContent.ReInitDlg(rSh, pCurTOXMark);
}

SwIndexMarkModalDlg::~SwIndexMarkModalDlg()
{
    SwViewShell::SetCareDialog(nullptr);
}

short SwIndexMarkModalDlg::run()
{
    short nRet = SfxDialogController::run();
    if (RET_OK == nRet)
        m_aContent.Apply();
    return nRet;
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
    std::unique_ptr<weld::ComboBox> m_xTypeListBox;
    std::unique_ptr<weld::ComboBox> m_xIdentifierBox;

    DECL_LINK(IdentifierHdl, weld::ComboBox&, void);
    DECL_LINK(ShortNameHdl, weld::Entry&, void);
    DECL_LINK(EnableHdl, weld::ComboBox&, void);

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

bool SwAuthorMarkPane::bIsFromComponent = true;

SwAuthorMarkPane::SwAuthorMarkPane(weld::DialogController &rDialog, weld::Builder& rBuilder, bool bNewDlg)
    : m_rDialog(rDialog)
    , bNewEntry(bNewDlg)
    , bBibAccessInitialized(false)
    , pSh(nullptr)
    , m_xFromComponentRB(rBuilder.weld_radio_button("frombibliography"))
    , m_xFromDocContentRB(rBuilder.weld_radio_button("fromdocument"))
    , m_xAuthorFI(rBuilder.weld_label("author"))
    , m_xTitleFI(rBuilder.weld_label("title"))
    , m_xEntryED(rBuilder.weld_entry("entryed"))
    , m_xEntryLB(rBuilder.weld_combo_box("entrylb"))
    , m_xActionBT(rBuilder.weld_button(bNewEntry ? OString("insert") : OString("modify")))
    , m_xCloseBT(rBuilder.weld_button("close"))
    , m_xCreateEntryPB(rBuilder.weld_button("new"))
    , m_xEditEntryPB(rBuilder.weld_button("edit"))
{
    m_xActionBT->show();
    m_xFromComponentRB->set_visible(bNewEntry);
    m_xFromDocContentRB->set_visible(bNewEntry);
    m_xFromComponentRB->set_active(bIsFromComponent);
    m_xFromDocContentRB->set_active(!bIsFromComponent);

    m_xActionBT->connect_clicked(LINK(this,SwAuthorMarkPane, InsertHdl));
    m_xCloseBT->connect_clicked(LINK(this,SwAuthorMarkPane, CloseHdl));
    m_xCreateEntryPB->connect_clicked(LINK(this,SwAuthorMarkPane, CreateEntryHdl));
    m_xEditEntryPB->connect_clicked(LINK(this,SwAuthorMarkPane, CreateEntryHdl));
    m_xFromComponentRB->connect_toggled(LINK(this,SwAuthorMarkPane, ChangeSourceHdl));
    m_xFromDocContentRB->connect_toggled(LINK(this,SwAuthorMarkPane, ChangeSourceHdl));
    m_xEntryED->connect_changed(LINK(this,SwAuthorMarkPane, EditModifyHdl));

    m_rDialog.set_title(SwResId(
                    bNewEntry ? STR_AUTHMRK_INSERT : STR_AUTHMRK_EDIT));

    m_xEntryED->set_visible(!bNewEntry);
    m_xEntryLB->set_visible(bNewEntry);
    if (bNewEntry)
    {
        m_xEntryLB->connect_changed(LINK(this, SwAuthorMarkPane, CompEntryHdl));
    }
}

void SwAuthorMarkPane::ReInitDlg(SwWrtShell& rWrtShell)
{
    pSh = &rWrtShell;
    InitControls();
}

IMPL_LINK_NOARG(SwAuthorMarkPane, CloseHdl, weld::Button&, void)
{
    if(bNewEntry)
    {
        SfxViewFrame::Current()->GetDispatcher()->Execute(FN_INSERT_AUTH_ENTRY_DLG,
                    SfxCallMode::ASYNCHRON|SfxCallMode::RECORD);
    }
    else
    {
        m_rDialog.response(RET_CANCEL);
    }
}

IMPL_LINK( SwAuthorMarkPane, CompEntryHdl, weld::ComboBox&, rBox, void)
{
    const OUString sEntry(rBox.get_active_text());
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
    if (rBox.get_active_text().isEmpty())
    {
        for(OUString & s : m_sFields)
            s.clear();
    }
    m_xAuthorFI->set_label(m_sFields[AUTH_FIELD_AUTHOR]);
    m_xTitleFI->set_label(m_sFields[AUTH_FIELD_TITLE]);
}

IMPL_LINK_NOARG(SwAuthorMarkPane, InsertHdl, weld::Button&, void)
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
                std::unique_ptr<weld::MessageDialog> xQuery(Application::CreateMessageDialog(m_rDialog.getDialog(),
                                                            VclMessageType::Question, VclButtonsType::YesNo,
                                                            SwResId(STR_QUERY_CHANGE_AUTH_ENTRY)));
                if (RET_YES != xQuery->run())
                    return;
            }
        }

        SwFieldMgr aMgr(pSh);
        OUStringBuffer sFields;
        for(OUString & s : m_sFields)
        {
            sFields.append(s).append(TOX_STYLE_DELIMITER);
        }
        if(bNewEntry)
        {
            if(bDifferent)
            {
                rtl::Reference<SwAuthEntry> xNewData(new SwAuthEntry);
                for(int i = 0; i < AUTH_FIELD_END; i++)
                    xNewData->SetAuthorField(static_cast<ToxAuthorityField>(i), m_sFields[i]);
                pSh->ChangeAuthorityData(xNewData.get());
            }
            SwInsertField_Data aData(TYP_AUTHORITY, 0, sFields.makeStringAndClear(), OUString(), 0 );
            aMgr.InsertField( aData );
        }
        else if(aMgr.GetCurField())
        {
            aMgr.UpdateCurField(0, sFields.makeStringAndClear(), OUString());
        }
    }
    if(!bNewEntry)
        CloseHdl(*m_xCloseBT);
}

IMPL_LINK(SwAuthorMarkPane, CreateEntryHdl, weld::Button&, rButton, void)
{
    bool bCreate = &rButton == m_xCreateEntryPB.get();
    OUString sOldId = m_sCreatedEntry[0];
    for(int i = 0; i < AUTH_FIELD_END; i++)
        m_sCreatedEntry[i] = bCreate ? OUString() : m_sFields[i];
    SwCreateAuthEntryDlg_Impl aDlg(m_rDialog.getDialog(),
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
            m_xEntryLB->remove_text(sOldId);
        }
        for(int i = 0; i < AUTH_FIELD_END; i++)
        {
            m_sFields[i] = aDlg.GetEntryText(static_cast<ToxAuthorityField>(i));
            m_sCreatedEntry[i] = m_sFields[i];
        }
        if(bNewEntry && !m_xFromDocContentRB->get_active())
        {
            m_xFromDocContentRB->set_active(true);
            ChangeSourceHdl(*m_xFromDocContentRB);
        }
        if(bCreate)
        {
            OSL_ENSURE(m_xEntryLB->find_text(m_sFields[AUTH_FIELD_IDENTIFIER]) == -1,
                        "entry exists!");
            m_xEntryLB->append_text(m_sFields[AUTH_FIELD_IDENTIFIER]);
            m_xEntryLB->set_active_text(m_sFields[AUTH_FIELD_IDENTIFIER]);
        }
        m_xEntryED->set_text(m_sFields[AUTH_FIELD_IDENTIFIER]);
        m_xAuthorFI->set_label(m_sFields[AUTH_FIELD_AUTHOR]);
        m_xTitleFI->set_label(m_sFields[AUTH_FIELD_TITLE]);
        m_xActionBT->set_sensitive(true);
    }
}

IMPL_LINK_NOARG(SwAuthorMarkPane, ChangeSourceHdl, weld::ToggleButton&, void)
{
    bool bFromComp = m_xFromComponentRB->get_active();
    bIsFromComponent = bFromComp;
    m_xCreateEntryPB->set_sensitive(!bIsFromComponent);
    m_xEntryLB->clear();
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
                m_xEntryLB->append_text(pNames[i]);
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
                m_xEntryLB->append_text(i);
        }
        if(!m_sCreatedEntry[AUTH_FIELD_IDENTIFIER].isEmpty())
            m_xEntryLB->append_text(m_sCreatedEntry[AUTH_FIELD_IDENTIFIER]);
    }
    m_xEntryLB->set_active(0);
    CompEntryHdl(*m_xEntryLB);
}

IMPL_LINK(SwAuthorMarkPane, EditModifyHdl, weld::Entry&, rEdit, void)
{
    Link<weld::Entry&,bool> aAllowed = LINK(this, SwAuthorMarkPane, IsEditAllowedHdl);
    bool bResult = aAllowed.Call(rEdit);
    m_xActionBT->set_sensitive(bResult);
    if(bResult)
    {
        OUString sEntry(rEdit.get_text());
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
        if (m_xEntryLB->find_text(sEntry) != -1)
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

IMPL_LINK(SwAuthorMarkPane, IsEditAllowedHdl, weld::Entry&, rEdit, bool)
{
    OUString sEntry = rEdit.get_text();
    bool bAllowed = false;
    if(!sEntry.isEmpty())
    {
        if (m_xEntryLB->find_text(sEntry) != -1)
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
        ChangeSourceHdl(m_xFromComponentRB->get_active() ? *m_xFromComponentRB : *m_xFromDocContentRB);
        m_xCreateEntryPB->set_sensitive(!m_xFromComponentRB->get_active());
        if(!m_xFromComponentRB->get_active() && !m_sCreatedEntry[0].isEmpty())
            for(int i = 0; i < AUTH_FIELD_END; i++)
                m_sFields[i] = m_sCreatedEntry[i];
    }
    if(bNewEntry || !pField || pField->GetTyp()->Which() != SwFieldIds::TableOfAuthorities)
        return;

    const SwAuthEntry* pEntry = static_cast<SwAuthorityField*>(pField)->GetAuthEntry();

    OSL_ENSURE(pEntry, "No authority entry found");
    if(!pEntry)
        return;
    for(int i = 0; i < AUTH_FIELD_END; i++)
        m_sFields[i] = pEntry->GetAuthorField(static_cast<ToxAuthorityField>(i));

    m_xEntryED->set_text(pEntry->GetAuthorField(AUTH_FIELD_IDENTIFIER));
    m_xAuthorFI->set_label(pEntry->GetAuthorField(AUTH_FIELD_AUTHOR));
    m_xTitleFI->set_label(pEntry->GetAuthorField(AUTH_FIELD_TITLE));
}

void SwAuthorMarkPane::Activate()
{
    m_xActionBT->set_sensitive(!pSh->HasReadonlySel());
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
            m_xTypeListBox = m_aBuilders.back()->weld_combo_box("listbox");
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
            m_xIdentifierBox = m_aBuilders.back()->weld_combo_box("combobox");
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
            pEdits[nIndex] = m_aBuilders.back()->weld_entry("entry");
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

IMPL_LINK(SwCreateAuthEntryDlg_Impl, IdentifierHdl, weld::ComboBox&, rBox, void)
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

IMPL_LINK(SwCreateAuthEntryDlg_Impl, EnableHdl, weld::ComboBox&, rBox, void)
{
    m_xOKBT->set_sensitive(m_bNameAllowed && rBox.get_active() != -1);
};

SwAuthMarkFloatDlg::SwAuthMarkFloatDlg(SfxBindings* _pBindings,
                                       SfxChildWindow* pChild,
                                       weld::Window *pParent,
                                       SfxChildWinInfo const * pInfo,
                                       bool bNew)
    : SfxModelessDialogController(_pBindings, pChild, pParent,
        "modules/swriter/ui/bibliographyentry.ui", "BibliographyEntryDialog")
    , m_aContent(*this, *m_xBuilder, bNew)
{
    Initialize(pInfo);
    SwWrtShell* pWrtShell = ::GetActiveWrtShell();
    if (pWrtShell)
        m_aContent.ReInitDlg(*pWrtShell);
}

void SwAuthMarkFloatDlg::Activate()
{
    SfxModelessDialogController::Activate();
    m_aContent.Activate();
}

void SwAuthMarkFloatDlg::ReInitDlg(SwWrtShell& rWrtShell)
{
    m_aContent.ReInitDlg( rWrtShell );
}

SwAuthMarkModalDlg::SwAuthMarkModalDlg(weld::Window *pParent, SwWrtShell& rSh)
    : SfxDialogController(pParent, "modules/swriter/ui/bibliographyentry.ui",
                          "BibliographyEntryDialog")
    , m_aContent(*this, *m_xBuilder, false)
{
    m_aContent.ReInitDlg(rSh);
}

short SwAuthMarkModalDlg::run()
{
    short ret = SfxDialogController::run();
    if (ret == RET_OK)
        Apply();
    return ret;
}

void SwAuthMarkModalDlg::Apply()
{
    m_aContent.InsertHdl(*m_aContent.m_xActionBT);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
