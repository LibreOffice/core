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


#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/DispatchHelper.hpp>

#include <comphelper/componentcontext.hxx>
#include <comphelper/processfactory.hxx>

#include <hintids.hxx>
#include <tools/urlobj.hxx>
#include <unotools/tempfile.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/msgbox.hxx>
#include <svl/lckbitem.hxx>
#include <svl/eitem.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <unotools/pathoptions.hxx>
#include <svtools/transfer.hxx>
#include <sfx2/dialogs.hrc>
#include <sfx2/dinfdlg.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/new.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/evntconf.hxx>
#include <svtools/sfxecode.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <svx/svxids.hrc>
#include <svx/drawitem.hxx>
#include <editeng/svxacorr.hxx>
#include <editeng/langitem.hxx>
#include <svx/fmshell.hxx>
#include <sfx2/linkmgr.hxx>

#include <svtools/htmlcfg.hxx>
#include <svx/ofaitem.hxx>
#include <SwSmartTagMgr.hxx>
#include <sfx2/app.hxx>
#include <basic/sbstar.hxx>
#include <basic/basmgr.hxx>
#include <sot/storage.hxx>
#include <sot/clsids.hxx>
#include <fmtcol.hxx>
#include <swevent.hxx>
#include <view.hxx>         // for the current view
#include <docsh.hxx>        // creation of documents
#include <docary.hxx>
#include <wrtsh.hxx>
#include <fldbas.hxx>
#include <viewopt.hxx>
#include <globdoc.hxx>
#include <fldwrap.hxx>
#include <redlndlg.hxx>
#include <docstyle.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <pagedesc.hxx>
#include <shellio.hxx>
#include <pview.hxx>
#include <srcview.hxx>
#include <poolfmt.hxx>
#include <usrpref.hxx>
#include <wdocsh.hxx>
#include <unotxdoc.hxx>
#include <acmplwrd.hxx>
#include <swmodule.hxx>
#include <unobaseclass.hxx>
#include <swwait.hxx>
#include <swcli.hxx>

#include <cmdid.h>
#include <globals.h>
#include <helpid.h>
#include <app.hrc>
#include <poolfmt.hrc>
#include <globals.hrc>
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ListboxControlActions.hpp>
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"
#include <com/sun/star/script/vba/XVBAEventProcessor.hpp>
#include <com/sun/star/script/vba/VBAEventId.hpp>
#include <editeng/acorrcfg.hxx>
#include <SwStyleNameMapper.hxx>

#include <sfx2/fcontnr.hxx>

#include "dialog.hrc"
#include "swabstdlg.hxx"

#include <ndtxt.hxx>    //#outline level,add by zhaojianwei

using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using ::rtl::OUString;
using namespace ::sfx2;
extern sal_Bool FindPhyStyle( SwDoc& , const String& , SfxStyleFamily );

/*--------------------------------------------------------------------
    Description:    create DocInfo (virtual)
 --------------------------------------------------------------------*/

SfxDocumentInfoDialog* SwDocShell::CreateDocumentInfoDialog(
                                Window *pParent, const SfxItemSet &rSet)
{
    SfxDocumentInfoDialog* pDlg = new SfxDocumentInfoDialog(pParent, rSet);
    //only with statistics, when this document is being shown, not
    //from within the Doc-Manager
    SwDocShell* pDocSh = (SwDocShell*) SfxObjectShell::Current();
    if( pDocSh == this )
    {
        //Not for SourceView.
        SfxViewShell *pVSh = SfxViewShell::Current();
        if ( pVSh && !pVSh->ISA(SwSrcView) )
        {
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");
            pDlg->AddTabPage(TP_DOC_STAT, SW_RESSTR(STR_DOC_STAT),pFact->GetTabPageCreatorFunc( TP_DOC_STAT ),0);
        }
    }
    return pDlg;
}

// Disable "multiple layout"

void    SwDocShell::ToggleBrowserMode(sal_Bool bSet, SwView* _pView )
{
    GetDoc()->set(IDocumentSettingAccess::BROWSE_MODE, bSet );
    UpdateFontList();
    SwView* pTempView = _pView ? _pView : (SwView*)GetView();
    if( pTempView )
    {
        pTempView->GetViewFrame()->GetBindings().Invalidate(FN_SHADOWCURSOR);
        if( !GetDoc()->getPrinter( false ) )
            pTempView->SetPrinter( GetDoc()->getPrinter( false ), SFX_PRINTER_PRINTER | SFX_PRINTER_JOBSETUP );
        GetDoc()->CheckDefaultPageFmt();
        SfxViewFrame *pTmpFrm = SfxViewFrame::GetFirst(this, sal_False);
        do {
            if( pTmpFrm != pTempView->GetViewFrame() )
            {
                pTmpFrm->DoClose();
                pTmpFrm = SfxViewFrame::GetFirst(this, sal_False);
            }
            else
                pTmpFrm = pTmpFrm->GetNext(*pTmpFrm, this, sal_False);

        } while ( pTmpFrm );
        const SwViewOption& rViewOptions = *pTempView->GetWrtShell().GetViewOptions();
        pTempView->GetWrtShell().CheckBrowseView( sal_True );
        pTempView->CheckVisArea();
        if( bSet )
        {
            const SvxZoomType eType = (SvxZoomType)rViewOptions.GetZoomType();
            if ( SVX_ZOOM_PERCENT != eType)
                ((SwView*)GetView())->SetZoom( eType );
        }
        pTempView->InvalidateBorder();
        pTempView->SetNewWindowAllowed(!bSet);
    }
}
// End of disabled "multiple layout"

// update text fields on document properties changes
void SwDocShell::DoFlushDocInfo()
{
    if ( !pDoc ) return;

    bool bUnlockView(true);
    if ( pWrtShell ) {
        bUnlockView = !pWrtShell->IsViewLocked();
        pWrtShell->LockView( sal_True );    // lock visible section
        pWrtShell->StartAllAction();
    }

    pDoc->DocInfoChgd();

    if ( pWrtShell ) {
        pWrtShell->EndAllAction();
        if ( bUnlockView ) {
            pWrtShell->LockView( sal_False );
        }
    }
}

static void lcl_processCompatibleSfxHint( const uno::Reference< script::vba::XVBAEventProcessor >& xVbaEvents, const SfxHint& rHint )
{
    using namespace com::sun::star::script::vba::VBAEventId;
    if ( rHint.ISA( SfxEventHint ) )
    {
        uno::Sequence< uno::Any > aArgs;
        sal_uLong nEventId = ((SfxEventHint&)rHint).GetEventId();
        switch( nEventId )
        {
            case SFX_EVENT_CREATEDOC:
                xVbaEvents->processVbaEvent( DOCUMENT_NEW, aArgs );
            break;
            case SFX_EVENT_OPENDOC:
                xVbaEvents->processVbaEvent( DOCUMENT_OPEN, aArgs );
            break;
        }
    }
}

/*--------------------------------------------------------------------
    Description: Notification on DocInfo changes
 --------------------------------------------------------------------*/

void SwDocShell::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if( !pDoc )
    {
        return ;
    }

    uno::Reference< script::vba::XVBAEventProcessor > xVbaEvents = pDoc->GetVbaEventProcessor();
    if( xVbaEvents.is() )
        lcl_processCompatibleSfxHint( xVbaEvents, rHint );

    sal_uInt16 nAction = 0;
    if( rHint.ISA(SfxSimpleHint) )
    {
        // switch for more actions
        switch( ((SfxSimpleHint&) rHint).GetId() )
        {
            case SFX_HINT_TITLECHANGED:
                if( GetMedium() )
                    nAction = 2;
            break;
        }
    }
    else if( rHint.ISA(SfxEventHint) &&
        ((SfxEventHint&) rHint).GetEventId() == SFX_EVENT_LOADFINISHED )
    {
        // #i38126# - own action id
        nAction = 3;
    }

    if( nAction )
    {
        sal_Bool bUnlockView = sal_True; //initializing prevents warning
        if( pWrtShell )
        {
            bUnlockView = !pWrtShell->IsViewLocked();
            pWrtShell->LockView( sal_True );    //lock visible section
            pWrtShell->StartAllAction();
        }
        switch( nAction )
        {
        case 2:
            pDoc->GetSysFldType( RES_FILENAMEFLD )->UpdateFlds();
            break;
        // #i38126# - own action for event LOADFINISHED
        // in order to avoid a modified document.
        // #i41679# - Also for the instance of <SwDoc>
        // it has to be assured, that it's not modified.
        // Perform the same as for action id 1, but disable <SetModified>.
        case 3:
            {
                const bool bResetModified = IsEnableSetModified();
                if ( bResetModified )
                    EnableSetModified( sal_False );
                // #i41679#
                const bool bIsDocModified = pDoc->IsModified();

                pDoc->DocInfoChgd( );

                // #i41679#
                if ( !bIsDocModified )
                    pDoc->ResetModified();
                if ( bResetModified )
                    EnableSetModified( sal_True );
            }
            break;
        }

        if( pWrtShell )
        {
            pWrtShell->EndAllAction();
            if( bUnlockView )
                pWrtShell->LockView( sal_False );
        }
    }
}

/*--------------------------------------------------------------------
    Description:    Notification Close Doc
 --------------------------------------------------------------------*/

sal_uInt16 SwDocShell::PrepareClose( sal_Bool bUI, sal_Bool bForBrowsing )
{
    sal_uInt16 nRet = SfxObjectShell::PrepareClose( bUI, bForBrowsing );

    if( sal_True == nRet ) //has to be queried against TRUE! (RET_NEWTASK)
        EndListening( *this );

    if( pDoc && IsInPrepareClose() )
    {
        uno::Reference< script::vba::XVBAEventProcessor > xVbaEvents = pDoc->GetVbaEventProcessor();
        if( xVbaEvents.is() )
        {
            using namespace com::sun::star::script::vba::VBAEventId;
            uno::Sequence< uno::Any > aArgs;
            xVbaEvents->processVbaEvent( DOCUMENT_CLOSE, aArgs );
        }
    }
    return nRet;
}

/*--------------------------------------------------------------------
    Description:    Organizer
 --------------------------------------------------------------------*/

sal_Bool SwDocShell::Insert( SfxObjectShell &rSource,
    sal_uInt16  nSourceIdx1,        // SourcePool: uppermost content level (templates/macros)
    sal_uInt16  nSourceIdx2,        // Index in the content
    sal_uInt16  nSourceIdx3,        // Index in the content level
    sal_uInt16 &rIdx1,              // and the same for the DestinationPool
    sal_uInt16 &rIdx2,              //      ""
    sal_uInt16 &rIdx3,              //      ""
    sal_uInt16 &rRemovedIdx )       // if doubles are being deleted, Pos back
{
    // #i48949# - actions aren't undoable. Thus, allow no undo
    // actions
    // Note: The undo action stack is cleared at the end of this method.
    ::sw::UndoGuard const undoGuard(GetDoc()->GetIDocumentUndoRedo());

    sal_Bool bRet = sal_False;

    if (INDEX_IGNORE == rIdx1 && CONTENT_STYLE == nSourceIdx1)
        rIdx1 = CONTENT_STYLE;

    if (CONTENT_STYLE == nSourceIdx1 && CONTENT_STYLE == rIdx1)
    {
        SfxStyleSheetBasePool* pHisPool  = rSource.GetStyleSheetPool();
        SwDocStyleSheetPool* pMyPool =
            (SwDocStyleSheetPool*)GetStyleSheetPool();

        // we can't move back and forth within ourselves
        if( pHisPool == pMyPool )
            return sal_False;

        if( INDEX_IGNORE == rIdx2 )
            rIdx2 = pMyPool->Count();

        // firstly "position" to the search-mask
        pHisPool->First();
        SfxStyleSheetBase* pHisSheet = (*pHisPool)[nSourceIdx2];

        // when such a template already exists: delete!
        const String& rOldName = pHisSheet->GetName();
        SfxStyleFamily eOldFamily( pHisSheet->GetFamily() );

        // never delete default PageDesc and Standard Charactertemplate !!!
        if( ( SFX_STYLE_FAMILY_PAGE == eOldFamily &&
              pDoc->GetPageDesc(0).GetName() ==
              rOldName ) ||
              ( SFX_STYLE_FAMILY_CHAR == eOldFamily &&
                rOldName == SwStyleNameMapper::GetTextUINameArray()[ RES_POOLCOLL_STANDARD -
                                                RES_POOLCOLL_TEXT_BEGIN ] ))
            return sal_False;

        SfxStyleFamily eMyOldFamily( pMyPool->GetSearchFamily() );
        sal_uInt16 nMySrchMask = pMyPool->GetSearchMask();

        SfxStyleSheetBase* pExist;
        if( ::FindPhyStyle( *pDoc, rOldName, eOldFamily ) )
        {
            // only take over, if desired!
            if( ERRCODE_BUTTON_OK != ErrorHandler::HandleError(
                *new MessageInfo( ERRCODE_SFXMSG_STYLEREPLACE, rOldName )) )
            {
                return sal_False;
            }

            // Because Replace needs the aStyleSheet-Member itself, the result
            // has to be copied from Find (see below))
            rtl::Reference< SwDocStyleSheet > xExist( new SwDocStyleSheet(
                    *(SwDocStyleSheet*)pMyPool->Find( rOldName, eOldFamily ) ) );
            pMyPool->Replace( *pHisSheet, *xExist.get() );

            // The ordering of the templates in the Pool is not changed.
            rIdx2 = rIdx1 = INDEX_IGNORE;

            GetDoc()->SetModified();

            return sal_True;
        }

        pMyPool->SetSearchMask( eOldFamily, nMySrchMask );

        // SwDocStyleSheetPool::Make returns a pointer to
        // SwDocStyleSheetPool::aStyleSheet that uses same members of
        // SwDocStyleSheetPool::Find. Therefore a copy has to be used
        // here. Attention: SfxStylessheetBase::SetFollow calls itself
        // a SwDocStyleSheetPool::Find, so that it's not sufficiant to
        // to eliminate the Find-calls in this method.

        rtl::Reference< SwDocStyleSheet > xNewSheet( new SwDocStyleSheet( (SwDocStyleSheet&)pMyPool
                ->Make(rOldName, eOldFamily, pHisSheet->GetMask() ) ) );
        if( SFX_STYLE_FAMILY_PAGE == eOldFamily && rSource.ISA(SwDocShell) )
        {
            // to deal with separately!!
            SwPageDesc* pDestDsc = (SwPageDesc*)xNewSheet->GetPageDesc();
            SwPageDesc* pCpyDsc = (SwPageDesc*)((SwDocStyleSheet*)pHisSheet)->GetPageDesc();
            pDoc->CopyPageDesc( *pCpyDsc, *pDestDsc );
        }
        else
            // populate the new templates with the attributes
            xNewSheet->SetItemSet( pHisSheet->GetItemSet() );

        pMyPool->SetSearchMask( SFX_STYLE_FAMILY_ALL, nMySrchMask );

        if( xNewSheet->IsUserDefined() || xNewSheet->IsUsed() )
        {
            // Used and User-defined templates are being showed. That's why
            // the Index of the template in the pool has to be found out.
            pExist = pMyPool->First();
            sal_uInt16 nIdx = 0;
            while( pExist )
            {
                if( pExist->GetName() == rOldName &&
                   eOldFamily == pExist->GetFamily() )
                {
                    rIdx2 = nIdx;
                    break;
                }
                pExist = pMyPool->Next();
                nIdx++;
            }
        }
        else
        {
            // Other templates are not being showed.
            rIdx1 = rIdx2 = INDEX_IGNORE;
        }

        // who gets the new one as parent? who uses the new one as Follow?
        // (always using the instanciated!!!)
        pMyPool->SetSearchMask( eOldFamily, nMySrchMask );
        pMyPool->SetOrganizerMode( sal_True );
        SfxStyleSheetBase* pTestSheet = pMyPool->First();
        while (pTestSheet)
        {
            if (pTestSheet->GetFamily() == eOldFamily &&
                pTestSheet->HasParentSupport()        &&
                pTestSheet->GetParent() == rOldName)
            {
                pTestSheet->SetParent(rOldName); // establish the link newly
            }

            if (pTestSheet->GetFamily() == eOldFamily &&
                pTestSheet->HasFollowSupport()        &&
                pTestSheet->GetFollow() == rOldName)
            {
                pTestSheet->SetFollow(rOldName); // establish the link newly
            }

            pTestSheet = pMyPool->Next();
        }
        pMyPool->SetOrganizerMode( SFX_CREATE_MODE_ORGANIZER == GetCreateMode() );

        // does the new one have a parent? if yes, search with the same name at our place.
        if (pHisSheet->HasParentSupport())
        {
            const String& rParentName = pHisSheet->GetParent();
            if (0 != rParentName.Len())
            {
                SfxStyleSheetBase* pParentOfNew = pMyPool->Find(rParentName,
                                                                eOldFamily);
                if (pParentOfNew)
                {
                    xNewSheet->SetParent(rParentName);
                }
                pMyPool->SetSearchMask( eOldFamily, nMySrchMask );
            }
        }

        // does the new one have a Follow? if yes, search with the same name at our place.
        if (pHisSheet->HasFollowSupport())
        {
            const String& rFollowName = pHisSheet->GetFollow();
            if (0 != rFollowName.Len())
            {
                SfxStyleSheetBase* pFollowOfNew = pMyPool->Find(rFollowName,
                                                                eOldFamily);
                if (pFollowOfNew)
                {
                    xNewSheet->SetFollow(rFollowName);
                }
                pMyPool->SetSearchMask( eOldFamily, nMySrchMask );
            }
        }

        // set old settings again
        pMyPool->SetSearchMask( eMyOldFamily, nMySrchMask );

        // Model changed
        OSL_ENSURE(pDoc, "Doc missing");
        GetDoc()->SetModified();

        bRet = sal_True;
    }
    else
        bRet = SfxObjectShell::Insert( rSource,
                    nSourceIdx1,
                    nSourceIdx2,
                    nSourceIdx3,
                    rIdx1,
                    rIdx2,
                    rIdx3,
                    rRemovedIdx);

    // #i48949# - actions aren't undoable and could have change
    // the document node array. Thus, clear the undo action stack.
    if (undoGuard.UndoWasEnabled())
    {
        GetDoc()->GetIDocumentUndoRedo().DelAllUndoObj();
    }

    return bRet;
}

/*--------------------------------------------------------------------
    Description:    template Remove
 --------------------------------------------------------------------*/

sal_Bool SwDocShell::Remove(sal_uInt16 nIdx1,       // see Insert
                        sal_uInt16 nIdx2,
                        sal_uInt16 nIdx3)
{
    sal_Bool bRet = sal_False;

    if (CONTENT_STYLE == nIdx1)
    {
        SwDocStyleSheetPool* pMyPool = (SwDocStyleSheetPool*)GetStyleSheetPool();

        pMyPool->First();       // update Pool before access!!
        SfxStyleSheetBase* pMySheet = (*pMyPool)[nIdx2];

        String aName( pMySheet->GetName() );
        SfxStyleFamily eFamily( pMySheet->GetFamily() );

        // never delete default PageDesc and Standard Character template!!!
        if( ( SFX_STYLE_FAMILY_PAGE == eFamily &&
              pDoc->GetPageDesc(0).GetName()
              == aName ) ||
              ( SFX_STYLE_FAMILY_CHAR == eFamily &&
                aName == SwStyleNameMapper::GetTextUINameArray()[ RES_POOLCOLL_STANDARD -
                                                RES_POOLCOLL_TEXT_BEGIN ] ))
            return sal_False;

        // so delete
        pMyPool->Remove( pMySheet );

        // now correct the Parents/Follows of all instanciated
        pMyPool->SetOrganizerMode( sal_True );
        SfxStyleSheetBase* pTestSheet = pMyPool->First();
        while (pTestSheet)
        {
            if (pTestSheet->GetFamily() == eFamily &&
                pTestSheet->HasParentSupport()     &&
                pTestSheet->GetParent() == aName)
            {
                pTestSheet->SetParent( aEmptyStr ); // resolve link
            }

            if (pTestSheet->GetFamily() == eFamily &&
                pTestSheet->HasFollowSupport()        &&
                pTestSheet->GetFollow() == aName)
            {
                pTestSheet->SetFollow( aEmptyStr ); // resolve link
            }

            pTestSheet = pMyPool->Next();
        }
        pMyPool->SetOrganizerMode( SFX_CREATE_MODE_ORGANIZER == GetCreateMode() );

        bRet = sal_True;
    }
    else
        bRet = SfxObjectShell::Remove(  nIdx1,
                                        nIdx2,
                                        nIdx3 );


    // Model changed
    OSL_ENSURE(pDoc, "Doc missing");
    GetDoc()->SetModified();

    return bRet;
}

void SwDocShell::Execute(SfxRequest& rReq)
{
    const SfxItemSet* pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem;
    sal_uInt16 nWhich = rReq.GetSlot();
    sal_Bool bDone = sal_False;
    switch ( nWhich )
    {
        case SID_AUTO_CORRECT_DLG:
        {
            SvxSwAutoFmtFlags* pAFlags = &SvxAutoCorrCfg::Get().GetAutoCorrect()->GetSwFlags();
            SwAutoCompleteWord& rACW = SwDoc::GetAutoCompleteWords();

            bool bOldLocked = rACW.IsLockWordLstLocked(),
                 bOldAutoCmpltCollectWords = pAFlags->bAutoCmpltCollectWords;

            rACW.SetLockWordLstLocked( true );

            editeng::SortedAutoCompleteStrings aTmpLst( rACW.GetWordList() );
            pAFlags->m_pAutoCompleteList = &aTmpLst;

            SfxApplication* pApp = SFX_APP();
            SfxRequest aAppReq(SID_AUTO_CORRECT_DLG, SFX_CALLMODE_SYNCHRON, pApp->GetPool());
            SfxBoolItem aSwOptions( SID_AUTO_CORRECT_DLG, sal_True );
            aAppReq.AppendItem(aSwOptions);

            // SMARTTAGS
            pAFlags->pSmartTagMgr = &SwSmartTagMgr::Get();

            SfxItemSet aSet( pApp->GetPool(), SID_AUTO_CORRECT_DLG, SID_AUTO_CORRECT_DLG, SID_OPEN_SMARTTAGOPTIONS, SID_OPEN_SMARTTAGOPTIONS, 0 );
            aSet.Put( aSwOptions );

            const SfxPoolItem* pOpenSmartTagOptionsItem = 0;
            if( pArgs && SFX_ITEM_SET == pArgs->GetItemState( SID_OPEN_SMARTTAGOPTIONS, sal_False, &pOpenSmartTagOptionsItem ) )
                aSet.Put( *static_cast<const SfxBoolItem*>(pOpenSmartTagOptionsItem) );

            SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
              SfxAbstractTabDialog* pDlg = pFact->CreateTabDialog( RID_OFA_AUTOCORR_DLG, NULL, &aSet, NULL );
              pDlg->Execute();
              delete pDlg;

            rACW.SetLockWordLstLocked( bOldLocked );

            SwEditShell::SetAutoFmtFlags( pAFlags );
            rACW.SetMinWordLen( pAFlags->nAutoCmpltWordLen );
            rACW.SetMaxCount( pAFlags->nAutoCmpltListLen );
            if (pAFlags->m_pAutoCompleteList)  // any changes?
            {
                rACW.CheckChangedList( aTmpLst );
                // clear the temp WordList pointer
                pAFlags->m_pAutoCompleteList = 0;
            }
            // remove all pointer we never delete the strings
            aTmpLst.clear();

            if( !bOldAutoCmpltCollectWords && bOldAutoCmpltCollectWords !=
                pAFlags->bAutoCmpltCollectWords )
            {
                // call on all Docs the idle formatter to start
                // the collection of Words
                TypeId aType = TYPE(SwDocShell);
                for( SwDocShell *pDocSh = (SwDocShell*)SfxObjectShell::GetFirst(&aType);
                     pDocSh;
                     pDocSh = (SwDocShell*)SfxObjectShell::GetNext( *pDocSh, &aType ) )
                {
                    SwDoc* pTmp = pDocSh->GetDoc();
                    if ( pTmp->GetCurrentViewShell() )  //swmod 071108//swmod 071225
                        pTmp->InvalidateAutoCompleteFlag();
                }
            }
        }
        break;

        case SID_PRINTPREVIEW:
            {
                sal_Bool bSet = sal_False, bFound = sal_False, bOnly = sal_True;
                SfxViewFrame *pTmpFrm = SfxViewFrame::GetFirst(this);
                SfxViewShell* pViewShell = SfxViewShell::Current();
                SwView* pCurrView = dynamic_cast< SwView *> ( pViewShell );
                sal_Bool bCurrent = IS_TYPE( SwPagePreView, pViewShell );

                while( pTmpFrm )    // search PreView
                {
                    if( IS_TYPE( SwView, pTmpFrm->GetViewShell()) )
                        bOnly = sal_False;
                    else if( IS_TYPE( SwPagePreView, pTmpFrm->GetViewShell()))
                    {
                        pTmpFrm->GetFrame().Appear();
                        bFound = sal_True;
                    }
                    if( bFound && !bOnly )
                        break;
                    pTmpFrm = pTmpFrm->GetNext(*pTmpFrm, this);
                }

                if( pArgs && SFX_ITEM_SET ==
                    pArgs->GetItemState( SID_PRINTPREVIEW, sal_False, &pItem ))
                    bSet = ((SfxBoolItem*)pItem)->GetValue();
                else
                    bSet = !bCurrent;

                sal_uInt16 nSlotId = 0;
                if( bSet && !bFound )   // Nothing found, so create new Preview
                        nSlotId = SID_VIEWSHELL1;
                else if( bFound && !bSet )
                    nSlotId = bOnly ? SID_VIEWSHELL0 : SID_VIEWSHELL1;

                if( nSlotId )
                {
                    // PagePreView in the WebDocShell
                    // is found under Id ViewShell2.
                    if( ISA(SwWebDocShell) && SID_VIEWSHELL1 == nSlotId )
                        nSlotId = SID_VIEWSHELL2;

                    if( pCurrView && pCurrView->GetDocShell() == this )
                        pTmpFrm = pCurrView->GetViewFrame();
                    else
                        pTmpFrm = SfxViewFrame::GetFirst( this );

                    pTmpFrm->GetDispatcher()->Execute( nSlotId, 0, 0, SFX_CALLMODE_ASYNCHRON );
                }

                rReq.SetReturnValue(SfxBoolItem(SID_PRINTPREVIEW, bSet ));
            }
            break;
        case SID_TEMPLATE_LOAD:
            {
                String aFileName;
                static sal_Bool bText = sal_True;
                static sal_Bool bFrame = sal_False;
                static sal_Bool bPage =  sal_False;
                static sal_Bool bNum =   sal_False;
                static sal_Bool bMerge = sal_False;
                sal_uInt16 nRet = USHRT_MAX;

                sal_uInt16 nFlags = bFrame ? SFX_LOAD_FRAME_STYLES : 0;
                if(bPage)
                    nFlags|= SFX_LOAD_PAGE_STYLES;
                if(bNum)
                    nFlags|= SFX_LOAD_NUM_STYLES;
                if(!nFlags || bText)
                    nFlags|= SFX_LOAD_TEXT_STYLES;
                if(bMerge)
                    nFlags|= SFX_MERGE_STYLES;

                if ( pArgs )
                {
                    SFX_REQUEST_ARG( rReq, pTemplateItem, SfxStringItem, SID_TEMPLATE_NAME, sal_False );
                    if ( pTemplateItem )
                    {
                        aFileName = pTemplateItem->GetValue();
                        SFX_REQUEST_ARG( rReq, pFlagsItem, SfxInt32Item, SID_TEMPLATE_LOAD, sal_False );
                        if ( pFlagsItem )
                            nFlags = (sal_uInt16) pFlagsItem->GetValue();
                    }
                }

                if ( !aFileName.Len() )
                {
                    SvtPathOptions aPathOpt;
                    SfxNewFileDialog* pNewFileDlg =
                        new SfxNewFileDialog(&GetView()->GetViewFrame()->GetWindow(), SFXWB_LOAD_TEMPLATE);
                    pNewFileDlg->SetTemplateFlags(nFlags);

                    nRet = pNewFileDlg->Execute();
                    if(RET_TEMPLATE_LOAD == nRet)
                    {
                        FileDialogHelper aDlgHelper( TemplateDescription::FILEOPEN_SIMPLE, 0 );
                        uno::Reference < XFilePicker > xFP = aDlgHelper.GetFilePicker();

                        xFP->setDisplayDirectory( aPathOpt.GetWorkPath() );

                        SfxObjectFactory &rFact = GetFactory();
                        SfxFilterMatcher aMatcher( rtl::OUString::createFromAscii(rFact.GetShortName()) );
                        SfxFilterMatcherIter aIter( aMatcher );
                        uno::Reference<XFilterManager> xFltMgr(xFP, UNO_QUERY);
                        const SfxFilter* pFlt = aIter.First();
                        while( pFlt )
                        {
                            // --> OD #i117339#
//                            if( pFlt && pFlt->IsAllowedAsTemplate() )
                            if( pFlt && pFlt->IsAllowedAsTemplate() &&
                                ( pFlt->GetUserData() == "CXML" ||
                                  pFlt->GetUserData() == "CXMLV" ) )
                            {
                                const String sWild = pFlt->GetWildcard().getGlob();
                                xFltMgr->appendFilter( pFlt->GetUIName(), sWild );
                            }
                            pFlt = aIter.Next();
                        }
                        sal_Bool bWeb = 0 != dynamic_cast< SwWebDocShell *>( this );
                        const SfxFilter *pOwnFlt =
                                SwDocShell::Factory().GetFilterContainer()->
                                GetFilter4FilterName(rtl::OUString("writer8"));

                        // make sure the default file format is also available
                        if(bWeb)
                        {
                            const String sWild = pOwnFlt->GetWildcard().getGlob();
                            xFltMgr->appendFilter( pOwnFlt->GetUIName(), sWild );
                        }

                        bool bError = false;
                        // catch exception if wrong filter is selected - should not happen anymore
                        try
                        {
                            xFltMgr->setCurrentFilter( pOwnFlt->GetUIName() );
                        }
                        catch (const uno::Exception&)
                        {
                            bError = true;
                        }

                        if( !bError && ERRCODE_NONE == aDlgHelper.Execute() )
                        {
                            aFileName = xFP->getFiles().getConstArray()[0];
                        }
                    }
                    else if( RET_OK == nRet)
                    {
                        aFileName = pNewFileDlg->GetTemplateFileName();
                    }

                    nFlags = pNewFileDlg->GetTemplateFlags();
                    rReq.AppendItem( SfxStringItem( SID_TEMPLATE_NAME, aFileName ) );
                    rReq.AppendItem( SfxInt32Item( SID_TEMPLATE_LOAD, (long) nFlags ) );
                    delete pNewFileDlg;
                }

                if( aFileName.Len() )
                {
                    SwgReaderOption aOpt;
                    aOpt.SetTxtFmts(    bText = (0 != (nFlags&SFX_LOAD_TEXT_STYLES) ));
                    aOpt.SetFrmFmts(    bFrame = (0 != (nFlags&SFX_LOAD_FRAME_STYLES)));
                    aOpt.SetPageDescs(  bPage = (0 != (nFlags&SFX_LOAD_PAGE_STYLES )));
                    aOpt.SetNumRules(   bNum = (0 != (nFlags&SFX_LOAD_NUM_STYLES  )));
                    //different meaning between SFX_MERGE_STYLES and aOpt.SetMerge!
                    bMerge = 0 != (nFlags&SFX_MERGE_STYLES);
                    aOpt.SetMerge( !bMerge );

                    SetError( LoadStylesFromFile( aFileName, aOpt, sal_False ), ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ));
                    if ( !GetError() )
                        rReq.Done();
                }
            }
            break;
            case SID_SOURCEVIEW:
            {
                SfxViewShell* pViewShell = GetView()
                                            ? (SfxViewShell*)GetView()
                                            : SfxViewShell::Current();
                SfxViewFrame*  pViewFrm = pViewShell->GetViewFrame();
                SwSrcView* pSrcView = dynamic_cast< SwSrcView *>( pViewShell );
                if(!pSrcView)
                {
                    // 3 possible state:
                    // 1 - file unsaved -> save as HTML
                    // 2 - file modified and HTML filter active -> save
                    // 3 - file saved in non-HTML -> QueryBox to save as HTML
                    const SfxFilter* pHtmlFlt =
                                    SwIoSystem::GetFilterOfFormat(
                                        rtl::OUString("HTML"),
                                        SwWebDocShell::Factory().GetFilterContainer() );
                    sal_Bool bLocalHasName = HasName();
                    if(bLocalHasName)
                    {
                        //check for filter type
                        const SfxFilter* pFlt = GetMedium()->GetFilter();
                        if(!pFlt || pFlt->GetUserData() != pHtmlFlt->GetUserData())
                        {
                            QueryBox aQuery(&pViewFrm->GetWindow(), SW_RES(MSG_SAVEAS_HTML_QUERY));
                            if(RET_YES == aQuery.Execute())
                                bLocalHasName = sal_False;
                            else
                                break;
                        }
                    }
                    if(!bLocalHasName)
                    {
                        FileDialogHelper aDlgHelper( TemplateDescription::FILESAVE_AUTOEXTENSION, 0 );
                        aDlgHelper.AddFilter( pHtmlFlt->GetFilterName(), pHtmlFlt->GetDefaultExtension() );
                        aDlgHelper.SetCurrentFilter( pHtmlFlt->GetFilterName() );
                        if( ERRCODE_NONE != aDlgHelper.Execute())
                        {
                            break;
                        }
                        String sPath = aDlgHelper.GetPath();
                        SfxStringItem aName(SID_FILE_NAME, sPath);
                        SfxStringItem aFilter(SID_FILTER_NAME, pHtmlFlt->GetName());
                        const SfxBoolItem* pBool = (const SfxBoolItem*)
                                pViewFrm->GetDispatcher()->Execute(
                                        SID_SAVEASDOC, SFX_CALLMODE_SYNCHRON, &aName, &aFilter, 0L );
                        if(!pBool || !pBool->GetValue())
                            break;
                    }
                }

                OSL_ENSURE(dynamic_cast<SwWebDocShell*>(this),
                            "SourceView only in WebDocShell");

                // the SourceView is not the 1 for SwWebDocShell
                sal_uInt16 nSlot = SID_VIEWSHELL1;
                sal_Bool bSetModified = sal_False;
                SfxPrinter* pSavePrinter = 0;
                if( 0 != pSrcView)
                {
                    SfxPrinter* pTemp = GetDoc()->getPrinter( false );
                    if(pTemp)
                        pSavePrinter = new SfxPrinter(*pTemp);
                    bSetModified = IsModified() || pSrcView->IsModified();
                    if(pSrcView->IsModified()||pSrcView->HasSourceSaved())
                    {
                        utl::TempFile aTempFile;
                        aTempFile.EnableKillingFile();
                        pSrcView->SaveContent(aTempFile.GetURL());
                        bDone = sal_True;
                        SvxMacro aMac(aEmptyStr, aEmptyStr, STARBASIC);
                        SfxEventConfiguration::ConfigureEvent(GlobalEventConfig::GetEventName( STR_EVENT_OPENDOC ), aMac, this);
                        SfxEventConfiguration::ConfigureEvent(GlobalEventConfig::GetEventName( STR_EVENT_PREPARECLOSEDOC ), aMac, this);
                        SfxEventConfiguration::ConfigureEvent(GlobalEventConfig::GetEventName( STR_EVENT_ACTIVATEDOC ),     aMac, this);
                        SfxEventConfiguration::ConfigureEvent(GlobalEventConfig::GetEventName( STR_EVENT_DEACTIVATEDOC ), aMac, this);
                        ReloadFromHtml(aTempFile.GetURL(), pSrcView);
                        nSlot = 0;
                    }
                    else
                    {
                        nSlot = SID_VIEWSHELL0;
                    }
                }
                if(nSlot)
                    pViewFrm->GetDispatcher()->Execute(nSlot, SFX_CALLMODE_SYNCHRON);
                if(bSetModified)
                    GetDoc()->SetModified();
                if(pSavePrinter)
                {
                    GetDoc()->setPrinter( pSavePrinter, true, true);
                    //pSavePrinter must not be deleted again
                }
                pViewFrm->GetBindings().SetState(SfxBoolItem(SID_SOURCEVIEW, nSlot == SID_VIEWSHELL2));
                pViewFrm->GetBindings().Invalidate( SID_NEWWINDOW );
                pViewFrm->GetBindings().Invalidate( SID_BROWSER_MODE );
                pViewFrm->GetBindings().Invalidate( FN_PRINT_LAYOUT );
            }
            break;
            case SID_GET_COLORLIST:
            {
                SvxColorListItem* pColItem = (SvxColorListItem*)GetItem(SID_COLOR_TABLE);
                XColorListRef pList = pColItem->GetColorList();
                rReq.SetReturnValue(OfaRefItem<XColorList>(SID_GET_COLORLIST, pList));
            }
            break;
        case FN_ABSTRACT_STARIMPRESS:
        case FN_ABSTRACT_NEWDOC:
        {
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

            AbstractSwInsertAbstractDlg* pDlg = pFact->CreateSwInsertAbstractDlg(0, DLG_INSERT_ABSTRACT );
            OSL_ENSURE(pDlg, "Dialogdiet fail!");
            if(RET_OK == pDlg->Execute())
            {
                sal_uInt8 nLevel = pDlg->GetLevel();
                sal_uInt8 nPara = pDlg->GetPara();
                SwDoc* pSmryDoc = new SwDoc();
                SfxObjectShellLock xDocSh( new SwDocShell( pSmryDoc, SFX_CREATE_MODE_STANDARD));
                xDocSh->DoInitNew( 0 );

                sal_Bool bImpress = FN_ABSTRACT_STARIMPRESS == nWhich;
                pDoc->Summary( pSmryDoc, nLevel, nPara, bImpress );
                if( bImpress )
                {
                    WriterRef xWrt;
                    // mba: looks as if relative URLs don't make sense here
                    ::GetRTFWriter( aEmptyStr, String(), xWrt );
                    SvMemoryStream *pStrm = new SvMemoryStream();
                    pStrm->SetBufferSize( 16348 );
                    SwWriter aWrt( *pStrm, *pSmryDoc );
                    ErrCode eErr = aWrt.Write( xWrt );
                    if( !ERRCODE_TOERROR( eErr ) )
                    {
                        uno::Reference< lang::XMultiServiceFactory > xORB = ::comphelper::getProcessServiceFactory();
                        uno::Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
                        uno::Reference< frame::XDispatchProvider > xProv(
                            xORB->createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.ModuleDispatcher"))), UNO_QUERY );
                        if ( xProv.is() )
                        {
                            ::rtl::OUString aCmd(RTL_CONSTASCII_USTRINGPARAM("SendOutlineToImpress"));
                            uno::Reference< frame::XDispatchHelper > xHelper( frame::DispatchHelper::create(xContext) );
                            pStrm->Seek( STREAM_SEEK_TO_END );
                            *pStrm << '\0';
                            pStrm->Seek( STREAM_SEEK_TO_BEGIN );

                            // Transfer ownership of stream to a lockbytes object
                            SvLockBytes aLockBytes( pStrm, sal_True );
                            SvLockBytesStat aStat;
                            if ( aLockBytes.Stat( &aStat, SVSTATFLAG_DEFAULT ) == ERRCODE_NONE )
                            {
                                sal_uInt32 nLen = aStat.nSize;
                                sal_uLong nRead = 0;
                                uno::Sequence< sal_Int8 > aSeq( nLen );
                                aLockBytes.ReadAt( 0, aSeq.getArray(), nLen, &nRead );

                                uno::Sequence< beans::PropertyValue > aArgs(1);
                                aArgs[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("RtfOutline"));
                                aArgs[0].Value <<= aSeq;
                                xHelper->executeDispatch( xProv, aCmd, ::rtl::OUString(), 0, aArgs );
                            }
                        }
                    }
                    else
                        ErrorHandler::HandleError(ErrCode( eErr ));
                }
                else
                {
                    // Create new document
                    SfxViewFrame *pFrame = SfxViewFrame::LoadDocument( *xDocSh, 0 );
                    SwView      *pCurrView = (SwView*) pFrame->GetViewShell();

                    // Set document's title
                    String aTmp( SW_RES(STR_ABSTRACT_TITLE) );
                    aTmp += GetTitle();
                    xDocSh->SetTitle( aTmp );
                    pCurrView->GetWrtShell().SetNewDoc();
                    pFrame->Show();
                    pSmryDoc->SetModified();
                }

            }
            delete pDlg;
        }
        break;
        case FN_OUTLINE_TO_CLIPBOARD:
        case FN_OUTLINE_TO_IMPRESS:
            {
                sal_Bool bEnable = IsEnableSetModified();
                EnableSetModified( sal_False );
                WriterRef xWrt;
                // mba: looks as if relative URLs don't make sense here
                ::GetRTFWriter( rtl::OUString('O'), rtl::OUString(), xWrt );
                SvMemoryStream *pStrm = new SvMemoryStream();
                pStrm->SetBufferSize( 16348 );
                SwWriter aWrt( *pStrm, *GetDoc() );
                ErrCode eErr = aWrt.Write( xWrt );
                EnableSetModified( bEnable );
                if( !ERRCODE_TOERROR( eErr ) )
                {
                    pStrm->Seek( STREAM_SEEK_TO_END );
                    *pStrm << '\0';
                    pStrm->Seek( STREAM_SEEK_TO_BEGIN );
                    if ( nWhich == FN_OUTLINE_TO_IMPRESS )
                    {
                        uno::Reference< lang::XMultiServiceFactory > xORB = ::comphelper::getProcessServiceFactory();
                        uno::Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
                        uno::Reference< frame::XDispatchProvider > xProv(
                            xORB->createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.ModuleDispatcher"))), UNO_QUERY );
                        if ( xProv.is() )
                        {
                            ::rtl::OUString aCmd(RTL_CONSTASCII_USTRINGPARAM("SendOutlineToImpress"));
                            uno::Reference< frame::XDispatchHelper > xHelper( frame::DispatchHelper::create(xContext) );
                            pStrm->Seek( STREAM_SEEK_TO_END );
                            *pStrm << '\0';
                            pStrm->Seek( STREAM_SEEK_TO_BEGIN );

                            // Transfer ownership of stream to a lockbytes object
                            SvLockBytes aLockBytes( pStrm, sal_True );
                            SvLockBytesStat aStat;
                            if ( aLockBytes.Stat( &aStat, SVSTATFLAG_DEFAULT ) == ERRCODE_NONE )
                            {
                                sal_uInt32 nLen = aStat.nSize;
                                sal_uLong nRead = 0;
                                uno::Sequence< sal_Int8 > aSeq( nLen );
                                aLockBytes.ReadAt( 0, aSeq.getArray(), nLen, &nRead );

                                uno::Sequence< beans::PropertyValue > aArgs(1);
                                aArgs[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("RtfOutline"));
                                aArgs[0].Value <<= aSeq;
                                xHelper->executeDispatch( xProv, aCmd, ::rtl::OUString(), 0, aArgs );
                            }
                        }
                    }
                    else
                    {
                        TransferDataContainer* pClipCntnr =
                                                    new TransferDataContainer;
                        ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >
                                                        xRef( pClipCntnr );

                        pClipCntnr->CopyAnyData( FORMAT_RTF, (sal_Char*)
                                    pStrm->GetData(), pStrm->GetEndOfData() );
                        pClipCntnr->CopyToClipboard(
                            GetView()? (Window*)&GetView()->GetEditWin() : 0 );
                        delete pStrm;
                    }
                }
                else
                    ErrorHandler::HandleError(ErrCode( eErr ));
            }
            break;
            case SID_SPELLCHECKER_CHANGED:
                //! false, true, true is on the save side but a probably overdone
                SW_MOD()->CheckSpellChanges(false, true, true, false );
            break;

        case SID_MAIL_PREPAREEXPORT:
        {
            //pWrtShell is not set in page preview
            if(pWrtShell)
                pWrtShell->StartAllAction();
            pDoc->UpdateFlds( NULL, false );
            pDoc->EmbedAllLinks();
            pDoc->RemoveInvisibleContent();
            if(pWrtShell)
                pWrtShell->EndAllAction();
        }
        break;

        case SID_MAIL_EXPORT_FINISHED:
        {
                if(pWrtShell)
                    pWrtShell->StartAllAction();
                //try to undo the removal of invisible content
                pDoc->RestoreInvisibleContent();
                if(pWrtShell)
                    pWrtShell->EndAllAction();
        }
        break;
        case FN_NEW_HTML_DOC:
        case FN_NEW_GLOBAL_DOC:
            {
                bDone = sal_False;
                sal_Bool bCreateHtml = FN_NEW_HTML_DOC == nWhich;

                sal_Bool bCreateByOutlineLevel = false;     //#outline level,add by zhaojianwei
                sal_Int32  nTemplateOutlineLevel = 0 ;      //#outline level,add by zhaojianwei

                String aFileName, aTemplateName;
                if( pArgs && SFX_ITEM_SET == pArgs->GetItemState( nWhich, sal_False, &pItem ) )
                {
                    aFileName = ((const SfxStringItem*)pItem)->GetValue();
                    SFX_ITEMSET_ARG( pArgs, pTemplItem, SfxStringItem, SID_TEMPLATE_NAME, sal_False );
                    if ( pTemplItem )
                        aTemplateName = pTemplItem->GetValue();
                }
                bool bError = false;
                if ( !aFileName.Len() )
                {
                    FileDialogHelper aDlgHelper( TemplateDescription::FILESAVE_AUTOEXTENSION_TEMPLATE, 0 );

                    const sal_Int16 nControlIds[] = {
                        CommonFilePickerElementIds::PUSHBUTTON_OK,
                        CommonFilePickerElementIds::PUSHBUTTON_CANCEL,
                        CommonFilePickerElementIds::LISTBOX_FILTER,
                        CommonFilePickerElementIds::CONTROL_FILEVIEW,
                        CommonFilePickerElementIds::EDIT_FILEURL,
                        ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION,
                        ExtendedFilePickerElementIds::LISTBOX_TEMPLATE,
                        0
                    };

                    const char* aHTMLHelpIds[] =
                    {
                         HID_SEND_HTML_CTRL_PUSHBUTTON_OK,
                         HID_SEND_HTML_CTRL_PUSHBUTTON_CANCEL,
                         HID_SEND_HTML_CTRL_LISTBOX_FILTER,
                         HID_SEND_HTML_CTRL_CONTROL_FILEVIEW,
                         HID_SEND_HTML_CTRL_EDIT_FILEURL,
                         HID_SEND_HTML_CTRL_CHECKBOX_AUTOEXTENSION,
                         HID_SEND_HTML_CTRL_LISTBOX_TEMPLATE,
                         ""
                    };

                    const char* aMasterHelpIds[] =
                    {
                         HID_SEND_MASTER_CTRL_PUSHBUTTON_OK,
                         HID_SEND_MASTER_CTRL_PUSHBUTTON_CANCEL,
                         HID_SEND_MASTER_CTRL_LISTBOX_FILTER,
                         HID_SEND_MASTER_CTRL_CONTROL_FILEVIEW,
                         HID_SEND_MASTER_CTRL_EDIT_FILEURL,
                         HID_SEND_MASTER_CTRL_CHECKBOX_AUTOEXTENSION,
                         HID_SEND_MASTER_CTRL_LISTBOX_TEMPLATE,
                         ""
                    };

                    const char** pHelpIds = bCreateHtml ? aHTMLHelpIds : aMasterHelpIds;
                    aDlgHelper.SetControlHelpIds( nControlIds, pHelpIds );
                    uno::Reference < XFilePicker > xFP = aDlgHelper.GetFilePicker();

                    const SfxFilter* pFlt;
                    sal_uInt16 nStrId;

                    if( bCreateHtml )
                    {
                        // for HTML there is only one filter!!
                        pFlt = SwIoSystem::GetFilterOfFormat(
                                rtl::OUString("HTML"),
                                SwWebDocShell::Factory().GetFilterContainer() );
                        nStrId = STR_LOAD_HTML_DOC;
                    }
                    else
                    {
                        // for Global-documents we now only offer the current one.
                        pFlt = SwGlobalDocShell::Factory().GetFilterContainer()->
                                    GetFilter4Extension( rtl::OUString("odm")  );
                        nStrId = STR_LOAD_GLOBAL_DOC;
                    }

                    if( pFlt )
                    {
                        uno::Reference<XFilterManager> xFltMgr(xFP, UNO_QUERY);
                        const String sWild = pFlt->GetWildcard().getGlob();
                        xFltMgr->appendFilter( pFlt->GetUIName(), sWild );
                        try
                        {
                            xFltMgr->setCurrentFilter( pFlt->GetUIName() ) ;
                        }
                        catch (const uno::Exception&)
                        {
                            bError = true;
                        }
                    }
                    if(!bError)
                    {
                        uno::Reference<XFilePickerControlAccess> xCtrlAcc(xFP, UNO_QUERY);

                        bool    bOutline[MAXLEVEL] = {false};
                        const SwOutlineNodes& rOutlNds = pDoc->GetNodes().GetOutLineNds();
                        if( !rOutlNds.empty() )
                        {
                            int nLevel;
                            for(sal_uInt16 n = 0; n < rOutlNds.size(); ++n )
                                if( ( nLevel = rOutlNds[n]->GetTxtNode()->GetAttrOutlineLevel()) > 0 &&
                                    ! bOutline[nLevel-1] )
                                {
                                    bOutline[nLevel-1] = true;
                                }
                        }

                        const sal_uInt16 nStyleCount = pDoc->GetTxtFmtColls()->size();
                        Sequence<OUString> aListBoxEntries( MAXLEVEL + nStyleCount);
                        OUString* pEntries = aListBoxEntries.getArray();
                        sal_Int32   nIdx = 0 ;

                        OUString    sOutline( SW_RESSTR(STR_FDLG_OUTLINE_LEVEL) );
                        for( sal_uInt16 i = 0; i < MAXLEVEL; ++i )
                        {
                            if( bOutline[i] )
                                pEntries[nIdx++] = sOutline + String::CreateFromInt32( i+1 );
                        }

                        OUString    sStyle( SW_RESSTR(STR_FDLG_STYLE) );
                        for(sal_uInt16 i = 0; i < nStyleCount; ++i)
                        {
                            SwTxtFmtColl &rTxtColl = *(*pDoc->GetTxtFmtColls())[ i ];
                            if( !rTxtColl.IsDefault() && rTxtColl.IsAtDocNodeSet() )
                            {
                                pEntries[nIdx++] = sStyle + rTxtColl.GetName();
                            }
                        }

                        aListBoxEntries.realloc(nIdx);
                        sal_Int16 nSelect = 0;

                        try
                        {
                            Any aTemplates(&aListBoxEntries, ::getCppuType(&aListBoxEntries));

                            xCtrlAcc->setValue( ExtendedFilePickerElementIds::LISTBOX_TEMPLATE,
                                ListboxControlActions::ADD_ITEMS , aTemplates );
                            Any aSelectPos(&nSelect, ::getCppuType(&nSelect));
                            xCtrlAcc->setValue( ExtendedFilePickerElementIds::LISTBOX_TEMPLATE,
                                ListboxControlActions::SET_SELECT_ITEM, aSelectPos );
                            xCtrlAcc->setLabel( ExtendedFilePickerElementIds::LISTBOX_TEMPLATE,
                                                    String(SW_RES( STR_FDLG_TEMPLATE_NAME )));
                        }
                        catch (const Exception&)
                        {
                            OSL_FAIL("control acces failed");
                        }

                        xFP->setTitle( SW_RESSTR( nStrId ));
                        SvtPathOptions aPathOpt;
                        xFP->setDisplayDirectory( aPathOpt.GetWorkPath() );
                        if( ERRCODE_NONE == aDlgHelper.Execute())
                        {
                            aFileName = xFP->getFiles().getConstArray()[0];
                            Any aTemplateValue = xCtrlAcc->getValue(
                                ExtendedFilePickerElementIds::LISTBOX_TEMPLATE,
                                ListboxControlActions::GET_SELECTED_ITEM );
                            OUString sTmpl;
                            aTemplateValue >>= sTmpl;

                            sal_Int32 nColonPos = sTmpl.indexOf( sal_Unicode(':') );
                            OUString sPrefix = sTmpl.copy( 0L, nColonPos );
                            if ( sPrefix == "Style" )
                            {
                                aTemplateName = sTmpl.copy( 7L );   //get string behind "Style: "
                            }
                            else if ( sPrefix == "Outline" )
                            {
                                nTemplateOutlineLevel = ( sTmpl.copy( 15L )).toInt32(); //get string behind "Outline: Leve  ";
                                bCreateByOutlineLevel = true;
                            }

                            if ( aFileName.Len() )
                            {
                                rReq.AppendItem( SfxStringItem( nWhich, aFileName ) );
                                if( aTemplateName.Len() )
                                    rReq.AppendItem( SfxStringItem( SID_TEMPLATE_NAME, aTemplateName ) );
                            }
                        }
                    }
                }

                if( aFileName.Len() )
                {
                    if( PrepareClose( sal_False ) )
                    {
                        SwWait aWait( *this, sal_True );

                        if ( bCreateByOutlineLevel )
                        {
                            bDone = bCreateHtml
                                ? pDoc->GenerateHTMLDoc( aFileName, nTemplateOutlineLevel )
                                : pDoc->GenerateGlobalDoc( aFileName, nTemplateOutlineLevel );
                        }
                        else
                        {
                            const SwTxtFmtColl* pSplitColl = 0;
                            if ( aTemplateName.Len() )
                                pSplitColl = pDoc->FindTxtFmtCollByName(aTemplateName);
                            bDone = bCreateHtml
                                ? pDoc->GenerateHTMLDoc( aFileName, pSplitColl )
                                : pDoc->GenerateGlobalDoc( aFileName, pSplitColl );
                        }
                        if( bDone )
                        {
                            SfxStringItem aName( SID_FILE_NAME, aFileName );
                            SfxStringItem aReferer( SID_REFERER, aEmptyStr );
                            SfxViewShell* pViewShell = SfxViewShell::GetFirst();
                            while(pViewShell)
                            {
                                //search for the view that created the call
                                if(pViewShell->GetObjectShell() == this && pViewShell->GetDispatcher())
                                {
                                    SfxFrameItem* pFrameItem = new SfxFrameItem( SID_DOCFRAME,
                                                        pViewShell->GetViewFrame() );
                                    SfxDispatcher* pDispatch = pViewShell->GetDispatcher();
                                    pDispatch->Execute(
                                            SID_OPENDOC,
                                            SFX_CALLMODE_ASYNCHRON,
                                            &aName,
                                            &aReferer,
                                            pFrameItem, 0L );

                                    delete pFrameItem;
                                    break;
                                }
                                pViewShell = SfxViewShell::GetNext(*pViewShell);
                            }
                        }
                    }
                    if( !bDone && !rReq.IsAPI() )
                    {
                        InfoBox( 0, SW_RESSTR( STR_CANTCREATE )).Execute();
                    }
                }
            }
            rReq.SetReturnValue(SfxBoolItem( nWhich, bDone ));
            if (bDone)
                rReq.Done();
            else
                rReq.Ignore();
            break;

        case SID_ATTR_YEAR2000:
            if ( pArgs && SFX_ITEM_SET == pArgs->GetItemState( nWhich , sal_False, &pItem ))
            {
                OSL_ENSURE(pItem->ISA(SfxUInt16Item), "wrong Item");
                sal_uInt16 nYear2K = ((SfxUInt16Item*)pItem)->GetValue();
                // iterate over Views and put the State to FormShells

                SfxViewFrame* pVFrame = SfxViewFrame::GetFirst( this );
                SfxViewShell* pViewShell = pVFrame ? pVFrame->GetViewShell() : 0;
                SwView* pCurrView = dynamic_cast< SwView* >( pViewShell );
                while(pCurrView)
                {
                    FmFormShell* pFormShell = pCurrView->GetFormShell();
                    if(pFormShell)
                        pFormShell->SetY2KState(nYear2K);
                    pVFrame = SfxViewFrame::GetNext( *pVFrame, this );
                    pViewShell = pVFrame ? pVFrame->GetViewShell() : 0;
                    pCurrView = dynamic_cast<SwView*>( pViewShell );
                }
                pDoc->GetNumberFormatter(sal_True)->SetYear2000(nYear2K);
            }
        break;
        case FN_OPEN_FILE:
        {
            SfxViewShell* pViewShell = GetView();
            if (!pViewShell)
                pViewShell = SfxViewShell::Current();

            if (!pViewShell)
                // Ok.  I did my best.
                break;

            SfxStringItem aApp(SID_DOC_SERVICE, rtl::OUString("com.sun.star.text.TextDocument"));
            SfxStringItem aTarget(SID_TARGETNAME, rtl::OUString("_blank"));
            pViewShell->GetDispatcher()->Execute(
                SID_OPENDOC, SFX_CALLMODE_API|SFX_CALLMODE_SYNCHRON, &aApp, &aTarget, 0L);
        }
        break;

        default: OSL_FAIL("wrong Dispatcher");
    }
}

 // #FIXME - align with NEW event stuff ( if possible )
#if 0
void lcl_processCompatibleSfxHint( const uno::Reference< document::XVbaEventsHelper >& xVbaEventsHelper, const SfxHint& rHint )
{
    if ( rHint.ISA( SfxEventHint ) )
    {
        uno::Sequence< uno::Any > aArgs;
        sal_uLong nEventId = ((SfxEventHint&)rHint).GetEventId();
        switch( nEventId )
        {
            case SFX_EVENT_CREATEDOC:
            {
                xVbaEventsHelper->ProcessCompatibleVbaEvent( VBAEVENT_DOCUMENT_NEW, aArgs );
                break;
            }
            case SFX_EVENT_OPENDOC:
            {
                xVbaEventsHelper->ProcessCompatibleVbaEvent( VBAEVENT_DOCUMENT_OPEN, aArgs );
                break;
            }
            default:
            {
                //do nothing
            }
        }
    }
}
#endif

long SwDocShell::DdeGetData( const String& rItem, const String& rMimeType,
                                uno::Any & rValue )
{
    return pDoc->GetData( rItem, rMimeType, rValue );
}

long SwDocShell::DdeSetData( const String& rItem, const String& rMimeType,
                            const uno::Any & rValue )
{
    return pDoc->SetData( rItem, rMimeType, rValue );
}

::sfx2::SvLinkSource* SwDocShell::DdeCreateLinkSource( const String& rItem )
{
    return pDoc->CreateLinkSource( rItem );
}

void SwDocShell::ReconnectDdeLink(SfxObjectShell& rServer)
{
    if ( pDoc ) {
        ::sfx2::LinkManager& rLinkManager = pDoc->GetLinkManager();
        rLinkManager.ReconnectDdeLink(rServer);
    }
}

void SwDocShell::FillClass( SvGlobalName * pClassName,
                                   sal_uInt32 * pClipFormat,
                                   String * /*pAppName*/,
                                   String * pLongUserName,
                                   String * pUserName,
                                   sal_Int32 nVersion,
                                   sal_Bool bTemplate /* = sal_False */) const
{
    if (nVersion == SOFFICE_FILEFORMAT_60)
    {
        *pClassName     = SvGlobalName( SO3_SW_CLASSID_60 );
        *pClipFormat    = SOT_FORMATSTR_ID_STARWRITER_60;
        *pLongUserName = SW_RESSTR(STR_WRITER_DOCUMENT_FULLTYPE);
    }
    else if (nVersion == SOFFICE_FILEFORMAT_8)
    {
        *pClassName     = SvGlobalName( SO3_SW_CLASSID_60 );
        *pClipFormat    = bTemplate ? SOT_FORMATSTR_ID_STARWRITER_8_TEMPLATE : SOT_FORMATSTR_ID_STARWRITER_8;
        *pLongUserName = SW_RESSTR(STR_WRITER_DOCUMENT_FULLTYPE);
    }
// #FIXME check with new Event handling
#if 0
    uno::Reference< document::XVbaEventsHelper > xVbaEventsHelper = pDoc->GetVbaEventsHelper();
    if( xVbaEventsHelper.is() )
        lcl_processCompatibleSfxHint( xVbaEventsHelper, rHint );
#endif

    *pUserName = SW_RESSTR(STR_HUMAN_SWDOC_NAME);
}

void SwDocShell::SetModified( sal_Bool bSet )
{
    SfxObjectShell::SetModified( bSet );
    if( IsEnableSetModified())
    {
         if (!pDoc->IsInCallModified() )
         {
            EnableSetModified( sal_False );
            if( bSet )
            {
                sal_Bool bOld = pDoc->IsModified();
                pDoc->SetModified();
                if( !bOld )
                {
                    pDoc->GetIDocumentUndoRedo().SetUndoNoResetModified();
                }
            }
            else
                pDoc->ResetModified();

            EnableSetModified( sal_True );
         }

        UpdateChildWindows();
        Broadcast(SfxSimpleHint(SFX_HINT_DOCCHANGED));
    }
}

void SwDocShell::UpdateChildWindows()
{
    // if necessary newly initialize Flddlg (i.e. for TYP_SETVAR)
    if(!GetView())
        return;
    SfxViewFrame* pVFrame = GetView()->GetViewFrame();
    SwFldDlgWrapper *pWrp = (SwFldDlgWrapper*)pVFrame->
            GetChildWindow( SwFldDlgWrapper::GetChildWindowId() );
    if( pWrp )
        pWrp->ReInitDlg( this );

    // if necessary newly initialize RedlineDlg
    SwRedlineAcceptChild *pRed = (SwRedlineAcceptChild*)pVFrame->
            GetChildWindow( SwRedlineAcceptChild::GetChildWindowId() );
    if( pRed )
        pRed->ReInitDlg( this );
}

// #i48748#
class SwReloadFromHtmlReader : public SwReader
{
    public:
        SwReloadFromHtmlReader( SfxMedium& _rTmpMedium,
                                const String& _rFilename,
                                SwDoc* _pDoc )
            : SwReader( _rTmpMedium, _rFilename, _pDoc )
        {
            SetBaseURL( _rFilename );
        }
};

void SwDocShell::ReloadFromHtml( const String& rStreamName, SwSrcView* pSrcView )
{
    sal_Bool bModified = IsModified();

    // The HTTP-Header fields have to be removed, otherwise
    // there are some from Meta-Tags dublicated or triplicated afterwards.
    ClearHeaderAttributesForSourceViewHack();

#ifndef DISABLE_SCRIPTING
    // The Document-Basic also bites the dust ...
    // A EnterBasicCall is not needed here, because nothing is called and
    // there can't be any Dok-Basic, that has not yet been loaded inside
    // of an HTML document.
    SvxHtmlOptions& rHtmlOptions = SvxHtmlOptions::Get();
    //#59620# HasBasic() shows, that there already is a BasicManager at the DocShell.
    //          That was always generated in HTML-Import, when there are
    //          Macros in the source code.
    if( rHtmlOptions.IsStarBasic() && HasBasic())
    {
        BasicManager *pBasicMan = GetBasicManager();
        if( pBasicMan && (pBasicMan != SFX_APP()->GetBasicManager()) )
        {
            sal_uInt16 nLibCount = pBasicMan->GetLibCount();
            while( nLibCount )
            {
                StarBASIC *pBasic = pBasicMan->GetLib( --nLibCount );
                if( pBasic )
                {
                    // Notify the IDE
                    SfxUsrAnyItem aShellItem( SID_BASICIDE_ARG_DOCUMENT_MODEL, makeAny( GetModel() ) );
                    String aLibName( pBasic->GetName() );
                    SfxStringItem aLibNameItem( SID_BASICIDE_ARG_LIBNAME, aLibName );
                    pSrcView->GetViewFrame()->GetDispatcher()->Execute(
                                            SID_BASICIDE_LIBREMOVED,
                                            SFX_CALLMODE_SYNCHRON,
                                            &aShellItem, &aLibNameItem, 0L );

                    // Only the modules are deleted from the standard-lib
                    if( nLibCount )
                        pBasicMan->RemoveLib( nLibCount, sal_True );
                    else
                        pBasic->Clear();
                }
            }

            OSL_ENSURE( pBasicMan->GetLibCount() <= 1,
                    "Deleting Basics didn't work" );
        }
    }
#endif
    sal_Bool bWasBrowseMode = pDoc->get(IDocumentSettingAccess::BROWSE_MODE);
    RemoveLink();

    // now also the UNO-Model has to be informed about the new Doc #51535#
    uno::Reference<text::XTextDocument> xDoc(GetBaseModel(), uno::UNO_QUERY);
    text::XTextDocument* pxDoc = xDoc.get();
    ((SwXTextDocument*)pxDoc)->InitNewDoc();

    AddLink();
    //#116402# update font list when new document is created
    UpdateFontList();
    pDoc->set(IDocumentSettingAccess::BROWSE_MODE, bWasBrowseMode);
    pSrcView->SetPool(&GetPool());


    const String& rMedname = GetMedium()->GetName();

    // The HTML template still has to be set
    SetHTMLTemplate( *GetDoc() );   //Styles from HTML.vor

    SfxViewShell* pViewShell = GetView() ? (SfxViewShell*)GetView()
                                         : SfxViewShell::Current();
    SfxViewFrame*  pViewFrm = pViewShell->GetViewFrame();
    pViewFrm->GetDispatcher()->Execute( SID_VIEWSHELL0, SFX_CALLMODE_SYNCHRON );

    SubInitNew();

    SfxMedium aMed( rStreamName, STREAM_READ );
    // #i48748# - use class <SwReloadFromHtmlReader>, because
    // the base URL has to be set to the filename of the document <rMedname>
    // and not to the base URL of the temporary file <aMed> in order to get
    // the URLs of the linked graphics correctly resolved.
    SwReloadFromHtmlReader aReader( aMed, rMedname, pDoc );

    aReader.Read( *ReadHTML );

    const SwView* pCurrView = GetView();
    //in print layout the first page(s) may have been formatted as a mix of browse
    //and print layout
    if(!bWasBrowseMode && pCurrView)
    {
        SwWrtShell& rWrtSh = pCurrView->GetWrtShell();
        if( rWrtSh.GetLayout())
            rWrtSh.CheckBrowseView( sal_True );
    }


    // Take HTTP-Header-Attibutes over into the DokInfo again.
    // The Base-URL doesn't matter here because TLX uses the one from the document
    // for absolutization.
    SetHeaderAttributesForSourceViewHack();

    if(bModified && !IsReadOnly())
        SetModified();
    else
        pDoc->ResetModified();
}

sal_uLong SwDocShell::LoadStylesFromFile( const String& rURL,
                    SwgReaderOption& rOpt, sal_Bool bUnoCall )
{
    sal_uLong nErr = 0;

    // Create a URL from filename
    INetURLObject aURLObj( rURL );
    String sURL( aURLObj.GetMainURL( INetURLObject::NO_DECODE ) );

    // Set filter:
    String sFactory(rtl::OUString::createFromAscii(SwDocShell::Factory().GetShortName()));
    SfxFilterMatcher aMatcher( sFactory );

    // search for filter in WebDocShell, too
    SfxMedium aMed( rURL, STREAM_STD_READ );
    const SfxFilter* pFlt = 0;
    aMatcher.DetectFilter( aMed, &pFlt, sal_False, sal_False );
    if(!pFlt)
    {
        String sWebFactory(rtl::OUString::createFromAscii(SwWebDocShell::Factory().GetShortName()));
        SfxFilterMatcher aWebMatcher( sWebFactory );
        aWebMatcher.DetectFilter( aMed, &pFlt, sal_False, sal_False );
    }
    // --> OD #i117339# - trigger import only for own formats
//    if( aMed.IsStorage() )
    bool bImport( false );
    {
        if ( aMed.IsStorage() )
        {
            // As <SfxMedium.GetFilter().IsOwnFormat() resp. IsOwnTemplateFormat()
            // does not work correct (e.g., MS Word 2007 XML Template),
            // use workaround provided by MAV.
            uno::Reference< embed::XStorage > xStorage = aMed.GetStorage();
            if ( xStorage.is() )
            {
                // use <try-catch> on retrieving <MediaType> in order to check,
                // if the storage is one of our own ones.
                try
                {
                    uno::Reference< beans::XPropertySet > xProps( xStorage, uno::UNO_QUERY_THROW );
                    const ::rtl::OUString aMediaTypePropName( RTL_CONSTASCII_USTRINGPARAM( "MediaType" ) );
                    xProps->getPropertyValue( aMediaTypePropName );
                    bImport = true;
                }
                catch (const uno::Exception&)
                {
                    bImport = false;
                }
            }
        }
    }
    if ( bImport )
    // <--
    {
        SwRead pRead =  ReadXML;
        SwReader* pReader = 0;
        SwPaM* pPam = 0;
        // the SW3IO - Reader need the pam/wrtshell, because only then he
        // insert the styles!
        if( bUnoCall )
        {
            SwNodeIndex aIdx( pDoc->GetNodes().GetEndOfContent(), -1 );
            pPam = new SwPaM( aIdx );
            pReader = new SwReader( aMed, rURL, *pPam );
        }
        else
        {
            pReader = new SwReader( aMed, rURL, *pWrtShell->GetCrsr() );
        }

        pRead->GetReaderOpt().SetTxtFmts( rOpt.IsTxtFmts() );
        pRead->GetReaderOpt().SetFrmFmts( rOpt.IsFrmFmts() );
        pRead->GetReaderOpt().SetPageDescs( rOpt.IsPageDescs() );
        pRead->GetReaderOpt().SetNumRules( rOpt.IsNumRules() );
        pRead->GetReaderOpt().SetMerge( rOpt.IsMerge() );

        if( bUnoCall )
        {
            UnoActionContext aAction( pDoc );
            nErr = pReader->Read( *pRead );
        }
        else
        {
            pWrtShell->StartAllAction();
            nErr = pReader->Read( *pRead );
            pWrtShell->EndAllAction();
        }
        delete pPam;
        delete pReader;
    }

    return nErr;
}

/*--------------------------------------------------------------------
    Get a client for an embedded object if possible.
 --------------------------------------------------------------------*/
SfxInPlaceClient* SwDocShell::GetIPClient( const ::svt::EmbeddedObjectRef& xObjRef )
{
    SfxInPlaceClient* pResult = NULL;

    SwWrtShell* pShell = GetWrtShell();
    if ( pShell )
    {
        pResult = pShell->GetView().FindIPClient( xObjRef.GetObject(), (Window*)&pShell->GetView().GetEditWin() );
        if ( !pResult )
            pResult = new SwOleClient( &pShell->GetView(), &pShell->GetView().GetEditWin(), xObjRef );
    }

    return pResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
