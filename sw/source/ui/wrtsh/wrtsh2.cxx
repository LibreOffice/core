/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
#include <IDocumentUndoRedo.hxx>
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
#include <SwRewriter.hxx>

#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>


/*------------------------------------------------------------------------
        Beschreibung:
------------------------------------------------------------------------*/

void SwWrtShell::Insert( SwField& rFld )
{
    ResetCursorStack();
    if(!_CanInsert())
        return;
    StartAllAction();

    SwRewriter aRewriter;
    aRewriter.AddRule(UNDO_ARG1, rFld.GetDescription());

    StartUndo(UNDO_INSERT, &aRewriter);

    bool bDeleted = false;
    const SwPaM* pAnnotationTextRange = NULL;
    if ( HasSelection() )
    {
        if ( rFld.GetTyp()->Which() == RES_POSTITFLD )
        {
            // for annotation fields:
            // - keep the current selection in order to create a corresponding annotation mark
            // - collapse cursur to its point
            const SwPaM& rCurrPaM = GetCurrentShellCursor();
            pAnnotationTextRange = new SwPaM( *rCurrPaM.GetPoint(), *rCurrPaM.GetMark() );
            ClearMark();
        }
        else
        {
            bDeleted = DelRight() != 0;
        }
    }

    SwEditShell::Insert2(rFld, bDeleted);

    if ( pAnnotationTextRange != NULL )
    {
        if ( GetDoc() != NULL )
        {
            IDocumentMarkAccess* pMarksAccess = GetDoc()->getIDocumentMarkAccess();
            pMarksAccess->makeAnnotationMark( *pAnnotationTextRange, ::rtl::OUString() );
        }
        delete pAnnotationTextRange;
    }

    EndUndo();
    EndAllAction();
}

/*--------------------------------------------------------------------
    Beschreibung: Felder Update anschmeissen
 --------------------------------------------------------------------*/



void SwWrtShell::UpdateInputFlds( SwInputFieldList* pLst )
{
    // ueber die Liste der Eingabefelder gehen und Updaten
    SwInputFieldList* pTmp = pLst;
    if( !pTmp )
        pTmp = new SwInputFieldList( this );

    const sal_uInt16 nCnt = pTmp->Count();
    if(nCnt)
    {
        pTmp->PushCrsr();

        sal_Bool bCancel = sal_False;
        ByteString aDlgPos;
        for( sal_uInt16 i = 0; i < nCnt && !bCancel; ++i )
        {
            pTmp->GotoFieldPos( i );
            SwField* pField = pTmp->GetField( i );
            if(pField->GetTyp()->Which() == RES_DROPDOWN)
                bCancel = StartDropDownFldDlg( pField, sal_True, &aDlgPos );
            else
                bCancel = StartInputFldDlg( pField, sal_True, 0, &aDlgPos);

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



sal_Bool SwWrtShell::StartInputFldDlg( SwField* pFld, sal_Bool bNextButton,
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
    sal_Bool bRet = RET_CANCEL == pDlg->Execute();
    if(pWindowState)
        *pWindowState = pDlg->GetWindowState();

    delete pDlg;
    GetWin()->Update();
    return bRet;
}
/* -----------------17.06.2003 10:18-----------------

 --------------------------------------------------*/
sal_Bool SwWrtShell::StartDropDownFldDlg(SwField* pFld, sal_Bool bNextButton, ByteString* pWindowState)
{
    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "SwAbstractDialogFactory fail!");

    AbstractDropDownFieldDialog* pDlg = pFact->CreateDropDownFieldDialog( NULL, *this, pFld, DLG_FLD_DROPDOWN ,bNextButton );
    DBG_ASSERT(pDlg, "Dialogdiet fail!");
    if(pWindowState && pWindowState->Len())
        pDlg->SetWindowState(*pWindowState);
    sal_uInt16 nRet = pDlg->Execute();
    if(pWindowState)
        *pWindowState = pDlg->GetWindowState();
    delete pDlg;
    sal_Bool bRet = RET_CANCEL == nRet;
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

sal_Bool SwWrtShell::UpdateTableOf(const SwTOXBase& rTOX, const SfxItemSet* pSet)
{
    sal_Bool bResult = sal_False;

    if(_CanInsert())
    {
        bResult = SwEditShell::UpdateTableOf(rTOX, pSet);

        if (pSet == NULL)
        {
            SwDoc *const pDoc_ = GetDoc();
            if (pDoc_)
            {
                pDoc_->GetIDocumentUndoRedo().DelAllUndoObj();
            }
        }
    }

    return bResult;
}

// handler for click on the field given as parameter.
// the cursor is positioned on the field.


void SwWrtShell::ClickToField( const SwField& rFld )
{
    bIsInClickToEdit = sal_True;
    switch( rFld.GetTyp()->Which() )
    {
    case RES_JUMPEDITFLD:
        {
            sal_uInt16 nSlotId = 0;
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

            Right( CRSR_SKIP_CHARS, sal_True, 1, sal_False );       // Feld selektieren

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
        {
            const SwInputField* pInputField = dynamic_cast<const SwInputField*>(&rFld);
            if ( pInputField == NULL )
            {
                StartInputFldDlg( (SwField*)&rFld, sal_False );
            }
        }
        break;

    case RES_SETEXPFLD:
        if( ((SwSetExpField&)rFld).GetInputFlag() )
            StartInputFldDlg( (SwField*)&rFld, sal_False );
        break;
    case RES_DROPDOWN :
        StartDropDownFldDlg( (SwField*)&rFld, sal_False );
    break;
    }

    bIsInClickToEdit = sal_False;
}


void SwWrtShell::ClickToINetAttr( const SwFmtINetFmt& rItem, sal_uInt16 nFilter )
{
    if( !rItem.GetValue().Len() )
        return ;

    bIsInClickToEdit = sal_True;

    // erstmal das evt. gesetzte ObjectSelect Macro ausfuehren
    const SvxMacro* pMac = rItem.GetMacro( SFX_EVENT_MOUSECLICK_OBJECT );
    if( pMac )
    {
        SwCallMouseEvent aCallEvent;
        aCallEvent.Set( &rItem );
        GetDoc()->CallEvent( SFX_EVENT_MOUSECLICK_OBJECT, aCallEvent, sal_False );
    }

    // damit die Vorlagenumsetzung sofort angezeigt wird
    ::LoadURL( rItem.GetValue(), this, nFilter, &rItem.GetTargetFrame() );
    const SwTxtINetFmt* pTxtAttr = rItem.GetTxtINetFmt();
    if( pTxtAttr )
    {
        const_cast<SwTxtINetFmt*>(pTxtAttr)->SetVisited( true );
        const_cast<SwTxtINetFmt*>(pTxtAttr)->SetVisitedValid( true );
    }

    bIsInClickToEdit = sal_False;
}



sal_Bool SwWrtShell::ClickToINetGrf( const Point& rDocPt, sal_uInt16 nFilter )
{
    sal_Bool bRet = sal_False;
    String sURL;
    String sTargetFrameName;
    const SwFrmFmt* pFnd = IsURLGrfAtPos( rDocPt, &sURL, &sTargetFrameName );
    if( pFnd && sURL.Len() )
    {
        bRet = sal_True;
        // erstmal das evt. gesetzte ObjectSelect Macro ausfuehren
        const SvxMacro* pMac = &pFnd->GetMacro().GetMacro( SFX_EVENT_MOUSECLICK_OBJECT );
        if( pMac )
        {
            SwCallMouseEvent aCallEvent;
            aCallEvent.Set( EVENT_OBJECT_URLITEM, pFnd );
            GetDoc()->CallEvent( SFX_EVENT_MOUSECLICK_OBJECT, aCallEvent, sal_False );
        }

        ::LoadURL( sURL, this, nFilter, &sTargetFrameName);
    }
    return bRet;
}


void LoadURL( const String& rURL, ViewShell* pVSh, sal_uInt16 nFilter,
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

    SfxBoolItem aNewView( SID_OPEN_NEW_VIEW, sal_False );
    //#39076# Silent kann lt. SFX entfernt werden.
//  SfxBoolItem aSilent( SID_SILENT, sal_True );
    SfxBoolItem aBrowse( SID_BROWSE, sal_True );

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
                                    const sal_uInt16 nAction )
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
            sal_Bool bDoesUndo = DoesUndo();
            SwUndoId nLastUndoId(UNDO_EMPTY);
            if (GetLastUndoInfo(0, & nLastUndoId))
            {
                if (UNDO_INSSECTION != nLastUndoId)
                {
                    DoUndo(false);
                }
            }
            UpdateSection( GetSectionFmtPos( *pIns->GetFmt() ), aSection );
            DoUndo( bDoesUndo );
        }
    }
}


