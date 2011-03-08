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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_editeng.hxx"

#include <vcl/wrkwin.hxx>
#include <vcl/dialog.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>

#include <editeng/lspcitem.hxx>
#include <editeng/flditem.hxx>
#include <impedit.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <editdbg.hxx>
#include <eerdll2.hxx>
#include <editeng/eerdll.hxx>
#include <edtspell.hxx>
#include <eeobj.hxx>
#include <editeng/txtrange.hxx>
#include <svl/urlbmk.hxx>
#include <svtools/colorcfg.hxx>
#include <svl/ctloptions.hxx>
#include <editeng/acorrcfg.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/adjitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/justifyitem.hxx>
#include <vcl/cmdevt.h>

#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/text/CharacterCompressionType.hpp>
#include <com/sun/star/i18n/InputSequenceCheckMode.hpp>

#include <comphelper/processfactory.hxx>

#include <sot/formats.hxx>

#include <unicode/ubidi.h>

using namespace ::com::sun::star;

USHORT lcl_CalcExtraSpace( ParaPortion*, const SvxLineSpacingItem& rLSItem )
{
    USHORT nExtra = 0;
    if ( rLSItem.GetInterLineSpaceRule() == SVX_INTER_LINE_SPACE_FIX )
    {
        nExtra = rLSItem.GetInterLineSpace();
    }

    return nExtra;
}

ImpEditEngine::ImpEditEngine( EditEngine* pEE, SfxItemPool* pItemPool ) :
    aPaperSize( 0x7FFFFFFF, 0x7FFFFFFF ),
    aMinAutoPaperSize( 0x0, 0x0 ),
    aMaxAutoPaperSize( 0x7FFFFFFF, 0x7FFFFFFF ),
    aEditDoc( pItemPool ),
    aWordDelimiters( RTL_CONSTASCII_USTRINGPARAM( "  .,;:-'`'?!_=\"{}()[]\0xFF" ) ),
    aGroupChars( RTL_CONSTASCII_USTRINGPARAM( "{}()[]" ) )
{
    pEditEngine         = pEE;
    pRefDev             = NULL;
    pVirtDev            = NULL;
    pEmptyItemSet       = NULL;
    pActiveView         = NULL;
    pSpellInfo          = NULL;
    pConvInfo           = NULL;
    pTextObjectPool     = NULL;
    mpIMEInfos          = NULL;
    pStylePool          = NULL;
    pUndoManager        = NULL;
    pUndoMarkSelection  = NULL;
    pTextRanger         = NULL;
    pColorConfig        = NULL;
    pCTLOptions         = NULL;

    nCurTextHeight      = 0;
    nBlockNotifications = 0;
    nBigTextObjectStart = 20;

    nStretchX           = 100;
    nStretchY           = 100;

    bInSelection        = FALSE;
    bOwnerOfRefDev      = FALSE;
    bDowning            = FALSE;
    bIsInUndo           = FALSE;
    bIsFormatting       = FALSE;
    bFormatted          = FALSE;
    bUpdate             = TRUE;
    bUseAutoColor       = TRUE;
    bForceAutoColor     = FALSE;
    bAddExtLeading      = FALSE;
    bUndoEnabled        = TRUE;
    bCallParaInsertedOrDeleted = FALSE;
    bImpConvertFirstCall= FALSE;
    bFirstWordCapitalization    = TRUE;

    eDefLanguage        = LANGUAGE_DONTKNOW;
    maBackgroundColor   = COL_AUTO;

    nAsianCompressionMode = text::CharacterCompressionType::NONE;
    bKernAsianPunctuation = FALSE;

    eDefaultHorizontalTextDirection = EE_HTEXTDIR_DEFAULT;


    aStatus.GetControlWord() =  EE_CNTRL_USECHARATTRIBS | EE_CNTRL_DOIDLEFORMAT |
                                EE_CNTRL_PASTESPECIAL | EE_CNTRL_UNDOATTRIBS |
                                EE_CNTRL_ALLOWBIGOBJS | EE_CNTRL_RTFSTYLESHEETS |
                                EE_CNTRL_FORMAT100;

    aSelEngine.SetFunctionSet( &aSelFuncSet );

    aStatusTimer.SetTimeout( 200 );
    aStatusTimer.SetTimeoutHdl( LINK( this, ImpEditEngine, StatusTimerHdl ) );

    aIdleFormatter.SetTimeout( 5 );
    aIdleFormatter.SetTimeoutHdl( LINK( this, ImpEditEngine, IdleFormatHdl ) );

    aOnlineSpellTimer.SetTimeout( 100 );
    aOnlineSpellTimer.SetTimeoutHdl( LINK( this, ImpEditEngine, OnlineSpellHdl ) );

    pRefDev             = EE_DLL()->GetGlobalData()->GetStdRefDevice();

    // Access data already from here on!
    SetRefDevice( pRefDev );
    InitDoc( FALSE );

    bCallParaInsertedOrDeleted = TRUE;

    aEditDoc.SetModifyHdl( LINK( this, ImpEditEngine, DocModified ) );

    mbLastTryMerge = FALSE;
}

ImpEditEngine::~ImpEditEngine()
{
    aStatusTimer.Stop();
    aOnlineSpellTimer.Stop();
    aIdleFormatter.Stop();

    // Destroying templates may otherwise cause unnecessary formatting,
    // when a parent template is destroyed.
    // And this after the destruction of the data!
    bDowning = TRUE;
    SetUpdateMode( FALSE );

    delete pVirtDev;
    delete pEmptyItemSet;
    delete pUndoManager;
    delete pTextRanger;
    delete mpIMEInfos;
    delete pColorConfig;
    delete pCTLOptions;
    if ( bOwnerOfRefDev )
        delete pRefDev;
    delete pSpellInfo;
}

void ImpEditEngine::SetRefDevice( OutputDevice* pRef )
{
    if ( bOwnerOfRefDev )
        delete pRefDev;

    pRefDev = pRef;
    bOwnerOfRefDev = FALSE;

    if ( !pRef )
        pRefDev = EE_DLL()->GetGlobalData()->GetStdRefDevice();

    nOnePixelInRef = (USHORT)pRefDev->PixelToLogic( Size( 1, 0 ) ).Width();

    if ( IsFormatted() )
    {
        FormatFullDoc();
        UpdateViews( (EditView*) 0);
    }
}

void ImpEditEngine::SetRefMapMode( const MapMode& rMapMode )
{
    if ( GetRefDevice()->GetMapMode() == rMapMode )
        return;

    // When RefDev == GlobalRefDev => create own!
    if ( !bOwnerOfRefDev && ( pRefDev == EE_DLL()->GetGlobalData()->GetStdRefDevice() ) )
    {
        pRefDev = new VirtualDevice;
        pRefDev->SetMapMode( MAP_TWIP );
        SetRefDevice( pRefDev );
        bOwnerOfRefDev = TRUE;
    }
    pRefDev->SetMapMode( rMapMode );
    nOnePixelInRef = (USHORT)pRefDev->PixelToLogic( Size( 1, 0 ) ).Width();
    if ( IsFormatted() )
    {
        FormatFullDoc();
        UpdateViews( (EditView*) 0);
    }
}

void ImpEditEngine::InitDoc( BOOL bKeepParaAttribs )
{
    USHORT nParas = aEditDoc.Count();
    for ( USHORT n = bKeepParaAttribs ? 1 : 0; n < nParas; n++ )
    {
        if ( aEditDoc[n]->GetStyleSheet() )
            EndListening( *aEditDoc[n]->GetStyleSheet(), FALSE );
    }

    if ( bKeepParaAttribs )
        aEditDoc.RemoveText();
    else
        aEditDoc.Clear();

    GetParaPortions().Reset();

    ParaPortion* pIniPortion = new ParaPortion( aEditDoc[0] );
    GetParaPortions().Insert( pIniPortion, 0 );

    bFormatted = FALSE;

    if ( IsCallParaInsertedOrDeleted() )
    {
        GetEditEnginePtr()->ParagraphDeleted( EE_PARA_ALL );
        GetEditEnginePtr()->ParagraphInserted( 0 );
    }

    if ( GetStatus().DoOnlineSpelling() )
        aEditDoc.GetObject( 0 )->CreateWrongList();
}

EditPaM ImpEditEngine::DeleteSelected( EditSelection aSel )
{
    EditPaM aPaM ( ImpDeleteSelection( aSel ) );
    return aPaM;
}

XubString ImpEditEngine::GetSelected( const EditSelection& rSel, const LineEnd eEnd  ) const
{
    XubString aText;
    if ( !rSel.HasRange() )
        return aText;

    String aSep = EditDoc::GetSepStr( eEnd );

    EditSelection aSel( rSel );
    aSel.Adjust( aEditDoc );

    ContentNode* pStartNode = aSel.Min().GetNode();
    ContentNode* pEndNode = aSel.Max().GetNode();
    USHORT nStartNode = aEditDoc.GetPos( pStartNode );
    USHORT nEndNode = aEditDoc.GetPos( pEndNode );

    DBG_ASSERT( nStartNode <= nEndNode, "Selection not sorted ?" );

    // iterate over the paragraphs ...
    for ( USHORT nNode = nStartNode; nNode <= nEndNode; nNode++ )
    {
        DBG_ASSERT( aEditDoc.SaveGetObject( nNode ), "Node not found: GetSelected" );
        ContentNode* pNode = aEditDoc.GetObject( nNode );

        xub_StrLen nStartPos = 0;
        xub_StrLen nEndPos = pNode->Len();
        if ( nNode == nStartNode )
            nStartPos = aSel.Min().GetIndex();
        if ( nNode == nEndNode ) // can also be == nStart!
            nEndPos = aSel.Max().GetIndex();

        aText += aEditDoc.GetParaAsString( pNode, nStartPos, nEndPos );
        if ( nNode < nEndNode )
            aText += aSep;
    }
    return aText;
}

BOOL ImpEditEngine::MouseButtonDown( const MouseEvent& rMEvt, EditView* pView )
{
    GetSelEngine().SetCurView( pView );
    SetActiveView( pView );

    if ( GetAutoCompleteText().Len() )
        SetAutoCompleteText( String(), TRUE );

    GetSelEngine().SelMouseButtonDown( rMEvt );
    // Special treatment
    EditSelection aCurSel( pView->pImpEditView->GetEditSelection() );
    if ( !rMEvt.IsShift() )
    {
        if ( rMEvt.GetClicks() == 2 )
        {
            // So that the SelectionEngine knows about the anchor.
            aSelEngine.CursorPosChanging( TRUE, FALSE );

            EditSelection aNewSelection( SelectWord( aCurSel ) );
            pView->pImpEditView->DrawSelection();
            pView->pImpEditView->SetEditSelection( aNewSelection );
            pView->pImpEditView->DrawSelection();
            pView->ShowCursor( TRUE, TRUE );
        }
        else if ( rMEvt.GetClicks() == 3 )
        {
            // So that the SelectionEngine knows about the anchor.
            aSelEngine.CursorPosChanging( TRUE, FALSE );

            EditSelection aNewSelection( aCurSel );
            aNewSelection.Min().SetIndex( 0 );
            aNewSelection.Max().SetIndex( aCurSel.Min().GetNode()->Len() );
            pView->pImpEditView->DrawSelection();
            pView->pImpEditView->SetEditSelection( aNewSelection );
            pView->pImpEditView->DrawSelection();
            pView->ShowCursor( TRUE, TRUE );
        }
    }
    return TRUE;
}

void ImpEditEngine::Command( const CommandEvent& rCEvt, EditView* pView )
{
    GetSelEngine().SetCurView( pView );
    SetActiveView( pView );
    if ( rCEvt.GetCommand() == COMMAND_VOICE )
    {
        const CommandVoiceData* pData = rCEvt.GetVoiceData();
        if ( pData->GetType() == VOICECOMMANDTYPE_DICTATION )
        {
            // Turn functions into KeyEvent if no corresponding method to
            // EditView/EditEngine so that Undo remains consistent.
            SfxPoolItem* pNewAttr = NULL;

            switch ( pData->GetCommand() )
            {
                case DICTATIONCOMMAND_UNKNOWN:
                {
                    pView->InsertText( pData->GetText() );
                }
                break;
                case DICTATIONCOMMAND_NEWPARAGRAPH:
                {
                    pView->PostKeyEvent( KeyEvent( 0, KeyCode( KEY_RETURN, 0 ) ) );
                }
                break;
                case DICTATIONCOMMAND_NEWLINE:
                {
                    pView->PostKeyEvent( KeyEvent( 0, KeyCode( KEY_RETURN, KEY_SHIFT ) ) );
                }
                break;
                case DICTATIONCOMMAND_TAB:
                {
                    pView->PostKeyEvent( KeyEvent( 0, KeyCode( KEY_TAB, 0 ) ) );
                }
                break;
                case DICTATIONCOMMAND_LEFT:
                {
                    pView->PostKeyEvent( KeyEvent( 0, KeyCode( KEY_LEFT, KEY_MOD1  ) ) );
                }
                break;
                case DICTATIONCOMMAND_RIGHT:
                {
                    pView->PostKeyEvent( KeyEvent( 0, KeyCode( KEY_RIGHT, KEY_MOD1  ) ) );
                }
                break;
                case DICTATIONCOMMAND_UP:
                {
                    pView->PostKeyEvent( KeyEvent( 0, KeyCode( KEY_UP, 0 ) ) );
                }
                break;
                case DICTATIONCOMMAND_DOWN:
                {
                    pView->PostKeyEvent( KeyEvent( 0, KeyCode( KEY_UP, 0 ) ) );
                }
                break;
                case DICTATIONCOMMAND_UNDO:
                {
                    pView->Undo();
                }
                break;
                case DICTATIONCOMMAND_DEL:
                {
                    pView->PostKeyEvent( KeyEvent( 0, KeyCode( KEY_LEFT, KEY_MOD1|KEY_SHIFT  ) ) );
                    pView->DeleteSelected();
                }
                break;
                case DICTATIONCOMMAND_BOLD_ON:
                {
                    pNewAttr = new SvxWeightItem( WEIGHT_BOLD, EE_CHAR_WEIGHT );
                }
                break;
                case DICTATIONCOMMAND_BOLD_OFF:
                {
                    pNewAttr = new SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT );
                }
                break;
                case DICTATIONCOMMAND_ITALIC_ON:
                {
                    pNewAttr = new SvxPostureItem( ITALIC_NORMAL, EE_CHAR_ITALIC );
                }
                break;
                case DICTATIONCOMMAND_ITALIC_OFF:
                {
                    pNewAttr = new SvxPostureItem( ITALIC_NORMAL, EE_CHAR_ITALIC );
                }
                break;
                case DICTATIONCOMMAND_UNDERLINE_ON:
                {
                    pNewAttr = new SvxUnderlineItem( UNDERLINE_SINGLE, EE_CHAR_UNDERLINE );
                }
                break;
                case DICTATIONCOMMAND_UNDERLINE_OFF:
                {
                    pNewAttr = new SvxUnderlineItem( UNDERLINE_NONE, EE_CHAR_UNDERLINE );
                }
                break;
            }

            if ( pNewAttr )
            {
                SfxItemSet aSet( GetEmptyItemSet() );
                aSet.Put( *pNewAttr );
                pView->SetAttribs( aSet );
                delete pNewAttr;
            }
        }
    }
    else if ( rCEvt.GetCommand() == COMMAND_STARTEXTTEXTINPUT )
    {
        pView->DeleteSelected();
        delete mpIMEInfos;
        EditPaM aPaM = pView->GetImpEditView()->GetEditSelection().Max();
        String aOldTextAfterStartPos = aPaM.GetNode()->Copy( aPaM.GetIndex() );
        USHORT nMax = aOldTextAfterStartPos.Search( CH_FEATURE );
        if ( nMax != STRING_NOTFOUND )  // don't overwrite features!
            aOldTextAfterStartPos.Erase( nMax );
        mpIMEInfos = new ImplIMEInfos( aPaM, aOldTextAfterStartPos );
        mpIMEInfos->bWasCursorOverwrite = !pView->IsInsertMode();
        UndoActionStart( EDITUNDO_INSERT );
    }
    else if ( rCEvt.GetCommand() == COMMAND_ENDEXTTEXTINPUT )
    {
        DBG_ASSERT( mpIMEInfos, "COMMAND_ENDEXTTEXTINPUT => Kein Start ?" );
        if( mpIMEInfos )
        {
            // #102812# convert quotes in IME text
            // works on the last input character, this is escpecially in Korean text often done
            // quotes that are inside of the string are not replaced!
            // Borrowed from sw: edtwin.cxx
            if ( mpIMEInfos->nLen )
            {
                EditSelection aSel( mpIMEInfos->aPos );
                aSel.Min().GetIndex() += mpIMEInfos->nLen-1;
                aSel.Max().GetIndex() =
                    aSel.Max().GetIndex() + mpIMEInfos->nLen;
                // #102812# convert quotes in IME text
                // works on the last input character, this is escpecially in Korean text often done
                // quotes that are inside of the string are not replaced!
                const sal_Unicode nCharCode = aSel.Min().GetNode()->GetChar( aSel.Min().GetIndex() );
                if ( ( GetStatus().DoAutoCorrect() ) && ( ( nCharCode == '\"' ) || ( nCharCode == '\'' ) ) )
                {
                    aSel = DeleteSelected( aSel );
                    aSel = AutoCorrect( aSel, nCharCode, mpIMEInfos->bWasCursorOverwrite );
                    pView->pImpEditView->SetEditSelection( aSel );
                }
            }

            ParaPortion* pPortion = FindParaPortion( mpIMEInfos->aPos.GetNode() );
            pPortion->MarkSelectionInvalid( mpIMEInfos->aPos.GetIndex(), 0 );

            BOOL bWasCursorOverwrite = mpIMEInfos->bWasCursorOverwrite;

            delete mpIMEInfos;
            mpIMEInfos = NULL;

            FormatAndUpdate( pView );

            pView->SetInsertMode( !bWasCursorOverwrite );
        }
        UndoActionEnd( EDITUNDO_INSERT );
    }
    else if ( rCEvt.GetCommand() == COMMAND_EXTTEXTINPUT )
    {
        DBG_ASSERT( mpIMEInfos, "COMMAND_EXTTEXTINPUT => No Start ?" );
        if( mpIMEInfos )
        {
            const CommandExtTextInputData* pData = rCEvt.GetExtTextInputData();

            if ( !pData->IsOnlyCursorChanged() )
            {
                EditSelection aSel( mpIMEInfos->aPos );
                aSel.Max().GetIndex() =
                    aSel.Max().GetIndex() + mpIMEInfos->nLen;
                aSel = DeleteSelected( aSel );
                aSel = ImpInsertText( aSel, pData->GetText() );

                if ( mpIMEInfos->bWasCursorOverwrite )
                {
                    USHORT nOldIMETextLen = mpIMEInfos->nLen;
                    USHORT nNewIMETextLen = pData->GetText().Len();

                    if ( ( nOldIMETextLen > nNewIMETextLen ) &&
                         ( nNewIMETextLen < mpIMEInfos->aOldTextAfterStartPos.Len() ) )
                    {
                        // restore old characters
                        USHORT nRestore = nOldIMETextLen - nNewIMETextLen;
                        EditPaM aPaM( mpIMEInfos->aPos );
                        aPaM.GetIndex() = aPaM.GetIndex() + nNewIMETextLen;
                        ImpInsertText( aPaM, mpIMEInfos->aOldTextAfterStartPos.Copy( nNewIMETextLen, nRestore ) );
                    }
                    else if ( ( nOldIMETextLen < nNewIMETextLen ) &&
                              ( nOldIMETextLen < mpIMEInfos->aOldTextAfterStartPos.Len() ) )
                    {
                        // overwrite
                        USHORT nOverwrite = nNewIMETextLen - nOldIMETextLen;
                        if ( ( nOldIMETextLen + nOverwrite ) > mpIMEInfos->aOldTextAfterStartPos.Len() )
                            nOverwrite = mpIMEInfos->aOldTextAfterStartPos.Len() - nOldIMETextLen;
                        DBG_ASSERT( nOverwrite && (nOverwrite < 0xFF00), "IME Overwrite?!" );
                        EditPaM aPaM( mpIMEInfos->aPos );
                        aPaM.GetIndex() = aPaM.GetIndex() + nNewIMETextLen;
                        EditSelection _aSel( aPaM );
                        _aSel.Max().GetIndex() =
                            _aSel.Max().GetIndex() + nOverwrite;
                        DeleteSelected( _aSel );
                    }
                }
                if ( pData->GetTextAttr() )
                {
                    mpIMEInfos->CopyAttribs( pData->GetTextAttr(), pData->GetText().Len() );
                    mpIMEInfos->bCursor = pData->IsCursorVisible();
                }
                else
                {
                    mpIMEInfos->DestroyAttribs();
                    mpIMEInfos->nLen = pData->GetText().Len();
                }

                ParaPortion* pPortion = FindParaPortion( mpIMEInfos->aPos.GetNode() );
                pPortion->MarkSelectionInvalid( mpIMEInfos->aPos.GetIndex(), 0 );
                FormatAndUpdate( pView );
            }

            EditSelection aNewSel = EditPaM( mpIMEInfos->aPos.GetNode(), mpIMEInfos->aPos.GetIndex()+pData->GetCursorPos() );
            pView->SetSelection( CreateESel( aNewSel ) );
            pView->SetInsertMode( !pData->IsCursorOverwrite() );

            if ( pData->IsCursorVisible() )
                pView->ShowCursor();
            else
                pView->HideCursor();
        }
    }
    else if ( rCEvt.GetCommand() == COMMAND_INPUTCONTEXTCHANGE )
    {
    }
    else if ( rCEvt.GetCommand() == COMMAND_CURSORPOS )
    {
        if ( mpIMEInfos && mpIMEInfos->nLen )
        {
            EditPaM aPaM( pView->pImpEditView->GetEditSelection().Max() );
            Rectangle aR1 = PaMtoEditCursor( aPaM, 0 );

            USHORT nInputEnd = mpIMEInfos->aPos.GetIndex() + mpIMEInfos->nLen;

            if ( !IsFormatted() )
                FormatDoc();

            ParaPortion* pParaPortion = GetParaPortions().SaveGetObject( GetEditDoc().GetPos( aPaM.GetNode() ) );
            USHORT nLine = pParaPortion->GetLines().FindLine( aPaM.GetIndex(), sal_True );
            EditLine* pLine = pParaPortion->GetLines().GetObject( nLine );
            if ( pLine && ( nInputEnd > pLine->GetEnd() ) )
                nInputEnd = pLine->GetEnd();
            Rectangle aR2 = PaMtoEditCursor( EditPaM( aPaM.GetNode(), nInputEnd ), GETCRSR_ENDOFLINE );
            Rectangle aRect = pView->GetImpEditView()->GetWindowPos( aR1 );
            pView->GetWindow()->SetCursorRect( &aRect, aR2.Left()-aR1.Right() );
        }
        else
        {
            pView->GetWindow()->SetCursorRect();
        }
    }
    else if ( rCEvt.GetCommand() == COMMAND_SELECTIONCHANGE )
    {
        const CommandSelectionChangeData *pData = rCEvt.GetSelectionChangeData();

        ESelection aSelection = pView->GetSelection();
        aSelection.Adjust();

        if( pView->HasSelection() )
        {
            aSelection.nEndPos = aSelection.nStartPos;
            aSelection.nStartPos += pData->GetStart();
            aSelection.nEndPos += pData->GetEnd();
        }
        else
        {
            aSelection.nStartPos = pData->GetStart();
            aSelection.nEndPos = pData->GetEnd();
        }
        pView->SetSelection( aSelection );
    }
    else if ( rCEvt.GetCommand() == COMMAND_PREPARERECONVERSION )
    {
        if ( pView->HasSelection() )
        {
            ESelection aSelection = pView->GetSelection();
            aSelection.Adjust();

            if ( aSelection.nStartPara != aSelection.nEndPara )
            {
                xub_StrLen aParaLen = pEditEngine->GetTextLen( aSelection.nStartPara );
                aSelection.nEndPara = aSelection.nStartPara;
                aSelection.nEndPos = aParaLen;
                pView->SetSelection( aSelection );
            }
        }
    }

    GetSelEngine().Command( rCEvt );
}

BOOL ImpEditEngine::MouseButtonUp( const MouseEvent& rMEvt, EditView* pView )
{
    GetSelEngine().SetCurView( pView );
    GetSelEngine().SelMouseButtonUp( rMEvt );
    bInSelection = FALSE;
    // Special treatments
    EditSelection aCurSel( pView->pImpEditView->GetEditSelection() );
    if ( !aCurSel.HasRange() )
    {
        if ( ( rMEvt.GetClicks() == 1 ) && rMEvt.IsLeft() && !rMEvt.IsMod2() )
        {
            const SvxFieldItem* pFld = pView->GetFieldUnderMousePointer();
            if ( pFld )
            {
                EditPaM aPaM( aCurSel.Max() );
                USHORT nPara = GetEditDoc().GetPos( aPaM.GetNode() );
                GetEditEnginePtr()->FieldClicked( *pFld, nPara, aPaM.GetIndex() );
            }
        }
    }
    return TRUE;
}

BOOL ImpEditEngine::MouseMove( const MouseEvent& rMEvt, EditView* pView )
{
    // MouseMove is called directly after ShowQuickHelp()!
    GetSelEngine().SetCurView( pView );
    GetSelEngine().SelMouseMove( rMEvt );
    return TRUE;
}

EditPaM ImpEditEngine::InsertText( EditSelection aSel, const XubString& rStr )
{
    EditPaM aPaM = ImpInsertText( aSel, rStr );
    return aPaM;
}

EditPaM ImpEditEngine::Clear()
{
    InitDoc( FALSE );

    EditPaM aPaM = aEditDoc.GetStartPaM();
    EditSelection aSel( aPaM );

    nCurTextHeight = 0;

    ResetUndoManager();

    for ( USHORT nView = aEditViews.Count(); nView; )
    {
        EditView* pView = aEditViews[--nView];
        DBG_CHKOBJ( pView, EditView, 0 );
        pView->pImpEditView->SetEditSelection( aSel );
    }

    return aPaM;
}

EditPaM ImpEditEngine::RemoveText()
{
    InitDoc( TRUE );

    EditPaM aStartPaM = aEditDoc.GetStartPaM();
    EditSelection aEmptySel( aStartPaM, aStartPaM );
    for ( USHORT nView = 0; nView < aEditViews.Count(); nView++ )
    {
        EditView* pView = aEditViews.GetObject(nView);
        DBG_CHKOBJ( pView, EditView, 0 );
        pView->pImpEditView->SetEditSelection( aEmptySel );
    }
    ResetUndoManager();
    return aEditDoc.GetStartPaM();
}


void ImpEditEngine::SetText( const XubString& rText )
{
    // RemoveText deletes the undo list!
    EditPaM aStartPaM = RemoveText();
    BOOL bUndoCurrentlyEnabled = IsUndoEnabled();
    // The text inserted manually can not be made reversable by the user
    EnableUndo( FALSE );

    EditSelection aEmptySel( aStartPaM, aStartPaM );
    EditPaM aPaM = aStartPaM;
    if ( rText.Len() )
        aPaM = ImpInsertText( aEmptySel, rText );

    for ( USHORT nView = 0; nView < aEditViews.Count(); nView++ )
    {
        EditView* pView = aEditViews[nView];
        DBG_CHKOBJ( pView, EditView, 0 );
        pView->pImpEditView->SetEditSelection( EditSelection( aPaM, aPaM ) );
        //  If no text then also no Format&Update
        // => The text remains.
        if ( !rText.Len() && GetUpdateMode() )
        {
            Rectangle aTmpRec( pView->GetOutputArea().TopLeft(),
                                Size( aPaperSize.Width(), nCurTextHeight ) );
            aTmpRec.Intersection( pView->GetOutputArea() );
            pView->GetWindow()->Invalidate( aTmpRec );
        }
    }
    if( !rText.Len() )  // otherwise it must be invalidated later, !bFormatted is enough.
        nCurTextHeight = 0;
    EnableUndo( bUndoCurrentlyEnabled );
    DBG_ASSERT( !HasUndoManager() || !GetUndoManager().GetUndoActionCount(), "Undo after SetText?" );
}


const SfxItemSet& ImpEditEngine::GetEmptyItemSet()
{
    if ( !pEmptyItemSet )
    {
        pEmptyItemSet = new SfxItemSet( aEditDoc.GetItemPool(), EE_ITEMS_START, EE_ITEMS_END );
        for ( USHORT nWhich = EE_ITEMS_START; nWhich <= EE_CHAR_END; nWhich++)
        {
            pEmptyItemSet->ClearItem( nWhich );
        }
    }
    return *pEmptyItemSet;
}

//  ----------------------------------------------------------------------
//  MISC
//  ----------------------------------------------------------------------
void ImpEditEngine::CursorMoved( ContentNode* pPrevNode )
{
    // Delete empty attributes, but only if paragraph is not empty!
    if ( pPrevNode->GetCharAttribs().HasEmptyAttribs() && pPrevNode->Len() )
        pPrevNode->GetCharAttribs().DeleteEmptyAttribs( aEditDoc.GetItemPool() );
}

void ImpEditEngine::TextModified()
{
    bFormatted = FALSE;

    if ( GetNotifyHdl().IsSet() )
    {
        EENotify aNotify( EE_NOTIFY_TEXTMODIFIED );
        aNotify.pEditEngine = GetEditEnginePtr();
        CallNotify( aNotify );
    }
}


void ImpEditEngine::ParaAttribsChanged( ContentNode* pNode )
{
    DBG_ASSERT( pNode, "ParaAttribsChanged: Which one?" );

    aEditDoc.SetModified( TRUE );
    bFormatted = FALSE;

    ParaPortion* pPortion = FindParaPortion( pNode );
    DBG_ASSERT( pPortion, "ParaAttribsChanged: Portion?" );
    pPortion->MarkSelectionInvalid( 0, pNode->Len() );

    USHORT nPara = aEditDoc.GetPos( pNode );
    pEditEngine->ParaAttribsChanged( nPara );

    ParaPortion* pNextPortion = GetParaPortions().SaveGetObject( nPara+1 );
    // => is formatted again anyway, if Invalid.
    if ( pNextPortion && !pNextPortion->IsInvalid() )
        CalcHeight( pNextPortion );
}

//  ----------------------------------------------------------------------
//  Cursor movements
//  ----------------------------------------------------------------------

EditSelection ImpEditEngine::MoveCursor( const KeyEvent& rKeyEvent, EditView* pEditView )
{
    // Actually, only necessary for up/down, but whatever.
    CheckIdleFormatter();

    EditPaM aPaM( pEditView->pImpEditView->GetEditSelection().Max() );

    EditPaM aOldPaM( aPaM );

    TextDirectionality eTextDirection = TextDirectionality_LeftToRight_TopToBottom;
    if ( IsVertical() )
        eTextDirection = TextDirectionality_TopToBottom_RightToLeft;
    else if ( IsRightToLeft( GetEditDoc().GetPos( aPaM.GetNode() ) ) )
        eTextDirection = TextDirectionality_RightToLeft_TopToBottom;

    KeyEvent aTranslatedKeyEvent = rKeyEvent.LogicalTextDirectionality( eTextDirection );

    BOOL bCtrl = aTranslatedKeyEvent.GetKeyCode().IsMod1() ? TRUE : FALSE;
    USHORT nCode = aTranslatedKeyEvent.GetKeyCode().GetCode();

    if ( DoVisualCursorTraveling( aPaM.GetNode() ) )
    {
        // Only for simple cursor movement...
        if ( !bCtrl && ( ( nCode == KEY_LEFT ) || ( nCode == KEY_RIGHT ) ) )
        {
            aPaM = CursorVisualLeftRight( pEditView, aPaM, rKeyEvent.GetKeyCode().IsMod2() ? i18n::CharacterIteratorMode::SKIPCHARACTER : i18n::CharacterIteratorMode::SKIPCELL, rKeyEvent.GetKeyCode().GetCode() == KEY_LEFT );
            nCode = 0;  // skip switch statement
        }
    }

    bool bKeyModifySelection = aTranslatedKeyEvent.GetKeyCode().IsShift();
    switch ( nCode )
    {
        case KEY_UP:        aPaM = CursorUp( aPaM, pEditView );
                            break;
        case KEY_DOWN:      aPaM = CursorDown( aPaM, pEditView );
                            break;
        case KEY_LEFT:      aPaM = bCtrl ? WordLeft( aPaM ) : CursorLeft( aPaM, aTranslatedKeyEvent.GetKeyCode().IsMod2() ? i18n::CharacterIteratorMode::SKIPCHARACTER : i18n::CharacterIteratorMode::SKIPCELL );
                            break;
        case KEY_RIGHT:     aPaM = bCtrl ? WordRight( aPaM ) : CursorRight( aPaM, aTranslatedKeyEvent.GetKeyCode().IsMod2() ? i18n::CharacterIteratorMode::SKIPCHARACTER : i18n::CharacterIteratorMode::SKIPCELL );
                            break;
        case KEY_HOME:      aPaM = bCtrl ? CursorStartOfDoc() : CursorStartOfLine( aPaM );
                            break;
        case KEY_END:       aPaM = bCtrl ? CursorEndOfDoc() : CursorEndOfLine( aPaM );
                            break;
        case KEY_PAGEUP:    aPaM = bCtrl ? CursorStartOfDoc() : PageUp( aPaM, pEditView );
                            break;
        case KEY_PAGEDOWN:  aPaM = bCtrl ? CursorEndOfDoc() : PageDown( aPaM, pEditView );
                            break;
        case com::sun::star::awt::Key::MOVE_TO_BEGIN_OF_LINE:
                            aPaM = CursorStartOfLine( aPaM );
                            bKeyModifySelection = false;
                            break;
        case com::sun::star::awt::Key::MOVE_TO_END_OF_LINE:
                            aPaM = CursorEndOfLine( aPaM );
                            bKeyModifySelection = false;
                            break;
        case com::sun::star::awt::Key::MOVE_WORD_BACKWARD:
                            aPaM = WordLeft( aPaM );
                            bKeyModifySelection = false;
                            break;
        case com::sun::star::awt::Key::MOVE_WORD_FORWARD:
                            aPaM = WordRight( aPaM );
                            bKeyModifySelection = false;
                            break;
        case com::sun::star::awt::Key::MOVE_TO_BEGIN_OF_PARAGRAPH:
                            aPaM = CursorStartOfParagraph( aPaM );
                            if( aPaM == aOldPaM )
                            {
                                aPaM = CursorLeft( aPaM, i18n::CharacterIteratorMode::SKIPCELL );
                                aPaM = CursorStartOfParagraph( aPaM );
                            }
                            bKeyModifySelection = false;
                            break;
        case com::sun::star::awt::Key::MOVE_TO_END_OF_PARAGRAPH:
                            aPaM = CursorEndOfParagraph( aPaM );
                            if( aPaM == aOldPaM )
                            {
                                aPaM = CursorRight( aPaM, i18n::CharacterIteratorMode::SKIPCELL );
                                aPaM = CursorEndOfParagraph( aPaM );
                            }
                            bKeyModifySelection = false;
                            break;
        case com::sun::star::awt::Key::MOVE_TO_BEGIN_OF_DOCUMENT:
                            aPaM = CursorStartOfDoc();
                            bKeyModifySelection = false;
                            break;
        case com::sun::star::awt::Key::MOVE_TO_END_OF_DOCUMENT:
                            aPaM = CursorEndOfDoc();
                            bKeyModifySelection = false;
                            break;
        case com::sun::star::awt::Key::SELECT_TO_BEGIN_OF_LINE:
                            aPaM = CursorStartOfLine( aPaM );
                            bKeyModifySelection = true;
                            break;
        case com::sun::star::awt::Key::SELECT_TO_END_OF_LINE:
                            aPaM = CursorEndOfLine( aPaM );
                            bKeyModifySelection = true;
                            break;
        case com::sun::star::awt::Key::SELECT_BACKWARD:
                            aPaM = CursorLeft( aPaM, i18n::CharacterIteratorMode::SKIPCELL );
                            bKeyModifySelection = true;
                            break;
        case com::sun::star::awt::Key::SELECT_FORWARD:
                            aPaM = CursorRight( aPaM, i18n::CharacterIteratorMode::SKIPCELL );
                            bKeyModifySelection = true;
                            break;
        case com::sun::star::awt::Key::SELECT_WORD_BACKWARD:
                            aPaM = WordLeft( aPaM );
                            bKeyModifySelection = true;
                            break;
        case com::sun::star::awt::Key::SELECT_WORD_FORWARD:
                            aPaM = WordRight( aPaM );
                            bKeyModifySelection = true;
                            break;
        case com::sun::star::awt::Key::SELECT_TO_BEGIN_OF_PARAGRAPH:
                            aPaM = CursorStartOfParagraph( aPaM );
                            if( aPaM == aOldPaM )
                            {
                                aPaM = CursorLeft( aPaM, i18n::CharacterIteratorMode::SKIPCELL );
                                aPaM = CursorStartOfParagraph( aPaM );
                            }
                            bKeyModifySelection = true;
                            break;
        case com::sun::star::awt::Key::SELECT_TO_END_OF_PARAGRAPH:
                            aPaM = CursorEndOfParagraph( aPaM );
                            if( aPaM == aOldPaM )
                            {
                                aPaM = CursorRight( aPaM, i18n::CharacterIteratorMode::SKIPCELL );
                                aPaM = CursorEndOfParagraph( aPaM );
                            }
                            bKeyModifySelection = true;
                            break;
        case com::sun::star::awt::Key::SELECT_TO_BEGIN_OF_DOCUMENT:
                            aPaM = CursorStartOfDoc();
                            bKeyModifySelection = true;
                            break;
        case com::sun::star::awt::Key::SELECT_TO_END_OF_DOCUMENT:
                            aPaM = CursorEndOfDoc();
                            bKeyModifySelection = true;
                            break;
    }

    if ( aOldPaM != aPaM )
    {
        CursorMoved( aOldPaM.GetNode() );
        if ( aStatus.NotifyCursorMovements() && ( aOldPaM.GetNode() != aPaM.GetNode() ) )
        {
            aStatus.GetStatusWord() = aStatus.GetStatusWord() | EE_STAT_CRSRLEFTPARA;
            aStatus.GetPrevParagraph() = aEditDoc.GetPos( aOldPaM.GetNode() );
        }
    }
    else
        aStatus.GetStatusWord() = aStatus.GetStatusWord() | EE_STAT_CRSRMOVEFAIL;

    // May cause, an CreateAnchor or deselection all
    aSelEngine.SetCurView( pEditView );
    aSelEngine.CursorPosChanging( bKeyModifySelection, aTranslatedKeyEvent.GetKeyCode().IsMod1() );
    EditPaM aOldEnd( pEditView->pImpEditView->GetEditSelection().Max() );
    pEditView->pImpEditView->GetEditSelection().Max() = aPaM;
    if ( bKeyModifySelection )
    {
        // Then the selection is expanded ...
        EditSelection aTmpNewSel( aOldEnd, aPaM );
        pEditView->pImpEditView->DrawSelection( aTmpNewSel );
    }
    else
        pEditView->pImpEditView->GetEditSelection().Min() = aPaM;

    return pEditView->pImpEditView->GetEditSelection();
}

EditPaM ImpEditEngine::CursorVisualStartEnd( EditView* pEditView, const EditPaM& rPaM, BOOL bStart )
{
    EditPaM aPaM( rPaM );

    USHORT nPara = GetEditDoc().GetPos( aPaM.GetNode() );
    ParaPortion* pParaPortion = GetParaPortions().SaveGetObject( nPara );

    USHORT nLine = pParaPortion->GetLines().FindLine( aPaM.GetIndex(), sal_False );
    EditLine* pLine = pParaPortion->GetLines().GetObject( nLine );
    BOOL bEmptyLine = pLine->GetStart() == pLine->GetEnd();

    pEditView->pImpEditView->nExtraCursorFlags = 0;

    if ( !bEmptyLine )
    {
        String aLine( *aPaM.GetNode(), pLine->GetStart(), pLine->GetEnd() - pLine->GetStart() );

        const sal_Unicode* pLineString = aLine.GetBuffer();

        UErrorCode nError = U_ZERO_ERROR;
        UBiDi* pBidi = ubidi_openSized( aLine.Len(), 0, &nError );

        const UBiDiLevel  nBidiLevel = IsRightToLeft( nPara ) ? 1 /*RTL*/ : 0 /*LTR*/;
        ubidi_setPara( pBidi, reinterpret_cast<const UChar *>(pLineString), aLine.Len(), nBidiLevel, NULL, &nError );   // UChar != sal_Unicode in MinGW

        USHORT nVisPos = bStart ? 0 : aLine.Len()-1;
        USHORT nLogPos = (USHORT)ubidi_getLogicalIndex( pBidi, nVisPos, &nError );

        ubidi_close( pBidi );

        aPaM.GetIndex() = nLogPos + pLine->GetStart();

        USHORT nTmp;
        USHORT nTextPortion = pParaPortion->GetTextPortions().FindPortion( aPaM.GetIndex(), nTmp, TRUE );
        TextPortion* pTextPortion = pParaPortion->GetTextPortions().GetObject( nTextPortion );
        USHORT nRTLLevel = pTextPortion->GetRightToLeft();

        BOOL bPortionRTL = nRTLLevel%2 ? TRUE : FALSE;

        if ( bStart )
        {
            pEditView->pImpEditView->SetCursorBidiLevel( bPortionRTL ? 0 : 1 );
            // Maybe we must be *behind* the character
            if ( bPortionRTL && pEditView->IsInsertMode() )
                aPaM.GetIndex()++;
        }
        else
        {
            pEditView->pImpEditView->SetCursorBidiLevel( bPortionRTL ? 1 : 0 );
            if ( !bPortionRTL && pEditView->IsInsertMode() )
                aPaM.GetIndex()++;
        }
    }

    return aPaM;
}

EditPaM ImpEditEngine::CursorVisualLeftRight( EditView* pEditView, const EditPaM& rPaM, USHORT nCharacterIteratorMode, BOOL bVisualToLeft )
{
    EditPaM aPaM( rPaM );

    USHORT nPara = GetEditDoc().GetPos( aPaM.GetNode() );
    ParaPortion* pParaPortion = GetParaPortions().SaveGetObject( nPara );

    USHORT nLine = pParaPortion->GetLines().FindLine( aPaM.GetIndex(), sal_False );
    EditLine* pLine = pParaPortion->GetLines().GetObject( nLine );
    BOOL bEmptyLine = pLine->GetStart() == pLine->GetEnd();

    pEditView->pImpEditView->nExtraCursorFlags = 0;

    BOOL bParaRTL = IsRightToLeft( nPara );

    BOOL bDone = FALSE;

    if ( bEmptyLine )
    {
        if ( bVisualToLeft )
        {
            aPaM = CursorUp( aPaM, pEditView );
            if ( aPaM != rPaM )
                aPaM = CursorVisualStartEnd( pEditView, aPaM, FALSE );
        }
        else
        {
            aPaM = CursorDown( aPaM, pEditView );
            if ( aPaM != rPaM )
                aPaM = CursorVisualStartEnd( pEditView, aPaM, TRUE );
        }

        bDone = TRUE;
    }

    BOOL bLogicalBackward = bParaRTL ? !bVisualToLeft : bVisualToLeft;

    if ( !bDone && pEditView->IsInsertMode() )
    {
        // Check if we are within a portion and don't have overwrite mode, then it's easy...
        USHORT nPortionStart;
        USHORT nTextPortion = pParaPortion->GetTextPortions().FindPortion( aPaM.GetIndex(), nPortionStart, FALSE );
        TextPortion* pTextPortion = pParaPortion->GetTextPortions().GetObject( nTextPortion );

        BOOL bPortionBoundary = ( aPaM.GetIndex() == nPortionStart ) || ( aPaM.GetIndex() == (nPortionStart+pTextPortion->GetLen()) );
        USHORT nRTLLevel = pTextPortion->GetRightToLeft();

        // Portion boundary doesn't matter if both have same RTL level
        USHORT nRTLLevelNextPortion = 0xFFFF;
        if ( bPortionBoundary && aPaM.GetIndex() && ( aPaM.GetIndex() < aPaM.GetNode()->Len() ) )
        {
            USHORT nTmp;
            USHORT nNextTextPortion = pParaPortion->GetTextPortions().FindPortion( aPaM.GetIndex()+1, nTmp, bLogicalBackward ? FALSE : TRUE );
            TextPortion* pNextTextPortion = pParaPortion->GetTextPortions().GetObject( nNextTextPortion );
            nRTLLevelNextPortion = pNextTextPortion->GetRightToLeft();
        }

        if ( !bPortionBoundary || ( nRTLLevel == nRTLLevelNextPortion ) )
        {
            if ( ( bVisualToLeft && !(nRTLLevel%2) ) || ( !bVisualToLeft && (nRTLLevel%2) ) )
            {
                aPaM = CursorLeft( aPaM, nCharacterIteratorMode );
                pEditView->pImpEditView->SetCursorBidiLevel( 1 );
            }
            else
            {
                aPaM = CursorRight( aPaM, nCharacterIteratorMode );
                pEditView->pImpEditView->SetCursorBidiLevel( 0 );
            }
            bDone = TRUE;
        }
    }

    if ( !bDone )
    {
        BOOL bGotoStartOfNextLine = FALSE;
        BOOL bGotoEndOfPrevLine = FALSE;

        String aLine( *aPaM.GetNode(), pLine->GetStart(), pLine->GetEnd() - pLine->GetStart() );
        USHORT nPosInLine = aPaM.GetIndex() - pLine->GetStart();

        const sal_Unicode* pLineString = aLine.GetBuffer();

        UErrorCode nError = U_ZERO_ERROR;
        UBiDi* pBidi = ubidi_openSized( aLine.Len(), 0, &nError );

        const UBiDiLevel  nBidiLevel = IsRightToLeft( nPara ) ? 1 /*RTL*/ : 0 /*LTR*/;
        ubidi_setPara( pBidi, reinterpret_cast<const UChar *>(pLineString), aLine.Len(), nBidiLevel, NULL, &nError );   // UChar != sal_Unicode in MinGW

        if ( !pEditView->IsInsertMode() )
        {
            BOOL bEndOfLine = nPosInLine == aLine.Len();
            USHORT nVisPos = (USHORT)ubidi_getVisualIndex( pBidi, !bEndOfLine ? nPosInLine : nPosInLine-1, &nError );
            if ( bVisualToLeft )
            {
                bGotoEndOfPrevLine = nVisPos == 0;
                if ( !bEndOfLine )
                    nVisPos--;
            }
            else
            {
                bGotoStartOfNextLine = nVisPos == (aLine.Len() - 1);
                if ( !bEndOfLine )
                    nVisPos++;
            }

            if ( !bGotoEndOfPrevLine && !bGotoStartOfNextLine )
            {
                USHORT nLogPos = (USHORT)ubidi_getLogicalIndex( pBidi, nVisPos, &nError );
                aPaM.GetIndex() = pLine->GetStart() + nLogPos;
                pEditView->pImpEditView->SetCursorBidiLevel( 0 );
            }
        }
        else
        {
            BOOL bWasBehind = FALSE;
            BOOL bBeforePortion = !nPosInLine || pEditView->pImpEditView->GetCursorBidiLevel() == 1;
            if ( nPosInLine && ( !bBeforePortion ) ) // before the next portion
                bWasBehind = TRUE;  // step one back, otherwise visual will be unusable when rtl portion follows.

            USHORT nPortionStart;
            USHORT nTextPortion = pParaPortion->GetTextPortions().FindPortion( aPaM.GetIndex(), nPortionStart, bBeforePortion );
            TextPortion* pTextPortion = pParaPortion->GetTextPortions().GetObject( nTextPortion );
            BOOL bRTLPortion = (pTextPortion->GetRightToLeft() % 2) != 0;

            // -1: We are 'behind' the character
            long nVisPos = (long)ubidi_getVisualIndex( pBidi, bWasBehind ? nPosInLine-1 : nPosInLine, &nError );
            if ( bVisualToLeft )
            {
                if ( !bWasBehind || bRTLPortion )
                    nVisPos--;
            }
            else
            {
                if ( bWasBehind || bRTLPortion || bBeforePortion )
                    nVisPos++;
            }

            bGotoEndOfPrevLine = nVisPos < 0;
            bGotoStartOfNextLine = nVisPos >= aLine.Len();

            if ( !bGotoEndOfPrevLine && !bGotoStartOfNextLine )
            {
                USHORT nLogPos = (USHORT)ubidi_getLogicalIndex( pBidi, nVisPos, &nError );

                aPaM.GetIndex() = pLine->GetStart() + nLogPos;

                // RTL portion, stay visually on the left side.
                USHORT _nPortionStart;

                USHORT _nTextPortion = pParaPortion->GetTextPortions().FindPortion( aPaM.GetIndex(), _nPortionStart, TRUE );
                TextPortion* _pTextPortion = pParaPortion->GetTextPortions().GetObject( _nTextPortion );
                if ( bVisualToLeft && !bRTLPortion && ( _pTextPortion->GetRightToLeft() % 2 ) )
                    aPaM.GetIndex()++;
                else if ( !bVisualToLeft && bRTLPortion && ( bWasBehind || !(_pTextPortion->GetRightToLeft() % 2 )) )
                    aPaM.GetIndex()++;

                pEditView->pImpEditView->SetCursorBidiLevel( _nPortionStart );
            }
        }

        ubidi_close( pBidi );

        if ( bGotoEndOfPrevLine )
        {
            aPaM = CursorUp( aPaM, pEditView );
            if ( aPaM != rPaM )
                aPaM = CursorVisualStartEnd( pEditView, aPaM, FALSE );
        }
        else if ( bGotoStartOfNextLine )
        {
            aPaM = CursorDown( aPaM, pEditView );
            if ( aPaM != rPaM )
                aPaM = CursorVisualStartEnd( pEditView, aPaM, TRUE );
        }
    }
    return aPaM;
}


EditPaM ImpEditEngine::CursorLeft( const EditPaM& rPaM, USHORT nCharacterIteratorMode )
{
    EditPaM aCurPaM( rPaM );
    EditPaM aNewPaM( aCurPaM );

    if ( aCurPaM.GetIndex() )
    {
        sal_Int32 nCount = 1;
        uno::Reference < i18n::XBreakIterator > _xBI( ImplGetBreakIterator() );
         aNewPaM.SetIndex( (USHORT)_xBI->previousCharacters( *aNewPaM.GetNode(), aNewPaM.GetIndex(), GetLocale( aNewPaM ), nCharacterIteratorMode, nCount, nCount ) );
    }
    else
    {
        ContentNode* pNode = aCurPaM.GetNode();
        pNode = GetPrevVisNode( pNode );
        if ( pNode )
        {
            aNewPaM.SetNode( pNode );
            aNewPaM.SetIndex( pNode->Len() );
        }
    }

    return aNewPaM;
}

EditPaM ImpEditEngine::CursorRight( const EditPaM& rPaM, USHORT nCharacterIteratorMode )
{
    EditPaM aCurPaM( rPaM );
    EditPaM aNewPaM( aCurPaM );

    if ( aCurPaM.GetIndex() < aCurPaM.GetNode()->Len() )
    {
        uno::Reference < i18n::XBreakIterator > _xBI( ImplGetBreakIterator() );
        sal_Int32 nCount = 1;
        aNewPaM.SetIndex( (USHORT)_xBI->nextCharacters( *aNewPaM.GetNode(), aNewPaM.GetIndex(), GetLocale( aNewPaM ), nCharacterIteratorMode, nCount, nCount ) );
    }
    else
    {
        ContentNode* pNode = aCurPaM.GetNode();
        pNode = GetNextVisNode( pNode );
        if ( pNode )
        {
            aNewPaM.SetNode( pNode );
            aNewPaM.SetIndex( 0 );
        }
    }

    return aNewPaM;
}

EditPaM ImpEditEngine::CursorUp( const EditPaM& rPaM, EditView* pView )
{
    DBG_ASSERT( pView, "No View - No Cursor Movement!" );

    ParaPortion* pPPortion = FindParaPortion( rPaM.GetNode() );
    DBG_ASSERT( pPPortion, "No matching portion found: CursorUp ");
    USHORT nLine = pPPortion->GetLineNumber( rPaM.GetIndex() );
    EditLine* pLine = pPPortion->GetLines().GetObject( nLine );

    long nX;
    if ( pView->pImpEditView->nTravelXPos == TRAVEL_X_DONTKNOW )
    {
        nX = GetXPos( pPPortion, pLine, rPaM.GetIndex() );
        pView->pImpEditView->nTravelXPos = nX+nOnePixelInRef;
    }
    else
        nX = pView->pImpEditView->nTravelXPos;

    EditPaM aNewPaM( rPaM );
    if ( nLine )    // same paragraph
    {
        EditLine* pPrevLine = pPPortion->GetLines().GetObject(nLine-1);
        aNewPaM.SetIndex( GetChar( pPPortion, pPrevLine, nX ) );
        // If a previous automatically wrapped line, and one has to be exactly
        // at the end of this line, the cursor lands on the current line at the
        // beginning. See Problem: Last character of an automatically wrapped
        // Row = cursor
        if ( aNewPaM.GetIndex() && ( aNewPaM.GetIndex() == pLine->GetStart() ) )
            aNewPaM = CursorLeft( aNewPaM );
    }
    else    // previous paragraph
    {
        ParaPortion* pPrevPortion = GetPrevVisPortion( pPPortion );
        if ( pPrevPortion )
        {
            pLine = pPrevPortion->GetLines().GetObject( pPrevPortion->GetLines().Count()-1 );
            DBG_ASSERT( pLine, "Line in front not found: CursorUp" );
            aNewPaM.SetNode( pPrevPortion->GetNode() );
            aNewPaM.SetIndex( GetChar( pPrevPortion, pLine, nX+nOnePixelInRef ) );
        }
    }

    return aNewPaM;
}

EditPaM ImpEditEngine::CursorDown( const EditPaM& rPaM, EditView* pView )
{
    DBG_ASSERT( pView, "No View - No Cursor Movement!" );

    ParaPortion* pPPortion = FindParaPortion( rPaM.GetNode() );
    DBG_ASSERT( pPPortion, "No matching portion found: CursorDown" );
    USHORT nLine = pPPortion->GetLineNumber( rPaM.GetIndex() );

    long nX;
    if ( pView->pImpEditView->nTravelXPos == TRAVEL_X_DONTKNOW )
    {
        EditLine* pLine = pPPortion->GetLines().GetObject(nLine);
        nX = GetXPos( pPPortion, pLine, rPaM.GetIndex() );
        pView->pImpEditView->nTravelXPos = nX+nOnePixelInRef;
    }
    else
        nX = pView->pImpEditView->nTravelXPos;

    EditPaM aNewPaM( rPaM );
    if ( nLine < pPPortion->GetLines().Count()-1 )
    {
        EditLine* pNextLine = pPPortion->GetLines().GetObject(nLine+1);
        aNewPaM.SetIndex( GetChar( pPPortion, pNextLine, nX ) );
        // Special treatment, see CursorUp ...
        if ( ( aNewPaM.GetIndex() == pNextLine->GetEnd() ) && ( aNewPaM.GetIndex() > pNextLine->GetStart() ) && ( aNewPaM.GetIndex() < pPPortion->GetNode()->Len() ) )
            aNewPaM = CursorLeft( aNewPaM );
    }
    else    // next paragraph
    {
        ParaPortion* pNextPortion = GetNextVisPortion( pPPortion );
        if ( pNextPortion )
        {
            EditLine* pLine = pNextPortion->GetLines().GetObject(0);
            DBG_ASSERT( pLine, "Line in front not found: CursorUp" );
            aNewPaM.SetNode( pNextPortion->GetNode() );
            // Never at the very end when several lines, because then a line
            // below the cursor appears.
            aNewPaM.SetIndex( GetChar( pNextPortion, pLine, nX+nOnePixelInRef ) );
            if ( ( aNewPaM.GetIndex() == pLine->GetEnd() ) && ( aNewPaM.GetIndex() > pLine->GetStart() ) && ( pNextPortion->GetLines().Count() > 1 ) )
                aNewPaM = CursorLeft( aNewPaM );
        }
    }

    return aNewPaM;
}

EditPaM ImpEditEngine::CursorStartOfLine( const EditPaM& rPaM )
{
    ParaPortion* pCurPortion = FindParaPortion( rPaM.GetNode() );
    DBG_ASSERT( pCurPortion, "No Portion for the PaM ?" );
    USHORT nLine = pCurPortion->GetLineNumber( rPaM.GetIndex() );
    EditLine* pLine = pCurPortion->GetLines().GetObject(nLine);
    DBG_ASSERT( pLine, "Current line not found ?!" );

    EditPaM aNewPaM( rPaM );
    aNewPaM.SetIndex( pLine->GetStart() );
    return aNewPaM;
}

EditPaM ImpEditEngine::CursorEndOfLine( const EditPaM& rPaM )
{
    ParaPortion* pCurPortion = FindParaPortion( rPaM.GetNode() );
    DBG_ASSERT( pCurPortion, "No Portion for the PaM ?" );
    USHORT nLine = pCurPortion->GetLineNumber( rPaM.GetIndex() );
    EditLine* pLine = pCurPortion->GetLines().GetObject(nLine);
    DBG_ASSERT( pLine, "Current line not found ?!" );

    EditPaM aNewPaM( rPaM );
    aNewPaM.SetIndex( pLine->GetEnd() );
    if ( pLine->GetEnd() > pLine->GetStart() )
    {
        if ( aNewPaM.GetNode()->IsFeature( aNewPaM.GetIndex() - 1 ) )
        {
            // When a soft break, be in front of it!
            EditCharAttrib* pNextFeature = aNewPaM.GetNode()->GetCharAttribs().FindFeature( aNewPaM.GetIndex()-1 );
            if ( pNextFeature && ( pNextFeature->GetItem()->Which() == EE_FEATURE_LINEBR ) )
                aNewPaM = CursorLeft( aNewPaM );
        }
        else if ( ( aNewPaM.GetNode()->GetChar( aNewPaM.GetIndex() - 1 ) == ' ' ) && ( aNewPaM.GetIndex() != aNewPaM.GetNode()->Len() ) )
        {
            // For a Blank in an auto wrapped line, it makes sense, to stand
            // in front of it, since the user wants to be after the word.
            // If this is changed, special treatment for Pos1 to End!
            aNewPaM = CursorLeft( aNewPaM );
        }
    }
    return aNewPaM;
}

EditPaM ImpEditEngine::CursorStartOfParagraph( const EditPaM& rPaM )
{
    EditPaM aPaM( rPaM.GetNode(), 0 );
    return aPaM;
}

EditPaM ImpEditEngine::CursorEndOfParagraph( const EditPaM& rPaM )
{
    EditPaM aPaM( rPaM.GetNode(), rPaM.GetNode()->Len() );
    return aPaM;
}

EditPaM ImpEditEngine::CursorStartOfDoc()
{
    EditPaM aPaM( aEditDoc.SaveGetObject( 0 ), 0 );
    return aPaM;
}

EditPaM ImpEditEngine::CursorEndOfDoc()
{
    ContentNode* pLastNode = aEditDoc.SaveGetObject( aEditDoc.Count()-1 );
    ParaPortion* pLastPortion = GetParaPortions().SaveGetObject( aEditDoc.Count()-1 );
    DBG_ASSERT( pLastNode && pLastPortion, "CursorEndOfDoc: Node or Portion not found" );

    if ( !pLastPortion->IsVisible() )
    {
        pLastNode = GetPrevVisNode( pLastPortion->GetNode() );
        DBG_ASSERT( pLastNode, "Kein sichtbarer Absatz?" );
        if ( !pLastNode )
            pLastNode = aEditDoc.SaveGetObject( aEditDoc.Count()-1 );
    }

    EditPaM aPaM( pLastNode, pLastNode->Len() );
    return aPaM;
}

EditPaM ImpEditEngine::PageUp( const EditPaM& rPaM, EditView* pView )
{
    Rectangle aRec = PaMtoEditCursor( rPaM );
    Point aTopLeft = aRec.TopLeft();
    aTopLeft.Y() -= pView->GetVisArea().GetHeight() *9/10;
    aTopLeft.X() += nOnePixelInRef;
    if ( aTopLeft.Y() < 0 )
    {
        aTopLeft.Y() = 0;
    }
    return GetPaM( aTopLeft );
}

EditPaM ImpEditEngine::PageDown( const EditPaM& rPaM, EditView* pView )
{
    Rectangle aRec = PaMtoEditCursor( rPaM );
    Point aBottomRight = aRec.BottomRight();
    aBottomRight.Y() += pView->GetVisArea().GetHeight() *9/10;
    aBottomRight.X() += nOnePixelInRef;
    long nHeight = GetTextHeight();
    if ( aBottomRight.Y() > nHeight )
    {
        aBottomRight.Y() = nHeight-2;
    }
    return GetPaM( aBottomRight );
}

EditPaM ImpEditEngine::WordLeft( const EditPaM& rPaM, sal_Int16 nWordType )
{
    USHORT nCurrentPos = rPaM.GetIndex();
    EditPaM aNewPaM( rPaM );
    if ( nCurrentPos == 0 )
    {
        // Previous paragraph...
        USHORT nCurPara = aEditDoc.GetPos( aNewPaM.GetNode() );
        ContentNode* pPrevNode = aEditDoc.SaveGetObject( --nCurPara );
        if ( pPrevNode )
        {
            aNewPaM.SetNode( pPrevNode );
            aNewPaM.SetIndex( pPrevNode->Len() );
        }
    }
    else
    {
        // we need to increase the position by 1 when retrieving the locale
        // since the attribute for the char left to the cursor position is returned
        EditPaM aTmpPaM( aNewPaM );
        xub_StrLen nMax = rPaM.GetNode()->Len();
        if ( aTmpPaM.GetIndex() < nMax )
            aTmpPaM.SetIndex( aTmpPaM.GetIndex() + 1 );
        lang::Locale aLocale( GetLocale( aTmpPaM ) );

        uno::Reference < i18n::XBreakIterator > _xBI( ImplGetBreakIterator() );
        i18n::Boundary aBoundary = _xBI->getWordBoundary( *aNewPaM.GetNode(), nCurrentPos, aLocale, nWordType, sal_True );
        if ( aBoundary.startPos >= nCurrentPos )
            aBoundary = _xBI->previousWord( *aNewPaM.GetNode(), nCurrentPos, aLocale, nWordType );
        aNewPaM.SetIndex( ( aBoundary.startPos != (-1) ) ? (USHORT)aBoundary.startPos : 0 );
    }

    return aNewPaM;
}

EditPaM ImpEditEngine::WordRight( const EditPaM& rPaM, sal_Int16 nWordType )
{
    xub_StrLen nMax = rPaM.GetNode()->Len();
    EditPaM aNewPaM( rPaM );
    if ( aNewPaM.GetIndex() < nMax )
    {
        // we need to increase the position by 1 when retrieving the locale
        // since the attribute for the char left to the cursor position is returned
        EditPaM aTmpPaM( aNewPaM );
        aTmpPaM.SetIndex( aTmpPaM.GetIndex() + 1 );
        lang::Locale aLocale( GetLocale( aTmpPaM ) );

        uno::Reference < i18n::XBreakIterator > _xBI( ImplGetBreakIterator() );
        i18n::Boundary aBoundary = _xBI->nextWord( *aNewPaM.GetNode(), aNewPaM.GetIndex(), aLocale, nWordType );
        aNewPaM.SetIndex( (USHORT)aBoundary.startPos );
    }
    // not 'else', maybe the index reached nMax now...
    if ( aNewPaM.GetIndex() >= nMax )
    {
        // Next paragraph ...
        USHORT nCurPara = aEditDoc.GetPos( aNewPaM.GetNode() );
        ContentNode* pNextNode = aEditDoc.SaveGetObject( ++nCurPara );
        if ( pNextNode )
        {
            aNewPaM.SetNode( pNextNode );
            aNewPaM.SetIndex( 0 );
        }
    }
    return aNewPaM;
}

EditPaM ImpEditEngine::StartOfWord( const EditPaM& rPaM, sal_Int16 nWordType )
{
    EditPaM aNewPaM( rPaM );

    // we need to increase the position by 1 when retrieving the locale
    // since the attribute for the char left to the cursor position is returned
    EditPaM aTmpPaM( aNewPaM );
    xub_StrLen nMax = rPaM.GetNode()->Len();
    if ( aTmpPaM.GetIndex() < nMax )
        aTmpPaM.SetIndex( aTmpPaM.GetIndex() + 1 );
    lang::Locale aLocale( GetLocale( aTmpPaM ) );

    uno::Reference < i18n::XBreakIterator > _xBI( ImplGetBreakIterator() );
    i18n::Boundary aBoundary = _xBI->getWordBoundary( *rPaM.GetNode(), rPaM.GetIndex(), aLocale, nWordType, sal_True );
    aNewPaM.SetIndex( (USHORT)aBoundary.startPos );
    return aNewPaM;
}

EditPaM ImpEditEngine::EndOfWord( const EditPaM& rPaM, sal_Int16 nWordType )
{
    EditPaM aNewPaM( rPaM );

    // we need to increase the position by 1 when retrieving the locale
    // since the attribute for the char left to the cursor position is returned
    EditPaM aTmpPaM( aNewPaM );
    xub_StrLen nMax = rPaM.GetNode()->Len();
    if ( aTmpPaM.GetIndex() < nMax )
        aTmpPaM.SetIndex( aTmpPaM.GetIndex() + 1 );
    lang::Locale aLocale( GetLocale( aTmpPaM ) );

    uno::Reference < i18n::XBreakIterator > _xBI( ImplGetBreakIterator() );
    i18n::Boundary aBoundary = _xBI->getWordBoundary( *rPaM.GetNode(), rPaM.GetIndex(), aLocale, nWordType, sal_True );
    aNewPaM.SetIndex( (USHORT)aBoundary.endPos );
    return aNewPaM;
}

EditSelection ImpEditEngine::SelectWord( const EditSelection& rCurSel, sal_Int16 nWordType, BOOL bAcceptStartOfWord )
{
    EditSelection aNewSel( rCurSel );
    EditPaM aPaM( rCurSel.Max() );

    // we need to increase the position by 1 when retrieving the locale
    // since the attribute for the char left to the cursor position is returned
    EditPaM aTmpPaM( aPaM );
    xub_StrLen nMax = aPaM.GetNode()->Len();
    if ( aTmpPaM.GetIndex() < nMax )
        aTmpPaM.SetIndex( aTmpPaM.GetIndex() + 1 );
    lang::Locale aLocale( GetLocale( aTmpPaM ) );

    uno::Reference < i18n::XBreakIterator > _xBI( ImplGetBreakIterator() );
    sal_Int16 nType = _xBI->getWordType( *aPaM.GetNode(), aPaM.GetIndex(), aLocale );
    if ( nType == i18n::WordType::ANY_WORD )
    {
        i18n::Boundary aBoundary = _xBI->getWordBoundary( *aPaM.GetNode(), aPaM.GetIndex(), aLocale, nWordType, sal_True );
        // don't select when curser at end of word
        if ( ( aBoundary.endPos > aPaM.GetIndex() ) &&
             ( ( aBoundary.startPos < aPaM.GetIndex() ) || ( bAcceptStartOfWord && ( aBoundary.startPos == aPaM.GetIndex() ) ) ) )
        {
            aNewSel.Min().SetIndex( (USHORT)aBoundary.startPos );
            aNewSel.Max().SetIndex( (USHORT)aBoundary.endPos );
        }
    }

    return aNewSel;
}

EditSelection ImpEditEngine::SelectSentence( const EditSelection& rCurSel )
{
    uno::Reference < i18n::XBreakIterator > _xBI( ImplGetBreakIterator() );
    const EditPaM& rPaM = rCurSel.Min();
    const ContentNode* pNode = rPaM.GetNode();
    // #i50710# line breaks are marked with 0x01 - the break iterator prefers 0x0a for that
    String sParagraph(*pNode);
    sParagraph.SearchAndReplaceAll(0x01,0x0a);
    //return Null if search starts at the beginning of the string
    long nStart = rPaM.GetIndex() ? _xBI->beginOfSentence( sParagraph, rPaM.GetIndex(), GetLocale( rPaM ) ) : 0;

    long nEnd = _xBI->endOfSentence( *pNode, rPaM.GetIndex(), GetLocale( rPaM ) );
    EditSelection aNewSel( rCurSel );
    DBG_ASSERT(nStart < pNode->Len() && nEnd <= pNode->Len(), "sentence indices out of range");
    aNewSel.Min().SetIndex( (USHORT)nStart );
    aNewSel.Max().SetIndex( (USHORT)nEnd );
    return aNewSel;
}

sal_Bool ImpEditEngine::IsInputSequenceCheckingRequired( sal_Unicode nChar, const EditSelection& rCurSel ) const
{
    uno::Reference < i18n::XBreakIterator > _xBI( ImplGetBreakIterator() );
    if (!pCTLOptions)
        pCTLOptions = new SvtCTLOptions;

    // get the index that really is first
    USHORT nFirstPos = rCurSel.Min().GetIndex();
    USHORT nMaxPos   = rCurSel.Max().GetIndex();
    if (nMaxPos < nFirstPos)
        nFirstPos = nMaxPos;

    sal_Bool bIsSequenceChecking =
        pCTLOptions->IsCTLFontEnabled() &&
        pCTLOptions->IsCTLSequenceChecking() &&
        nFirstPos != 0 && /* first char needs not to be checked */
        _xBI.is() && i18n::ScriptType::COMPLEX == _xBI->getScriptType( rtl::OUString( nChar ), 0 );

    return bIsSequenceChecking;
}

 bool lcl_HasStrongLTR ( const String& rTxt, xub_StrLen nStart, xub_StrLen nEnd )
 {
     for ( xub_StrLen nCharIdx = nStart; nCharIdx < nEnd; ++nCharIdx )
     {
         const UCharDirection nCharDir = u_charDirection ( rTxt.GetChar ( nCharIdx ));
         if ( nCharDir == U_LEFT_TO_RIGHT ||
              nCharDir == U_LEFT_TO_RIGHT_EMBEDDING ||
              nCharDir == U_LEFT_TO_RIGHT_OVERRIDE )
             return true;
     }
     return false;
 }



void ImpEditEngine::InitScriptTypes( USHORT nPara )
{
    ParaPortion* pParaPortion = GetParaPortions().SaveGetObject( nPara );
    ScriptTypePosInfos& rTypes = pParaPortion->aScriptInfos;
    rTypes.Remove( 0, rTypes.Count() );

    ContentNode* pNode = pParaPortion->GetNode();
    if ( pNode->Len() )
    {
        uno::Reference < i18n::XBreakIterator > _xBI( ImplGetBreakIterator() );

        String aText( *pNode );

        // To handle fields put the character from the field in the string,
        // because endOfScript( ... ) will skip the CH_FEATURE, because this is WEAK
        EditCharAttrib* pField = pNode->GetCharAttribs().FindNextAttrib( EE_FEATURE_FIELD, 0 );
        while ( pField )
        {
            ::rtl::OUString aFldText( ((EditCharAttribField*)pField)->GetFieldValue() );
            if ( aFldText.getLength() )
            {
                aText.SetChar( pField->GetStart(), aFldText.getStr()[0] );
                short nFldScriptType = _xBI->getScriptType( aFldText, 0 );

                for ( USHORT nCharInField = 1; nCharInField < aFldText.getLength(); nCharInField++ )
                {
                    short nTmpType = _xBI->getScriptType( aFldText, nCharInField );

                    // First char from field wins...
                    if ( nFldScriptType == i18n::ScriptType::WEAK )
                    {
                        nFldScriptType = nTmpType;
                        aText.SetChar( pField->GetStart(), aFldText.getStr()[nCharInField] );
                    }

                    // ...  but if the first one is LATIN, and there are CJK or CTL chars too,
                    // we prefer that ScripType because we need an other font.
                    if ( ( nTmpType == i18n::ScriptType::ASIAN ) || ( nTmpType == i18n::ScriptType::COMPLEX ) )
                    {
                        aText.SetChar( pField->GetStart(), aFldText.getStr()[nCharInField] );
                        break;
                    }
                }
            }
            // #112831# Last Field might go from 0xffff to 0x0000
            pField = pField->GetEnd() ? pNode->GetCharAttribs().FindNextAttrib( EE_FEATURE_FIELD, pField->GetEnd() ) : NULL;
        }

        ::rtl::OUString aOUText( aText );
        USHORT nTextLen = (USHORT)aOUText.getLength();

        sal_Int32 nPos = 0;
        short nScriptType = _xBI->getScriptType( aOUText, nPos );
        rTypes.Insert( ScriptTypePosInfo( nScriptType, (USHORT)nPos, nTextLen ), rTypes.Count() );
        nPos = _xBI->endOfScript( aOUText, nPos, nScriptType );
        while ( ( nPos != (-1) ) && ( nPos < nTextLen ) )
        {
            rTypes[rTypes.Count()-1].nEndPos = (USHORT)nPos;

            nScriptType = _xBI->getScriptType( aOUText, nPos );
            long nEndPos = _xBI->endOfScript( aOUText, nPos, nScriptType );

            if ( ( nScriptType == i18n::ScriptType::WEAK ) || ( nScriptType == rTypes[rTypes.Count()-1].nScriptType ) )
            {
                // Expand last ScriptTypePosInfo, don't create weak or unecessary portions
                rTypes[rTypes.Count()-1].nEndPos = (USHORT)nEndPos;
            }
            else
            {
                if ( _xBI->getScriptType( aOUText, nPos - 1 ) == i18n::ScriptType::WEAK )
                {
                    switch ( u_charType(aOUText.iterateCodePoints(&nPos, 0) ) ) {
                    case U_NON_SPACING_MARK:
                    case U_ENCLOSING_MARK:
                    case U_COMBINING_SPACING_MARK:
                        --nPos;
                        rTypes[rTypes.Count()-1].nEndPos--;
                        break;
                    }
                }
                rTypes.Insert( ScriptTypePosInfo( nScriptType, (USHORT)nPos, nTextLen ), rTypes.Count() );
            }

            nPos = nEndPos;
        }

        if ( rTypes[0].nScriptType == i18n::ScriptType::WEAK )
            rTypes[0].nScriptType = ( rTypes.Count() > 1 ) ? rTypes[1].nScriptType : GetI18NScriptTypeOfLanguage( GetDefaultLanguage() );

        // create writing direction information:
        if ( !pParaPortion->aWritingDirectionInfos.Count() )
            InitWritingDirections( nPara );

        // i89825: Use CTL font for numbers embedded into an RTL run:
        WritingDirectionInfos& rDirInfos = pParaPortion->aWritingDirectionInfos;
        for ( USHORT n = 0; n < rDirInfos.Count(); ++n )
        {
            const xub_StrLen nStart = rDirInfos[n].nStartPos;
            const xub_StrLen nEnd   = rDirInfos[n].nEndPos;
            const BYTE nCurrDirType = rDirInfos[n].nType;

            if ( nCurrDirType % 2 == UBIDI_RTL  || // text in RTL run
                ( nCurrDirType > UBIDI_LTR && !lcl_HasStrongLTR( aText, nStart, nEnd ) ) ) // non-strong text in embedded LTR run
            {
                USHORT nIdx = 0;

                // Skip entries in ScriptArray which are not inside the RTL run:
                while ( nIdx < rTypes.Count() && rTypes[nIdx].nStartPos < nStart )
                    ++nIdx;

                // Remove any entries *inside* the current run:
                while ( nIdx < rTypes.Count() && rTypes[nIdx].nEndPos <= nEnd )
                    rTypes.Remove( nIdx );

                // special case:
                if(nIdx < rTypes.Count() && rTypes[nIdx].nStartPos < nStart && rTypes[nIdx].nEndPos > nEnd)
                {
                    rTypes.Insert( ScriptTypePosInfo( rTypes[nIdx].nScriptType, (USHORT)nEnd, rTypes[nIdx].nEndPos ), nIdx );
                    rTypes[nIdx].nEndPos = nStart;
                }

                if( nIdx )
                    rTypes[nIdx - 1].nEndPos = nStart;

                rTypes.Insert( ScriptTypePosInfo( i18n::ScriptType::COMPLEX, (USHORT)nStart, (USHORT)nEnd), nIdx );
                ++nIdx;

                if( nIdx < rTypes.Count() )
                    rTypes[nIdx].nStartPos = nEnd;
            }
        }

#if OSL_DEBUG_LEVEL > 1
        USHORT nDebugStt = 0;
        USHORT nDebugEnd = 0;
        short nDebugType = 0;
        for ( USHORT n = 0; n < rTypes.Count(); ++n )
        {
            nDebugStt  = rTypes[n].nStartPos;
            nDebugEnd  = rTypes[n].nEndPos;
            nDebugType = rTypes[n].nScriptType;
        }
#endif
    }
}

USHORT ImpEditEngine::GetScriptType( const EditPaM& rPaM, USHORT* pEndPos ) const
{
    USHORT nScriptType = 0;

    if ( pEndPos )
        *pEndPos = rPaM.GetNode()->Len();

    if ( rPaM.GetNode()->Len() )
    {
         USHORT nPara = GetEditDoc().GetPos( rPaM.GetNode() );
        ParaPortion* pParaPortion = GetParaPortions().SaveGetObject( nPara );
        if ( !pParaPortion->aScriptInfos.Count() )
            ((ImpEditEngine*)this)->InitScriptTypes( nPara );

        ScriptTypePosInfos& rTypes = pParaPortion->aScriptInfos;
        USHORT nPos = rPaM.GetIndex();
        for ( USHORT n = 0; n < rTypes.Count(); n++ )
        {
            if ( ( rTypes[n].nStartPos <= nPos ) && ( rTypes[n].nEndPos >= nPos ) )
               {
                nScriptType = rTypes[n].nScriptType;
                if( pEndPos )
                    *pEndPos = rTypes[n].nEndPos;
                break;
            }
        }
    }
    return nScriptType ? nScriptType : GetI18NScriptTypeOfLanguage( GetDefaultLanguage() );
}

USHORT ImpEditEngine::GetScriptType( const EditSelection& rSel ) const
{
    EditSelection aSel( rSel );
    aSel.Adjust( aEditDoc );

    short nScriptType = 0;

     USHORT nStartPara = GetEditDoc().GetPos( aSel.Min().GetNode() );
     USHORT nEndPara = GetEditDoc().GetPos( aSel.Max().GetNode() );

    for ( USHORT nPara = nStartPara; nPara <= nEndPara; nPara++ )
    {
        ParaPortion* pParaPortion = GetParaPortions().SaveGetObject( nPara );
        if ( !pParaPortion->aScriptInfos.Count() )
            ((ImpEditEngine*)this)->InitScriptTypes( nPara );

        ScriptTypePosInfos& rTypes = pParaPortion->aScriptInfos;

        // find the first(!) script type position that holds the
        // complete selection. Thus it will work for selections as
        // well as with just moving the cursor from char to char.
        USHORT nS = ( nPara == nStartPara ) ? aSel.Min().GetIndex() : 0;
        USHORT nE = ( nPara == nEndPara ) ? aSel.Max().GetIndex() : pParaPortion->GetNode()->Len();
        for ( USHORT n = 0; n < rTypes.Count(); n++ )
        {
            if (rTypes[n].nStartPos <= nS  &&  nE <= rTypes[n].nEndPos)
               {
                if ( rTypes[n].nScriptType != i18n::ScriptType::WEAK )
                {
                    nScriptType |= GetItemScriptType ( rTypes[n].nScriptType );
                }
                else
                {
                    if ( !nScriptType && n )
                    {
                        // #93548# When starting with WEAK, use prev ScriptType...
                        nScriptType = rTypes[n-1].nScriptType;
                    }
                }
                break;
            }
        }
    }
    return nScriptType ? nScriptType : GetI18NScriptTypeOfLanguage( GetDefaultLanguage() );
}

BOOL ImpEditEngine::IsScriptChange( const EditPaM& rPaM ) const
{
    BOOL bScriptChange = FALSE;

    if ( rPaM.GetNode()->Len() )
    {
        USHORT nPara = GetEditDoc().GetPos( rPaM.GetNode() );
        ParaPortion* pParaPortion = GetParaPortions().SaveGetObject( nPara );
        if ( !pParaPortion->aScriptInfos.Count() )
            ((ImpEditEngine*)this)->InitScriptTypes( nPara );

        ScriptTypePosInfos& rTypes = pParaPortion->aScriptInfos;
        USHORT nPos = rPaM.GetIndex();
        for ( USHORT n = 0; n < rTypes.Count(); n++ )
        {
            if ( rTypes[n].nStartPos == nPos )
               {
                bScriptChange = TRUE;
                break;
            }
        }
    }
    return bScriptChange;
}

BOOL ImpEditEngine::HasScriptType( USHORT nPara, USHORT nType ) const
{
    BOOL bTypeFound = FALSE;

    ParaPortion* pParaPortion = GetParaPortions().SaveGetObject( nPara );
    if ( !pParaPortion->aScriptInfos.Count() )
        ((ImpEditEngine*)this)->InitScriptTypes( nPara );

    ScriptTypePosInfos& rTypes = pParaPortion->aScriptInfos;
    for ( USHORT n = rTypes.Count(); n && !bTypeFound; )
    {
        if ( rTypes[--n].nScriptType == nType )
                bTypeFound = TRUE;
    }
    return bTypeFound;
}

void ImpEditEngine::InitWritingDirections( USHORT nPara )
{
    ParaPortion* pParaPortion = GetParaPortions().SaveGetObject( nPara );
    WritingDirectionInfos& rInfos = pParaPortion->aWritingDirectionInfos;
    rInfos.Remove( 0, rInfos.Count() );

    BOOL bCTL = FALSE;
    ScriptTypePosInfos& rTypes = pParaPortion->aScriptInfos;
    for ( USHORT n = 0; n < rTypes.Count(); n++ )
    {
        if ( rTypes[n].nScriptType == i18n::ScriptType::COMPLEX )
           {
            bCTL = TRUE;
            break;
        }
    }

    const UBiDiLevel nBidiLevel = IsRightToLeft( nPara ) ? 1 /*RTL*/ : 0 /*LTR*/;
    if ( ( bCTL || ( nBidiLevel == 1 /*RTL*/ ) ) && pParaPortion->GetNode()->Len() )
    {

        String aText( *pParaPortion->GetNode() );

        //
        // Bidi functions from icu 2.0
        //
        UErrorCode nError = U_ZERO_ERROR;
        UBiDi* pBidi = ubidi_openSized( aText.Len(), 0, &nError );
        nError = U_ZERO_ERROR;

        ubidi_setPara( pBidi, reinterpret_cast<const UChar *>(aText.GetBuffer()), aText.Len(), nBidiLevel, NULL, &nError ); // UChar != sal_Unicode in MinGW
        nError = U_ZERO_ERROR;

        long nCount = ubidi_countRuns( pBidi, &nError );

        int32_t nStart = 0;
        int32_t nEnd;
        UBiDiLevel nCurrDir;

        for ( USHORT nIdx = 0; nIdx < nCount; ++nIdx )
        {
            ubidi_getLogicalRun( pBidi, nStart, &nEnd, &nCurrDir );
            rInfos.Insert( WritingDirectionInfo( nCurrDir, (USHORT)nStart, (USHORT)nEnd ), rInfos.Count() );
            nStart = nEnd;
        }

        ubidi_close( pBidi );
    }

    // No infos mean no CTL and default dir is L2R...
    if ( !rInfos.Count() )
        rInfos.Insert( WritingDirectionInfo( 0, 0, (USHORT)pParaPortion->GetNode()->Len() ), rInfos.Count() );

}

BOOL ImpEditEngine::IsRightToLeft( USHORT nPara ) const
{
    BOOL bR2L = FALSE;
    const SvxFrameDirectionItem* pFrameDirItem = NULL;

    if ( !IsVertical() )
    {
        bR2L = GetDefaultHorizontalTextDirection() == EE_HTEXTDIR_R2L;
        pFrameDirItem = &(const SvxFrameDirectionItem&)GetParaAttrib( nPara, EE_PARA_WRITINGDIR );
        if ( pFrameDirItem->GetValue() == FRMDIR_ENVIRONMENT )
        {
            // #103045# if DefaultHorizontalTextDirection is set, use that value, otherwise pool default.
            if ( GetDefaultHorizontalTextDirection() != EE_HTEXTDIR_DEFAULT )
            {
                pFrameDirItem = NULL; // bR2L allready set to default horizontal text direction
            }
            else
            {
                // Use pool default
                pFrameDirItem = &(const SvxFrameDirectionItem&)((ImpEditEngine*)this)->GetEmptyItemSet().Get( EE_PARA_WRITINGDIR );
            }
        }
    }

    if ( pFrameDirItem )
        bR2L = pFrameDirItem->GetValue() == FRMDIR_HORI_RIGHT_TOP;

    return bR2L;
}

BOOL ImpEditEngine::HasDifferentRTLLevels( const ContentNode* pNode )
{
    USHORT nPara = GetEditDoc().GetPos( (ContentNode*)pNode );
    ParaPortion* pParaPortion = GetParaPortions().SaveGetObject( nPara );

    BOOL bHasDifferentRTLLevels = FALSE;

    USHORT nRTLLevel = IsRightToLeft( nPara ) ? 1 : 0;
    for ( USHORT n = 0; n < pParaPortion->GetTextPortions().Count(); n++ )
    {
        TextPortion* pTextPortion = pParaPortion->GetTextPortions().GetObject( n );
        if ( pTextPortion->GetRightToLeft() != nRTLLevel )
        {
            bHasDifferentRTLLevels = TRUE;
            break;
        }
    }
    return bHasDifferentRTLLevels;
}


BYTE ImpEditEngine::GetRightToLeft( USHORT nPara, USHORT nPos, USHORT* pStart, USHORT* pEnd )
{
    BYTE nRightToLeft = 0;

    ContentNode* pNode = aEditDoc.SaveGetObject( nPara );
    if ( pNode && pNode->Len() )
    {
        ParaPortion* pParaPortion = GetParaPortions().SaveGetObject( nPara );
        if ( !pParaPortion->aWritingDirectionInfos.Count() )
            InitWritingDirections( nPara );

        WritingDirectionInfos& rDirInfos = pParaPortion->aWritingDirectionInfos;
        for ( USHORT n = 0; n < rDirInfos.Count(); n++ )
        {
            if ( ( rDirInfos[n].nStartPos <= nPos ) && ( rDirInfos[n].nEndPos >= nPos ) )
               {
                nRightToLeft = rDirInfos[n].nType;
                if ( pStart )
                    *pStart = rDirInfos[n].nStartPos;
                if ( pEnd )
                    *pEnd = rDirInfos[n].nEndPos;
                break;
            }
        }
    }
    return nRightToLeft;
}

SvxAdjust ImpEditEngine::GetJustification( USHORT nPara ) const
{
    SvxAdjust eJustification = SVX_ADJUST_LEFT;

    if ( !aStatus.IsOutliner() )
    {
        eJustification = ((const SvxAdjustItem&) GetParaAttrib( nPara, EE_PARA_JUST )).GetAdjust();

        if ( IsRightToLeft( nPara ) )
        {
            if ( eJustification == SVX_ADJUST_LEFT )
                eJustification = SVX_ADJUST_RIGHT;
            else if ( eJustification == SVX_ADJUST_RIGHT )
                eJustification = SVX_ADJUST_LEFT;
        }
    }
    return eJustification;
}

SvxCellJustifyMethod ImpEditEngine::GetJustifyMethod( USHORT nPara ) const
{
    const SvxJustifyMethodItem& rItem = static_cast<const SvxJustifyMethodItem&>(
        GetParaAttrib(nPara, EE_PARA_JUST_METHOD));
    return static_cast<SvxCellJustifyMethod>(rItem.GetEnumValue());
}

SvxCellVerJustify ImpEditEngine::GetVerJustification( USHORT nPara ) const
{
    const SvxVerJustifyItem& rItem = static_cast<const SvxVerJustifyItem&>(
        GetParaAttrib(nPara, EE_PARA_VER_JUST));
    return static_cast<SvxCellVerJustify>(rItem.GetEnumValue());
}

//  ----------------------------------------------------------------------
//  Text changes
//  ----------------------------------------------------------------------

void ImpEditEngine::ImpRemoveChars( const EditPaM& rPaM, USHORT nChars, EditUndoRemoveChars* pCurUndo )
{
    if ( IsUndoEnabled() && !IsInUndo() )
    {
        XubString aStr( rPaM.GetNode()->Copy( rPaM.GetIndex(), nChars ) );

        // Check whether attributes are deleted or changed:
        USHORT nStart = rPaM.GetIndex();
        USHORT nEnd = nStart + nChars;
        CharAttribArray& rAttribs = rPaM.GetNode()->GetCharAttribs().GetAttribs();

        for ( USHORT nAttr = 0; nAttr < rAttribs.Count(); nAttr++ )
        {
            EditCharAttrib* pAttr = rAttribs[nAttr];
            if ( ( pAttr->GetEnd() >= nStart ) && ( pAttr->GetStart() < nEnd ) )
            {
                EditSelection aSel( rPaM );
                aSel.Max().GetIndex() = aSel.Max().GetIndex() + nChars;
                EditUndoSetAttribs* pAttrUndo = CreateAttribUndo( aSel, GetEmptyItemSet() );
                InsertUndo( pAttrUndo );
                break;  // for
            }
        }
        if ( pCurUndo && ( CreateEditPaM( pCurUndo->GetEPaM() ) == rPaM ) )
            pCurUndo->GetStr() += aStr;
        else
            InsertUndo( new EditUndoRemoveChars( this, CreateEPaM( rPaM ), aStr ) );
    }

    aEditDoc.RemoveChars( rPaM, nChars );
    TextModified();
}

EditSelection ImpEditEngine::ImpMoveParagraphs( Range aOldPositions, USHORT nNewPos )
{
    aOldPositions.Justify();
    BOOL bValidAction = ( (long)nNewPos < aOldPositions.Min() ) || ( (long)nNewPos > aOldPositions.Max() );
    DBG_ASSERT( bValidAction, "Move in itself?" );
    DBG_ASSERT( aOldPositions.Max() <= (long)GetParaPortions().Count(), "totally over it: MoveParagraphs" );

    EditSelection aSelection;

    if ( !bValidAction )
    {
        aSelection = aEditDoc.GetStartPaM();
        return aSelection;
    }

    ULONG nParaCount = GetParaPortions().Count();

    if ( nNewPos >= nParaCount )
        nNewPos = GetParaPortions().Count();

    // Height may change when moving first or last Paragraph
    ParaPortion* pRecalc1 = NULL;
    ParaPortion* pRecalc2 = NULL;
    ParaPortion* pRecalc3 = NULL;
    ParaPortion* pRecalc4 = NULL;

    if ( nNewPos == 0 ) // Move to Start
    {
        pRecalc1 = GetParaPortions().GetObject( 0 );
        pRecalc2 = GetParaPortions().GetObject( (USHORT)aOldPositions.Min() );

    }
    else if ( nNewPos == nParaCount )
    {
        pRecalc1 = GetParaPortions().GetObject( (USHORT)(nParaCount-1) );
        pRecalc2 = GetParaPortions().GetObject( (USHORT)aOldPositions.Max() );
    }

    if ( aOldPositions.Min() == 0 ) // Move from Start
    {
        pRecalc3 = GetParaPortions().GetObject( 0 );
        pRecalc4 = GetParaPortions().GetObject(
            sal::static_int_cast< USHORT >( aOldPositions.Max()+1 ) );
    }
    else if ( (USHORT)aOldPositions.Max() == (nParaCount-1) )
    {
        pRecalc3 = GetParaPortions().GetObject( (USHORT)aOldPositions.Max() );
        pRecalc4 = GetParaPortions().GetObject( (USHORT)(aOldPositions.Min()-1) );
    }

    MoveParagraphsInfo aMoveParagraphsInfo( sal::static_int_cast< USHORT >(aOldPositions.Min()), sal::static_int_cast< USHORT >(aOldPositions.Max()), nNewPos );
    aBeginMovingParagraphsHdl.Call( &aMoveParagraphsInfo );

    if ( IsUndoEnabled() && !IsInUndo())
        InsertUndo( new EditUndoMoveParagraphs( this, aOldPositions, nNewPos ) );

    // do not lose sight of the Position !
    ParaPortion* pDestPortion = GetParaPortions().SaveGetObject( nNewPos );

    ParaPortionList aTmpPortionList;
    USHORT i;
    for ( i = (USHORT)aOldPositions.Min(); i <= (USHORT)aOldPositions.Max(); i++  )
    {
        // always aOldPositions.Min(), since Remove().
        ParaPortion* pTmpPortion = GetParaPortions().GetObject( (USHORT)aOldPositions.Min() );
        GetParaPortions().Remove( (USHORT)aOldPositions.Min() );
        aEditDoc.Remove( (USHORT)aOldPositions.Min() );
        aTmpPortionList.Insert( pTmpPortion, aTmpPortionList.Count() );
    }

    USHORT nRealNewPos = pDestPortion ? GetParaPortions().GetPos( pDestPortion ) : GetParaPortions().Count();
    DBG_ASSERT( nRealNewPos != USHRT_MAX, "ImpMoveParagraphs: Invalid Position!" );

    for ( i = 0; i < (USHORT)aTmpPortionList.Count(); i++  )
    {
        ParaPortion* pTmpPortion = aTmpPortionList.GetObject( i );
        if ( i == 0 )
            aSelection.Min().SetNode( pTmpPortion->GetNode() );

        aSelection.Max().SetNode( pTmpPortion->GetNode() );
        aSelection.Max().SetIndex( pTmpPortion->GetNode()->Len() );

        ContentNode* pN = pTmpPortion->GetNode();
        aEditDoc.Insert( pN, nRealNewPos+i );

        GetParaPortions().Insert( pTmpPortion, nRealNewPos+i );
    }

    aEndMovingParagraphsHdl.Call( &aMoveParagraphsInfo );

    if ( GetNotifyHdl().IsSet() )
    {
        EENotify aNotify( EE_NOTIFY_PARAGRAPHSMOVED );
        aNotify.pEditEngine = GetEditEnginePtr();
        aNotify.nParagraph = nNewPos;
        aNotify.nParam1 = sal::static_int_cast< USHORT >(aOldPositions.Min());
        aNotify.nParam2 = sal::static_int_cast< USHORT >(aOldPositions.Max());
        CallNotify( aNotify );
    }

    aEditDoc.SetModified( TRUE );

    if ( pRecalc1 )
        CalcHeight( pRecalc1 );
    if ( pRecalc2 )
        CalcHeight( pRecalc2 );
    if ( pRecalc3 )
        CalcHeight( pRecalc3 );
    if ( pRecalc4 )
        CalcHeight( pRecalc4 );

    aTmpPortionList.Remove( 0, aTmpPortionList.Count() );   // important !

#ifdef EDITDEBUG
    GetParaPortions().DbgCheck(aEditDoc);
#endif
    return aSelection;
}


EditPaM ImpEditEngine::ImpConnectParagraphs( ContentNode* pLeft, ContentNode* pRight, BOOL bBackward )
{
    DBG_ASSERT( pLeft != pRight, "Join together the same paragraph ?" );
    DBG_ASSERT( aEditDoc.GetPos( pLeft ) != USHRT_MAX, "Inserted node not found (1)" );
    DBG_ASSERT( aEditDoc.GetPos( pRight ) != USHRT_MAX, "Inserted node not found (2)" );

    USHORT nParagraphTobeDeleted = aEditDoc.GetPos( pRight );
    DeletedNodeInfo* pInf = new DeletedNodeInfo( (ULONG)pRight, nParagraphTobeDeleted );
    aDeletedNodes.Insert( pInf, aDeletedNodes.Count() );

    GetEditEnginePtr()->ParagraphConnected( aEditDoc.GetPos( pLeft ), aEditDoc.GetPos( pRight ) );

    if ( IsUndoEnabled() && !IsInUndo() )
    {
        InsertUndo( new EditUndoConnectParas( this,
            aEditDoc.GetPos( pLeft ), pLeft->Len(),
            pLeft->GetContentAttribs().GetItems(), pRight->GetContentAttribs().GetItems(),
            pLeft->GetStyleSheet(), pRight->GetStyleSheet(), bBackward ) );
    }

    if ( bBackward )
    {
        pLeft->SetStyleSheet( pRight->GetStyleSheet(), TRUE );
        pLeft->GetContentAttribs().GetItems().Set( pRight->GetContentAttribs().GetItems() );
        pLeft->GetCharAttribs().GetDefFont() = pRight->GetCharAttribs().GetDefFont();
    }

    ParaAttribsChanged( pLeft );

    // First search for Portions since pRight is gone after ConnectParagraphs.
    ParaPortion* pLeftPortion = FindParaPortion( pLeft );
    ParaPortion* pRightPortion = FindParaPortion( pRight );
    DBG_ASSERT( pLeftPortion, "Blind Portion in ImpConnectParagraphs(1)" );
    DBG_ASSERT( pRightPortion, "Blind Portion in ImpConnectParagraphs(2)" );
    DBG_ASSERT( nParagraphTobeDeleted == GetParaPortions().GetPos( pRightPortion ), "NodePos != PortionPos?" );

    if ( GetStatus().DoOnlineSpelling() )
    {
        xub_StrLen nEnd = pLeft->Len();
        xub_StrLen nInv = nEnd ? nEnd-1 : nEnd;
        pLeft->GetWrongList()->ClearWrongs( nInv, 0xFFFF, pLeft );  // Possibly remove one
        pLeft->GetWrongList()->MarkInvalid( nInv, nEnd+1 );
        // Take over misspelled words
        USHORT nRWrongs = pRight->GetWrongList()->Count();
        for ( USHORT nW = 0; nW < nRWrongs; nW++ )
        {
            WrongRange aWrong = pRight->GetWrongList()->GetObject( nW );
            if ( aWrong.nStart != 0 )   // Not a subsequent
            {
                aWrong.nStart = aWrong.nStart + nEnd;
                aWrong.nEnd = aWrong.nEnd + nEnd;
                pLeft->GetWrongList()->InsertWrong( aWrong, pLeft->GetWrongList()->Count() );
            }
        }
    }

    if ( IsCallParaInsertedOrDeleted() )
        GetEditEnginePtr()->ParagraphDeleted( nParagraphTobeDeleted );

    EditPaM aPaM = aEditDoc.ConnectParagraphs( pLeft, pRight );
    GetParaPortions().Remove( nParagraphTobeDeleted );
    delete pRightPortion;

    pLeftPortion->MarkSelectionInvalid( aPaM.GetIndex(), pLeft->Len() );

    // the right node is deleted by EditDoc:ConnectParagraphs().
    if ( GetTextRanger() )
    {
        // By joining together the two, the left is although reformatted,
        // however if its height does not change then the formatting receives
        // the change of the total text hight too late...
        for ( USHORT n = nParagraphTobeDeleted; n < GetParaPortions().Count(); n++ )
        {
            ParaPortion* pPP = GetParaPortions().GetObject( n );
            pPP->MarkSelectionInvalid( 0, pPP->GetNode()->Len() );
            pPP->GetLines().Reset();
        }
    }

    TextModified();

    return aPaM;
}

EditPaM ImpEditEngine::DeleteLeftOrRight( const EditSelection& rSel, BYTE nMode, BYTE nDelMode )
{
    DBG_ASSERT( !EditSelection( rSel ).DbgIsBuggy( aEditDoc ), "Index out of range in DeleteLeftOrRight" );

    if ( rSel.HasRange() )  // only then Delete Selection
        return ImpDeleteSelection( rSel );

    const EditPaM aCurPos( rSel.Max() );
    EditPaM aDelStart( aCurPos );
    EditPaM aDelEnd( aCurPos );
    if ( nMode == DEL_LEFT )
    {
        if ( nDelMode == DELMODE_SIMPLE )
        {
            aDelStart = CursorLeft( aCurPos, i18n::CharacterIteratorMode::SKIPCHARACTER );
        }
        else if ( nDelMode == DELMODE_RESTOFWORD )
        {
            aDelStart = StartOfWord( aCurPos );
            if ( aDelStart.GetIndex() == aCurPos.GetIndex() )
                aDelStart = WordLeft( aCurPos );
        }
        else    // DELMODE_RESTOFCONTENT
        {
            aDelStart.SetIndex( 0 );
            if ( aDelStart == aCurPos )
            {
                // Complete paragraph previous
                ContentNode* pPrev = GetPrevVisNode( aCurPos.GetNode() );
                if ( pPrev )
                    aDelStart = EditPaM( pPrev, 0 );
            }
        }
    }
    else
    {
        if ( nDelMode == DELMODE_SIMPLE )
        {
            aDelEnd = CursorRight( aCurPos );
        }
        else if ( nDelMode == DELMODE_RESTOFWORD )
        {
            aDelEnd = EndOfWord( aCurPos );
            if (aDelEnd.GetIndex() == aCurPos.GetIndex())
            {
                xub_StrLen nLen = aCurPos.GetNode()->Len();
                // end of para?
                if (aDelEnd.GetIndex() == nLen)
                    aDelEnd = WordLeft( aCurPos );
                else // there's still sth to delete on the right
                {
                    aDelEnd = EndOfWord( WordRight( aCurPos ) );
                    // if there'n no next word...
                    if (aDelEnd.GetIndex() == nLen )
                        aDelEnd.SetIndex( nLen );
                }
            }
        }
        else    // DELMODE_RESTOFCONTENT
        {
            aDelEnd.SetIndex( aCurPos.GetNode()->Len() );
            if ( aDelEnd == aCurPos )
            {
                // Complete paragraph next
                ContentNode* pNext = GetNextVisNode( aCurPos.GetNode() );
                if ( pNext )
                    aDelEnd = EditPaM( pNext, pNext->Len() );
            }
        }
    }

    // ConnectParagraphs  not enoguh for different Nodes when
    // DELMODE_RESTOFCONTENT.
    if ( ( nDelMode == DELMODE_RESTOFCONTENT ) || ( aDelStart.GetNode() == aDelEnd.GetNode() ) )
        return ImpDeleteSelection( EditSelection( aDelStart, aDelEnd ) );

    // Decide now if to delete selection (RESTOFCONTENTS)
    BOOL bSpecialBackward = ( ( nMode == DEL_LEFT ) && ( nDelMode == DELMODE_SIMPLE ) )
                                ? TRUE : FALSE;
    if ( aStatus.IsAnyOutliner() )
        bSpecialBackward = FALSE;

    return ImpConnectParagraphs( aDelStart.GetNode(), aDelEnd.GetNode(), bSpecialBackward );
}

EditPaM ImpEditEngine::ImpDeleteSelection( EditSelection aSel )
{
    if ( !aSel.HasRange() )
        return aSel.Min();

    aSel.Adjust( aEditDoc );
    EditPaM aStartPaM( aSel.Min() );
    EditPaM aEndPaM( aSel.Max() );

    CursorMoved( aStartPaM.GetNode() ); // only so that newly set Attributes dissapear...
    CursorMoved( aEndPaM.GetNode() );   // only so that newly set Attributes dissapear...

    DBG_ASSERT( aStartPaM.GetIndex() <= aStartPaM.GetNode()->Len(), "Index out of range in ImpDeleteSelection" );
    DBG_ASSERT( aEndPaM.GetIndex() <= aEndPaM.GetNode()->Len(), "Index out of range in ImpDeleteSelection" );

    USHORT nStartNode = aEditDoc.GetPos( aStartPaM.GetNode() );
    USHORT nEndNode = aEditDoc.GetPos( aEndPaM.GetNode() );

    DBG_ASSERT( nEndNode != USHRT_MAX, "Start > End ?!" );
    DBG_ASSERT( nStartNode <= nEndNode, "Start > End ?!" );

    // Remove all nodes in between ....
    for ( ULONG z = nStartNode+1; z < nEndNode; z++ )
    {
        // Always nStartNode+1, due to Remove()!
        ImpRemoveParagraph( nStartNode+1 );
    }

    if ( aStartPaM.GetNode() != aEndPaM.GetNode() )
    {
        // The Rest of the StartNodes...
        USHORT nChars;
        nChars = aStartPaM.GetNode()->Len() - aStartPaM.GetIndex();
        ImpRemoveChars( aStartPaM, nChars );
        ParaPortion* pPortion = FindParaPortion( aStartPaM.GetNode() );
        DBG_ASSERT( pPortion, "Blind Portion in ImpDeleteSelection(3)" );
        pPortion->MarkSelectionInvalid( aStartPaM.GetIndex(), aStartPaM.GetNode()->Len() );

        // The beginning of the EndNodes....
        nChars = aEndPaM.GetIndex();
        aEndPaM.SetIndex( 0 );
        ImpRemoveChars( aEndPaM, nChars );
        pPortion = FindParaPortion( aEndPaM.GetNode() );
        DBG_ASSERT( pPortion, "Blind Portion in ImpDeleteSelection(4)" );
        pPortion->MarkSelectionInvalid( 0, aEndPaM.GetNode()->Len() );
        // Join together....
        aStartPaM = ImpConnectParagraphs( aStartPaM.GetNode(), aEndPaM.GetNode() );
    }
    else
    {
        USHORT nChars;
        nChars = aEndPaM.GetIndex() - aStartPaM.GetIndex();
        ImpRemoveChars( aStartPaM, nChars );
        ParaPortion* pPortion = FindParaPortion( aStartPaM.GetNode() );
        DBG_ASSERT( pPortion, "Blind Portion in ImpDeleteSelection(5)" );
        pPortion->MarkInvalid( aEndPaM.GetIndex(), aStartPaM.GetIndex() - aEndPaM.GetIndex() );
    }

    UpdateSelections();
    TextModified();
    return aStartPaM;
}

void ImpEditEngine::ImpRemoveParagraph( USHORT nPara )
{
    ContentNode* pNode = aEditDoc.SaveGetObject( nPara );
    ContentNode* pNextNode = aEditDoc.SaveGetObject( nPara+1 );
    ParaPortion* pPortion = GetParaPortions().SaveGetObject( nPara );

    DBG_ASSERT( pNode, "Blind Node in ImpRemoveParagraph" );
    DBG_ASSERT( pPortion, "Blind Portion in ImpRemoveParagraph(2)" );

    DeletedNodeInfo* pInf = new DeletedNodeInfo( (ULONG)pNode, nPara );
    aDeletedNodes.Insert( pInf, aDeletedNodes.Count() );

    // The node is managed by the undo and possibly destroyed!
    /* delete */ aEditDoc.Remove( nPara );
    GetParaPortions().Remove( nPara );
    delete pPortion;

    if ( IsCallParaInsertedOrDeleted() )
    {
        GetEditEnginePtr()->ParagraphDeleted( nPara );
    }

    // Extra-Space may be determined again in the following. For
    // ParaAttribsChanged the paragraph is unfortunately formatted again,
    // however this method should not be time critical!
    if ( pNextNode )
        ParaAttribsChanged( pNextNode );

    if ( IsUndoEnabled() && !IsInUndo() )
        InsertUndo( new EditUndoDelContent( this, pNode, nPara ) );
    else
    {
        aEditDoc.RemoveItemsFromPool( pNode );
        if ( pNode->GetStyleSheet() )
            EndListening( *pNode->GetStyleSheet(), FALSE );
        delete pNode;
    }
}

EditPaM ImpEditEngine::AutoCorrect( const EditSelection& rCurSel, xub_Unicode c,
                                    bool bOverwrite, Window* pFrameWin )
{
    EditSelection aSel( rCurSel );
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get()->GetAutoCorrect();
    if ( pAutoCorrect )
    {
        if ( aSel.HasRange() )
            aSel = ImpDeleteSelection( rCurSel );

        // #i78661 allow application to turn off capitalization of
        // start sentence explicitly.
        // (This is done by setting IsFirstWordCapitalization to FALSE.)
        BOOL bOldCptlSttSntnc = pAutoCorrect->IsAutoCorrFlag( CptlSttSntnc );
        if (!IsFirstWordCapitalization())
        {
            ESelection aESel( CreateESel(aSel) );
            EditSelection aFirstWordSel;
            EditSelection aSecondWordSel;
            if (aESel.nEndPara == 0)    // is this the first para?
            {
                // select first word...
                // start by checking if para starts with word.
                aFirstWordSel = SelectWord( CreateSel(ESelection()) );
                if (aFirstWordSel.Min().GetIndex() == 0 && aFirstWordSel.Max().GetIndex() == 0)
                {
                    // para does not start with word -> select next/first word
                    EditPaM aRightWord( WordRight( aFirstWordSel.Max(), 1 ) );
                    aFirstWordSel = SelectWord( EditSelection( aRightWord ) );
                }

                // select second word
                // (sometimes aSel mightnot point to the end of the first word
                // but to some following char like '.'. ':', ...
                // In those cases we need aSecondWordSel to see if aSel
                // will actually effect the first word.)
                EditPaM aRight2Word( WordRight( aFirstWordSel.Max(), 1 ) );
                aSecondWordSel = SelectWord( EditSelection( aRight2Word ) );
            }
            BOOL bIsFirstWordInFirstPara = aESel.nEndPara == 0 &&
                    aFirstWordSel.Max().GetIndex() <= aSel.Max().GetIndex() &&
                    aSel.Max().GetIndex() <= aSecondWordSel.Min().GetIndex();

            if (bIsFirstWordInFirstPara)
                pAutoCorrect->SetAutoCorrFlag( CptlSttSntnc, IsFirstWordCapitalization() );
        }

        ContentNode* pNode = aSel.Max().GetNode();
        USHORT nIndex = aSel.Max().GetIndex();
        EdtAutoCorrDoc aAuto( this, pNode, nIndex, c );
        pAutoCorrect->AutoCorrect( aAuto, *pNode, nIndex, c, !bOverwrite, pFrameWin );
        aSel.Max().SetIndex( aAuto.GetCursor() );

        // #i78661 since the SvxAutoCorrect object used here is
        // shared we need to reset the value to it's original state.
        pAutoCorrect->SetAutoCorrFlag( CptlSttSntnc, bOldCptlSttSntnc );
    }
    return aSel.Max();
}


EditPaM ImpEditEngine::InsertText( const EditSelection& rCurSel,
        xub_Unicode c, BOOL bOverwrite, sal_Bool bIsUserInput )
{
    DBG_ASSERT( c != '\t', "Tab for InsertText ?" );
    DBG_ASSERT( c != '\n', "Word wrapping for InsertText ?");

    EditPaM aPaM( rCurSel.Min() );

    BOOL bDoOverwrite = ( bOverwrite &&
            ( aPaM.GetIndex() < aPaM.GetNode()->Len() ) ) ? TRUE : FALSE;

    BOOL bUndoAction = ( rCurSel.HasRange() || bDoOverwrite );

    if ( bUndoAction )
        UndoActionStart( EDITUNDO_INSERT );

    if ( rCurSel.HasRange() )
    {
        aPaM = ImpDeleteSelection( rCurSel );
    }
    else if ( bDoOverwrite )
    {
        // If selected, then do not also overwrite a character!
        EditSelection aTmpSel( aPaM );
        aTmpSel.Max().GetIndex()++;
        DBG_ASSERT( !aTmpSel.DbgIsBuggy( aEditDoc ), "Overwrite: Wrong selection! ");
        ImpDeleteSelection( aTmpSel );
    }

    if ( aPaM.GetNode()->Len() < MAXCHARSINPARA )
    {
        if (bIsUserInput && IsInputSequenceCheckingRequired( c, rCurSel ))
        {
            uno::Reference < i18n::XExtendedInputSequenceChecker > _xISC( ImplGetInputSequenceChecker() );
            if (!pCTLOptions)
                pCTLOptions = new SvtCTLOptions;

            if (_xISC.is() || pCTLOptions)
            {
                xub_StrLen nTmpPos = aPaM.GetIndex();
                sal_Int16 nCheckMode = pCTLOptions->IsCTLSequenceCheckingRestricted() ?
                        i18n::InputSequenceCheckMode::STRICT : i18n::InputSequenceCheckMode::BASIC;

                // the text that needs to be checked is only the one
                // before the current cursor position
                rtl::OUString aOldText( aPaM.GetNode()->Copy(0, nTmpPos) );
                rtl::OUString aNewText( aOldText );
                if (pCTLOptions->IsCTLSequenceCheckingTypeAndReplace())
                {
                    /*const xub_StrLen nPrevPos = static_cast< xub_StrLen >*/( _xISC->correctInputSequence( aNewText, nTmpPos - 1, c, nCheckMode ) );

                    // find position of first character that has changed
                    sal_Int32 nOldLen = aOldText.getLength();
                    sal_Int32 nNewLen = aNewText.getLength();
                    const sal_Unicode *pOldTxt = aOldText.getStr();
                    const sal_Unicode *pNewTxt = aNewText.getStr();
                    sal_Int32 nChgPos = 0;
                    while ( nChgPos < nOldLen && nChgPos < nNewLen &&
                            pOldTxt[nChgPos] == pNewTxt[nChgPos] )
                        ++nChgPos;

                    xub_StrLen nChgLen = static_cast< xub_StrLen >( nNewLen - nChgPos );
                    String aChgText( aNewText.copy( nChgPos ), nChgLen );

                    // select text from first pos to be changed to current pos
                    EditSelection aSel( EditPaM( aPaM.GetNode(), (USHORT) nChgPos ), aPaM );

                    if (aChgText.Len())
                        return InsertText( aSel, aChgText ); // implicitly handles undo
                    else
                        return aPaM;
                }
                else
                {
                    // should the character be ignored (i.e. not get inserted) ?
                    if (!_xISC->checkInputSequence( aOldText, nTmpPos - 1, c, nCheckMode ))
                        return aPaM;    // nothing to be done -> no need for undo
                }
            }

            // at this point now we will insert the character 'normally' some lines below...
        }

        if ( IsUndoEnabled() && !IsInUndo() )
        {
            EditUndoInsertChars* pNewUndo = new EditUndoInsertChars( this, CreateEPaM( aPaM ), c );
            BOOL bTryMerge = ( !bDoOverwrite && ( c != ' ' ) ) ? TRUE : FALSE;
            InsertUndo( pNewUndo, bTryMerge );
        }

        aEditDoc.InsertText( (const EditPaM&)aPaM, c );
        ParaPortion* pPortion = FindParaPortion( aPaM.GetNode() );
        DBG_ASSERT( pPortion, "Blind Portion in InsertText" );
        pPortion->MarkInvalid( aPaM.GetIndex(), 1 );
        aPaM.GetIndex()++;   // does not do EditDoc-Method anymore
    }

    TextModified();

    if ( bUndoAction )
        UndoActionEnd( EDITUNDO_INSERT );

    return aPaM;
}

EditPaM ImpEditEngine::ImpInsertText( EditSelection aCurSel, const XubString& rStr )
{
    UndoActionStart( EDITUNDO_INSERT );

    EditPaM aPaM;
    if ( aCurSel.HasRange() )
        aPaM = ImpDeleteSelection( aCurSel );
    else
        aPaM = aCurSel.Max();

    EditPaM aCurPaM( aPaM );    // for the Invalidate

    // get word boundaries in order to clear possible WrongList entries
    // and invalidate all the necessary text (everything after and including the
    // start of the word)
    EditSelection aCurWord( SelectWord( aCurPaM, i18n::WordType::DICTIONARY_WORD ) );

    XubString aText( rStr );
    aText.ConvertLineEnd( LINEEND_LF );
    SfxVoidItem aTabItem( EE_FEATURE_TAB );

    // Converts to linesep = \n
    // Token LINE_SEP query,
    // since the MAC-Compiler makes something else from \n !

    USHORT nStart = 0;
    while ( nStart < aText.Len() )
    {
        USHORT nEnd = aText.Search( LINE_SEP, nStart );
        if ( nEnd == STRING_NOTFOUND )
            nEnd = aText.Len(); // not dereference!

        // Start == End => empty line
        if ( nEnd > nStart )
        {
            XubString aLine( aText, nStart, nEnd-nStart );
            xub_StrLen nChars = aPaM.GetNode()->Len() + aLine.Len();
            if ( nChars > MAXCHARSINPARA )
            {
                USHORT nMaxNewChars = MAXCHARSINPARA-aPaM.GetNode()->Len();
                nEnd -= ( aLine.Len() - nMaxNewChars ); // Then the characters end up in the next paragraph.
                aLine.Erase( nMaxNewChars );            // Delete the Rest...
            }
            if ( IsUndoEnabled() && !IsInUndo() )
                InsertUndo( new EditUndoInsertChars( this, CreateEPaM( aPaM ), aLine ) );
            // Tabs ?
            if ( aLine.Search( '\t' ) == STRING_NOTFOUND )
                aPaM = aEditDoc.InsertText( aPaM, aLine );
            else
            {
                USHORT nStart2 = 0;
                while ( nStart2 < aLine.Len() )
                {
                    USHORT nEnd2 = aLine.Search( '\t', nStart2 );
                    if ( nEnd2 == STRING_NOTFOUND )
                        nEnd2 = aLine.Len();    // not dereference!

                    if ( nEnd2 > nStart2 )
                        aPaM = aEditDoc.InsertText( aPaM, XubString( aLine, nStart2, nEnd2-nStart2 ) );
                    if ( nEnd2 < aLine.Len() )
                    {
                        aPaM = aEditDoc.InsertFeature( aPaM, aTabItem );
                    }
                    nStart2 = nEnd2+1;
                }
            }
            ParaPortion* pPortion = FindParaPortion( aPaM.GetNode() );
            DBG_ASSERT( pPortion, "Blind Portion in InsertText" );

            // now remove the Wrongs (red spell check marks) from both words...
            WrongList *pWrongs = aCurPaM.GetNode()->GetWrongList();
            if (pWrongs && pWrongs->HasWrongs())
                pWrongs->ClearWrongs( aCurWord.Min().GetIndex(), aPaM.GetIndex(), aPaM.GetNode() );
            // ... and mark both words as 'to be checked again'
            pPortion->MarkInvalid( aCurWord.Min().GetIndex(), aLine.Len() );
        }
        if ( nEnd < aText.Len() )
            aPaM = ImpInsertParaBreak( aPaM );

        nStart = nEnd+1;
    }

    UndoActionEnd( EDITUNDO_INSERT );

    TextModified();
    return aPaM;
}

EditPaM ImpEditEngine::ImpFastInsertText( EditPaM aPaM, const XubString& rStr )
{
    DBG_ASSERT( rStr.Search( 0x0A ) == STRING_NOTFOUND, "FastInsertText: Newline not allowed! ");
    DBG_ASSERT( rStr.Search( 0x0D ) == STRING_NOTFOUND, "FastInsertText: Newline not allowed! ");
    DBG_ASSERT( rStr.Search( '\t' ) == STRING_NOTFOUND, "FastInsertText: Newline not allowed! ");

    if ( ( aPaM.GetNode()->Len() + rStr.Len() ) < MAXCHARSINPARA )
    {
        if ( IsUndoEnabled() && !IsInUndo() )
            InsertUndo( new EditUndoInsertChars( this, CreateEPaM( aPaM ), rStr ) );

        aPaM = aEditDoc.InsertText( aPaM, rStr );
        TextModified();
    }
    else
    {
        aPaM = ImpInsertText( aPaM, rStr );
    }

    return aPaM;
}

EditPaM ImpEditEngine::ImpInsertFeature( EditSelection aCurSel, const SfxPoolItem& rItem )
{
    EditPaM aPaM;
    if ( aCurSel.HasRange() )
        aPaM = ImpDeleteSelection( aCurSel );
    else
        aPaM = aCurSel.Max();

    if ( aPaM.GetIndex() >= 0xfffe )
        return aPaM;

    if ( IsUndoEnabled() && !IsInUndo() )
        InsertUndo( new EditUndoInsertFeature( this, CreateEPaM( aPaM ), rItem ) );
    aPaM = aEditDoc.InsertFeature( aPaM, rItem );

    ParaPortion* pPortion = FindParaPortion( aPaM.GetNode() );
    DBG_ASSERT( pPortion, "Blind Portion in InsertFeature" );
    pPortion->MarkInvalid( aPaM.GetIndex()-1, 1 );

    TextModified();

    return aPaM;
}

EditPaM ImpEditEngine::ImpInsertParaBreak( const EditSelection& rCurSel, BOOL bKeepEndingAttribs )
{
    EditPaM aPaM;
    if ( rCurSel.HasRange() )
        aPaM = ImpDeleteSelection( rCurSel );
    else
        aPaM = rCurSel.Max();

    return ImpInsertParaBreak( aPaM, bKeepEndingAttribs );
}

EditPaM ImpEditEngine::ImpInsertParaBreak( const EditPaM& rPaM, BOOL bKeepEndingAttribs )
{
    if ( aEditDoc.Count() >= 0xFFFE )
    {
        OSL_FAIL( "Can't process more than 64K paragraphs!" );
        return rPaM;
    }

    if ( IsUndoEnabled() && !IsInUndo() )
        InsertUndo( new EditUndoSplitPara( this, aEditDoc.GetPos( rPaM.GetNode() ), rPaM.GetIndex() ) );

    EditPaM aPaM( aEditDoc.InsertParaBreak( rPaM, bKeepEndingAttribs ) );

    if ( GetStatus().DoOnlineSpelling() )
    {
        xub_StrLen nEnd = rPaM.GetNode()->Len();
        aPaM.GetNode()->CreateWrongList();
        WrongList* pLWrongs = rPaM.GetNode()->GetWrongList();
        WrongList* pRWrongs = aPaM.GetNode()->GetWrongList();
        // take over misspelled words:
        USHORT nLWrongs = pLWrongs->Count();
        for ( USHORT nW = 0; nW < nLWrongs; nW++ )
        {
            WrongRange& rWrong = pLWrongs->GetObject( nW );
            // Correct only if really a word gets overlapped in the process of
            // Spell checking
            if ( rWrong.nStart > nEnd )
            {
                pRWrongs->InsertWrong( rWrong, pRWrongs->Count() );
                WrongRange& rRWrong = pRWrongs->GetObject( pRWrongs->Count() - 1 );
                rRWrong.nStart = rRWrong.nStart - nEnd;
                rRWrong.nEnd = rRWrong.nEnd - nEnd;
            }
            else if ( ( rWrong.nStart < nEnd ) && ( rWrong.nEnd > nEnd ) )
                rWrong.nEnd = nEnd;
        }
        USHORT nInv = nEnd ? nEnd-1 : nEnd;
        if ( nEnd )
            pLWrongs->MarkInvalid( nInv, nEnd );
        else
            pLWrongs->SetValid();
        pRWrongs->SetValid(); // otherwise 0 - 0xFFFF
        pRWrongs->MarkInvalid( 0, 1 );  // Only test the first word
    }

    ParaPortion* pPortion = FindParaPortion( rPaM.GetNode() );
    DBG_ASSERT( pPortion, "Blind Portion in ImpInsertParaBreak" );
    pPortion->MarkInvalid( rPaM.GetIndex(), 0 );

    // Optimization: Do not place unnecessarily many getPos to Listen!
    // Here, as in undo, but also in all other methods.
    USHORT nPos = GetParaPortions().GetPos( pPortion );
    ParaPortion* pNewPortion = new ParaPortion( aPaM.GetNode() );
    GetParaPortions().Insert( pNewPortion, nPos + 1 );
    ParaAttribsChanged( pNewPortion->GetNode() );
    if ( IsCallParaInsertedOrDeleted() )
        GetEditEnginePtr()->ParagraphInserted( nPos+1 );

    CursorMoved( rPaM.GetNode() );  // if empty Attributes have emerged.
    TextModified();
    return aPaM;
}

EditPaM ImpEditEngine::ImpFastInsertParagraph( USHORT nPara )
{
    if ( IsUndoEnabled() && !IsInUndo() )
    {
        if ( nPara )
        {
            DBG_ASSERT( aEditDoc.SaveGetObject( nPara-1 ), "FastInsertParagraph: Prev does not exist" );
            InsertUndo( new EditUndoSplitPara( this, nPara-1, aEditDoc.GetObject( nPara-1 )->Len() ) );
        }
        else
            InsertUndo( new EditUndoSplitPara( this, 0, 0 ) );
    }

    ContentNode* pNode = new ContentNode( aEditDoc.GetItemPool() );
    // If flat mode, then later no Font is set:
    pNode->GetCharAttribs().GetDefFont() = aEditDoc.GetDefFont();

    if ( GetStatus().DoOnlineSpelling() )
        pNode->CreateWrongList();

    aEditDoc.Insert( pNode, nPara );

    ParaPortion* pNewPortion = new ParaPortion( pNode );
    GetParaPortions().Insert( pNewPortion, nPara );
    if ( IsCallParaInsertedOrDeleted() )
        GetEditEnginePtr()->ParagraphInserted( nPara );

    return EditPaM( pNode, 0 );
}

EditPaM ImpEditEngine::InsertParaBreak( EditSelection aCurSel )
{
    EditPaM aPaM( ImpInsertParaBreak( aCurSel ) );
    if ( aStatus.DoAutoIndenting() )
    {
        USHORT nPara = aEditDoc.GetPos( aPaM.GetNode() );
        DBG_ASSERT( nPara > 0, "AutoIndenting: Error!" );
        XubString aPrevParaText( GetEditDoc().GetParaAsString( nPara-1 ) );
        USHORT n = 0;
        while ( ( n < aPrevParaText.Len() ) &&
                ( ( aPrevParaText.GetChar(n) == ' ' ) || ( aPrevParaText.GetChar(n) == '\t' ) ) )
        {
            if ( aPrevParaText.GetChar(n) == '\t' )
                aPaM = ImpInsertFeature( aPaM, SfxVoidItem( EE_FEATURE_TAB ) );
            else
                aPaM = ImpInsertText( aPaM, aPrevParaText.GetChar(n) );
            n++;
        }

    }
    return aPaM;
}

EditPaM ImpEditEngine::InsertTab( EditSelection aCurSel )
{
    EditPaM aPaM( ImpInsertFeature( aCurSel, SfxVoidItem( EE_FEATURE_TAB ) ) );
    return aPaM;
}

EditPaM ImpEditEngine::InsertField( EditSelection aCurSel, const SvxFieldItem& rFld )
{
    EditPaM aPaM( ImpInsertFeature( aCurSel, rFld ) );
    return aPaM;
}

BOOL ImpEditEngine::UpdateFields()
{
    BOOL bChanges = FALSE;
    USHORT nParas = GetEditDoc().Count();
    for ( USHORT nPara = 0; nPara < nParas; nPara++ )
    {
        BOOL bChangesInPara = FALSE;
        ContentNode* pNode = GetEditDoc().GetObject( nPara );
        DBG_ASSERT( pNode, "NULL-Pointer in Doc" );
        CharAttribArray& rAttribs = pNode->GetCharAttribs().GetAttribs();
        for ( USHORT nAttr = 0; nAttr < rAttribs.Count(); nAttr++ )
        {
            EditCharAttrib* pAttr = rAttribs[nAttr];
            if ( pAttr->Which() == EE_FEATURE_FIELD )
            {
                EditCharAttribField* pField = (EditCharAttribField*)pAttr;
                EditCharAttribField* pCurrent = new EditCharAttribField( *pField );
                pField->Reset();

                if ( aStatus.MarkFields() )
                    pField->GetFldColor() = new Color( GetColorConfig().GetColorValue( svtools::WRITERFIELDSHADINGS ).nColor );

                XubString aFldValue = GetEditEnginePtr()->CalcFieldValue(
                                        (const SvxFieldItem&)*pField->GetItem(),
                                        nPara, pField->GetStart(),
                                        pField->GetTxtColor(), pField->GetFldColor() );
                pField->GetFieldValue() = aFldValue;
                if ( *pField != *pCurrent )
                {
                    bChanges = TRUE;
                    bChangesInPara = TRUE;
                }
                delete pCurrent;
            }
        }
        if ( bChangesInPara )
        {
            // If possible be more precise when invalidate.
            ParaPortion* pPortion = GetParaPortions().GetObject( nPara );
            DBG_ASSERT( pPortion, "NULL-Pointer in Doc" );
            pPortion->MarkSelectionInvalid( 0, pNode->Len() );
        }
    }
    return bChanges;
}

EditPaM ImpEditEngine::InsertLineBreak( EditSelection aCurSel )
{
    EditPaM aPaM( ImpInsertFeature( aCurSel, SfxVoidItem( EE_FEATURE_LINEBR ) ) );
    return aPaM;
}

//  ----------------------------------------------------------------------
//  Helper functions
//  ----------------------------------------------------------------------
Rectangle ImpEditEngine::PaMtoEditCursor( EditPaM aPaM, USHORT nFlags )
{
    DBG_ASSERT( GetUpdateMode(), "Must not be reached when Update=FALSE: PaMtoEditCursor" );

    Rectangle aEditCursor;
    long nY = 0;
    for ( USHORT nPortion = 0; nPortion < GetParaPortions().Count(); nPortion++ )
    {
        ParaPortion* pPortion = GetParaPortions().GetObject(nPortion);
        ContentNode* pNode = pPortion->GetNode();
        DBG_ASSERT( pNode, "Invalid Node in Portion!" );
        if ( pNode != aPaM.GetNode() )
        {
            nY += pPortion->GetHeight();
        }
        else
        {
            aEditCursor = GetEditCursor( pPortion, aPaM.GetIndex(), nFlags );
            aEditCursor.Top() += nY;
            aEditCursor.Bottom() += nY;
            return aEditCursor;
        }
    }
    OSL_FAIL( "Portion not found!" );
    return aEditCursor;
}

EditPaM ImpEditEngine::GetPaM( Point aDocPos, BOOL bSmart )
{
    DBG_ASSERT( GetUpdateMode(), "Must not be reached when Update=FALSE: GetPaM" );

    long nY = 0;
    long nTmpHeight;
    EditPaM aPaM;
    USHORT nPortion;
    for ( nPortion = 0; nPortion < GetParaPortions().Count(); nPortion++ )
    {
        ParaPortion* pPortion = GetParaPortions().GetObject(nPortion);
        nTmpHeight = pPortion->GetHeight();     // should also be correct for !bVisible!
        nY += nTmpHeight;
        if ( nY > aDocPos.Y() )
        {
            nY -= nTmpHeight;
            aDocPos.Y() -= nY;
            // Skip invisible Portions:
            while ( pPortion && !pPortion->IsVisible() )
            {
                nPortion++;
                pPortion = GetParaPortions().SaveGetObject( nPortion );
            }
            DBG_ASSERT( pPortion, "No visible paragraph found: GetPaM" );
            aPaM = GetPaM( pPortion, aDocPos, bSmart );
            return aPaM;

        }
    }
    // Then search for the last visible:
    nPortion = GetParaPortions().Count()-1;
    while ( nPortion && !GetParaPortions()[nPortion]->IsVisible() )
        nPortion--;

    DBG_ASSERT( GetParaPortions()[nPortion]->IsVisible(), "No visible paragraph found: GetPaM" );
    aPaM.SetNode( GetParaPortions()[nPortion]->GetNode() );
    aPaM.SetIndex( GetParaPortions()[nPortion]->GetNode()->Len() );
    return aPaM;
}

sal_uInt32 ImpEditEngine::GetTextHeight() const
{
    DBG_ASSERT( GetUpdateMode(), "Should not be used for Update=FALSE: GetTextHeight" );
    DBG_ASSERT( IsFormatted() || IsFormatting(), "GetTextHeight: Not formatted" );
    return nCurTextHeight;
}

sal_uInt32 ImpEditEngine::CalcTextWidth( BOOL bIgnoreExtraSpace )
{
    // If still not formatted and not in the process.
    // Will be brought in the formatting for AutoPageSize.
    if ( !IsFormatted() && !IsFormatting() )
        FormatDoc();

    EditLine* pLine;

    long nMaxWidth = 0;
    long nCurWidth = 0;

    // --------------------------------------------------
    // Over all the paragraphs ...
    // --------------------------------------------------
    USHORT nParas = GetParaPortions().Count();
    for ( USHORT nPara = 0; nPara < nParas; nPara++ )
    {
        ParaPortion* pPortion = GetParaPortions().GetObject( nPara );
        if ( pPortion->IsVisible() )
        {
            const SvxLRSpaceItem& rLRItem = GetLRSpaceItem( pPortion->GetNode() );
            sal_Int32 nSpaceBeforeAndMinLabelWidth = GetSpaceBeforeAndMinLabelWidth( pPortion->GetNode() );

            // --------------------------------------------------
            // On the lines of the paragraph ...
            // --------------------------------------------------
            ULONG nLines = pPortion->GetLines().Count();
            for ( USHORT nLine = 0; nLine < nLines; nLine++ )
            {
                pLine = pPortion->GetLines().GetObject( nLine );
                DBG_ASSERT( pLine, "NULL-Pointer in the line iterator in CalcWidth" );
                // nCurWidth = pLine->GetStartPosX();
                // For Center- or Right- alignment it depends on the paper
                // width, here not prefered. I general, it is best not leave it
                // to StartPosX, also the right indents have to be taken into
                // account!
                nCurWidth = GetXValue( rLRItem.GetTxtLeft() + nSpaceBeforeAndMinLabelWidth );
                if ( nLine == 0 )
                {
                    long nFI = GetXValue( rLRItem.GetTxtFirstLineOfst() );
                    nCurWidth -= nFI;
                    if ( pPortion->GetBulletX() > nCurWidth )
                    {
                        nCurWidth += nFI;   // LI?
                        if ( pPortion->GetBulletX() > nCurWidth )
                            nCurWidth = pPortion->GetBulletX();
                    }
                }
                nCurWidth += GetXValue( rLRItem.GetRight() );
                nCurWidth += CalcLineWidth( pPortion, pLine, bIgnoreExtraSpace );
                if ( nCurWidth > nMaxWidth )
                {
                    nMaxWidth = nCurWidth;
                }
            }
        }
    }
    if ( nMaxWidth < 0 )
        nMaxWidth = 0;

    nMaxWidth++; // widen it, because in CreateLines for >= is wrapped.
    return (sal_uInt32)nMaxWidth;
}

sal_uInt32 ImpEditEngine::CalcLineWidth( ParaPortion* pPortion, EditLine* pLine, BOOL bIgnoreExtraSpace )
{
    USHORT nPara = GetEditDoc().GetPos( pPortion->GetNode() );

    // #114278# Saving both layout mode and language (since I'm
    // potentially changing both)
    GetRefDevice()->Push( PUSH_TEXTLAYOUTMODE|PUSH_TEXTLANGUAGE );

    ImplInitLayoutMode( GetRefDevice(), nPara, 0xFFFF );

    SvxAdjust eJustification = GetJustification( nPara );

    // Calculation of the width without the Indents ...
    sal_uInt32 nWidth = 0;
    USHORT nPos = pLine->GetStart();
    for ( USHORT nTP = pLine->GetStartPortion(); nTP <= pLine->GetEndPortion(); nTP++ )
    {
        TextPortion* pTextPortion = pPortion->GetTextPortions().GetObject( nTP );
        switch ( pTextPortion->GetKind() )
        {
            case PORTIONKIND_FIELD:
            case PORTIONKIND_HYPHENATOR:
            case PORTIONKIND_TAB:
            {
                nWidth += pTextPortion->GetSize().Width();
            }
            break;
            case PORTIONKIND_TEXT:
            {
                if ( ( eJustification != SVX_ADJUST_BLOCK ) || ( !bIgnoreExtraSpace ) )
                {
                    nWidth += pTextPortion->GetSize().Width();
                }
                else
                {
                    SvxFont aTmpFont( pPortion->GetNode()->GetCharAttribs().GetDefFont() );
                    SeekCursor( pPortion->GetNode(), nPos+1, aTmpFont );
                    aTmpFont.SetPhysFont( GetRefDevice() );
                    ImplInitDigitMode( GetRefDevice(), 0, 0, 0, aTmpFont.GetLanguage() );
                    nWidth += aTmpFont.QuickGetTextSize( GetRefDevice(), *pPortion->GetNode(), nPos, pTextPortion->GetLen(), NULL ).Width();
                }
            }
            break;
        }
        nPos = nPos + pTextPortion->GetLen();
    }

    GetRefDevice()->Pop();

    return nWidth;
}

sal_uInt32 ImpEditEngine::CalcTextHeight()
{
    DBG_ASSERT( GetUpdateMode(), "Should not be used when Update=FALSE: CalcTextHeight" );
    sal_uInt32 nY = 0;
    for ( USHORT nPortion = 0; nPortion < GetParaPortions().Count(); nPortion++ )
        nY += GetParaPortions()[nPortion]->GetHeight();
    return nY;
}

USHORT ImpEditEngine::GetLineCount( USHORT nParagraph ) const
{
    DBG_ASSERT( nParagraph < GetParaPortions().Count(), "GetLineCount: Out of range" );
    ParaPortion* pPPortion = GetParaPortions().SaveGetObject( nParagraph );
    DBG_ASSERT( pPPortion, "Paragraph not found: GetLineCount" );
    if ( pPPortion )
        return pPPortion->GetLines().Count();

    return 0xFFFF;
}

xub_StrLen ImpEditEngine::GetLineLen( USHORT nParagraph, USHORT nLine ) const
{
    DBG_ASSERT( nParagraph < GetParaPortions().Count(), "GetLineLen: Out of range" );
    ParaPortion* pPPortion = GetParaPortions().SaveGetObject( nParagraph );
    DBG_ASSERT( pPPortion, "Paragraph not found: GetLineLen" );
    if ( pPPortion && ( nLine < pPPortion->GetLines().Count() ) )
    {
        EditLine* pLine = pPPortion->GetLines().GetObject( nLine );
        DBG_ASSERT( pLine, "Line not found: GetLineHeight" );
        return pLine->GetLen();
    }

    return 0xFFFF;
}

void ImpEditEngine::GetLineBoundaries( /*out*/USHORT &rStart, /*out*/USHORT &rEnd, USHORT nParagraph, USHORT nLine ) const
{
    DBG_ASSERT( nParagraph < GetParaPortions().Count(), "GetLineCount: Out of range" );
    ParaPortion* pPPortion = GetParaPortions().SaveGetObject( nParagraph );
    DBG_ASSERT( pPPortion, "Paragraph not found: GetLineBoundaries" );
    rStart = rEnd = 0xFFFF;     // default values in case of error
    if ( pPPortion && ( nLine < pPPortion->GetLines().Count() ) )
    {
        EditLine* pLine = pPPortion->GetLines().GetObject( nLine );
        DBG_ASSERT( pLine, "Line not found: GetLineBoundaries" );
        rStart = pLine->GetStart();
        rEnd   = pLine->GetEnd();
    }
}

USHORT ImpEditEngine::GetLineNumberAtIndex( USHORT nPara, USHORT nIndex ) const
{
    USHORT nLineNo = 0xFFFF;
    ContentNode* pNode = GetEditDoc().SaveGetObject( nPara );
    DBG_ASSERT( pNode, "GetLineNumberAtIndex: invalid paragraph index" );
    if (pNode)
    {
        // we explicitly allow for the index to point at the character right behind the text
        const bool bValidIndex = /*0 <= nIndex &&*/ nIndex <= pNode->Len();
        DBG_ASSERT( bValidIndex, "GetLineNumberAtIndex: invalid index" );
        const USHORT nLineCount = GetLineCount( nPara );
        if (nIndex == pNode->Len())
            nLineNo = nLineCount > 0 ? nLineCount - 1 : 0;
        else if (bValidIndex)   // nIndex < pNode->Len()
        {
            USHORT nStart = USHRT_MAX, nEnd = USHRT_MAX;
            for (USHORT i = 0;  i < nLineCount && nLineNo == 0xFFFF;  ++i)
            {
                GetLineBoundaries( nStart, nEnd, nPara, i );
                if (nStart <= nIndex && nIndex < nEnd)
                    nLineNo = i;
            }
        }
    }
    return nLineNo;
}

USHORT ImpEditEngine::GetLineHeight( USHORT nParagraph, USHORT nLine )
{
    DBG_ASSERT( nParagraph < GetParaPortions().Count(), "GetLineCount: Out of range" );
    ParaPortion* pPPortion = GetParaPortions().SaveGetObject( nParagraph );
    DBG_ASSERT( pPPortion, "Paragraph not found: GetLineHeight" );
    if ( pPPortion && ( nLine < pPPortion->GetLines().Count() ) )
    {
        EditLine* pLine = pPPortion->GetLines().GetObject( nLine );
        DBG_ASSERT( pLine, "Paragraph not found: GetLineHeight" );
        return pLine->GetHeight();
    }

    return 0xFFFF;
}

sal_uInt32 ImpEditEngine::GetParaHeight( USHORT nParagraph )
{
    sal_uInt32 nHeight = 0;

    ParaPortion* pPPortion = GetParaPortions().SaveGetObject( nParagraph );
    DBG_ASSERT( pPPortion, "Paragraph not found: GetParaHeight" );

    if ( pPPortion )
        nHeight = pPPortion->GetHeight();

    return nHeight;
}

void ImpEditEngine::UpdateSelections()
{
    USHORT nInvNodes = aDeletedNodes.Count();

    // Check whether one of the selections is at a deleted node...
    // If the node is valid, the index has yet to be examined!
    for ( USHORT nView = 0; nView < aEditViews.Count(); nView++ )
    {
        EditView* pView = aEditViews.GetObject(nView);
        DBG_CHKOBJ( pView, EditView, 0 );
        EditSelection aCurSel( pView->pImpEditView->GetEditSelection() );
        BOOL bChanged = FALSE;
        for ( USHORT n = 0; n < nInvNodes; n++ )
        {
            DeletedNodeInfo* pInf = aDeletedNodes.GetObject( n );
            if ( ( ( ULONG )(aCurSel.Min().GetNode()) == pInf->GetInvalidAdress() ) ||
                 ( ( ULONG )(aCurSel.Max().GetNode()) == pInf->GetInvalidAdress() ) )
            {
                // Use ParaPortions, as now also hidden paragraphs have to be
                // taken into account!
                USHORT nPara = pInf->GetPosition();
                ParaPortion* pPPortion = GetParaPortions().SaveGetObject( nPara );
                if ( !pPPortion ) // Last paragraph
                {
                    nPara = GetParaPortions().Count()-1;
                    pPPortion = GetParaPortions().GetObject( nPara );
                }
                DBG_ASSERT( pPPortion, "Empty Document in UpdateSelections ?" );
                // Do not end up from a hidden paragraph:
                USHORT nCurPara = nPara;
                USHORT nLastPara = GetParaPortions().Count()-1;
                while ( nPara <= nLastPara && !GetParaPortions()[nPara]->IsVisible() )
                    nPara++;
                if ( nPara > nLastPara ) // then also backwards ...
                {
                    nPara = nCurPara;
                    while ( nPara && !GetParaPortions()[nPara]->IsVisible() )
                        nPara--;
                }
                DBG_ASSERT( GetParaPortions()[nPara]->IsVisible(), "No visible paragraph found: UpdateSelections" );

                ParaPortion* pParaPortion = GetParaPortions()[nPara];
                EditSelection aTmpSelection( EditPaM( pParaPortion->GetNode(), 0 ) );
                pView->pImpEditView->SetEditSelection( aTmpSelection );
                bChanged=TRUE;
                break;  // for loop
            }
        }
        if ( !bChanged )
        {
            // Check Index if node shrunk.
            if ( aCurSel.Min().GetIndex() > aCurSel.Min().GetNode()->Len() )
            {
                aCurSel.Min().GetIndex() = aCurSel.Min().GetNode()->Len();
                pView->pImpEditView->SetEditSelection( aCurSel );
            }
            if ( aCurSel.Max().GetIndex() > aCurSel.Max().GetNode()->Len() )
            {
                aCurSel.Max().GetIndex() = aCurSel.Max().GetNode()->Len();
                pView->pImpEditView->SetEditSelection( aCurSel );
            }
        }
    }

    // Delete ...
    for ( USHORT n = 0; n < nInvNodes; n++ )
    {
        DeletedNodeInfo* pInf = aDeletedNodes.GetObject( n );
        delete pInf;
    }
    aDeletedNodes.Remove( 0, aDeletedNodes.Count() );
}

EditSelection ImpEditEngine::ConvertSelection( USHORT nStartPara, USHORT nStartPos,
                             USHORT nEndPara, USHORT nEndPos ) const
{
    EditSelection aNewSelection;

    // Start...
    ContentNode* pNode = aEditDoc.SaveGetObject( nStartPara );
    USHORT nIndex = nStartPos;
    if ( !pNode )
    {
        pNode = aEditDoc[ aEditDoc.Count()-1 ];
        nIndex = pNode->Len();
    }
    else if ( nIndex > pNode->Len() )
        nIndex = pNode->Len();

    aNewSelection.Min().SetNode( pNode );
    aNewSelection.Min().SetIndex( nIndex );

    // End...
    pNode = aEditDoc.SaveGetObject( nEndPara );
    nIndex = nEndPos;
    if ( !pNode )
    {
        pNode = aEditDoc[ aEditDoc.Count()-1 ];
        nIndex = pNode->Len();
    }
    else if ( nIndex > pNode->Len() )
        nIndex = pNode->Len();

    aNewSelection.Max().SetNode( pNode );
    aNewSelection.Max().SetIndex( nIndex );

    return aNewSelection;
}

EditSelection ImpEditEngine::MatchGroup( const EditSelection& rSel )
{
    EditSelection aMatchSel;
    EditSelection aTmpSel( rSel );
    aTmpSel.Adjust( GetEditDoc() );
    if ( (  aTmpSel.Min().GetNode() != aTmpSel.Max().GetNode() ) ||
         ( ( aTmpSel.Max().GetIndex() - aTmpSel.Min().GetIndex() ) > 1 ) )
    {
        return aMatchSel;
    }

    USHORT nPos = aTmpSel.Min().GetIndex();
    ContentNode* pNode = aTmpSel.Min().GetNode();
    if ( nPos >= pNode->Len() )
        return aMatchSel;

    USHORT nMatchChar = aGroupChars.Search( pNode->GetChar( nPos ) );
    if ( nMatchChar != STRING_NOTFOUND )
    {
        USHORT nNode = aEditDoc.GetPos( pNode );
        if ( ( nMatchChar % 2 ) == 0 )
        {
            // Search forward...
            xub_Unicode nSC = aGroupChars.GetChar( nMatchChar );
            DBG_ASSERT( aGroupChars.Len() > (nMatchChar+1), "Invalid group of MatchChars!" );
            xub_Unicode nEC = aGroupChars.GetChar( nMatchChar+1 );

            USHORT nCur = aTmpSel.Min().GetIndex()+1;
            USHORT nLevel = 1;
            while ( pNode && nLevel )
            {
                XubString& rStr = *pNode;
                while ( nCur < rStr.Len() )
                {
                    if ( rStr.GetChar( nCur ) == nSC )
                        nLevel++;
                    else if ( rStr.GetChar( nCur ) == nEC )
                    {
                        nLevel--;
                        if ( !nLevel )
                            break;  // while nCur...
                    }
                    nCur++;
                }

                if ( nLevel )
                {
                    nNode++;
                    pNode = nNode < aEditDoc.Count() ? aEditDoc.GetObject( nNode ) : 0;
                    nCur = 0;
                }
            }
            if ( nLevel == 0 )  // found
            {
                aMatchSel.Min() = aTmpSel.Min();
                aMatchSel.Max() = EditPaM( pNode, nCur+1 );
            }
        }
        else
        {
            // Search backwards...
            xub_Unicode nEC = aGroupChars.GetChar( nMatchChar );
            xub_Unicode nSC = aGroupChars.GetChar( nMatchChar-1 );

            USHORT nCur = aTmpSel.Min().GetIndex()-1;
            USHORT nLevel = 1;
            while ( pNode && nLevel )
            {
                if ( pNode->Len() )
                {
                    XubString& rStr = *pNode;
                    while ( nCur )
                    {
                        if ( rStr.GetChar( nCur ) == nSC )
                        {
                            nLevel--;
                            if ( !nLevel )
                                break;  // while nCur...
                        }
                        else if ( rStr.GetChar( nCur ) == nEC )
                            nLevel++;

                        nCur--;
                    }
                }

                if ( nLevel )
                {
                    pNode = nNode ? aEditDoc.GetObject( --nNode ) : 0;
                    if ( pNode )
                        nCur = pNode->Len()-1;  // no matter if negative, because if Len ()
                }
            }

            if ( nLevel == 0 )  // found
            {
                aMatchSel.Min() = aTmpSel.Min();
                aMatchSel.Min().GetIndex()++;   // after the character
                aMatchSel.Max() = EditPaM( pNode, nCur );
            }
        }
    }
    return aMatchSel;
}

void ImpEditEngine::StopSelectionMode()
{
    if ( ( IsInSelectionMode() || aSelEngine.IsInSelection() ) && pActiveView )
    {
        pActiveView->pImpEditView->DrawSelection();
        EditSelection aSel( pActiveView->pImpEditView->GetEditSelection() );
        aSel.Min() = aSel.Max();
        pActiveView->pImpEditView->SetEditSelection( aSel );
        pActiveView->ShowCursor();
        aSelEngine.Reset();
        bInSelection = FALSE;
    }
}

void ImpEditEngine::SetActiveView( EditView* pView )
{
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // Actually, now bHasVisSel and HideSelection would be necessary     !!!

    if ( pView == pActiveView )
        return;

    if ( pActiveView && pActiveView->HasSelection() )
        pActiveView->pImpEditView->DrawSelection();

    pActiveView = pView;

    if ( pActiveView && pActiveView->HasSelection() )
        pActiveView->pImpEditView->DrawSelection();

    //  NN: Quick fix for #78668#:
    //  When editing of a cell in Calc is ended, the edit engine is not deleted,
    //  only the edit views are removed. If mpIMEInfos is still set in that case,
    //  mpIMEInfos->aPos points to an invalid selection.
    //  -> reset mpIMEInfos now
    //  (probably something like this is necessary whenever the content is modified
    //  from the outside)

    if ( !pView && mpIMEInfos )
    {
        delete mpIMEInfos;
        mpIMEInfos = NULL;
    }
}

uno::Reference< datatransfer::XTransferable > ImpEditEngine::CreateTransferable( const EditSelection& rSelection ) const
{
    EditSelection aSelection( rSelection );
    aSelection.Adjust( GetEditDoc() );

    EditDataObject* pDataObj = new EditDataObject;
    uno::Reference< datatransfer::XTransferable > xDataObj;
    xDataObj = pDataObj;

    XubString aText( GetSelected( aSelection ) );
    aText.ConvertLineEnd(); // System specific
    pDataObj->GetString() = aText;

    SvxFontItem::EnableStoreUnicodeNames( TRUE );
    WriteBin( pDataObj->GetStream(), aSelection, TRUE );
    pDataObj->GetStream().Seek( 0 );
    SvxFontItem::EnableStoreUnicodeNames( FALSE );

    ((ImpEditEngine*)this)->WriteRTF( pDataObj->GetRTFStream(), aSelection );
    pDataObj->GetRTFStream().Seek( 0 );

    if ( ( aSelection.Min().GetNode() == aSelection.Max().GetNode() )
            && ( aSelection.Max().GetIndex() == (aSelection.Min().GetIndex()+1) ) )
    {
        const EditCharAttrib* pAttr = aSelection.Min().GetNode()->GetCharAttribs().
            FindFeature( aSelection.Min().GetIndex() );
        if ( pAttr &&
            ( pAttr->GetStart() == aSelection.Min().GetIndex() ) &&
            ( pAttr->Which() == EE_FEATURE_FIELD ) )
        {
            const SvxFieldItem* pField = (const SvxFieldItem*)pAttr->GetItem();
            const SvxFieldData* pFld = pField->GetField();
            if ( pFld && pFld->ISA( SvxURLField ) )
            {
                // Office-Bookmark
                String aURL( ((const SvxURLField*)pFld)->GetURL() );
                String aTxt( ((const SvxURLField*)pFld)->GetRepresentation() );
                pDataObj->GetURL() = aURL;
            }
        }
    }

    return xDataObj;
}

EditSelection ImpEditEngine::InsertText( uno::Reference< datatransfer::XTransferable >& rxDataObj, const String& rBaseURL, const EditPaM& rPaM, BOOL bUseSpecial )
{
    EditSelection aNewSelection( rPaM );

    if ( rxDataObj.is() )
    {
        datatransfer::DataFlavor aFlavor;
        BOOL bDone = FALSE;

        if ( bUseSpecial )
        {
            // BIN
            SotExchange::GetFormatDataFlavor( SOT_FORMATSTR_ID_EDITENGINE, aFlavor );
            if ( rxDataObj->isDataFlavorSupported( aFlavor ) )
            {
                try
                {
                    uno::Any aData = rxDataObj->getTransferData( aFlavor );
                    uno::Sequence< sal_Int8 > aSeq;
                    aData >>= aSeq;
                    {
                        SvMemoryStream aBinStream( aSeq.getArray(), aSeq.getLength(), STREAM_READ );
                        aNewSelection = Read( aBinStream, rBaseURL, EE_FORMAT_BIN, rPaM );
                    }
                    bDone = TRUE;
                }
                catch( const ::com::sun::star::uno::Exception& )
                {
                }
            }

            if ( !bDone )
            {
                // RTF
                SotExchange::GetFormatDataFlavor( SOT_FORMAT_RTF, aFlavor );
                if ( rxDataObj->isDataFlavorSupported( aFlavor ) )
                {
                    try
                    {
                        uno::Any aData = rxDataObj->getTransferData( aFlavor );
                        uno::Sequence< sal_Int8 > aSeq;
                        aData >>= aSeq;
                        {
                            SvMemoryStream aRTFStream( aSeq.getArray(), aSeq.getLength(), STREAM_READ );
                            aNewSelection = Read( aRTFStream, rBaseURL, EE_FORMAT_RTF, rPaM );
                        }
                        bDone = TRUE;
                    }
                    catch( const ::com::sun::star::uno::Exception& )
                    {
                    }
                }
            }
            if ( !bDone )
            {
                // XML ?
                // Currently, there is nothing like "The" XML format, StarOffice doesn't offer plain XML in Clipboard...
            }
        }
        if ( !bDone )
        {
            SotExchange::GetFormatDataFlavor( SOT_FORMAT_STRING, aFlavor );
            if ( rxDataObj->isDataFlavorSupported( aFlavor ) )
            {
                try
                {
                    uno::Any aData = rxDataObj->getTransferData( aFlavor );
                    ::rtl::OUString aText;
                    aData >>= aText;
                    aNewSelection = ImpInsertText( rPaM, aText );
                       bDone = TRUE;
                }
                catch( ... )
                {
                    ; // #i9286# can happen, even if isDataFlavorSupported returns true...
                }
            }
        }
    }

    return aNewSelection;
}

Range ImpEditEngine::GetInvalidYOffsets( ParaPortion* pPortion )
{
    Range aRange( 0, 0 );

    if ( pPortion->IsVisible() )
    {
        const SvxULSpaceItem& rULSpace = (const SvxULSpaceItem&)pPortion->GetNode()->GetContentAttribs().GetItem( EE_PARA_ULSPACE );
        const SvxLineSpacingItem& rLSItem = (const SvxLineSpacingItem&)pPortion->GetNode()->GetContentAttribs().GetItem( EE_PARA_SBL );
        USHORT nSBL = ( rLSItem.GetInterLineSpaceRule() == SVX_INTER_LINE_SPACE_FIX )
                            ? GetYValue( rLSItem.GetInterLineSpace() ) : 0;

        // only from the top ...
        USHORT nFirstInvalid = 0xFFFF;
        USHORT nLine;
        for ( nLine = 0; nLine < pPortion->GetLines().Count(); nLine++ )
        {
            EditLine* pL = pPortion->GetLines().GetObject( nLine );
            if ( pL->IsInvalid() )
            {
                nFirstInvalid = nLine;
                break;
            }
            if ( nLine && !aStatus.IsOutliner() )   // not the first line
                aRange.Min() += nSBL;
            aRange.Min() += pL->GetHeight();
        }
        DBG_ASSERT( nFirstInvalid != 0xFFFF, "No invalid line found in GetInvalidYOffset(1)" );


        // Syndicate and more ...
        aRange.Max() = aRange.Min();
        aRange.Max() += pPortion->GetFirstLineOffset();
        if ( nFirstInvalid != 0 )   // Only if the first line is invalid
            aRange.Min() = aRange.Max();

        USHORT nLastInvalid = pPortion->GetLines().Count()-1;
        for ( nLine = nFirstInvalid; nLine < pPortion->GetLines().Count(); nLine++ )
        {
            EditLine* pL = pPortion->GetLines().GetObject( nLine );
            if ( pL->IsValid() )
            {
                nLastInvalid = nLine;
                break;
            }

            if ( nLine && !aStatus.IsOutliner() )
                aRange.Max() += nSBL;
            aRange.Max() += pL->GetHeight();
        }

        if( ( rLSItem.GetInterLineSpaceRule() == SVX_INTER_LINE_SPACE_PROP ) && rLSItem.GetPropLineSpace() &&
            ( rLSItem.GetPropLineSpace() < 100 ) )
        {
            EditLine* pL = pPortion->GetLines().GetObject( nFirstInvalid );
            long n = pL->GetTxtHeight() * ( 100 - rLSItem.GetPropLineSpace() );
            n /= 100;
            aRange.Min() -= n;
            aRange.Max() += n;
        }

        if ( ( nLastInvalid == pPortion->GetLines().Count()-1 ) && ( !aStatus.IsOutliner() ) )
            aRange.Max() += GetYValue( rULSpace.GetLower() );
    }
    return aRange;
}

EditPaM ImpEditEngine::GetPaM( ParaPortion* pPortion, Point aDocPos, BOOL bSmart )
{
    DBG_ASSERT( pPortion->IsVisible(), "Why GetPaM() for an invisible paragraph?" );
    DBG_ASSERT( IsFormatted(), "GetPaM: Not formatted" );

    USHORT nCurIndex = 0;
    EditPaM aPaM;
    aPaM.SetNode( pPortion->GetNode() );

    const SvxLineSpacingItem& rLSItem = (const SvxLineSpacingItem&)pPortion->GetNode()->GetContentAttribs().GetItem( EE_PARA_SBL );
    USHORT nSBL = ( rLSItem.GetInterLineSpaceRule() == SVX_INTER_LINE_SPACE_FIX )
                        ? GetYValue( rLSItem.GetInterLineSpace() ) : 0;

    long nY = pPortion->GetFirstLineOffset();

    DBG_ASSERT( pPortion->GetLines().Count(), "Empty ParaPortion in GetPaM!" );

    EditLine* pLine = 0;
    for ( USHORT nLine = 0; nLine < pPortion->GetLines().Count(); nLine++ )
    {
        EditLine* pTmpLine = pPortion->GetLines().GetObject( nLine );
        nY += pTmpLine->GetHeight();
        if ( !aStatus.IsOutliner() )
            nY += nSBL;
        if ( nY > aDocPos.Y() )
        {
            pLine = pTmpLine;
            break;                  // correct Y-position is not of interest
        }

        nCurIndex = nCurIndex + pTmpLine->GetLen();
    }

    if ( !pLine ) // may happen only in the range of SA!
    {
        #ifdef DBG_UTIL
         const SvxULSpaceItem& rULSpace =(const SvxULSpaceItem&)pPortion->GetNode()->GetContentAttribs().GetItem( EE_PARA_ULSPACE );
         DBG_ASSERT( nY+GetYValue( rULSpace.GetLower() ) >= aDocPos.Y() , "Index in no line, GetPaM ?" );
        #endif
        aPaM.SetIndex( pPortion->GetNode()->Len() );
        return aPaM;
    }

    // If no line found, only just X-Position => Index
    nCurIndex = GetChar( pPortion, pLine, aDocPos.X(), bSmart );
    aPaM.SetIndex( nCurIndex );

    if ( nCurIndex && ( nCurIndex == pLine->GetEnd() ) &&
         ( pLine != pPortion->GetLines().GetObject( pPortion->GetLines().Count()-1) ) )
    {
        aPaM = CursorLeft( aPaM, ::com::sun::star::i18n::CharacterIteratorMode::SKIPCELL );
    }

    return aPaM;
}

USHORT ImpEditEngine::GetChar( ParaPortion* pParaPortion, EditLine* pLine, long nXPos, BOOL bSmart )
{
    DBG_ASSERT( pLine, "No line received: GetChar" );

    USHORT nChar = 0xFFFF;
    USHORT nCurIndex = pLine->GetStart();


    // Search best matching portion with GetPortionXOffset()
    for ( USHORT i = pLine->GetStartPortion(); i <= pLine->GetEndPortion(); i++ )
    {
        TextPortion* pPortion = pParaPortion->GetTextPortions().GetObject( i );
        long nXLeft = GetPortionXOffset( pParaPortion, pLine, i );
        long nXRight = nXLeft + pPortion->GetSize().Width();
        if ( ( nXLeft <= nXPos ) && ( nXRight >= nXPos ) )
        {
             nChar = nCurIndex;

            // Search within Portion...

            // Don't search within special portions...
            if ( pPortion->GetKind() != PORTIONKIND_TEXT )
            {
                // ...but check on which side
                if ( bSmart )
                {
                    long nLeftDiff = nXPos-nXLeft;
                    long nRightDiff = nXRight-nXPos;
                    if ( nRightDiff < nLeftDiff )
                        nChar++;
                }
            }
            else
            {
                USHORT nMax = pPortion->GetLen();
                USHORT nOffset = 0xFFFF;
                USHORT nTmpCurIndex = nChar - pLine->GetStart();

                long nXInPortion = nXPos - nXLeft;
                if ( pPortion->IsRightToLeft() )
                    nXInPortion = nXRight - nXPos;

                // Search in Array...
                for ( USHORT x = 0; x < nMax; x++ )
                {
                    long nTmpPosMax = pLine->GetCharPosArray().GetObject( nTmpCurIndex+x );
                    if ( nTmpPosMax > nXInPortion )
                    {
                        // Check whether this or the previous...
                        long nTmpPosMin = x ? pLine->GetCharPosArray().GetObject( nTmpCurIndex+x-1 ) : 0;
                        long nDiffLeft = nXInPortion - nTmpPosMin;
                        long nDiffRight = nTmpPosMax - nXInPortion;
                        DBG_ASSERT( nDiffLeft >= 0, "DiffLeft negative" );
                        DBG_ASSERT( nDiffRight >= 0, "DiffRight negative" );
                        nOffset = ( bSmart && ( nDiffRight < nDiffLeft ) ) ? x+1 : x;
                        // I18N: If there are character position with the length of 0,
                        // they belong to the same character, we can not use this position as an index.
                        // Skip all 0-positions, cheaper than using XBreakIterator:
                        if ( nOffset < nMax )
                        {
                            const long nX = pLine->GetCharPosArray().GetObject(nOffset);
                            while ( ( (nOffset+1) < nMax ) && ( pLine->GetCharPosArray().GetObject(nOffset+1) == nX ) )
                                nOffset++;
                        }
                        break;
                    }
                }

                // There should not be any inaccuracies when using the
                // CharPosArray! Maybe for kerning?
                // 0xFFF happens for example for Outline-Font when at the very end.
                if ( nOffset == 0xFFFF )
                    nOffset = nMax;

                DBG_ASSERT( nOffset <= nMax, "nOffset > nMax" );

                nChar = nChar + nOffset;

                // Check if index is within a cell:
                if ( nChar && ( nChar < pParaPortion->GetNode()->Len() ) )
                {
                    EditPaM aPaM( pParaPortion->GetNode(), nChar+1 );
                    USHORT nScriptType = GetScriptType( aPaM );
                    if ( nScriptType == i18n::ScriptType::COMPLEX )
                    {
                        uno::Reference < i18n::XBreakIterator > _xBI( ImplGetBreakIterator() );
                        sal_Int32 nCount = 1;
                        lang::Locale aLocale = GetLocale( aPaM );
                        USHORT nRight = (USHORT)_xBI->nextCharacters( *pParaPortion->GetNode(), nChar, aLocale, ::com::sun::star::i18n::CharacterIteratorMode::SKIPCELL, nCount, nCount );
                        USHORT nLeft = (USHORT)_xBI->previousCharacters( *pParaPortion->GetNode(), nRight, aLocale, ::com::sun::star::i18n::CharacterIteratorMode::SKIPCELL, nCount, nCount );
                        if ( ( nLeft != nChar ) && ( nRight != nChar ) )
                        {
                            nChar = ( Abs( nRight - nChar ) < Abs( nLeft - nChar ) ) ? nRight : nLeft;
                        }
                    }
                }
            }
        }

        nCurIndex = nCurIndex + pPortion->GetLen();
    }

    if ( nChar == 0xFFFF )
    {
        nChar = ( nXPos <= pLine->GetStartPosX() ) ? pLine->GetStart() : pLine->GetEnd();
    }

    return nChar;
}

Range ImpEditEngine::GetLineXPosStartEnd( ParaPortion* pParaPortion, EditLine* pLine )
{
    Range aLineXPosStartEnd;

    USHORT nPara = GetEditDoc().GetPos( pParaPortion->GetNode() );
    if ( !IsRightToLeft( nPara ) )
    {
        aLineXPosStartEnd.Min() = pLine->GetStartPosX();
        aLineXPosStartEnd.Max() = pLine->GetStartPosX() + pLine->GetTextWidth();
    }
    else
    {
        aLineXPosStartEnd.Min() = GetPaperSize().Width() - ( pLine->GetStartPosX() + pLine->GetTextWidth() );
        aLineXPosStartEnd.Max() = GetPaperSize().Width() - pLine->GetStartPosX();
    }


    return aLineXPosStartEnd;
}

long ImpEditEngine::GetPortionXOffset( ParaPortion* pParaPortion, EditLine* pLine, USHORT nTextPortion )
{
    long nX = pLine->GetStartPosX();

    for ( USHORT i = pLine->GetStartPortion(); i < nTextPortion; i++ )
    {
        TextPortion* pPortion = pParaPortion->GetTextPortions().GetObject( i );
        switch ( pPortion->GetKind() )
        {
            case PORTIONKIND_FIELD:
            case PORTIONKIND_TEXT:
            case PORTIONKIND_HYPHENATOR:
            case PORTIONKIND_TAB:
            {
                nX += pPortion->GetSize().Width();
            }
            break;
        }
    }

    USHORT nPara = GetEditDoc().GetPos( pParaPortion->GetNode() );
    BOOL bR2LPara = IsRightToLeft( nPara );

    TextPortion* pDestPortion = pParaPortion->GetTextPortions().GetObject( nTextPortion );
    if ( pDestPortion->GetKind() != PORTIONKIND_TAB )
    {
        if ( !bR2LPara && pDestPortion->GetRightToLeft() )
        {
            // Portions behind must be added, visual before this portion
            sal_uInt16 nTmpPortion = nTextPortion+1;
            while ( nTmpPortion <= pLine->GetEndPortion() )
            {
                TextPortion* pNextTextPortion = pParaPortion->GetTextPortions().GetObject( nTmpPortion );
                if ( pNextTextPortion->GetRightToLeft() && ( pNextTextPortion->GetKind() != PORTIONKIND_TAB ) )
                    nX += pNextTextPortion->GetSize().Width();
                else
                    break;
                nTmpPortion++;
            }
            // Portions before must be removed, visual behind this portion
            nTmpPortion = nTextPortion;
            while ( nTmpPortion > pLine->GetStartPortion() )
            {
                --nTmpPortion;
                TextPortion* pPrevTextPortion = pParaPortion->GetTextPortions().GetObject( nTmpPortion );
                if ( pPrevTextPortion->GetRightToLeft() && ( pPrevTextPortion->GetKind() != PORTIONKIND_TAB ) )
                    nX -= pPrevTextPortion->GetSize().Width();
                else
                    break;
            }
        }
        else if ( bR2LPara && !pDestPortion->IsRightToLeft() )
        {
            // Portions behind must be removed, visual behind this portion
            sal_uInt16 nTmpPortion = nTextPortion+1;
            while ( nTmpPortion <= pLine->GetEndPortion() )
            {
                TextPortion* pNextTextPortion = pParaPortion->GetTextPortions().GetObject( nTmpPortion );
                if ( !pNextTextPortion->IsRightToLeft() && ( pNextTextPortion->GetKind() != PORTIONKIND_TAB ) )
                    nX += pNextTextPortion->GetSize().Width();
                else
                    break;
                nTmpPortion++;
            }
            // Portions before must be added, visual before this portion
            nTmpPortion = nTextPortion;
            while ( nTmpPortion > pLine->GetStartPortion() )
            {
                --nTmpPortion;
                TextPortion* pPrevTextPortion = pParaPortion->GetTextPortions().GetObject( nTmpPortion );
                if ( !pPrevTextPortion->IsRightToLeft() && ( pPrevTextPortion->GetKind() != PORTIONKIND_TAB ) )
                    nX -= pPrevTextPortion->GetSize().Width();
                else
                    break;
            }
        }
    }
    if ( bR2LPara )
    {
        // Switch X postions...
        DBG_ASSERT( GetTextRanger() || GetPaperSize().Width(), "GetPortionXOffset - paper size?!" );
        DBG_ASSERT( GetTextRanger() || (nX <= GetPaperSize().Width()), "GetPortionXOffset - position out of paper size!" );
        nX = GetPaperSize().Width() - nX;
        nX -= pDestPortion->GetSize().Width();
    }

    return nX;
}

long ImpEditEngine::GetXPos( ParaPortion* pParaPortion, EditLine* pLine, USHORT nIndex, BOOL bPreferPortionStart )
{
    DBG_ASSERT( pLine, "No line received: GetXPos" );
    DBG_ASSERT( ( nIndex >= pLine->GetStart() ) && ( nIndex <= pLine->GetEnd() ) , "GetXPos has to be called properly!" );

    BOOL bDoPreferPortionStart = bPreferPortionStart;
    // Assure that the portion belongs to this line:
    if ( nIndex == pLine->GetStart() )
        bDoPreferPortionStart = TRUE;
    else if ( nIndex == pLine->GetEnd() )
        bDoPreferPortionStart = FALSE;

    USHORT nTextPortionStart = 0;
    USHORT nTextPortion = pParaPortion->GetTextPortions().FindPortion( nIndex, nTextPortionStart, bDoPreferPortionStart );

    DBG_ASSERT( ( nTextPortion >= pLine->GetStartPortion() ) && ( nTextPortion <= pLine->GetEndPortion() ), "GetXPos: Portion not in current line! " );

    TextPortion* pPortion = pParaPortion->GetTextPortions().GetObject( nTextPortion );

    long nX = GetPortionXOffset( pParaPortion, pLine, nTextPortion );

    // calc text width, portion size may include CJK/CTL spacing...
    // But the array migh not be init yet, if using text ranger this method is called within CreateLines()...
    long nPortionTextWidth = pPortion->GetSize().Width();
    if ( ( pPortion->GetKind() == PORTIONKIND_TEXT ) && pPortion->GetLen() && !GetTextRanger() )
        nPortionTextWidth = pLine->GetCharPosArray().GetObject( nTextPortionStart + pPortion->GetLen() - 1 - pLine->GetStart() );

    if ( nTextPortionStart != nIndex )
    {
        // Search within portion...
        if ( nIndex == ( nTextPortionStart + pPortion->GetLen() ) )
        {
            // End of Portion
            if ( pPortion->GetKind() == PORTIONKIND_TAB )
            {
                if ( (nTextPortion+1) < pParaPortion->GetTextPortions().Count() )
                {
                    TextPortion* pNextPortion = pParaPortion->GetTextPortions().GetObject( nTextPortion+1 );
                    if ( pNextPortion->GetKind() != PORTIONKIND_TAB )
                    {
                        if ( !bPreferPortionStart )
                            nX = GetXPos( pParaPortion, pLine, nIndex, TRUE );
                        else if ( !IsRightToLeft( GetEditDoc().GetPos( pParaPortion->GetNode() ) ) )
                            nX += nPortionTextWidth;
                    }
                }
                else if ( !IsRightToLeft( GetEditDoc().GetPos( pParaPortion->GetNode() ) ) )
                {
                    nX += nPortionTextWidth;
                }
            }
            else if ( !pPortion->IsRightToLeft() )
            {
                nX += nPortionTextWidth;
            }
        }
        else if ( pPortion->GetKind() == PORTIONKIND_TEXT )
        {
            DBG_ASSERT( nIndex != pLine->GetStart(), "Strange behavior in new GetXPos()" );
            DBG_ASSERT( pLine && pLine->GetCharPosArray().Count(), "svx::ImpEditEngine::GetXPos(), portion in an empty line?" );

            if( pLine->GetCharPosArray().Count() )
            {
                USHORT nPos = nIndex - 1 - pLine->GetStart();
                if( nPos >= pLine->GetCharPosArray().Count() )
                {
                    nPos = pLine->GetCharPosArray().Count()-1;
                    OSL_FAIL("svx::ImpEditEngine::GetXPos(), index out of range!");
                }

                // old code restored see #i112788 (which leaves #i74188 unfixed again)
                long nPosInPortion = pLine->GetCharPosArray().GetObject( nPos );

                if ( !pPortion->IsRightToLeft() )
                {
                    nX += nPosInPortion;
                }
                else
                {
                    nX += nPortionTextWidth - nPosInPortion;
                }

                if ( pPortion->GetExtraInfos() && pPortion->GetExtraInfos()->bCompressed )
                {
                    nX += pPortion->GetExtraInfos()->nPortionOffsetX;
                    if ( pPortion->GetExtraInfos()->nAsianCompressionTypes & CHAR_PUNCTUATIONRIGHT )
                    {
                        BYTE nType = GetCharTypeForCompression( pParaPortion->GetNode()->GetChar( nIndex ) );
                        if ( nType == CHAR_PUNCTUATIONRIGHT )
                        {
                            USHORT n = nIndex - nTextPortionStart;
                            const sal_Int32* pDXArray = pLine->GetCharPosArray().GetData()+( nTextPortionStart-pLine->GetStart() );
                            sal_Int32 nCharWidth = ( ( (n+1) < pPortion->GetLen() ) ? pDXArray[n] : pPortion->GetSize().Width() )
                                                            - ( n ? pDXArray[n-1] : 0 );
                            if ( (n+1) < pPortion->GetLen() )
                            {
                                // smaller, when char behind is CHAR_PUNCTUATIONRIGHT also
                                nType = GetCharTypeForCompression( pParaPortion->GetNode()->GetChar( nIndex+1 ) );
                                if ( nType == CHAR_PUNCTUATIONRIGHT )
                                {
                                    sal_Int32 nNextCharWidth = ( ( (n+2) < pPortion->GetLen() ) ? pDXArray[n+1] : pPortion->GetSize().Width() )
                                                                    - pDXArray[n];
                                    sal_Int32 nCompressed = nNextCharWidth/2;
                                    nCompressed *= pPortion->GetExtraInfos()->nMaxCompression100thPercent;
                                    nCompressed /= 10000;
                                    nCharWidth += nCompressed;
                                }
                            }
                            else
                            {
                                nCharWidth *= 2;    // last char pos to portion end is only compressed size
                            }
                            nX += nCharWidth/2; // 50% compression
                        }
                    }
                }
            }
        }
    }
    else // if ( nIndex == pLine->GetStart() )
    {
        if ( pPortion->IsRightToLeft() )
        {
            nX += nPortionTextWidth;
        }
    }

    return nX;
}

void ImpEditEngine::CalcHeight( ParaPortion* pPortion )
{
    pPortion->nHeight = 0;
    pPortion->nFirstLineOffset = 0;

    if ( pPortion->IsVisible() )
    {
        DBG_ASSERT( pPortion->GetLines().Count(), "Paragraph with no lines in ParaPortion::CalcHeight" );
        for ( USHORT nLine = 0; nLine < pPortion->GetLines().Count(); nLine++ )
            pPortion->nHeight += pPortion->GetLines().GetObject( nLine )->GetHeight();

        if ( !aStatus.IsOutliner() )
        {
            const SvxULSpaceItem& rULItem = (const SvxULSpaceItem&)pPortion->GetNode()->GetContentAttribs().GetItem( EE_PARA_ULSPACE );
            const SvxLineSpacingItem& rLSItem = (const SvxLineSpacingItem&)pPortion->GetNode()->GetContentAttribs().GetItem( EE_PARA_SBL );
            USHORT nSBL = ( rLSItem.GetInterLineSpaceRule() == SVX_INTER_LINE_SPACE_FIX ) ? GetYValue( rLSItem.GetInterLineSpace() ) : 0;

            if ( nSBL )
            {
                if ( pPortion->GetLines().Count() > 1 )
                    pPortion->nHeight += ( pPortion->GetLines().Count() - 1 ) * nSBL;
                if ( aStatus.ULSpaceSummation() )
                    pPortion->nHeight += nSBL;
            }

            USHORT nPortion = GetParaPortions().GetPos( pPortion );
            if ( nPortion || aStatus.ULSpaceFirstParagraph() )
            {
                USHORT nUpper = GetYValue( rULItem.GetUpper() );
                pPortion->nHeight += nUpper;
                pPortion->nFirstLineOffset = nUpper;
            }

            if ( ( nPortion != (GetParaPortions().Count()-1) ) )
            {
                pPortion->nHeight += GetYValue( rULItem.GetLower() );   // not in the last
            }


            if ( nPortion && !aStatus.ULSpaceSummation() )
            {
                ParaPortion* pPrev = GetParaPortions().SaveGetObject( nPortion-1 );
                const SvxULSpaceItem& rPrevULItem = (const SvxULSpaceItem&)pPrev->GetNode()->GetContentAttribs().GetItem( EE_PARA_ULSPACE );
                const SvxLineSpacingItem& rPrevLSItem = (const SvxLineSpacingItem&)pPrev->GetNode()->GetContentAttribs().GetItem( EE_PARA_SBL );

                // In realation between WinWord6/Writer3:
                // With a proportional line spacing the paragraph spacing is
                // also manipulated.
                // Only Writer3: Do not add up, but minimum distance.

                // check if distance by LineSpacing > Upper:
                USHORT nExtraSpace = GetYValue( lcl_CalcExtraSpace( pPortion, rLSItem ) );
                if ( nExtraSpace > pPortion->nFirstLineOffset )
                {
                    // Paragraph becomes 'bigger':
                    pPortion->nHeight += ( nExtraSpace - pPortion->nFirstLineOffset );
                    pPortion->nFirstLineOffset = nExtraSpace;
                }

                // Determine nFirstLineOffset now f(pNode) => now f(pNode, pPrev):
                USHORT nPrevLower = GetYValue( rPrevULItem.GetLower() );

                // This PrevLower is still in the height of PrevPortion ...
                if ( nPrevLower > pPortion->nFirstLineOffset )
                {
                    // Paragraph is 'small':
                    pPortion->nHeight -= pPortion->nFirstLineOffset;
                    pPortion->nFirstLineOffset = 0;
                }
                else if ( nPrevLower )
                {
                    // Paragraph becomes 'somewhat smaller':
                    pPortion->nHeight -= nPrevLower;
                    pPortion->nFirstLineOffset =
                        pPortion->nFirstLineOffset - nPrevLower;
                }
                // I find it not so good, but Writer3 feature:
                // Check if distance by LineSpacing > Lower: this value is not
                // stuck in the height of PrevPortion.
                if ( !pPrev->IsInvalid() )
                {
                    nExtraSpace = GetYValue( lcl_CalcExtraSpace( pPrev, rPrevLSItem ) );
                    if ( nExtraSpace > nPrevLower )
                    {
                        USHORT nMoreLower = nExtraSpace - nPrevLower;
                        // Paragraph becomes 'bigger', 'grows' downwards:
                        if ( nMoreLower > pPortion->nFirstLineOffset )
                        {
                            pPortion->nHeight += ( nMoreLower - pPortion->nFirstLineOffset );
                            pPortion->nFirstLineOffset = nMoreLower;
                        }
                    }
                }
            }
        }
    }
}

Rectangle ImpEditEngine::GetEditCursor( ParaPortion* pPortion, USHORT nIndex, USHORT nFlags )
{
    DBG_ASSERT( pPortion->IsVisible(), "Why GetEditCursor() for an invisible paragraph?" );
    DBG_ASSERT( IsFormatted() || GetTextRanger(), "GetEditCursor: Not formatted" );

    /*
     GETCRSR_ENDOFLINE: If after the last character of a wrapped line, remaining
     at the end of the line, not the beginning of the next one.
     Purpose:   - END => really after the last character
                - Selection....
    */

    long nY = pPortion->GetFirstLineOffset();

    const SvxLineSpacingItem& rLSItem = (const SvxLineSpacingItem&)pPortion->GetNode()->GetContentAttribs().GetItem( EE_PARA_SBL );
    USHORT nSBL = ( rLSItem.GetInterLineSpaceRule() == SVX_INTER_LINE_SPACE_FIX )
                        ? GetYValue( rLSItem.GetInterLineSpace() ) : 0;

    USHORT nCurIndex = 0;
    DBG_ASSERT( pPortion->GetLines().Count(), "Empty ParaPortion in GetEditCursor!" );
    EditLine* pLine = 0;
    BOOL bEOL = ( nFlags & GETCRSR_ENDOFLINE ) ? TRUE : FALSE;
    for ( USHORT nLine = 0; nLine < pPortion->GetLines().Count(); nLine++ )
    {
        EditLine* pTmpLine = pPortion->GetLines().GetObject( nLine );
        if ( ( pTmpLine->GetStart() == nIndex ) || ( pTmpLine->IsIn( nIndex, bEOL ) ) )
        {
            pLine = pTmpLine;
            break;
        }

        nCurIndex = nCurIndex + pTmpLine->GetLen();
        nY += pTmpLine->GetHeight();
        if ( !aStatus.IsOutliner() )
            nY += nSBL;
    }
    if ( !pLine )
    {
        // Cursor at the End of the paragraph.
        DBG_ASSERT( nIndex == nCurIndex, "Index dead wrong in GetEditCursor!" );

        pLine = pPortion->GetLines().GetObject( pPortion->GetLines().Count()-1 );
        nY -= pLine->GetHeight();
        if ( !aStatus.IsOutliner() )
            nY -= nSBL;
        nCurIndex = nCurIndex -  pLine->GetLen();
    }

    Rectangle aEditCursor;

    aEditCursor.Top() = nY;
    nY += pLine->GetHeight();
    aEditCursor.Bottom() = nY-1;

    // Search within the line...
    long nX;

    if ( ( nIndex == pLine->GetStart() ) && ( nFlags & GETCRSR_STARTOFLINE ) )
    {
        Range aXRange = GetLineXPosStartEnd( pPortion, pLine );
        nX = !IsRightToLeft( GetEditDoc().GetPos( pPortion->GetNode() ) ) ? aXRange.Min() : aXRange.Max();
    }
    else if ( ( nIndex == pLine->GetEnd() ) && ( nFlags & GETCRSR_ENDOFLINE ) )
    {
        Range aXRange = GetLineXPosStartEnd( pPortion, pLine );
        nX = !IsRightToLeft( GetEditDoc().GetPos( pPortion->GetNode() ) ) ? aXRange.Max() : aXRange.Min();
    }
    else
    {
        nX = GetXPos( pPortion, pLine, nIndex, ( nFlags & GETCRSR_PREFERPORTIONSTART ) ? TRUE : FALSE );
    }

    aEditCursor.Left() = aEditCursor.Right() = nX;

    if ( nFlags & GETCRSR_TXTONLY )
        aEditCursor.Top() = aEditCursor.Bottom() - pLine->GetTxtHeight() + 1;
    else
        aEditCursor.Top() = aEditCursor.Bottom() - Min( pLine->GetTxtHeight(), pLine->GetHeight() ) + 1;

    return aEditCursor;
}

void ImpEditEngine::SetValidPaperSize( const Size& rNewSz )
{
    aPaperSize = rNewSz;

    long nMinWidth = aStatus.AutoPageWidth() ? aMinAutoPaperSize.Width() : 0;
    long nMaxWidth = aStatus.AutoPageWidth() ? aMaxAutoPaperSize.Width() : 0x7FFFFFFF;
    long nMinHeight = aStatus.AutoPageHeight() ? aMinAutoPaperSize.Height() : 0;
    long nMaxHeight = aStatus.AutoPageHeight() ? aMaxAutoPaperSize.Height() : 0x7FFFFFFF;

    // Minimum/Maximum width:
    if ( aPaperSize.Width() < nMinWidth )
        aPaperSize.Width() = nMinWidth;
    else if ( aPaperSize.Width() > nMaxWidth )
        aPaperSize.Width() = nMaxWidth;

    // Minimum/Maximum height:
    if ( aPaperSize.Height() < nMinHeight )
        aPaperSize.Height() = nMinHeight;
    else if ( aPaperSize.Height() > nMaxHeight )
        aPaperSize.Height() = nMaxHeight;
}

void ImpEditEngine::IndentBlock( EditView* pEditView, BOOL bRight )
{
    ESelection aESel( CreateESel( pEditView->pImpEditView->GetEditSelection() ) );
    aESel.Adjust();

    // Only if more selected Paragraphs ...
    if ( aESel.nEndPara > aESel.nStartPara )
    {
        ESelection aNewSel = aESel;
        aNewSel.nStartPos = 0;
        aNewSel.nEndPos = 0xFFFF;

        if ( aESel.nEndPos == 0 )
        {
            aESel.nEndPara--;       // then not this paragraph ...
            aNewSel.nEndPos = 0;
        }

        pEditView->pImpEditView->DrawSelection();
        pEditView->pImpEditView->SetEditSelection(
                        pEditView->pImpEditView->GetEditSelection().Max() );
        UndoActionStart( bRight ? EDITUNDO_INDENTBLOCK : EDITUNDO_UNINDENTBLOCK );

        for ( USHORT nPara = aESel.nStartPara; nPara <= aESel.nEndPara; nPara++ )
        {
            ContentNode* pNode = GetEditDoc().GetObject( nPara );
            if ( bRight )
            {
                // Insert Tabs
                EditPaM aPaM( pNode, 0 );
                InsertTab( aPaM );
            }
            else
            {
                // Remove Tabs
                EditCharAttrib* pFeature = pNode->GetCharAttribs().FindFeature( 0 );
                if ( pFeature && ( pFeature->GetStart() == 0 ) &&
                   ( pFeature->GetItem()->Which() == EE_FEATURE_TAB ) )
                {
                    EditPaM aStartPaM( pNode, 0 );
                    EditPaM aEndPaM( pNode, 1 );
                    ImpDeleteSelection( EditSelection( aStartPaM, aEndPaM ) );
                }
            }
        }

        UndoActionEnd( bRight ? EDITUNDO_INDENTBLOCK : EDITUNDO_UNINDENTBLOCK );
        UpdateSelections();
        FormatAndUpdate( pEditView );

        ContentNode* pLastNode = GetEditDoc().GetObject( aNewSel.nEndPara );
        if ( pLastNode->Len() < aNewSel.nEndPos )
            aNewSel.nEndPos = pLastNode->Len();
        pEditView->pImpEditView->SetEditSelection( CreateSel( aNewSel ) );
        pEditView->pImpEditView->DrawSelection();
        pEditView->pImpEditView->ShowCursor( FALSE, TRUE );
    }
}

rtl::Reference<SvxForbiddenCharactersTable> ImpEditEngine::GetForbiddenCharsTable( BOOL bGetInternal ) const
{
    rtl::Reference<SvxForbiddenCharactersTable> xF = xForbiddenCharsTable;
    if ( !xF.is() && bGetInternal )
        xF = EE_DLL()->GetGlobalData()->GetForbiddenCharsTable();
    return xF;
}

void ImpEditEngine::SetForbiddenCharsTable( rtl::Reference<SvxForbiddenCharactersTable> xForbiddenChars )
{
    EE_DLL()->GetGlobalData()->SetForbiddenCharsTable( xForbiddenChars );
}

svtools::ColorConfig& ImpEditEngine::GetColorConfig()
{
    if ( !pColorConfig )
        pColorConfig = new svtools::ColorConfig;

    return *pColorConfig;
}

BOOL ImpEditEngine::IsVisualCursorTravelingEnabled()
{
    BOOL bVisualCursorTravaling = FALSE;

    if( !pCTLOptions )
        pCTLOptions = new SvtCTLOptions;

    if ( pCTLOptions->IsCTLFontEnabled() && ( pCTLOptions->GetCTLCursorMovement() == SvtCTLOptions::MOVEMENT_VISUAL ) )
    {
        bVisualCursorTravaling = TRUE;
    }

    return bVisualCursorTravaling;

}

BOOL ImpEditEngine::DoVisualCursorTraveling( const ContentNode* )
{
    // Don't check if it's necessary, because we also need it when leaving the paragraph
    return IsVisualCursorTravelingEnabled();
}


void ImpEditEngine::CallNotify( EENotify& rNotify )
{
    if ( !nBlockNotifications )
    {
        GetNotifyHdl().Call( &rNotify );
    }
    else
    {
        EENotify* pNewNotify = new EENotify( rNotify );
        aNotifyCache.Insert( pNewNotify, aNotifyCache.Count() );
    }
}

void ImpEditEngine::EnterBlockNotifications()
{
    if( !nBlockNotifications )
    {
        // #109864# Send out START notification immediately, to allow
        // external, non-queued events to be captured as well from
        // client side
        EENotify aNotify( EE_NOTIFY_BLOCKNOTIFICATION_START );
        aNotify.pEditEngine = GetEditEnginePtr();
        GetNotifyHdl().Call( &aNotify );
    }

    nBlockNotifications++;
}

void ImpEditEngine::LeaveBlockNotifications()
{
    DBG_ASSERT( nBlockNotifications, "LeaveBlockNotifications - Why?" );

    nBlockNotifications--;
    if ( !nBlockNotifications )
    {
        // Call blocked notify events...
        while ( aNotifyCache.Count() )
        {
            EENotify* pNotify = aNotifyCache[0];
            // Remove from list before calling, maybe we enter LeaveBlockNotifications while calling the handler...
            aNotifyCache.Remove( 0 );
            GetNotifyHdl().Call( pNotify );
            delete pNotify;
        }

        EENotify aNotify( EE_NOTIFY_BLOCKNOTIFICATION_END );
        aNotify.pEditEngine = GetEditEnginePtr();
        GetNotifyHdl().Call( &aNotify );
    }
}

IMPL_LINK( ImpEditEngine, DocModified, void*, EMPTYARG )
{
    aModifyHdl.Call( NULL /*GetEditEnginePtr()*/ ); // NULL, because also used for Outliner
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
