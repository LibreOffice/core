/*************************************************************************
 *
 *  $RCSfile: idxmrk.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: jp $ $Date: 2000-11-20 09:18:54 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#include "helpid.h"
#define _SVSTDARR_STRINGSSORT
#include <svtools/svstdarr.hxx>

#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _TXTCMP_HXX //autogen
#include <svtools/txtcmp.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _IDXMRK_HXX
#include <idxmrk.hxx>
#endif
#ifndef _TXTTXMRK_HXX
#include <txttxmrk.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _TOXMGR_HXX
#include <toxmgr.hxx>
#endif
#ifndef _MULTMRK_HXX
#include <multmrk.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>                   // fuer Undo-Ids
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _INDEX_HRC
#include <index.hrc>
#endif
#ifndef _IDXMRK_HRC
#include <idxmrk.hrc>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _FLDMGR_HXX
#include <fldmgr.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#include <utlui.hrc>
#ifndef _SWCONT_HXX
#include <swcont.hxx>
#endif
#ifndef _AUTHFLD_HXX
#include <authfld.hxx>
#endif

#define POS_CONTENT 0
#define POS_INDEX   1
#define POS_USER    2

static sal_uInt16 nTypePos = 1; // TOX_INDEX as standard
static sal_uInt16 nKey1Pos = USHRT_MAX;

static sal_uInt16 nKey2Pos = USHRT_MAX;

using namespace ::rtl;
using namespace ::comphelper;
using namespace ::com::sun::star;
#define C2U(cChar) OUString::createFromAscii(cChar)
/*--------------------------------------------------------------------
     Beschreibung:  Dialog zum Einfuegen einer Verzeichnismarkierung
 --------------------------------------------------------------------*/
SwIndexMarkDlg::SwIndexMarkDlg(Window *pParent,
                               sal_Bool bNewDlg,
                               const ResId& rResId ) :
      Window(pParent, rResId),
    bDel(sal_False),
    bNewMark(bNewDlg),
    pTOXMgr(0),
    pSh(0),
    aOKBT(this,     SW_RES(BT_OK    )),
    aCancelBT(this, SW_RES(BT_CANCEL )),
    aHelpBT(this,   SW_RES(BT_HELP   )),
    aDelBT(this,    SW_RES(BT_DEL   )),
    aNewBT(this,    SW_RES(BT_NEW   )),
    aPrevBT(this,   SW_RES(BT_PREV  )),
    aPrevSameBT(this,SW_RES(BT_PREVSAME)),
    aNextBT(this,   SW_RES(BT_NXT   )),
    aNextSameBT(this,SW_RES(BT_NXTSAME)),
    aTypeFT (this,  SW_RES(LBL_INDEX    )),
    aTypeDCB(this,  SW_RES(DCB_INDEX    )),
    aEntryFT(this,  SW_RES(LBL_ENTRY    )),
    aEntryED(this,  SW_RES(SL_ENTRY )),
    aKeyFT(this,    SW_RES(LBL_KEY  )),
    aKeyDCB(this,   SW_RES(DCB_KEY  )),
    aKey2FT(this,   SW_RES(LBL_KEY2 )),
    aKey2DCB(this,  SW_RES(DCB_KEY2 )),
    aLevelFT(this,  SW_RES(LBL_LEVEL    )),
    aLevelED(this,  SW_RES(SL_LEVEL )),
    aMainEntryCB(this, SW_RES(CB_MAIN_ENTRY )),
    aApplyToAllCB(this,SW_RES(CB_APPLY_TO_ALL)),
    aSearchCaseSensitiveCB(this,    SW_RES(CB_CASESENSITIVE )),
    aSearchCaseWordOnlyCB(this,     SW_RES(CB_WORDONLY      )),
    aIndexGB(this,  SW_RES(GB_INDEX )),
    bSelected(sal_False)
{
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
    aSearchCaseSensitiveCB.SetHelpId(   HID_INSERT_IDX_MRK_SRCH_CASESENSITIVE   );
    aSearchCaseWordOnlyCB.SetHelpId(    HID_INSERT_IDX_MRK_SRCH_WORDONLY        );


    GetParent()->SetText( SW_RESSTR( bNewMark ? STR_IDXMRK_INSERT : STR_IDXMRK_EDIT));

    aDelBT.SetClickHdl(LINK(this,SwIndexMarkDlg,        DelHdl));
    aPrevBT.SetClickHdl(LINK(this,SwIndexMarkDlg,       PrevHdl));
    aPrevSameBT.SetClickHdl(LINK(this,SwIndexMarkDlg,   PrevSameHdl));
    aNextBT.SetClickHdl(LINK(this,SwIndexMarkDlg,       NextHdl));
    aNextSameBT.SetClickHdl(LINK(this,SwIndexMarkDlg,   NextSameHdl));
    //aTypeDCB.SetModifyHdl(LINK(this,SwIndexMarkDlg,   ModifyHdl));
    aTypeDCB.SetSelectHdl(LINK(this,SwIndexMarkDlg,     ModifyHdl));
    aKeyDCB.SetModifyHdl(LINK(this,SwIndexMarkDlg,      KeyDCBModifyHdl));
    aOKBT.SetClickHdl(LINK(this,SwIndexMarkDlg,         InsertHdl));
    aCancelBT.SetClickHdl(LINK(this,SwIndexMarkDlg,     CloseHdl));
    aEntryED.SetModifyHdl(LINK(this,SwIndexMarkDlg,     ModifyHdl));
    aNewBT.SetClickHdl(LINK(this, SwIndexMarkDlg,       NewUserIdxHdl));
    aApplyToAllCB.SetClickHdl(LINK(this, SwIndexMarkDlg, SearchTypeHdl));

    if(bNewMark)
    {
        aDelBT.Hide();
        ImageList aTempList(SW_RES(IMG_NAVI_ENTRYBMP));
        aNewBT.SetImage(aTempList.GetImage(SID_SW_START + CONTENT_TYPE_INDEX));
    }
    else
    {
        aNewBT.Hide();
        OKButton aTmp(this, WB_HIDE);
        aOKBT.SetText( aTmp.GetText() );
    }
}

/*--------------------------------------------------------------------
     Beschreibung:  Controls neu initialisieren mit der aktuellen
                    Markierung
 --------------------------------------------------------------------*/


void SwIndexMarkDlg::InitControls()
{
    DBG_ASSERT(pSh && pTOXMgr, "Shell nicht da?")
    // Inhalts-Verzeichnis
    const SwTOXType* pType = pTOXMgr->GetTOXType(TOX_CONTENT, 0);
    ASSERT(pType, "Kein Verzeichnistyp !!");
    String sTmpTypeSelection;
    if(aTypeDCB.GetSelectEntryCount())
        sTmpTypeSelection = aTypeDCB.GetSelectEntry();
    aTypeDCB.Clear();
    aTypeDCB.InsertEntry(pType->GetTypeName());

    // Stichwort-Verzeichnis
    pType = pTOXMgr->GetTOXType(TOX_INDEX, 0);
    ASSERT(pType, "Kein Verzeichnistyp !!");
    aTypeDCB.InsertEntry(pType->GetTypeName());

    // Benutzerverzeichnisse
    sal_uInt16 nCount = pSh->GetTOXTypeCount(TOX_USER);
    for(sal_uInt16 i=0; i< nCount; ++i)
        aTypeDCB.InsertEntry( pSh->GetTOXType(TOX_USER, i)->GetTypeName() );

    // KeyWords Primary einlesen
    SvStringsSort aArr;
    nCount = pSh->GetTOIKeys( TOI_PRIMARY, aArr );
    for(i=0; i < nCount; ++i)
        aKeyDCB.InsertEntry( *aArr[ i ] );

    // KeyWords Secondary einlesen
    nCount = pSh->GetTOIKeys( TOI_SECONDARY, aArr );
    for(i=0; i < nCount; ++i)
        aKey2DCB.InsertEntry( *aArr[ i ] );

    // Aktueller Eintrag
    const SwTOXMark* pMark = pTOXMgr->GetCurTOXMark();
    if( pMark && !bNewMark)
    {
        // Controls-Handling

        // nur wenn mehr als eins vorhanden
        // wenn gleich landet es auf dem gleichen Eintrag
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
    {   // aktuelle Selektion (1.Element) anzeigen  ????
        sal_uInt16 nCnt = pSh->GetCrsrCnt();
        if (nCnt < 2)
        {
            bSelected = !pSh->HasSelection();
            aOrgStr = pSh->GetView().GetSelectionTextParam(sal_True, sal_False);
            aEntryED.SetText(aOrgStr);

            //alle gleichen Eintraege aufzunehmen darf nur im Body und auch da nur
            //bei vorhandener einfacher Selektion erlaubt werden
            const sal_uInt16 nFrmType = pSh->GetFrmType(0,sal_True);
            aApplyToAllCB.Show();
            aSearchCaseSensitiveCB.Show();
            aSearchCaseWordOnlyCB.Show();
            aApplyToAllCB.Enable(0 != aOrgStr.Len() &&
                0 == (nFrmType & ( FRMTYPE_HEADER | FRMTYPE_FOOTER | FRMTYPE_FLY_ANY )));
            SearchTypeHdl(&aApplyToAllCB);
        }

        // Verzeichnistyp ist default
        if( sTmpTypeSelection.Len() &&
            LISTBOX_ENTRY_NOTFOUND != aTypeDCB.GetEntryPos( sTmpTypeSelection ) )
            aTypeDCB.SelectEntry(sTmpTypeSelection);
        else
            aTypeDCB.SelectEntry(aTypeDCB.GetEntry(nTypePos));
        ModifyHdl(&aTypeDCB);
    }
}
/* -----------------07.09.99 10:43-------------------
    Change the content of aEntryED if text is selected
 --------------------------------------------------*/

void    SwIndexMarkDlg::Activate()
{
    // aktuelle Selektion (1.Element) anzeigen  ????
    if(bNewMark)
    {
        sal_uInt16 nCnt = pSh->GetCrsrCnt();
        if (nCnt < 2)
        {
            bSelected = !pSh->HasSelection();
            aOrgStr = pSh->GetView().GetSelectionTextParam(sal_True, sal_False);
            aEntryED.SetText(aOrgStr);

            //alle gleichen Eintraege aufzunehmen darf nur im Body und auch da nur
            //bei vorhandener einfacher Selektion erlaubt werden
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
     Beschreibung:  Ok-Button auswerten
 --------------------------------------------------------------------*/
void SwIndexMarkDlg::Apply()
{
    InsertUpdate();
    if(bSelected)
        pSh->ResetSelect(0, sal_False);
}

/*--------------------------------------------------------------------
    Beschreibung: Aenderungen uebernehmen
 --------------------------------------------------------------------*/


void SwIndexMarkDlg::InsertUpdate()
{
    pSh->StartUndo(UNDO_INSATTR);
    pSh->StartAllAction();

    if( bNewMark )
        InsertMark();
    else if( !pSh->HasReadonlySel() )
    {
        if( bDel )
            pTOXMgr->DeleteTOXMark();
        else if( pTOXMgr->GetCurTOXMark() )
            UpdateMark();
    }

    pSh->EndAllAction();
    pSh->EndUndo(UNDO_INSATTR);

    if((nTypePos = aTypeDCB.GetEntryPos(aTypeDCB.GetSelectEntry())) == LISTBOX_ENTRY_NOTFOUND)
        nTypePos = 0;

    nKey1Pos = aKeyDCB.GetEntryPos(aKeyDCB.GetText());
    nKey2Pos = aKey2DCB.GetEntryPos(aKey2DCB.GetText());
}

/*--------------------------------------------------------------------
     Beschreibung:  Marke einfuegen
 --------------------------------------------------------------------*/
void lcl_SelectSameStrings(SwWrtShell& rSh, BOOL bWordOnly, BOOL bCaseSensitive)
{
    rSh.Push();
    utl::SearchParam aParam( rSh.GetSelTxt(), utl::SearchParam::SRCH_NORMAL,
                                                TRUE, FALSE, FALSE );

    aParam.SetCaseSensitive( bCaseSensitive );
    aParam.SetSrchWordOnly( bWordOnly );

    rSh.ClearMark();
    ULONG nRet = rSh.Find( aParam,  DOCPOS_START, DOCPOS_END,
                        (FindRanges)(FND_IN_SELALL|FND_IN_BODYONLY), FALSE );
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
        case POS_INDEX:     // Stichwortverzeichnismarke
        {
            UpdateKeyBoxes();
            String  aPrim(aKeyDCB.GetText());
            String  aSec(aKey2DCB.GetText());
            aDesc.SetPrimKey(aPrim);
            aDesc.SetSecKey(aSec);
            aDesc.SetMainEntry(aMainEntryCB.IsChecked());
        }
        break;
        default:            // Userdefinedverz.-Marke
        {
            String aName(aTypeDCB.GetSelectEntry());
            aDesc.SetTOUName(aName);
        }
    }
    if (aOrgStr != aEntryED.GetText())
        aDesc.SetAltStr(aEntryED.GetText());
    BOOL bApplyAll = aApplyToAllCB.IsChecked();
    BOOL bWordOnly = aApplyToAllCB.IsChecked();
    BOOL bCaseSensitive = aSearchCaseSensitiveCB.IsChecked();

    pSh->StartAllAction();
    // hier muessen alle gleichen Strings selektiert werden
    // damit der Eintrag auf alle gleichen Strings angewandt wird
    if(bApplyAll)
    {
        lcl_SelectSameStrings(*pSh, bWordOnly, bCaseSensitive);
    }
    aDesc.SetLevel(nLevel);
    SwTOXMgr aMgr(pSh);
    aMgr.InsertTOXMark(aDesc);
    if(bApplyAll)
        pSh->Pop(FALSE);

    pSh->EndAllAction();
}

/*--------------------------------------------------------------------
     Beschreibung:  Marke Updaten
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
    aDesc.SetLevel(aLevelED.GetValue());
    if(pAltText)
        aDesc.SetAltStr(*pAltText);

    String  aPrim(aKeyDCB.GetText());
    if(aPrim.Len())
        aDesc.SetPrimKey(aPrim);
    String  aSec(aKey2DCB.GetText());
    if(aSec.Len())
        aDesc.SetSecKey(aSec);
    aDesc.SetMainEntry(aMainEntryCB.IsVisible() && aMainEntryCB.IsChecked());
    pTOXMgr->UpdateTOXMark(aDesc);
}

/*--------------------------------------------------------------------
    Beschreibung: Neue Keys eintragen
 --------------------------------------------------------------------*/


void SwIndexMarkDlg::UpdateKeyBoxes()
{
    String aKey(aKeyDCB.GetText());
    sal_uInt16 nPos = aKeyDCB.GetEntryPos(aKey);

    if(nPos == LISTBOX_ENTRY_NOTFOUND && aKey.Len() > 0)
    {   // neuen Key erzeugen
        aKeyDCB.InsertEntry(aKey);
    }

    aKey = aKey2DCB.GetText();
    nPos = aKey2DCB.GetEntryPos(aKey);

    if(nPos == LISTBOX_ENTRY_NOTFOUND && aKey.Len() > 0)
    {   // neuen Key erzeugen
        aKey2DCB.InsertEntry(aKey);
    }
}
/* -----------------13.10.99 15:10-------------------

 --------------------------------------------------*/
class SwNewUserIdxDlg : public ModalDialog
{
    OKButton        aOKPB;
    CancelButton    aCancelPB;
    HelpButton      aHelpPB;
    GroupBox        aNameGB;
    FixedText       aNameFT;
    Edit            aNameED;

    SwIndexMarkDlg* pDlg;

    DECL_LINK( ModifyHdl, Edit*);

    public:
        SwNewUserIdxDlg(SwIndexMarkDlg* pParent) :
            ModalDialog(pParent, SW_RES(DLG_NEW_USER_IDX)),
            pDlg(pParent),
            aOKPB(this, ResId(      PB_OK       )),
            aCancelPB(this, ResId(  PB_CANCEL   )),
            aHelpPB(this, ResId(    PB_HELP     )),
            aNameGB(this, ResId(    GB_NAME     )),
            aNameFT(this, ResId(    FT_NAME     )),
            aNameED(this, ResId(    ED_NAME     ))
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

IMPL_LINK( SwIndexMarkDlg, NewUserIdxHdl, Button*, pButton)
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
/* -----------------------------17.01.00 12:18--------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK( SwIndexMarkDlg, SearchTypeHdl, CheckBox*, pBox)
{
    sal_Bool bEnable = pBox->IsChecked() && pBox->IsEnabled();
    aSearchCaseWordOnlyCB.Enable(bEnable);
    aSearchCaseSensitiveCB.Enable(bEnable);
    return 0;
}
/* -----------------07.09.99 10:30-------------------

 --------------------------------------------------*/
IMPL_LINK( SwIndexMarkDlg, InsertHdl, Button *, pButton )
{
    Apply();
    //close the dialog if only one entry is available
    if(!bNewMark && !aPrevBT.IsVisible() && !aNextBT.IsVisible())
        CloseHdl(pButton);
    return 0;
}
/* -----------------07.09.99 10:29-------------------

 --------------------------------------------------*/
IMPL_LINK( SwIndexMarkDlg, CloseHdl, Button *, pButton )
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
     Beschreibung:  VerzeichnisTyp auswaehlen nur bei Einfuegen
 --------------------------------------------------------------------*/
IMPL_LINK( SwIndexMarkDlg, ModifyHdl, ListBox *, pBox )
{
    if(&aTypeDCB == pBox)
    {
        // Verzeichnistyp setzen
        sal_uInt16 nPos = aTypeDCB.GetEntryPos(aTypeDCB.GetSelectEntry());
        sal_Bool bLevelEnable = sal_False,
             bKeyEnable   = sal_False,
             bSetKey2     = sal_False,
             bKey2Enable  = sal_False;
        if(nPos == POS_INDEX)
        {
            bKeyEnable = sal_True;
            aKeyDCB.SetText(aKeyDCB.GetEntry(nKey1Pos));
            if(aKeyDCB.GetText().Len() > 0)
            {
                bSetKey2 = bKey2Enable = sal_True;
                aKey2DCB.SetText(aKey2DCB.GetEntry(nKey2Pos));
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
    }
    aOKBT.Enable(aEntryED.GetText().Len() || pSh->GetCrsrCnt(sal_False));
    return 0;
}

/*--------------------------------------------------------------------
     Beschreibung:
 --------------------------------------------------------------------*/


IMPL_LINK_INLINE_START( SwIndexMarkDlg, NextHdl, Button *, EMPTYARG )
{
    InsertUpdate();
    pTOXMgr->NextTOXMark();
    UpdateDialog();
    return 0;
}
IMPL_LINK_INLINE_END( SwIndexMarkDlg, NextHdl, Button *, EMPTYARG )


IMPL_LINK_INLINE_START( SwIndexMarkDlg, NextSameHdl, Button *, EMPTYARG )
{
    InsertUpdate();
    pTOXMgr->NextTOXMark(sal_True);
    UpdateDialog();
    return 0;
}
IMPL_LINK_INLINE_END( SwIndexMarkDlg, NextSameHdl, Button *, EMPTYARG )


IMPL_LINK_INLINE_START( SwIndexMarkDlg, PrevHdl, Button *, EMPTYARG )
{
    InsertUpdate();
    pTOXMgr->PrevTOXMark();
    UpdateDialog();
    return 0;
}
IMPL_LINK_INLINE_END( SwIndexMarkDlg, PrevHdl, Button *, EMPTYARG )


IMPL_LINK_INLINE_START( SwIndexMarkDlg, PrevSameHdl, Button *, EMPTYARG )
{
    InsertUpdate();
    pTOXMgr->PrevTOXMark(sal_True);
    UpdateDialog();

    return 0;
}
IMPL_LINK_INLINE_END( SwIndexMarkDlg, PrevSameHdl, Button *, EMPTYARG )


IMPL_LINK( SwIndexMarkDlg, DelHdl, Button *, EMPTYARG )
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
     Beschreibung: Dialoganzeige erneuern
 --------------------------------------------------------------------*/


void SwIndexMarkDlg::UpdateDialog()
{
    DBG_ASSERT(pSh && pTOXMgr, "Shell nicht da?")
    SwTOXMark* pMark = pTOXMgr->GetCurTOXMark();
    ASSERT(pMark, "Keine aktuelle Markierung");
    if(!pMark)
        return;

    ViewShell::SetCareWin( GetParent() );

    aOrgStr = pMark->GetText();
    aEntryED.SetText(aOrgStr);

    // Verzeichnistyp setzen
    sal_Bool bLevelEnable = sal_True,
         bKeyEnable   = sal_False,
         bKey2Enable  = sal_False;

    TOXTypes eCurType = pMark->GetTOXType()->GetType();
    if(TOX_INDEX == eCurType)
    {
        bLevelEnable = sal_False;
        bKeyEnable = sal_True;
        bKey2Enable = 0 != pMark->GetPrimaryKey().Len();
        aKeyDCB.SetText( pMark->GetPrimaryKey() );
        aKey2DCB.SetText( pMark->GetSecondaryKey() );
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

    // Verzeichnis-Typ setzen
    aTypeDCB.SelectEntry(pMark->GetTOXType()->GetTypeName());

    // Next - Prev - Buttons setzen
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
     Beschreibung: Enable Disable des 2. Schluessels
 --------------------------------------------------------------------*/


IMPL_LINK( SwIndexMarkDlg, KeyDCBModifyHdl, ComboBox *, pBox )
{
    sal_Bool bEnable = pBox->GetText().Len() > 0;
    if(!bEnable)
        aKey2DCB.SetText(aEmptyStr);
    aKey2DCB.Enable(bEnable);
    aKey2FT.Enable(bEnable);
    return 0;
}

/*-----------------25.02.94 21:19-------------------
 dtor ueberladen
--------------------------------------------------*/


SwIndexMarkDlg::~SwIndexMarkDlg()
{
    delete pTOXMgr;
    ViewShell::SetCareWin( 0 );
}
/* -----------------07.09.99 08:41-------------------

 --------------------------------------------------*/
void    SwIndexMarkDlg::ReInitDlg(SwWrtShell& rWrtShell)
{
    pSh = &rWrtShell;
    delete pTOXMgr;
    pTOXMgr = new SwTOXMgr(pSh);
    InitControls();
}
/* -----------------06.10.99 10:00-------------------

 --------------------------------------------------*/
SwIndexMarkFloatDlg::SwIndexMarkFloatDlg(SfxBindings* pBindings,
                                   SfxChildWindow* pChild,
                                   Window *pParent,
                                   sal_Bool bNew) :
    SfxModelessDialog(pBindings, pChild, pParent, SW_RES(DLG_INSIDXMARK)),
    aDlg(this, bNew, ResId(WIN_DLG))
{
    FreeResource();
}
/* -----------------06.10.99 10:27-------------------

 --------------------------------------------------*/
void    SwIndexMarkFloatDlg::Activate()
{
    SfxModelessDialog::Activate();
    GetDlg().Activate();
}
/* -----------------06.10.99 10:35-------------------

 --------------------------------------------------*/
SwIndexMarkModalDlg::SwIndexMarkModalDlg(Window *pParent, SwWrtShell& rSh) :
    SvxStandardDialog(pParent, SW_RES(DLG_EDIT_IDXMARK)),
    aDlg(this, sal_False, ResId(WIN_DLG))
{
    FreeResource();
    aDlg.ReInitDlg(rSh);
}
/* -----------------06.10.99 10:46-------------------

 --------------------------------------------------*/
void    SwIndexMarkModalDlg::Apply()
{
    aDlg.Apply();
}
/* -----------------07.09.99 08:15-------------------

 --------------------------------------------------*/
SFX_IMPL_CHILDWINDOW(SwInsertIdxMarkWrapper, FN_INSERT_IDX_ENTRY_DLG)

SwInsertIdxMarkWrapper::SwInsertIdxMarkWrapper( Window *pParentWindow,
                            sal_uInt16 nId,
                            SfxBindings* pBindings,
                            SfxChildWinInfo* pInfo ) :
        SfxChildWindow(pParentWindow, nId)
{

    pWindow = new SwIndexMarkFloatDlg(pBindings, this, pParentWindow );

    SwIndexMarkDlg& rDlg = ((SwIndexMarkFloatDlg*)pWindow)->GetDlg();
    SwWrtShell* pWrtShell = ::GetActiveWrtShell();
    DBG_ASSERT(pWrtShell, "No shell?")
    rDlg.ReInitDlg(*pWrtShell);

    ((SwIndexMarkFloatDlg*)pWindow)->Initialize(pInfo);
    eChildAlignment = SFX_ALIGN_NOALIGNMENT;
}
/* -----------------07.09.99 09:14-------------------

 --------------------------------------------------*/
SfxChildWinInfo SwInsertIdxMarkWrapper::GetInfo() const
{
    SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();

    return aInfo;
}
/* -----------------16.09.99 14:19-------------------

 --------------------------------------------------*/
class SwCreateAuthEntryDlg_Impl : public ModalDialog
{
    OKButton        aOKBT;
    CancelButton    aCancelBT;
    HelpButton      aHelpBT;

    GroupBox        aEntriesGB;

    FixedText*      pFixedTexts[AUTH_FIELD_END];
    ListBox*        pTypeListBox;
    ComboBox*       pIdentifierBox;
    Edit*           pEdits[AUTH_FIELD_END];

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

    void            SetCheckNameHdl(Link& rLink) {aShortNameCheckLink = rLink;}

};
/*-- 15.09.99 08:43:24---------------------------------------------------

  -----------------------------------------------------------------------*/
struct TextInfo
{
    sal_uInt16 nToxField;
    sal_uInt16 nHelpId;
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

sal_Bool SwAuthMarkDlg::bIsFromComponent = sal_True;

SwAuthMarkDlg::SwAuthMarkDlg(  Window *pParent,
                               const ResId& rResId,
                               sal_Bool bNewDlg) :
    Window(pParent, rResId),
    aFromComponentRB(   this, ResId(RB_FROMCOMPONENT    )),
    aFromDocContentRB(  this, ResId(RB_FROMDOCCONTENT   )),
    aEntryFT(   this, ResId(FT_ENTRY    )),
    aEntryED(   this, ResId(ED_ENTRY    )),
    aEntryLB(   this, ResId(LB_ENTRY    )),
    aAuthorFT(  this, ResId(FT_AUTHOR       )),
    aAuthorFI(  this, ResId(FI_AUTHOR   )),
    aTitleFT(   this, ResId(FT_TITLE    )),
    aTitleFI(   this, ResId(FI_TITLE    )),
    aEntryGB(   this, ResId(GB_ENTRY    )),
    aOKBT(      this, ResId(PB_OK       )),
    aCancelBT(  this, ResId(PB_CANCEL   )),
    aHelpBT(    this, ResId(PB_HELP )),
    sChangeST(  ResId(ST_CHANGE)),
    aCreateEntryPB(this,ResId(PB_CREATEENTRY)),
    aEditEntryPB(this,  ResId(PB_EDITENTRY)),
    bNewEntry(bNewDlg),
    pSh(0),
    bBibAccessInitialized(sal_False)
{
    SetStyle(GetStyle()|WB_DIALOGCONTROL);
    FreeResource();

    aFromComponentRB.SetHelpId(HID_AUTH_MARK_DLG_FROM_COMP_RB);
    aFromDocContentRB.SetHelpId(HID_AUTH_MARK_DLG_FROM_DOC_RB );
    aEntryED.SetHelpId(HID_AUTH_MARK_DLG_ID_LISTBOX           );
    aEntryLB.SetHelpId(HID_AUTH_MARK_DLG_ID_LISTBOX           );

    aFromComponentRB.Show(bNewEntry);
    aFromDocContentRB.Show(bNewEntry);
    aFromComponentRB.Check(bIsFromComponent);
    aFromDocContentRB.Check(!bIsFromComponent);

    aOKBT       .SetHelpId(HID_INSERT_AUTH_MRK_OK   );
    aCancelBT   .SetHelpId(HID_INSERT_AUTH_MRK_CLOSE);
    aEntryED        .SetHelpId(HID_INSERT_AUTH_MRK_ENTRY        );
    aCreateEntryPB  .SetHelpId(HID_INSERT_AUTH_MRK_CREATE_ENTRY );
    aEditEntryPB    .SetHelpId(HID_INSERT_AUTH_MRK_EDIT_ENTRY   );

    aOKBT.SetClickHdl(LINK(this,SwAuthMarkDlg, InsertHdl));
    aCancelBT.SetClickHdl(LINK(this,SwAuthMarkDlg, CloseHdl));
    aCreateEntryPB.SetClickHdl(LINK(this,SwAuthMarkDlg, CreateEntryHdl));
    aEditEntryPB.SetClickHdl(LINK(this,SwAuthMarkDlg, CreateEntryHdl));
    aFromComponentRB.SetClickHdl(LINK(this,SwAuthMarkDlg, ChangeSourceHdl));
    aFromDocContentRB.SetClickHdl(LINK(this,SwAuthMarkDlg, ChangeSourceHdl));

    GetParent()->SetText(String(SW_RES(
                    bNewEntry ? STR_AUTHMRK_INSERT : STR_AUTHMRK_EDIT)));
    aEntryED.Show(!bNewEntry);
    aEntryLB.Show(bNewEntry);
    if(!bNewEntry)
    {
        aOKBT.SetText(sChangeST);
    }
    else
    {
        aEntryLB.SetSelectHdl(LINK(this, SwAuthMarkDlg, CompEntryHdl));
    }
}
/*-- 15.09.99 08:43:25---------------------------------------------------

  -----------------------------------------------------------------------*/
SwAuthMarkDlg::~SwAuthMarkDlg()
{
}
/*-- 15.09.99 08:43:25---------------------------------------------------

  -----------------------------------------------------------------------*/
void    SwAuthMarkDlg::ReInitDlg(SwWrtShell& rWrtShell)
{
    pSh = &rWrtShell;
    InitControls();
}
/* -----------------15.09.99 08:57-------------------

 --------------------------------------------------*/
IMPL_LINK( SwAuthMarkDlg, CloseHdl, PushButton *, EMPTYARG )
{
    if(bNewEntry)
    {
        sal_uInt16 nSlot = FN_INSERT_AUTH_ENTRY_DLG;
        SfxViewFrame::Current()->GetDispatcher()->Execute(nSlot,
                    SFX_CALLMODE_ASYNCHRON|SFX_CALLMODE_RECORD);
    }
    else
    {
        ((SwAuthMarkModalDlg*)GetParent())->EndDialog(RET_CANCEL);
    }
    return 0;
}
/* -----------------06.12.99 13:54-------------------

 --------------------------------------------------*/
String lcl_FindColumnEntry(const beans::PropertyValue* pFields, sal_Int32 nLen, const String& rColumnTitle)
{
    String sRet;
    OUString uColumnTitle = rColumnTitle;
    for(sal_uInt16 i = 0; i < nLen; i++)
    {
        if(pFields[i].Name == uColumnTitle &&
            pFields[i].Value.getValueType() == ::getCppuType((const OUString*)0))

        {
            OUString uTmp;
            pFields[i].Value >>= uTmp;
            sRet = String(uTmp);
            break;
        }
    }
    return sRet;
}
/* -----------------------------07.12.99 15:39--------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK( SwAuthMarkDlg, CompEntryHdl, ListBox*, pBox)
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
                if(aEntry.getValueType() == ::getCppuType((uno::Sequence<beans::PropertyValue>*)0))

                {
                    uno::Sequence<beans::PropertyValue> aFieldProps = *(uno::Sequence<beans::PropertyValue>*)aEntry.getValue();
                    const beans::PropertyValue* pProps = aFieldProps.getConstArray();
                    for(sal_uInt16 i = 0; i < AUTH_FIELD_END; i++)
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
    aAuthorFI.SetText(m_sFields[AUTH_FIELD_AUTHOR]);
    aTitleFI.SetText(m_sFields[AUTH_FIELD_TITLE]);
    return 0;
}

/* -----------------15.09.99 08:57-------------------

 --------------------------------------------------*/
IMPL_LINK( SwAuthMarkDlg, InsertHdl, PushButton *, EMPTYARG )
{
    //insert or update the SwAuthorityField...
    if(pSh)
    {
        sal_Bool bDifferent = sal_False;
        DBG_ASSERT(m_sFields[AUTH_FIELD_IDENTIFIER].Len() , "No Id is set!")
        DBG_ASSERT(m_sFields[AUTH_FIELD_AUTHORITY_TYPE].Len() , "No authority type is set!")
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
                QueryBox aQuery(this, SW_RES(DLG_CHANGE_AUTH_ENTRY));
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
            aMgr.InsertFld( TYP_AUTHORITY, 0, sFields, aEmptyStr, 0 );
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
/* -----------------17.09.99 13:06-------------------

 --------------------------------------------------*/
IMPL_LINK(SwAuthMarkDlg, CreateEntryHdl, PushButton*, pButton)
{
    sal_Bool bCreate = pButton == &aCreateEntryPB;
    String sOldId = m_sCreatedEntry[0];
    for(sal_uInt16 i = 0; i < AUTH_FIELD_END; i++)
        m_sCreatedEntry[i] = bCreate ? aEmptyStr : m_sFields[i];
    SwCreateAuthEntryDlg_Impl aDlg(pButton,
                bCreate ? m_sCreatedEntry : m_sFields,
                *pSh, bNewEntry, bCreate);
    if(bNewEntry)
    {
        aDlg.SetCheckNameHdl(LINK(this, SwAuthMarkDlg, IsEntryAllowedHdl));
    }
    if(RET_OK == aDlg.Execute())
    {
        if(bCreate && sOldId.Len())
        {
            aEntryLB.RemoveEntry(sOldId);
        }
        for(sal_uInt16 i = 0; i < AUTH_FIELD_END; i++)
        {
            m_sFields[i] = aDlg.GetEntryText((ToxAuthorityField)i);
            m_sCreatedEntry[i] = m_sFields[i];
        }
        if(bNewEntry && !aFromDocContentRB.IsChecked())
        {
            aFromDocContentRB.Check(sal_True);
            ChangeSourceHdl(&aFromDocContentRB);
        }
        if(bCreate)
        {
            DBG_ASSERT(LISTBOX_ENTRY_NOTFOUND ==
                        aEntryLB.GetEntryPos(m_sFields[AUTH_FIELD_IDENTIFIER]),
                        "entry exists!")
            aEntryLB.InsertEntry(m_sFields[AUTH_FIELD_IDENTIFIER]);
            aEntryLB.SelectEntry(m_sFields[AUTH_FIELD_IDENTIFIER]);
        }
        aEntryED.SetText(m_sFields[AUTH_FIELD_IDENTIFIER]);
        aAuthorFI.SetText(m_sFields[AUTH_FIELD_AUTHOR]);
        aTitleFI.SetText(m_sFields[AUTH_FIELD_TITLE]);
    }
    return 0;
}
/* -----------------------------20.12.99 14:26--------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK(SwAuthMarkDlg, ChangeSourceHdl, RadioButton*, pButton)
{
    sal_Bool bFromComp = (pButton == &aFromComponentRB);
    bIsFromComponent = bFromComp;
    aCreateEntryPB.Enable(!bIsFromComponent);
    aEntryLB.Clear();
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
                if(aNames.getValueType() == ::getCppuType((uno::Sequence<beans::PropertyValue>*)0))
                {
                    uno::Sequence<beans::PropertyValue> aSeq = *(uno::Sequence<beans::PropertyValue>*)aNames.getValue();
                    const beans::PropertyValue* pArr = aSeq.getConstArray();
                    for(sal_uInt16 i = 0; i < aSeq.getLength(); i++)
                    {
                        String sTitle = pArr[i].Name;
                        sal_Int16 nField;
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
                aEntryLB.InsertEntry(pNames[i]);
            }
        }
    }
    else
    {
        const SwAuthorityFieldType* pFType = (const SwAuthorityFieldType*)
                                    pSh->GetFldType(RES_AUTHORITY, aEmptyStr);
        if(pFType)
        {
            SvStringsDtor aIds;
            pFType->GetAllEntryIdentifiers( aIds );
            for(sal_uInt16 n = 0; n < aIds.Count(); n++)
                aEntryLB.InsertEntry(*aIds.GetObject(n));
        }
        if(m_sCreatedEntry[AUTH_FIELD_IDENTIFIER].Len())
            aEntryLB.InsertEntry(m_sCreatedEntry[AUTH_FIELD_IDENTIFIER]);
    }
    aEntryLB.SelectEntryPos(0);
    CompEntryHdl(&aEntryLB);
    return 0;
}
/* -----------------------------20.12.99 15:11--------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK(SwAuthMarkDlg, IsEntryAllowedHdl, Edit*, pEdit)
{
    String sEntry = pEdit->GetText();
    sal_Bool bAllowed = sal_False;
    if(sEntry.Len())
    {
        if(aEntryLB.GetEntryPos(sEntry) != LISTBOX_ENTRY_NOTFOUND)
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
/* -----------------21.09.99 14:19-------------------

 --------------------------------------------------*/
void SwAuthMarkDlg::InitControls()
{
    DBG_ASSERT(pSh, "Shell nicht da?")
    SwField* pField = pSh->GetCurFld();
    ASSERT(bNewEntry || pField, "Keine aktuelle Markierung");
    if(bNewEntry)
    {
        ChangeSourceHdl(aFromComponentRB.IsChecked() ? &aFromComponentRB : &aFromDocContentRB);
        aCreateEntryPB.Enable(!aFromComponentRB.IsChecked());
        if(!aFromComponentRB.IsChecked() && m_sCreatedEntry[0].Len())
            for(sal_uInt16 i = 0; i < AUTH_FIELD_END; i++)
                m_sFields[i] = m_sCreatedEntry[i];
    }
    if(bNewEntry || !pField || pField->GetTyp()->Which() != RES_AUTHORITY)
        return;

    const SwAuthEntry* pEntry = ((SwAuthorityFieldType*)pField->GetTyp())->
            GetEntryByIdentifier(((SwAuthorityField*)pField)->
                                    GetFieldText(AUTH_FIELD_IDENTIFIER));

    DBG_ASSERT(pEntry, "No authority entry found")
    if(!pEntry)
        return;
    for(sal_uInt16 i = 0; i < AUTH_FIELD_END; i++)
        m_sFields[i] = pEntry->GetAuthorField((ToxAuthorityField)i);

    aEntryED.SetText(pEntry->GetAuthorField(AUTH_FIELD_IDENTIFIER));
    aAuthorFI.SetText(pEntry->GetAuthorField(AUTH_FIELD_AUTHOR));
    aTitleFI.SetText(pEntry->GetAuthorField(AUTH_FIELD_TITLE));

}

/* -----------------07.09.99 08:15-------------------

 --------------------------------------------------*/
SFX_IMPL_CHILDWINDOW(SwInsertAuthMarkWrapper, FN_INSERT_AUTH_ENTRY_DLG)

SwInsertAuthMarkWrapper::SwInsertAuthMarkWrapper(   Window *pParentWindow,
                            sal_uInt16 nId,
                            SfxBindings* pBindings,
                            SfxChildWinInfo* pInfo ) :
        SfxChildWindow(pParentWindow, nId)
{

    pWindow = new SwAuthMarkFloatDlg(pBindings, this, pParentWindow );

    SwAuthMarkDlg& rDlg = ((SwAuthMarkFloatDlg*)pWindow)->GetDlg();
    ((SwAuthMarkFloatDlg*)pWindow)->Initialize(pInfo);
    SwWrtShell* pWrtShell = ::GetActiveWrtShell();
    DBG_ASSERT(pWrtShell, "No shell?")
    rDlg.ReInitDlg(*pWrtShell);

    eChildAlignment = SFX_ALIGN_NOALIGNMENT;
}
/* -----------------07.09.99 09:14-------------------

 --------------------------------------------------*/
SfxChildWinInfo SwInsertAuthMarkWrapper::GetInfo() const
{
    SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();
    return aInfo;
}
/* -----------------19.10.99 11:16-------------------

 --------------------------------------------------*/
void    SwInsertAuthMarkWrapper::ReInitDlg(SwWrtShell& rWrtShell)
{
    ((SwAuthMarkFloatDlg*)pWindow)->GetDlg().ReInitDlg(rWrtShell);
}

/* -----------------16.09.99 14:27-------------------

 --------------------------------------------------*/
SwCreateAuthEntryDlg_Impl::SwCreateAuthEntryDlg_Impl(Window* pParent,
        const String pFields[],
        SwWrtShell& rSh,
        sal_Bool bNewEntry,
        sal_Bool bCreate) :
    ModalDialog(pParent, SW_RES(DLG_CREATE_AUTH_ENTRY)),
    aOKBT(this,         ResId(PB_OK         )),
    aCancelBT(this,     ResId(PB_CANCEL     )),
    aHelpBT(this,       ResId(PB_HELP       )),
    aEntriesGB(this,    ResId(GB_ENTRIES    )),
    pIdentifierBox(0),
    pTypeListBox(0),
    rWrtSh(rSh),
    m_bNewEntryMode(bNewEntry),
    m_bNameAllowed(sal_True)
{
    FreeResource();
    Point aGBPos(aEntriesGB.GetPosPixel());
    Point aTL1(aGBPos);
    Size aGBSz(aEntriesGB.GetSizePixel());
    long nControlSpace = aGBSz.Width() / 4;
    long nControlWidth = nControlSpace - 2 * aTL1.X();
    aTL1.X() *= 2;
    aTL1.Y() *= 5;
    Point aTR1(aTL1);
    aTR1.X() += nControlSpace;
    Point aTL2(aTR1);
    aTL2.X() += nControlSpace;
    Point aTR2(aTL2);
    aTR2.X() += nControlSpace;
    Size aFixedTextSize(aGBSz);
    Size aTmpSz(8,10);
    aTmpSz = LogicToPixel(aTmpSz, MAP_APPFONT);
    aFixedTextSize.Height() = aTmpSz.Width();
    Size aEditSize(aFixedTextSize);
    aFixedTextSize.Width() = nControlWidth + aGBPos.X();
    aEditSize.Height() = aTmpSz.Height();
    aEditSize.Width() = nControlWidth;

    sal_uInt16 nOffset = aTmpSz.Width() * 3 / 2;
    sal_Bool bLeft = sal_True;
    for(sal_uInt16 nIndex = 0; nIndex < AUTH_FIELD_END; nIndex++)
    {
        const TextInfo aCurInfo = aTextInfoArr[nIndex];

        pFixedTexts[nIndex] = new FixedText(this);

        pFixedTexts[nIndex]->SetSizePixel(aFixedTextSize);
        pFixedTexts[nIndex]->SetPosPixel(bLeft ? aTL1 : aTL2);
        pFixedTexts[nIndex]->SetText(SW_RES(STR_AUTH_FIELD_START + aCurInfo.nToxField));
        pFixedTexts[nIndex]->Show();
        pEdits[nIndex] = 0;
        if( AUTH_FIELD_AUTHORITY_TYPE == aCurInfo.nToxField )
        {
            pTypeListBox = new ListBox(this, WB_DROPDOWN|WB_BORDER);
            for(sal_uInt16 j = 0; j < AUTH_TYPE_END; j++)
                pTypeListBox->InsertEntry(String(SW_RES(STR_AUTH_TYPE_START + j)));
            if(pFields[aCurInfo.nToxField].Len())
            {
                sal_uInt16 nIndexPos = pFields[aCurInfo.nToxField].ToInt32();
                pTypeListBox->SelectEntryPos(nIndexPos);
            }
            Size aTmp(aEditSize);
            aTmp.Height() *= 4;
            pTypeListBox->SetSizePixel(aTmp);
            pTypeListBox->SetPosPixel(bLeft ? aTR1 : aTR2);
            pTypeListBox->Show();
            pTypeListBox->SetSelectHdl(LINK(this, SwCreateAuthEntryDlg_Impl, EnableHdl));
            pTypeListBox->SetHelpId(aCurInfo.nHelpId);

        }
        else if(AUTH_FIELD_IDENTIFIER == aCurInfo.nToxField && !m_bNewEntryMode)
        {
            pIdentifierBox = new ComboBox(this, WB_BORDER|WB_DROPDOWN);
            pIdentifierBox->SetSelectHdl(LINK(this,
                                    SwCreateAuthEntryDlg_Impl, IdentifierHdl));


            const SwAuthorityFieldType* pFType = (const SwAuthorityFieldType*)
                                        rSh.GetFldType(RES_AUTHORITY, aEmptyStr);
            if(pFType)
            {
                SvStringsDtor aIds;
                pFType->GetAllEntryIdentifiers( aIds );
                for(sal_uInt16 n = 0; n < aIds.Count(); n++)
                    pIdentifierBox->InsertEntry(*aIds.GetObject(n));
            }
            pIdentifierBox->SetText(pFields[aCurInfo.nToxField]);
            Size aTmp(aEditSize);
            aTmp.Height() *= 4;
            pIdentifierBox->SetSizePixel(aTmp);
            pIdentifierBox->SetPosPixel(bLeft ? aTR1 : aTR2);
            pIdentifierBox->Show();
            pIdentifierBox->SetHelpId(aCurInfo.nHelpId);
        }
        else
        {
            pEdits[nIndex] = new Edit(this, WB_BORDER);
            pEdits[nIndex]->SetSizePixel(aEditSize);
            pEdits[nIndex]->SetPosPixel(bLeft ? aTR1 : aTR2);
            pEdits[nIndex]->SetText(pFields[aCurInfo.nToxField]);
            pEdits[nIndex]->Show();
            pEdits[nIndex]->SetHelpId(aCurInfo.nHelpId);
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

    long nHeightDiff = - aGBSz.Height();
    aGBSz.Height() = aTL1.Y();
    nHeightDiff += aGBSz.Height();
    aEntriesGB.SetSizePixel(aGBSz);
    Size aDlgSize(GetSizePixel());
    aDlgSize.Height() += nHeightDiff;
    SetSizePixel(aDlgSize);

}
/* -----------------16.09.99 14:47-------------------

 --------------------------------------------------*/
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
/* -----------------16.09.99 14:27-------------------

 --------------------------------------------------*/
String  SwCreateAuthEntryDlg_Impl::GetEntryText(ToxAuthorityField eField) const
{
    String sRet;
    if( AUTH_FIELD_AUTHORITY_TYPE == eField )
    {
        DBG_ASSERT(pTypeListBox, "No ListBox")
        sRet = pTypeListBox->GetSelectEntryPos();
    }
    else if( AUTH_FIELD_IDENTIFIER == eField && !m_bNewEntryMode)
    {
        DBG_ASSERT(pIdentifierBox, "No ComboBox")
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
/* -----------------21.09.99 13:54-------------------

 --------------------------------------------------*/
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
                if(AUTH_FIELD_IDENTIFIER == i)
                    continue;
                if(AUTH_FIELD_AUTHORITY_TYPE == i)
                    pTypeListBox->SelectEntry(
                                pEntry->GetAuthorField((ToxAuthorityField)i));
                else
                    pEdits[i]->SetText(
                                pEntry->GetAuthorField((ToxAuthorityField)i));
            }
        }
    }
    return 0;
}
/* -----------------------------20.12.99 15:07--------------------------------

 ---------------------------------------------------------------------------*/

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
/* -----------------------------20.12.99 15:54--------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK(SwCreateAuthEntryDlg_Impl, EnableHdl, ListBox*, pBox)
{
    aOKBT.Enable(m_bNameAllowed && pBox->GetSelectEntryCount());
    return 0;
};
/* -----------------06.10.99 10:00-------------------

 --------------------------------------------------*/
SwAuthMarkFloatDlg::SwAuthMarkFloatDlg(SfxBindings* pBindings,
                                   SfxChildWindow* pChild,
                                   Window *pParent,
                                   sal_Bool bNew) :
    SfxModelessDialog(pBindings, pChild, pParent, SW_RES(DLG_INSAUTHMARK)),
    aDlg(this, ResId(WIN_DLG), bNew)
{
    FreeResource();
}
/* -----------------06.10.99 10:27-------------------

 --------------------------------------------------*/
void    SwAuthMarkFloatDlg::Activate()
{
    SfxModelessDialog::Activate();
    GetDlg().Activate();
}
/* -----------------06.10.99 10:35-------------------

 --------------------------------------------------*/
SwAuthMarkModalDlg::SwAuthMarkModalDlg(Window *pParent, SwWrtShell& rSh) :
    SvxStandardDialog(pParent, SW_RES(DLG_EDIT_AUTHMARK)),
    aDlg(this, ResId(WIN_DLG), sal_False)
{
    FreeResource();
    aDlg.ReInitDlg(rSh);
}
/* -----------------06.10.99 10:46-------------------

 --------------------------------------------------*/
void    SwAuthMarkModalDlg::Apply()
{
    aDlg.InsertHdl(0);
}
/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.3  2000/10/25 12:07:02  jp
    Spellchecker/Hyphenator are not longer member of the shells

    Revision 1.2  2000/10/20 14:18:04  os
    use comphelper methods

    Revision 1.1.1.1  2000/09/18 17:14:44  hr
    initial import

    Revision 1.105  2000/09/18 16:05:53  willem.vandorp
    OpenOffice header added.

    Revision 1.104  2000/09/07 15:59:25  os
    change: SFX_DISPATCHER/SFX_BINDINGS removed

    Revision 1.103  2000/07/03 08:27:55  os
    #72742# resource warnings corrected

    Revision 1.102  2000/06/27 19:42:06  jp
    Bug #70447#: select the current TOXMark if the EditDialog is open

    Revision 1.101  2000/05/26 07:21:31  os
    old SW Basic API Slots removed

    Revision 1.100  2000/05/16 17:29:33  jp
    Changes for Unicode

    Revision 1.99  2000/05/16 09:15:13  os
    project usr removed

    Revision 1.98  2000/04/18 15:32:23  os
    UNICODE

    Revision 1.97  2000/04/04 15:21:07  os
    #74729# Author-Title assignment corrected

    Revision 1.96  2000/03/30 13:19:03  os
    UNO III

    Revision 1.95  2000/03/23 07:42:45  os
    UNO III

    Revision 1.94  2000/02/18 14:36:57  kz
    #65293# include PropertyValue

    Revision 1.93  2000/02/11 14:55:46  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.92  2000/01/31 10:48:04  os
    #71961# TOX_INDEX as default

    Revision 1.91  2000/01/18 14:44:14  os
    #72050# base class changed

    Revision 1.90  2000/01/18 09:15:45  os
    #72010# type is numeric

    Revision 1.89  2000/01/17 12:02:19  os
    #70703# apply to all similar texts: search options case sensitive and word only

    Revision 1.88  1999/12/27 12:22:18  os
    #71307# HelpIds added

    Revision 1.87  1999/12/21 13:47:39  os
    #70508# select source for bibliography entries

    Revision 1.86  1999/12/14 10:18:42  os
    #70819# new index button now an ImageButton at a better position

    Revision 1.85  1999/12/07 16:37:15  os
    #70508# get data from bib component

    Revision 1.84  1999/11/30 13:47:52  os
    #68736# HelpId for insert index mark dialogs

    Revision 1.83  1999/11/25 08:56:01  os
    prevent from changing content in readonly

    Revision 1.82  1999/10/19 09:32:39  os
    casts corrected

    Revision 1.81  1999/10/18 11:42:53  os
    Helpids for Modeless idx entry dialogs

    Revision 1.80  1999/10/13 14:14:59  os
    create new user index via button

    Revision 1.79  1999/10/11 09:00:21  os
    AuthorityField(Type):Import/Export interfaces

    Revision 1.78  1999/10/06 12:11:23  os
    edit tox entry dialogs now modal

    Revision 1.77  1999/09/21 13:43:20  os
    edit Authority fields

    Revision 1.76  1999/09/20 09:36:38  os
    new Dialog: SwCreateAuthEntryDlg_Impl

    Revision 1.75  1999/09/15 13:57:24  os
    dialog for authorities

    Revision 1.74  1999/09/14 13:29:37  os
    call ModifyHdl if entry text is changed

    Revision 1.73  1999/09/07 13:55:04  os
    Insert/EditIndexEntry as FloatingWindow

    Revision 1.72  1999/08/24 08:41:10  OS
    level count of user indexes now on MAXLEVEL


      Rev 1.71   24 Aug 1999 10:41:10   OS
   level count of user indexes now on MAXLEVEL

      Rev 1.70   23 Aug 1999 10:24:30   OS
   extended indexes: InsertTOXMark simplified; main entry in indexes

      Rev 1.69   18 Jun 1999 11:53:56   OS
   #66778# empty alternative texts are not allowed

      Rev 1.68   10 Jun 1999 13:02:04   OS
   restore selection

      Rev 1.67   28 Apr 1999 12:42:54   OS
   #65478# Level fuer Benutzerverzeichnis erlauben

      Rev 1.66   21 Jan 1999 09:15:06   JP
   Task #58677#: Crsr in Readonly Bereichen zulassen

      Rev 1.65   20 Jan 1999 11:35:28   OS
   #60881# Verzeichniseintraege mehrfach einf?gen

      Rev 1.64   10 Feb 1998 07:49:00   OS
   Level nur fuer Inhaltsverzeichnisse #47169#

      Rev 1.63   25 Nov 1997 10:51:02   MA
   includes

      Rev 1.62   05 Nov 1997 12:42:56   OS
   richtiges Flag uebergeben #43766#

------------------------------------------------------------------------*/


