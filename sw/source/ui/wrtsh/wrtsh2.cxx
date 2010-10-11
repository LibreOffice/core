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

#include <hintids.hxx>      // define ITEMIDs
#include <svl/macitem.hxx>
#include <sfx2/frame.hxx>
#include <vcl/msgbox.hxx>
#include <svl/urihelper.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/linkmgr.hxx>
#include <fmtinfmt.hxx>
#include <frmatr.hxx>
#include <swtypes.hxx>      // SET_CURR_SHELL
#include <wrtsh.hxx>
#include <docsh.hxx>
#include <fldbas.hxx>       // Felder
#include <expfld.hxx>
#include <ddefld.hxx>
#include <docufld.hxx>
#include <reffld.hxx>
#include <swundo.hxx>
#include <doc.hxx>
#include <viewopt.hxx>      // SwViewOptions
#include <frmfmt.hxx>       // fuer UpdateTable
#include <swtable.hxx>      // fuer UpdateTable
#include <mdiexp.hxx>
#include <view.hxx>
#include <swevent.hxx>
#include <poolfmt.hxx>
#include <section.hxx>
#include <navicont.hxx>
#include <navipi.hxx>
#include <crsskip.hxx>
#include <txtinet.hxx>
#include <cmdid.h>
#include <wrtsh.hrc>
#include "swabstdlg.hxx"
#include "fldui.hrc"

#include <undobj.hxx>

#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>


/*------------------------------------------------------------------------
        Beschreibung:
------------------------------------------------------------------------*/

void SwWrtShell::Insert(SwField &rFld)
{
    ResetCursorStack();
    if(!_CanInsert())
        return;
    StartAllAction();

    SwRewriter aRewriter;
    aRewriter.AddRule(UNDO_ARG1, rFld.GetDescription());

    StartUndo(UNDO_INSERT, &aRewriter);

    bool bDeleted = false;
    if( HasSelection() )
    {
        bDeleted = DelRight() != 0;
    }

    SwEditShell::Insert2(rFld, bDeleted);
    EndUndo(UNDO_INSERT);
    EndAllAction();
}

/*--------------------------------------------------------------------
    Beschreibung: Felder Update anschmeissen
 --------------------------------------------------------------------*/



void SwWrtShell::UpdateInputFlds( SwInputFieldList* pLst, BOOL bOnlyInSel )
{
    // ueber die Liste der Eingabefelder gehen und Updaten
    SwInputFieldList* pTmp = pLst;
    if( !pTmp )
        pTmp = new SwInputFieldList( this );

    if (bOnlyInSel)
        pTmp->RemoveUnselectedFlds();

    const USHORT nCnt = pTmp->Count();
    if(nCnt)
    {
        pTmp->PushCrsr();

        BOOL bCancel = FALSE;
        ByteString aDlgPos;
        for( USHORT i = 0; i < nCnt && !bCancel; ++i )
        {
            pTmp->GotoFieldPos( i );
            SwField* pField = pTmp->GetField( i );
            if(pField->GetTyp()->Which() == RES_DROPDOWN)
                bCancel = StartDropDownFldDlg( pField, TRUE, &aDlgPos );
            else
                bCancel = StartInputFldDlg( pField, TRUE, 0, &aDlgPos);

            // Sonst Updatefehler bei Multiselektion:
            pTmp->GetField( i )->GetTyp()->UpdateFlds();
        }
        pTmp->PopCrsr();
    }

    if( !pLst )
        delete pTmp;
}


/*--------------------------------------------------------------------
    Beschreibung: EingabeDialog fuer ein bestimmtes Feld starten
 --------------------------------------------------------------------*/



BOOL SwWrtShell::StartInputFldDlg( SwField* pFld, BOOL bNextButton,
                                    Window* pParentWin, ByteString* pWindowState )
{
//JP 14.08.96: Bug 30332 - nach Umbau der modularietaet im SFX, muss jetzt
//              das TopWindow der Application benutzt werden.
//  SwFldInputDlg* pDlg = new SwFldInputDlg( GetWin(), *this, pFld );

    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "Dialogdiet fail!");
    AbstractFldInputDlg* pDlg = pFact->CreateFldInputDlg( DLG_FLD_INPUT,
                                                        pParentWin, *this, pFld, bNextButton);
    DBG_ASSERT(pDlg, "Dialogdiet fail!");
    if(pWindowState && pWindowState->Len())
        pDlg->SetWindowState(*pWindowState);
    BOOL bRet = RET_CANCEL == pDlg->Execute();
    if(pWindowState)
        *pWindowState = pDlg->GetWindowState();

    delete pDlg;
    GetWin()->Update();
    return bRet;
}
/* -----------------17.06.2003 10:18-----------------

 --------------------------------------------------*/
BOOL SwWrtShell::StartDropDownFldDlg(SwField* pFld, BOOL bNextButton, ByteString* pWindowState)
{
    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "SwAbstractDialogFactory fail!");

    AbstractDropDownFieldDialog* pDlg = pFact->CreateDropDownFieldDialog( NULL, *this, pFld, DLG_FLD_DROPDOWN ,bNextButton );
    DBG_ASSERT(pDlg, "Dialogdiet fail!");
    if(pWindowState && pWindowState->Len())
        pDlg->SetWindowState(*pWindowState);
    USHORT nRet = pDlg->Execute();
    if(pWindowState)
        *pWindowState = pDlg->GetWindowState();
    delete pDlg;
    BOOL bRet = RET_CANCEL == nRet;
    GetWin()->Update();
    if(RET_YES == nRet)
    {
        GetView().GetViewFrame()->GetDispatcher()->Execute(FN_EDIT_FIELD, SFX_CALLMODE_SYNCHRON);
    }
    return bRet;
}

/*--------------------------------------------------------------------
    Beschreibung: Verzeichnis einfuegen Selektion loeschen
 --------------------------------------------------------------------*/



void SwWrtShell::InsertTableOf(const SwTOXBase& rTOX, const SfxItemSet* pSet)
{
    if(!_CanInsert())
        return;

    if(HasSelection())
        DelRight();

    SwEditShell::InsertTableOf(rTOX, pSet);
}


/*--------------------------------------------------------------------
    Beschreibung: Verzeichnis Updaten Selektion loeschen
 --------------------------------------------------------------------*/

BOOL SwWrtShell::UpdateTableOf(const SwTOXBase& rTOX, const SfxItemSet* pSet)
{
    BOOL bResult = FALSE;

    if(_CanInsert())
    {
        bResult = SwEditShell::UpdateTableOf(rTOX, pSet);

        if (pSet == NULL)
        {
            SwDoc * _pDoc = GetDoc();

            if (_pDoc != NULL)
                _pDoc->DelAllUndoObj();
        }
    }

    return bResult;
}

// handler for click on the field given as parameter.
// the cursor is positioned on the field.


void SwWrtShell::ClickToField( const SwField& rFld )
{
    bIsInClickToEdit = TRUE;
    switch( rFld.GetTyp()->Which() )
    {
    case RES_JUMPEDITFLD:
        {
            USHORT nSlotId = 0;
            switch( rFld.GetFormat() )
            {
            case JE_FMT_TABLE:
                nSlotId = FN_INSERT_TABLE;
                break;

            case JE_FMT_FRAME:
                nSlotId = FN_INSERT_FRAME;
                break;

            case JE_FMT_GRAPHIC:    nSlotId = SID_INSERT_GRAPHIC;       break;
            case JE_FMT_OLE:        nSlotId = SID_INSERT_OBJECT;        break;

//          case JE_FMT_TEXT:
            }

            Right( CRSR_SKIP_CHARS, TRUE, 1, FALSE );       // Feld selektieren

            if( nSlotId )
            {
                StartUndo( UNDO_START );
                //#97295# immediately select the right shell
                GetView().StopShellTimer();
                GetView().GetViewFrame()->GetDispatcher()->Execute( nSlotId,
                            SFX_CALLMODE_SYNCHRON|SFX_CALLMODE_RECORD );
                EndUndo( UNDO_END );
            }
        }
        break;

    case RES_MACROFLD:
        {
            const SwMacroField *pFld = (const SwMacroField*)&rFld;
            String sText( rFld.GetPar2() );
            String sRet( sText );
            ExecMacro( pFld->GetSvxMacro(), &sRet );

            // return Wert veraendert?
            if( sRet != sText )
            {
                StartAllAction();
                ((SwField&)rFld).SetPar2( sRet );
                ((SwField&)rFld).GetTyp()->UpdateFlds();
                EndAllAction();
            }
        }
        break;

    case RES_GETREFFLD:
        StartAllAction();
        SwCrsrShell::GotoRefMark( ((SwGetRefField&)rFld).GetSetRefName(),
                                    ((SwGetRefField&)rFld).GetSubType(),
                                    ((SwGetRefField&)rFld).GetSeqNo() );
        EndAllAction();
        break;

    case RES_INPUTFLD:
        StartInputFldDlg( (SwField*)&rFld, FALSE );
        break;

    case RES_SETEXPFLD:
        if( ((SwSetExpField&)rFld).GetInputFlag() )
            StartInputFldDlg( (SwField*)&rFld, FALSE );
        break;
    case RES_DROPDOWN :
        StartDropDownFldDlg( (SwField*)&rFld, FALSE );
    break;
    }

    bIsInClickToEdit = FALSE;
}



void SwWrtShell::ClickToINetAttr( const SwFmtINetFmt& rItem, USHORT nFilter )
{
    if( !rItem.GetValue().Len() )
        return ;

    bIsInClickToEdit = TRUE;

    // erstmal das evt. gesetzte ObjectSelect Macro ausfuehren
    const SvxMacro* pMac = rItem.GetMacro( SFX_EVENT_MOUSECLICK_OBJECT );
    if( pMac )
    {
        SwCallMouseEvent aCallEvent;
        aCallEvent.Set( &rItem );
        GetDoc()->CallEvent( SFX_EVENT_MOUSECLICK_OBJECT, aCallEvent, FALSE );
    }

    // damit die Vorlagenumsetzung sofort angezeigt wird
    ::LoadURL( rItem.GetValue(), this, nFilter, &rItem.GetTargetFrame() );
    const SwTxtINetFmt* pTxtAttr = rItem.GetTxtINetFmt();
    if( pTxtAttr )
    {
        const_cast<SwTxtINetFmt*>(pTxtAttr)->SetVisited( true );
        const_cast<SwTxtINetFmt*>(pTxtAttr)->SetVisitedValid( true );
    }

    bIsInClickToEdit = FALSE;
}



BOOL SwWrtShell::ClickToINetGrf( const Point& rDocPt, USHORT nFilter )
{
    BOOL bRet = FALSE;
    String sURL;
    String sTargetFrameName;
    const SwFrmFmt* pFnd = IsURLGrfAtPos( rDocPt, &sURL, &sTargetFrameName );
    if( pFnd && sURL.Len() )
    {
        bRet = TRUE;
        // erstmal das evt. gesetzte ObjectSelect Macro ausfuehren
        const SvxMacro* pMac = &pFnd->GetMacro().GetMacro( SFX_EVENT_MOUSECLICK_OBJECT );
        if( pMac )
        {
            SwCallMouseEvent aCallEvent;
            aCallEvent.Set( EVENT_OBJECT_URLITEM, pFnd );
            GetDoc()->CallEvent( SFX_EVENT_MOUSECLICK_OBJECT, aCallEvent, FALSE );
        }

        ::LoadURL( sURL, this, nFilter, &sTargetFrameName);
    }
    return bRet;
}


void LoadURL( const String& rURL, ViewShell* pVSh, USHORT nFilter,
              const String *pTargetFrameName )
{
    ASSERT( rURL.Len() && pVSh, "was soll hier geladen werden?" );
    if( !rURL.Len() || !pVSh )
        return ;

    // die Shell kann auch 0 sein !!!!!
    SwWrtShell *pSh = 0;
    if ( pVSh && pVSh->ISA(SwCrsrShell) )
    {
        //Eine CrsrShell ist auch immer eine WrtShell
        pSh = (SwWrtShell*)pVSh;
    }
    else
        return;

    SwDocShell* pDShell = pSh->GetView().GetDocShell();
    DBG_ASSERT( pDShell, "No DocShell?!");
    String sTargetFrame;
    if( pTargetFrameName && pTargetFrameName->Len() )
        sTargetFrame = *pTargetFrameName;
    else if( pDShell ) {
        using namespace ::com::sun::star;
        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
            pDShell->GetModel(), uno::UNO_QUERY_THROW);
        uno::Reference<document::XDocumentProperties> xDocProps
            = xDPS->getDocumentProperties();
        sTargetFrame = xDocProps->getDefaultTarget();
    }

    String sReferer;
    if( pDShell && pDShell->GetMedium() )
        sReferer = pDShell->GetMedium()->GetName();
    SfxViewFrame* pViewFrm = pSh->GetView().GetViewFrame();
    SfxFrameItem aView( SID_DOCFRAME, pViewFrm );
    SfxStringItem aName( SID_FILE_NAME, rURL );
    SfxStringItem aTargetFrameName( SID_TARGETNAME, sTargetFrame );
    SfxStringItem aReferer( SID_REFERER, sReferer );

    SfxBoolItem aNewView( SID_OPEN_NEW_VIEW, FALSE );
    //#39076# Silent kann lt. SFX entfernt werden.
//  SfxBoolItem aSilent( SID_SILENT, TRUE );
    SfxBoolItem aBrowse( SID_BROWSE, TRUE );

    if( nFilter & URLLOAD_NEWVIEW )
        aTargetFrameName.SetValue( String::CreateFromAscii("_blank") );

    const SfxPoolItem* aArr[] = {
                &aName,
                &aNewView, /*&aSilent,*/
                &aReferer,
                &aView, &aTargetFrameName,
                &aBrowse,
                0L
    };

    pViewFrm->GetDispatcher()->GetBindings()->Execute( SID_OPENDOC, aArr,
            SFX_CALLMODE_ASYNCHRON|SFX_CALLMODE_RECORD );
}

void SwWrtShell::NavigatorPaste( const NaviContentBookmark& rBkmk,
                                    const USHORT nAction )
{
    if( EXCHG_IN_ACTION_COPY == nAction )
    {
        // Einfuegen
        String sURL = rBkmk.GetURL();
        //handelt es sich um ein Sprung innerhalb des akt. Docs?
        const SwDocShell* pDocShell = GetView().GetDocShell();
        if(pDocShell->HasName())
        {
            const String rName = pDocShell->GetMedium()->GetURLObject().GetURLNoMark();

            if(COMPARE_EQUAL == sURL.CompareTo(rName, rName.Len()))
                sURL.Erase(0, rName.Len());
        }
        SwFmtINetFmt aFmt( sURL, aEmptyStr );
        InsertURL( aFmt, rBkmk.GetDescription() );
    }
    else
    {
        SwSectionData aSection( FILE_LINK_SECTION, GetUniqueSectionName( 0 ) );
        String aLinkFile( rBkmk.GetURL().GetToken(0, '#') );
        aLinkFile += sfx2::cTokenSeperator;
        aLinkFile += sfx2::cTokenSeperator;
        aLinkFile += rBkmk.GetURL().GetToken(1, '#');
        aSection.SetLinkFileName( aLinkFile );
        aSection.SetProtectFlag( true );
        const SwSection* pIns = InsertSection( aSection );
        if( EXCHG_IN_ACTION_MOVE == nAction && pIns )
        {
            aSection = SwSectionData(*pIns);
            aSection.SetLinkFileName( aEmptyStr );
            aSection.SetType( CONTENT_SECTION );
            aSection.SetProtectFlag( false );

            // the update of content from linked section at time delete
            // the undostack. Then the change of the section dont create
            // any undoobject. -  BUG 69145
            BOOL bDoesUndo = DoesUndo();
            if( UNDO_INSSECTION != GetUndoIds() )
                DoUndo( FALSE );
            UpdateSection( GetSectionFmtPos( *pIns->GetFmt() ), aSection );
            DoUndo( bDoesUndo );
        }
    }
}


