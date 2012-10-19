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

#include "swuiidxmrk.hxx"
#include <hintids.hxx>
#include <helpid.h>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/SearchOptions.hpp>
#include <com/sun/star/util/SearchFlags.hpp>
#include <com/sun/star/i18n/TransliterationModules.hpp>
#include <svl/stritem.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/eitem.hxx>
#include <svtools/txtcmp.hxx>
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
#define POS_USER    2

static sal_uInt16 nTypePos = 1; // TOX_INDEX as standard
static sal_uInt16 nKey1Pos = USHRT_MAX;

static sal_uInt16 nKey2Pos = USHRT_MAX;

using namespace com::sun::star;
using namespace com::sun::star::i18n;
using namespace com::sun::star::lang;
using namespace com::sun::star::util;
using namespace com::sun::star::i18n;
using ::rtl::OUString;
using namespace ::comphelper;
using namespace ::com::sun::star;

/*--------------------------------------------------------------------
     Description:   dialog to insert a directory selection
 --------------------------------------------------------------------*/
SwIndexMarkDlg::SwIndexMarkDlg(Window *pParent,
                               sal_Bool bNewDlg,
                               const ResId& rResId,
                               sal_Int32 _nOptionsId, SwWrtShell& rWrtShell ) :
      Window(pParent, rResId),
    aIndexFL(this,  SW_RES(FL_INDEX )),
    aTypeFT (this,  SW_RES(LBL_INDEX    )),
    aTypeDCB(this,  SW_RES(DCB_INDEX    )),
    aNewBT(this,    SW_RES(BT_NEW   )),

    aEntryFT(this,  SW_RES(LBL_ENTRY    )),
    aEntryED(this,  SW_RES(SL_ENTRY )),
    aPhoneticFT0(this,  SW_RES(FT_PHONETIC_1 )),
    aPhoneticED0(this,  SW_RES(ED_PHONETIC_1 )),

    aKeyFT(this,    SW_RES(LBL_KEY  )),
    aKeyDCB(this,   SW_RES(DCB_KEY  )),
    aPhoneticFT1(this,  SW_RES(FT_PHONETIC_2 )),
    aPhoneticED1(this,  SW_RES(ED_PHONETIC_2 )),

    aKey2FT(this,   SW_RES(LBL_KEY2 )),
    aKey2DCB(this,  SW_RES(DCB_KEY2 )),
    aPhoneticFT2(this,  SW_RES(FT_PHONETIC_3 )),
    aPhoneticED2(this,  SW_RES(ED_PHONETIC_3 )),

    aLevelFT(this,  SW_RES(LBL_LEVEL    )),
    aLevelED(this,  SW_RES(SL_LEVEL )),
    aMainEntryCB(this, SW_RES(CB_MAIN_ENTRY )),
    aApplyToAllCB(this,SW_RES(CB_APPLY_TO_ALL)),
    aSearchCaseSensitiveCB(this,    SW_RES(CB_CASESENSITIVE )),
    aSearchCaseWordOnlyCB(this,     SW_RES(CB_WORDONLY      )),


    aOKBT(this,     SW_RES(BT_OK    )),
    aCancelBT(this, SW_RES(BT_CANCEL )),
    aHelpBT(this,   SW_RES(BT_HELP   )),
    aDelBT(this,    SW_RES(BT_DEL   )),

    aPrevSameBT(this,SW_RES(BT_PREVSAME)),
    aNextSameBT(this,SW_RES(BT_NXTSAME)),
    aPrevBT(this,   SW_RES(BT_PREV  )),
    aNextBT(this,   SW_RES(BT_NXT   )),

    nOptionsId( _nOptionsId ),
    bDel(sal_False),
    bNewMark(bNewDlg),
    bSelected(sal_False),

    bPhoneticED0_ChangedByUser(sal_False),
    bPhoneticED1_ChangedByUser(sal_False),
    bPhoneticED2_ChangedByUser(sal_False),
    nLangForPhoneticReading(2052),
    bIsPhoneticReadingEnabled(sal_False),
    xExtendedIndexEntrySupplier(NULL),
    pTOXMgr(0),
    pSh(&rWrtShell)
{
    aNewBT.SetAccessibleRelationMemberOf(&aIndexFL);

    if( SvtCJKOptions().IsCJKFontEnabled() )
    {
        uno::Reference< lang::XMultiServiceFactory > xMSF = getProcessServiceFactory();

        xExtendedIndexEntrySupplier =
            uno::Reference< i18n::XExtendedIndexEntrySupplier > (
                    xMSF->createInstance( C2U("com.sun.star.i18n.IndexEntrySupplier") ),
                                                                        uno::UNO_QUERY );
    }

    SetStyle(GetStyle()|WB_DIALOGCONTROL);
    FreeResource();
    aOKBT           .SetHelpId(HID_INSERT_IDX_MRK_OK        );
    aCancelBT       .SetHelpId(HID_INSERT_IDX_MRK_CLOSE     );
    aDelBT          .SetHelpId(HID_INSERT_IDX_MRK_DELETE    );
    aNewBT          .SetHelpId(HID_INSERT_IDX_MRK_NEW       );
    aPrevBT         .SetHelpId(HID_INSERT_IDX_MRK_PREV      );
    aPrevSameBT     .SetHelpId(HID_INSERT_IDX_MRK_PREV_SAME );
    aNextBT         .SetHelpId(HID_INSERT_IDX_MRK_NEXT      );
    aNextSameBT     .SetHelpId(HID_INSERT_IDX_MRK_NEXT_SAME );
    aTypeDCB        .SetHelpId(HID_INSERT_IDX_MRK_TYPE      );
    aEntryED        .SetHelpId(HID_INSERT_IDX_MRK_ENTRY     );
    aKeyDCB         .SetHelpId(HID_INSERT_IDX_MRK_PRIM_KEY  );
    aKey2DCB        .SetHelpId(HID_INSERT_IDX_MRK_SEC_KEY   );
    aLevelED        .SetHelpId(HID_INSERT_IDX_MRK_LEVEL     );
    aMainEntryCB    .SetHelpId(HID_INSERT_IDX_MRK_MAIN_ENTRY);
    aApplyToAllCB   .SetHelpId(HID_INSERT_IDX_MRK_APPLY_ALL );
    aPhoneticED0    .SetHelpId(HID_INSERT_IDX_MRK_PHONETIC_READING );
    aPhoneticED1    .SetHelpId(HID_INSERT_IDX_MRK_PHONETIC_READING );
    aPhoneticED2    .SetHelpId(HID_INSERT_IDX_MRK_PHONETIC_READING );

    aSearchCaseSensitiveCB.SetHelpId(   HID_INSERT_IDX_MRK_SRCH_CASESENSITIVE   );
    aSearchCaseWordOnlyCB.SetHelpId(    HID_INSERT_IDX_MRK_SRCH_WORDONLY        );


    GetParent()->SetText( SW_RESSTR( bNewMark ? STR_IDXMRK_INSERT : STR_IDXMRK_EDIT));

    aDelBT.SetClickHdl(LINK(this,SwIndexMarkDlg,        DelHdl));
    aPrevBT.SetClickHdl(LINK(this,SwIndexMarkDlg,       PrevHdl));
    aPrevSameBT.SetClickHdl(LINK(this,SwIndexMarkDlg,   PrevSameHdl));
    aNextBT.SetClickHdl(LINK(this,SwIndexMarkDlg,       NextHdl));
    aNextSameBT.SetClickHdl(LINK(this,SwIndexMarkDlg,   NextSameHdl));
    aTypeDCB.SetSelectHdl(LINK(this,SwIndexMarkDlg,     ModifyHdl));
    aKeyDCB.SetModifyHdl(LINK(this,SwIndexMarkDlg,      KeyDCBModifyHdl));
    aKey2DCB.SetModifyHdl(LINK(this,SwIndexMarkDlg,     KeyDCBModifyHdl));
    aOKBT.SetClickHdl(LINK(this,SwIndexMarkDlg,         InsertHdl));
    aCancelBT.SetClickHdl(LINK(this,SwIndexMarkDlg,     CloseHdl));
    aEntryED.SetModifyHdl(LINK(this,SwIndexMarkDlg,     ModifyHdl));
    aNewBT.SetClickHdl(LINK(this, SwIndexMarkDlg,       NewUserIdxHdl));
    aApplyToAllCB.SetClickHdl(LINK(this, SwIndexMarkDlg, SearchTypeHdl));
    aPhoneticED0.SetModifyHdl(LINK(this,SwIndexMarkDlg, PhoneticEDModifyHdl));
    aPhoneticED1.SetModifyHdl(LINK(this,SwIndexMarkDlg, PhoneticEDModifyHdl));
    aPhoneticED2.SetModifyHdl(LINK(this,SwIndexMarkDlg, PhoneticEDModifyHdl));

    if(bNewMark)
    {
        aDelBT.Hide();
        ImageList aTempList( SW_RES( IMG_NAVI_ENTRYBMP ) );
        aNewBT.SetModeImage( aTempList.GetImage( SID_SW_START + CONTENT_TYPE_INDEX ) );
    }
    else
    {
        aNewBT.Hide();
        OKButton aTmp(this, WB_HIDE);
        aOKBT.SetText( aTmp.GetText() );
    }

    aEntryED.GrabFocus();
}

/*--------------------------------------------------------------------
     Description: Newly initialise controls with the new selection
 --------------------------------------------------------------------*/
void SwIndexMarkDlg::InitControls()
{
    OSL_ENSURE(pSh && pTOXMgr, "no shell?");
    // contents index
    const SwTOXType* pType = pTOXMgr->GetTOXType(TOX_CONTENT, 0);
    OSL_ENSURE(pType, "Kein Verzeichnistyp !!");
    String sTmpTypeSelection;
    if(aTypeDCB.GetSelectEntryCount())
        sTmpTypeSelection = aTypeDCB.GetSelectEntry();
    aTypeDCB.Clear();
    aTypeDCB.InsertEntry(pType->GetTypeName());

    // keyword index
    pType = pTOXMgr->GetTOXType(TOX_INDEX, 0);
    OSL_ENSURE(pType, "Kein Verzeichnistyp !!");
    aTypeDCB.InsertEntry(pType->GetTypeName());

    // user index
    sal_uInt16 nCount = pSh->GetTOXTypeCount(TOX_USER);
    sal_uInt16 i;
    for( i = 0; i < nCount; ++i )
        aTypeDCB.InsertEntry( pSh->GetTOXType(TOX_USER, i)->GetTypeName() );

    // read keywords primary
    std::vector<String> aArr;
    nCount = pSh->GetTOIKeys( TOI_PRIMARY, aArr );
    std::sort(aArr.begin(), aArr.end());
    for (std::vector<String>::iterator it = aArr.begin(); it != aArr.end(); ++it)
        aKeyDCB.InsertEntry( *it );

    // read keywords secondary
    nCount = pSh->GetTOIKeys( TOI_SECONDARY, aArr );
    std::sort(aArr.begin(), aArr.end());
    for (std::vector<String>::iterator it = aArr.begin(); it != aArr.end(); ++it)
        aKey2DCB.InsertEntry( *it );

    UpdateLanguageDependenciesForPhoneticReading();

    // current entry
    const SwTOXMark* pMark = pTOXMgr->GetCurTOXMark();
    if( pMark && !bNewMark)
    {
        // Controls-Handling

        // onyl if there are more than one
        // if equal it lands at the same entry
        pSh->SttCrsrMove();

        const SwTOXMark* pMoveMark;
        sal_Bool bShow = sal_False;

        pMoveMark = &pSh->GotoTOXMark( *pMark, TOX_PRV );
        if( pMoveMark != pMark )
            pSh->GotoTOXMark( *pMoveMark, TOX_NXT ), bShow = sal_True;
        aPrevBT.Enable( pMoveMark != pMark );
        pMoveMark = &pSh->GotoTOXMark( *pMark, TOX_NXT );
        if( pMoveMark != pMark )
            pSh->GotoTOXMark( *pMoveMark, TOX_PRV ), bShow = sal_True;
        aNextBT.Enable( pMoveMark != pMark );
        if( bShow )
        {
            aPrevBT.Show();
            aNextBT.Show();
            bShow = sal_False;
        }

        pMoveMark = &pSh->GotoTOXMark( *pMark, TOX_SAME_PRV );
        if( pMoveMark != pMark )
            pSh->GotoTOXMark( *pMoveMark, TOX_SAME_NXT ), bShow = sal_True;
        aPrevSameBT.Enable( pMoveMark != pMark );
        pMoveMark = &pSh->GotoTOXMark( *pMark, TOX_SAME_NXT );
        if( pMoveMark != pMark )
            pSh->GotoTOXMark( *pMoveMark, TOX_SAME_PRV ), bShow = sal_True;
        aNextSameBT.Enable( pMoveMark != pMark );
        if( bShow )
        {
            aNextSameBT.Show();
            aPrevSameBT.Show();
        }
        pSh->EndCrsrMove();

        aTypeFT.Show();

        aTypeDCB.Enable(sal_False);
        aTypeFT.Enable(sal_False);

        UpdateDialog();
    }
    else
    {   // display current selection (first element) ????
        sal_uInt16 nCnt = pSh->GetCrsrCnt();
        if (nCnt < 2)
        {
            bSelected = !pSh->HasSelection();
            aOrgStr = pSh->GetView().GetSelectionTextParam(sal_True, sal_False);
            aEntryED.SetText(aOrgStr);

            //to include all equal entries may only be allowed in the body and even there
            //only when a simple selection exists
            const sal_uInt16 nFrmType = pSh->GetFrmType(0,sal_True);
            aApplyToAllCB.Show();
            aSearchCaseSensitiveCB.Show();
            aSearchCaseWordOnlyCB.Show();
            aApplyToAllCB.Enable(0 != aOrgStr.Len() &&
                0 == (nFrmType & ( FRMTYPE_HEADER | FRMTYPE_FOOTER | FRMTYPE_FLY_ANY )));
            SearchTypeHdl(&aApplyToAllCB);
        }

        // index type is default
        if( sTmpTypeSelection.Len() &&
            LISTBOX_ENTRY_NOTFOUND != aTypeDCB.GetEntryPos( sTmpTypeSelection ) )
            aTypeDCB.SelectEntry(sTmpTypeSelection);
        else
            aTypeDCB.SelectEntry(aTypeDCB.GetEntry(nTypePos));
        ModifyHdl(&aTypeDCB);
    }
}

void    SwIndexMarkDlg::UpdateLanguageDependenciesForPhoneticReading()
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

String  SwIndexMarkDlg::GetDefaultPhoneticReading( const String& rText )
{
    if( !bIsPhoneticReadingEnabled )
        return aEmptyStr;

    lang::Locale aLocale( SvxCreateLocale( LanguageType( nLangForPhoneticReading ) ) );
    return xExtendedIndexEntrySupplier->getPhoneticCandidate(rText, aLocale);
}

/* --------------------------------------------------
    Change the content of aEntryED if text is selected
 --------------------------------------------------*/
void    SwIndexMarkDlg::Activate()
{
    // display current selection (first element) ????
    if(bNewMark)
    {
        sal_uInt16 nCnt = pSh->GetCrsrCnt();
        if (nCnt < 2)
        {
            bSelected = !pSh->HasSelection();
            aOrgStr = pSh->GetView().GetSelectionTextParam(sal_True, sal_False);
            aEntryED.SetText(aOrgStr);

            //to include all equal entries may only be allowed in the body and even there
            //only when a simple selection exists
            const sal_uInt16 nFrmType = pSh->GetFrmType(0,sal_True);
            aApplyToAllCB.Show();
            aSearchCaseSensitiveCB.Show();
            aSearchCaseWordOnlyCB.Show();
            aApplyToAllCB.Enable(0 != aOrgStr.Len() &&
                0 == (nFrmType & ( FRMTYPE_HEADER | FRMTYPE_FOOTER | FRMTYPE_FLY_ANY )));
            SearchTypeHdl(&aApplyToAllCB);
        }
        ModifyHdl(&aTypeDCB);
    }
}

/*--------------------------------------------------------------------
     Description:   evaluate Ok-Button
 --------------------------------------------------------------------*/
void SwIndexMarkDlg::Apply()
{
    InsertUpdate();
    if(bSelected)
        pSh->ResetSelect(0, sal_False);
}

/*--------------------------------------------------------------------
    Description: apply changes
 --------------------------------------------------------------------*/
void SwIndexMarkDlg::InsertUpdate()
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

    if((nTypePos = aTypeDCB.GetEntryPos(aTypeDCB.GetSelectEntry())) == LISTBOX_ENTRY_NOTFOUND)
        nTypePos = 0;

    nKey1Pos = aKeyDCB.GetEntryPos(aKeyDCB.GetText());
    nKey2Pos = aKey2DCB.GetEntryPos(aKey2DCB.GetText());
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
                        SvxCreateLocale( GetAppLanguage() ),
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


void SwIndexMarkDlg::InsertMark()
{
    sal_uInt16 nPos = aTypeDCB.GetEntryPos(aTypeDCB.GetSelectEntry());
    TOXTypes eType = nPos == POS_CONTENT ? TOX_CONTENT :
                        nPos == POS_INDEX ? TOX_INDEX : TOX_USER;

    SwTOXMarkDescription aDesc(eType);

    sal_uInt16 nLevel = (sal_uInt16)aLevelED.Denormalize(aLevelED.GetValue());
    switch(nPos)
    {
        case POS_CONTENT : break;
        case POS_INDEX:     // keyword index mark
        {
            UpdateKeyBoxes();
            String  aPrim(aKeyDCB.GetText());
            String  aSec(aKey2DCB.GetText());
            aDesc.SetPrimKey(aPrim);
            aDesc.SetSecKey(aSec);
            aDesc.SetMainEntry(aMainEntryCB.IsChecked());
            aDesc.SetPhoneticReadingOfAltStr(aPhoneticED0.GetText());
            aDesc.SetPhoneticReadingOfPrimKey(aPhoneticED1.GetText());
            aDesc.SetPhoneticReadingOfSecKey(aPhoneticED2.GetText());
        }
        break;
        default:            // Userdefined index mark
        {
            String aName(aTypeDCB.GetSelectEntry());
            aDesc.SetTOUName(aName);
        }
    }
    if (aOrgStr != aEntryED.GetText())
        aDesc.SetAltStr(aEntryED.GetText());
    sal_Bool bApplyAll = aApplyToAllCB.IsChecked();
    sal_Bool bWordOnly = aSearchCaseWordOnlyCB.IsChecked();
    sal_Bool bCaseSensitive = aSearchCaseSensitiveCB.IsChecked();

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
void SwIndexMarkDlg::UpdateMark()
{
    String  aAltText(aEntryED.GetText());
    String* pAltText = aOrgStr != aEntryED.GetText() ? &aAltText : 0;
    //empty alternative texts are not allowed
    if(pAltText && !pAltText->Len())
        return;

    UpdateKeyBoxes();

    sal_uInt16 nPos = aTypeDCB.GetEntryPos(aTypeDCB.GetSelectEntry());
    TOXTypes eType = TOX_USER;
    if(POS_CONTENT == nPos)
        eType = TOX_CONTENT;
    else if(POS_INDEX == nPos)
        eType = TOX_INDEX;

    SwTOXMarkDescription aDesc(eType);
    aDesc.SetLevel( static_cast< int >(aLevelED.GetValue()) );
    if(pAltText)
        aDesc.SetAltStr(*pAltText);

    String  aPrim(aKeyDCB.GetText());
    if(aPrim.Len())
        aDesc.SetPrimKey(aPrim);
    String  aSec(aKey2DCB.GetText());
    if(aSec.Len())
        aDesc.SetSecKey(aSec);

    if(eType == TOX_INDEX)
    {
        aDesc.SetPhoneticReadingOfAltStr(aPhoneticED0.GetText());
        aDesc.SetPhoneticReadingOfPrimKey(aPhoneticED1.GetText());
        aDesc.SetPhoneticReadingOfSecKey(aPhoneticED2.GetText());
    }
    aDesc.SetMainEntry(aMainEntryCB.IsVisible() && aMainEntryCB.IsChecked());
    pTOXMgr->UpdateTOXMark(aDesc);
}

/*--------------------------------------------------------------------
    Description: insert new keys
 --------------------------------------------------------------------*/
void SwIndexMarkDlg::UpdateKeyBoxes()
{
    String aKey(aKeyDCB.GetText());
    sal_uInt16 nPos = aKeyDCB.GetEntryPos(aKey);

    if(nPos == LISTBOX_ENTRY_NOTFOUND && aKey.Len() > 0)
    {   // create new key
        aKeyDCB.InsertEntry(aKey);
    }

    aKey = aKey2DCB.GetText();
    nPos = aKey2DCB.GetEntryPos(aKey);

    if(nPos == LISTBOX_ENTRY_NOTFOUND && aKey.Len() > 0)
    {   // create new key
        aKey2DCB.InsertEntry(aKey);
    }
}

class SwNewUserIdxDlg : public ModalDialog
{
    OKButton        aOKPB;
    CancelButton    aCancelPB;
    HelpButton      aHelpPB;
    FixedLine       aNameFL;
    FixedText       aNameFT;
    Edit            aNameED;

    SwIndexMarkDlg* pDlg;

    DECL_LINK( ModifyHdl, Edit*);

    public:
        SwNewUserIdxDlg(SwIndexMarkDlg* pParent) :
            ModalDialog(pParent, SW_RES(DLG_NEW_USER_IDX)),
            aOKPB(this, SW_RES(     PB_OK       )),
            aCancelPB(this, SW_RES( PB_CANCEL   )),
            aHelpPB(this, SW_RES(   PB_HELP     )),
            aNameFL(this, SW_RES(    FL_NAME     )),
            aNameFT(this, SW_RES(   FT_NAME     )),
            aNameED(this, SW_RES(    ED_NAME     )),
            pDlg(pParent)
            {
                FreeResource();
                aNameED.SetModifyHdl(LINK(this, SwNewUserIdxDlg, ModifyHdl));
                aOKPB.Enable(sal_False);
                aNameED.GrabFocus();
            }

    virtual void    Apply();
    String  GetName(){return aNameED.GetText();}
};
void SwNewUserIdxDlg::Apply()
{
}

IMPL_LINK( SwNewUserIdxDlg, ModifyHdl, Edit*, pEdit)
{
    aOKPB.Enable(pEdit->GetText().Len() && !pDlg->IsTOXType(pEdit->GetText()));
    return 0;
}

IMPL_LINK_NOARG(SwIndexMarkDlg, NewUserIdxHdl)
{
    SwNewUserIdxDlg* pDlg = new SwNewUserIdxDlg(this);
    if(RET_OK == pDlg->Execute())
    {
        String sNewName(pDlg->GetName());
        aTypeDCB.InsertEntry(sNewName);
        aTypeDCB.SelectEntry(sNewName);
    }
    delete pDlg;
    return 0;
}

IMPL_LINK( SwIndexMarkDlg, SearchTypeHdl, CheckBox*, pBox)
{
    sal_Bool bEnable = pBox->IsChecked() && pBox->IsEnabled();
    aSearchCaseWordOnlyCB.Enable(bEnable);
    aSearchCaseSensitiveCB.Enable(bEnable);
    return 0;
}

IMPL_LINK( SwIndexMarkDlg, InsertHdl, Button *, pButton )
{
    Apply();
    //close the dialog if only one entry is available
    if(!bNewMark && !aPrevBT.IsVisible() && !aNextBT.IsVisible())
        CloseHdl(pButton);
    return 0;
}

IMPL_LINK_NOARG(SwIndexMarkDlg, CloseHdl)
{
    if(bNewMark)
    {
        sal_uInt16 nSlot = FN_INSERT_IDX_ENTRY_DLG;
        SfxViewFrame::Current()->GetDispatcher()->Execute(nSlot,
                    SFX_CALLMODE_ASYNCHRON|SFX_CALLMODE_RECORD);
    }
    else
    {
        ((SwIndexMarkModalDlg*)GetParent())->EndDialog(RET_CANCEL);
    }
    return 0;
}

/*--------------------------------------------------------------------
     Description:   select index type only when inserting
 --------------------------------------------------------------------*/
IMPL_LINK( SwIndexMarkDlg, ModifyHdl, ListBox *, pBox )
{
    if(&aTypeDCB == pBox)
    {
        // set index type
        sal_uInt16 nPos = aTypeDCB.GetEntryPos(aTypeDCB.GetSelectEntry());
        sal_Bool bLevelEnable = sal_False,
             bKeyEnable   = sal_False,
             bSetKey2     = sal_False,
             bKey2Enable  = sal_False,
            bEntryHasText   = sal_False,
            bKey1HasText    = sal_False,
            bKey2HasText    = sal_False;
        if(nPos == POS_INDEX)
        {
            if(aEntryED.GetText().Len())
                bEntryHasText = sal_True;
            aPhoneticED0.SetText(GetDefaultPhoneticReading(aEntryED.GetText()));

            bKeyEnable = sal_True;
            aKeyDCB.SetText(aKeyDCB.GetEntry(nKey1Pos));
            aPhoneticED1.SetText(GetDefaultPhoneticReading(aKeyDCB.GetText()));
            if(aKeyDCB.GetText().Len() > 0)
            {
                bKey1HasText = bSetKey2 = bKey2Enable = sal_True;
                aKey2DCB.SetText(aKey2DCB.GetEntry(nKey2Pos));
                aPhoneticED2.SetText(GetDefaultPhoneticReading(aKey2DCB.GetText()));
                if(aKey2DCB.GetText().Len())
                    bKey2HasText = sal_True;
            }
        }
        else
        {
            bLevelEnable = sal_True;
            aLevelED.SetMax(MAXLEVEL);
            aLevelED.SetValue(aLevelED.Normalize(0));
            bSetKey2 = sal_True;
        }
        aLevelFT.Show(bLevelEnable);
        aLevelED.Show(bLevelEnable);
        aMainEntryCB.Show(nPos == POS_INDEX);

        aKeyFT.Enable(bKeyEnable);
        aKeyDCB.Enable(bKeyEnable);
        if ( bSetKey2 )
        {
            aKey2DCB.Enable(bKey2Enable);
            aKey2FT.Enable(bKey2Enable);
        }
        aPhoneticFT0.Enable(bKeyEnable&&bEntryHasText&&bIsPhoneticReadingEnabled);
        aPhoneticED0.Enable(bKeyEnable&&bEntryHasText&&bIsPhoneticReadingEnabled);
        aPhoneticFT1.Enable(bKeyEnable&&bKey1HasText&&bIsPhoneticReadingEnabled);
        aPhoneticED1.Enable(bKeyEnable&&bKey1HasText&&bIsPhoneticReadingEnabled);
        aPhoneticFT2.Enable(bKeyEnable&&bKey2HasText&&bIsPhoneticReadingEnabled);
        aPhoneticED2.Enable(bKeyEnable&&bKey2HasText&&bIsPhoneticReadingEnabled);
    }
    else //aEntryED  !!aEntryED is not a ListBox but a Edit
    {
        sal_Bool bHasText = (aEntryED.GetText().Len()>0);
        if(!bHasText)
        {
            aPhoneticED0.SetText(aEmptyStr);
            bPhoneticED0_ChangedByUser = sal_False;
        }
        else if(!bPhoneticED0_ChangedByUser)
            aPhoneticED0.SetText(GetDefaultPhoneticReading(aEntryED.GetText()));

        aPhoneticFT0.Enable(bHasText&&bIsPhoneticReadingEnabled);
        aPhoneticED0.Enable(bHasText&&bIsPhoneticReadingEnabled);
    }
    aOKBT.Enable(!pSh->HasReadonlySel() &&
        (aEntryED.GetText().Len() || pSh->GetCrsrCnt(sal_False)));
    return 0;
}

IMPL_LINK_NOARG_INLINE_START(SwIndexMarkDlg, NextHdl)
{
    InsertUpdate();
    pTOXMgr->NextTOXMark();
    UpdateDialog();
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwIndexMarkDlg, NextHdl)

IMPL_LINK_NOARG_INLINE_START(SwIndexMarkDlg, NextSameHdl)
{
    InsertUpdate();
    pTOXMgr->NextTOXMark(sal_True);
    UpdateDialog();
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwIndexMarkDlg, NextSameHdl)

IMPL_LINK_NOARG_INLINE_START(SwIndexMarkDlg, PrevHdl)
{
    InsertUpdate();
    pTOXMgr->PrevTOXMark();
    UpdateDialog();
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwIndexMarkDlg, PrevHdl)

IMPL_LINK_NOARG_INLINE_START(SwIndexMarkDlg, PrevSameHdl)
{
    InsertUpdate();
    pTOXMgr->PrevTOXMark(sal_True);
    UpdateDialog();

    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwIndexMarkDlg, PrevSameHdl)

IMPL_LINK_NOARG(SwIndexMarkDlg, DelHdl)
{
    bDel = sal_True;
    InsertUpdate();
    bDel = sal_False;

    if(pTOXMgr->GetCurTOXMark())
        UpdateDialog();
    else
    {
        CloseHdl(&aCancelBT);
        SfxViewFrame::Current()->GetBindings().Invalidate(FN_EDIT_IDX_ENTRY_DLG);
    }
    return 0;
}

/*--------------------------------------------------------------------
     Description: renew dialog view
 --------------------------------------------------------------------*/
void SwIndexMarkDlg::UpdateDialog()
{
    OSL_ENSURE(pSh && pTOXMgr, "no shell?");
    SwTOXMark* pMark = pTOXMgr->GetCurTOXMark();
    OSL_ENSURE(pMark, "no current marker");
    if(!pMark)
        return;

    ViewShell::SetCareWin( GetParent() );

    aOrgStr = pMark->GetText();
    aEntryED.SetText(aOrgStr);

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
        aKeyDCB.SetText( pMark->GetPrimaryKey() );
        aKey2DCB.SetText( pMark->GetSecondaryKey() );
        aPhoneticED0.SetText( pMark->GetTextReading() );
        aPhoneticED1.SetText( pMark->GetPrimaryKeyReading() );
        aPhoneticED2.SetText( pMark->GetSecondaryKeyReading() );
        aMainEntryCB.Check(pMark->IsMainEntry());
    }
    else if(TOX_CONTENT == eCurType || TOX_USER == eCurType)
    {
        aLevelED.SetValue(aLevelED.Normalize(pMark->GetLevel()));
    }
    aKeyFT.Enable(bKeyEnable);
    aKeyDCB.Enable(bKeyEnable);
    aLevelED.SetMax(MAXLEVEL);
    aLevelFT.Show(bLevelEnable);
    aLevelED.Show(bLevelEnable);
    aMainEntryCB.Show(!bLevelEnable);
    aKey2FT.Enable(bKey2Enable);
    aKey2DCB.Enable(bKey2Enable);

    UpdateLanguageDependenciesForPhoneticReading();
    aPhoneticFT0.Enable(bKeyEnable&&bEntryHasText&&bIsPhoneticReadingEnabled);
    aPhoneticED0.Enable(bKeyEnable&&bEntryHasText&&bIsPhoneticReadingEnabled);
    aPhoneticFT1.Enable(bKeyEnable&&bKey1HasText&&bIsPhoneticReadingEnabled);
    aPhoneticED1.Enable(bKeyEnable&&bKey1HasText&&bIsPhoneticReadingEnabled);
    aPhoneticFT2.Enable(bKeyEnable&&bKey2HasText&&bIsPhoneticReadingEnabled);
    aPhoneticED2.Enable(bKeyEnable&&bKey2HasText&&bIsPhoneticReadingEnabled);

    // set index type
    aTypeDCB.SelectEntry(pMark->GetTOXType()->GetTypeName());

    // set Next - Prev - Buttons
    pSh->SttCrsrMove();
    if( aPrevBT.IsVisible() )
    {
        const SwTOXMark* pMoveMark = &pSh->GotoTOXMark( *pMark, TOX_PRV );
        if( pMoveMark != pMark )
            pSh->GotoTOXMark( *pMoveMark, TOX_NXT );
        aPrevBT.Enable( pMoveMark != pMark );
        pMoveMark = &pSh->GotoTOXMark( *pMark, TOX_NXT );
        if( pMoveMark != pMark )
            pSh->GotoTOXMark( *pMoveMark, TOX_PRV );
        aNextBT.Enable( pMoveMark != pMark );
    }

    if( aPrevSameBT.IsVisible() )
    {
        const SwTOXMark* pMoveMark = &pSh->GotoTOXMark( *pMark, TOX_SAME_PRV );
        if( pMoveMark != pMark )
            pSh->GotoTOXMark( *pMoveMark, TOX_SAME_NXT );
        aPrevSameBT.Enable( pMoveMark != pMark );
        pMoveMark = &pSh->GotoTOXMark( *pMark, TOX_SAME_NXT );
        if( pMoveMark != pMark )
            pSh->GotoTOXMark( *pMoveMark, TOX_SAME_PRV );
        aNextSameBT.Enable( pMoveMark != pMark );
    }

    sal_Bool bEnable = !pSh->HasReadonlySel();
    aOKBT.Enable( bEnable );
    aDelBT.Enable( bEnable );
    aEntryED.SetReadOnly( !bEnable );
    aLevelED.SetReadOnly( !bEnable );
    aKeyDCB.SetReadOnly( !bEnable );
    aKey2DCB.SetReadOnly( !bEnable );

    pSh->SelectTxtAttr( RES_TXTATR_TOXMARK, pMark->GetTxtTOXMark() );
    // we need the point at the start of the attribut
    pSh->SwapPam();

    pSh->EndCrsrMove();
}

/*--------------------------------------------------------------------
     Remind whether the edit boxes for Phonetic reading are changed manually
 --------------------------------------------------------------------*/
IMPL_LINK( SwIndexMarkDlg, PhoneticEDModifyHdl, Edit *, pEdit )
{
    if(&aPhoneticED0 == pEdit)
    {
        bPhoneticED0_ChangedByUser = pEdit->GetText().Len()>0;
    }
    else if(&aPhoneticED1 == pEdit)
    {
        bPhoneticED1_ChangedByUser = pEdit->GetText().Len()>0;
    }
    else if(&aPhoneticED2 == pEdit)
    {
        bPhoneticED2_ChangedByUser = pEdit->GetText().Len()>0;
    }
    return 0;
}

/*--------------------------------------------------------------------
     Description: Enable Disable of the 2nd key
 --------------------------------------------------------------------*/
IMPL_LINK( SwIndexMarkDlg, KeyDCBModifyHdl, ComboBox *, pBox )
{
    if(&aKeyDCB == pBox)
    {
        sal_Bool bEnable = pBox->GetText().Len() > 0;
        if(!bEnable)
        {
            aKey2DCB.SetText(aEmptyStr);
            aPhoneticED1.SetText(aEmptyStr);
            aPhoneticED2.SetText(aEmptyStr);
            bPhoneticED1_ChangedByUser = sal_False;
            bPhoneticED2_ChangedByUser = sal_False;
        }
        else
        {
            if(pBox->IsInDropDown())
            {
                //reset bPhoneticED1_ChangedByUser if a completly new string is selected
                bPhoneticED1_ChangedByUser = sal_False;
            }
            if(!bPhoneticED1_ChangedByUser)
                aPhoneticED1.SetText(GetDefaultPhoneticReading(pBox->GetText()));
        }
        aKey2DCB.Enable(bEnable);
        aKey2FT.Enable(bEnable);
    }
    else if(&aKey2DCB == pBox)
    {
        if(!(pBox->GetText().Len()>0))
        {
            aPhoneticED2.SetText(aEmptyStr);
            bPhoneticED2_ChangedByUser = sal_False;
        }
        else
        {
            if(pBox->IsInDropDown())
            {
                //reset bPhoneticED1_ChangedByUser if a completly new string is selected
                bPhoneticED2_ChangedByUser = sal_False;
            }
            if(!bPhoneticED2_ChangedByUser)
                aPhoneticED2.SetText(GetDefaultPhoneticReading(pBox->GetText()));
        }
    }
    sal_Bool    bKey1HasText    = (0 != aKeyDCB.GetText().Len());
    sal_Bool    bKey2HasText    = (0 != aKey2DCB.GetText().Len());

    aPhoneticFT1.Enable(bKey1HasText&&bIsPhoneticReadingEnabled);
    aPhoneticED1.Enable(bKey1HasText&bIsPhoneticReadingEnabled);
    aPhoneticFT2.Enable(bKey2HasText&bIsPhoneticReadingEnabled);
    aPhoneticED2.Enable(bKey2HasText&bIsPhoneticReadingEnabled);

    return 0;
}

/*--------------------------------------------------
 overload dtor
--------------------------------------------------*/
SwIndexMarkDlg::~SwIndexMarkDlg()
{
    delete pTOXMgr;
    ViewShell::SetCareWin( 0 );
}

void    SwIndexMarkDlg::ReInitDlg(SwWrtShell& rWrtShell, SwTOXMark* pCurTOXMark)
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
                                   SfxChildWindow* pChild,
                                   Window *pParent,
                                SfxChildWinInfo* pInfo,
                                   sal_Bool bNew) :
SfxModelessDialog(_pBindings, pChild, pParent, SvtCJKOptions().IsCJKFontEnabled()?SW_RES(DLG_INSIDXMARK_CJK):SW_RES(DLG_INSIDXMARK)),
    aDlg(this, bNew, SW_RES(WIN_DLG), SvtCJKOptions().IsCJKFontEnabled()?DLG_INSIDXMARK_CJK:DLG_INSIDXMARK, *::GetActiveWrtShell())
{
    FreeResource();
    aDlg.ReInitDlg(*::GetActiveWrtShell());
    Initialize(pInfo);
}

void    SwIndexMarkFloatDlg::Activate()
{
    SfxModelessDialog::Activate();
    aDlg.Activate();
}

void SwIndexMarkFloatDlg::ReInitDlg(SwWrtShell& rWrtShell)
{
    aDlg.ReInitDlg( rWrtShell );
}

SwIndexMarkModalDlg::SwIndexMarkModalDlg(Window *pParent, SwWrtShell& rSh, SwTOXMark* pCurTOXMark) :
SvxStandardDialog(pParent, SvtCJKOptions().IsCJKFontEnabled()?SW_RES(DLG_EDIT_IDXMARK_CJK):SW_RES(DLG_EDIT_IDXMARK)),
    aDlg(this, sal_False, SW_RES(WIN_DLG), SvtCJKOptions().IsCJKFontEnabled()?DLG_EDIT_IDXMARK_CJK:DLG_EDIT_IDXMARK, rSh)
{
    FreeResource();
    aDlg.ReInitDlg(rSh, pCurTOXMark);
}

void    SwIndexMarkModalDlg::Apply()
{
    aDlg.Apply();
}

class SwCreateAuthEntryDlg_Impl : public ModalDialog
{
    FixedLine       aEntriesFL;

    FixedText*      pFixedTexts[AUTH_FIELD_END];
    ListBox*        pTypeListBox;
    ComboBox*       pIdentifierBox;
    Edit*           pEdits[AUTH_FIELD_END];

    OKButton        aOKBT;
    CancelButton    aCancelBT;
    HelpButton      aHelpBT;

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
                            sal_Bool bCreate);
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
                            pEntry->GetAuthorField((ToxAuthorityField)i) : aEmptyStr;
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
    sal_Bool bCreate = pButton == m_pCreateEntryPB;
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
             uno::Reference< lang::XMultiServiceFactory > xMSF = getProcessServiceFactory();
            xBibAccess = uno::Reference< container::XNameAccess > (
                            xMSF->createInstance( C2U("com.sun.star.frame.Bibliography") ),
                                                                        uno::UNO_QUERY );
            uno::Reference< beans::XPropertySet >  xPropSet(xBibAccess, uno::UNO_QUERY);
            OUString uPropName(C2U("BibliographyDataFieldNames"));
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
            std::vector<String> aIds;
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
        sal_Bool bCreate) :
    ModalDialog(pParent, SW_RES(DLG_CREATE_AUTH_ENTRY)),
    aEntriesFL(this,    SW_RES(FL_ENTRIES    )),
    pTypeListBox(0),
    pIdentifierBox(0),
    aOKBT(this,         SW_RES(PB_OK         )),
    aCancelBT(this,     SW_RES(PB_CANCEL        )),
    aHelpBT(this,       SW_RES(PB_HELP      )),
    rWrtSh(rSh),
    m_bNewEntryMode(bNewEntry),
    m_bNameAllowed(sal_True)
{
    FreeResource();
    Point aFLPos(aEntriesFL.GetPosPixel());
    Point aTL1(aFLPos);
    Size aFLSz(aEntriesFL.GetSizePixel().Width(), GetSizePixel().Height());
    long nControlSpace = aFLSz.Width() / 4;
    long nControlWidth = nControlSpace - 2 * aTL1.X();
    aTL1.X() *= 2;
    aTL1.Y() *= 5;
    Point aTR1(aTL1);
    aTR1.X() += nControlSpace;
    Point aTL2(aTR1);
    aTL2.X() += nControlSpace;
    Point aTR2(aTL2);
    aTR2.X() += nControlSpace;
    Size aFixedTextSize(aFLSz);
    Size aTmpSz(8,10);
    aTmpSz = LogicToPixel(aTmpSz, MAP_APPFONT);
    aFixedTextSize.Height() = aTmpSz.Width();
    Size aEditSize(aFixedTextSize);
    aFixedTextSize.Width() = nControlWidth + aFLPos.X();
    aEditSize.Height() = aTmpSz.Height();
    aEditSize.Width() = nControlWidth;

    sal_uInt16 nOffset = static_cast< sal_uInt16 >(aTmpSz.Width() * 3 / 2);
    sal_Bool bLeft = sal_True;
    Window* pRefWindow = 0;
    for(sal_uInt16 nIndex = 0; nIndex < AUTH_FIELD_END; nIndex++)
    {
        const TextInfo aCurInfo = aTextInfoArr[nIndex];

        pFixedTexts[nIndex] = new FixedText(this);
        if(nIndex)
            pFixedTexts[nIndex]->SetZOrder( pRefWindow, WINDOW_ZORDER_BEHIND );
        else
            pFixedTexts[nIndex]->SetZOrder( 0, WINDOW_ZORDER_FIRST );

        pRefWindow = pFixedTexts[nIndex];

        pFixedTexts[nIndex]->SetSizePixel(aFixedTextSize);
        pFixedTexts[nIndex]->SetPosPixel(bLeft ? aTL1 : aTL2);
        pFixedTexts[nIndex]->SetText(SW_RES(STR_AUTH_FIELD_START + aCurInfo.nToxField));
        pFixedTexts[nIndex]->Show();
        pEdits[nIndex] = 0;
        if( AUTH_FIELD_AUTHORITY_TYPE == aCurInfo.nToxField )
        {
            pTypeListBox = new ListBox(this, WB_DROPDOWN|WB_BORDER);
            pTypeListBox->SetZOrder( pRefWindow, WINDOW_ZORDER_BEHIND );
            pRefWindow = pTypeListBox;
            for(sal_uInt16 j = 0; j < AUTH_TYPE_END; j++)
                pTypeListBox->InsertEntry(String(SW_RES(STR_AUTH_TYPE_START + j)));
            if(pFields[aCurInfo.nToxField].Len())
            {
                sal_uInt16 nIndexPos = static_cast< sal_uInt16 >(pFields[aCurInfo.nToxField].ToInt32());
                pTypeListBox->SelectEntryPos(nIndexPos);
            }
            Size aTmp(aEditSize);
            aTmp.Height() *= 4;
            pTypeListBox->SetSizePixel(aTmp);
            pTypeListBox->SetPosPixel(bLeft ? aTR1 : aTR2);
            pTypeListBox->Show();
            pTypeListBox->SetSelectHdl(LINK(this, SwCreateAuthEntryDlg_Impl, EnableHdl));
            pTypeListBox->SetHelpId(aCurInfo.pHelpId);

        }
        else if(AUTH_FIELD_IDENTIFIER == aCurInfo.nToxField && !m_bNewEntryMode)
        {
            pIdentifierBox = new ComboBox(this, WB_BORDER|WB_DROPDOWN);
            pIdentifierBox->SetZOrder( pRefWindow, WINDOW_ZORDER_BEHIND );
            pRefWindow = pIdentifierBox;

            pIdentifierBox->SetSelectHdl(LINK(this,
                                    SwCreateAuthEntryDlg_Impl, IdentifierHdl));


            const SwAuthorityFieldType* pFType = (const SwAuthorityFieldType*)
                                        rSh.GetFldType(RES_AUTHORITY, aEmptyStr);
            if(pFType)
            {
                std::vector<String> aIds;
                pFType->GetAllEntryIdentifiers( aIds );
                for(size_t n = 0; n < aIds.size(); ++n)
                    pIdentifierBox->InsertEntry(aIds[n]);
            }
            pIdentifierBox->SetText(pFields[aCurInfo.nToxField]);
            Size aTmp(aEditSize);
            aTmp.Height() *= 4;
            pIdentifierBox->SetSizePixel(aTmp);
            pIdentifierBox->SetPosPixel(bLeft ? aTR1 : aTR2);
            pIdentifierBox->Show();
            pIdentifierBox->SetHelpId(aCurInfo.pHelpId);
        }
        else
        {
            pEdits[nIndex] = new Edit(this, WB_BORDER);
            pEdits[nIndex]->SetZOrder( pRefWindow, WINDOW_ZORDER_BEHIND );
            pRefWindow = pEdits[nIndex];
            pEdits[nIndex]->SetSizePixel(aEditSize);
            pEdits[nIndex]->SetPosPixel(bLeft ? aTR1 : aTR2);
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
        }
        if(bLeft)
        {
            aTL1.Y() += nOffset;
            aTR1.Y() += nOffset;
        }
        else
        {
            aTL2.Y() += nOffset;
            aTR2.Y() += nOffset;
        }
        bLeft = !bLeft;
    }
    EnableHdl(pTypeListBox);

    long nHeightDiff = - aFLSz.Height();
    aFLSz.Height() = aTL1.Y();
    nHeightDiff += aFLSz.Height();
    Size aDlgSize(GetSizePixel());
    aDlgSize.Height() += nHeightDiff;
    SetSizePixel(aDlgSize);

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
        sRet = String::CreateFromInt32(pTypeListBox->GetSelectEntryPos());
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
        aOKBT.Enable(pTypeListBox->GetSelectEntryCount() && bEnable);
    }
    return 0;
}

IMPL_LINK(SwCreateAuthEntryDlg_Impl, EnableHdl, ListBox*, pBox)
{
    aOKBT.Enable(m_bNameAllowed && pBox->GetSelectEntryCount());
    return 0;
};

SwAuthMarkFloatDlg::SwAuthMarkFloatDlg(SfxBindings* _pBindings,
                                   SfxChildWindow* pChild,
                                   Window *pParent,
                                SfxChildWinInfo* pInfo,
                                   sal_Bool bNew)
    : SfxModelessDialog(_pBindings, pChild, pParent,
        "BibliographyEntyDialog", "modules/swriter/ui/bibliographyentry.ui")
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
    : SvxStandardDialog(pParent, "BibliographyEntyDialog",
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
