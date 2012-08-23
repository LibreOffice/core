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

#include <vcl/lstbox.hxx>
#include <svl/stritem.hxx>
#include <sfx2/request.hxx>
#include <svx/htmlmode.hxx>
#include <dbfld.hxx>
#include <flddat.hxx>
#include <fmtfld.hxx>
#include <viewopt.hxx>
#include <fldedt.hxx>
#include <docsh.hxx>
#include <swmodule.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <expfld.hxx>
#include <fldtdlg.hxx>
#include <fldpage.hxx>
#include <docufld.hxx>
#include <cmdid.h>
#include <globals.hrc>
#include <sfx2/bindings.hxx>
#include <switerator.hxx>

using namespace ::com::sun::star;

SwFldPage::SwFldPage( Window *pParent, const ResId &rId,
                        const SfxItemSet &rAttrSet )
    :SfxTabPage     (pParent, rId, rAttrSet),
    m_pCurFld       (0),
    m_pWrtShell     (0),
    m_nPageId       ( static_cast< sal_uInt16 >(rId.GetId()) ),
    m_nTypeSel      (LISTBOX_ENTRY_NOTFOUND),
    m_nSelectionSel (LISTBOX_ENTRY_NOTFOUND),
    m_bFldEdit      (sal_False),
    m_bInsert           (sal_True),
    m_bFldDlgHtmlMode   (sal_False),
    m_bRefresh          (sal_False),
    m_bFirstHTMLInit    (sal_True)
{

}

SwFldPage::~SwFldPage()
{
}

/*--------------------------------------------------------------------
    Description: initialise TabPage
 --------------------------------------------------------------------*/

void SwFldPage::Init()
{
    SwDocShell* pDocSh = (SwDocShell*)SfxObjectShell::Current();
    sal_Bool bNewMode = 0 != (::GetHtmlMode(pDocSh) & HTMLMODE_ON);

    m_bFldEdit = 0 == GetTabDialog();

    // newly initialise FieldManager. important for
    // Dok-Switch (fldtdlg:ReInitTabPage)
    m_pCurFld = m_aMgr.GetCurFld();

    if( bNewMode != m_bFldDlgHtmlMode )
    {
        m_bFldDlgHtmlMode = bNewMode;

        // initialise Rangelistbox
        if( m_bFldDlgHtmlMode && m_bFirstHTMLInit )
        {
            m_bFirstHTMLInit = sal_False;
            SwWrtShell *pSh = m_pWrtShell;
            if(! pSh)
                pSh = ::GetActiveWrtShell();
            if(pSh)
            {
                SwDoc* pDoc = pSh->GetDoc();
                pSh->InsertFldType( SwSetExpFieldType( pDoc,
                                    rtl::OUString("HTML_ON"), 1));
                pSh->InsertFldType( SwSetExpFieldType(pDoc,
                                    rtl::OUString("HTML_OFF"), 1));
            }
        }
    }
}

/*--------------------------------------------------------------------
     Description: newly initialise page
 --------------------------------------------------------------------*/

void SwFldPage::ActivatePage()
{
    EnableInsert(m_bInsert);
}

/*--------------------------------------------------------------------
     Description: complete reset; edit new field
 --------------------------------------------------------------------*/

void SwFldPage::EditNewField( sal_Bool bOnlyActivate )
{
    if( !bOnlyActivate )
    {
        m_nTypeSel = LISTBOX_ENTRY_NOTFOUND;
    }
    m_nSelectionSel = LISTBOX_ENTRY_NOTFOUND;
    m_bRefresh = sal_True;
    Reset(*(SfxItemSet*)0);
    m_bRefresh = sal_False;
}

/*--------------------------------------------------------------------
     Description: insert field
 --------------------------------------------------------------------*/

sal_Bool SwFldPage::InsertFld(sal_uInt16 nTypeId, sal_uInt16 nSubType, const String& rPar1,
                            const String& rPar2, sal_uLong nFormatId,
                            sal_Unicode cSeparator, sal_Bool bIsAutomaticLanguage)
{
    sal_Bool bRet = sal_False;
    SwView* pView = GetActiveView();
    SwWrtShell *pSh = m_pWrtShell ? m_pWrtShell : pView->GetWrtShellPtr();

    if (!IsFldEdit())   // insert new field
    {
        SwInsertFld_Data aData(nTypeId, nSubType, rPar1, rPar2, nFormatId, 0, cSeparator, bIsAutomaticLanguage );
        //#i26566# provide parent for SwWrtShell::StartInputFldDlg
        aData.pParent = &GetTabDialog()->GetOKButton();
        bRet = m_aMgr.InsertFld( aData );

        uno::Reference< frame::XDispatchRecorder > xRecorder =
                pView->GetViewFrame()->GetBindings().GetRecorder();
        if ( xRecorder.is() )
        {
            sal_Bool bRecordDB = TYP_DBFLD == nTypeId ||
                            TYP_DBSETNUMBERFLD == nTypeId ||
                            TYP_DBNUMSETFLD == nTypeId ||
                            TYP_DBNEXTSETFLD == nTypeId ||
                            TYP_DBNAMEFLD == nTypeId ;

            SfxRequest aReq( pView->GetViewFrame(),
                    bRecordDB ?  FN_INSERT_DBFIELD : FN_INSERT_FIELD );
            if(bRecordDB)
            {
                aReq.AppendItem(SfxStringItem
                        (FN_INSERT_DBFIELD,rPar1.GetToken(0, DB_DELIM)));
                aReq.AppendItem(SfxStringItem
                        (FN_PARAM_1,rPar1.GetToken(1, DB_DELIM)));
                aReq.AppendItem(SfxInt32Item
                        (FN_PARAM_3,rPar1.GetToken(1, DB_DELIM).ToInt32()));
                aReq.AppendItem(SfxStringItem
                        (FN_PARAM_2,rPar1.GetToken(3, DB_DELIM)));
            }
            else
            {
                aReq.AppendItem(SfxStringItem(FN_INSERT_FIELD, rPar1));
                aReq.AppendItem(SfxStringItem
                        (FN_PARAM_3, rtl::OUString(cSeparator)));
                aReq.AppendItem(SfxUInt16Item(FN_PARAM_FIELD_SUBTYPE, nSubType));
            }
            aReq.AppendItem(SfxUInt16Item(FN_PARAM_FIELD_TYPE   , nTypeId));
            aReq.AppendItem(SfxStringItem(FN_PARAM_FIELD_CONTENT, rPar2));
            aReq.AppendItem(SfxUInt32Item(FN_PARAM_FIELD_FORMAT , nFormatId));
            aReq.Done();
        }

    }
    else    // change field
    {
        SwField *const pTmpFld = m_pCurFld->CopyField();

        String sPar1(rPar1);
        String sPar2(rPar2);
        switch( nTypeId )
        {
        case TYP_DATEFLD:
        case TYP_TIMEFLD:
            nSubType = static_cast< sal_uInt16 >(((nTypeId == TYP_DATEFLD) ? DATEFLD : TIMEFLD) |
                       ((nSubType == DATE_VAR) ? 0 : FIXEDFLD));
            break;

        case TYP_DBNAMEFLD:
        case TYP_DBNEXTSETFLD:
        case TYP_DBNUMSETFLD:
        case TYP_DBSETNUMBERFLD:
            {
                xub_StrLen nPos = 0;
                SwDBData aData;

                aData.sDataSource = rPar1.GetToken(0, DB_DELIM, nPos);
                aData.sCommand = rPar1.GetToken(0, DB_DELIM, nPos);
                aData.nCommandType = rPar1.GetToken(0, DB_DELIM, nPos).ToInt32();
                sPar1 = rPar1.Copy(nPos);

                ((SwDBNameInfField*)pTmpFld)->SetDBData(aData);
            }
            break;

        case TYP_DBFLD:
            {
                SwDBData aData;
                aData.sDataSource = rPar1.GetToken(0, DB_DELIM);
                aData.sCommand = rPar1.GetToken(1, DB_DELIM);
                aData.nCommandType = rPar1.GetToken(2, DB_DELIM).ToInt32();
                String sColumn = rPar1.GetToken(3, DB_DELIM);

                SwDBFieldType* pOldTyp = (SwDBFieldType*)pTmpFld->GetTyp();
                SwDBFieldType* pTyp = (SwDBFieldType*)pSh->InsertFldType(
                        SwDBFieldType(pSh->GetDoc(), sColumn, aData));

                SwIterator<SwFmtFld,SwFieldType> aIter( *pOldTyp );

                for( SwFmtFld* pFmtFld = aIter.First(); pFmtFld; pFmtFld = aIter.Next() )
                {
                    if( pFmtFld->GetFld() == m_pCurFld)
                    {
                        pFmtFld->RegisterToFieldType(*pTyp);
                        pTmpFld->ChgTyp(pTyp);
                        break;
                    }
                }
            }
            break;

        case TYP_SEQFLD:
            {
                SwSetExpFieldType* pTyp = (SwSetExpFieldType*)pTmpFld->GetTyp();
                pTyp->SetOutlineLvl( static_cast< sal_uInt8 >(nSubType & 0xff));
                pTyp->SetDelimiter(rtl::OUString(cSeparator));

                nSubType = nsSwGetSetExpType::GSE_SEQ;
            }
            break;

        case TYP_INPUTFLD:
            {
                // User- or SetField ?
                if (m_aMgr.GetFldType(RES_USERFLD, sPar1) == 0 &&
                !(pTmpFld->GetSubType() & INP_TXT)) // SETEXPFLD
                {
                    SwSetExpField* pFld = (SwSetExpField*)pTmpFld;
                    pFld->SetPromptText(sPar2);
                    sPar2 = pFld->GetPar2();
                }
            }
            break;
        case TYP_DOCINFOFLD:
            {
                if( nSubType == nsSwDocInfoSubType::DI_CUSTOM )
                {
                    SwDocInfoField* pDocInfo = static_cast<SwDocInfoField*>( pTmpFld );
                    pDocInfo->SetName( rPar1 );
                }
            }
            break;
        }

        pSh->StartAllAction();

        pTmpFld->SetSubType(nSubType);
        pTmpFld->SetAutomaticLanguage(bIsAutomaticLanguage);

        m_aMgr.UpdateCurFld( nFormatId, sPar1, sPar2, pTmpFld );

        m_pCurFld = m_aMgr.GetCurFld();

        switch (nTypeId)
        {
            case TYP_HIDDENTXTFLD:
            case TYP_HIDDENPARAFLD:
                m_aMgr.EvalExpFlds(pSh);
                break;
        }

        pSh->SetUndoNoResetModified();
        pSh->EndAllAction();
    }

    return bRet;
}

void SwFldPage::SavePos( const ListBox* pLst1, const ListBox* pLst2,
                         const ListBox* pLst3 )
{
    const ListBox* aLBArr [ coLBCount ] = { pLst1, pLst2, pLst3 };

    const ListBox** ppLB = aLBArr;
    for( int i = 0; i < coLBCount; ++i, ++ppLB )
        if( (*ppLB) && (*ppLB)->GetEntryCount() )
            m_aLstStrArr[ i ] = (*ppLB)->GetSelectEntry();
        else
            m_aLstStrArr[ i ].Erase();
}

void SwFldPage::RestorePos(ListBox* pLst1, ListBox* pLst2, ListBox* pLst3)
{
    sal_uInt16 nPos = 0;
    ListBox* aLBArr [ coLBCount ] = { pLst1, pLst2, pLst3 };
    ListBox** ppLB = aLBArr;
    for( int i = 0; i < coLBCount; ++i, ++ppLB )
        if( (*ppLB) && (*ppLB)->GetEntryCount() && m_aLstStrArr[ i ].Len() &&
            LISTBOX_ENTRY_NOTFOUND !=
                        ( nPos = (*ppLB)->GetEntryPos(m_aLstStrArr[ i ] ) ) )
            (*ppLB)->SelectEntryPos( nPos );
}

/*--------------------------------------------------------------------
     Description: Insert new fields
 --------------------------------------------------------------------*/

IMPL_LINK( SwFldPage, InsertHdl, Button *, pBtn )
{
    SwFldDlg *pDlg = (SwFldDlg*)GetTabDialog();

    if (pDlg)
    {
        pDlg->InsertHdl();

        if (pBtn)
            pBtn->GrabFocus();  // because of InputField-Dlg
    }
    else
    {
        SwFldEditDlg *pEditDlg = (SwFldEditDlg *)GetParent();
        pEditDlg->InsertHdl();
    }

    return 0;
}

/*--------------------------------------------------------------------
     Description: enable/disable "Insert"-Button
 --------------------------------------------------------------------*/

void SwFldPage::EnableInsert(sal_Bool bEnable)
{
    SwFldDlg *pDlg = (SwFldDlg*)GetTabDialog();

    if (pDlg)
    {
        if (pDlg->GetCurPageId() == m_nPageId)
            pDlg->EnableInsert(bEnable);
    }
    else
    {
        SwFldEditDlg *pEditDlg = (SwFldEditDlg *)GetParent();
        pEditDlg->EnableInsert(bEnable);
    }

    m_bInsert = bEnable;
}

IMPL_LINK_NOARG(SwFldPage, NumFormatHdl)
{
    InsertHdl();

    return 0;
}

void SwFldPage::SetWrtShell( SwWrtShell* pShell )
{
    m_pWrtShell = pShell;
    m_aMgr.SetWrtShell( pShell );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
