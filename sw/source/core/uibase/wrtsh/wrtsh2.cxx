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

#include <hintids.hxx>
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
#include <swtypes.hxx>
#include <wrtsh.hxx>
#include <docsh.hxx>
#include <fldbas.hxx>
#include <expfld.hxx>
#include <ddefld.hxx>
#include <docufld.hxx>
#include <reffld.hxx>
#include <swundo.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <viewopt.hxx>
#include <frmfmt.hxx>
#include <swtable.hxx>
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

#include <xmloff/odffields.hxx>

void SwWrtShell::Insert(SwField &rFld)
{
    ResetCursorStack();
    if(!CanInsert())
        return;
    StartAllAction();

    SwRewriter aRewriter;
    aRewriter.AddRule(UndoArg1, rFld.GetDescription());

    StartUndo(UNDO_INSERT, &aRewriter);

    bool bDeleted = false;
    const SwPaM* pAnnotationTextRange = NULL;
    if ( HasSelection() )
    {
        if ( rFld.GetTyp()->Which() == RES_POSTITFLD )
        {
            // for annotation fields:
            // - keep the current selection in order to create a corresponding annotation mark
            // - collapse cursor to its end
            if ( IsTableMode() )
            {
                GetTblCrs()->Normalize( false );
                const SwPosition rStartPos( *(GetTblCrs()->GetMark()->nNode.GetNode().GetCntntNode()), 0 );
                KillPams();
                EndPara();
                const SwPosition rEndPos( *GetCurrentShellCursor().GetPoint() );
                pAnnotationTextRange = new SwPaM( rStartPos, rEndPos );
            }
            else
            {
                NormalizePam( sal_False );
                const SwPaM& rCurrPaM = GetCurrentShellCursor();
                pAnnotationTextRange = new SwPaM( *rCurrPaM.GetPoint(), *rCurrPaM.GetMark() );
                ClearMark();
            }
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

// Start the field update

void SwWrtShell::UpdateInputFlds( SwInputFieldList* pLst )
{
    // Go through the list of fields and updating
    SwInputFieldList* pTmp = pLst;
    if( !pTmp )
        pTmp = new SwInputFieldList( this );

    const sal_uInt16 nCnt = pTmp->Count();
    if(nCnt)
    {
        pTmp->PushCrsr();

        sal_Bool bCancel = sal_False;
        OString aDlgPos;
        for( sal_uInt16 i = 0; i < nCnt && !bCancel; ++i )
        {
            pTmp->GotoFieldPos( i );
            SwField* pField = pTmp->GetField( i );
            if(pField->GetTyp()->Which() == RES_DROPDOWN)
                bCancel = StartDropDownFldDlg( pField, sal_True, &aDlgPos );
            else
                bCancel = StartInputFldDlg( pField, sal_True, 0, &aDlgPos);

            if (!bCancel)
            {
                // Otherwise update error at multi-selection:
                pTmp->GetField( i )->GetTyp()->UpdateFlds();
            }
        }
        pTmp->PopCrsr();
    }

    if( !pLst )
        delete pTmp;
}

// Listener class: will close InputField dialog if input field(s)
// is(are) deleted (for instance, by an extension) after the dialog shows up.
// Otherwise, the for loop in SwWrtShell::UpdateInputFlds will crash when doing:
//         'pTmp->GetField( i )->GetTyp()->UpdateFlds();'
// on a deleted field.
class FieldDeletionModify : public SwModify
{
    public:
        FieldDeletionModify(AbstractFldInputDlg* pInputFieldDlg) : mpInputFieldDlg(pInputFieldDlg) {}

        void Modify( const SfxPoolItem* pOld, const SfxPoolItem *)
        {
            // Input fields have been deleted: better to close the dialog
            if (pOld->Which() == RES_FIELD_DELETED)
            {
                mpInputFieldDlg->EndDialog(RET_CANCEL);
            }
        }
    private:
        AbstractFldInputDlg* mpInputFieldDlg;
};

// Start input dialog for a specific field

sal_Bool SwWrtShell::StartInputFldDlg( SwField* pFld, sal_Bool bNextButton,
                                    Window* pParentWin, OString* pWindowState )
{

    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "Dialogdiet fail!");
    AbstractFldInputDlg* pDlg = pFact->CreateFldInputDlg(pParentWin, *this, pFld, bNextButton);
    OSL_ENSURE(pDlg, "Dialogdiet fail!");
    if(pWindowState && !pWindowState->isEmpty())
        pDlg->SetWindowState(*pWindowState);

    // Register for possible input field deletion while dialog is open
    FieldDeletionModify aModify(pDlg);
    GetDoc()->GetUnoCallBack()->Add(&aModify);

    sal_Bool bRet = RET_CANCEL == pDlg->Execute();

    // Dialog closed, remove modification listener
    GetDoc()->GetUnoCallBack()->Remove(&aModify);

    if(pWindowState)
        *pWindowState = pDlg->GetWindowState();

    delete pDlg;
    GetWin()->Update();
    return bRet;
}

sal_Bool SwWrtShell::StartDropDownFldDlg(SwField* pFld, sal_Bool bNextButton, OString* pWindowState)
{
    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

    AbstractDropDownFieldDialog* pDlg = pFact->CreateDropDownFieldDialog(NULL, *this, pFld, bNextButton);
    OSL_ENSURE(pDlg, "Dialogdiet fail!");
    if(pWindowState && !pWindowState->isEmpty())
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

// Insert directory - remove selection

void SwWrtShell::InsertTableOf(const SwTOXBase& rTOX, const SfxItemSet* pSet)
{
    if(!CanInsert())
        return;

    if(HasSelection())
        DelRight();

    SwEditShell::InsertTableOf(rTOX, pSet);
}

// Update directory - remove selection

sal_Bool SwWrtShell::UpdateTableOf(const SwTOXBase& rTOX, const SfxItemSet* pSet)
{
    sal_Bool bResult = sal_False;

    if(CanInsert())
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
    // cross reference field must not be selected because it moves the cursor
    if (RES_GETREFFLD != rFld.GetTyp()->Which())
    {
        StartAllAction();
        Right( CRSR_SKIP_CHARS, true, 1, false ); // Select the field.
        NormalizePam();
        EndAllAction();
    }

    bIsInClickToEdit = true;
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

            }

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
            const OUString sText( rFld.GetPar2() );
            OUString sRet( sText );
            ExecMacro( pFld->GetSvxMacro(), &sRet );

            // return value changed?
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
    default:
        SAL_WARN_IF(rFld.IsClickable(), "sw", "unhandled clickable field!");
    }

    bIsInClickToEdit = false;
}

void SwWrtShell::ClickToINetAttr( const SwFmtINetFmt& rItem, sal_uInt16 nFilter )
{
    if( rItem.GetValue().isEmpty() )
        return ;

    bIsInClickToEdit = true;

    // At first run the possibly set ObjectSelect Macro
    const SvxMacro* pMac = rItem.GetMacro( SFX_EVENT_MOUSECLICK_OBJECT );
    if( pMac )
    {
        SwCallMouseEvent aCallEvent;
        aCallEvent.Set( &rItem );
        GetDoc()->CallEvent( SFX_EVENT_MOUSECLICK_OBJECT, aCallEvent, false );
    }

    // So that the implementation of templates is displayed immediately
    ::LoadURL( *this, rItem.GetValue(), nFilter, rItem.GetTargetFrame() );
    const SwTxtINetFmt* pTxtAttr = rItem.GetTxtINetFmt();
    if( pTxtAttr )
    {
        const_cast<SwTxtINetFmt*>(pTxtAttr)->SetVisited( true );
        const_cast<SwTxtINetFmt*>(pTxtAttr)->SetVisitedValid( true );
    }

    bIsInClickToEdit = false;
}

bool SwWrtShell::ClickToINetGrf( const Point& rDocPt, sal_uInt16 nFilter )
{
    bool bRet = false;
    OUString sURL;
    OUString sTargetFrameName;
    const SwFrmFmt* pFnd = IsURLGrfAtPos( rDocPt, &sURL, &sTargetFrameName );
    if( pFnd && !sURL.isEmpty() )
    {
        bRet = true;
        // At first run the possibly set ObjectSelect Macro
        const SvxMacro* pMac = &pFnd->GetMacro().GetMacro( SFX_EVENT_MOUSECLICK_OBJECT );
        if( pMac )
        {
            SwCallMouseEvent aCallEvent;
            aCallEvent.Set( EVENT_OBJECT_URLITEM, pFnd );
            GetDoc()->CallEvent( SFX_EVENT_MOUSECLICK_OBJECT, aCallEvent, false );
        }

        ::LoadURL(*this, sURL, nFilter, sTargetFrameName);
    }
    return bRet;
}

void LoadURL( SwViewShell& rVSh, const OUString& rURL, sal_uInt16 nFilter,
              const OUString& rTargetFrameName )
{
    OSL_ENSURE( !rURL.isEmpty(), "what should be loaded here?" );
    if( rURL.isEmpty() )
        return ;

    // The shell could be 0 also!!!!!
    if ( !rVSh.ISA(SwCrsrShell) )
        return;

    //A CrsrShell is always a WrtShell
    SwWrtShell &rSh = (SwWrtShell&)rVSh;

    SwDocShell* pDShell = rSh.GetView().GetDocShell();
    OSL_ENSURE( pDShell, "No DocShell?!");
    OUString sTargetFrame(rTargetFrameName);
    if (sTargetFrame.isEmpty() && pDShell)
    {
        using namespace ::com::sun::star;
        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
            pDShell->GetModel(), uno::UNO_QUERY_THROW);
        uno::Reference<document::XDocumentProperties> xDocProps
            = xDPS->getDocumentProperties();
        sTargetFrame = xDocProps->getDefaultTarget();
    }

    OUString sReferer;
    if( pDShell && pDShell->GetMedium() )
        sReferer = pDShell->GetMedium()->GetName();
    SfxViewFrame* pViewFrm = rSh.GetView().GetViewFrame();
    SfxFrameItem aView( SID_DOCFRAME, pViewFrm );
    SfxStringItem aName( SID_FILE_NAME, rURL );
    SfxStringItem aTargetFrameName( SID_TARGETNAME, sTargetFrame );
    SfxStringItem aReferer( SID_REFERER, sReferer );

    SfxBoolItem aNewView( SID_OPEN_NEW_VIEW, false );
    //#39076# Silent can be removed accordingly to SFX.
    SfxBoolItem aBrowse( SID_BROWSE, true );

    if( nFilter & URLLOAD_NEWVIEW )
        aTargetFrameName.SetValue( OUString("_blank") );

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
        // Insert
        OUString sURL = rBkmk.GetURL();
        // Is this is a jump within the current Doc?
        const SwDocShell* pDocShell = GetView().GetDocShell();
        if(pDocShell->HasName())
        {
            const OUString rName = pDocShell->GetMedium()->GetURLObject().GetURLNoMark();

            if (sURL.startsWith(rName))
            {
                if (sURL.getLength()>rName.getLength())
                {
                    sURL = sURL.copy(rName.getLength());
                }
                else
                {
                    sURL = OUString();
                }
            }
        }
        SwFmtINetFmt aFmt( sURL, OUString() );
        InsertURL( aFmt, rBkmk.GetDescription() );
    }
    else
    {
        SwSectionData aSection( FILE_LINK_SECTION, GetUniqueSectionName() );
        OUString aLinkFile( rBkmk.GetURL().getToken(0, '#') );
        aLinkFile += OUString(sfx2::cTokenSeparator);
        aLinkFile += OUString(sfx2::cTokenSeparator);
        aLinkFile += rBkmk.GetURL().getToken(1, '#');
        aSection.SetLinkFileName( aLinkFile );
        aSection.SetProtectFlag( true );
        const SwSection* pIns = InsertSection( aSection );
        if( EXCHG_IN_ACTION_MOVE == nAction && pIns )
        {
            aSection = SwSectionData(*pIns);
            aSection.SetLinkFileName( OUString() );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
