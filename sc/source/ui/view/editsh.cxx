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
#include "precompiled_sc.hxx"



//------------------------------------------------------------------

#include <com/sun/star/linguistic2/XThesaurus.hpp>

#include "scitems.hxx"
#include <editeng/eeitem.hxx>

#include <svx/clipfmtitem.hxx>
#include <svx/svxdlg.hxx>
#include <editeng/cntritem.hxx>
#include <editeng/outliner.hxx>
#include <editeng/unolingu.hxx>
#include <editeng/crsditem.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <editeng/escpitem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/fontitem.hxx>
#include <svx/hlnkitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/shdditem.hxx>
#include <svl/srchitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <sfx2/basedlgs.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <sot/exchange.hxx>
#include <svtools/cliplistener.hxx>
#include <svl/whiter.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/sound.hxx>
#include <sot/formats.hxx>
#include <svtools/transfer.hxx>
#include <svl/stritem.hxx>

#define _EDITSH_CXX
#include "editsh.hxx"

#include "scresid.hxx"
#include "global.hxx"
#include "sc.hrc"
#include "scmod.hxx"
#include "inputhdl.hxx"
#include "viewutil.hxx"
#include "viewdata.hxx"
#include "document.hxx"
//CHINA001 #include "namepast.hxx"
#include "reffind.hxx"
#include "tabvwsh.hxx"
//CHINA001 #include "textdlgs.hxx"
#include "editutil.hxx"
#include "globstr.hrc"

#define ScEditShell
#include "scslots.hxx"

#include "scui_def.hxx" //CHINA001
#include "scabstdlg.hxx" //CHINA001


using namespace ::com::sun::star;


TYPEINIT1( ScEditShell, SfxShell );

SFX_IMPL_INTERFACE(ScEditShell, SfxShell, ScResId(SCSTR_EDITSHELL))
{
    SFX_POPUPMENU_REGISTRATION( ScResId(RID_POPUP_EDIT) );
}


ScEditShell::ScEditShell(EditView* pView, ScViewData* pData) :
    pEditView       (pView),
    pViewData       (pData),
    pClipEvtLstnr   (NULL),
    bPastePossible  (sal_False),
    bIsInsertMode   (sal_True)
{
    SetPool( pEditView->GetEditEngine()->GetEmptyItemSet().GetPool() );
    SetUndoManager( &pEditView->GetEditEngine()->GetUndoManager() );
    SetName(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("EditCell")));
}

ScEditShell::~ScEditShell()
{
    if ( pClipEvtLstnr )
    {
        pClipEvtLstnr->AddRemoveListener( pViewData->GetActiveWin(), sal_False );

        //  #122057# The listener may just now be waiting for the SolarMutex and call the link
        //  afterwards, in spite of RemoveListener. So the link has to be reset, too.
        pClipEvtLstnr->ClearCallbackLink();

        pClipEvtLstnr->release();
    }
}

ScInputHandler* ScEditShell::GetMyInputHdl()
{
    return SC_MOD()->GetInputHdl( pViewData->GetViewShell() );
}

void ScEditShell::SetEditView(EditView* pView)
{
    pEditView = pView;
    pEditView->SetInsertMode( bIsInsertMode );
    SetPool( pEditView->GetEditEngine()->GetEmptyItemSet().GetPool() );
    SetUndoManager( &pEditView->GetEditEngine()->GetUndoManager() );
}

void lcl_RemoveAttribs( EditView& rEditView )
{
    ScEditEngineDefaulter* pEngine = static_cast<ScEditEngineDefaulter*>(rEditView.GetEditEngine());

    sal_Bool bOld = pEngine->GetUpdateMode();
    pEngine->SetUpdateMode(sal_False);

    String aName = ScGlobal::GetRscString( STR_UNDO_DELETECONTENTS );
    pEngine->GetUndoManager().EnterListAction( aName, aName );

    rEditView.RemoveAttribs(sal_True);
    pEngine->RepeatDefaults();      // #97226# paragraph attributes from cell formats must be preserved

    pEngine->GetUndoManager().LeaveListAction();

    pEngine->SetUpdateMode(bOld);
}

void lclInsertCharacter( EditView* pTableView, EditView* pTopView, sal_Unicode cChar )
{
    String aString( cChar );
    if( pTableView )
        pTableView->InsertText( aString );
    if( pTopView )
        pTopView->InsertText( aString );
}

void ScEditShell::Execute( SfxRequest& rReq )
{
    const SfxItemSet*   pReqArgs    = rReq.GetArgs();
    sal_uInt16              nSlot       = rReq.GetSlot();
    SfxBindings&        rBindings   = pViewData->GetBindings();

    ScInputHandler* pHdl = GetMyInputHdl();
    DBG_ASSERT(pHdl,"kein ScInputHandler");

    EditView* pTopView   = pHdl->GetTopView();      // hat Eingabezeile den Focus?
    EditView* pTableView = pHdl->GetTableView();

    DBG_ASSERT(pTableView,"no EditView :-(");
    /* #i91683# No EditView if spell-check dialog is active and positioned on
     * an error and user immediately (without double click or F2) selected a
     * text portion of that cell with the mouse and wanted to modify it. */
    /* FIXME: Bailing out only cures the symptom and prevents a crash, no edit
     * action is possible. A real fix somehow would need to create a valid
     * EditView from the spell-check view. */
    if (!pTableView)
        return;

    EditEngine* pEngine = pTableView->GetEditEngine();

    pHdl->DataChanging();
    sal_Bool bSetSelIsRef = sal_False;
    sal_Bool bSetModified = sal_True;

    switch ( nSlot )
    {
        case FID_INS_CELL_CONTENTS: // Insert-Taste, weil als Acc definiert
            bIsInsertMode = !pTableView->IsInsertMode();
            pTableView->SetInsertMode( bIsInsertMode );
            if (pTopView)
                pTopView->SetInsertMode( bIsInsertMode );
            rBindings.Invalidate( SID_ATTR_INSERT );
            break;

        case SID_ATTR_INSERT:
            if ( pReqArgs )
            {
                bIsInsertMode = ((const SfxBoolItem&)pReqArgs->Get(nSlot)).GetValue();
                pTableView->SetInsertMode( bIsInsertMode );
                if (pTopView)
                    pTopView->SetInsertMode( bIsInsertMode );
                rBindings.Invalidate( SID_ATTR_INSERT );
            }
            break;

        case SID_THES:
            {
                String aReplaceText;
                SFX_REQUEST_ARG( rReq, pItem2, SfxStringItem, SID_THES , sal_False );
                if (pItem2)
                    aReplaceText = pItem2->GetValue();
                if (aReplaceText.Len() > 0)
                    ReplaceTextWithSynonym( *pEditView, aReplaceText );
            }
            break;

        case SID_COPY:
            pTableView->Copy();
            bSetModified = sal_False;
            break;

        case SID_CUT:
            pTableView->Cut();
            if (pTopView)
                pTopView->DeleteSelected();
            break;

        case SID_PASTE:
            pTableView->PasteSpecial();
            if (pTopView)
                pTopView->Paste();
            break;

        case SID_DELETE:
            pTableView->DeleteSelected();
            if (pTopView)
                pTopView->DeleteSelected();
            break;

        case SID_CELL_FORMAT_RESET:                 // "Standard"
            lcl_RemoveAttribs( *pTableView );
            if ( pTopView )
                lcl_RemoveAttribs( *pTopView );
            break;

        case SID_CLIPBOARD_FORMAT_ITEMS:
            {
                sal_uLong nFormat = 0;
                const SfxPoolItem* pItem;
                if ( pReqArgs &&
                     pReqArgs->GetItemState(nSlot, sal_True, &pItem) == SFX_ITEM_SET &&
                     pItem->ISA(SfxUInt32Item) )
                {
                    nFormat = ((const SfxUInt32Item*)pItem)->GetValue();
                }

                if ( nFormat )
                {
                    if (SOT_FORMAT_STRING == nFormat)
                        pTableView->Paste();
                    else
                        pTableView->PasteSpecial();

                    if (pTopView)
                        pTopView->Paste();
                }
            }
            break;

        case SID_PASTE_SPECIAL:
            {
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                SfxAbstractPasteDialog* pDlg = pFact->CreatePasteDialog( pViewData->GetDialogParent() );
                sal_uLong nFormat = 0;
                if ( pDlg )
                {
                    pDlg->Insert( SOT_FORMAT_STRING, EMPTY_STRING );
                    pDlg->Insert( SOT_FORMAT_RTF,    EMPTY_STRING );

                    TransferableDataHelper aDataHelper(
                        TransferableDataHelper::CreateFromSystemClipboard( pViewData->GetActiveWin() ) );

                    nFormat = pDlg->GetFormat( aDataHelper.GetTransferable() );
                    DELETEZ(pDlg);
                }

                // while the dialog was open, edit mode may have been stopped
                if (!SC_MOD()->IsInputMode())
                {
                    Sound::Beep();
                    return;
                }

                if (nFormat > 0)
                {
                    if (SOT_FORMAT_STRING == nFormat)
                        pTableView->Paste();
                    else
                        pTableView->PasteSpecial();

                    if (pTopView)
                        pTopView->Paste();
                }

                if (pTopView)
                    pTopView->GetWindow()->GrabFocus();
            }
            break;

        case SID_SELECTALL:
            {
                sal_uInt16 nPar = pEngine->GetParagraphCount();
                if (nPar)
                {
                    xub_StrLen nLen = pEngine->GetTextLen(nPar-1);
                    pTableView->SetSelection(ESelection(0,0,nPar-1,nLen));
                    if (pTopView)
                        pTopView->SetSelection(ESelection(0,0,nPar-1,nLen));
                }
                bSetModified = sal_False;
            }
            break;

        case SID_CHARMAP:
            {
                sal_uInt16 nScript = pTableView->GetSelectedScriptType();
                sal_uInt16 nFontWhich = ( nScript == SCRIPTTYPE_ASIAN ) ? EE_CHAR_FONTINFO_CJK :
                                ( ( nScript == SCRIPTTYPE_COMPLEX ) ? EE_CHAR_FONTINFO_CTL :
                                                                        EE_CHAR_FONTINFO );
                const SvxFontItem& rItem = (const SvxFontItem&)
                            pTableView->GetAttribs().Get(nFontWhich);

                String aString;
                SvxFontItem aNewItem( EE_CHAR_FONTINFO );

                const SfxItemSet *pArgs = rReq.GetArgs();
                const SfxPoolItem* pItem = 0;
                if( pArgs )
                    pArgs->GetItemState(GetPool().GetWhich(SID_CHARMAP), sal_False, &pItem);

                if ( pItem )
                {
                    aString = ((const SfxStringItem*)pItem)->GetValue();
                    const SfxPoolItem* pFtItem = NULL;
                    pArgs->GetItemState( GetPool().GetWhich(SID_ATTR_SPECIALCHAR), sal_False, &pFtItem);
                    const SfxStringItem* pFontItem = PTR_CAST( SfxStringItem, pFtItem );
                    if ( pFontItem )
                    {
                        String aFontName(pFontItem->GetValue());
                        Font aFont(aFontName, Size(1,1)); // Size nur wg. CTOR
                        aNewItem = SvxFontItem( aFont.GetFamily(), aFont.GetName(),
                                    aFont.GetStyleName(), aFont.GetPitch(),
                                    aFont.GetCharSet(), ATTR_FONT  );
                    }
                    else
                        aNewItem = rItem;
                }
                else
                {
                    ScViewUtil::ExecuteCharMap( rItem, *pViewData->GetViewShell()->GetViewFrame(), aNewItem, aString );

                    // while the dialog was open, edit mode may have been stopped
                    if (!SC_MOD()->IsInputMode())
                    {
                        Sound::Beep();
                        return;
                    }
                }

                if ( aString.Len() )
                {
                    //  if string contains WEAK characters, set all fonts
                    sal_uInt8 nSetScript;
                    ScDocument* pDoc = pViewData->GetDocument();
                    if ( pDoc->HasStringWeakCharacters( aString ) )
                        nSetScript = SCRIPTTYPE_LATIN | SCRIPTTYPE_ASIAN | SCRIPTTYPE_COMPLEX;
                    else
                        nSetScript = pDoc->GetStringScriptType( aString );

                    SfxItemSet aSet( pTableView->GetEmptyItemSet() );
                    SvxScriptSetItem aSetItem( SID_ATTR_CHAR_FONT, GetPool() );
                    aSetItem.PutItemForScriptType( nSetScript, aNewItem );
                    aSet.Put( aSetItem.GetItemSet(), sal_False );

                    //  SetAttribs an der View selektiert ein Wort, wenn nichts selektiert ist
                    pTableView->GetEditEngine()->QuickSetAttribs( aSet, pTableView->GetSelection() );
                    pTableView->InsertText(aString);
                    if (pTopView)
                        pTopView->InsertText(aString);

                    SfxStringItem aStringItem( SID_CHARMAP, aString );
                    SfxStringItem aFontItem( SID_ATTR_SPECIALCHAR, aNewItem.GetFamilyName() );
                    rReq.AppendItem( aFontItem );
                    rReq.AppendItem( aStringItem );
                    rReq.Done();


                }

                if (pTopView)
                    pTopView->GetWindow()->GrabFocus();
            }
            break;

        case FID_INSERT_NAME:
            {
                ScDocument*     pDoc = pViewData->GetDocument();
                //CHINA001 ScNamePasteDlg* pDlg = new ScNamePasteDlg( pViewData->GetDialogParent(),
                //CHINA001                              pDoc->GetRangeName(), sal_False );
                                                // "Liste" disablen
                ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                DBG_ASSERT(pFact, "ScAbstractFactory create fail!");//CHINA001

                AbstractScNamePasteDlg* pDlg = pFact->CreateScNamePasteDlg( pViewData->GetDialogParent(), pDoc->GetRangeName(), RID_SCDLG_NAMES_PASTE, sal_False );
                DBG_ASSERT(pDlg, "Dialog create fail!");//CHINA001
                short nRet = pDlg->Execute();
                // pDlg is needed below

                // while the dialog was open, edit mode may have been stopped
                if (!SC_MOD()->IsInputMode())
                {
                    Sound::Beep();
                    delete pDlg;
                    return;
                }

                if ( nRet == BTN_PASTE_NAME )
                {
                    String aName = pDlg->GetSelectedName();
                    pTableView->InsertText(aName);
                    if (pTopView)
                        pTopView->InsertText(aName);
                }
                delete pDlg;

                if (pTopView)
                    pTopView->GetWindow()->GrabFocus();
            }
            break;

        case SID_CHAR_DLG:
            {
                SfxItemSet aAttrs( pTableView->GetAttribs() );

                SfxObjectShell* pObjSh = pViewData->GetSfxDocShell();

                //CHINA001 ScCharDlg* pDlg = new ScCharDlg( pViewData->GetDialogParent(), &aAttrs, pObjSh );
                ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                DBG_ASSERT(pFact, "ScAbstractFactory create fail!");//CHINA001

                SfxAbstractTabDialog* pDlg = pFact->CreateScCharDlg( pViewData->GetDialogParent(), &aAttrs,
                                                                     pObjSh, RID_SCDLG_CHAR );
                DBG_ASSERT(pDlg, "Dialog create fail!");//CHINA001
                short nRet = pDlg->Execute();
                // pDlg is needed below

                // while the dialog was open, edit mode may have been stopped
                if (!SC_MOD()->IsInputMode())
                {
                    Sound::Beep();
                    delete pDlg;
                    return;
                }

                if ( nRet == RET_OK )
                {
                    const SfxItemSet* pOut = pDlg->GetOutputItemSet();
                    pTableView->SetAttribs( *pOut );
                }
                delete pDlg;
            }
            break;

        case SID_TOGGLE_REL:
            {
                sal_Bool bOk = sal_False;
                if (pEngine->GetParagraphCount() == 1)
                {
                    String aText = pEngine->GetText();
                    ESelection aSel = pEditView->GetSelection();    // aktuelle View

                    ScRefFinder aFinder( aText, pViewData->GetDocument() );
                    aFinder.ToggleRel( aSel.nStartPos, aSel.nEndPos );
                    if (aFinder.GetFound())
                    {
                        String aNew = aFinder.GetText();
                        ESelection aNewSel( 0,aFinder.GetSelStart(), 0,aFinder.GetSelEnd() );
                        pEngine->SetText( aNew );
                        pTableView->SetSelection( aNewSel );
                        if ( pTopView )
                        {
                            pTopView->GetEditEngine()->SetText( aNew );
                            pTopView->SetSelection( aNewSel );
                        }
                        bOk = sal_True;

                        //  Referenz wird selektiert -> beim Tippen nicht ueberschreiben
                        bSetSelIsRef = sal_True;
                    }
                }
                if (!bOk)
                    Sound::Beep();              // keine Referenzen oder mehrere Absaetze
            }
            break;

        case SID_HYPERLINK_SETLINK:
            if( pReqArgs )
            {
                const SfxPoolItem* pItem;
                if ( pReqArgs->GetItemState( SID_HYPERLINK_SETLINK, sal_True, &pItem ) == SFX_ITEM_SET )
                {
                    const SvxHyperlinkItem* pHyper = (const SvxHyperlinkItem*) pItem;
                    const String& rName     = pHyper->GetName();
                    const String& rURL      = pHyper->GetURL();
                    const String& rTarget   = pHyper->GetTargetFrame();
                    SvxLinkInsertMode eMode = pHyper->GetInsertMode();

                    sal_Bool bDone = sal_False;
                    if ( eMode == HLINK_DEFAULT || eMode == HLINK_FIELD )
                    {
                        const SvxURLField* pURLField = GetURLField();
                        if ( pURLField )
                        {
                            //  altes Feld selektieren

                            ESelection aSel = pTableView->GetSelection();
                            aSel.Adjust();
                            aSel.nEndPara = aSel.nStartPara;
                            aSel.nEndPos = aSel.nStartPos + 1;
                            pTableView->SetSelection( aSel );

                            //  neues Feld einfuegen

                            SvxURLField aURLField( rURL, rName, SVXURLFORMAT_REPR );
                            aURLField.SetTargetFrame( rTarget );
                            SvxFieldItem aURLItem( aURLField, EE_FEATURE_FIELD );
                            pTableView->InsertField( aURLItem );
                            pTableView->SetSelection( aSel );       // select inserted field

                            //  #57254# jetzt doch auch Felder in der Top-View

                            if ( pTopView )
                            {
                                aSel = pTopView->GetSelection();
                                aSel.nEndPara = aSel.nStartPara;
                                aSel.nEndPos = aSel.nStartPos + 1;
                                pTopView->SetSelection( aSel );
                                pTopView->InsertField( aURLItem );
                                pTopView->SetSelection( aSel );     // select inserted field
                            }

                            bDone = sal_True;
                        }
                    }

                    if (!bDone)
                    {
                        pViewData->GetViewShell()->
                            InsertURL( rName, rURL, rTarget, (sal_uInt16) eMode );

                        //  InsertURL an der ViewShell schaltet bei "Button"
                        //  die EditShell ab, darum sofort return

                        return;
                    }
                }
            }
            break;

            case SID_OPEN_HYPERLINK:
                {
                    const SvxURLField* pURLField = GetURLField();
                    if ( pURLField )
                        ScGlobal::OpenURL( pURLField->GetURL(), pURLField->GetTargetFrame() );
                    return;
                }
                //break;

        case FN_INSERT_SOFT_HYPHEN:
            lclInsertCharacter( pTableView, pTopView, CHAR_SHY );
        break;
        case FN_INSERT_HARDHYPHEN:
            lclInsertCharacter( pTableView, pTopView, CHAR_NBHY );
        break;
        case FN_INSERT_HARD_SPACE:
            lclInsertCharacter( pTableView, pTopView, CHAR_NBSP );
        break;
        case SID_INSERT_RLM:
            lclInsertCharacter( pTableView, pTopView, CHAR_RLM );
        break;
        case SID_INSERT_LRM:
            lclInsertCharacter( pTableView, pTopView, CHAR_LRM );
        break;
        case SID_INSERT_ZWSP:
            lclInsertCharacter( pTableView, pTopView, CHAR_ZWSP );
        break;
        case SID_INSERT_ZWNBSP:
            lclInsertCharacter( pTableView, pTopView, CHAR_ZWNBSP );
        break;
    }

    pHdl->DataChanged(sal_False, bSetModified);
    if (bSetSelIsRef)
        pHdl->SetSelIsRef(sal_True);
}

void lcl_DisableAll( SfxItemSet& rSet )    // disable all slots
{
    SfxWhichIter aIter( rSet );
    sal_uInt16 nWhich = aIter.FirstWhich();
    while (nWhich)
    {
        rSet.DisableItem( nWhich );
        nWhich = aIter.NextWhich();
    }
}

void __EXPORT ScEditShell::GetState( SfxItemSet& rSet )
{
    // #125326# When deactivating the view, edit mode is stopped, but the EditShell is left active
    // (a shell can't be removed from within Deactivate). In that state, the EditView isn't inserted
    // into the EditEngine, so it can have an invalid selection and must not be used.
    if ( !pViewData->HasEditView( pViewData->GetActivePart() ) )
    {
        lcl_DisableAll( rSet );
        return;
    }

    ScInputHandler* pHdl = GetMyInputHdl();
    EditView* pActiveView = pHdl ? pHdl->GetActiveView() : pEditView;

    SfxWhichIter aIter( rSet );
    sal_uInt16 nWhich = aIter.FirstWhich();
    while (nWhich)
    {
        switch (nWhich)
        {
            case SID_ATTR_INSERT:   // Statuszeile
                {
                    if ( pActiveView )
                        rSet.Put( SfxBoolItem( nWhich, pActiveView->IsInsertMode() ) );
                    else
                        rSet.Put( SfxBoolItem( nWhich, 42 ) );
                }
                break;

            case SID_HYPERLINK_GETLINK:
                {
                    SvxHyperlinkItem aHLinkItem;
                    const SvxURLField* pURLField = GetURLField();
                    if ( pURLField )
                    {
                        aHLinkItem.SetName( pURLField->GetRepresentation() );
                        aHLinkItem.SetURL( pURLField->GetURL() );
                        aHLinkItem.SetTargetFrame( pURLField->GetTargetFrame() );
                    }
                    else if ( pActiveView )
                    {
                        // use selected text as name for urls
                        String sReturn = pActiveView->GetSelected();
                        sReturn.Erase(255);
                        sReturn.EraseTrailingChars();
                        aHLinkItem.SetName(sReturn);
                    }
                    rSet.Put(aHLinkItem);
                }
                break;

            case SID_OPEN_HYPERLINK:
                {
                    if ( !GetURLField() )
                        rSet.DisableItem( nWhich );
                }
                break;

            case SID_TRANSLITERATE_HALFWIDTH:
            case SID_TRANSLITERATE_FULLWIDTH:
            case SID_TRANSLITERATE_HIRAGANA:
            case SID_TRANSLITERATE_KATAGANA:
            case SID_INSERT_RLM:
            case SID_INSERT_LRM:
            case SID_INSERT_ZWNBSP:
            case SID_INSERT_ZWSP:
                ScViewUtil::HideDisabledSlot( rSet, pViewData->GetBindings(), nWhich );
            break;

            case SID_THES:
                {
                    String          aStatusVal;
                    LanguageType    nLang = LANGUAGE_NONE;
                    bool bIsLookUpWord = GetStatusValueForThesaurusFromContext( aStatusVal, nLang, *pActiveView );
                    rSet.Put( SfxStringItem( SID_THES, aStatusVal ) );

                    // disable thesaurus context menu entry if there is nothing to look up
                    sal_Bool bCanDoThesaurus = ScModule::HasThesaurusLanguage( nLang );
                    if (!bIsLookUpWord || !bCanDoThesaurus)
                        rSet.DisableItem( SID_THES );
                }
                break;


        }
        nWhich = aIter.NextWhich();
    }
}

const SvxURLField* ScEditShell::GetURLField()
{
    ScInputHandler* pHdl = GetMyInputHdl();
    EditView* pActiveView = pHdl ? pHdl->GetActiveView() : pEditView;
    if ( pActiveView )
    {
        const SvxFieldItem* pFieldItem = pActiveView->GetFieldAtSelection();
        if (pFieldItem)
        {
            const SvxFieldData* pField = pFieldItem->GetField();
            if ( pField && pField->ISA(SvxURLField) )
                return (const SvxURLField*)pField;
        }
    }

    return NULL;
}

IMPL_LINK( ScEditShell, ClipboardChanged, TransferableDataHelper*, pDataHelper )
{
    if ( pDataHelper )
    {
        bPastePossible = ( pDataHelper->HasFormat( SOT_FORMAT_STRING ) || pDataHelper->HasFormat( SOT_FORMAT_RTF ) );

        SfxBindings& rBindings = pViewData->GetBindings();
        rBindings.Invalidate( SID_PASTE );
        rBindings.Invalidate( SID_PASTE_SPECIAL );
        rBindings.Invalidate( SID_CLIPBOARD_FORMAT_ITEMS );
    }
    return 0;
}

void __EXPORT ScEditShell::GetClipState( SfxItemSet& rSet )
{
    if ( !pClipEvtLstnr )
    {
        // create listener
        pClipEvtLstnr = new TransferableClipboardListener( LINK( this, ScEditShell, ClipboardChanged ) );
        pClipEvtLstnr->acquire();
        Window* pWin = pViewData->GetActiveWin();
        pClipEvtLstnr->AddRemoveListener( pWin, sal_True );

        // get initial state
        TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( pViewData->GetActiveWin() ) );
        bPastePossible = ( aDataHelper.HasFormat( SOT_FORMAT_STRING ) || aDataHelper.HasFormat( SOT_FORMAT_RTF ) );
    }

    SfxWhichIter aIter( rSet );
    sal_uInt16 nWhich = aIter.FirstWhich();
    while (nWhich)
    {
        switch (nWhich)
        {
            case SID_PASTE:
            case SID_PASTE_SPECIAL:
                if( !bPastePossible )
                    rSet.DisableItem( nWhich );
                break;
            case SID_CLIPBOARD_FORMAT_ITEMS:
                if( bPastePossible )
                {
                    SvxClipboardFmtItem aFormats( SID_CLIPBOARD_FORMAT_ITEMS );
                    TransferableDataHelper aDataHelper(
                            TransferableDataHelper::CreateFromSystemClipboard( pViewData->GetActiveWin() ) );

                    if ( aDataHelper.HasFormat( SOT_FORMAT_STRING ) )
                        aFormats.AddClipbrdFormat( SOT_FORMAT_STRING );
                    if ( aDataHelper.HasFormat( SOT_FORMAT_RTF ) )
                        aFormats.AddClipbrdFormat( SOT_FORMAT_RTF );

                    rSet.Put( aFormats );
                }
                else
                    rSet.DisableItem( nWhich );
                break;
        }
        nWhich = aIter.NextWhich();
    }
}

void lcl_InvalidateUnder( SfxBindings& rBindings )
{
    rBindings.Invalidate( SID_ATTR_CHAR_UNDERLINE );
    rBindings.Invalidate( SID_ULINE_VAL_NONE );
    rBindings.Invalidate( SID_ULINE_VAL_SINGLE );
    rBindings.Invalidate( SID_ULINE_VAL_DOUBLE );
    rBindings.Invalidate( SID_ULINE_VAL_DOTTED );
}

void ScEditShell::ExecuteAttr(SfxRequest& rReq)
{
    SfxItemSet          aSet( pEditView->GetEmptyItemSet() );
    SfxBindings&        rBindings   = pViewData->GetBindings();
    const SfxItemSet*   pArgs       = rReq.GetArgs();
    sal_uInt16              nSlot       = rReq.GetSlot();

    switch ( nSlot )
    {
        case SID_ATTR_CHAR_FONTHEIGHT:
        case SID_ATTR_CHAR_FONT:
            {
                if (pArgs)
                {
                    // #i78017 establish the same behaviour as in Writer
                    sal_uInt16 nScript = SCRIPTTYPE_LATIN | SCRIPTTYPE_ASIAN | SCRIPTTYPE_COMPLEX;
                    if (nSlot == SID_ATTR_CHAR_FONT)
                    {
                        nScript = pEditView->GetSelectedScriptType();
                        if (nScript == 0) nScript = ScGlobal::GetDefaultScriptType();
                    }

                    SfxItemPool& rPool = GetPool();
                    SvxScriptSetItem aSetItem( nSlot, rPool );
                    sal_uInt16 nWhich = rPool.GetWhich( nSlot );
                    aSetItem.PutItemForScriptType( nScript, pArgs->Get( nWhich ) );

                    aSet.Put( aSetItem.GetItemSet(), sal_False );
                }
            }
            break;

        case SID_ATTR_CHAR_COLOR:
            {
                if (pArgs)
                {
                    aSet.Put( pArgs->Get( pArgs->GetPool()->GetWhich( nSlot ) ) );
                    rBindings.Invalidate( nSlot );
                }
            }
            break;

        //  Toggles

        case SID_ATTR_CHAR_WEIGHT:
            {
                // #i78017 establish the same behaviour as in Writer
                sal_uInt16 nScript = SCRIPTTYPE_LATIN | SCRIPTTYPE_ASIAN | SCRIPTTYPE_COMPLEX;

                SfxItemPool& rPool = GetPool();

                sal_Bool bOld = sal_False;
                SvxScriptSetItem aOldSetItem( nSlot, rPool );
                aOldSetItem.GetItemSet().Put( pEditView->GetAttribs(), sal_False );
                const SfxPoolItem* pCore = aOldSetItem.GetItemOfScript( nScript );
                if ( pCore && ((const SvxWeightItem*)pCore)->GetWeight() > WEIGHT_NORMAL )
                    bOld = sal_True;

                SvxScriptSetItem aSetItem( nSlot, rPool );
                aSetItem.PutItemForScriptType( nScript,
                            SvxWeightItem( bOld ? WEIGHT_NORMAL : WEIGHT_BOLD, EE_CHAR_WEIGHT ) );
                aSet.Put( aSetItem.GetItemSet(), sal_False );

                rBindings.Invalidate( nSlot );
            }
            break;

        case SID_ATTR_CHAR_POSTURE:
            {
                // #i78017 establish the same behaviour as in Writer
                sal_uInt16 nScript = SCRIPTTYPE_LATIN | SCRIPTTYPE_ASIAN | SCRIPTTYPE_COMPLEX;

                SfxItemPool& rPool = GetPool();

                sal_Bool bOld = sal_False;
                SvxScriptSetItem aOldSetItem( nSlot, rPool );
                aOldSetItem.GetItemSet().Put( pEditView->GetAttribs(), sal_False );
                const SfxPoolItem* pCore = aOldSetItem.GetItemOfScript( nScript );
                if ( pCore && ((const SvxPostureItem*)pCore)->GetValue() != ITALIC_NONE )
                    bOld = sal_True;

                SvxScriptSetItem aSetItem( nSlot, rPool );
                aSetItem.PutItemForScriptType( nScript,
                            SvxPostureItem( bOld ? ITALIC_NONE : ITALIC_NORMAL, EE_CHAR_ITALIC ) );
                aSet.Put( aSetItem.GetItemSet(), sal_False );

                rBindings.Invalidate( nSlot );
            }
            break;

        case SID_ULINE_VAL_NONE:
            aSet.Put( SvxUnderlineItem( UNDERLINE_NONE, EE_CHAR_UNDERLINE ) );
            lcl_InvalidateUnder( rBindings );
            break;

        case SID_ATTR_CHAR_UNDERLINE:   // Toggles
        case SID_ULINE_VAL_SINGLE:
        case SID_ULINE_VAL_DOUBLE:
        case SID_ULINE_VAL_DOTTED:
            {
                FontUnderline eOld = ((const SvxUnderlineItem&) pEditView->
                                    GetAttribs().Get(EE_CHAR_UNDERLINE)).GetLineStyle();
                FontUnderline eNew = eOld;
                switch (nSlot)
                {
                    case SID_ATTR_CHAR_UNDERLINE:
                        eNew = ( eOld != UNDERLINE_NONE ) ? UNDERLINE_NONE : UNDERLINE_SINGLE;
                        break;
                    case SID_ULINE_VAL_SINGLE:
                        eNew = ( eOld == UNDERLINE_SINGLE ) ? UNDERLINE_NONE : UNDERLINE_SINGLE;
                        break;
                    case SID_ULINE_VAL_DOUBLE:
                        eNew = ( eOld == UNDERLINE_DOUBLE ) ? UNDERLINE_NONE : UNDERLINE_DOUBLE;
                        break;
                    case SID_ULINE_VAL_DOTTED:
                        eNew = ( eOld == UNDERLINE_DOTTED ) ? UNDERLINE_NONE : UNDERLINE_DOTTED;
                        break;
                }
                aSet.Put( SvxUnderlineItem( eNew, EE_CHAR_UNDERLINE ) );
                lcl_InvalidateUnder( rBindings );
            }
            break;

        case SID_ATTR_CHAR_OVERLINE:
            {
                FontUnderline eOld = ((const SvxOverlineItem&) pEditView->
                                    GetAttribs().Get(EE_CHAR_OVERLINE)).GetLineStyle();
                FontUnderline eNew = ( eOld != UNDERLINE_NONE ) ? UNDERLINE_NONE : UNDERLINE_SINGLE;
                aSet.Put( SvxOverlineItem( eNew, EE_CHAR_OVERLINE ) );
                rBindings.Invalidate( nSlot );
            }
            break;

        case SID_ATTR_CHAR_STRIKEOUT:
            {
                sal_Bool bOld = ((const SvxCrossedOutItem&)pEditView->GetAttribs().
                                Get(EE_CHAR_STRIKEOUT)).GetValue() != STRIKEOUT_NONE;
                aSet.Put( SvxCrossedOutItem( bOld ? STRIKEOUT_NONE : STRIKEOUT_SINGLE, EE_CHAR_STRIKEOUT ) );
                rBindings.Invalidate( nSlot );
            }
            break;

        case SID_ATTR_CHAR_SHADOWED:
            {
                sal_Bool bOld = ((const SvxShadowedItem&)pEditView->GetAttribs().
                                Get(EE_CHAR_SHADOW)).GetValue();
                aSet.Put( SvxShadowedItem( !bOld, EE_CHAR_SHADOW ) );
                rBindings.Invalidate( nSlot );
            }
            break;

        case SID_ATTR_CHAR_CONTOUR:
            {
                sal_Bool bOld = ((const SvxContourItem&)pEditView->GetAttribs().
                                Get(EE_CHAR_OUTLINE)).GetValue();
                aSet.Put( SvxContourItem( !bOld, EE_CHAR_OUTLINE ) );
                rBindings.Invalidate( nSlot );
            }
            break;

        case SID_SET_SUPER_SCRIPT:
            {
                SvxEscapement eOld = (SvxEscapement) ((const SvxEscapementItem&)
                        pEditView->GetAttribs().Get(EE_CHAR_ESCAPEMENT)).GetEnumValue();
                SvxEscapement eNew = (eOld == SVX_ESCAPEMENT_SUPERSCRIPT) ?
                                        SVX_ESCAPEMENT_OFF : SVX_ESCAPEMENT_SUPERSCRIPT;
                aSet.Put( SvxEscapementItem( eNew, EE_CHAR_ESCAPEMENT ) );
                rBindings.Invalidate( nSlot );
            }
            break;
        case SID_SET_SUB_SCRIPT:
            {
                SvxEscapement eOld = (SvxEscapement) ((const SvxEscapementItem&)
                        pEditView->GetAttribs().Get(EE_CHAR_ESCAPEMENT)).GetEnumValue();
                SvxEscapement eNew = (eOld == SVX_ESCAPEMENT_SUBSCRIPT) ?
                                        SVX_ESCAPEMENT_OFF : SVX_ESCAPEMENT_SUBSCRIPT;
                aSet.Put( SvxEscapementItem( eNew, EE_CHAR_ESCAPEMENT ) );
                rBindings.Invalidate( nSlot );
            }
            break;
    }

    //
    //  anwenden
    //

    EditEngine* pEngine = pEditView->GetEditEngine();
    sal_Bool bOld = pEngine->GetUpdateMode();
    pEngine->SetUpdateMode(sal_False);

    pEditView->SetAttribs( aSet );

    pEngine->SetUpdateMode(bOld);
    pEditView->Invalidate();

    ScInputHandler* pHdl = GetMyInputHdl();
    pHdl->SetModified();

    rReq.Done();
}

void ScEditShell::GetAttrState(SfxItemSet &rSet)
{
    if ( !pViewData->HasEditView( pViewData->GetActivePart() ) )    // #125326#
    {
        lcl_DisableAll( rSet );
        return;
    }

    SfxItemSet aAttribs = pEditView->GetAttribs();
    rSet.Put( aAttribs );

    //  choose font info according to selection script type

    sal_uInt16 nScript = pEditView->GetSelectedScriptType();
    if (nScript == 0) nScript = ScGlobal::GetDefaultScriptType();

    // #i55929# input-language-dependent script type (depends on input language if nothing selected)
    sal_uInt16 nInputScript = nScript;
    if ( !pEditView->GetSelection().HasRange() )
    {
        LanguageType nInputLang = pViewData->GetActiveWin()->GetInputLanguage();
        if (nInputLang != LANGUAGE_DONTKNOW && nInputLang != LANGUAGE_SYSTEM)
            nInputScript = SvtLanguageOptions::GetScriptTypeOfLanguage( nInputLang );
    }

    // #i55929# according to spec, nInputScript is used for font and font height only
    if ( rSet.GetItemState( EE_CHAR_FONTINFO ) != SFX_ITEM_UNKNOWN )
        ScViewUtil::PutItemScript( rSet, aAttribs, EE_CHAR_FONTINFO, nInputScript );
    if ( rSet.GetItemState( EE_CHAR_FONTHEIGHT ) != SFX_ITEM_UNKNOWN )
        ScViewUtil::PutItemScript( rSet, aAttribs, EE_CHAR_FONTHEIGHT, nInputScript );
    if ( rSet.GetItemState( EE_CHAR_WEIGHT ) != SFX_ITEM_UNKNOWN )
        ScViewUtil::PutItemScript( rSet, aAttribs, EE_CHAR_WEIGHT, nScript );
    if ( rSet.GetItemState( EE_CHAR_ITALIC ) != SFX_ITEM_UNKNOWN )
        ScViewUtil::PutItemScript( rSet, aAttribs, EE_CHAR_ITALIC, nScript );

    //  Unterstreichung

    SfxItemState eState = aAttribs.GetItemState( EE_CHAR_UNDERLINE, sal_True );
    if ( eState == SFX_ITEM_DONTCARE )
    {
        rSet.InvalidateItem( SID_ULINE_VAL_NONE );
        rSet.InvalidateItem( SID_ULINE_VAL_SINGLE );
        rSet.InvalidateItem( SID_ULINE_VAL_DOUBLE );
        rSet.InvalidateItem( SID_ULINE_VAL_DOTTED );
    }
    else
    {
        FontUnderline eUnderline = ((const SvxUnderlineItem&)
                    aAttribs.Get(EE_CHAR_UNDERLINE)).GetLineStyle();
        sal_uInt16 nId = SID_ULINE_VAL_NONE;
        switch (eUnderline)
        {
            case UNDERLINE_SINGLE:  nId = SID_ULINE_VAL_SINGLE; break;
            case UNDERLINE_DOUBLE:  nId = SID_ULINE_VAL_DOUBLE; break;
            case UNDERLINE_DOTTED:  nId = SID_ULINE_VAL_DOTTED; break;
            default:
                break;
        }
        rSet.Put( SfxBoolItem( nId, sal_True ) );
    }

    //! Testen, ob Klammer-Hervorhebung aktiv ist !!!!
    ScInputHandler* pHdl = GetMyInputHdl();
    if ( pHdl && pHdl->IsFormulaMode() )
        rSet.ClearItem( EE_CHAR_WEIGHT );   // hervorgehobene Klammern hier nicht
}

String ScEditShell::GetSelectionText( sal_Bool bWholeWord )
{
    String aStrSelection;

    if ( pViewData->HasEditView( pViewData->GetActivePart() ) )    // #125326#
    {
        if ( bWholeWord )
        {
            EditEngine* pEngine = pEditView->GetEditEngine();
            ESelection  aSel = pEditView->GetSelection();
            String      aStrCurrentDelimiters = pEngine->GetWordDelimiters();

            pEngine->SetWordDelimiters( String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(" .,;\"'")) );
            aStrSelection = pEngine->GetWord( aSel.nEndPara, aSel.nEndPos );
            pEngine->SetWordDelimiters( aStrCurrentDelimiters );
        }
        else
        {
            aStrSelection = pEditView->GetSelected();
        }
    }

    return aStrSelection;
}

void ScEditShell::ExecuteUndo(SfxRequest& rReq)
{
    //  #81733# Undo must be handled here because it's called for both EditViews

    ScInputHandler* pHdl = GetMyInputHdl();
    DBG_ASSERT(pHdl,"no ScInputHandler");
    EditView* pTopView   = pHdl->GetTopView();
    EditView* pTableView = pHdl->GetTableView();
    DBG_ASSERT(pTableView,"no EditView");

    pHdl->DataChanging();

    const SfxItemSet* pReqArgs = rReq.GetArgs();
    sal_uInt16 nSlot = rReq.GetSlot();
    switch ( nSlot )
    {
        case SID_UNDO:
        case SID_REDO:
            {
                sal_Bool bIsUndo = ( nSlot == SID_UNDO );

                sal_uInt16 nCount = 1;
                const SfxPoolItem* pItem;
                if ( pReqArgs && pReqArgs->GetItemState( nSlot, sal_True, &pItem ) == SFX_ITEM_SET )
                    nCount = ((const SfxUInt16Item*)pItem)->GetValue();

                for (sal_uInt16 i=0; i<nCount; i++)
                {
                    if ( bIsUndo )
                    {
                        pTableView->Undo();
                        if (pTopView)
                            pTopView->Undo();
                    }
                    else
                    {
                        pTableView->Redo();
                        if (pTopView)
                            pTopView->Redo();
                    }
                }
            }
            break;
    }
    pViewData->GetBindings().InvalidateAll(sal_False);

    pHdl->DataChanged();
}

void ScEditShell::GetUndoState(SfxItemSet &rSet)
{
    //  Undo state is taken from normal ViewFrame state function

    SfxViewFrame* pViewFrm = pViewData->GetViewShell()->GetViewFrame();
    if ( pViewFrm && GetUndoManager() )
    {
        SfxWhichIter aIter(rSet);
        sal_uInt16 nWhich = aIter.FirstWhich();
        while( nWhich )
        {
            pViewFrm->GetSlotState( nWhich, NULL, &rSet );
            nWhich = aIter.NextWhich();
        }
    }

    //  disable if no action in input line EditView

    ScInputHandler* pHdl = GetMyInputHdl();
    DBG_ASSERT(pHdl,"no ScInputHandler");
    EditView* pTopView = pHdl->GetTopView();
    if (pTopView)
    {
        ::svl::IUndoManager& rTopMgr = pTopView->GetEditEngine()->GetUndoManager();
        if ( rTopMgr.GetUndoActionCount() == 0 )
            rSet.DisableItem( SID_UNDO );
        if ( rTopMgr.GetRedoActionCount() == 0 )
            rSet.DisableItem( SID_REDO );
    }
}

void ScEditShell::ExecuteTrans( SfxRequest& rReq )
{
    sal_Int32 nType = ScViewUtil::GetTransliterationType( rReq.GetSlot() );
    if ( nType )
    {
        ScInputHandler* pHdl = GetMyInputHdl();
        DBG_ASSERT( pHdl, "no ScInputHandler" );

        EditView* pTopView   = pHdl->GetTopView();
        EditView* pTableView = pHdl->GetTableView();
        DBG_ASSERT( pTableView, "no EditView" );

        pHdl->DataChanging();

        pTableView->TransliterateText( nType );
        if (pTopView)
            pTopView->TransliterateText( nType );

        pHdl->DataChanged();
    }
}

