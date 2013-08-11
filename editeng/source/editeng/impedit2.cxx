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
#include <editeng/adjustitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/justifyitem.hxx>

#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/text/CharacterCompressionType.hpp>
#include <com/sun/star/i18n/InputSequenceCheckMode.hpp>

#include <comphelper/processfactory.hxx>

#include <sot/formats.hxx>

#include <unicode/ubidi.h>

#include <boost/scoped_ptr.hpp>

using namespace ::com::sun::star;

static sal_uInt16 lcl_CalcExtraSpace( ParaPortion*, const SvxLineSpacingItem& rLSItem )
{
    sal_uInt16 nExtra = 0;
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
    aWordDelimiters("  .,;:-'`'?!_=\"{}()[]\0xFF"),
    bKernAsianPunctuation(false),
    bAddExtLeading(false),
    bIsFormatting(false),
    bFormatted(false),
    bInSelection(false),
    bIsInUndo(false),
    bUpdate(true),
    bUndoEnabled(true),
    bOwnerOfRefDev(false),
    bDowning(false),
    bUseAutoColor(true),
    bForceAutoColor(false),
    bCallParaInsertedOrDeleted(false),
    bImpConvertFirstCall(false),
    bFirstWordCapitalization(true),
    mbLastTryMerge(false)
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
    nCurTextHeightNTP   = 0;
    nBlockNotifications = 0;
    nBigTextObjectStart = 20;

    nStretchX           = 100;
    nStretchY           = 100;

    eDefLanguage        = LANGUAGE_DONTKNOW;
    maBackgroundColor   = COL_AUTO;

    nAsianCompressionMode = text::CharacterCompressionType::NONE;

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

    // Access data already from here on!
    SetRefDevice( NULL );
    InitDoc( sal_False );

    bCallParaInsertedOrDeleted = true;

    aEditDoc.SetModifyHdl( LINK( this, ImpEditEngine, DocModified ) );
}

ImpEditEngine::~ImpEditEngine()
{
    aStatusTimer.Stop();
    aOnlineSpellTimer.Stop();
    aIdleFormatter.Stop();

    // Destroying templates may otherwise cause unnecessary formatting,
    // when a parent template is destroyed.
    // And this after the destruction of the data!
    bDowning = true;
    SetUpdateMode( sal_False );

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

    if ( !pRef )
    {
        pRefDev = new VirtualDevice;
        pRefDev->SetMapMode( MAP_TWIP );
        bOwnerOfRefDev = true;
    } else
    {
        pRefDev = pRef;
        bOwnerOfRefDev = false;
    }

    nOnePixelInRef = (sal_uInt16)pRefDev->PixelToLogic( Size( 1, 0 ) ).Width();

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

    if ( !bOwnerOfRefDev )
    {
        pRefDev = new VirtualDevice;
        pRefDev->SetMapMode( MAP_TWIP );
        SetRefDevice( pRefDev );
        bOwnerOfRefDev = true;
    }
    pRefDev->SetMapMode( rMapMode );
    nOnePixelInRef = (sal_uInt16)pRefDev->PixelToLogic( Size( 1, 0 ) ).Width();
    if ( IsFormatted() )
    {
        FormatFullDoc();
        UpdateViews( (EditView*) 0);
    }
}

void ImpEditEngine::InitDoc(bool bKeepParaAttribs)
{
    sal_Int32 nParas = aEditDoc.Count();
    for ( sal_Int32 n = bKeepParaAttribs ? 1 : 0; n < nParas; n++ )
    {
        if ( aEditDoc[n]->GetStyleSheet() )
            EndListening( *aEditDoc[n]->GetStyleSheet(), sal_False );
    }

    if ( bKeepParaAttribs )
        aEditDoc.RemoveText();
    else
        aEditDoc.Clear();

    GetParaPortions().Reset();

    ParaPortion* pIniPortion = new ParaPortion( aEditDoc[0] );
    GetParaPortions().Insert(0, pIniPortion);

    bFormatted = false;

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
    sal_Int32 nStartNode = aEditDoc.GetPos( pStartNode );
    sal_Int32 nEndNode = aEditDoc.GetPos( pEndNode );

    OSL_ENSURE( nStartNode <= nEndNode, "Selection not sorted ?" );

    // iterate over the paragraphs ...
    for ( sal_Int32 nNode = nStartNode; nNode <= nEndNode; nNode++ )
    {
        OSL_ENSURE( aEditDoc.GetObject( nNode ), "Node not found: GetSelected" );
        const ContentNode* pNode = aEditDoc.GetObject( nNode );

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

sal_Bool ImpEditEngine::MouseButtonDown( const MouseEvent& rMEvt, EditView* pView )
{
    GetSelEngine().SetCurView( pView );
    SetActiveView( pView );

    if ( GetAutoCompleteText().Len() )
        SetAutoCompleteText( String(), sal_True );

    GetSelEngine().SelMouseButtonDown( rMEvt );
    // Special treatment
    EditSelection aCurSel( pView->pImpEditView->GetEditSelection() );
    if ( !rMEvt.IsShift() )
    {
        if ( rMEvt.GetClicks() == 2 )
        {
            // So that the SelectionEngine knows about the anchor.
            aSelEngine.CursorPosChanging( sal_True, sal_False );

            EditSelection aNewSelection( SelectWord( aCurSel ) );
            pView->pImpEditView->DrawSelection();
            pView->pImpEditView->SetEditSelection( aNewSelection );
            pView->pImpEditView->DrawSelection();
            pView->ShowCursor( sal_True, sal_True );
        }
        else if ( rMEvt.GetClicks() == 3 )
        {
            // So that the SelectionEngine knows about the anchor.
            aSelEngine.CursorPosChanging( sal_True, sal_False );

            EditSelection aNewSelection( aCurSel );
            aNewSelection.Min().SetIndex( 0 );
            aNewSelection.Max().SetIndex( aCurSel.Min().GetNode()->Len() );
            pView->pImpEditView->DrawSelection();
            pView->pImpEditView->SetEditSelection( aNewSelection );
            pView->pImpEditView->DrawSelection();
            pView->ShowCursor( sal_True, sal_True );
        }
    }
    return sal_True;
}

void ImpEditEngine::Command( const CommandEvent& rCEvt, EditView* pView )
{
    GetSelEngine().SetCurView( pView );
    SetActiveView( pView );
    if ( rCEvt.GetCommand() == COMMAND_STARTEXTTEXTINPUT )
    {
        pView->DeleteSelected();
        delete mpIMEInfos;
        EditPaM aPaM = pView->GetImpEditView()->GetEditSelection().Max();
        String aOldTextAfterStartPos = aPaM.GetNode()->Copy( aPaM.GetIndex() );
        sal_uInt16 nMax = aOldTextAfterStartPos.Search( CH_FEATURE );
        if ( nMax != STRING_NOTFOUND )  // don't overwrite features!
            aOldTextAfterStartPos.Erase( nMax );
        mpIMEInfos = new ImplIMEInfos( aPaM, aOldTextAfterStartPos );
        mpIMEInfos->bWasCursorOverwrite = !pView->IsInsertMode();
        UndoActionStart( EDITUNDO_INSERT );
    }
    else if ( rCEvt.GetCommand() == COMMAND_ENDEXTTEXTINPUT )
    {
        OSL_ENSURE( mpIMEInfos, "COMMAND_ENDEXTTEXTINPUT => Kein Start ?" );
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

            sal_Bool bWasCursorOverwrite = mpIMEInfos->bWasCursorOverwrite;

            delete mpIMEInfos;
            mpIMEInfos = NULL;

            FormatAndUpdate( pView );

            pView->SetInsertMode( !bWasCursorOverwrite );
        }
        UndoActionEnd( EDITUNDO_INSERT );
    }
    else if ( rCEvt.GetCommand() == COMMAND_EXTTEXTINPUT )
    {
        OSL_ENSURE( mpIMEInfos, "COMMAND_EXTTEXTINPUT => No Start ?" );
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
                    sal_uInt16 nOldIMETextLen = mpIMEInfos->nLen;
                    sal_uInt16 nNewIMETextLen = pData->GetText().Len();

                    if ( ( nOldIMETextLen > nNewIMETextLen ) &&
                         ( nNewIMETextLen < mpIMEInfos->aOldTextAfterStartPos.Len() ) )
                    {
                        // restore old characters
                        sal_uInt16 nRestore = nOldIMETextLen - nNewIMETextLen;
                        EditPaM aPaM( mpIMEInfos->aPos );
                        aPaM.GetIndex() = aPaM.GetIndex() + nNewIMETextLen;
                        ImpInsertText( aPaM, mpIMEInfos->aOldTextAfterStartPos.Copy( nNewIMETextLen, nRestore ) );
                    }
                    else if ( ( nOldIMETextLen < nNewIMETextLen ) &&
                              ( nOldIMETextLen < mpIMEInfos->aOldTextAfterStartPos.Len() ) )
                    {
                        // overwrite
                        sal_uInt16 nOverwrite = nNewIMETextLen - nOldIMETextLen;
                        if ( ( nOldIMETextLen + nOverwrite ) > mpIMEInfos->aOldTextAfterStartPos.Len() )
                            nOverwrite = mpIMEInfos->aOldTextAfterStartPos.Len() - nOldIMETextLen;
                        OSL_ENSURE( nOverwrite && (nOverwrite < 0xFF00), "IME Overwrite?!" );
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

            sal_uInt16 nInputEnd = mpIMEInfos->aPos.GetIndex() + mpIMEInfos->nLen;

            if ( !IsFormatted() )
                FormatDoc();

            ParaPortion* pParaPortion = GetParaPortions().SafeGetObject( GetEditDoc().GetPos( aPaM.GetNode() ) );
            sal_uInt16 nLine = pParaPortion->GetLines().FindLine( aPaM.GetIndex(), sal_True );
            const EditLine* pLine = pParaPortion->GetLines()[nLine];
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
    else if ( rCEvt.GetCommand() == COMMAND_QUERYCHARPOSITION )
    {
        if ( mpIMEInfos && mpIMEInfos->nLen )
        {
            EditPaM aPaM( pView->pImpEditView->GetEditSelection().Max() );
            if ( !IsFormatted() )
                FormatDoc();

            ParaPortion* pParaPortion = GetParaPortions().SafeGetObject( GetEditDoc().GetPos( aPaM.GetNode() ) );
            sal_uInt16 nLine = pParaPortion->GetLines().FindLine( aPaM.GetIndex(), sal_True );
            const EditLine* pLine = pParaPortion->GetLines()[nLine];
            if ( pLine )
            {
                Rectangle* aRects = new Rectangle[ mpIMEInfos->nLen ];
                for (sal_uInt16 i = 0; i < mpIMEInfos->nLen; ++i)
                {
                    sal_uInt16 nInputPos = mpIMEInfos->aPos.GetIndex() + i;
                    if ( nInputPos > pLine->GetEnd() )
                        nInputPos = pLine->GetEnd();
                    Rectangle aR2 = GetEditCursor( pParaPortion, nInputPos );
                    aRects[ i ] = pView->GetImpEditView()->GetWindowPos( aR2 );
                }
                pView->GetWindow()->SetCompositionCharRect( aRects, mpIMEInfos->nLen );
                delete[] aRects;
            }
        }
    }

    GetSelEngine().Command( rCEvt );
}

sal_Bool ImpEditEngine::MouseButtonUp( const MouseEvent& rMEvt, EditView* pView )
{
    GetSelEngine().SetCurView( pView );
    GetSelEngine().SelMouseButtonUp( rMEvt );
    bInSelection = false;
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
                sal_Int32 nPara = GetEditDoc().GetPos( aPaM.GetNode() );
                GetEditEnginePtr()->FieldClicked( *pFld, nPara, aPaM.GetIndex() );
            }
        }
    }
    return sal_True;
}

sal_Bool ImpEditEngine::MouseMove( const MouseEvent& rMEvt, EditView* pView )
{
    // MouseMove is called directly after ShowQuickHelp()!
    GetSelEngine().SetCurView( pView );
    GetSelEngine().SelMouseMove( rMEvt );
    return sal_True;
}

EditPaM ImpEditEngine::InsertText(const EditSelection& aSel, const String& rStr)
{
    EditPaM aPaM = ImpInsertText( aSel, rStr );
    return aPaM;
}

EditPaM ImpEditEngine::Clear()
{
    InitDoc( sal_False );

    EditPaM aPaM = aEditDoc.GetStartPaM();
    EditSelection aSel( aPaM );

    nCurTextHeight = 0;
    nCurTextHeightNTP = 0;

    ResetUndoManager();

    for (size_t nView = aEditViews.size(); nView; )
    {
        EditView* pView = aEditViews[--nView];
        DBG_CHKOBJ( pView, EditView, 0 );
        pView->pImpEditView->SetEditSelection( aSel );
    }

    return aPaM;
}

EditPaM ImpEditEngine::RemoveText()
{
    InitDoc( sal_True );

    EditPaM aStartPaM = aEditDoc.GetStartPaM();
    EditSelection aEmptySel( aStartPaM, aStartPaM );
    for (size_t nView = 0; nView < aEditViews.size(); ++nView)
    {
        EditView* pView = aEditViews[nView];
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
    sal_Bool bUndoCurrentlyEnabled = IsUndoEnabled();
    // The text inserted manually can not be made reversable by the user
    EnableUndo( sal_False );

    EditSelection aEmptySel( aStartPaM, aStartPaM );
    EditPaM aPaM = aStartPaM;
    if ( rText.Len() )
        aPaM = ImpInsertText( aEmptySel, rText );

    for (size_t nView = 0; nView < aEditViews.size(); ++nView)
    {
        EditView* pView = aEditViews[nView];
        DBG_CHKOBJ( pView, EditView, 0 );
        pView->pImpEditView->SetEditSelection( EditSelection( aPaM, aPaM ) );
        //  If no text then also no Format&Update
        // => The text remains.
        if ( !rText.Len() && GetUpdateMode() )
        {
            Rectangle aTmpRect( pView->GetOutputArea().TopLeft(),
                                Size( aPaperSize.Width(), nCurTextHeight ) );
            aTmpRect.Intersection( pView->GetOutputArea() );
            pView->GetWindow()->Invalidate( aTmpRect );
        }
    }
    if( !rText.Len() ) {    // otherwise it must be invalidated later, !bFormatted is enough.
        nCurTextHeight = 0;
        nCurTextHeightNTP = 0;
    }
    EnableUndo( bUndoCurrentlyEnabled );
    OSL_ENSURE( !HasUndoManager() || !GetUndoManager().GetUndoActionCount(), "Undo after SetText?" );
}


const SfxItemSet& ImpEditEngine::GetEmptyItemSet()
{
    if ( !pEmptyItemSet )
    {
        pEmptyItemSet = new SfxItemSet( aEditDoc.GetItemPool(), EE_ITEMS_START, EE_ITEMS_END );
        for ( sal_uInt16 nWhich = EE_ITEMS_START; nWhich <= EE_CHAR_END; nWhich++)
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
    bFormatted = false;

    if ( GetNotifyHdl().IsSet() )
    {
        EENotify aNotify( EE_NOTIFY_TEXTMODIFIED );
        aNotify.pEditEngine = GetEditEnginePtr();
        CallNotify( aNotify );
    }
}


void ImpEditEngine::ParaAttribsChanged( ContentNode* pNode )
{
    OSL_ENSURE( pNode, "ParaAttribsChanged: Which one?" );

    aEditDoc.SetModified( sal_True );
    bFormatted = false;

    ParaPortion* pPortion = FindParaPortion( pNode );
    OSL_ENSURE( pPortion, "ParaAttribsChanged: Portion?" );
    pPortion->MarkSelectionInvalid( 0, pNode->Len() );

    sal_Int32 nPara = aEditDoc.GetPos( pNode );
    pEditEngine->ParaAttribsChanged( nPara );

    ParaPortion* pNextPortion = GetParaPortions().SafeGetObject( nPara+1 );
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

    sal_Bool bCtrl = aTranslatedKeyEvent.GetKeyCode().IsMod1() ? sal_True : sal_False;
    sal_uInt16 nCode = aTranslatedKeyEvent.GetKeyCode().GetCode();

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

EditPaM ImpEditEngine::CursorVisualStartEnd( EditView* pEditView, const EditPaM& rPaM, sal_Bool bStart )
{
    EditPaM aPaM( rPaM );

    sal_Int32 nPara = GetEditDoc().GetPos( aPaM.GetNode() );
    ParaPortion* pParaPortion = GetParaPortions().SafeGetObject( nPara );

    sal_uInt16 nLine = pParaPortion->GetLines().FindLine( aPaM.GetIndex(), sal_False );
    const EditLine* pLine = pParaPortion->GetLines()[nLine];
    bool bEmptyLine = pLine->GetStart() == pLine->GetEnd();

    pEditView->pImpEditView->nExtraCursorFlags = 0;

    if ( !bEmptyLine )
    {
        String aLine(aPaM.GetNode()->GetString(), pLine->GetStart(), pLine->GetEnd() - pLine->GetStart());

        const sal_Unicode* pLineString = aLine.GetBuffer();

        UErrorCode nError = U_ZERO_ERROR;
        UBiDi* pBidi = ubidi_openSized( aLine.Len(), 0, &nError );

        const UBiDiLevel  nBidiLevel = IsRightToLeft( nPara ) ? 1 /*RTL*/ : 0 /*LTR*/;
        ubidi_setPara( pBidi, reinterpret_cast<const UChar *>(pLineString), aLine.Len(), nBidiLevel, NULL, &nError );   // UChar != sal_Unicode in MinGW

        sal_uInt16 nVisPos = bStart ? 0 : aLine.Len()-1;
        sal_uInt16 nLogPos = (sal_uInt16)ubidi_getLogicalIndex( pBidi, nVisPos, &nError );

        ubidi_close( pBidi );

        aPaM.GetIndex() = nLogPos + pLine->GetStart();

        sal_uInt16 nTmp;
        sal_uInt16 nTextPortion = pParaPortion->GetTextPortions().FindPortion( aPaM.GetIndex(), nTmp, sal_True );
        const TextPortion* pTextPortion = pParaPortion->GetTextPortions()[nTextPortion];
        sal_uInt16 nRTLLevel = pTextPortion->GetRightToLeft();
        sal_Bool bPortionRTL = (nRTLLevel%2) ? sal_True : sal_False;

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

EditPaM ImpEditEngine::CursorVisualLeftRight( EditView* pEditView, const EditPaM& rPaM, sal_uInt16 nCharacterIteratorMode, sal_Bool bVisualToLeft )
{
    EditPaM aPaM( rPaM );

    sal_Int32 nPara = GetEditDoc().GetPos( aPaM.GetNode() );
    ParaPortion* pParaPortion = GetParaPortions().SafeGetObject( nPara );

    sal_uInt16 nLine = pParaPortion->GetLines().FindLine( aPaM.GetIndex(), sal_False );
    const EditLine* pLine = pParaPortion->GetLines()[nLine];
    bool bEmptyLine = pLine->GetStart() == pLine->GetEnd();

    pEditView->pImpEditView->nExtraCursorFlags = 0;

    sal_Bool bParaRTL = IsRightToLeft( nPara );

    sal_Bool bDone = sal_False;

    if ( bEmptyLine )
    {
        if ( bVisualToLeft )
        {
            aPaM = CursorUp( aPaM, pEditView );
            if ( aPaM != rPaM )
                aPaM = CursorVisualStartEnd( pEditView, aPaM, sal_False );
        }
        else
        {
            aPaM = CursorDown( aPaM, pEditView );
            if ( aPaM != rPaM )
                aPaM = CursorVisualStartEnd( pEditView, aPaM, sal_True );
        }

        bDone = sal_True;
    }

    sal_Bool bLogicalBackward = bParaRTL ? !bVisualToLeft : bVisualToLeft;

    if ( !bDone && pEditView->IsInsertMode() )
    {
        // Check if we are within a portion and don't have overwrite mode, then it's easy...
        sal_uInt16 nPortionStart;
        sal_uInt16 nTextPortion = pParaPortion->GetTextPortions().FindPortion( aPaM.GetIndex(), nPortionStart, sal_False );
        const TextPortion* pTextPortion = pParaPortion->GetTextPortions()[nTextPortion];

        sal_Bool bPortionBoundary = ( aPaM.GetIndex() == nPortionStart ) || ( aPaM.GetIndex() == (nPortionStart+pTextPortion->GetLen()) );
        sal_uInt16 nRTLLevel = pTextPortion->GetRightToLeft();

        // Portion boundary doesn't matter if both have same RTL level
        sal_uInt16 nRTLLevelNextPortion = 0xFFFF;
        if ( bPortionBoundary && aPaM.GetIndex() && ( aPaM.GetIndex() < aPaM.GetNode()->Len() ) )
        {
            sal_uInt16 nTmp;
            sal_uInt16 nNextTextPortion = pParaPortion->GetTextPortions().FindPortion( aPaM.GetIndex()+1, nTmp, bLogicalBackward ? sal_False : sal_True );
            const TextPortion* pNextTextPortion = pParaPortion->GetTextPortions()[nNextTextPortion];
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
            bDone = sal_True;
        }
    }

    if ( !bDone )
    {
        sal_Bool bGotoStartOfNextLine = sal_False;
        sal_Bool bGotoEndOfPrevLine = sal_False;

        String aLine(aPaM.GetNode()->GetString(), pLine->GetStart(), pLine->GetEnd() - pLine->GetStart());
        sal_uInt16 nPosInLine = aPaM.GetIndex() - pLine->GetStart();

        const sal_Unicode* pLineString = aLine.GetBuffer();

        UErrorCode nError = U_ZERO_ERROR;
        UBiDi* pBidi = ubidi_openSized( aLine.Len(), 0, &nError );

        const UBiDiLevel  nBidiLevel = IsRightToLeft( nPara ) ? 1 /*RTL*/ : 0 /*LTR*/;
        ubidi_setPara( pBidi, reinterpret_cast<const UChar *>(pLineString), aLine.Len(), nBidiLevel, NULL, &nError );   // UChar != sal_Unicode in MinGW

        if ( !pEditView->IsInsertMode() )
        {
            sal_Bool bEndOfLine = nPosInLine == aLine.Len();
            sal_uInt16 nVisPos = (sal_uInt16)ubidi_getVisualIndex( pBidi, !bEndOfLine ? nPosInLine : nPosInLine-1, &nError );
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
                sal_uInt16 nLogPos = (sal_uInt16)ubidi_getLogicalIndex( pBidi, nVisPos, &nError );
                aPaM.GetIndex() = pLine->GetStart() + nLogPos;
                pEditView->pImpEditView->SetCursorBidiLevel( 0 );
            }
        }
        else
        {
            sal_Bool bWasBehind = sal_False;
            sal_Bool bBeforePortion = !nPosInLine || pEditView->pImpEditView->GetCursorBidiLevel() == 1;
            if ( nPosInLine && ( !bBeforePortion ) ) // before the next portion
                bWasBehind = sal_True;  // step one back, otherwise visual will be unusable when rtl portion follows.

            sal_uInt16 nPortionStart;
            sal_uInt16 nTextPortion = pParaPortion->GetTextPortions().FindPortion( aPaM.GetIndex(), nPortionStart, bBeforePortion );
            const TextPortion* pTextPortion = pParaPortion->GetTextPortions()[nTextPortion];
            sal_Bool bRTLPortion = (pTextPortion->GetRightToLeft() % 2) != 0;

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
                sal_uInt16 nLogPos = (sal_uInt16)ubidi_getLogicalIndex( pBidi, nVisPos, &nError );

                aPaM.GetIndex() = pLine->GetStart() + nLogPos;

                // RTL portion, stay visually on the left side.
                sal_uInt16 _nPortionStart;
                // sal_uInt16 nTextPortion = pParaPortion->GetTextPortions().FindPortion( aPaM.GetIndex(), nPortionStart, !bRTLPortion );
                sal_uInt16 _nTextPortion = pParaPortion->GetTextPortions().FindPortion( aPaM.GetIndex(), _nPortionStart, sal_True );
                const TextPortion* _pTextPortion = pParaPortion->GetTextPortions()[_nTextPortion];
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
                aPaM = CursorVisualStartEnd( pEditView, aPaM, sal_False );
        }
        else if ( bGotoStartOfNextLine )
        {
            aPaM = CursorDown( aPaM, pEditView );
            if ( aPaM != rPaM )
                aPaM = CursorVisualStartEnd( pEditView, aPaM, sal_True );
        }
    }
    return aPaM;
}


EditPaM ImpEditEngine::CursorLeft( const EditPaM& rPaM, sal_uInt16 nCharacterIteratorMode )
{
    EditPaM aCurPaM( rPaM );
    EditPaM aNewPaM( aCurPaM );

    if ( aCurPaM.GetIndex() )
    {
        sal_Int32 nCount = 1;
        uno::Reference < i18n::XBreakIterator > _xBI( ImplGetBreakIterator() );
         aNewPaM.SetIndex(
             (sal_uInt16)_xBI->previousCharacters(
                 aNewPaM.GetNode()->GetString(), aNewPaM.GetIndex(), GetLocale( aNewPaM ), nCharacterIteratorMode, nCount, nCount));
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

EditPaM ImpEditEngine::CursorRight( const EditPaM& rPaM, sal_uInt16 nCharacterIteratorMode )
{
    EditPaM aCurPaM( rPaM );
    EditPaM aNewPaM( aCurPaM );

    if ( aCurPaM.GetIndex() < aCurPaM.GetNode()->Len() )
    {
        uno::Reference < i18n::XBreakIterator > _xBI( ImplGetBreakIterator() );
        sal_Int32 nCount = 1;
        aNewPaM.SetIndex(
            (sal_uInt16)_xBI->nextCharacters(
                aNewPaM.GetNode()->GetString(), aNewPaM.GetIndex(), GetLocale( aNewPaM ), nCharacterIteratorMode, nCount, nCount));
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
    OSL_ENSURE( pView, "No View - No Cursor Movement!" );

    const ParaPortion* pPPortion = FindParaPortion( rPaM.GetNode() );
    OSL_ENSURE( pPPortion, "No matching portion found: CursorUp ");
    sal_uInt16 nLine = pPPortion->GetLineNumber( rPaM.GetIndex() );
    const EditLine* pLine = pPPortion->GetLines()[nLine];

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
        const EditLine* pPrevLine = pPPortion->GetLines()[nLine-1];
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
        const ParaPortion* pPrevPortion = GetPrevVisPortion( pPPortion );
        if ( pPrevPortion )
        {
            pLine = pPrevPortion->GetLines()[pPrevPortion->GetLines().Count()-1];
            OSL_ENSURE( pLine, "Line in front not found: CursorUp" );
            aNewPaM.SetNode( pPrevPortion->GetNode() );
            aNewPaM.SetIndex( GetChar( pPrevPortion, pLine, nX+nOnePixelInRef ) );
        }
    }

    return aNewPaM;
}

EditPaM ImpEditEngine::CursorDown( const EditPaM& rPaM, EditView* pView )
{
    OSL_ENSURE( pView, "No View - No Cursor Movement!" );

    const ParaPortion* pPPortion = FindParaPortion( rPaM.GetNode() );
    OSL_ENSURE( pPPortion, "No matching portion found: CursorDown" );
    sal_uInt16 nLine = pPPortion->GetLineNumber( rPaM.GetIndex() );

    long nX;
    if ( pView->pImpEditView->nTravelXPos == TRAVEL_X_DONTKNOW )
    {
        const EditLine* pLine = pPPortion->GetLines()[nLine];
        nX = GetXPos( pPPortion, pLine, rPaM.GetIndex() );
        pView->pImpEditView->nTravelXPos = nX+nOnePixelInRef;
    }
    else
        nX = pView->pImpEditView->nTravelXPos;

    EditPaM aNewPaM( rPaM );
    if ( nLine < pPPortion->GetLines().Count()-1 )
    {
        const EditLine* pNextLine = pPPortion->GetLines()[nLine+1];
        aNewPaM.SetIndex( GetChar( pPPortion, pNextLine, nX ) );
        // Special treatment, see CursorUp ...
        if ( ( aNewPaM.GetIndex() == pNextLine->GetEnd() ) && ( aNewPaM.GetIndex() > pNextLine->GetStart() ) && ( aNewPaM.GetIndex() < pPPortion->GetNode()->Len() ) )
            aNewPaM = CursorLeft( aNewPaM );
    }
    else    // next paragraph
    {
        const ParaPortion* pNextPortion = GetNextVisPortion( pPPortion );
        if ( pNextPortion )
        {
            const EditLine* pLine = pNextPortion->GetLines()[0];
            OSL_ENSURE( pLine, "Line in front not found: CursorUp" );
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
    const ParaPortion* pCurPortion = FindParaPortion( rPaM.GetNode() );
    OSL_ENSURE( pCurPortion, "No Portion for the PaM ?" );
    sal_uInt16 nLine = pCurPortion->GetLineNumber( rPaM.GetIndex() );
    const EditLine* pLine = pCurPortion->GetLines()[nLine];
    OSL_ENSURE( pLine, "Current line not found ?!" );

    EditPaM aNewPaM( rPaM );
    aNewPaM.SetIndex( pLine->GetStart() );
    return aNewPaM;
}

EditPaM ImpEditEngine::CursorEndOfLine( const EditPaM& rPaM )
{
    const ParaPortion* pCurPortion = FindParaPortion( rPaM.GetNode() );
    OSL_ENSURE( pCurPortion, "No Portion for the PaM ?" );
    sal_uInt16 nLine = pCurPortion->GetLineNumber( rPaM.GetIndex() );
    const EditLine* pLine = pCurPortion->GetLines()[nLine];
    OSL_ENSURE( pLine, "Current line not found ?!" );

    EditPaM aNewPaM( rPaM );
    aNewPaM.SetIndex( pLine->GetEnd() );
    if ( pLine->GetEnd() > pLine->GetStart() )
    {
        if ( aNewPaM.GetNode()->IsFeature( aNewPaM.GetIndex() - 1 ) )
        {
            // When a soft break, be in front of it!
            const EditCharAttrib* pNextFeature = aNewPaM.GetNode()->GetCharAttribs().FindFeature( aNewPaM.GetIndex()-1 );
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
    EditPaM aPaM(rPaM);
    aPaM.SetIndex(0);
    return aPaM;
}

EditPaM ImpEditEngine::CursorEndOfParagraph( const EditPaM& rPaM )
{
    EditPaM aPaM(rPaM);
    aPaM.SetIndex(rPaM.GetNode()->Len());
    return aPaM;
}

EditPaM ImpEditEngine::CursorStartOfDoc()
{
    EditPaM aPaM( aEditDoc.GetObject( 0 ), 0 );
    return aPaM;
}

EditPaM ImpEditEngine::CursorEndOfDoc()
{
    ContentNode* pLastNode = aEditDoc.GetObject( aEditDoc.Count()-1 );
    ParaPortion* pLastPortion = GetParaPortions().SafeGetObject( aEditDoc.Count()-1 );
    OSL_ENSURE( pLastNode && pLastPortion, "CursorEndOfDoc: Node or Portion not found" );

    if ( !pLastPortion->IsVisible() )
    {
        pLastNode = GetPrevVisNode( pLastPortion->GetNode() );
        OSL_ENSURE( pLastNode, "Kein sichtbarer Absatz?" );
        if ( !pLastNode )
            pLastNode = aEditDoc.GetObject( aEditDoc.Count()-1 );
    }

    EditPaM aPaM( pLastNode, pLastNode->Len() );
    return aPaM;
}

EditPaM ImpEditEngine::PageUp( const EditPaM& rPaM, EditView* pView )
{
    Rectangle aRect = PaMtoEditCursor( rPaM );
    Point aTopLeft = aRect.TopLeft();
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
    Rectangle aRect = PaMtoEditCursor( rPaM );
    Point aBottomRight = aRect.BottomRight();
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
    sal_uInt16 nCurrentPos = rPaM.GetIndex();
    EditPaM aNewPaM( rPaM );
    if ( nCurrentPos == 0 )
    {
        // Previous paragraph...
        sal_Int32 nCurPara = aEditDoc.GetPos( aNewPaM.GetNode() );
        ContentNode* pPrevNode = aEditDoc.GetObject( --nCurPara );
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
        i18n::Boundary aBoundary =
            _xBI->getWordBoundary(aNewPaM.GetNode()->GetString(), nCurrentPos, aLocale, nWordType, true);
        if ( aBoundary.startPos >= nCurrentPos )
            aBoundary = _xBI->previousWord(
                aNewPaM.GetNode()->GetString(), nCurrentPos, aLocale, nWordType);
        aNewPaM.SetIndex( ( aBoundary.startPos != (-1) ) ? (sal_uInt16)aBoundary.startPos : 0 );
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
        i18n::Boundary aBoundary = _xBI->nextWord(
            aNewPaM.GetNode()->GetString(), aNewPaM.GetIndex(), aLocale, nWordType);
        aNewPaM.SetIndex( (sal_uInt16)aBoundary.startPos );
    }
    // not 'else', maybe the index reached nMax now...
    if ( aNewPaM.GetIndex() >= nMax )
    {
        // Next paragraph ...
        sal_Int32 nCurPara = aEditDoc.GetPos( aNewPaM.GetNode() );
        ContentNode* pNextNode = aEditDoc.GetObject( ++nCurPara );
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
    i18n::Boundary aBoundary = _xBI->getWordBoundary(
        rPaM.GetNode()->GetString(), rPaM.GetIndex(), aLocale, nWordType, true);

    aNewPaM.SetIndex( (sal_uInt16)aBoundary.startPos );
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
    i18n::Boundary aBoundary = _xBI->getWordBoundary(
        rPaM.GetNode()->GetString(), rPaM.GetIndex(), aLocale, nWordType, true);

    aNewPaM.SetIndex( (sal_uInt16)aBoundary.endPos );
    return aNewPaM;
}

EditSelection ImpEditEngine::SelectWord( const EditSelection& rCurSel, sal_Int16 nWordType, sal_Bool bAcceptStartOfWord )
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
    sal_Int16 nType = _xBI->getWordType(
        aPaM.GetNode()->GetString(), aPaM.GetIndex(), aLocale);

    if ( nType == i18n::WordType::ANY_WORD )
    {
        i18n::Boundary aBoundary = _xBI->getWordBoundary(
            aPaM.GetNode()->GetString(), aPaM.GetIndex(), aLocale, nWordType, true);

        // don't select when curser at end of word
        if ( ( aBoundary.endPos > aPaM.GetIndex() ) &&
             ( ( aBoundary.startPos < aPaM.GetIndex() ) || ( bAcceptStartOfWord && ( aBoundary.startPos == aPaM.GetIndex() ) ) ) )
        {
            aNewSel.Min().SetIndex( (sal_uInt16)aBoundary.startPos );
            aNewSel.Max().SetIndex( (sal_uInt16)aBoundary.endPos );
        }
    }

    return aNewSel;
}

EditSelection ImpEditEngine::SelectSentence( const EditSelection& rCurSel )
    const
{
    uno::Reference < i18n::XBreakIterator > _xBI( ImplGetBreakIterator() );
    const EditPaM& rPaM = rCurSel.Min();
    const ContentNode* pNode = rPaM.GetNode();
    // #i50710# line breaks are marked with 0x01 - the break iterator prefers 0x0a for that
    String sParagraph = pNode->GetString();
    sParagraph.SearchAndReplaceAll(0x01,0x0a);
    //return Null if search starts at the beginning of the string
    sal_Int32 nStart = rPaM.GetIndex() ? _xBI->beginOfSentence( sParagraph, rPaM.GetIndex(), GetLocale( rPaM ) ) : 0;

    sal_Int32 nEnd = _xBI->endOfSentence(
        pNode->GetString(), rPaM.GetIndex(), GetLocale(rPaM));

    EditSelection aNewSel( rCurSel );
    OSL_ENSURE(pNode->Len() ? (nStart < pNode->Len()) : (nStart == 0), "sentence start index out of range");
    OSL_ENSURE(nEnd <= pNode->Len(), "sentence end index out of range");
    aNewSel.Min().SetIndex( (sal_uInt16)nStart );
    aNewSel.Max().SetIndex( (sal_uInt16)nEnd );
    return aNewSel;
}

sal_Bool ImpEditEngine::IsInputSequenceCheckingRequired( sal_Unicode nChar, const EditSelection& rCurSel ) const
{
    uno::Reference < i18n::XBreakIterator > _xBI( ImplGetBreakIterator() );
    if (!pCTLOptions)
        pCTLOptions = new SvtCTLOptions;

    // get the index that really is first
    sal_uInt16 nFirstPos = rCurSel.Min().GetIndex();
    sal_uInt16 nMaxPos   = rCurSel.Max().GetIndex();
    if (nMaxPos < nFirstPos)
        nFirstPos = nMaxPos;

    sal_Bool bIsSequenceChecking =
        pCTLOptions->IsCTLFontEnabled() &&
        pCTLOptions->IsCTLSequenceChecking() &&
        nFirstPos != 0 && /* first char needs not to be checked */
        _xBI.is() && i18n::ScriptType::COMPLEX == _xBI->getScriptType( OUString( nChar ), 0 );

    return bIsSequenceChecking;
}

static  bool lcl_HasStrongLTR ( const String& rTxt, xub_StrLen nStart, xub_StrLen nEnd )
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



void ImpEditEngine::InitScriptTypes( sal_Int32 nPara )
{
    ParaPortion* pParaPortion = GetParaPortions().SafeGetObject( nPara );
    ScriptTypePosInfos& rTypes = pParaPortion->aScriptInfos;
    rTypes.clear();

    ContentNode* pNode = pParaPortion->GetNode();
    if ( pNode->Len() )
    {
        uno::Reference < i18n::XBreakIterator > _xBI( ImplGetBreakIterator() );

        String aText = pNode->GetString();

        // To handle fields put the character from the field in the string,
        // because endOfScript( ... ) will skip the CH_FEATURE, because this is WEAK
        const EditCharAttrib* pField = pNode->GetCharAttribs().FindNextAttrib( EE_FEATURE_FIELD, 0 );
        while ( pField )
        {
            OUString aFldText = static_cast<const EditCharAttribField*>(pField)->GetFieldValue();
            if ( !aFldText.isEmpty() )
            {
                aText.SetChar( pField->GetStart(), aFldText.getStr()[0] );
                short nFldScriptType = _xBI->getScriptType( aFldText, 0 );

                for ( sal_uInt16 nCharInField = 1; nCharInField < aFldText.getLength(); nCharInField++ )
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

        OUString aOUText( aText );
        sal_uInt16 nTextLen = (sal_uInt16)aOUText.getLength();

        sal_Int32 nPos = 0;
        short nScriptType = _xBI->getScriptType( aOUText, nPos );
        rTypes.push_back( ScriptTypePosInfo( nScriptType, (sal_uInt16)nPos, nTextLen ) );
        nPos = _xBI->endOfScript( aOUText, nPos, nScriptType );
        while ( ( nPos != (-1) ) && ( nPos < nTextLen ) )
        {
            rTypes.back().nEndPos = (sal_uInt16)nPos;

            nScriptType = _xBI->getScriptType( aOUText, nPos );
            long nEndPos = _xBI->endOfScript( aOUText, nPos, nScriptType );

            if ( ( nScriptType == i18n::ScriptType::WEAK ) || ( nScriptType == rTypes.back().nScriptType ) )
            {
                // Expand last ScriptTypePosInfo, don't create weak or unecessary portions
                rTypes.back().nEndPos = (sal_uInt16)nEndPos;
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
                        rTypes.back().nEndPos--;
                        break;
                    }
                }
                rTypes.push_back( ScriptTypePosInfo( nScriptType, (sal_uInt16)nPos, nTextLen ) );
            }

            nPos = nEndPos;
        }

        if ( rTypes[0].nScriptType == i18n::ScriptType::WEAK )
            rTypes[0].nScriptType = ( rTypes.size() > 1 ) ? rTypes[1].nScriptType : GetI18NScriptTypeOfLanguage( GetDefaultLanguage() );

        // create writing direction information:
        if ( pParaPortion->aWritingDirectionInfos.empty() )
            InitWritingDirections( nPara );

        // i89825: Use CTL font for numbers embedded into an RTL run:
        WritingDirectionInfos& rDirInfos = pParaPortion->aWritingDirectionInfos;
        for ( size_t n = 0; n < rDirInfos.size(); ++n )
        {
            const xub_StrLen nStart = rDirInfos[n].nStartPos;
            const xub_StrLen nEnd   = rDirInfos[n].nEndPos;
            const sal_uInt8 nCurrDirType = rDirInfos[n].nType;

            if ( nCurrDirType % 2 == UBIDI_RTL  || // text in RTL run
                ( nCurrDirType > UBIDI_LTR && !lcl_HasStrongLTR( aText, nStart, nEnd ) ) ) // non-strong text in embedded LTR run
            {
                size_t nIdx = 0;

                // Skip entries in ScriptArray which are not inside the RTL run:
                while ( nIdx < rTypes.size() && rTypes[nIdx].nStartPos < nStart )
                    ++nIdx;

                // Remove any entries *inside* the current run:
                while ( nIdx < rTypes.size() && rTypes[nIdx].nEndPos <= nEnd )
                    rTypes.erase( rTypes.begin()+nIdx );

                // special case:
                if(nIdx < rTypes.size() && rTypes[nIdx].nStartPos < nStart && rTypes[nIdx].nEndPos > nEnd)
                {
                    rTypes.insert( rTypes.begin()+nIdx, ScriptTypePosInfo( rTypes[nIdx].nScriptType, (sal_uInt16)nEnd, rTypes[nIdx].nEndPos ) );
                    rTypes[nIdx].nEndPos = nStart;
                }

                if( nIdx )
                    rTypes[nIdx - 1].nEndPos = nStart;

                rTypes.insert( rTypes.begin()+nIdx, ScriptTypePosInfo( i18n::ScriptType::COMPLEX, (sal_uInt16)nStart, (sal_uInt16)nEnd) );
                ++nIdx;

                if( nIdx < rTypes.size() )
                    rTypes[nIdx].nStartPos = nEnd;
            }
        }
    }
}

sal_uInt16 ImpEditEngine::GetScriptType( const EditPaM& rPaM, sal_uInt16* pEndPos ) const
{
    sal_uInt16 nScriptType = 0;

    if ( pEndPos )
        *pEndPos = rPaM.GetNode()->Len();

    if ( rPaM.GetNode()->Len() )
    {
        sal_Int32 nPara = GetEditDoc().GetPos( rPaM.GetNode() );
        const ParaPortion* pParaPortion = GetParaPortions().SafeGetObject( nPara );
        if ( pParaPortion->aScriptInfos.empty() )
            ((ImpEditEngine*)this)->InitScriptTypes( nPara );

        const ScriptTypePosInfos& rTypes = pParaPortion->aScriptInfos;
        sal_uInt16 nPos = rPaM.GetIndex();
        for ( size_t n = 0; n < rTypes.size(); n++ )
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

sal_uInt16 ImpEditEngine::GetScriptType( const EditSelection& rSel ) const
{
    EditSelection aSel( rSel );
    aSel.Adjust( aEditDoc );

    short nScriptType = 0;

     sal_Int32 nStartPara = GetEditDoc().GetPos( aSel.Min().GetNode() );
     sal_Int32 nEndPara = GetEditDoc().GetPos( aSel.Max().GetNode() );

    for ( sal_Int32 nPara = nStartPara; nPara <= nEndPara; nPara++ )
    {
        const ParaPortion* pParaPortion = GetParaPortions().SafeGetObject( nPara );
        if ( pParaPortion->aScriptInfos.empty() )
            ((ImpEditEngine*)this)->InitScriptTypes( nPara );

        const ScriptTypePosInfos& rTypes = pParaPortion->aScriptInfos;

        // find the first(!) script type position that holds the
        // complete selection. Thus it will work for selections as
        // well as with just moving the cursor from char to char.
        sal_uInt16 nS = ( nPara == nStartPara ) ? aSel.Min().GetIndex() : 0;
        sal_uInt16 nE = ( nPara == nEndPara ) ? aSel.Max().GetIndex() : pParaPortion->GetNode()->Len();
        for ( size_t n = 0; n < rTypes.size(); n++ )
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

sal_Bool ImpEditEngine::IsScriptChange( const EditPaM& rPaM ) const
{
    bool bScriptChange = false;

    if ( rPaM.GetNode()->Len() )
    {
        sal_Int32 nPara = GetEditDoc().GetPos( rPaM.GetNode() );
        const ParaPortion* pParaPortion = GetParaPortions().SafeGetObject( nPara );
        if ( pParaPortion->aScriptInfos.empty() )
            ((ImpEditEngine*)this)->InitScriptTypes( nPara );

        const ScriptTypePosInfos& rTypes = pParaPortion->aScriptInfos;
        sal_uInt16 nPos = rPaM.GetIndex();
        for ( size_t n = 0; n < rTypes.size(); n++ )
        {
            if ( rTypes[n].nStartPos == nPos )
               {
                bScriptChange = true;
                break;
            }
        }
    }
    return bScriptChange;
}

sal_Bool ImpEditEngine::HasScriptType( sal_Int32 nPara, sal_uInt16 nType ) const
{
    bool bTypeFound = false;

    const ParaPortion* pParaPortion = GetParaPortions().SafeGetObject( nPara );
    if ( pParaPortion->aScriptInfos.empty() )
        ((ImpEditEngine*)this)->InitScriptTypes( nPara );

    const ScriptTypePosInfos& rTypes = pParaPortion->aScriptInfos;
    for ( size_t n = rTypes.size(); n && !bTypeFound; )
    {
        if ( rTypes[--n].nScriptType == nType )
                bTypeFound = true;
    }
    return bTypeFound;
}

void ImpEditEngine::InitWritingDirections( sal_Int32 nPara )
{
    ParaPortion* pParaPortion = GetParaPortions().SafeGetObject( nPara );
    WritingDirectionInfos& rInfos = pParaPortion->aWritingDirectionInfos;
    rInfos.clear();

    sal_Bool bCTL = sal_False;
    ScriptTypePosInfos& rTypes = pParaPortion->aScriptInfos;
    for ( size_t n = 0; n < rTypes.size(); n++ )
    {
        if ( rTypes[n].nScriptType == i18n::ScriptType::COMPLEX )
           {
            bCTL = sal_True;
            break;
        }
    }

    const UBiDiLevel nBidiLevel = IsRightToLeft( nPara ) ? 1 /*RTL*/ : 0 /*LTR*/;
    if ( ( bCTL || ( nBidiLevel == 1 /*RTL*/ ) ) && pParaPortion->GetNode()->Len() )
    {

        String aText = pParaPortion->GetNode()->GetString();

        //
        // Bidi functions from icu 2.0
        //
        UErrorCode nError = U_ZERO_ERROR;
        UBiDi* pBidi = ubidi_openSized( aText.Len(), 0, &nError );
        nError = U_ZERO_ERROR;

        ubidi_setPara( pBidi, reinterpret_cast<const UChar *>(aText.GetBuffer()), aText.Len(), nBidiLevel, NULL, &nError ); // UChar != sal_Unicode in MinGW
        nError = U_ZERO_ERROR;

        size_t nCount = ubidi_countRuns( pBidi, &nError );

        /* ubidi_countRuns can return -1 in case of error */
        if(nCount > 0)
        {
            int32_t nStart = 0;
            int32_t nEnd;
            UBiDiLevel nCurrDir;

            for ( size_t nIdx = 0; nIdx < nCount; ++nIdx )
            {
                ubidi_getLogicalRun( pBidi, nStart, &nEnd, &nCurrDir );
                rInfos.push_back( WritingDirectionInfo( nCurrDir, (sal_uInt16)nStart, (sal_uInt16)nEnd ) );
                nStart = nEnd;
            }
        }

        ubidi_close( pBidi );
    }

    // No infos mean no CTL and default dir is L2R...
    if ( rInfos.empty() )
        rInfos.push_back( WritingDirectionInfo( 0, 0, (sal_uInt16)pParaPortion->GetNode()->Len() ) );

}

sal_Bool ImpEditEngine::IsRightToLeft( sal_Int32 nPara ) const
{
    sal_Bool bR2L = sal_False;
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
                pFrameDirItem = NULL; // bR2L already set to default horizontal text direction
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

sal_Bool ImpEditEngine::HasDifferentRTLLevels( const ContentNode* pNode )
{
    sal_Int32 nPara = GetEditDoc().GetPos( (ContentNode*)pNode );
    ParaPortion* pParaPortion = GetParaPortions().SafeGetObject( nPara );

    sal_Bool bHasDifferentRTLLevels = sal_False;

    sal_uInt16 nRTLLevel = IsRightToLeft( nPara ) ? 1 : 0;
    for ( sal_uInt16 n = 0; n < pParaPortion->GetTextPortions().Count(); n++ )
    {
        const TextPortion* pTextPortion = pParaPortion->GetTextPortions()[n];
        if ( pTextPortion->GetRightToLeft() != nRTLLevel )
        {
            bHasDifferentRTLLevels = sal_True;
            break;
        }
    }
    return bHasDifferentRTLLevels;
}


sal_uInt8 ImpEditEngine::GetRightToLeft( sal_Int32 nPara, sal_uInt16 nPos, sal_uInt16* pStart, sal_uInt16* pEnd )
{
    sal_uInt8 nRightToLeft = 0;

    ContentNode* pNode = aEditDoc.GetObject( nPara );
    if ( pNode && pNode->Len() )
    {
        ParaPortion* pParaPortion = GetParaPortions().SafeGetObject( nPara );
        if ( pParaPortion->aWritingDirectionInfos.empty() )
            InitWritingDirections( nPara );

        WritingDirectionInfos& rDirInfos = pParaPortion->aWritingDirectionInfos;
        for ( size_t n = 0; n < rDirInfos.size(); n++ )
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

SvxAdjust ImpEditEngine::GetJustification( sal_Int32 nPara ) const
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

SvxCellJustifyMethod ImpEditEngine::GetJustifyMethod( sal_Int32 nPara ) const
{
    const SvxJustifyMethodItem& rItem = static_cast<const SvxJustifyMethodItem&>(
        GetParaAttrib(nPara, EE_PARA_JUST_METHOD));
    return static_cast<SvxCellJustifyMethod>(rItem.GetEnumValue());
}

SvxCellVerJustify ImpEditEngine::GetVerJustification( sal_Int32 nPara ) const
{
    const SvxVerJustifyItem& rItem = static_cast<const SvxVerJustifyItem&>(
        GetParaAttrib(nPara, EE_PARA_VER_JUST));
    return static_cast<SvxCellVerJustify>(rItem.GetEnumValue());
}

//  ----------------------------------------------------------------------
//  Text changes
//  ----------------------------------------------------------------------

void ImpEditEngine::ImpRemoveChars( const EditPaM& rPaM, sal_uInt16 nChars, EditUndoRemoveChars* pCurUndo )
{
    if ( IsUndoEnabled() && !IsInUndo() )
    {
        XubString aStr( rPaM.GetNode()->Copy( rPaM.GetIndex(), nChars ) );

        // Check whether attributes are deleted or changed:
        sal_uInt16 nStart = rPaM.GetIndex();
        sal_uInt16 nEnd = nStart + nChars;
        const CharAttribList::AttribsType& rAttribs = rPaM.GetNode()->GetCharAttribs().GetAttribs();
        for (size_t i = 0, n = rAttribs.size(); i < n; ++i)
        {
            const EditCharAttrib& rAttr = rAttribs[i];
            if (rAttr.GetEnd() >= nStart && rAttr.GetStart() < nEnd)
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
            InsertUndo(new EditUndoRemoveChars(pEditEngine, CreateEPaM(rPaM), aStr));
    }

    aEditDoc.RemoveChars( rPaM, nChars );
    TextModified();
}

EditSelection ImpEditEngine::ImpMoveParagraphs( Range aOldPositions, sal_Int32 nNewPos )
{
    aOldPositions.Justify();
    sal_Bool bValidAction = ( (long)nNewPos < aOldPositions.Min() ) || ( (long)nNewPos > aOldPositions.Max() );
    OSL_ENSURE( bValidAction, "Move in itself?" );
    OSL_ENSURE( aOldPositions.Max() <= (long)GetParaPortions().Count(), "totally over it: MoveParagraphs" );

    EditSelection aSelection;

    if ( !bValidAction )
    {
        aSelection = aEditDoc.GetStartPaM();
        return aSelection;
    }

    sal_Int32 nParaCount = GetParaPortions().Count();

    if ( nNewPos >= nParaCount )
        nNewPos = nParaCount;

    // Height may change when moving first or last Paragraph
    ParaPortion* pRecalc1 = NULL;
    ParaPortion* pRecalc2 = NULL;
    ParaPortion* pRecalc3 = NULL;
    ParaPortion* pRecalc4 = NULL;

    if ( nNewPos == 0 ) // Move to Start
    {
        pRecalc1 = GetParaPortions()[0];
        pRecalc2 = GetParaPortions()[aOldPositions.Min()];

    }
    else if ( nNewPos == nParaCount )
    {
        pRecalc1 = GetParaPortions()[nParaCount-1];
        pRecalc2 = GetParaPortions()[aOldPositions.Max()];
    }

    if ( aOldPositions.Min() == 0 ) // Move from Start
    {
        pRecalc3 = GetParaPortions()[0];
        pRecalc4 = GetParaPortions()[aOldPositions.Max()+1];
    }
    else if ( (sal_uInt16)aOldPositions.Max() == (nParaCount-1) )
    {
        pRecalc3 = GetParaPortions()[aOldPositions.Max()];
        pRecalc4 = GetParaPortions()[aOldPositions.Min()-1];
    }

    MoveParagraphsInfo aMoveParagraphsInfo( aOldPositions.Min(), aOldPositions.Max(), nNewPos );
    aBeginMovingParagraphsHdl.Call( &aMoveParagraphsInfo );

    if ( IsUndoEnabled() && !IsInUndo())
        InsertUndo(new EditUndoMoveParagraphs(pEditEngine, aOldPositions, nNewPos));

    // do not lose sight of the Position !
    ParaPortion* pDestPortion = GetParaPortions().SafeGetObject( nNewPos );

    ParaPortionList aTmpPortionList;
    for (sal_Int32 i = aOldPositions.Min(); i <= aOldPositions.Max(); i++  )
    {
        // always aOldPositions.Min(), since Remove().
        ParaPortion* pTmpPortion = GetParaPortions().Release(aOldPositions.Min());
        aEditDoc.Release( aOldPositions.Min() );
        aTmpPortionList.Append(pTmpPortion);
    }

    sal_Int32 nRealNewPos = pDestPortion ? GetParaPortions().GetPos( pDestPortion ) : GetParaPortions().Count();
    OSL_ENSURE( nRealNewPos != EE_PARA_NOT_FOUND, "ImpMoveParagraphs: Invalid Position!" );

    for (sal_Int32 i = 0; i < aTmpPortionList.Count(); ++i)
    {
        ParaPortion* pTmpPortion = aTmpPortionList[i];
        if ( i == 0 )
            aSelection.Min().SetNode( pTmpPortion->GetNode() );

        aSelection.Max().SetNode( pTmpPortion->GetNode() );
        aSelection.Max().SetIndex( pTmpPortion->GetNode()->Len() );

        ContentNode* pN = pTmpPortion->GetNode();
        aEditDoc.Insert(nRealNewPos+i, pN);

        GetParaPortions().Insert(nRealNewPos+i, pTmpPortion);
    }

    aEndMovingParagraphsHdl.Call( &aMoveParagraphsInfo );

    if ( GetNotifyHdl().IsSet() )
    {
        EENotify aNotify( EE_NOTIFY_PARAGRAPHSMOVED );
        aNotify.pEditEngine = GetEditEnginePtr();
        aNotify.nParagraph = nNewPos;
        aNotify.nParam1 = aOldPositions.Min();
        aNotify.nParam2 = aOldPositions.Max();
        CallNotify( aNotify );
    }

    aEditDoc.SetModified( sal_True );

    if ( pRecalc1 )
        CalcHeight( pRecalc1 );
    if ( pRecalc2 )
        CalcHeight( pRecalc2 );
    if ( pRecalc3 )
        CalcHeight( pRecalc3 );
    if ( pRecalc4 )
        CalcHeight( pRecalc4 );

    while( aTmpPortionList.Count() > 0 )
        aTmpPortionList.Release( aTmpPortionList.Count() - 1 );

#if OSL_DEBUG_LEVEL > 2
    GetParaPortions().DbgCheck(aEditDoc);
#endif
    return aSelection;
}


EditPaM ImpEditEngine::ImpConnectParagraphs( ContentNode* pLeft, ContentNode* pRight, sal_Bool bBackward )
{
    OSL_ENSURE( pLeft != pRight, "Join together the same paragraph ?" );
    OSL_ENSURE( aEditDoc.GetPos( pLeft ) != EE_PARA_NOT_FOUND, "Inserted node not found (1)" );
    OSL_ENSURE( aEditDoc.GetPos( pRight ) != EE_PARA_NOT_FOUND, "Inserted node not found (2)" );

    // #i120020# it is possible that left and right are *not* in the desired order (left/right)
    // so correct it. This correction is needed, else an invalid SfxLinkUndoAction will be
    // created from ConnectParagraphs below. Assert this situation, it should be corrected by the
    // caller.
    if(aEditDoc.GetPos( pLeft ) > aEditDoc.GetPos( pRight ))
    {
        OSL_ENSURE(false, "ImpConnectParagraphs wit wrong order of pLeft/pRight nodes (!)");
        std::swap(pLeft, pRight);
    }

    sal_Int32 nParagraphTobeDeleted = aEditDoc.GetPos( pRight );
    DeletedNodeInfo* pInf = new DeletedNodeInfo( (sal_uIntPtr)pRight, nParagraphTobeDeleted );
    aDeletedNodes.push_back(pInf);

    GetEditEnginePtr()->ParagraphConnected( aEditDoc.GetPos( pLeft ), aEditDoc.GetPos( pRight ) );

    if ( IsUndoEnabled() && !IsInUndo() )
    {
        InsertUndo( new EditUndoConnectParas(pEditEngine,
            aEditDoc.GetPos( pLeft ), pLeft->Len(),
            pLeft->GetContentAttribs().GetItems(), pRight->GetContentAttribs().GetItems(),
            pLeft->GetStyleSheet(), pRight->GetStyleSheet(), bBackward ) );
    }

    if ( bBackward )
    {
        pLeft->SetStyleSheet( pRight->GetStyleSheet(), sal_True );
        pLeft->GetContentAttribs().GetItems().Set( pRight->GetContentAttribs().GetItems() );
        pLeft->GetCharAttribs().GetDefFont() = pRight->GetCharAttribs().GetDefFont();
    }

    ParaAttribsChanged( pLeft );

    // First search for Portions since pRight is gone after ConnectParagraphs.
    ParaPortion* pLeftPortion = FindParaPortion( pLeft );
    OSL_ENSURE( pLeftPortion, "Blind Portion in ImpConnectParagraphs(1)" );

    if ( GetStatus().DoOnlineSpelling() )
    {
        xub_StrLen nEnd = pLeft->Len();
        xub_StrLen nInv = nEnd ? nEnd-1 : nEnd;
        pLeft->GetWrongList()->ClearWrongs( nInv, 0xFFFF, pLeft );  // Possibly remove one
        pLeft->GetWrongList()->MarkInvalid( nInv, nEnd+1 );
        // Take over misspelled words
        WrongList* pRWrongs = pRight->GetWrongList();
        for (WrongList::iterator i = pRWrongs->begin(); i < pRWrongs->end(); ++i)
        {
            if (i->nStart != 0)   // Not a subsequent
            {
                i->nStart = i->nStart + nEnd;
                i->nEnd = i->nEnd + nEnd;
                pLeft->GetWrongList()->push_back(*i);
            }
        }
    }

    if ( IsCallParaInsertedOrDeleted() )
        GetEditEnginePtr()->ParagraphDeleted( nParagraphTobeDeleted );

    EditPaM aPaM = aEditDoc.ConnectParagraphs( pLeft, pRight );
    GetParaPortions().Remove( nParagraphTobeDeleted );

    pLeftPortion->MarkSelectionInvalid( aPaM.GetIndex(), pLeft->Len() );

    // the right node is deleted by EditDoc:ConnectParagraphs().
    if ( GetTextRanger() )
    {
        // By joining together the two, the left is although reformatted,
        // however if its height does not change then the formatting receives
        // the change of the total text hight too late...
        for ( sal_Int32 n = nParagraphTobeDeleted; n < GetParaPortions().Count(); n++ )
        {
            ParaPortion* pPP = GetParaPortions()[n];
            pPP->MarkSelectionInvalid( 0, pPP->GetNode()->Len() );
            pPP->GetLines().Reset();
        }
    }

    TextModified();

    return aPaM;
}

EditPaM ImpEditEngine::DeleteLeftOrRight( const EditSelection& rSel, sal_uInt8 nMode, sal_uInt8 nDelMode )
{
    OSL_ENSURE( !EditSelection( rSel ).DbgIsBuggy( aEditDoc ), "Index out of range in DeleteLeftOrRight" );

    if ( rSel.HasRange() )  // only then Delete Selection
        return ImpDeleteSelection( rSel );

    EditPaM aCurPos( rSel.Max() );
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
                const xub_StrLen nLen(aCurPos.GetNode()->Len());

                // #i120020# when 0 == nLen, aDelStart needs to be adapted, not
                // aDelEnd. This would (and did) lead to a wrong order in the
                // ImpConnectParagraphs call later.
                if(nLen)
                {
                    // end of para?
                    if (aDelEnd.GetIndex() == nLen)
                    {
                        aDelEnd = WordLeft( aCurPos );
                    }
                    else // there's still sth to delete on the right
                    {
                        aDelEnd = EndOfWord( WordRight( aCurPos ) );
                        // if there'n no next word...
                        if (aDelEnd.GetIndex() == nLen )
                        {
                            aDelEnd.SetIndex( nLen );
                        }
                    }
                }
                else
                {
                    aDelStart = WordLeft(aCurPos);
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
    sal_Bool bSpecialBackward = ( ( nMode == DEL_LEFT ) && ( nDelMode == DELMODE_SIMPLE ) )
                                ? sal_True : sal_False;
    if ( aStatus.IsAnyOutliner() )
        bSpecialBackward = sal_False;

    return ImpConnectParagraphs( aDelStart.GetNode(), aDelEnd.GetNode(), bSpecialBackward );
}

EditPaM ImpEditEngine::ImpDeleteSelection(const EditSelection& rCurSel)
{
    if ( !rCurSel.HasRange() )
        return rCurSel.Min();

    EditSelection aCurSel(rCurSel);
    aCurSel.Adjust( aEditDoc );
    EditPaM aStartPaM(aCurSel.Min());
    EditPaM aEndPaM(aCurSel.Max());

    CursorMoved( aStartPaM.GetNode() ); // only so that newly set Attributes dissapear...
    CursorMoved( aEndPaM.GetNode() );   // only so that newly set Attributes dissapear...

    OSL_ENSURE( aStartPaM.GetIndex() <= aStartPaM.GetNode()->Len(), "Index out of range in ImpDeleteSelection" );
    OSL_ENSURE( aEndPaM.GetIndex() <= aEndPaM.GetNode()->Len(), "Index out of range in ImpDeleteSelection" );

    sal_Int32 nStartNode = aEditDoc.GetPos( aStartPaM.GetNode() );
    sal_Int32 nEndNode = aEditDoc.GetPos( aEndPaM.GetNode() );

    OSL_ENSURE( nEndNode != EE_PARA_NOT_FOUND, "Start > End ?!" );
    OSL_ENSURE( nStartNode <= nEndNode, "Start > End ?!" );

    // Remove all nodes in between ....
    for ( sal_Int32 z = nStartNode+1; z < nEndNode; z++ )
    {
        // Always nStartNode+1, due to Remove()!
        ImpRemoveParagraph( nStartNode+1 );
    }

    if ( aStartPaM.GetNode() != aEndPaM.GetNode() )
    {
        // The Rest of the StartNodes...
        sal_uInt16 nChars;
        nChars = aStartPaM.GetNode()->Len() - aStartPaM.GetIndex();
        ImpRemoveChars( aStartPaM, nChars );
        ParaPortion* pPortion = FindParaPortion( aStartPaM.GetNode() );
        OSL_ENSURE( pPortion, "Blind Portion in ImpDeleteSelection(3)" );
        pPortion->MarkSelectionInvalid( aStartPaM.GetIndex(), aStartPaM.GetNode()->Len() );

        // The beginning of the EndNodes....
        nChars = aEndPaM.GetIndex();
        aEndPaM.SetIndex( 0 );
        ImpRemoveChars( aEndPaM, nChars );
        pPortion = FindParaPortion( aEndPaM.GetNode() );
        OSL_ENSURE( pPortion, "Blind Portion in ImpDeleteSelection(4)" );
        pPortion->MarkSelectionInvalid( 0, aEndPaM.GetNode()->Len() );
        // Join together....
        aStartPaM = ImpConnectParagraphs( aStartPaM.GetNode(), aEndPaM.GetNode() );
    }
    else
    {
        sal_uInt16 nChars;
        nChars = aEndPaM.GetIndex() - aStartPaM.GetIndex();
        ImpRemoveChars( aStartPaM, nChars );
        ParaPortion* pPortion = FindParaPortion( aStartPaM.GetNode() );
        OSL_ENSURE( pPortion, "Blind Portion in ImpDeleteSelection(5)" );
        pPortion->MarkInvalid( aEndPaM.GetIndex(), aStartPaM.GetIndex() - aEndPaM.GetIndex() );
    }

    UpdateSelections();
    TextModified();
    return aStartPaM;
}

void ImpEditEngine::ImpRemoveParagraph( sal_Int32 nPara )
{
    ContentNode* pNode = aEditDoc.GetObject( nPara );
    ContentNode* pNextNode = aEditDoc.GetObject( nPara+1 );

    OSL_ENSURE( pNode, "Blind Node in ImpRemoveParagraph" );

    DeletedNodeInfo* pInf = new DeletedNodeInfo( (sal_uIntPtr)pNode, nPara );
    aDeletedNodes.push_back(pInf);

    // The node is managed by the undo and possibly destroyed!
    aEditDoc.Release( nPara );
    GetParaPortions().Remove( nPara );

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
        InsertUndo(new EditUndoDelContent(pEditEngine, pNode, nPara));
    else
    {
        aEditDoc.RemoveItemsFromPool(*pNode);
        if ( pNode->GetStyleSheet() )
            EndListening( *pNode->GetStyleSheet(), sal_False );
        delete pNode;
    }
}

EditPaM ImpEditEngine::AutoCorrect( const EditSelection& rCurSel, sal_Unicode c,
                                    sal_Bool bOverwrite, Window* pFrameWin )
{
    EditSelection aSel( rCurSel );
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
    if ( pAutoCorrect )
    {
        if ( aSel.HasRange() )
            aSel = ImpDeleteSelection( rCurSel );

        // #i78661 allow application to turn off capitalization of
        // start sentence explicitly.
        // (This is done by setting IsFirstWordCapitalization to sal_False.)
        sal_Bool bOldCptlSttSntnc = pAutoCorrect->IsAutoCorrFlag( CptlSttSntnc );
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
            sal_Bool bIsFirstWordInFirstPara = aESel.nEndPara == 0 &&
                    aFirstWordSel.Max().GetIndex() <= aSel.Max().GetIndex() &&
                    aSel.Max().GetIndex() <= aSecondWordSel.Min().GetIndex();

            if (bIsFirstWordInFirstPara)
                pAutoCorrect->SetAutoCorrFlag( CptlSttSntnc, IsFirstWordCapitalization() );
        }

        ContentNode* pNode = aSel.Max().GetNode();
        sal_uInt16 nIndex = aSel.Max().GetIndex();
        EdtAutoCorrDoc aAuto(pEditEngine, pNode, nIndex, c);
        // FIXME: this _must_ be called with reference to the actual node text!
        String const& rNodeString(pNode->GetString());
        pAutoCorrect->DoAutoCorrect(
            aAuto, rNodeString, nIndex, c, !bOverwrite, pFrameWin );
        aSel.Max().SetIndex( aAuto.GetCursor() );

        // #i78661 since the SvxAutoCorrect object used here is
        // shared we need to reset the value to it's original state.
        pAutoCorrect->SetAutoCorrFlag( CptlSttSntnc, bOldCptlSttSntnc );
    }
    return aSel.Max();
}


EditPaM ImpEditEngine::InsertText( const EditSelection& rCurSel,
        sal_Unicode c, sal_Bool bOverwrite, sal_Bool bIsUserInput )
{
    OSL_ENSURE( c != '\t', "Tab for InsertText ?" );
    OSL_ENSURE( c != '\n', "Word wrapping for InsertText ?");

    EditPaM aPaM( rCurSel.Min() );

    sal_Bool bDoOverwrite = ( bOverwrite &&
            ( aPaM.GetIndex() < aPaM.GetNode()->Len() ) ) ? sal_True : sal_False;

    sal_Bool bUndoAction = ( rCurSel.HasRange() || bDoOverwrite );

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
        OSL_ENSURE( !aTmpSel.DbgIsBuggy( aEditDoc ), "Overwrite: Wrong selection! ");
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
                OUString aOldText( aPaM.GetNode()->Copy(0, nTmpPos) );
                OUString aNewText( aOldText );
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

                    String aChgText( aNewText.copy( nChgPos ) );

                    // select text from first pos to be changed to current pos
                    EditSelection aSel( EditPaM( aPaM.GetNode(), (sal_uInt16) nChgPos ), aPaM );

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
            EditUndoInsertChars* pNewUndo = new EditUndoInsertChars(pEditEngine, CreateEPaM(aPaM), OUString(c));
            sal_Bool bTryMerge = ( !bDoOverwrite && ( c != ' ' ) ) ? sal_True : sal_False;
            InsertUndo( pNewUndo, bTryMerge );
        }

        aEditDoc.InsertText( (const EditPaM&)aPaM, OUString(c) );
        ParaPortion* pPortion = FindParaPortion( aPaM.GetNode() );
        OSL_ENSURE( pPortion, "Blind Portion in InsertText" );
        pPortion->MarkInvalid( aPaM.GetIndex(), 1 );
        aPaM.GetIndex()++;   // does not do EditDoc-Method anymore
    }

    TextModified();

    if ( bUndoAction )
        UndoActionEnd( EDITUNDO_INSERT );

    return aPaM;
}

EditPaM ImpEditEngine::ImpInsertText(const EditSelection& aCurSel, const String& rStr)
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
    // #i107201# do the expensive SelectWord call only if online spelling is active
    EditSelection aCurWord;
    if ( GetStatus().DoOnlineSpelling() )
        aCurWord = SelectWord( aCurPaM, i18n::WordType::DICTIONARY_WORD );

    XubString aText(convertLineEnd(rStr, LINEEND_LF));
    SfxVoidItem aTabItem( EE_FEATURE_TAB );

    // Converts to linesep = \n
    // Token LINE_SEP query,
    // since the MAC-Compiler makes something else from \n !

    // fdo#39869 The loop run variable must be capable to hold STRLEN_MAX+1,
    // that with STRING32 would be SAL_MAX_INT32+1 but with 16-bit is 0xFFFF+1
    sal_uInt32 nStart = 0;
    while ( nStart < aText.Len() )
    {
        sal_uInt32 nEnd = aText.Search( LINE_SEP, static_cast<xub_StrLen>(nStart) );
        if ( nEnd == STRING_NOTFOUND )
            nEnd = aText.Len(); // not dereference!

        // Start == End => empty line
        if ( nEnd > nStart )
        {
            XubString aLine( aText, nStart, static_cast<xub_StrLen>(nEnd-nStart) );
            xub_StrLen nChars = aPaM.GetNode()->Len() + aLine.Len();
            if ( nChars > MAXCHARSINPARA )
            {
                xub_StrLen nMaxNewChars = MAXCHARSINPARA-aPaM.GetNode()->Len();
                nEnd -= ( aLine.Len() - nMaxNewChars ); // Then the characters end up in the next paragraph.
                aLine.Erase( nMaxNewChars );            // Delete the Rest...
            }
            if ( IsUndoEnabled() && !IsInUndo() )
                InsertUndo(new EditUndoInsertChars(pEditEngine, CreateEPaM(aPaM), aLine));
            // Tabs ?
            if ( aLine.Search( '\t' ) == STRING_NOTFOUND )
                aPaM = aEditDoc.InsertText( aPaM, aLine );
            else
            {
                sal_uInt32 nStart2 = 0;
                while ( nStart2 < aLine.Len() )
                {
                    sal_uInt32 nEnd2 = aLine.Search( '\t', static_cast<xub_StrLen>(nStart2) );
                    if ( nEnd2 == STRING_NOTFOUND )
                        nEnd2 = aLine.Len();    // not dereference!

                    if ( nEnd2 > nStart2 )
                        aPaM = aEditDoc.InsertText( aPaM, XubString( aLine,
                                    static_cast<xub_StrLen>(nStart2),
                                    static_cast<xub_StrLen>(nEnd2-nStart2) ) );
                    if ( nEnd2 < aLine.Len() )
                    {
                        aPaM = aEditDoc.InsertFeature( aPaM, aTabItem );
                    }
                    nStart2 = nEnd2+1;
                }
            }
            ParaPortion* pPortion = FindParaPortion( aPaM.GetNode() );
            OSL_ENSURE( pPortion, "Blind Portion in InsertText" );

            if ( GetStatus().DoOnlineSpelling() )
            {
                // now remove the Wrongs (red spell check marks) from both words...
                WrongList *pWrongs = aCurPaM.GetNode()->GetWrongList();
                if (pWrongs && !pWrongs->empty())
                    pWrongs->ClearWrongs( aCurWord.Min().GetIndex(), aPaM.GetIndex(), aPaM.GetNode() );
                // ... and mark both words as 'to be checked again'
                pPortion->MarkInvalid( aCurWord.Min().GetIndex(), aLine.Len() );
            }
            else
                pPortion->MarkInvalid( aCurPaM.GetIndex(), aLine.Len() );
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
    OSL_ENSURE( rStr.Search( 0x0A ) == STRING_NOTFOUND, "FastInsertText: Newline not allowed! ");
    OSL_ENSURE( rStr.Search( 0x0D ) == STRING_NOTFOUND, "FastInsertText: Newline not allowed! ");
    OSL_ENSURE( rStr.Search( '\t' ) == STRING_NOTFOUND, "FastInsertText: Newline not allowed! ");

    if ( ( aPaM.GetNode()->Len() + rStr.Len() ) < MAXCHARSINPARA )
    {
        if ( IsUndoEnabled() && !IsInUndo() )
            InsertUndo(new EditUndoInsertChars(pEditEngine, CreateEPaM(aPaM), rStr));

        aPaM = aEditDoc.InsertText( aPaM, rStr );
        TextModified();
    }
    else
    {
        aPaM = ImpInsertText( aPaM, rStr );
    }

    return aPaM;
}

EditPaM ImpEditEngine::ImpInsertFeature(const EditSelection& rCurSel, const SfxPoolItem& rItem)
{
    EditPaM aPaM;
    if ( rCurSel.HasRange() )
        aPaM = ImpDeleteSelection( rCurSel );
    else
        aPaM = rCurSel.Max();

    if ( aPaM.GetIndex() >= 0xfffe )
        return aPaM;

    if ( IsUndoEnabled() && !IsInUndo() )
        InsertUndo(new EditUndoInsertFeature(pEditEngine, CreateEPaM(aPaM), rItem));
    aPaM = aEditDoc.InsertFeature( aPaM, rItem );

    ParaPortion* pPortion = FindParaPortion( aPaM.GetNode() );
    OSL_ENSURE( pPortion, "Blind Portion in InsertFeature" );
    pPortion->MarkInvalid( aPaM.GetIndex()-1, 1 );

    TextModified();

    return aPaM;
}

EditPaM ImpEditEngine::ImpInsertParaBreak( const EditSelection& rCurSel, bool bKeepEndingAttribs )
{
    EditPaM aPaM;
    if ( rCurSel.HasRange() )
        aPaM = ImpDeleteSelection( rCurSel );
    else
        aPaM = rCurSel.Max();

    return ImpInsertParaBreak( aPaM, bKeepEndingAttribs );
}

EditPaM ImpEditEngine::ImpInsertParaBreak( EditPaM& rPaM, bool bKeepEndingAttribs )
{
    if ( aEditDoc.Count() >= EE_PARA_MAX_COUNT )
    {
        SAL_WARN( "editeng", "ImpEditEngine::ImpInsertParaBreak - can't process more than "
                << EE_PARA_MAX_COUNT << " paragraphs!");
        return rPaM;
    }

    if ( IsUndoEnabled() && !IsInUndo() )
        InsertUndo(new EditUndoSplitPara(pEditEngine, aEditDoc.GetPos(rPaM.GetNode()), rPaM.GetIndex()));

    EditPaM aPaM( aEditDoc.InsertParaBreak( rPaM, bKeepEndingAttribs ) );

    if ( GetStatus().DoOnlineSpelling() )
    {
        xub_StrLen nEnd = rPaM.GetNode()->Len();
        aPaM.GetNode()->CreateWrongList();
        WrongList* pLWrongs = rPaM.GetNode()->GetWrongList();
        WrongList* pRWrongs = aPaM.GetNode()->GetWrongList();
        // take over misspelled words:
        for(WrongList::iterator i = pLWrongs->begin(); i < pLWrongs->end(); ++i)
        {
            // Correct only if really a word gets overlapped in the process of
            // Spell checking
            if (i->nStart > nEnd)
            {
                pRWrongs->push_back(*i);
                WrongRange& rRWrong = pRWrongs->back();
                rRWrong.nStart = rRWrong.nStart - nEnd;
                rRWrong.nEnd = rRWrong.nEnd - nEnd;
            }
            else if (i->nStart < nEnd && i->nEnd > nEnd)
                i->nEnd = nEnd;
        }
        sal_uInt16 nInv = nEnd ? nEnd-1 : nEnd;
        if ( nEnd )
            pLWrongs->MarkInvalid( nInv, nEnd );
        else
            pLWrongs->SetValid();
        pRWrongs->SetValid(); // otherwise 0 - 0xFFFF
        pRWrongs->MarkInvalid( 0, 1 );  // Only test the first word
    }

    ParaPortion* pPortion = FindParaPortion( rPaM.GetNode() );
    OSL_ENSURE( pPortion, "Blind Portion in ImpInsertParaBreak" );
    pPortion->MarkInvalid( rPaM.GetIndex(), 0 );

    // Optimization: Do not place unnecessarily many getPos to Listen!
    // Here, as in undo, but also in all other methods.
    sal_Int32 nPos = GetParaPortions().GetPos( pPortion );
    ParaPortion* pNewPortion = new ParaPortion( aPaM.GetNode() );
    GetParaPortions().Insert(nPos+1, pNewPortion);
    ParaAttribsChanged( pNewPortion->GetNode() );
    if ( IsCallParaInsertedOrDeleted() )
        GetEditEnginePtr()->ParagraphInserted( nPos+1 );

    CursorMoved( rPaM.GetNode() );  // if empty Attributes have emerged.
    TextModified();
    return aPaM;
}

EditPaM ImpEditEngine::ImpFastInsertParagraph( sal_Int32 nPara )
{
    if ( IsUndoEnabled() && !IsInUndo() )
    {
        if ( nPara )
        {
            OSL_ENSURE( aEditDoc.GetObject( nPara-1 ), "FastInsertParagraph: Prev does not exist" );
            InsertUndo(new EditUndoSplitPara(pEditEngine, nPara-1, aEditDoc.GetObject( nPara-1 )->Len()));
        }
        else
            InsertUndo(new EditUndoSplitPara(pEditEngine, 0, 0));
    }

    ContentNode* pNode = new ContentNode( aEditDoc.GetItemPool() );
    // If flat mode, then later no Font is set:
    pNode->GetCharAttribs().GetDefFont() = aEditDoc.GetDefFont();

    if ( GetStatus().DoOnlineSpelling() )
        pNode->CreateWrongList();

    aEditDoc.Insert(nPara, pNode);

    ParaPortion* pNewPortion = new ParaPortion( pNode );
    GetParaPortions().Insert(nPara, pNewPortion);
    if ( IsCallParaInsertedOrDeleted() )
        GetEditEnginePtr()->ParagraphInserted( nPara );

    return EditPaM( pNode, 0 );
}

EditPaM ImpEditEngine::InsertParaBreak( EditSelection aCurSel )
{
    EditPaM aPaM( ImpInsertParaBreak( aCurSel ) );
    if ( aStatus.DoAutoIndenting() )
    {
        sal_Int32 nPara = aEditDoc.GetPos( aPaM.GetNode() );
        OSL_ENSURE( nPara > 0, "AutoIndenting: Error!" );
        XubString aPrevParaText( GetEditDoc().GetParaAsString( nPara-1 ) );
        sal_uInt16 n = 0;
        while ( ( n < aPrevParaText.Len() ) &&
                ( ( aPrevParaText.GetChar(n) == ' ' ) || ( aPrevParaText.GetChar(n) == '\t' ) ) )
        {
            if ( aPrevParaText.GetChar(n) == '\t' )
                aPaM = ImpInsertFeature( aPaM, SfxVoidItem( EE_FEATURE_TAB ) );
            else
                aPaM = ImpInsertText( aPaM, OUString(aPrevParaText.GetChar(n)) );
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

EditPaM ImpEditEngine::InsertField(const EditSelection& rCurSel, const SvxFieldItem& rFld)
{
    return ImpInsertFeature(rCurSel, rFld);
}

sal_Bool ImpEditEngine::UpdateFields()
{
    bool bChanges = false;
    sal_Int32 nParas = GetEditDoc().Count();
    for ( sal_Int32 nPara = 0; nPara < nParas; nPara++ )
    {
        bool bChangesInPara = false;
        ContentNode* pNode = GetEditDoc().GetObject( nPara );
        OSL_ENSURE( pNode, "NULL-Pointer in Doc" );
        CharAttribList::AttribsType& rAttribs = pNode->GetCharAttribs().GetAttribs();
        for (size_t nAttr = 0; nAttr < rAttribs.size(); ++nAttr)
        {
            EditCharAttrib& rAttr = rAttribs[nAttr];
            if (rAttr.Which() == EE_FEATURE_FIELD)
            {
                EditCharAttribField& rField = static_cast<EditCharAttribField&>(rAttr);
                boost::scoped_ptr<EditCharAttribField> pCurrent(new EditCharAttribField(rField));
                rField.Reset();

                if ( aStatus.MarkFields() )
                    rField.GetFldColor() = new Color( GetColorConfig().GetColorValue( svtools::WRITERFIELDSHADINGS ).nColor );

                OUString aFldValue =
                    GetEditEnginePtr()->CalcFieldValue(
                        static_cast<const SvxFieldItem&>(*rField.GetItem()),
                        nPara, rField.GetStart(), rField.GetTxtColor(), rField.GetFldColor());

                rField.SetFieldValue(aFldValue);
                if (rField != *pCurrent)
                {
                    bChanges = true;
                    bChangesInPara = true;
                }
            }
        }
        if ( bChangesInPara )
        {
            // If possible be more precise when invalidate.
            ParaPortion* pPortion = GetParaPortions()[nPara];
            OSL_ENSURE( pPortion, "NULL-Pointer in Doc" );
            pPortion->MarkSelectionInvalid( 0, pNode->Len() );
        }
    }
    return bChanges;
}

EditPaM ImpEditEngine::InsertLineBreak(const EditSelection& aCurSel)
{
    EditPaM aPaM( ImpInsertFeature( aCurSel, SfxVoidItem( EE_FEATURE_LINEBR ) ) );
    return aPaM;
}

//  ----------------------------------------------------------------------
//  Helper functions
//  ----------------------------------------------------------------------
Rectangle ImpEditEngine::PaMtoEditCursor( EditPaM aPaM, sal_uInt16 nFlags )
{
    OSL_ENSURE( GetUpdateMode(), "Must not be reached when Update=FALSE: PaMtoEditCursor" );

    Rectangle aEditCursor;
    long nY = 0;
    for ( sal_Int32 nPortion = 0; nPortion < GetParaPortions().Count(); nPortion++ )
    {
        ParaPortion* pPortion = GetParaPortions()[nPortion];
        ContentNode* pNode = pPortion->GetNode();
        OSL_ENSURE( pNode, "Invalid Node in Portion!" );
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

EditPaM ImpEditEngine::GetPaM( Point aDocPos, sal_Bool bSmart )
{
    OSL_ENSURE( GetUpdateMode(), "Must not be reached when Update=FALSE: GetPaM" );

    long nY = 0;
    long nTmpHeight;
    EditPaM aPaM;
    sal_Int32 nPortion;
    for ( nPortion = 0; nPortion < GetParaPortions().Count(); nPortion++ )
    {
        ParaPortion* pPortion = GetParaPortions()[nPortion];
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
                pPortion = GetParaPortions().SafeGetObject( nPortion );
            }
            OSL_ENSURE( pPortion, "No visible paragraph found: GetPaM" );
            aPaM = GetPaM( pPortion, aDocPos, bSmart );
            return aPaM;

        }
    }
    // Then search for the last visible:
    nPortion = GetParaPortions().Count()-1;
    while ( nPortion && !GetParaPortions()[nPortion]->IsVisible() )
        nPortion--;

    OSL_ENSURE( GetParaPortions()[nPortion]->IsVisible(), "No visible paragraph found: GetPaM" );
    aPaM.SetNode( GetParaPortions()[nPortion]->GetNode() );
    aPaM.SetIndex( GetParaPortions()[nPortion]->GetNode()->Len() );
    return aPaM;
}

sal_uInt32 ImpEditEngine::GetTextHeight() const
{
    OSL_ENSURE( GetUpdateMode(), "Should not be used for Update=FALSE: GetTextHeight" );
    OSL_ENSURE( IsFormatted() || IsFormatting(), "GetTextHeight: Not formatted" );
    return nCurTextHeight;
}

sal_uInt32 ImpEditEngine::CalcTextWidth( sal_Bool bIgnoreExtraSpace )
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
    sal_Int32 nParas = GetParaPortions().Count();
    for ( sal_Int32 nPara = 0; nPara < nParas; nPara++ )
    {
        ParaPortion* pPortion = GetParaPortions()[nPara];
        if ( pPortion->IsVisible() )
        {
            const SvxLRSpaceItem& rLRItem = GetLRSpaceItem( pPortion->GetNode() );
            sal_Int32 nSpaceBeforeAndMinLabelWidth = GetSpaceBeforeAndMinLabelWidth( pPortion->GetNode() );

            // --------------------------------------------------
            // On the lines of the paragraph ...
            // --------------------------------------------------
            sal_uLong nLines = pPortion->GetLines().Count();
            for ( sal_uInt16 nLine = 0; nLine < nLines; nLine++ )
            {
                pLine = pPortion->GetLines()[nLine];
                OSL_ENSURE( pLine, "NULL-Pointer in the line iterator in CalcWidth" );
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

sal_uInt32 ImpEditEngine::CalcLineWidth( ParaPortion* pPortion, EditLine* pLine, sal_Bool bIgnoreExtraSpace )
{
    sal_Int32 nPara = GetEditDoc().GetPos( pPortion->GetNode() );

    // #114278# Saving both layout mode and language (since I'm
    // potentially changing both)
    GetRefDevice()->Push( PUSH_TEXTLAYOUTMODE|PUSH_TEXTLANGUAGE );

    ImplInitLayoutMode( GetRefDevice(), nPara, 0xFFFF );

    SvxAdjust eJustification = GetJustification( nPara );

    // Calculation of the width without the Indents ...
    sal_uInt32 nWidth = 0;
    sal_uInt16 nPos = pLine->GetStart();
    for ( sal_uInt16 nTP = pLine->GetStartPortion(); nTP <= pLine->GetEndPortion(); nTP++ )
    {
        const TextPortion* pTextPortion = pPortion->GetTextPortions()[nTP];
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
                    ImplInitDigitMode(GetRefDevice(), aTmpFont.GetLanguage());
                    nWidth += aTmpFont.QuickGetTextSize( GetRefDevice(), pPortion->GetNode()->GetString(), nPos, pTextPortion->GetLen(), NULL ).Width();
                }
            }
            break;
        }
        nPos = nPos + pTextPortion->GetLen();
    }

    GetRefDevice()->Pop();

    return nWidth;
}

sal_uInt32 ImpEditEngine::GetTextHeightNTP() const
{
    DBG_ASSERT( GetUpdateMode(), "Should not be used for Update=FALSE: GetTextHeight" );
    DBG_ASSERT( IsFormatted() || IsFormatting(), "GetTextHeight: Not formatted" );
    return nCurTextHeightNTP;
}

sal_uInt32 ImpEditEngine::CalcTextHeight( sal_uInt32* pHeightNTP )
{
    OSL_ENSURE( GetUpdateMode(), "Should not be used when Update=FALSE: CalcTextHeight" );
    sal_uInt32 nY = 0;
    sal_uInt32 nPH;
    sal_uInt32 nEmptyHeight = 0;
    for ( sal_Int32 nPortion = 0; nPortion < GetParaPortions().Count(); nPortion++ ) {
        ParaPortion* pPortion = GetParaPortions()[nPortion];
        nPH = pPortion->GetHeight();
        nY += nPH;
        if( pHeightNTP ) {
            if ( pPortion->IsEmpty() )
                nEmptyHeight += nPH;
            else
                nEmptyHeight = 0;
        }
    }

    if ( pHeightNTP )
        *pHeightNTP = nY - nEmptyHeight;

    return nY;
}

sal_uInt16 ImpEditEngine::GetLineCount( sal_Int32 nParagraph ) const
{
    OSL_ENSURE( 0 <= nParagraph && nParagraph < GetParaPortions().Count(), "GetLineCount: Out of range" );
    const ParaPortion* pPPortion = GetParaPortions().SafeGetObject( nParagraph );
    OSL_ENSURE( pPPortion, "Paragraph not found: GetLineCount" );
    if ( pPPortion )
        return pPPortion->GetLines().Count();

    return 0xFFFF;
}

xub_StrLen ImpEditEngine::GetLineLen( sal_Int32 nParagraph, sal_uInt16 nLine ) const
{
    OSL_ENSURE( 0 <= nParagraph && nParagraph < GetParaPortions().Count(), "GetLineLen: Out of range" );
    const ParaPortion* pPPortion = GetParaPortions().SafeGetObject( nParagraph );
    OSL_ENSURE( pPPortion, "Paragraph not found: GetLineLen" );
    if ( pPPortion && ( nLine < pPPortion->GetLines().Count() ) )
    {
        const EditLine* pLine = pPPortion->GetLines()[nLine];
        OSL_ENSURE( pLine, "Line not found: GetLineHeight" );
        return pLine->GetLen();
    }

    return 0xFFFF;
}

void ImpEditEngine::GetLineBoundaries( /*out*/sal_uInt16 &rStart, /*out*/sal_uInt16 &rEnd, sal_Int32 nParagraph, sal_uInt16 nLine ) const
{
    OSL_ENSURE( 0 <= nParagraph && nParagraph < GetParaPortions().Count(), "GetLineCount: Out of range" );
    const ParaPortion* pPPortion = GetParaPortions().SafeGetObject( nParagraph );
    OSL_ENSURE( pPPortion, "Paragraph not found: GetLineBoundaries" );
    rStart = rEnd = 0xFFFF;     // default values in case of error
    if ( pPPortion && ( nLine < pPPortion->GetLines().Count() ) )
    {
        const EditLine* pLine = pPPortion->GetLines()[nLine];
        OSL_ENSURE( pLine, "Line not found: GetLineBoundaries" );
        rStart = pLine->GetStart();
        rEnd   = pLine->GetEnd();
    }
}

sal_uInt16 ImpEditEngine::GetLineNumberAtIndex( sal_Int32 nPara, sal_uInt16 nIndex ) const
{
    sal_uInt16 nLineNo = 0xFFFF;
    const ContentNode* pNode = GetEditDoc().GetObject( nPara );
    OSL_ENSURE( pNode, "GetLineNumberAtIndex: invalid paragraph index" );
    if (pNode)
    {
        // we explicitly allow for the index to point at the character right behind the text
        const bool bValidIndex = /*0 <= nIndex &&*/ nIndex <= pNode->Len();
        OSL_ENSURE( bValidIndex, "GetLineNumberAtIndex: invalid index" );
        const sal_uInt16 nLineCount = GetLineCount( nPara );
        if (nIndex == pNode->Len())
            nLineNo = nLineCount > 0 ? nLineCount - 1 : 0;
        else if (bValidIndex)   // nIndex < pNode->Len()
        {
            sal_uInt16 nStart = USHRT_MAX, nEnd = USHRT_MAX;
            for (sal_uInt16 i = 0;  i < nLineCount && nLineNo == 0xFFFF;  ++i)
            {
                GetLineBoundaries( nStart, nEnd, nPara, i );
                if (nStart <= nIndex && nIndex < nEnd)
                    nLineNo = i;
            }
        }
    }
    return nLineNo;
}

sal_uInt16 ImpEditEngine::GetLineHeight( sal_Int32 nParagraph, sal_uInt16 nLine )
{
    OSL_ENSURE( 0 <= nParagraph && nParagraph < GetParaPortions().Count(), "GetLineCount: Out of range" );
    ParaPortion* pPPortion = GetParaPortions().SafeGetObject( nParagraph );
    OSL_ENSURE( pPPortion, "Paragraph not found: GetLineHeight" );
    if ( pPPortion && ( nLine < pPPortion->GetLines().Count() ) )
    {
        const EditLine* pLine = pPPortion->GetLines()[nLine];
        OSL_ENSURE( pLine, "Paragraph not found: GetLineHeight" );
        return pLine->GetHeight();
    }

    return 0xFFFF;
}

sal_uInt32 ImpEditEngine::GetParaHeight( sal_Int32 nParagraph )
{
    sal_uInt32 nHeight = 0;

    ParaPortion* pPPortion = GetParaPortions().SafeGetObject( nParagraph );
    OSL_ENSURE( pPPortion, "Paragraph not found: GetParaHeight" );

    if ( pPPortion )
        nHeight = pPPortion->GetHeight();

    return nHeight;
}

void ImpEditEngine::UpdateSelections()
{
    // Check whether one of the selections is at a deleted node...
    // If the node is valid, the index has yet to be examined!
    for (size_t nView = 0; nView < aEditViews.size(); ++nView)
    {
        EditView* pView = aEditViews[nView];
        DBG_CHKOBJ( pView, EditView, 0 );
        EditSelection aCurSel( pView->pImpEditView->GetEditSelection() );
        bool bChanged = false;
        for (size_t i = 0, n = aDeletedNodes.size(); i < n; ++i)
        {
            const DeletedNodeInfo& rInf = aDeletedNodes[i];
            if ( ( ( sal_uLong )(aCurSel.Min().GetNode()) == rInf.GetInvalidAdress() ) ||
                 ( ( sal_uLong )(aCurSel.Max().GetNode()) == rInf.GetInvalidAdress() ) )
            {
                // Use ParaPortions, as now also hidden paragraphs have to be
                // taken into account!
                sal_Int32 nPara = rInf.GetPosition();
                ParaPortion* pPPortion = GetParaPortions().SafeGetObject( nPara );
                if ( !pPPortion ) // Last paragraph
                {
                    nPara = GetParaPortions().Count()-1;
                    pPPortion = GetParaPortions()[nPara];
                }
                OSL_ENSURE( pPPortion, "Empty Document in UpdateSelections ?" );
                // Do not end up from a hidden paragraph:
                sal_Int32 nCurPara = nPara;
                sal_Int32 nLastPara = GetParaPortions().Count()-1;
                while ( nPara <= nLastPara && !GetParaPortions()[nPara]->IsVisible() )
                    nPara++;
                if ( nPara > nLastPara ) // then also backwards ...
                {
                    nPara = nCurPara;
                    while ( nPara && !GetParaPortions()[nPara]->IsVisible() )
                        nPara--;
                }
                OSL_ENSURE( GetParaPortions()[nPara]->IsVisible(), "No visible paragraph found: UpdateSelections" );

                ParaPortion* pParaPortion = GetParaPortions()[nPara];
                EditSelection aTmpSelection( EditPaM( pParaPortion->GetNode(), 0 ) );
                pView->pImpEditView->SetEditSelection( aTmpSelection );
                bChanged=sal_True;
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

    aDeletedNodes.clear();
}

EditSelection ImpEditEngine::ConvertSelection(
    sal_Int32 nStartPara, sal_uInt16 nStartPos, sal_Int32 nEndPara, sal_uInt16 nEndPos )
{
    EditSelection aNewSelection;

    // Start...
    ContentNode* pNode = aEditDoc.GetObject( nStartPara );
    sal_uInt16 nIndex = nStartPos;
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
    pNode = aEditDoc.GetObject( nEndPara );
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

uno::Reference< datatransfer::XTransferable > ImpEditEngine::CreateTransferable( const EditSelection& rSelection )
{
    EditSelection aSelection( rSelection );
    aSelection.Adjust( GetEditDoc() );

    EditDataObject* pDataObj = new EditDataObject;
    uno::Reference< datatransfer::XTransferable > xDataObj;
    xDataObj = pDataObj;

    XubString aText(convertLineEnd(GetSelected(aSelection), GetSystemLineEnd())); // System specific
    pDataObj->GetString() = aText;

    SvxFontItem::EnableStoreUnicodeNames( sal_True );
    WriteBin( pDataObj->GetStream(), aSelection, sal_True );
    pDataObj->GetStream().Seek( 0 );
    SvxFontItem::EnableStoreUnicodeNames( sal_False );

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
                pDataObj->GetURL() = aURL;
            }
        }
    }

    return xDataObj;
}

EditSelection ImpEditEngine::InsertText( uno::Reference< datatransfer::XTransferable >& rxDataObj, const String& rBaseURL, const EditPaM& rPaM, sal_Bool bUseSpecial )
{
    EditSelection aNewSelection( rPaM );

    if ( rxDataObj.is() )
    {
        datatransfer::DataFlavor aFlavor;
        sal_Bool bDone = sal_False;

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
                    bDone = sal_True;
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
                        bDone = sal_True;
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
                    OUString aText;
                    aData >>= aText;
                    aNewSelection = ImpInsertText( rPaM, aText );
                       bDone = sal_True;
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
        sal_uInt16 nSBL = ( rLSItem.GetInterLineSpaceRule() == SVX_INTER_LINE_SPACE_FIX )
                            ? GetYValue( rLSItem.GetInterLineSpace() ) : 0;

        // only from the top ...
        sal_uInt16 nFirstInvalid = 0xFFFF;
        sal_uInt16 nLine;
        for ( nLine = 0; nLine < pPortion->GetLines().Count(); nLine++ )
        {
            const EditLine* pL = pPortion->GetLines()[nLine];
            if ( pL->IsInvalid() )
            {
                nFirstInvalid = nLine;
                break;
            }
            if ( nLine && !aStatus.IsOutliner() )   // not the first line
                aRange.Min() += nSBL;
            aRange.Min() += pL->GetHeight();
        }
        OSL_ENSURE( nFirstInvalid != 0xFFFF, "No invalid line found in GetInvalidYOffset(1)" );


        // Syndicate and more ...
        aRange.Max() = aRange.Min();
        aRange.Max() += pPortion->GetFirstLineOffset();
        if ( nFirstInvalid != 0 )   // Only if the first line is invalid
            aRange.Min() = aRange.Max();

        sal_uInt16 nLastInvalid = pPortion->GetLines().Count()-1;
        for ( nLine = nFirstInvalid; nLine < pPortion->GetLines().Count(); nLine++ )
        {
            const EditLine* pL = pPortion->GetLines()[nLine];
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
            const EditLine* pL = pPortion->GetLines()[nFirstInvalid];
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

EditPaM ImpEditEngine::GetPaM( ParaPortion* pPortion, Point aDocPos, sal_Bool bSmart )
{
    OSL_ENSURE( pPortion->IsVisible(), "Why GetPaM() for an invisible paragraph?" );
    OSL_ENSURE( IsFormatted(), "GetPaM: Not formatted" );

    sal_uInt16 nCurIndex = 0;
    EditPaM aPaM;
    aPaM.SetNode( pPortion->GetNode() );

    const SvxLineSpacingItem& rLSItem = (const SvxLineSpacingItem&)pPortion->GetNode()->GetContentAttribs().GetItem( EE_PARA_SBL );
    sal_uInt16 nSBL = ( rLSItem.GetInterLineSpaceRule() == SVX_INTER_LINE_SPACE_FIX )
                        ? GetYValue( rLSItem.GetInterLineSpace() ) : 0;

    long nY = pPortion->GetFirstLineOffset();

    OSL_ENSURE( pPortion->GetLines().Count(), "Empty ParaPortion in GetPaM!" );

    const EditLine* pLine = NULL;
    for ( sal_uInt16 nLine = 0; nLine < pPortion->GetLines().Count(); nLine++ )
    {
        const EditLine* pTmpLine = pPortion->GetLines()[nLine];
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
#if OSL_DEBUG_LEVEL > 0
        const SvxULSpaceItem& rULSpace =(const SvxULSpaceItem&)pPortion->GetNode()->GetContentAttribs().GetItem( EE_PARA_ULSPACE );
        OSL_ENSURE( nY+GetYValue( rULSpace.GetLower() ) >= aDocPos.Y() , "Index in no line, GetPaM ?" );
#endif
        aPaM.SetIndex( pPortion->GetNode()->Len() );
        return aPaM;
    }

    // If no line found, only just X-Position => Index
    nCurIndex = GetChar( pPortion, pLine, aDocPos.X(), bSmart );
    aPaM.SetIndex( nCurIndex );

    if ( nCurIndex && ( nCurIndex == pLine->GetEnd() ) &&
         ( pLine != pPortion->GetLines()[pPortion->GetLines().Count()-1] ) )
    {
        aPaM = CursorLeft( aPaM, ::com::sun::star::i18n::CharacterIteratorMode::SKIPCELL );
    }

    return aPaM;
}

sal_uInt16 ImpEditEngine::GetChar(
    const ParaPortion* pParaPortion, const EditLine* pLine, long nXPos, bool bSmart)
{
    OSL_ENSURE( pLine, "No line received: GetChar" );

    sal_uInt16 nChar = 0xFFFF;
    sal_uInt16 nCurIndex = pLine->GetStart();


    // Search best matching portion with GetPortionXOffset()
    for ( sal_uInt16 i = pLine->GetStartPortion(); i <= pLine->GetEndPortion(); i++ )
    {
        const TextPortion* pPortion = pParaPortion->GetTextPortions()[i];
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
                sal_uInt16 nMax = pPortion->GetLen();
                sal_uInt16 nOffset = 0xFFFF;
                sal_uInt16 nTmpCurIndex = nChar - pLine->GetStart();

                long nXInPortion = nXPos - nXLeft;
                if ( pPortion->IsRightToLeft() )
                    nXInPortion = nXRight - nXPos;

                // Search in Array...
                for ( sal_uInt16 x = 0; x < nMax; x++ )
                {
                    long nTmpPosMax = pLine->GetCharPosArray()[nTmpCurIndex+x];
                    if ( nTmpPosMax > nXInPortion )
                    {
                        // Check whether this or the previous...
                        long nTmpPosMin = x ? pLine->GetCharPosArray()[nTmpCurIndex+x-1] : 0;
                        long nDiffLeft = nXInPortion - nTmpPosMin;
                        long nDiffRight = nTmpPosMax - nXInPortion;
                        OSL_ENSURE( nDiffLeft >= 0, "DiffLeft negative" );
                        OSL_ENSURE( nDiffRight >= 0, "DiffRight negative" );
                        nOffset = ( bSmart && ( nDiffRight < nDiffLeft ) ) ? x+1 : x;
                        // I18N: If there are character position with the length of 0,
                        // they belong to the same character, we can not use this position as an index.
                        // Skip all 0-positions, cheaper than using XBreakIterator:
                        if ( nOffset < nMax )
                        {
                            const long nX = pLine->GetCharPosArray()[nOffset];
                            while ( ( (nOffset+1) < nMax ) && ( pLine->GetCharPosArray()[nOffset+1] == nX ) )
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

                OSL_ENSURE( nOffset <= nMax, "nOffset > nMax" );

                nChar = nChar + nOffset;

                // Check if index is within a cell:
                if ( nChar && ( nChar < pParaPortion->GetNode()->Len() ) )
                {
                    EditPaM aPaM( pParaPortion->GetNode(), nChar+1 );
                    sal_uInt16 nScriptType = GetScriptType( aPaM );
                    if ( nScriptType == i18n::ScriptType::COMPLEX )
                    {
                        uno::Reference < i18n::XBreakIterator > _xBI( ImplGetBreakIterator() );
                        sal_Int32 nCount = 1;
                        lang::Locale aLocale = GetLocale( aPaM );
                        sal_uInt16 nRight = (sal_uInt16)_xBI->nextCharacters(
                            pParaPortion->GetNode()->GetString(), nChar, aLocale, ::com::sun::star::i18n::CharacterIteratorMode::SKIPCELL, nCount, nCount );
                        sal_uInt16 nLeft = (sal_uInt16)_xBI->previousCharacters(
                            pParaPortion->GetNode()->GetString(), nRight, aLocale, ::com::sun::star::i18n::CharacterIteratorMode::SKIPCELL, nCount, nCount );
                        if ( ( nLeft != nChar ) && ( nRight != nChar ) )
                        {
                            nChar = ( std::abs( nRight - nChar ) < std::abs( nLeft - nChar ) ) ? nRight : nLeft;
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

Range ImpEditEngine::GetLineXPosStartEnd( const ParaPortion* pParaPortion, const EditLine* pLine ) const
{
    Range aLineXPosStartEnd;

    sal_Int32 nPara = GetEditDoc().GetPos( pParaPortion->GetNode() );
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

long ImpEditEngine::GetPortionXOffset(
    const ParaPortion* pParaPortion, const EditLine* pLine, sal_uInt16 nTextPortion) const
{
    long nX = pLine->GetStartPosX();

    for ( sal_uInt16 i = pLine->GetStartPortion(); i < nTextPortion; i++ )
    {
        const TextPortion* pPortion = pParaPortion->GetTextPortions()[i];
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

    sal_Int32 nPara = GetEditDoc().GetPos( pParaPortion->GetNode() );
    sal_Bool bR2LPara = IsRightToLeft( nPara );

    const TextPortion* pDestPortion = pParaPortion->GetTextPortions()[nTextPortion];
    if ( pDestPortion->GetKind() != PORTIONKIND_TAB )
    {
        if ( !bR2LPara && pDestPortion->GetRightToLeft() )
        {
            // Portions behind must be added, visual before this portion
            sal_uInt16 nTmpPortion = nTextPortion+1;
            while ( nTmpPortion <= pLine->GetEndPortion() )
            {
                const TextPortion* pNextTextPortion = pParaPortion->GetTextPortions()[nTmpPortion];
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
                const TextPortion* pPrevTextPortion = pParaPortion->GetTextPortions()[nTmpPortion];
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
                const TextPortion* pNextTextPortion = pParaPortion->GetTextPortions()[nTmpPortion];
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
                const TextPortion* pPrevTextPortion = pParaPortion->GetTextPortions()[nTmpPortion];
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
        OSL_ENSURE( GetTextRanger() || GetPaperSize().Width(), "GetPortionXOffset - paper size?!" );
        OSL_ENSURE( GetTextRanger() || (nX <= GetPaperSize().Width()), "GetPortionXOffset - position out of paper size!" );
        nX = GetPaperSize().Width() - nX;
        nX -= pDestPortion->GetSize().Width();
    }

    return nX;
}

long ImpEditEngine::GetXPos(
    const ParaPortion* pParaPortion, const EditLine* pLine, sal_uInt16 nIndex, bool bPreferPortionStart) const
{
    OSL_ENSURE( pLine, "No line received: GetXPos" );
    OSL_ENSURE( ( nIndex >= pLine->GetStart() ) && ( nIndex <= pLine->GetEnd() ) , "GetXPos has to be called properly!" );

    sal_Bool bDoPreferPortionStart = bPreferPortionStart;
    // Assure that the portion belongs to this line:
    if ( nIndex == pLine->GetStart() )
        bDoPreferPortionStart = sal_True;
    else if ( nIndex == pLine->GetEnd() )
        bDoPreferPortionStart = sal_False;

    sal_uInt16 nTextPortionStart = 0;
    sal_uInt16 nTextPortion = pParaPortion->GetTextPortions().FindPortion( nIndex, nTextPortionStart, bDoPreferPortionStart );

    OSL_ENSURE( ( nTextPortion >= pLine->GetStartPortion() ) && ( nTextPortion <= pLine->GetEndPortion() ), "GetXPos: Portion not in current line! " );

    const TextPortion* pPortion = pParaPortion->GetTextPortions()[nTextPortion];

    long nX = GetPortionXOffset( pParaPortion, pLine, nTextPortion );

    // calc text width, portion size may include CJK/CTL spacing...
    // But the array migh not be init yet, if using text ranger this method is called within CreateLines()...
    long nPortionTextWidth = pPortion->GetSize().Width();
    if ( ( pPortion->GetKind() == PORTIONKIND_TEXT ) && pPortion->GetLen() && !GetTextRanger() )
        nPortionTextWidth = pLine->GetCharPosArray()[nTextPortionStart + pPortion->GetLen() - 1 - pLine->GetStart()];

    if ( nTextPortionStart != nIndex )
    {
        // Search within portion...
        if ( nIndex == ( nTextPortionStart + pPortion->GetLen() ) )
        {
            // End of Portion
            if ( pPortion->GetKind() == PORTIONKIND_TAB )
            {
                if ( static_cast<size_t>(nTextPortion+1) < pParaPortion->GetTextPortions().Count() )
                {
                    const TextPortion* pNextPortion = pParaPortion->GetTextPortions()[nTextPortion+1];
                    if ( pNextPortion->GetKind() != PORTIONKIND_TAB )
                    {
                        if ( !bPreferPortionStart )
                            nX = GetXPos( pParaPortion, pLine, nIndex, sal_True );
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
            OSL_ENSURE( nIndex != pLine->GetStart(), "Strange behavior in new GetXPos()" );
            OSL_ENSURE( pLine && pLine->GetCharPosArray().size(), "svx::ImpEditEngine::GetXPos(), portion in an empty line?" );

            if( pLine->GetCharPosArray().size() )
            {
                sal_uInt16 nPos = nIndex - 1 - pLine->GetStart();
                if( nPos >= pLine->GetCharPosArray().size() )
                {
                    nPos = pLine->GetCharPosArray().size()-1;
                    OSL_FAIL("svx::ImpEditEngine::GetXPos(), index out of range!");
                }

                // old code restored see #i112788 (which leaves #i74188 unfixed again)
                long nPosInPortion = pLine->GetCharPosArray()[nPos];

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
                        sal_uInt8 nType = GetCharTypeForCompression( pParaPortion->GetNode()->GetChar( nIndex ) );
                        if ( nType == CHAR_PUNCTUATIONRIGHT )
                        {
                            sal_uInt16 n = nIndex - nTextPortionStart;
                            const sal_Int32* pDXArray = NULL;
                            if (!pLine->GetCharPosArray().empty())
                                pDXArray = &pLine->GetCharPosArray()[0]+( nTextPortionStart-pLine->GetStart() );
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
        OSL_ENSURE( pPortion->GetLines().Count(), "Paragraph with no lines in ParaPortion::CalcHeight" );
        for (size_t nLine = 0; nLine < pPortion->GetLines().Count(); ++nLine)
            pPortion->nHeight += pPortion->GetLines()[nLine]->GetHeight();

        if ( !aStatus.IsOutliner() )
        {
            const SvxULSpaceItem& rULItem = (const SvxULSpaceItem&)pPortion->GetNode()->GetContentAttribs().GetItem( EE_PARA_ULSPACE );
            const SvxLineSpacingItem& rLSItem = (const SvxLineSpacingItem&)pPortion->GetNode()->GetContentAttribs().GetItem( EE_PARA_SBL );
            sal_uInt16 nSBL = ( rLSItem.GetInterLineSpaceRule() == SVX_INTER_LINE_SPACE_FIX ) ? GetYValue( rLSItem.GetInterLineSpace() ) : 0;

            if ( nSBL )
            {
                if ( pPortion->GetLines().Count() > 1 )
                    pPortion->nHeight += ( pPortion->GetLines().Count() - 1 ) * nSBL;
                if ( aStatus.ULSpaceSummation() )
                    pPortion->nHeight += nSBL;
            }

            sal_Int32 nPortion = GetParaPortions().GetPos( pPortion );
            if ( nPortion || aStatus.ULSpaceFirstParagraph() )
            {
                sal_uInt16 nUpper = GetYValue( rULItem.GetUpper() );
                pPortion->nHeight += nUpper;
                pPortion->nFirstLineOffset = nUpper;
            }

            if ( ( nPortion != (GetParaPortions().Count()-1) ) )
            {
                pPortion->nHeight += GetYValue( rULItem.GetLower() );   // not in the last
            }


            if ( nPortion && !aStatus.ULSpaceSummation() )
            {
                ParaPortion* pPrev = GetParaPortions().SafeGetObject( nPortion-1 );
                const SvxULSpaceItem& rPrevULItem = (const SvxULSpaceItem&)pPrev->GetNode()->GetContentAttribs().GetItem( EE_PARA_ULSPACE );
                const SvxLineSpacingItem& rPrevLSItem = (const SvxLineSpacingItem&)pPrev->GetNode()->GetContentAttribs().GetItem( EE_PARA_SBL );

                // In realation between WinWord6/Writer3:
                // With a proportional line spacing the paragraph spacing is
                // also manipulated.
                // Only Writer3: Do not add up, but minimum distance.

                // check if distance by LineSpacing > Upper:
                sal_uInt16 nExtraSpace = GetYValue( lcl_CalcExtraSpace( pPortion, rLSItem ) );
                if ( nExtraSpace > pPortion->nFirstLineOffset )
                {
                    // Paragraph becomes 'bigger':
                    pPortion->nHeight += ( nExtraSpace - pPortion->nFirstLineOffset );
                    pPortion->nFirstLineOffset = nExtraSpace;
                }

                // Determine nFirstLineOffset now f(pNode) => now f(pNode, pPrev):
                sal_uInt16 nPrevLower = GetYValue( rPrevULItem.GetLower() );

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
                        sal_uInt16 nMoreLower = nExtraSpace - nPrevLower;
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

Rectangle ImpEditEngine::GetEditCursor( ParaPortion* pPortion, sal_uInt16 nIndex, sal_uInt16 nFlags )
{
    OSL_ENSURE( pPortion->IsVisible(), "Why GetEditCursor() for an invisible paragraph?" );
    OSL_ENSURE( IsFormatted() || GetTextRanger(), "GetEditCursor: Not formatted" );

    /*
     GETCRSR_ENDOFLINE: If after the last character of a wrapped line, remaining
     at the end of the line, not the beginning of the next one.
     Purpose:   - END => really after the last character
                - Selection....
    */

    long nY = pPortion->GetFirstLineOffset();

    const SvxLineSpacingItem& rLSItem = (const SvxLineSpacingItem&)pPortion->GetNode()->GetContentAttribs().GetItem( EE_PARA_SBL );
    sal_uInt16 nSBL = ( rLSItem.GetInterLineSpaceRule() == SVX_INTER_LINE_SPACE_FIX )
                        ? GetYValue( rLSItem.GetInterLineSpace() ) : 0;

    sal_uInt16 nCurIndex = 0;
    OSL_ENSURE( pPortion->GetLines().Count(), "Empty ParaPortion in GetEditCursor!" );
    const EditLine* pLine = NULL;
    sal_Bool bEOL = ( nFlags & GETCRSR_ENDOFLINE ) ? sal_True : sal_False;
    for ( sal_uInt16 nLine = 0; nLine < pPortion->GetLines().Count(); nLine++ )
    {
        const EditLine* pTmpLine = pPortion->GetLines()[nLine];
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
        OSL_ENSURE( nIndex == nCurIndex, "Index dead wrong in GetEditCursor!" );

        pLine = pPortion->GetLines()[pPortion->GetLines().Count()-1];
        nY -= pLine->GetHeight();
        if ( !aStatus.IsOutliner() )
            nY -= nSBL;
        nCurIndex = nCurIndex - pLine->GetLen();
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
        nX = GetXPos( pPortion, pLine, nIndex, ( nFlags & GETCRSR_PREFERPORTIONSTART ) ? sal_True : sal_False );
    }

    aEditCursor.Left() = aEditCursor.Right() = nX;

    if ( nFlags & GETCRSR_TXTONLY )
        aEditCursor.Top() = aEditCursor.Bottom() - pLine->GetTxtHeight() + 1;
    else
        aEditCursor.Top() = aEditCursor.Bottom() - std::min( pLine->GetTxtHeight(), pLine->GetHeight() ) + 1;

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

void ImpEditEngine::IndentBlock( EditView* pEditView, sal_Bool bRight )
{
    ESelection aESel( CreateESel( pEditView->pImpEditView->GetEditSelection() ) );
    aESel.Adjust();

    // Only if more selected Paragraphs ...
    if ( aESel.nEndPara > aESel.nStartPara )
    {
        ESelection aNewSel = aESel;
        aNewSel.nStartPos = 0;
        aNewSel.nEndPos = EE_TEXTPOS_ALL;

        if ( aESel.nEndPos == 0 )
        {
            aESel.nEndPara--;       // then not this paragraph ...
            aNewSel.nEndPos = 0;
        }

        pEditView->pImpEditView->DrawSelection();
        pEditView->pImpEditView->SetEditSelection(
                        pEditView->pImpEditView->GetEditSelection().Max() );
        UndoActionStart( bRight ? EDITUNDO_INDENTBLOCK : EDITUNDO_UNINDENTBLOCK );

        for ( sal_Int32 nPara = aESel.nStartPara; nPara <= aESel.nEndPara; nPara++ )
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
                const EditCharAttrib* pFeature = pNode->GetCharAttribs().FindFeature( 0 );
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
        pEditView->pImpEditView->ShowCursor( sal_False, sal_True );
    }
}

rtl::Reference<SvxForbiddenCharactersTable> ImpEditEngine::GetForbiddenCharsTable( sal_Bool bGetInternal ) const
{
    rtl::Reference<SvxForbiddenCharactersTable> xF = xForbiddenCharsTable;
    if ( !xF.is() && bGetInternal )
        xF = EE_DLL().GetGlobalData()->GetForbiddenCharsTable();
    return xF;
}

void ImpEditEngine::SetForbiddenCharsTable( rtl::Reference<SvxForbiddenCharactersTable> xForbiddenChars )
{
    EE_DLL().GetGlobalData()->SetForbiddenCharsTable( xForbiddenChars );
}

svtools::ColorConfig& ImpEditEngine::GetColorConfig()
{
    if ( !pColorConfig )
        pColorConfig = new svtools::ColorConfig;

    return *pColorConfig;
}

sal_Bool ImpEditEngine::IsVisualCursorTravelingEnabled()
{
    sal_Bool bVisualCursorTravaling = sal_False;

    if( !pCTLOptions )
        pCTLOptions = new SvtCTLOptions;

    if ( pCTLOptions->IsCTLFontEnabled() && ( pCTLOptions->GetCTLCursorMovement() == SvtCTLOptions::MOVEMENT_VISUAL ) )
    {
        bVisualCursorTravaling = sal_True;
    }

    return bVisualCursorTravaling;

}

sal_Bool ImpEditEngine::DoVisualCursorTraveling( const ContentNode* )
{
    // Don't check if it's necessary, because we also need it when leaving the paragraph
    return IsVisualCursorTravelingEnabled();
}


void ImpEditEngine::CallNotify( EENotify& rNotify )
{
    if ( !nBlockNotifications )
        GetNotifyHdl().Call( &rNotify );
    else
        aNotifyCache.push_back(rNotify);
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
    OSL_ENSURE( nBlockNotifications, "LeaveBlockNotifications - Why?" );

    nBlockNotifications--;
    if ( !nBlockNotifications )
    {
        // Call blocked notify events...
        while(!aNotifyCache.empty())
        {
            EENotify aNotify(aNotifyCache[0]);
            // Remove from list before calling, maybe we enter LeaveBlockNotifications while calling the handler...
            aNotifyCache.erase(aNotifyCache.begin());
            GetNotifyHdl().Call( &aNotify );
        }

        EENotify aNotify( EE_NOTIFY_BLOCKNOTIFICATION_END );
        aNotify.pEditEngine = GetEditEnginePtr();
        GetNotifyHdl().Call( &aNotify );
    }
}

IMPL_LINK_NOARG(ImpEditEngine, DocModified)
{
    aModifyHdl.Call( NULL /*GetEditEnginePtr()*/ ); // NULL, because also used for Outliner
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
