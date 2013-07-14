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

#include "swuiidxmrk.hxx"
#include <hintids.hxx>
#include <helpid.h>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/Bibliography.hpp>
#include <com/sun/star/i18n/TransliterationModules.hpp>
#include <com/sun/star/i18n/IndexEntrySupplier.hpp>
#include <com/sun/star/util/SearchOptions.hpp>
#include <com/sun/star/util/SearchFlags.hpp>
#include <svl/stritem.hxx>
#include <vcl/layout.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/eitem.hxx>
#include <unotools/textsearch.hxx>
#include <editeng/scripttypeitem.hxx>
#include <svl/itemset.hxx>
#include <editeng/langitem.hxx>
#include "editeng/unolingu.hxx"
#include <swtypes.hxx>
#include <idxmrk.hxx>
#include <txttxmrk.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <multmrk.hxx>
#include <swundo.hxx>                   // for Undo-Ids
#include <cmdid.h>
#include <app.hrc>
#include <index.hrc>
#include <idxmrk.hrc>
#include <swmodule.hxx>
#include <fldmgr.hxx>
#include <fldbas.hxx>
#include <utlui.hrc>
#include <swcont.hxx>
#include <svl/cjkoptions.hxx>
#include <ndtxt.hxx>
#include <breakit.hxx>
#include <SwRewriter.hxx>

#include <unomid.h>


#define POS_CONTENT 0
#define POS_INDEX   1

static sal_uInt16 nTypePos = 1; // TOX_INDEX as standard
static sal_uInt16 nKey1Pos = USHRT_MAX;

static sal_uInt16 nKey2Pos = USHRT_MAX;

using namespace com::sun::star;
using namespace com::sun::star::i18n;
using namespace com::sun::star::lang;
using namespace com::sun::star::util;
using namespace ::comphelper;
using namespace ::com::sun::star;

/*--------------------------------------------------------------------
     Description:   dialog to insert a directory selection
 --------------------------------------------------------------------*/
SwIndexMarkPane::SwIndexMarkPane(Dialog &rDialog, sal_Bool bNewDlg,
    SwWrtShell& rWrtShell)
    : m_rDialog(rDialog)
    , bNewMark(bNewDlg)
    , bSelected(sal_False)
    , bPhoneticED0_ChangedByUser(sal_False)
    , bPhoneticED1_ChangedByUser(sal_False)
    , bPhoneticED2_ChangedByUser(sal_False)
    , nLangForPhoneticReading(LANGUAGE_CHINESE_SIMPLIFIED)
    , bIsPhoneticReadingEnabled(sal_False)
    , xExtendedIndexEntrySupplier(NULL)
    , pTOXMgr(0)
    , pSh(&rWrtShell)
{
    rDialog.get(m_pFrame, "frame");
    rDialog.get(m_pTypeFT, "typeft");
    rDialog.get(m_pTypeDCB, "typecb");
    rDialog.get(m_pNewBT, "new");
    m_pNewBT->SetAccessibleRelationMemberOf(m_pFrame->get_label_widget());
    rDialog.get(m_pEntryED, "entryed");
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

    rDialog.SetText( SW_RESSTR( bNewMark ? STR_IDXMRK_INSERT : STR_IDXMRK_EDIT));

    m_pDelBT->SetClickHdl(LINK(this,SwIndexMarkPane,        DelHdl));
    m_pPrevBT->SetClickHdl(LINK(this,SwIndexMarkPane,       PrevHdl));
    m_pPrevSameBT->SetClickHdl(LINK(this,SwIndexMarkPane,   PrevSameHdl));
    m_pNextBT->SetClickHdl(LINK(this,SwIndexMarkPane,       NextHdl));
    m_pNextSameBT->SetClickHdl(LINK(this,SwIndexMarkPane,   NextSameHdl));
    m_pTypeDCB->SetSelectHdl(LINK(this,SwIndexMarkPane,     ModifyHdl));
    m_pKey1DCB->SetModifyHdl(LINK(this,SwIndexMarkPane,      KeyDCBModifyHdl));
    m_pKey2DCB->SetModifyHdl(LINK(this,SwIndexMarkPane,     KeyDCBModifyHdl));
    m_pCloseBT->SetClickHdl(LINK(this,SwIndexMarkPane,     CloseHdl));
    m_pEntryED->SetModifyHdl(LINK(this,SwIndexMarkPane,     ModifyHdl));
    m_pNewBT->SetClickHdl(LINK(this, SwIndexMarkPane,       NewUserIdxHdl));
    m_pApplyToAllCB->SetClickHdl(LINK(this, SwIndexMarkPane, SearchTypeHdl));
    m_pPhoneticED0->SetModifyHdl(LINK(this,SwIndexMarkPane, PhoneticEDModifyHdl));
    m_pPhoneticED1->SetModifyHdl(LINK(this,SwIndexMarkPane, PhoneticEDModifyHdl));
    m_pPhoneticED2->SetModifyHdl(LINK(this,SwIndexMarkPane, PhoneticEDModifyHdl));

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

/*--------------------------------------------------------------------
     Description: Newly initialise controls with the new selection
 --------------------------------------------------------------------*/
void SwIndexMarkPane::InitControls()
{
    OSL_ENSURE(pSh && pTOXMgr, "no shell?");
    // contents index
    const SwTOXType* pType = pTOXMgr->GetTOXType(TOX_CONTENT, 0);
    OSL_ENSURE(pType, "Kein Verzeichnistyp !!");
    String sTmpTypeSelection;
    if(m_pTypeDCB->GetSelectEntryCount())
        sTmpTypeSelection = m_pTypeDCB->GetSelectEntry();
    m_pTypeDCB->Clear();
    m_pTypeDCB->InsertEntry(pType->GetTypeName());

    // keyword index
    pType = pTOXMgr->GetTOXType(TOX_INDEX, 0);
    OSL_ENSURE(pType, "Kein Verzeichnistyp !!");
    m_pTypeDCB->InsertEntry(pType->GetTypeName());

    // user index
    sal_uInt16 nCount = pSh->GetTOXTypeCount(TOX_USER);
    sal_uInt16 i;
    for( i = 0; i < nCount; ++i )
        m_pTypeDCB->InsertEntry( pSh->GetTOXType(TOX_USER, i)->GetTypeName() );

    // read keywords primary
    std::vector<String> aArr;
    nCount = pSh->GetTOIKeys( TOI_PRIMARY, aArr );
    std::sort(aArr.begin(), aArr.end());
    for (std::vector<String>::iterator it = aArr.begin(); it != aArr.end(); ++it)
        m_pKey1DCB->InsertEntry( *it );

    // read keywords secondary
    nCount = pSh->GetTOIKeys( TOI_SECONDARY, aArr );
    std::sort(aArr.begin(), aArr.end());
    for (std::vector<String>::iterator it = aArr.begin(); it != aArr.end(); ++it)
        m_pKey2DCB->InsertEntry( *it );

    UpdateLanguageDependenciesForPhoneticReading();

    // current entry
    const SwTOXMark* pMark = pTOXMgr->GetCurTOXMark();
    if( pMark && !bNewMark)
    {
        // Controls-Handling

        // only if there are more than one
        // if equal it lands at the same entry
        pSh->SttCrsrMove();

        const SwTOXMark* pMoveMark;
        bool bShow = false;

        pMoveMark = &pSh->GotoTOXMark( *pMark, TOX_PRV );
        if( pMoveMark != pMark )
            pSh->GotoTOXMark( *pMoveMark, TOX_NXT ), bShow = true;
        m_pPrevBT->Enable( pMoveMark != pMark );
        pMoveMark = &pSh->GotoTOXMark( *pMark, TOX_NXT );
        if( pMoveMark != pMark )
            pSh->GotoTOXMark( *pMoveMark, TOX_PRV ), bShow = true;
        m_pNextBT->Enable( pMoveMark != pMark );
        if( bShow )
        {
            m_pPrevBT->Show();
            m_pNextBT->Show();
            bShow = false;
        }

        pMoveMark = &pSh->GotoTOXMark( *pMark, TOX_SAME_PRV );
        if( pMoveMark != pMark )
            pSh->GotoTOXMark( *pMoveMark, TOX_SAME_NXT ), bShow = true;
        m_pPrevSameBT->Enable( pMoveMark != pMark );
        pMoveMark = &pSh->GotoTOXMark( *pMark, TOX_SAME_NXT );
        if( pMoveMark != pMark )
            pSh->GotoTOXMark( *pMoveMark, TOX_SAME_PRV ), bShow = true;
        m_pNextSameBT->Enable( pMoveMark != pMark );
        if( bShow )
        {
            m_pNextSameBT->Show();
            m_pPrevSameBT->Show();
        }
        pSh->EndCrsrMove();

        m_pTypeFT->Show();

        m_pTypeDCB->Enable(sal_False);
        m_pTypeFT->Enable(sal_False);

        UpdateDialog();
    }
    else
    {   // display current selection (first element) ????
        sal_uInt16 nCnt = pSh->GetCrsrCnt();
        if (nCnt < 2)
        {
            bSelected = !pSh->HasSelection();
            aOrgStr = pSh->GetView().GetSelectionTextParam(sal_True, sal_False);
            m_pEntryED->SetText(aOrgStr);

            //to include all equal entries may only be allowed in the body and even there
            //only when a simple selection exists
            const sal_uInt16 nFrmType = pSh->GetFrmType(0,sal_True);
            m_pApplyToAllCB->Show();
            m_pSearchCaseSensitiveCB->Show();
            m_pSearchCaseWordOnlyCB->Show();
            m_pApplyToAllCB->Enable(0 != aOrgStr.Len() &&
                0 == (nFrmType & ( FRMTYPE_HEADER | FRMTYPE_FOOTER | FRMTYPE_FLY_ANY )));
            SearchTypeHdl(m_pApplyToAllCB);
        }

        // index type is default
        if( sTmpTypeSelection.Len() &&
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
        bIsPhoneticReadingEnabled = sal_False;
        return;
    }
    bIsPhoneticReadingEnabled = sal_True;

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
        SwTxtTOXMark* pTxtTOXMark = pMark->GetTxtTOXMark();
        OSL_ENSURE(pTxtTOXMark, "need current SwTxtTOXMark");
        if(!pTxtTOXMark)
            return;
        const SwTxtNode* pTxtNode = pTxtTOXMark->GetpTxtNd();
        OSL_ENSURE(pTxtNode, "need current SwTxtNode");
        if(!pTxtNode)
            return;
        xub_StrLen nTextIndex = *pTxtTOXMark->GetStart();
        nLangForPhoneticReading = pTxtNode->GetLang( nTextIndex );
    }
    else //if dialog is opened to create a new mark
    {
        sal_uInt16 nScriptType = pSh->GetScriptType();
        sal_uInt16 nWhich;
        switch(nScriptType)
        {
            case SCRIPTTYPE_ASIAN: nWhich = RES_CHRATR_CJK_LANGUAGE; break;
            case SCRIPTTYPE_COMPLEX:nWhich = RES_CHRATR_CTL_LANGUAGE; break;
            default:nWhich = RES_CHRATR_LANGUAGE; break;
        }
        SfxItemSet aLangSet(pSh->GetAttrPool(), nWhich, nWhich);
        pSh->GetCurAttr(aLangSet);
        nLangForPhoneticReading = ((const SvxLanguageItem&)aLangSet.Get(nWhich)).GetLanguage();
    }

}

String  SwIndexMarkPane::GetDefaultPhoneticReading( const String& rText )
{
    if( !bIsPhoneticReadingEnabled )
        return aEmptyStr;

    return xExtendedIndexEntrySupplier->getPhoneticCandidate(rText, LanguageTag::convertToLocale( nLangForPhoneticReading ));
}

/* --------------------------------------------------
    Change the content of m_pEntryED if text is selected
 --------------------------------------------------*/
void    SwIndexMarkPane::Activate()
{
    // display current selection (first element) ????
    if(bNewMark)
    {
        sal_uInt16 nCnt = pSh->GetCrsrCnt();
        if (nCnt < 2)
        {
            bSelected = !pSh->HasSelection();
            aOrgStr = pSh->GetView().GetSelectionTextParam(sal_True, sal_False);
            m_pEntryED->SetText(aOrgStr);

            //to include all equal entries may only be allowed in the body and even there
            //only when a simple selection exists
            const sal_uInt16 nFrmType = pSh->GetFrmType(0,sal_True);
            m_pApplyToAllCB->Show();
            m_pSearchCaseSensitiveCB->Show();
            m_pSearchCaseWordOnlyCB->Show();
            m_pApplyToAllCB->Enable(0 != aOrgStr.Len() &&
                0 == (nFrmType & ( FRMTYPE_HEADER | FRMTYPE_FOOTER | FRMTYPE_FLY_ANY )));
            SearchTypeHdl(m_pApplyToAllCB);
        }
        ModifyHdl(m_pTypeDCB);
    }
}

/*--------------------------------------------------------------------
     Description:   evaluate Ok-Button
 --------------------------------------------------------------------*/
void SwIndexMarkPane::Apply()
{
    InsertUpdate();
    if(bSelected)
        pSh->ResetSelect(0, sal_False);
}

/*--------------------------------------------------------------------
    Description: apply changes
 --------------------------------------------------------------------*/
void SwIndexMarkPane::InsertUpdate()
{
    pSh->StartUndo(bDel ? UNDO_INDEX_ENTRY_DELETE : UNDO_INDEX_ENTRY_INSERT);
    pSh->StartAllAction();

    SwRewriter aRewriter;

    if( bNewMark )
    {
        InsertMark();

        if ( pTOXMgr->GetCurTOXMark())
            aRewriter.AddRule(UndoArg1, pTOXMgr->GetCurTOXMark()->GetText());
    }
    else if( !pSh->HasReadonlySel() )
    {
        if ( pTOXMgr->GetCurTOXMark())
            aRewriter.AddRule(UndoArg1,
                              pTOXMgr->GetCurTOXMark()->GetText());

        if( bDel )
            pTOXMgr->DeleteTOXMark();
        else if( pTOXMgr->GetCurTOXMark() )
            UpdateMark();
    }

    pSh->EndAllAction();
    pSh->EndUndo(bDel ? UNDO_INDEX_ENTRY_DELETE : UNDO_INDEX_ENTRY_INSERT);

    if((nTypePos = m_pTypeDCB->GetEntryPos(m_pTypeDCB->GetSelectEntry())) == LISTBOX_ENTRY_NOTFOUND)
        nTypePos = 0;

    nKey1Pos = m_pKey1DCB->GetEntryPos(m_pKey1DCB->GetText());
    nKey2Pos = m_pKey2DCB->GetEntryPos(m_pKey2DCB->GetText());
}

/*--------------------------------------------------------------------
     Description:   insert mark
 --------------------------------------------------------------------*/
static void lcl_SelectSameStrings(SwWrtShell& rSh, sal_Bool bWordOnly, sal_Bool bCaseSensitive)
{
    rSh.Push();

    SearchOptions aSearchOpt(
                        SearchAlgorithms_ABSOLUTE,
                        ( bWordOnly ? SearchFlags::NORM_WORD_ONLY : 0 ),
                        rSh.GetSelTxt(), OUString(),
                        GetAppLanguageTag().getLocale(),
                        0, 0, 0,
                        (bCaseSensitive
                            ? 0
                            : TransliterationModules_IGNORE_CASE) );

    rSh.ClearMark();
    sal_Bool bCancel;

    //todo/mba: assuming that notes should not be searched
    sal_Bool bSearchInNotes = sal_False;
    rSh.Find( aSearchOpt,  bSearchInNotes, DOCPOS_START, DOCPOS_END, bCancel,
                        (FindRanges)(FND_IN_SELALL|FND_IN_BODYONLY), sal_False );
}


void SwIndexMarkPane::InsertMark()
{
    sal_uInt16 nPos = m_pTypeDCB->GetEntryPos(m_pTypeDCB->GetSelectEntry());
    TOXTypes eType = nPos == POS_CONTENT ? TOX_CONTENT :
                        nPos == POS_INDEX ? TOX_INDEX : TOX_USER;

    SwTOXMarkDescription aDesc(eType);

    sal_uInt16 nLevel = (sal_uInt16)m_pLevelNF->Denormalize(m_pLevelNF->GetValue());
    switch(nPos)
    {
        case POS_CONTENT : break;
        case POS_INDEX:     // keyword index mark
        {
            UpdateKeyBoxes();
            String  aPrim(m_pKey1DCB->GetText());
            String  aSec(m_pKey2DCB->GetText());
            aDesc.SetPrimKey(aPrim);
            aDesc.SetSecKey(aSec);
            aDesc.SetMainEntry(m_pMainEntryCB->IsChecked());
            aDesc.SetPhoneticReadingOfAltStr(m_pPhoneticED0->GetText());
            aDesc.SetPhoneticReadingOfPrimKey(m_pPhoneticED1->GetText());
            aDesc.SetPhoneticReadingOfSecKey(m_pPhoneticED2->GetText());
        }
        break;
        default:            // Userdefined index mark
        {
            String aName(m_pTypeDCB->GetSelectEntry());
            aDesc.SetTOUName(aName);
        }
    }
    if (OUString(aOrgStr) != m_pEntryED->GetText())
        aDesc.SetAltStr(m_pEntryED->GetText());
    sal_Bool bApplyAll = m_pApplyToAllCB->IsChecked();
    sal_Bool bWordOnly = m_pSearchCaseWordOnlyCB->IsChecked();
    sal_Bool bCaseSensitive = m_pSearchCaseSensitiveCB->IsChecked();

    pSh->StartAllAction();
    // all equal strings have to be selected here so that the
    // entry is apllied to all equal strings
    if(bApplyAll)
    {
        lcl_SelectSameStrings(*pSh, bWordOnly, bCaseSensitive);
    }
    aDesc.SetLevel(nLevel);
    SwTOXMgr aMgr(pSh);
    aMgr.InsertTOXMark(aDesc);
    if(bApplyAll)
        pSh->Pop(sal_False);

    pSh->EndAllAction();
}

/*--------------------------------------------------------------------
     Description:   update mark
 --------------------------------------------------------------------*/
void SwIndexMarkPane::UpdateMark()
{
    String  aAltText(m_pEntryED->GetText());
    String* pAltText = OUString(aOrgStr) != m_pEntryED->GetText() ? &aAltText : 0;
    //empty alternative texts are not allowed
    if(pAltText && !pAltText->Len())
        return;

    UpdateKeyBoxes();

    sal_uInt16 nPos = m_pTypeDCB->GetEntryPos(m_pTypeDCB->GetSelectEntry());
    TOXTypes eType = TOX_USER;
    if(POS_CONTENT == nPos)
        eType = TOX_CONTENT;
    else if(POS_INDEX == nPos)
        eType = TOX_INDEX;

    SwTOXMarkDescription aDesc(eType);
    aDesc.SetLevel( static_cast< int >(m_pLevelNF->GetValue()) );
    if(pAltText)
        aDesc.SetAltStr(*pAltText);

    String  aPrim(m_pKey1DCB->GetText());
    if(aPrim.Len())
        aDesc.SetPrimKey(aPrim);
    String  aSec(m_pKey2DCB->GetText());
    if(aSec.Len())
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

/*--------------------------------------------------------------------
    Description: insert new keys
 --------------------------------------------------------------------*/
void SwIndexMarkPane::UpdateKeyBoxes()
{
    String aKey(m_pKey1DCB->GetText());
    sal_uInt16 nPos = m_pKey1DCB->GetEntryPos(aKey);

    if(nPos == LISTBOX_ENTRY_NOTFOUND && aKey.Len() > 0)
    {   // create new key
        m_pKey1DCB->InsertEntry(aKey);
    }

    aKey = m_pKey2DCB->GetText();
    nPos = m_pKey2DCB->GetEntryPos(aKey);

    if(nPos == LISTBOX_ENTRY_NOTFOUND && aKey.Len() > 0)
    {   // create new key
        m_pKey2DCB->InsertEntry(aKey);
    }
}

class SwNewUserIdxDlg : public ModalDialog
{
    OKButton*        m_pOKPB;
    Edit*            m_pNameED;

    SwIndexMarkPane* m_pDlg;

    DECL_LINK( ModifyHdl, Edit*);

    public:
        SwNewUserIdxDlg(SwIndexMarkPane* pPane)
            : ModalDialog(&(pPane->GetDialog()), "NewUserIndexDialog",
                "modules/swriter/ui/newuserindexdialog.ui")
            , m_pDlg(pPane)
            {
                get(m_pOKPB, "ok");
                get(m_pNameED, "entry");
                m_pNameED->SetModifyHdl(LINK(this, SwNewUserIdxDlg, ModifyHdl));
                m_pOKPB->Enable(sal_False);
                m_pNameED->GrabFocus();
            }

    virtual void    Apply();
    String  GetName(){return m_pNameED->GetText();}
};
void SwNewUserIdxDlg::Apply()
{
}

IMPL_LINK( SwNewUserIdxDlg, ModifyHdl, Edit*, pEdit)
{
    m_pOKPB->Enable(!pEdit->GetText().isEmpty() && !m_pDlg->IsTOXType(pEdit->GetText()));
    return 0;
}

IMPL_LINK_NOARG(SwIndexMarkPane, NewUserIdxHdl)
{
    SwNewUserIdxDlg* pDlg = new SwNewUserIdxDlg(this);
    if(RET_OK == pDlg->Execute())
    {
        String sNewName(pDlg->GetName());
        m_pTypeDCB->InsertEntry(sNewName);
        m_pTypeDCB->SelectEntry(sNewName);
    }
    delete pDlg;
    return 0;
}

IMPL_LINK( SwIndexMarkPane, SearchTypeHdl, CheckBox*, pBox)
{
    sal_Bool bEnable = pBox->IsChecked() && pBox->IsEnabled();
    m_pSearchCaseWordOnlyCB->Enable(bEnable);
    m_pSearchCaseSensitiveCB->Enable(bEnable);
    return 0;
}

IMPL_LINK( SwIndexMarkPane, InsertHdl, Button *, pButton )
{
    Apply();
    //close the dialog if only one entry is available
    if(!bNewMark && !m_pPrevBT->IsVisible() && !m_pNextBT->IsVisible())
        CloseHdl(pButton);
    return 0;
}

IMPL_LINK_NOARG(SwIndexMarkPane, CloseHdl)
{
    if(bNewMark)
    {
        sal_uInt16 nSlot = FN_INSERT_IDX_ENTRY_DLG;
        SfxViewFrame::Current()->GetDispatcher()->Execute(nSlot,
                    SFX_CALLMODE_ASYNCHRON|SFX_CALLMODE_RECORD);
    }
    else
    {
        m_rDialog.EndDialog(RET_CANCEL);
    }
    return 0;
}

/*--------------------------------------------------------------------
     Description:   select index type only when inserting
 --------------------------------------------------------------------*/
IMPL_LINK( SwIndexMarkPane, ModifyHdl, ListBox *, pBox )
{
    if (m_pTypeDCB == pBox)
    {
        // set index type
        sal_uInt16 nPos = m_pTypeDCB->GetEntryPos(m_pTypeDCB->GetSelectEntry());
        sal_Bool bLevelEnable = sal_False,
             bKeyEnable   = sal_False,
             bSetKey2     = sal_False,
             bKey2Enable  = sal_False,
            bEntryHasText   = sal_False,
            bKey1HasText    = sal_False,
            bKey2HasText    = sal_False;
        if(nPos == POS_INDEX)
        {
            if(!m_pEntryED->GetText().isEmpty())
                bEntryHasText = sal_True;
            m_pPhoneticED0->SetText(GetDefaultPhoneticReading(m_pEntryED->GetText()));

            bKeyEnable = sal_True;
            m_pKey1DCB->SetText(m_pKey1DCB->GetEntry(nKey1Pos));
            m_pPhoneticED1->SetText(GetDefaultPhoneticReading(m_pKey1DCB->GetText()));
            if(!m_pKey1DCB->GetText().isEmpty())
            {
                bKey1HasText = bSetKey2 = bKey2Enable = sal_True;
                m_pKey2DCB->SetText(m_pKey2DCB->GetEntry(nKey2Pos));
                m_pPhoneticED2->SetText(GetDefaultPhoneticReading(m_pKey2DCB->GetText()));
                if(!m_pKey2DCB->GetText().isEmpty())
                    bKey2HasText = sal_True;
            }
        }
        else
        {
            bLevelEnable = sal_True;
            m_pLevelNF->SetMax(MAXLEVEL);
            m_pLevelNF->SetValue(m_pLevelNF->Normalize(0));
            bSetKey2 = sal_True;
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
        bool bHasText = (!m_pEntryED->GetText().isEmpty());
        if(!bHasText)
        {
            m_pPhoneticED0->SetText(aEmptyStr);
            bPhoneticED0_ChangedByUser = sal_False;
        }
        else if(!bPhoneticED0_ChangedByUser)
            m_pPhoneticED0->SetText(GetDefaultPhoneticReading(m_pEntryED->GetText()));

        m_pPhoneticFT0->Enable(bHasText&&bIsPhoneticReadingEnabled);
        m_pPhoneticED0->Enable(bHasText&&bIsPhoneticReadingEnabled);
    }
    m_pOKBT->Enable(!pSh->HasReadonlySel() &&
        (!m_pEntryED->GetText().isEmpty() || pSh->GetCrsrCnt(sal_False)));
    return 0;
}

IMPL_LINK_NOARG_INLINE_START(SwIndexMarkPane, NextHdl)
{
    InsertUpdate();
    pTOXMgr->NextTOXMark();
    UpdateDialog();
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwIndexMarkPane, NextHdl)

IMPL_LINK_NOARG_INLINE_START(SwIndexMarkPane, NextSameHdl)
{
    InsertUpdate();
    pTOXMgr->NextTOXMark(sal_True);
    UpdateDialog();
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwIndexMarkPane, NextSameHdl)

IMPL_LINK_NOARG_INLINE_START(SwIndexMarkPane, PrevHdl)
{
    InsertUpdate();
    pTOXMgr->PrevTOXMark();
    UpdateDialog();
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwIndexMarkPane, PrevHdl)

IMPL_LINK_NOARG_INLINE_START(SwIndexMarkPane, PrevSameHdl)
{
    InsertUpdate();
    pTOXMgr->PrevTOXMark(sal_True);
    UpdateDialog();

    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwIndexMarkPane, PrevSameHdl)

IMPL_LINK_NOARG(SwIndexMarkPane, DelHdl)
{
    bDel = sal_True;
    InsertUpdate();
    bDel = sal_False;

    if(pTOXMgr->GetCurTOXMark())
        UpdateDialog();
    else
    {
        CloseHdl(m_pCloseBT);
        SfxViewFrame::Current()->GetBindings().Invalidate(FN_EDIT_IDX_ENTRY_DLG);
    }
    return 0;
}

/*--------------------------------------------------------------------
     Description: renew dialog view
 --------------------------------------------------------------------*/
void SwIndexMarkPane::UpdateDialog()
{
    OSL_ENSURE(pSh && pTOXMgr, "no shell?");
    SwTOXMark* pMark = pTOXMgr->GetCurTOXMark();
    OSL_ENSURE(pMark, "no current marker");
    if(!pMark)
        return;

    ViewShell::SetCareWin(&m_rDialog);

    aOrgStr = pMark->GetText();
    m_pEntryED->SetText(aOrgStr);

    // set index type
    sal_Bool bLevelEnable = sal_True,
         bKeyEnable   = sal_False,
         bKey2Enable  = sal_False,
         bEntryHasText  = sal_False,
         bKey1HasText   = sal_False,
         bKey2HasText   = sal_False;

    TOXTypes eCurType = pMark->GetTOXType()->GetType();
    if(TOX_INDEX == eCurType)
    {
        bLevelEnable = sal_False;
        bKeyEnable = sal_True;
        bKey1HasText = bKey2Enable = 0 != pMark->GetPrimaryKey().Len();
        bKey2HasText = 0 != pMark->GetSecondaryKey().Len();
        bEntryHasText = 0 != pMark->GetText().Len();
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
    pSh->SttCrsrMove();
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

    sal_Bool bEnable = !pSh->HasReadonlySel();
    m_pOKBT->Enable( bEnable );
    m_pDelBT->Enable( bEnable );
    m_pEntryED->SetReadOnly( !bEnable );
    m_pLevelNF->SetReadOnly( !bEnable );
    m_pKey1DCB->SetReadOnly( !bEnable );
    m_pKey2DCB->SetReadOnly( !bEnable );

    pSh->SelectTxtAttr( RES_TXTATR_TOXMARK, pMark->GetTxtTOXMark() );
    // we need the point at the start of the attribut
    pSh->SwapPam();

    pSh->EndCrsrMove();
}

/*--------------------------------------------------------------------
     Remind whether the edit boxes for Phonetic reading are changed manually
 --------------------------------------------------------------------*/
IMPL_LINK( SwIndexMarkPane, PhoneticEDModifyHdl, Edit *, pEdit )
{
    if (m_pPhoneticED0 == pEdit)
    {
        bPhoneticED0_ChangedByUser = !pEdit->GetText().isEmpty();
    }
    else if (m_pPhoneticED1 == pEdit)
    {
        bPhoneticED1_ChangedByUser = !pEdit->GetText().isEmpty();
    }
    else if (m_pPhoneticED2 == pEdit)
    {
        bPhoneticED2_ChangedByUser = !pEdit->GetText().isEmpty();
    }
    return 0;
}

/*--------------------------------------------------------------------
     Description: Enable Disable of the 2nd key
 --------------------------------------------------------------------*/
IMPL_LINK( SwIndexMarkPane, KeyDCBModifyHdl, ComboBox *, pBox )
{
    if (m_pKey1DCB == pBox)
    {
        sal_Bool bEnable = !pBox->GetText().isEmpty();
        if(!bEnable)
        {
            m_pKey2DCB->SetText(aEmptyStr);
            m_pPhoneticED1->SetText(aEmptyStr);
            m_pPhoneticED2->SetText(aEmptyStr);
            bPhoneticED1_ChangedByUser = sal_False;
            bPhoneticED2_ChangedByUser = sal_False;
        }
        else
        {
            if(pBox->IsInDropDown())
            {
                //reset bPhoneticED1_ChangedByUser if a completely new string is selected
                bPhoneticED1_ChangedByUser = sal_False;
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
            m_pPhoneticED2->SetText(aEmptyStr);
            bPhoneticED2_ChangedByUser = sal_False;
        }
        else
        {
            if(pBox->IsInDropDown())
            {
                //reset bPhoneticED1_ChangedByUser if a completely new string is selected
                bPhoneticED2_ChangedByUser = sal_False;
            }
            if(!bPhoneticED2_ChangedByUser)
                m_pPhoneticED2->SetText(GetDefaultPhoneticReading(pBox->GetText()));
        }
    }
    bool    bKey1HasText    = (!m_pKey1DCB->GetText().isEmpty());
    bool    bKey2HasText    = (!m_pKey2DCB->GetText().isEmpty());

    m_pPhoneticFT1->Enable(bKey1HasText && bIsPhoneticReadingEnabled);
    m_pPhoneticED1->Enable(bKey1HasText && bIsPhoneticReadingEnabled);
    m_pPhoneticFT2->Enable(bKey2HasText && bIsPhoneticReadingEnabled);
    m_pPhoneticED2->Enable(bKey2HasText && bIsPhoneticReadingEnabled);

    return 0;
}

/*--------------------------------------------------
 overload dtor
--------------------------------------------------*/
SwIndexMarkPane::~SwIndexMarkPane()
{
    delete pTOXMgr;
    ViewShell::SetCareWin( 0 );
}

void    SwIndexMarkPane::ReInitDlg(SwWrtShell& rWrtShell, SwTOXMark* pCurTOXMark)
{
    pSh = &rWrtShell;
    delete pTOXMgr;
    pTOXMgr = new SwTOXMgr(pSh);
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
    SfxChildWindow* pChild, Window *pParent,
    SfxChildWinInfo* pInfo, sal_Bool bNew)
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

SwIndexMarkModalDlg::SwIndexMarkModalDlg(Window *pParent, SwWrtShell& rSh, SwTOXMark* pCurTOXMark)
    : SvxStandardDialog(pParent, "IndexEntryDialog", "modules/swriter/ui/indexentry.ui")
    , m_aContent(*this, sal_False, rSh)
{
    m_aContent.ReInitDlg(rSh, pCurTOXMark);
}

void SwIndexMarkModalDlg::Apply()
{
    m_aContent.Apply();
}

class SwCreateAuthEntryDlg_Impl : public ModalDialog
{
    FixedText*      pFixedTexts[AUTH_FIELD_END];
    ListBox*        pTypeListBox;
    ComboBox*       pIdentifierBox;
    Edit*           pEdits[AUTH_FIELD_END];

    OKButton*       m_pOKBT;

    Link            aShortNameCheckLink;

    SwWrtShell&     rWrtSh;

    sal_Bool            m_bNewEntryMode;
    sal_Bool            m_bNameAllowed;

    DECL_LINK(IdentifierHdl, ComboBox*);
    DECL_LINK(ShortNameHdl, Edit*);
    DECL_LINK(EnableHdl, ListBox* pBox);

public:
    SwCreateAuthEntryDlg_Impl(Window* pParent,
                            const String pFields[],
                            SwWrtShell& rSh,
                            sal_Bool bNewEntry,
                            bool bCreate);
    ~SwCreateAuthEntryDlg_Impl();

    String          GetEntryText(ToxAuthorityField eField) const;

    void            SetCheckNameHdl(const Link& rLink) {aShortNameCheckLink = rLink;}

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

sal_Bool SwAuthorMarkPane::bIsFromComponent = sal_True;

SwAuthorMarkPane::SwAuthorMarkPane(Dialog &rDialog, sal_Bool bNewDlg)
    : m_rDialog(rDialog)
    , bNewEntry(bNewDlg)
    , bBibAccessInitialized(sal_False)
    , pSh(0)
{
    m_rDialog.get(m_pFromComponentRB, "frombibliography");
    m_rDialog.get(m_pFromDocContentRB, "fromdocument");
    m_rDialog.get(m_pAuthorFI, "author");
    m_rDialog.get(m_pTitleFI, "title");
    m_rDialog.get(m_pEntryED, "entryed");
    m_rDialog.get(m_pEntryLB, "entrylb");
    m_rDialog.get(m_pActionBT,
        bNewEntry ? OString("insert") : OString("modify"));
    m_pActionBT->Show(true);
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

    m_rDialog.SetText(SW_RESSTR(
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

IMPL_LINK_NOARG(SwAuthorMarkPane, CloseHdl)
{
    if(bNewEntry)
    {
        sal_uInt16 nSlot = FN_INSERT_AUTH_ENTRY_DLG;
        SfxViewFrame::Current()->GetDispatcher()->Execute(nSlot,
                    SFX_CALLMODE_ASYNCHRON|SFX_CALLMODE_RECORD);
    }
    else
    {
        m_rDialog.EndDialog(RET_CANCEL);
    }
    return 0;
}

static String lcl_FindColumnEntry(const beans::PropertyValue* pFields, sal_Int32 nLen, const String& rColumnTitle)
{
    String sRet;
    OUString uColumnTitle = rColumnTitle;
    for(sal_uInt16 i = 0; i < nLen; i++)
    {
        OUString uTmp;
        if(pFields[i].Name == uColumnTitle &&
            (pFields[i].Value >>= uTmp))
        {
            sRet = String(uTmp);
            break;
        }
    }
    return sRet;
}

IMPL_LINK( SwAuthorMarkPane, CompEntryHdl, ListBox*, pBox)
{
    String sEntry(pBox->GetSelectEntry());
    if(bIsFromComponent)
    {
        if(xBibAccess.is() && sEntry.Len())
        {
            OUString uEntry(sEntry);
            if(xBibAccess->hasByName(uEntry))
            {
                uno::Any aEntry(xBibAccess->getByName(uEntry));
                uno::Sequence<beans::PropertyValue> aFieldProps;
                if(aEntry >>= aFieldProps)
                {
                    const beans::PropertyValue* pProps = aFieldProps.getConstArray();
                    for(sal_uInt16 i = 0; i < AUTH_FIELD_END && i < aFieldProps.getLength(); i++)
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
        if(sEntry.Len())
        {
            const SwAuthorityFieldType* pFType = (const SwAuthorityFieldType*)
                                        pSh->GetFldType(RES_AUTHORITY, aEmptyStr);
            const SwAuthEntry*  pEntry = pFType ? pFType->GetEntryByIdentifier(sEntry) : 0;
            for(sal_uInt16 i = 0; i < AUTH_FIELD_END; i++)
                m_sFields[i] = pEntry ?
                            pEntry->GetAuthorField((ToxAuthorityField)i) : OUString();
        }
    }
    if(!pBox->GetSelectEntry().Len())
    {
        for(sal_uInt16 i = 0; i < AUTH_FIELD_END; i++)
            m_sFields[i] = aEmptyStr;
    }
    m_pAuthorFI->SetText(m_sFields[AUTH_FIELD_AUTHOR]);
    m_pTitleFI->SetText(m_sFields[AUTH_FIELD_TITLE]);
    return 0;
}

IMPL_LINK_NOARG(SwAuthorMarkPane, InsertHdl)
{
    //insert or update the SwAuthorityField...
    if(pSh)
    {
        sal_Bool bDifferent = sal_False;
        OSL_ENSURE(m_sFields[AUTH_FIELD_IDENTIFIER].Len() , "No Id is set!");
        OSL_ENSURE(m_sFields[AUTH_FIELD_AUTHORITY_TYPE].Len() , "No authority type is set!");
        //check if the entry already exists with different content
        const SwAuthorityFieldType* pFType = (const SwAuthorityFieldType*)
                                        pSh->GetFldType(RES_AUTHORITY, aEmptyStr);
        const SwAuthEntry*  pEntry = pFType ?
                pFType->GetEntryByIdentifier( m_sFields[AUTH_FIELD_IDENTIFIER])
                : 0;
        if(pEntry)
        {
            for(sal_uInt16 i = 0; i < AUTH_FIELD_END && !bDifferent; i++)
                bDifferent |= m_sFields[i] != pEntry->GetAuthorField((ToxAuthorityField)i);
            if(bDifferent)
            {
                QueryBox aQuery(&m_rDialog, SW_RES(DLG_CHANGE_AUTH_ENTRY));
                if(RET_YES != aQuery.Execute())
                    return 0;
            }
        }

        SwFldMgr aMgr(pSh);
        String sFields;
        for(sal_uInt16 i = 0; i < AUTH_FIELD_END; i++)
        {
            sFields += m_sFields[i];
            sFields += TOX_STYLE_DELIMITER;
        }
        if(bNewEntry)
        {
            if(bDifferent)
            {
                SwAuthEntry aNewData;
                for(sal_uInt16 i = 0; i < AUTH_FIELD_END; i++)
                    aNewData.SetAuthorField((ToxAuthorityField)i, m_sFields[i]);
                pSh->ChangeAuthorityData(&aNewData);
            }
            SwInsertFld_Data aData(TYP_AUTHORITY, 0, sFields, aEmptyStr, 0 );
            aMgr.InsertFld( aData );
        }
        else if(aMgr.GetCurFld())
        {
            aMgr.UpdateCurFld(0, sFields, aEmptyStr);
        }
    }
    if(!bNewEntry)
        CloseHdl(0);
    return 0;
}

IMPL_LINK(SwAuthorMarkPane, CreateEntryHdl, PushButton*, pButton)
{
    bool bCreate = pButton == m_pCreateEntryPB;
    String sOldId = m_sCreatedEntry[0];
    for(sal_uInt16 i = 0; i < AUTH_FIELD_END; i++)
        m_sCreatedEntry[i] = bCreate ? aEmptyStr : m_sFields[i];
    SwCreateAuthEntryDlg_Impl aDlg(pButton,
                bCreate ? m_sCreatedEntry : m_sFields,
                *pSh, bNewEntry, bCreate);
    if(bNewEntry)
    {
        aDlg.SetCheckNameHdl(LINK(this, SwAuthorMarkPane, IsEntryAllowedHdl));
    }
    if(RET_OK == aDlg.Execute())
    {
        if(bCreate && sOldId.Len())
        {
            m_pEntryLB->RemoveEntry(sOldId);
        }
        for(sal_uInt16 i = 0; i < AUTH_FIELD_END; i++)
        {
            m_sFields[i] = aDlg.GetEntryText((ToxAuthorityField)i);
            m_sCreatedEntry[i] = m_sFields[i];
        }
        if(bNewEntry && !m_pFromDocContentRB->IsChecked())
        {
            m_pFromDocContentRB->Check(sal_True);
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
    return 0;
}

IMPL_LINK(SwAuthorMarkPane, ChangeSourceHdl, RadioButton*, pButton)
{
    sal_Bool bFromComp = (pButton == m_pFromComponentRB);
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
                    for(sal_uInt16 i = 0; i < aSeq.getLength(); i++)
                    {
                        String sTitle = pArr[i].Name;
                        sal_Int16 nField = 0;
                        pArr[i].Value >>= nField;
                        if(nField >= 0 && nField < AUTH_FIELD_END)
                            m_sColumnTitles[nField] = sTitle;
                    }
                }
            }
            bBibAccessInitialized = sal_True;
        }
        if(xBibAccess.is())
        {
            uno::Sequence<OUString> aIdentifiers = xBibAccess->getElementNames();
            const OUString* pNames = aIdentifiers.getConstArray();
            for(sal_uInt16 i = 0; i < aIdentifiers.getLength(); i++)
            {
                m_pEntryLB->InsertEntry(pNames[i]);
            }
        }
    }
    else
    {
        const SwAuthorityFieldType* pFType = (const SwAuthorityFieldType*)
                                    pSh->GetFldType(RES_AUTHORITY, aEmptyStr);
        if(pFType)
        {
            std::vector<OUString> aIds;
            pFType->GetAllEntryIdentifiers( aIds );
            for(size_t n = 0; n < aIds.size(); ++n)
                m_pEntryLB->InsertEntry(aIds[n]);
        }
        if(m_sCreatedEntry[AUTH_FIELD_IDENTIFIER].Len())
            m_pEntryLB->InsertEntry(m_sCreatedEntry[AUTH_FIELD_IDENTIFIER]);
    }
    m_pEntryLB->SelectEntryPos(0);
    CompEntryHdl(m_pEntryLB);
    return 0;
}

IMPL_LINK(SwAuthorMarkPane, EditModifyHdl, Edit*, pEdit)
{
    Link aAllowed = LINK(this, SwAuthorMarkPane, IsEntryAllowedHdl);
    long nResult = aAllowed.Call(pEdit);
    m_pActionBT->Enable(nResult > 0);
    if(nResult)
    {
        String sEntry(pEdit->GetText());
        m_sFields[AUTH_FIELD_IDENTIFIER] = sEntry;
        m_sCreatedEntry[AUTH_FIELD_IDENTIFIER] = sEntry;
    }
    return 0;
};

IMPL_LINK(SwAuthorMarkPane, IsEntryAllowedHdl, Edit*, pEdit)
{
    String sEntry = pEdit->GetText();
    sal_Bool bAllowed = sal_False;
    if(sEntry.Len())
    {
        if(m_pEntryLB->GetEntryPos(sEntry) != LISTBOX_ENTRY_NOTFOUND)
            return 0;
        else if(bIsFromComponent)
        {
            const SwAuthorityFieldType* pFType = (const SwAuthorityFieldType*)
                                        pSh->GetFldType(RES_AUTHORITY, aEmptyStr);
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
    SwField* pField = pSh->GetCurFld();
    OSL_ENSURE(bNewEntry || pField, "no current marker");
    if(bNewEntry)
    {
        ChangeSourceHdl(m_pFromComponentRB->IsChecked() ? m_pFromComponentRB : m_pFromDocContentRB);
        m_pCreateEntryPB->Enable(!m_pFromComponentRB->IsChecked());
        if(!m_pFromComponentRB->IsChecked() && m_sCreatedEntry[0].Len())
            for(sal_uInt16 i = 0; i < AUTH_FIELD_END; i++)
                m_sFields[i] = m_sCreatedEntry[i];
    }
    if(bNewEntry || !pField || pField->GetTyp()->Which() != RES_AUTHORITY)
        return;

    const SwAuthEntry* pEntry = ((SwAuthorityFieldType*)pField->GetTyp())->
            GetEntryByHandle(((SwAuthorityField*)pField)->GetHandle());

    OSL_ENSURE(pEntry, "No authority entry found");
    if(!pEntry)
        return;
    for(sal_uInt16 i = 0; i < AUTH_FIELD_END; i++)
        m_sFields[i] = pEntry->GetAuthorField((ToxAuthorityField)i);

    m_pEntryED->SetText(pEntry->GetAuthorField(AUTH_FIELD_IDENTIFIER));
    m_pAuthorFI->SetText(pEntry->GetAuthorField(AUTH_FIELD_AUTHOR));
    m_pTitleFI->SetText(pEntry->GetAuthorField(AUTH_FIELD_TITLE));
}

void SwAuthorMarkPane::Activate()
{
    m_pActionBT->Enable(!pSh->HasReadonlySel());
}

SwCreateAuthEntryDlg_Impl::SwCreateAuthEntryDlg_Impl(Window* pParent,
        const String pFields[],
        SwWrtShell& rSh,
        sal_Bool bNewEntry,
        bool bCreate)
    : ModalDialog(pParent, "CreateAuthorEntryDialog", "modules/swriter/ui/createauthorentry.ui")

    ,

    pTypeListBox(0),
    pIdentifierBox(0),
    rWrtSh(rSh),
    m_bNewEntryMode(bNewEntry),
    m_bNameAllowed(sal_True)
{
    get(m_pOKBT, "ok");

    VclGrid *pLeft = get<VclGrid>("leftgrid");
    VclGrid *pRight = get<VclGrid>("rightgrid");

    bool bLeft = true;
    sal_Int32 nLeftRow(0), nRightRow(0);
    for(sal_uInt16 nIndex = 0; nIndex < AUTH_FIELD_END; nIndex++)
    {
        const TextInfo aCurInfo = aTextInfoArr[nIndex];

        pFixedTexts[nIndex] = new FixedText(bLeft ? pLeft : pRight, WB_VCENTER);

        pFixedTexts[nIndex]->set_grid_left_attach(0);
        pFixedTexts[nIndex]->set_grid_top_attach(bLeft ? nLeftRow : nRightRow);
        pFixedTexts[nIndex]->SetText(SW_RES(STR_AUTH_FIELD_START + aCurInfo.nToxField));
        pFixedTexts[nIndex]->Show();
        pEdits[nIndex] = 0;
        if( AUTH_FIELD_AUTHORITY_TYPE == aCurInfo.nToxField )
        {
            pTypeListBox = new ListBox(bLeft ? pLeft : pRight, WB_DROPDOWN|WB_BORDER|WB_VCENTER);
            for(sal_uInt16 j = 0; j < AUTH_TYPE_END; j++)
                pTypeListBox->InsertEntry(SW_RESSTR(STR_AUTH_TYPE_START + j));
            if(pFields[aCurInfo.nToxField].Len())
            {
                sal_uInt16 nIndexPos = static_cast< sal_uInt16 >(pFields[aCurInfo.nToxField].ToInt32());
                pTypeListBox->SelectEntryPos(nIndexPos);
            }
            pTypeListBox->set_grid_left_attach(1);
            pTypeListBox->set_grid_top_attach(bLeft ? nLeftRow : nRightRow);
            pTypeListBox->set_hexpand(true);
            pTypeListBox->Show();
            pTypeListBox->SetSelectHdl(LINK(this, SwCreateAuthEntryDlg_Impl, EnableHdl));
            pTypeListBox->SetHelpId(aCurInfo.pHelpId);
            pFixedTexts[nIndex]->set_mnemonic_widget(pTypeListBox);
        }
        else if(AUTH_FIELD_IDENTIFIER == aCurInfo.nToxField && !m_bNewEntryMode)
        {
            pIdentifierBox = new ComboBox(bLeft ? pLeft : pRight, WB_BORDER|WB_DROPDOWN|WB_VCENTER);

            pIdentifierBox->SetSelectHdl(LINK(this,
                                    SwCreateAuthEntryDlg_Impl, IdentifierHdl));


            const SwAuthorityFieldType* pFType = (const SwAuthorityFieldType*)
                                        rSh.GetFldType(RES_AUTHORITY, aEmptyStr);
            if(pFType)
            {
                std::vector<OUString> aIds;
                pFType->GetAllEntryIdentifiers( aIds );
                for(size_t n = 0; n < aIds.size(); ++n)
                    pIdentifierBox->InsertEntry(aIds[n]);
            }
            pIdentifierBox->SetText(pFields[aCurInfo.nToxField]);
            pIdentifierBox->set_grid_left_attach(1);
            pIdentifierBox->set_grid_top_attach(bLeft ? nLeftRow : nRightRow);
            pIdentifierBox->set_hexpand(true);
            pIdentifierBox->Show();
            pIdentifierBox->SetHelpId(aCurInfo.pHelpId);
            pFixedTexts[nIndex]->set_mnemonic_widget(pIdentifierBox);
        }
        else
        {
            pEdits[nIndex] = new Edit(bLeft ? pLeft : pRight, WB_BORDER|WB_VCENTER);
            pEdits[nIndex]->SetWidthInChars(14);
            pEdits[nIndex]->set_grid_left_attach(1);
            pEdits[nIndex]->set_grid_top_attach(bLeft ? nLeftRow : nRightRow);
            pEdits[nIndex]->set_hexpand(true);
            pEdits[nIndex]->SetText(pFields[aCurInfo.nToxField]);
            pEdits[nIndex]->Show();
            pEdits[nIndex]->SetHelpId(aCurInfo.pHelpId);
            if(AUTH_FIELD_IDENTIFIER == aCurInfo.nToxField)
            {
                pEdits[nIndex]->SetModifyHdl(LINK(this, SwCreateAuthEntryDlg_Impl, ShortNameHdl));
                m_bNameAllowed = pFields[nIndex].Len() > 0;
                if(!bCreate)
                {
                    pFixedTexts[nIndex]->Enable(sal_False);
                    pEdits[nIndex]->Enable(sal_False);
                }
            }
            pFixedTexts[nIndex]->set_mnemonic_widget(pEdits[nIndex]);
        }
        if(bLeft)
            ++nLeftRow;
        else
            ++nRightRow;
        bLeft = !bLeft;
    }
    EnableHdl(pTypeListBox);
}

SwCreateAuthEntryDlg_Impl::~SwCreateAuthEntryDlg_Impl()
{
    for(sal_uInt16 i = 0; i < AUTH_FIELD_END; i++)
    {
        delete pFixedTexts[i];
        delete pEdits[i];
    }
    delete pTypeListBox;
    delete pIdentifierBox;
}

String  SwCreateAuthEntryDlg_Impl::GetEntryText(ToxAuthorityField eField) const
{
    String sRet;
    if( AUTH_FIELD_AUTHORITY_TYPE == eField )
    {
        OSL_ENSURE(pTypeListBox, "No ListBox");
        sRet = OUString::number(pTypeListBox->GetSelectEntryPos());
    }
    else if( AUTH_FIELD_IDENTIFIER == eField && !m_bNewEntryMode)
    {
        OSL_ENSURE(pIdentifierBox, "No ComboBox");
        sRet = pIdentifierBox->GetText();
    }
    else
    {
        for(sal_uInt16 nIndex = 0; nIndex < AUTH_FIELD_END; nIndex++)
        {
            const TextInfo aCurInfo = aTextInfoArr[nIndex];
            if(aCurInfo.nToxField == eField)
            {
                sRet = pEdits[nIndex]->GetText();
                break;
            }
        }
    }
    return sRet;
}

IMPL_LINK(SwCreateAuthEntryDlg_Impl, IdentifierHdl, ComboBox*, pBox)
{
    const SwAuthorityFieldType* pFType = (const SwAuthorityFieldType*)
                                rWrtSh.GetFldType(RES_AUTHORITY, aEmptyStr);
    if(pFType)
    {
        const SwAuthEntry* pEntry = pFType->GetEntryByIdentifier(
                                                        pBox->GetText());
        if(pEntry)
        {
            for(sal_uInt16 i = 0; i < AUTH_FIELD_END; i++)
            {
                const TextInfo aCurInfo = aTextInfoArr[i];
                if(AUTH_FIELD_IDENTIFIER == aCurInfo.nToxField)
                    continue;
                if(AUTH_FIELD_AUTHORITY_TYPE == aCurInfo.nToxField)
                    pTypeListBox->SelectEntry(
                                pEntry->GetAuthorField(aCurInfo.nToxField));
                else
                    pEdits[i]->SetText(
                                pEntry->GetAuthorField(aCurInfo.nToxField));
            }
        }
    }
    return 0;
}

IMPL_LINK(SwCreateAuthEntryDlg_Impl, ShortNameHdl, Edit*, pEdit)
{
    if(aShortNameCheckLink.IsSet())
    {
        sal_Bool bEnable = 0 != aShortNameCheckLink.Call(pEdit);
        m_bNameAllowed |= bEnable;
        m_pOKBT->Enable(pTypeListBox->GetSelectEntryCount() && bEnable);
    }
    return 0;
}

IMPL_LINK(SwCreateAuthEntryDlg_Impl, EnableHdl, ListBox*, pBox)
{
    m_pOKBT->Enable(m_bNameAllowed && pBox->GetSelectEntryCount());
    return 0;
};

SwAuthMarkFloatDlg::SwAuthMarkFloatDlg(SfxBindings* _pBindings,
                                   SfxChildWindow* pChild,
                                   Window *pParent,
                                SfxChildWinInfo* pInfo,
                                   sal_Bool bNew)
    : SfxModelessDialog(_pBindings, pChild, pParent,
        "BibliographyEntryDialog", "modules/swriter/ui/bibliographyentry.ui")
    , m_aContent(*this, bNew)
{
    Initialize(pInfo);
    SwWrtShell* pWrtShell = ::GetActiveWrtShell();
    OSL_ENSURE(pWrtShell, "No shell?");
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

SwAuthMarkModalDlg::SwAuthMarkModalDlg(Window *pParent, SwWrtShell& rSh)
    : SvxStandardDialog(pParent, "BibliographyEntryDialog",
        "modules/swriter/ui/bibliographyentry.ui")
    , m_aContent(*this, sal_False)
{
    m_aContent.ReInitDlg(rSh);
}

void SwAuthMarkModalDlg::Apply()
{
    m_aContent.InsertHdl(0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
