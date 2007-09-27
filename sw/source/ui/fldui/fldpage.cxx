/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fldpage.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 11:47:33 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif



#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SVX_HTMLMODE_HXX //autogen
#include <svx/htmlmode.hxx>
#endif

#ifndef _DBFLD_HXX //autogen
#include <dbfld.hxx>
#endif
#ifndef _FLDDAT_HXX
#include <flddat.hxx>
#endif
#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _FLDEDT_HXX
#include <fldedt.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _EXPFLD_HXX
#include <expfld.hxx>
#endif
#ifndef _FLDTDLG_HXX
#include <fldtdlg.hxx>
#endif
#ifndef _FLDPAGE_HXX
#include <fldpage.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#ifndef _SFX_BINDINGS_HXX
#include <sfx2/bindings.hxx>
#endif

using namespace ::com::sun::star;

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SwFldPage::SwFldPage( Window *pParent, const ResId &rId,
                        const SfxItemSet &rAttrSet )
    :SfxTabPage     (pParent, rId, rAttrSet),
    m_pCurFld       (0),
    m_pWrtShell     (0),
    m_nPageId       ( static_cast< USHORT >(rId.GetId()) ),
    m_nTypeSel      (LISTBOX_ENTRY_NOTFOUND),
    m_nSelectionSel (LISTBOX_ENTRY_NOTFOUND),
    m_bFldEdit      (FALSE),
    m_bInsert           (TRUE),
    m_bFldDlgHtmlMode   (FALSE),
    m_bRefresh          (FALSE),
    m_bFirstHTMLInit    (TRUE)
{
//  FreeResource();
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SwFldPage::~SwFldPage()
{
}

/*--------------------------------------------------------------------
    Beschreibung: TabPage initialisieren
 --------------------------------------------------------------------*/

void SwFldPage::Init()
{
    SwDocShell* pDocSh = (SwDocShell*)SfxObjectShell::Current();
    BOOL bNewMode = 0 != (::GetHtmlMode(pDocSh) & HTMLMODE_ON);

    m_bFldEdit = 0 == GetTabDialog();

    // FieldManager neu initialisieren wichtig fuer
    // Dok-Wechsel (fldtdlg:ReInitTabPage)
    m_pCurFld = m_aMgr.GetCurFld();

    if( bNewMode != m_bFldDlgHtmlMode )
    {
        m_bFldDlgHtmlMode = bNewMode;

        // Bereichslistbox initialisieren
        if( m_bFldDlgHtmlMode && m_bFirstHTMLInit )
        {
            m_bFirstHTMLInit = FALSE;
            SwWrtShell *pSh = m_pWrtShell;
            if(! pSh)
                pSh = ::GetActiveWrtShell();
            if(pSh)
            {
                SwDoc* pDoc = pSh->GetDoc();
                pSh->InsertFldType( SwSetExpFieldType( pDoc,
                                    String::CreateFromAscii("HTML_ON"), 1));
                pSh->InsertFldType( SwSetExpFieldType(pDoc,
                                    String::CreateFromAscii("HTML_OFF"), 1));
            }
        }
    }
}

/*--------------------------------------------------------------------
     Beschreibung: Seite neu initialisieren
 --------------------------------------------------------------------*/

void SwFldPage::ActivatePage()
{
    EnableInsert(m_bInsert);
}

/*--------------------------------------------------------------------
     Beschreibung: Kompletter Reset; neues Feld editieren
 --------------------------------------------------------------------*/

void SwFldPage::EditNewField( BOOL bOnlyActivate )
{
    if( !bOnlyActivate )
    {
        m_nTypeSel = LISTBOX_ENTRY_NOTFOUND;
    }
    m_nSelectionSel = LISTBOX_ENTRY_NOTFOUND;
    m_bRefresh = TRUE;
    Reset(*(SfxItemSet*)0);
    m_bRefresh = FALSE;
}

/*--------------------------------------------------------------------
     Beschreibung: Feld einfuegen
 --------------------------------------------------------------------*/

BOOL SwFldPage::InsertFld(USHORT nTypeId, USHORT nSubType, const String& rPar1,
                            const String& rPar2, ULONG nFormatId,
                            sal_Unicode cSeparator, BOOL bIsAutomaticLanguage)
{
    BOOL bRet = FALSE;
    SwView* pView = GetActiveView();
    SwWrtShell *pSh = m_pWrtShell ? m_pWrtShell : pView->GetWrtShellPtr();

    if (!IsFldEdit())   // Neues Feld einfuegen
    {
        SwInsertFld_Data aData(nTypeId, nSubType, rPar1, rPar2, nFormatId, 0, cSeparator, bIsAutomaticLanguage );
        //#i26566# provide parent for SwWrtShell::StartInputFldDlg
        aData.pParent = &GetTabDialog()->GetOKButton();
        bRet = m_aMgr.InsertFld( aData );

        uno::Reference< frame::XDispatchRecorder > xRecorder =
                pView->GetViewFrame()->GetBindings().GetRecorder();
        if ( xRecorder.is() )
        {
            BOOL bRecordDB = TYP_DBFLD == nTypeId ||
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
                        (FN_PARAM_3,String(cSeparator)));
                aReq.AppendItem(SfxUInt16Item(FN_PARAM_FIELD_SUBTYPE, nSubType));
            }
            aReq.AppendItem(SfxUInt16Item(FN_PARAM_FIELD_TYPE   , nTypeId));
            aReq.AppendItem(SfxStringItem(FN_PARAM_FIELD_CONTENT, rPar2));
            aReq.AppendItem(SfxUInt32Item(FN_PARAM_FIELD_FORMAT , nFormatId));
            aReq.Done();
        }

    }
    else    // Feld aendern
    {
        SwField * pTmpFld = m_pCurFld->Copy();

        String sPar1(rPar1);
        String sPar2(rPar2);
        BOOL bDBChanged = FALSE;
        switch( nTypeId )
        {
        case TYP_DATEFLD:
        case TYP_TIMEFLD:
            nSubType = static_cast< USHORT >(((nTypeId == TYP_DATEFLD) ? DATEFLD : TIMEFLD) |
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
                bDBChanged = TRUE;
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

                SwClientIter aIter( *pOldTyp );

                for( SwFmtFld* pFmtFld = (SwFmtFld*)aIter.First( TYPE(SwFmtFld) );
                    pFmtFld; pFmtFld = (SwFmtFld*)aIter.Next() )
                {
                    if( pFmtFld->GetFld() == m_pCurFld)
                    {
                        pTyp->Add(pFmtFld); // Feld auf neuen Typ umhaengen
                        pTmpFld->ChgTyp(pTyp);
                        break;
                    }
                }
                bDBChanged = TRUE;
            }
            break;

        case TYP_SEQFLD:
            {
                SwSetExpFieldType* pTyp = (SwSetExpFieldType*)pTmpFld->GetTyp();
                pTyp->SetOutlineLvl( static_cast< BYTE >(nSubType & 0xff));
                pTyp->SetDelimiter(cSeparator);

                nSubType = nsSwGetSetExpType::GSE_SEQ;
            }
            break;

        case TYP_INPUTFLD:
            {
                // User- oder SetField ?
                if (m_aMgr.GetFldType(RES_USERFLD, sPar1) == 0 &&
                !(pTmpFld->GetSubType() & INP_TXT)) // SETEXPFLD
                {
                    SwSetExpField* pFld = (SwSetExpField*)pTmpFld;
                    pFld->SetPromptText(sPar2);
                    sPar2 = pFld->GetPar2();
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

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

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

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwFldPage::RestorePos(ListBox* pLst1, ListBox* pLst2, ListBox* pLst3)
{
    USHORT nPos = 0;
    ListBox* aLBArr [ coLBCount ] = { pLst1, pLst2, pLst3 };
    ListBox** ppLB = aLBArr;
    for( int i = 0; i < coLBCount; ++i, ++ppLB )
        if( (*ppLB) && (*ppLB)->GetEntryCount() && m_aLstStrArr[ i ].Len() &&
            LISTBOX_ENTRY_NOTFOUND !=
                        ( nPos = (*ppLB)->GetEntryPos(m_aLstStrArr[ i ] ) ) )
            (*ppLB)->SelectEntryPos( nPos );
}

/*--------------------------------------------------------------------
     Beschreibung: Einfuegen von neuen Feldern
 --------------------------------------------------------------------*/

IMPL_LINK( SwFldPage, InsertHdl, Button *, pBtn )
{
    SwFldDlg *pDlg = (SwFldDlg*)GetTabDialog();

    if (pDlg)
    {
        pDlg->InsertHdl();

        if (pBtn)
            pBtn->GrabFocus();  // Wegen InputField-Dlg
    }
    else
    {
        SwFldEditDlg *pEditDlg = (SwFldEditDlg *)GetParent();
        pEditDlg->InsertHdl();
    }

    return 0;
}

/*--------------------------------------------------------------------
     Beschreibung: "Einfuegen"-Button Enablen/Disablen
 --------------------------------------------------------------------*/

void SwFldPage::EnableInsert(BOOL bEnable)
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

/*--------------------------------------------------------------------
     Beschreibung:
 --------------------------------------------------------------------*/

IMPL_LINK( SwFldPage, NumFormatHdl, ListBox *, EMPTYARG )
{
    InsertHdl();

    return 0;
}
/*-- 19.12.2005 14:05:47---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwFldPage::SetWrtShell( SwWrtShell* pShell )
{
    m_pWrtShell = pShell;
    m_aMgr.SetWrtShell( pShell );
}
