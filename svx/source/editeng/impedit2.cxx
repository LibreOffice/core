/*************************************************************************
 *
 *  $RCSfile: impedit2.cxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: mt $ $Date: 2001-03-06 15:54:49 $
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


#include <eeng_pch.hxx>

#pragma hdrstop

#include <srchitem.hxx>
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#include <lspcitem.hxx>
#include <flditem.hxx>
#include <impedit.hxx>
#include <editeng.hxx>
#include <editview.hxx>
#include <editdbg.hxx>
#include <eerdll2.hxx>
#include <eerdll.hxx>
#include <edtspell.hxx>
#include <eeobj.hxx>
#include <txtrange.hxx>
#include <svtools/urlbmk.hxx>


#include <fhgtitem.hxx>
#include <lrspitem.hxx>
#include <ulspitem.hxx>
#include <wghtitem.hxx>
#include <postitem.hxx>
#include <udlnitem.hxx>
#include <scripttypeitem.hxx>

#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif

#ifndef _SFX_FCONTNR_HXX //autogen
#include <sfx2/fcontnr.hxx>
#endif

#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif

#ifndef _SV_CLIP_HXX //autogen
#include <vcl/clip.hxx>
#endif

#ifndef _SV_DRAG_HXX //autogen
#include <vcl/drag.hxx>
#endif

#ifndef _VCL_CMDEVT_H
#include <vcl/cmdevt.h>
#endif

#ifndef SVX_LIGHT
#ifndef _SFXFRAME_HXX //autogen
#include <sfx2/frame.hxx>
#endif
#endif

#ifndef _COM_SUN_STAR_TEXT_CHARACTERITERATORMODE_HPP_
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_WORDTYPE_HPP_
#include <com/sun/star/i18n/WordType.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_SCRIPTTYPE_HPP_
#include <com/sun/star/i18n/ScriptType.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XCLIPBOARD_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#endif

#include <comphelper/processfactory.hxx>

#include <sot/exchange.hxx>
#include <sot/formats.hxx>


#define LINE_SEP    0x0A

using namespace ::com::sun::star;

USHORT lcl_CalcExtraSpace( ParaPortion* pPortion, const SvxLineSpacingItem& rLSItem )
{
    USHORT nExtra = 0;
    /* if ( ( rLSItem.GetInterLineSpaceRule() == SVX_INTER_LINE_SPACE_PROP )
            && ( rLSItem.GetPropLineSpace() != 100 ) )
    {
        // ULONG nH = pPortion->GetNode()->GetCharAttribs().GetDefFont().GetSize().Height();
        ULONG nH = pPortion->GetLines().GetObject( 0 )->GetHeight();
        long n = nH * rLSItem.GetPropLineSpace();
        n /= 100;
        n -= nH;    // nur den Abstand
        if ( n > 0 )
            nExtra = (USHORT)n;
    }
    else */
    if ( rLSItem.GetInterLineSpaceRule() == SVX_INTER_LINE_SPACE_FIX )
    {
        nExtra = rLSItem.GetInterLineSpace();
    }

    return nExtra;
}

// ----------------------------------------------------------------------
//  class ImpEditEngine
//  ----------------------------------------------------------------------

ImpEditEngine::ImpEditEngine( EditEngine* pEE, SfxItemPool* pItemPool ) :
    aEditDoc( pItemPool ),
    aPaperSize( 0x7FFFFFFF, 0x7FFFFFFF ),
    aMaxAutoPaperSize( 0x7FFFFFFF, 0x7FFFFFFF ),
    aMinAutoPaperSize( 0x0, 0x0 ),
    aGroupChars( RTL_CONSTASCII_USTRINGPARAM( "{}()[]" ) ),
    aWordDelimiters( RTL_CONSTASCII_USTRINGPARAM( "  .,;:-'`'?!_=\"{}()[]\0xFF" ) )
{
    pEditEngine         = pEE;
    pRefDev             = NULL;
    pVirtDev            = NULL;
    pEmptyItemSet       = NULL;
    pActiveView         = NULL;
    pSpellInfo          = NULL;
    pTextObjectPool     = NULL;
    pDragAndDropInfo    = NULL;
    pDestroyedMarker    = NULL;
    mpIMEInfos          = NULL;
    pStylePool          = NULL;
    pUndoManager        = NULL;
    pUndoMarkSelection  = NULL;
    pTextRanger         = NULL;

    nCurTextHeight      = 0;
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
    bUndoEnabled        = TRUE;
    bCallParaInsertedOrDeleted = TRUE;


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

    // Ab hier wird schon auf Daten zugegriffen!
    SetRefDevice( pRefDev );
    InitDoc( FALSE );
}

ImpEditEngine::~ImpEditEngine()
{
    aStatusTimer.Stop();
    aOnlineSpellTimer.Stop();
    aIdleFormatter.Stop();

    if ( pDestroyedMarker )
        *pDestroyedMarker = TRUE;

    // das Zerstoeren von Vorlagen kann sonst unnoetiges Formatieren ausloesen,
    // wenn eine Parent-Vorlage zerstoert wird.
    // Und das nach dem Zerstoeren der Daten!
    bDowning = TRUE;
    SetUpdateMode( FALSE );

    delete pVirtDev;
    delete pEmptyItemSet;
    delete pDragAndDropInfo;
    delete pUndoManager;
    delete pTextRanger;
    delete mpIMEInfos;
    if ( bOwnerOfRefDev )
        delete pRefDev;
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

    // Wenn RefDev == GlobalRefDev => eigenes anlegen!
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

#ifndef SVX_LIGHT
    if ( GetStatus().DoOnlineSpelling() )
        aEditDoc.GetObject( 0 )->CreateWrongList();
#endif // !SVX_LIGHT
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

    DBG_ASSERT( nStartNode <= nEndNode, "Selektion nicht sortiert ?" );

    // ueber die Absaetze iterieren...
    for ( USHORT nNode = nStartNode; nNode <= nEndNode; nNode++ )
    {
        DBG_ASSERT( aEditDoc.SaveGetObject( nNode ), "Node nicht gefunden: GetSelected" );
        ContentNode* pNode = aEditDoc.GetObject( nNode );

        xub_StrLen nStartPos = 0;
        xub_StrLen nEndPos = pNode->Len();
        if ( nNode == nStartNode )
            nStartPos = aSel.Min().GetIndex();
        if ( nNode == nEndNode ) // kann auch == nStart sein!
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
    // Sonderbehandlungen
    EditSelection aCurSel( pView->pImpEditView->GetEditSelection() );
    if ( !rMEvt.IsShift() )
    {
        if ( rMEvt.GetClicks() == 2 )
        {
            // damit die SelectionEngine weiss, dass Anker.
            aSelEngine.CursorPosChanging( TRUE, FALSE );

            EditSelection aNewSelection( SelectWord( aCurSel ) );
            pView->pImpEditView->DrawSelection();
            pView->pImpEditView->SetEditSelection( aNewSelection );
            pView->pImpEditView->DrawSelection();
            pView->ShowCursor( TRUE, TRUE );
        }
        else if ( rMEvt.GetClicks() == 3 )
        {
            // damit die SelectionEngine weiss, dass Anker.
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
#ifndef SVX_LIGHT
    GetSelEngine().SetCurView( pView );
    SetActiveView( pView );
    // Sonderbehandlung: Verschieben von Feldern.
    if ( rCEvt.GetCommand() == COMMAND_STARTDRAG )
    {
        if ( rCEvt.IsMouseEvent() && !pView->HasSelection() )
        {
            // Sonderbehandlung: Verschieben von Feldern.
            USHORT nPara, nPos;
            const SvxFieldItem* pField = pView->GetFieldUnderMousePointer( nPara, nPos );
            if ( pField )
            {
                pDragAndDropInfo = new DragAndDropInfo( *pView->GetWindow() );
                pDragAndDropInfo->pField = pField;
                // Die SelectionEngien muss glauben, dass vorher in
                // eine Selection geklickt wurde.
                MouseEvent aHackyMouseEvent( rCEvt.GetMousePosPixel(), 1 );
                GetSelEngine().SelMouseButtonUp( aHackyMouseEvent );
                GetSelEngine().SelMouseButtonDown( aHackyMouseEvent );
                GetSelEngine().SelMouseMove( aHackyMouseEvent );
                ESelection aESel( nPara, nPos, nPara, nPos+1 );
                pDragAndDropInfo->aDropSel = aESel;
                EditSelection aSel = CreateSel( aESel );
                pView->pImpEditView->GetEditSelection() = aSel;
                pView->pImpEditView->DrawSelection();
                BOOL bGotoCursor = pView->pImpEditView->DoAutoScroll();
                BOOL bForceCursor = ( pDragAndDropInfo ? FALSE : TRUE ) && !IsInSelectionMode();
                pView->ShowCursor( bGotoCursor, bForceCursor );
            }
        }
    }
    else if ( rCEvt.GetCommand() == COMMAND_VOICE )
    {
        const CommandVoiceData* pData = rCEvt.GetVoiceData();
        if ( pData->GetType() == VOICECOMMANDTYPE_DICTATION )
        {
            // Funktionen auf KeyEvents umbiegen, wenn keine entsprechende
            // Methode an EditView/EditEngine, damit Undo konsistent bleibt.

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
        mpIMEInfos = new ImplIMEInfos( pView->GetImpEditView()->GetEditSelection().Max() );
        mpIMEInfos->bWasCursorOverwrite = !pView->IsInsertMode();
    }
    else if ( rCEvt.GetCommand() == COMMAND_ENDEXTTEXTINPUT )
    {
        DBG_ASSERT( mpIMEInfos, "COMMAND_ENDEXTTEXTINPUT => Kein Start ?" );
        if( mpIMEInfos )
        {
            ParaPortion* pPortion = FindParaPortion( mpIMEInfos->aPos.GetNode() );
            pPortion->MarkSelectionInvalid( mpIMEInfos->aPos.GetIndex(), 0 );

            BOOL bWasCursorOverwrite = mpIMEInfos->bWasCursorOverwrite;

            delete mpIMEInfos;
            mpIMEInfos = NULL;

            FormatAndUpdate( pView );

            pView->SetInsertMode( !bWasCursorOverwrite );
        }
    }
    else if ( rCEvt.GetCommand() == COMMAND_EXTTEXTINPUT )
    {
        DBG_ASSERT( mpIMEInfos, "COMMAND_EXTTEXTINPUT => Kein Start ?" );
        if( mpIMEInfos )
        {
            const CommandExtTextInputData* pData = rCEvt.GetExtTextInputData();

            if ( !pData->IsOnlyCursorChanged() )
            {
                EditSelection aSel( mpIMEInfos->aPos );
                aSel.Max().GetIndex() += mpIMEInfos->nLen;
                aSel = DeleteSelected( aSel );
                aSel = ImpInsertText( aSel, pData->GetText() );

                if ( pData->GetTextAttr() )
                {
                    mpIMEInfos->CopyAttribs( pData->GetTextAttr(), pData->GetText().Len() );
                    mpIMEInfos->bCursor = pData->IsCursorVisible();
                }
                else
                {
                    mpIMEInfos->DestroyAttribs();
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
#endif // !SVX_LIGHT

    GetSelEngine().Command( rCEvt );
}

BOOL ImpEditEngine::MouseButtonUp( const MouseEvent& rMEvt, EditView* pView )
{
    GetSelEngine().SetCurView( pView );
    GetSelEngine().SelMouseButtonUp( rMEvt );
    bInSelection = FALSE;
    // Sonderbehandlungen
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
    // MouseMove wird sofort nach ShowQuickHelp() gerufen!
//  if ( GetAutoCompleteText().Len() )
//      SetAutoCompleteText( String(), TRUE );
    GetSelEngine().SetCurView( pView );
    GetSelEngine().SelMouseMove( rMEvt );
    return TRUE;
}



EditSelection ImpEditEngine::CutCopy( EditView* pView, BOOL bCut )
{
    EditSelection aSel( pView->pImpEditView->GetEditSelection() );
    if ( !aSel.HasRange() )
        return aSel;

    CopyData( aSel, EXCHANGE_CLIPBOARD );

    if ( bCut )
    {
        pView->pImpEditView->DrawSelection();
        aSel = ImpDeleteSelection( aSel );
    }
    return aSel;
}

EditSelection ImpEditEngine::Paste( EditView* pView, BOOL bUseSpecial )
{
    EditSelection aSel( pView->pImpEditView->GetEditSelection() );

    // Wenn keine Datan, dann auch nicht Selektion loeschen:
    if ( !HasData( EXCHANGE_CLIPBOARD ) )
        return aSel;

    if ( aSel.HasRange() )
    {
        pView->pImpEditView->DrawSelection();
        aSel = ImpDeleteSelection( aSel );
    }

    if ( pView->pImpEditView->DoSingleLinePaste() )
    {
        uno::Reference< datatransfer::XTransferable > xDataObj;

        uno::Reference< lang::XMultiServiceFactory > xMSF( ::comphelper::getProcessServiceFactory() );
        uno::Reference< datatransfer::clipboard::XClipboard > xClipboard( xMSF->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.datatransfer.clipboard.SystemClipboard" ) ), uno::UNO_QUERY );
        if ( xClipboard.is() )
            xDataObj = xClipboard->getContents();

        if ( xDataObj.is() )
        {
            datatransfer::DataFlavor aFlavor;
            SotExchange::GetFormatDataFlavor( SOT_FORMAT_STRING, aFlavor );
            if ( xDataObj->isDataFlavorSupported( aFlavor ) )
            {
                uno::Any aData = xDataObj->getTransferData( aFlavor );
                ::rtl::OUString aTmpText;
                aData >>= aTmpText;
                String aText( aTmpText );
                aText.ConvertLineEnd( LINEEND_LF );
                aText.SearchAndReplaceAll( LINE_SEP, ' ' );
                aSel = ImpInsertText( aSel, aText );
            }
        }
    }
    else
    {
        aSel = PasteData( aSel.Min(), EXCHANGE_CLIPBOARD, bUseSpecial );
    }

    return aSel;
}

EditPaM ImpEditEngine::InsertText( EditSelection aSel, const XubString& rStr )
{
    EditPaM aPaM = ImpInsertText( aSel, rStr );
    return aPaM;
}

BOOL ImpEditEngine::Search( const SvxSearchItem& rSearchItem, EditView* pEditView )
{
    EditSelection aSel( pEditView->pImpEditView->GetEditSelection() );

    aSel.Adjust( aEditDoc );
    EditPaM aStartPaM( aSel.Max() );
    if ( rSearchItem.GetSelection() && !rSearchItem.GetBackward() )
        aStartPaM = aSel.Min();

    EditSelection aFoundSel;
    BOOL bFound = ImpSearch( rSearchItem, aSel, aStartPaM, aFoundSel );
    if ( bFound && ( aFoundSel == aSel ) )  // Bei Rueckwaetssuche
    {
        aStartPaM = aSel.Min();
        bFound = ImpSearch( rSearchItem, aSel, aStartPaM, aFoundSel );
    }

    pEditView->pImpEditView->DrawSelection();
    if ( bFound )
    {
        // Erstmal das Min einstellen, damit das ganze Wort in den sichtbaren Bereich kommt.
        pEditView->pImpEditView->SetEditSelection( aFoundSel.Min() );
        pEditView->ShowCursor( TRUE, FALSE );
        pEditView->pImpEditView->SetEditSelection( aFoundSel );
    }
    else
        pEditView->pImpEditView->SetEditSelection( aSel.Max() );

    pEditView->pImpEditView->DrawSelection();
    pEditView->ShowCursor( TRUE, FALSE );
    return bFound;
}

USHORT ImpEditEngine::SearchAndReplace( const XubString& rBefore, const XubString& rAfter, EditView* pView )
{
    EditSelection aSel( pView->pImpEditView->GetEditSelection() );
    aSel.Adjust( aEditDoc );

    USHORT nStartNode, nEndNode;
    EditSelection   aTmpSel;

    short nDiff = rAfter.Len() - rBefore.Len();
    USHORT nReplaces = 0;

    BOOL bRange = aSel.HasRange();
    if ( bRange )
    {
        pView->pImpEditView->DrawSelection();
        nStartNode = aEditDoc.GetPos( aSel.Min().GetNode() );
        nEndNode = aEditDoc.GetPos( aSel.Max().GetNode() );
    }
    else
    {
        nStartNode = 0;
        nEndNode = aEditDoc.Count()-1;
    }

    // ueber die Absaetze iterieren...
    for ( USHORT nNode = nStartNode; nNode <= nEndNode; nNode++ )
    {
        ContentNode* pNode = aEditDoc.GetObject( nNode );
        ParaPortion* pPortion = GetParaPortions().GetObject( nNode );

        // pPortion koennte durch GetObject opimiert werden!
        DBG_ASSERT( aEditDoc.SaveGetObject( nNode ), "Node nicht gefunden: Search&Replace" );
        DBG_ASSERT( GetParaPortions().SaveGetObject( nNode ), "Portion nicht gefunden: Search&Replace" );

        USHORT nStartPos = 0;
        USHORT nEndPos = pNode->Len();
        if ( bRange )
        {
            if ( nNode == nStartNode )
                nStartPos = aSel.Min().GetIndex();
            if ( nNode == nEndNode ) // kann auch == nStart sein!
                nEndPos = aSel.Max().GetIndex();
        }

        USHORT nXStart = pNode->Search( rBefore, nStartPos );
        while ( nXStart != STRING_NOTFOUND )
        {
            USHORT nXEnd = nXStart + rBefore.Len();
            if ( nXEnd > nEndPos )
                break;

            // gefundenes Wort 'selektieren'
            aTmpSel.Min().SetNode( pNode );
            aTmpSel.Min().SetIndex( nXStart );
            aTmpSel.Max().SetNode( pNode );
            aTmpSel.Max().SetIndex( nXEnd );

            ImpDeleteSelection( aTmpSel );
            aTmpSel.Max().SetIndex( nXStart );  // schneller als = DeleteSel...
            ImpInsertText( aTmpSel, rAfter );

            // Selektion korrigieren...
            if ( bRange && ( nNode == nEndNode ) )
            {
                aSel.Max().GetIndex() += nDiff;
                pView->pImpEditView->SetEditSelection( aSel );
            }

            nEndPos += nDiff;
            nReplaces++;

            pPortion->MarkInvalid( nXStart, nDiff );
            nXStart = pNode->Search( rBefore, nXStart+rAfter.Len() );
        }
    }

    UpdateSelections();
    FormatAndUpdate( pView );
    return nReplaces;
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
    // RemoveText loescht die Undo-Liste!
    EditPaM aStartPaM = RemoveText();
    BOOL bUndoCurrentlyEnabled = IsUndoEnabled();
    // Der von Hand reingesteckte Text kann nicht vom Anwender rueckgaengig gemacht werden.
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
        // Wenn kein Text, dann auch Kein Format&Update
        // => Der Text bleibt stehen.
        if ( !rText.Len() && GetUpdateMode() )
        {
            Rectangle aTmpRec( pView->GetOutputArea().TopLeft(),
                                Size( aPaperSize.Width(), nCurTextHeight ) );
            aTmpRec.Intersection( pView->GetOutputArea() );
            pView->GetWindow()->Invalidate( aTmpRec );
        }
    }
    if( !rText.Len() )  // sonst muss spaeter noch invalidiert werden, !bFormatted reicht.
        nCurTextHeight = 0;
    EnableUndo( bUndoCurrentlyEnabled );
    DBG_ASSERT( !HasUndoManager() || !GetUndoManager().GetUndoActionCount(), "Undo nach SetText?" );
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
    // Leere Attribute loeschen, aber nur, wenn Absatz nicht leer!
    if ( pPrevNode->GetCharAttribs().HasEmptyAttribs() && pPrevNode->Len() )
        pPrevNode->GetCharAttribs().DeleteEmptyAttribs( aEditDoc.GetItemPool() );
}

void ImpEditEngine::ParaAttribsChanged( ContentNode* pNode )
{
    DBG_ASSERT( pNode, "ParaAttribsChanged: Welcher?" );

    aEditDoc.SetModified( TRUE );
    bFormatted = FALSE;

    ParaPortion* pPortion = FindParaPortion( pNode );
    DBG_ASSERT( pPortion, "ParaAttribsChanged: Portion?" );
    pPortion->MarkSelectionInvalid( 0, pNode->Len() );

    USHORT nPara = aEditDoc.GetPos( pNode );
    pEditEngine->ParaAttribsChanged( nPara );

    ParaPortion* pNextPortion = GetParaPortions().SaveGetObject( nPara+1 );
    // => wird sowieso noch formatiert, wenn Invalid.
    if ( pNextPortion && !pNextPortion->IsInvalid() )
        CalcHeight( pNextPortion );
}

//  ----------------------------------------------------------------------
//  Cursorbewegungen
//  ----------------------------------------------------------------------

EditSelection ImpEditEngine::MoveCursor( const KeyEvent& rKeyEvent, EditView* pEditView )
{
    // Eigentlich nur bei Up/Down noetig, aber was solls.
    CheckIdleFormatter();

    EditPaM aPaM( pEditView->pImpEditView->GetEditSelection().Max() );

    EditPaM aOldPaM( aPaM );

    BOOL bCtrl = rKeyEvent.GetKeyCode().IsMod1() ? TRUE : FALSE;

    USHORT nCode = rKeyEvent.GetKeyCode().GetCode();

    if ( IsVertical() )
    {
        switch ( nCode )
        {
            case KEY_UP:    nCode = KEY_LEFT;
                            break;
            case KEY_DOWN:  nCode = KEY_RIGHT;
                            break;
            case KEY_LEFT:  nCode = KEY_DOWN;
                            break;
            case KEY_RIGHT: nCode = KEY_UP;
                            break;
        }
    }

    switch ( nCode )
    {
        case KEY_UP:        aPaM = CursorUp( aPaM, pEditView );
                            break;
        case KEY_DOWN:      aPaM = CursorDown( aPaM, pEditView );
                            break;
        case KEY_LEFT:      aPaM = bCtrl ? WordLeft( aPaM ) : CursorLeft( aPaM );
                            break;
        case KEY_RIGHT:     aPaM = bCtrl ? WordRight( aPaM ) : CursorRight( aPaM );
                            break;
        case KEY_HOME:      aPaM = bCtrl ? CursorStartOfDoc() : CursorStartOfLine( aPaM );
                            break;
        case KEY_END:       aPaM = bCtrl ? CursorEndOfDoc() : CursorEndOfLine( aPaM );
                            break;
        case KEY_PAGEUP:    aPaM = bCtrl ? CursorStartOfDoc() : PageUp( aPaM, pEditView );
                            break;
        case KEY_PAGEDOWN:  aPaM = bCtrl ? CursorEndOfDoc() : PageDown( aPaM, pEditView );
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

    // Bewirkt evtl. ein CreateAnchor oder Deselection all
    aSelEngine.SetCurView( pEditView );
    aSelEngine.CursorPosChanging( rKeyEvent.GetKeyCode().IsShift(), rKeyEvent.GetKeyCode().IsMod1() );
    EditPaM aOldEnd( pEditView->pImpEditView->GetEditSelection().Max() );
    pEditView->pImpEditView->GetEditSelection().Max() = aPaM;
    if ( rKeyEvent.GetKeyCode().IsShift() )
    {
        // Dann wird die Selektion erweitert...
        EditSelection aTmpNewSel( aOldEnd, aPaM );
        pEditView->pImpEditView->DrawSelection( aTmpNewSel );
    }
    else
        pEditView->pImpEditView->GetEditSelection().Min() = aPaM;

    return pEditView->pImpEditView->GetEditSelection();
}

EditPaM ImpEditEngine::CursorLeft( const EditPaM& rPaM )
{
    EditPaM aNewPaM( rPaM );

    if ( rPaM.GetIndex() )
    {
        uno::Reference < i18n::XBreakIterator > xBI = ImplGetBreakIterator();
        sal_Int32 nCount = 1;
        aNewPaM.SetIndex( (USHORT)xBI->previousCharacters( *aNewPaM.GetNode(), aNewPaM.GetIndex(), GetLocale( aNewPaM ), i18n::CharacterIteratorMode::SKIPCHARACTER, nCount, nCount ) );
    }
    else
    {
        ContentNode* pNode = rPaM.GetNode();
        pNode = GetPrevVisNode( pNode );
        if ( pNode )
        {
            aNewPaM.SetNode( pNode );
            aNewPaM.SetIndex( pNode->Len() );
        }
    }

    return aNewPaM;
}

EditPaM ImpEditEngine::CursorRight( const EditPaM& rPaM )
{
    EditPaM aNewPaM( rPaM );
    if ( rPaM.GetIndex() < rPaM.GetNode()->Len() )
    {
        uno::Reference < i18n::XBreakIterator > xBI = ImplGetBreakIterator();
        sal_Int32 nCount = 1;
        aNewPaM.SetIndex( (USHORT)xBI->nextCharacters( *aNewPaM.GetNode(), aNewPaM.GetIndex(), GetLocale( aNewPaM ), i18n::CharacterIteratorMode::SKIPCHARACTER, nCount, nCount ) );
    }
    else
    {
        ContentNode* pNode = rPaM.GetNode();
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
    DBG_ASSERT( pView, "Keine View - Keine Cursorbewegung!" );

    ParaPortion* pPPortion = FindParaPortion( rPaM.GetNode() );
    DBG_ASSERT( pPPortion, "Keine passende Portion gefunden: CursorUp" );
    USHORT nLine = pPPortion->GetLineNumber( rPaM.GetIndex() );
    EditLine* pLine = pPPortion->GetLines().GetObject( nLine );

    long nX;
    if ( pView->pImpEditView->nTravelXPos == TRAVEL_X_DONTKNOW )
    {
        nX = pPPortion->GetXPos( pLine, rPaM.GetIndex() );
        pView->pImpEditView->nTravelXPos = nX+nOnePixelInRef;
    }
    else
        nX = pView->pImpEditView->nTravelXPos;

    EditPaM aNewPaM( rPaM );
    if ( nLine )    // gleicher Absatz
    {
        EditLine* pPrevLine = pPPortion->GetLines().GetObject(nLine-1);
        aNewPaM.SetIndex( pPPortion->GetChar( pPrevLine, nX ) );
        // Wenn davor eine autom.Umgebrochene Zeile, und ich muss genau an das
        // Ende dieser Zeile, landet der Cursor in der aktuellen Zeile am Anfang
        // Siehe Problem: Letztes Zeichen einer autom.umgebr. Zeile = Cursor
        if ( aNewPaM.GetIndex() && ( aNewPaM.GetIndex() == pLine->GetStart() ) )
            aNewPaM = CursorLeft( aNewPaM );
    }
    else    // vorheriger Absatz
    {
        ParaPortion* pPrevPortion = GetPrevVisPortion( pPPortion );
        if ( pPrevPortion )
        {
            pLine = pPrevPortion->GetLines().GetObject( pPrevPortion->GetLines().Count()-1 );
            DBG_ASSERT( pLine, "Zeile davor nicht gefunden: CursorUp" );
            aNewPaM.SetNode( pPrevPortion->GetNode() );
            aNewPaM.SetIndex( pPrevPortion->GetChar( pLine, nX+nOnePixelInRef ) );
        }
    }

    return aNewPaM;
}

EditPaM ImpEditEngine::CursorDown( const EditPaM& rPaM, EditView* pView )
{
    DBG_ASSERT( pView, "Keine View - Keine Cursorbewegung!" );

    ParaPortion* pPPortion = FindParaPortion( rPaM.GetNode() );
    DBG_ASSERT( pPPortion, "Keine passende Portion gefunden: CursorDown" );
    USHORT nLine = pPPortion->GetLineNumber( rPaM.GetIndex() );

    long nX;
    if ( pView->pImpEditView->nTravelXPos == TRAVEL_X_DONTKNOW )
    {
        EditLine* pLine = pPPortion->GetLines().GetObject(nLine);
        nX = pPPortion->GetXPos( pLine, rPaM.GetIndex() );
        pView->pImpEditView->nTravelXPos = nX+nOnePixelInRef;
    }
    else
        nX = pView->pImpEditView->nTravelXPos;

    EditPaM aNewPaM( rPaM );
    if ( nLine < pPPortion->GetLines().Count()-1 )
    {
        EditLine* pNextLine = pPPortion->GetLines().GetObject(nLine+1);
        aNewPaM.SetIndex( pPPortion->GetChar( pNextLine, nX ) );
        // Sonderbehandlung siehe CursorUp...
        if ( ( aNewPaM.GetIndex() == pNextLine->GetEnd() ) && ( aNewPaM.GetIndex() > pNextLine->GetStart() ) && ( aNewPaM.GetIndex() < pPPortion->GetNode()->Len() ) )
            aNewPaM = CursorLeft( aNewPaM );
    }
    else    // naechster Absatz
    {
        ParaPortion* pNextPortion = GetNextVisPortion( pPPortion );
        if ( pNextPortion )
        {
            EditLine* pLine = pNextPortion->GetLines().GetObject(0);
            DBG_ASSERT( pLine, "Zeile davor nicht gefunden: CursorUp" );
            aNewPaM.SetNode( pNextPortion->GetNode() );
            // Nie ganz ans Ende wenn mehrere Zeilen, da dann eine
            // Zeile darunter der Cursor angezeigt wird.
            aNewPaM.SetIndex( pNextPortion->GetChar( pLine, nX+nOnePixelInRef ) );
            if ( ( aNewPaM.GetIndex() == pLine->GetEnd() ) && ( aNewPaM.GetIndex() > pLine->GetStart() ) && ( pNextPortion->GetLines().Count() > 1 ) )
                aNewPaM = CursorLeft( aNewPaM );
        }
    }

    return aNewPaM;
}

EditPaM ImpEditEngine::CursorStartOfLine( const EditPaM& rPaM )
{
    ParaPortion* pCurPortion = FindParaPortion( rPaM.GetNode() );
    DBG_ASSERT( pCurPortion, "Keine Portion fuer den PaM ?" );
    USHORT nLine = pCurPortion->GetLineNumber( rPaM.GetIndex() );
    EditLine* pLine = pCurPortion->GetLines().GetObject(nLine);
    DBG_ASSERT( pLine, "Aktuelle Zeile nicht gefunden ?!" );

    EditPaM aNewPaM( rPaM );
    aNewPaM.SetIndex( pLine->GetStart() );
    return aNewPaM;
}

EditPaM ImpEditEngine::CursorEndOfLine( const EditPaM& rPaM )
{
    ParaPortion* pCurPortion = FindParaPortion( rPaM.GetNode() );
    DBG_ASSERT( pCurPortion, "Keine Portion fuer den PaM ?" );
    USHORT nLine = pCurPortion->GetLineNumber( rPaM.GetIndex() );
    EditLine* pLine = pCurPortion->GetLines().GetObject(nLine);
    DBG_ASSERT( pLine, "Aktuelle Zeile nicht gefunden ?!" );

    EditPaM aNewPaM( rPaM );
    aNewPaM.SetIndex( pLine->GetEnd() );
    if ( pLine->GetEnd() > pLine->GetStart() )
    {
        xub_Unicode cLastChar = aNewPaM.GetNode()->GetChar( aNewPaM.GetIndex()-1 );
        if ( aNewPaM.GetNode()->IsFeature( aNewPaM.GetIndex() - 1 ) )
        {
            // Bei einem weichen Umbruch muss ich davor stehen!
            EditCharAttrib* pNextFeature = aNewPaM.GetNode()->GetCharAttribs().FindFeature( aNewPaM.GetIndex()-1 );
            if ( pNextFeature && ( pNextFeature->GetItem()->Which() == EE_FEATURE_LINEBR ) )
                aNewPaM = CursorLeft( aNewPaM );
        }
        else if ( ( aNewPaM.GetNode()->GetChar( aNewPaM.GetIndex() - 1 ) == ' ' ) && ( aNewPaM.GetIndex() != aNewPaM.GetNode()->Len() ) )
        {
            // Bei einem Blank in einer autom. umgebrochenen Zeile macht es Sinn,
            // davor zu stehen, da der Anwender hinter das Wort will.
            // Wenn diese geaendert wird, Sonderbehandlung fuer Pos1 nach End!
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
    DBG_ASSERT( pLastNode && pLastPortion, "CursorEndOfDoc: Node oder Portion nicht gefunden" );

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
        // Vorheriger Absatz...
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
        uno::Reference < i18n::XBreakIterator > xBI = ImplGetBreakIterator();
        i18n::Boundary aBoundary = xBI->getWordBoundary( *aNewPaM.GetNode(), nCurrentPos, GetLocale( EditPaM( aNewPaM.GetNode(), nCurrentPos ) ), nWordType, sal_True );
        if ( aBoundary.startPos == nCurrentPos )
            aBoundary = xBI->previousWord( *aNewPaM.GetNode(), nCurrentPos, GetLocale( EditPaM( aNewPaM.GetNode(), nCurrentPos ) ), nWordType );
        aNewPaM.SetIndex( (USHORT)aBoundary.startPos );
    }

    return aNewPaM;
}

EditPaM ImpEditEngine::WordRight( const EditPaM& rPaM, sal_Int16 nWordType )
{
    xub_StrLen nMax = rPaM.GetNode()->Len();
    EditPaM aNewPaM( rPaM );
    if ( aNewPaM.GetIndex() < nMax )
    {
        uno::Reference < i18n::XBreakIterator > xBI = ImplGetBreakIterator();
        i18n::Boundary aBoundary = xBI->nextWord( *aNewPaM.GetNode(), aNewPaM.GetIndex(), GetLocale( aNewPaM ), nWordType );
        aNewPaM.SetIndex( (USHORT)aBoundary.startPos );
    }
    // not 'else', maybe the index reached nMax now...
    if ( aNewPaM.GetIndex() >= nMax )
    {
        // Naechster Absatz...
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
    uno::Reference < i18n::XBreakIterator > xBI = ImplGetBreakIterator();
    i18n::Boundary aBoundary = xBI->getWordBoundary( *rPaM.GetNode(), rPaM.GetIndex(), GetLocale( rPaM ), nWordType, sal_True );
    aNewPaM.SetIndex( (USHORT)aBoundary.startPos );
    return aNewPaM;
}

EditPaM ImpEditEngine::EndOfWord( const EditPaM& rPaM, sal_Int16 nWordType )
{
    EditPaM aNewPaM( rPaM );
    uno::Reference < i18n::XBreakIterator > xBI = ImplGetBreakIterator();
    i18n::Boundary aBoundary = xBI->getWordBoundary( *rPaM.GetNode(), rPaM.GetIndex(), GetLocale( rPaM ), nWordType, sal_True );
    aNewPaM.SetIndex( (USHORT)aBoundary.endPos );
    return aNewPaM;
}

EditSelection ImpEditEngine::SelectWord( const EditSelection& rCurSel, sal_Int16 nWordType )
{
    EditSelection aNewSel( rCurSel );
    EditPaM aPaM( rCurSel.Max() );
    uno::Reference < i18n::XBreakIterator > xBI = ImplGetBreakIterator();
    sal_Int16 nType = xBI->getWordType( *aPaM.GetNode(), aPaM.GetIndex(), GetLocale( aPaM ) );
    if ( nType == i18n::WordType::ANY_WORD )
    {
        i18n::Boundary aBoundary = xBI->getWordBoundary( *aPaM.GetNode(), aPaM.GetIndex(), GetLocale( aPaM ), nWordType, sal_True );
        aNewSel.Min().SetIndex( (USHORT)aBoundary.startPos );
        aNewSel.Max().SetIndex( (USHORT)aBoundary.endPos );
    }

    return aNewSel;
}

void ImpEditEngine::InitScriptTypes( USHORT nPara )
{
    ParaPortion* pParaPortion = GetParaPortions().SaveGetObject( nPara );
    ScriptTypePosInfos& rTypes = pParaPortion->aScriptInfos;
    rTypes.Remove( 0, rTypes.Count() );

    ContentNode* pNode = pParaPortion->GetNode();
    if ( pNode->Len() )
    {
        uno::Reference < i18n::XBreakIterator > xBI = ImplGetBreakIterator();
        String aText( *pNode );

        // Fields?
        EditCharAttrib* pField = pNode->GetCharAttribs().FindNextAttrib( EE_FEATURE_FIELD, 0 );
        while ( pField )
        {
            String aFieldValue = ((EditCharAttribField*)pField)->GetFieldValue();
            if ( aFieldValue.Len() )
            {
                // First char from field wins...
                aText.SetChar( pField->GetStart(), aFieldValue.GetChar(0) );
            }
            pField = pNode->GetCharAttribs().FindNextAttrib( EE_FEATURE_FIELD, pField->GetEnd() );
        }

        ::rtl::OUString aOUText( aText );
        USHORT nTextLen = (USHORT)aOUText.getLength();

        long nPos = 0;
        short nScriptType = xBI->getScriptType( aOUText, nPos );
        rTypes.Insert( ScriptTypePosInfo( nScriptType, (USHORT)nPos, nTextLen ), rTypes.Count() );
        nPos = xBI->endOfScript( aOUText, nPos, nScriptType );
        while ( ( nPos != (-1) ) && ( nPos < nTextLen ) )
        {
            rTypes[rTypes.Count()-1].nEndPos = (USHORT)nPos;

            nScriptType = xBI->getScriptType( aOUText, nPos );
            if ( nScriptType == i18n::ScriptType::WEAK )
                nScriptType = rTypes[rTypes.Count()-1].nScriptType;

            rTypes.Insert( ScriptTypePosInfo( nScriptType, (USHORT)nPos, nTextLen ), rTypes.Count() );
            nPos = xBI->endOfScript( aOUText, nPos, nScriptType );
        }

        if ( rTypes[0].nScriptType == i18n::ScriptType::WEAK )
            rTypes[0].nScriptType = ( rTypes.Count() > 1 ) ? rTypes[1].nScriptType : i18n::ScriptType::LATIN;
    }
}

USHORT ImpEditEngine::GetScriptType( const EditPaM& rPaM, USHORT* pEndPos ) const
{
    USHORT nScriptType = i18n::ScriptType::LATIN;

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
    return nScriptType;
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

        USHORT nS = ( nPara == nStartPara ) ? aSel.Min().GetIndex() : 0;
        USHORT nE = ( nPara == nEndPara ) ? aSel.Max().GetIndex() : pParaPortion->GetNode()->Len();
        for ( USHORT n = 0; n < rTypes.Count(); n++ )
        {
            if ( ( rTypes[n].nStartPos <= nE ) && ( rTypes[n].nEndPos >= nS ) )
               {
                switch ( rTypes[n].nScriptType )
                {
                    case i18n::ScriptType::LATIN:
                        nScriptType |= SCRIPTTYPE_LATIN;
                        break;
                    case i18n::ScriptType::ASIAN:
                        nScriptType |= SCRIPTTYPE_ASIAN;
                        break;
                    case i18n::ScriptType::COMPLEX:
                        nScriptType |= SCRIPTTYPE_COMPLEX;
                        break;
                }
            }
        }
    }
    return nScriptType;
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



//  ----------------------------------------------------------------------
//  Textaenderung
//  ----------------------------------------------------------------------

void ImpEditEngine::ImpRemoveChars( const EditPaM& rPaM, USHORT nChars, EditUndoRemoveChars* pCurUndo )
{
    if ( IsUndoEnabled() && !IsInUndo() )
    {
        XubString aStr( rPaM.GetNode()->Copy( rPaM.GetIndex(), nChars ) );

        // Pruefen, ob Attribute geloescht oder geaendert werden:
        USHORT nStart = rPaM.GetIndex();
        USHORT nEnd = nStart + nChars;
        CharAttribArray& rAttribs = rPaM.GetNode()->GetCharAttribs().GetAttribs();
        USHORT nAttrs = rAttribs.Count();
        for ( USHORT nAttr = 0; nAttr < rAttribs.Count(); nAttr++ )
        {
            EditCharAttrib* pAttr = rAttribs[nAttr];
            if ( ( pAttr->GetEnd() >= nStart ) && ( pAttr->GetStart() < nEnd ) )
            {
                EditSelection aSel( rPaM );
                aSel.Max().GetIndex() += nChars;
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
    DBG_ASSERT( bValidAction, "Move in sich selbst ?" );
    DBG_ASSERT( aOldPositions.Max() <= (long)GetParaPortions().Count(), "Voll drueber weg: MoveParagraphs" );

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
        pRecalc4 = GetParaPortions().GetObject( aOldPositions.Max()+1 );
    }
    else if ( (USHORT)aOldPositions.Max() == (nParaCount-1) )
    {
        pRecalc3 = GetParaPortions().GetObject( (USHORT)aOldPositions.Max() );
        pRecalc4 = GetParaPortions().GetObject( (USHORT)(aOldPositions.Min()-1) );
    }

    if ( IsUndoEnabled() && !IsInUndo())
        InsertUndo( new EditUndoMoveParagraphs( this, aOldPositions, nNewPos ) );

    // Position nicht aus dem Auge verlieren!
    ParaPortion* pDestPortion = GetParaPortions().SaveGetObject( nNewPos );

    ParaPortionList aTmpPortionList;
    USHORT i;
    for ( i = (USHORT)aOldPositions.Min(); i <= (USHORT)aOldPositions.Max(); i++  )
    {
        // Immer aOldPositions.Min(), da Remove().
        ParaPortion* pTmpPortion = GetParaPortions().GetObject( (USHORT)aOldPositions.Min() );
        GetParaPortions().Remove( (USHORT)aOldPositions.Min() );
        aEditDoc.Remove( (USHORT)aOldPositions.Min() );
        aTmpPortionList.Insert( pTmpPortion, aTmpPortionList.Count() );
    }

    USHORT nRealNewPos = pDestPortion ? GetParaPortions().GetPos( pDestPortion ) : GetParaPortions().Count();
    DBG_ASSERT( nRealNewPos != USHRT_MAX, "ImpMoveParagraphs: Ungueltige Position!" );

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

    aEditDoc.SetModified( TRUE );

    if ( pRecalc1 )
        CalcHeight( pRecalc1 );
    if ( pRecalc2 )
        CalcHeight( pRecalc2 );
    if ( pRecalc3 )
        CalcHeight( pRecalc3 );
    if ( pRecalc4 )
        CalcHeight( pRecalc4 );

    aTmpPortionList.Remove( 0, aTmpPortionList.Count() );   // wichtig !

#ifdef EDITDEBUG
    GetParaPortions().DbgCheck(aEditDoc);
#endif
    return aSelection;
}


EditPaM ImpEditEngine::ImpConnectParagraphs( ContentNode* pLeft, ContentNode* pRight, BOOL bBackward )
{
    DBG_ASSERT( pLeft != pRight, "Den gleichen Absatz zusammenfuegen ?" );
    DBG_ASSERT( aEditDoc.GetPos( pLeft ) != USHRT_MAX, "Einzufuegenden Node nicht gefunden(1)" );
    DBG_ASSERT( aEditDoc.GetPos( pRight ) != USHRT_MAX, "Einzufuegenden Node nicht gefunden(2)" );

    USHORT nParagraphTobeDeleted = aEditDoc.GetPos( pRight );
    DeletedNodeInfo* pInf = new DeletedNodeInfo( (ULONG)pRight, nParagraphTobeDeleted );
    aDeletedNodes.Insert( pInf, aDeletedNodes.Count() );

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

    // Erstmal Portions suchen, da pRight nach ConnectParagraphs weg.
    ParaPortion* pLeftPortion = FindParaPortion( pLeft );
    ParaPortion* pRightPortion = FindParaPortion( pRight );
    DBG_ASSERT( pLeftPortion, "Blinde Portion in ImpConnectParagraphs(1)" );
    DBG_ASSERT( pRightPortion, "Blinde Portion in ImpConnectParagraphs(2)" );

#ifndef SVX_LIGHT
    if ( GetStatus().DoOnlineSpelling() )
    {
        xub_StrLen nEnd = pLeft->Len();
        xub_StrLen nInv = nEnd ? nEnd-1 : nEnd;
        pLeft->GetWrongList()->ClearWrongs( nInv, 0xFFFF, pLeft );  // Evtl. einen wegnehmen
        pLeft->GetWrongList()->MarkInvalid( nInv, nEnd+1 );
        // Falschgeschriebene Woerter ruebernehmen:
        USHORT nRWrongs = pRight->GetWrongList()->Count();
        for ( USHORT nW = 0; nW < nRWrongs; nW++ )
        {
            WrongRange aWrong = pRight->GetWrongList()->GetObject( nW );
            if ( aWrong.nStart != 0 )   // Nicht ein anschliessender
            {
                aWrong.nStart += nEnd;
                aWrong.nEnd += nEnd;
                pLeft->GetWrongList()->InsertWrong( aWrong, pLeft->GetWrongList()->Count() );
            }
        }
    }
#endif

    EditPaM aPaM = aEditDoc.ConnectParagraphs( pLeft, pRight );
    if ( IsCallParaInsertedOrDeleted() )
        GetEditEnginePtr()->ParagraphDeleted( nParagraphTobeDeleted );

    pLeftPortion->MarkSelectionInvalid( aPaM.GetIndex(), pLeft->Len() );

    DBG_ASSERT( nParagraphTobeDeleted == GetParaPortions().GetPos( pRightPortion ), "NodePos != PortionPos?" );
    GetParaPortions().Remove( nParagraphTobeDeleted );
    delete pRightPortion;
    // der rechte Node wird von EditDoc::ConnectParagraphs() geloescht.

    if ( GetTextRanger() )
    {
        // Durch das zusammenfuegen wird der linke zwar neu formatiert, aber
        // wenn sich dessen Hoehe nicht aendert bekommt die Formatierung die
        // Aenderung der Gesaamthoehe des Textes zu spaet mit...
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
    DBG_ASSERT( !EditSelection( rSel ).DbgIsBuggy( aEditDoc ), "Index im Wald in DeleteLeftOrRight" )

    if ( rSel.HasRange() )  // dann nur Sel. loeschen
        return ImpDeleteSelection( rSel );

    const EditPaM aCurPos( rSel.Max() );
    EditPaM aDelStart( aCurPos );
    EditPaM aDelEnd( aCurPos );
    if ( nMode == DEL_LEFT )
    {
        if ( nDelMode == DELMODE_SIMPLE )
        {
            aDelStart = CursorLeft( aCurPos );
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
                // kompletter Absatz davor
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
            if ( aDelEnd.GetIndex() == aCurPos.GetIndex() )
                aDelEnd = WordLeft( aCurPos );
        }
        else    // DELMODE_RESTOFCONTENT
        {
            aDelEnd.SetIndex( aCurPos.GetNode()->Len() );
            if ( aDelEnd == aCurPos )
            {
                // kompletter Absatz dahinter
                ContentNode* pNext = GetNextVisNode( aCurPos.GetNode() );
                if ( pNext )
                    aDelEnd = EditPaM( pNext, pNext->Len() );
            }
        }
    }

    // Bei DELMODE_RESTOFCONTENT reicht bei verschiedenen Nodes
    // kein ConnectParagraphs.
    if ( ( nDelMode == DELMODE_RESTOFCONTENT ) || ( aDelStart.GetNode() == aDelEnd.GetNode() ) )
        return ImpDeleteSelection( EditSelection( aDelStart, aDelEnd ) );

    // Jetzt entscheiden, ob noch Selektion loeschen (RESTOFCONTENTS)
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

    CursorMoved( aStartPaM.GetNode() ); // nur damit neu eingestellte Attribute verschwinden...
    CursorMoved( aEndPaM.GetNode() );   // nur damit neu eingestellte Attribute verschwinden...

    DBG_ASSERT( aStartPaM.GetIndex() <= aStartPaM.GetNode()->Len(), "Index im Wald in ImpDeleteSelection" )
    DBG_ASSERT( aEndPaM.GetIndex() <= aEndPaM.GetNode()->Len(), "Index im Wald in ImpDeleteSelection" )

    USHORT nStartNode = aEditDoc.GetPos( aStartPaM.GetNode() );
    USHORT nEndNode = aEditDoc.GetPos( aEndPaM.GetNode() );

    DBG_ASSERT( nEndNode != USHRT_MAX, "Start > End ?!" );
    DBG_ASSERT( nStartNode <= nEndNode, "Start > End ?!" );

    // Alle Nodes dazwischen entfernen....
    for ( ULONG z = nStartNode+1; z < nEndNode; z++ )
    {
        // Immer nStartNode+1, wegen Remove()!
        ImpRemoveParagraph( nStartNode+1 );
    }

    if ( aStartPaM.GetNode() != aEndPaM.GetNode() )
    {
        // Den Rest des StartNodes...
        USHORT nChars;
        nChars = aStartPaM.GetNode()->Len() - aStartPaM.GetIndex();
        ImpRemoveChars( aStartPaM, nChars );
        ParaPortion* pPortion = FindParaPortion( aStartPaM.GetNode() );
        DBG_ASSERT( pPortion, "Blinde Portion in ImpDeleteSelection(3)" );
        pPortion->MarkSelectionInvalid( aStartPaM.GetIndex(), aStartPaM.GetNode()->Len() );

        // Den Anfang des EndNodes....
        nChars = aEndPaM.GetIndex();
        aEndPaM.SetIndex( 0 );
        ImpRemoveChars( aEndPaM, nChars );
        pPortion = FindParaPortion( aEndPaM.GetNode() );
        DBG_ASSERT( pPortion, "Blinde Portion in ImpDeleteSelection(4)" );
        pPortion->MarkSelectionInvalid( 0, aEndPaM.GetNode()->Len() );
        // Zusammenfuegen....
        aStartPaM = ImpConnectParagraphs( aStartPaM.GetNode(), aEndPaM.GetNode() );
    }
    else
    {
        USHORT nChars;
        nChars = aEndPaM.GetIndex() - aStartPaM.GetIndex();
        ImpRemoveChars( aStartPaM, nChars );
        ParaPortion* pPortion = FindParaPortion( aStartPaM.GetNode() );
        DBG_ASSERT( pPortion, "Blinde Portion in ImpDeleteSelection(5)" );
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

    DBG_ASSERT( pNode, "Blinder Node in ImpRemoveParagraph" );
    DBG_ASSERT( pPortion, "Blinde Portion in ImpRemoveParagraph(2)" );

    DeletedNodeInfo* pInf = new DeletedNodeInfo( (ULONG)pNode, nPara );
    aDeletedNodes.Insert( pInf, aDeletedNodes.Count() );

    // Der Node wird vom Undo verwaltet und ggf. zerstoert!
    /* delete */ aEditDoc.Remove( nPara );
    GetParaPortions().Remove( nPara );
    delete pPortion;
    if ( IsCallParaInsertedOrDeleted() )
        GetEditEnginePtr()->ParagraphDeleted( nPara );

    // Im folgenden muss ggf. Extra-Space neu ermittelt werden.
    // Bei ParaAttribsChanged wird leider der Absatz neu formatiert,
    // aber diese Methode sollte nicht Zeitkritsch sein!
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

EditPaM ImpEditEngine::AutoCorrect( const EditSelection& rCurSel, xub_Unicode c, BOOL bOverwrite )
{
    EditSelection aSel( rCurSel );
#ifndef SVX_LIGHT
    SvxAutoCorrect* pAutoCorrect = EE_DLL()->GetGlobalData()->GetAutoCorrect();
    if ( pAutoCorrect )
    {
        if ( aSel.HasRange() )
            aSel = ImpDeleteSelection( rCurSel );
        ContentNode* pNode = aSel.Max().GetNode();
        USHORT nIndex = aSel.Max().GetIndex();
        EdtAutoCorrDoc aAuto( this, pNode, nIndex, c );
        pAutoCorrect->AutoCorrect( aAuto, *pNode, nIndex, c, !bOverwrite );
        aSel.Max().SetIndex( aAuto.GetCursor() );
    }
#endif // !SVX_LIGHT
    return aSel.Max();
}


EditPaM ImpEditEngine::InsertText( const EditSelection& rCurSel, xub_Unicode c, BOOL bOverwrite )
{
    DBG_ASSERT( c != '\t', "Tab bei InsertText ?" );
    DBG_ASSERT( c != '\n', "Zeilenumbruch bei InsertText ?" );

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
        // Wenn Selektion, dann nicht auch noch ein Zeichen ueberschreiben!
        EditSelection aTmpSel( aPaM );
        aTmpSel.Max().GetIndex()++;
        DBG_ASSERT( !aTmpSel.DbgIsBuggy( aEditDoc ), "Overwrite: Fehlerhafte Selektion!" );
        ImpDeleteSelection( aTmpSel );
    }

    if ( aPaM.GetNode()->Len() < MAXCHARSINPARA )
    {
        if ( IsUndoEnabled() && !IsInUndo() )
        {
                EditUndoInsertChars* pNewUndo = new EditUndoInsertChars( this, CreateEPaM( aPaM ), c );
                BOOL bTryMerge = ( !bDoOverwrite && ( c != ' ' ) ) ? TRUE : FALSE;
                InsertUndo( pNewUndo, bTryMerge );
        }

        aEditDoc.InsertText( (const EditPaM&)aPaM, c );
        ParaPortion* pPortion = FindParaPortion( aPaM.GetNode() );
        DBG_ASSERT( pPortion, "Blinde Portion in InsertText" );
        pPortion->MarkInvalid( aPaM.GetIndex(), 1 );
        aPaM.GetIndex()++;  // macht EditDoc-Methode nicht mehr
    }

    TextModified();

    if ( bUndoAction )
        UndoActionEnd( EDITUNDO_INSERT );

    return aPaM;
}

EditPaM ImpEditEngine::ImpInsertText( EditSelection aCurSel, const XubString& rStr )
{
    EditPaM aPaM;
    if ( aCurSel.HasRange() )
        aPaM = ImpDeleteSelection( aCurSel );
    else
        aPaM = aCurSel.Max();

    EditPaM aCurPaM( aPaM );    // fuers Invalidieren

    XubString aText( rStr );
    aText.ConvertLineEnd( LINEEND_LF );
    SfxVoidItem aTabItem( EE_FEATURE_TAB );

    // Konvertiert nach LineSep = \n
    // Token mit LINE_SEP abfragen,
    // da der MAC-Compiler aus \n etwas anderes macht!

    USHORT nStart = 0;
    while ( nStart < aText.Len() )
    {
        USHORT nEnd = aText.Search( LINE_SEP, nStart );
        if ( nEnd == STRING_NOTFOUND )
            nEnd = aText.Len(); // nicht dereferenzieren!

        // Start == End => Leerzeile
        if ( nEnd > nStart )
        {
            XubString aLine( aText, nStart, nEnd-nStart );
            xub_StrLen nChars = aPaM.GetNode()->Len() + aLine.Len();
            if ( nChars > MAXCHARSINPARA )
            {
                DBG_ERROR( "Info: MaxChars reached !" );
                aLine.Erase( MAXCHARSINPARA-aPaM.GetNode()->Len() );
                nEnd -= ( nChars - MAXCHARSINPARA );    // Dann landen die Zeichen im naechsten Absatz.
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
                        nEnd2 = aLine.Len();    // nicht dereferenzieren!

                    if ( nEnd2 > nStart2 )
                        aPaM = aEditDoc.InsertText( aPaM, XubString( aLine, nStart2, nEnd2-nStart2 ) );
                    if ( nEnd2 < aLine.Len() )
                    {
                        // aPaM = ImpInsertFeature( EditSelection( aPaM, aPaM ),  );
                        aPaM = aEditDoc.InsertFeature( aPaM, aTabItem );
                    }
                    nStart2 = nEnd2+1;
                }
            }
            ParaPortion* pPortion = FindParaPortion( aPaM.GetNode() );
            DBG_ASSERT( pPortion, "Blinde Portion in InsertText" );
            pPortion->MarkInvalid( aCurPaM.GetIndex(), aLine.Len() );
        }
        if ( nEnd < aText.Len() )
            aPaM = ImpInsertParaBreak( aPaM );

        nStart = nEnd+1;
    }

    TextModified();
    return aPaM;
}

EditPaM ImpEditEngine::ImpFastInsertText( EditPaM aPaM, const XubString& rStr )
{
    DBG_ASSERT( rStr.Search( 0x0A ) == STRING_NOTFOUND, "FastInsertText: Zeilentrenner nicht erlaubt!" );
    DBG_ASSERT( rStr.Search( 0x0D ) == STRING_NOTFOUND, "FastInsertText: Zeilentrenner nicht erlaubt!" );
    DBG_ASSERT( rStr.Search( '\t' ) == STRING_NOTFOUND, "FastInsertText: Features nicht erlaubt!" );

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

    if ( IsUndoEnabled() && !IsInUndo() )
        InsertUndo( new EditUndoInsertFeature( this, CreateEPaM( aPaM ), rItem ) );
    aPaM = aEditDoc.InsertFeature( aPaM, rItem );

    ParaPortion* pPortion = FindParaPortion( aPaM.GetNode() );
    DBG_ASSERT( pPortion, "Blinde Portion in InsertFeature" );
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
    if ( IsUndoEnabled() && !IsInUndo() )
        InsertUndo( new EditUndoSplitPara( this, aEditDoc.GetPos( rPaM.GetNode() ), rPaM.GetIndex() ) );

    EditPaM aPaM( aEditDoc.InsertParaBreak( rPaM, bKeepEndingAttribs ) );

#ifndef SVX_LIGHT
    if ( GetStatus().DoOnlineSpelling() )
    {
        xub_StrLen nEnd = rPaM.GetNode()->Len();
        aPaM.GetNode()->CreateWrongList();
        WrongList* pLWrongs = rPaM.GetNode()->GetWrongList();
        WrongList* pRWrongs = aPaM.GetNode()->GetWrongList();
        // Falschgeschriebene Woerter ruebernehmen:
        USHORT nLWrongs = pLWrongs->Count();
        for ( USHORT nW = 0; nW < nLWrongs; nW++ )
        {
            WrongRange& rWrong = pLWrongs->GetObject( nW );
            // Nur wenn wirklich dahinter, ein ueberlappendes wird beim Spell korrigiert
            if ( rWrong.nStart > nEnd )
            {
                pRWrongs->InsertWrong( rWrong, pRWrongs->Count() );
                WrongRange& rRWrong = pRWrongs->GetObject( pRWrongs->Count() - 1 );
                rRWrong.nStart -= nEnd;
                rRWrong.nEnd -= nEnd;
            }
            else if ( ( rWrong.nStart < nEnd ) && ( rWrong.nEnd > nEnd ) )
                rWrong.nEnd = nEnd;
        }
        USHORT nInv = nEnd ? nEnd-1 : nEnd;
        if ( nEnd )
            pLWrongs->MarkInvalid( nInv, nEnd );
        else
            pLWrongs->SetValid();
        pRWrongs->SetValid(); // sonst 0 - 0xFFFF
        pRWrongs->MarkInvalid( 0, 1 );  // Nur das erste Wort testen
    }
#endif // !SVX_LIGHT


    ParaPortion* pPortion = FindParaPortion( rPaM.GetNode() );
    DBG_ASSERT( pPortion, "Blinde Portion in ImpInsertParaBreak" );
    pPortion->MarkInvalid( rPaM.GetIndex(), 0 );

    // Optimieren: Nicht unnoetig viele GetPos auf die Listen ansetzen!
    // Hier z.B. bei Undo, aber auch in allen anderen Methoden.
    USHORT nPos = GetParaPortions().GetPos( pPortion );
    ParaPortion* pNewPortion = new ParaPortion( aPaM.GetNode() );
    GetParaPortions().Insert( pNewPortion, nPos + 1 );
    ParaAttribsChanged( pNewPortion->GetNode() );
    if ( IsCallParaInsertedOrDeleted() )
        GetEditEnginePtr()->ParagraphInserted( nPos+1 );

    CursorMoved( rPaM.GetNode() );  // falls leeres Attribut entstanden.
    TextModified();
    return aPaM;
}

EditPaM ImpEditEngine::ImpFastInsertParagraph( USHORT nPara )
{
    if ( IsUndoEnabled() && !IsInUndo() )
    {
        if ( nPara )
        {
            DBG_ASSERT( aEditDoc.SaveGetObject( nPara-1 ), "FastInsertParagraph: Prev existiert nicht" );
            InsertUndo( new EditUndoSplitPara( this, nPara-1, aEditDoc.GetObject( nPara-1 )->Len() ) );
        }
        else
            InsertUndo( new EditUndoSplitPara( this, 0, 0 ) );
    }

    ContentNode* pNode = new ContentNode( aEditDoc.GetItemPool() );
    // Falls FlatMode, wird spaeter kein Font eingestellt:
    pNode->GetCharAttribs().GetDefFont() = aEditDoc.GetDefFont();

#ifndef SVX_LIGHT
    if ( GetStatus().DoOnlineSpelling() )
        pNode->CreateWrongList();
#endif // !SVX_LIGHT

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
        DBG_ASSERT( nPara > 0, "AutoIndenting: Fehler!" );
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
        DBG_ASSERT( pNode, "NULL-Pointer im Doc" );
        CharAttribArray& rAttribs = pNode->GetCharAttribs().GetAttribs();
        USHORT nAttrs = rAttribs.Count();
        for ( USHORT nAttr = 0; nAttr < rAttribs.Count(); nAttr++ )
        {
            EditCharAttrib* pAttr = rAttribs[nAttr];
            if ( pAttr->Which() == EE_FEATURE_FIELD )
            {
                EditCharAttribField* pField = (EditCharAttribField*)pAttr;
                EditCharAttribField* pCurrent = new EditCharAttribField( *pField );
                pField->Reset();
                // Felder sind per default grau.
                if ( aStatus.MarkFields() )
                    pField->GetFldColor() = new Color( COL_LIGHTGRAY );

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
            // ggf. etwas genauer invalidieren.
            ParaPortion* pPortion = GetParaPortions().GetObject( nPara );
            DBG_ASSERT( pPortion, "NULL-Pointer im Doc" );
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
//  Hilfsfunktionen
//  ----------------------------------------------------------------------
Rectangle ImpEditEngine::PaMtoEditCursor( EditPaM aPaM, USHORT nFlags )
{
    DBG_ASSERT( GetUpdateMode(), "Darf bei Update=FALSE nicht erreicht werden: PaMtoEditCursor" );

    Rectangle aEditCursor;
    long nY = 0;
    for ( USHORT nPortion = 0; nPortion < GetParaPortions().Count(); nPortion++ )
    {
        ParaPortion* pPortion = GetParaPortions().GetObject(nPortion);
        ContentNode* pNode = pPortion->GetNode();
        DBG_ASSERT( pNode, "Ungueltiger Node in Portion!" );
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
    DBG_ERROR( "Portion nicht gefunden!" );
    return aEditCursor;
}

EditPaM ImpEditEngine::GetPaM( Point aDocPos, BOOL bSmart )
{
    DBG_ASSERT( GetUpdateMode(), "Darf bei Update=FALSE nicht erreicht werden: GetPaM" );

    long nY = 0;
    long nTmpHeight;
    EditPaM aPaM;
    USHORT nPortion;
    for ( nPortion = 0; nPortion < GetParaPortions().Count(); nPortion++ )
    {
        ParaPortion* pPortion = GetParaPortions().GetObject(nPortion);
        nTmpHeight = pPortion->GetHeight();     // sollte auch bei !bVisible richtig sein!
        nY += nTmpHeight;
        if ( nY > aDocPos.Y() )
        {
            nY -= nTmpHeight;
            aDocPos.Y() -= nY;
            // unsichtbare Portions ueberspringen:
            while ( pPortion && !pPortion->IsVisible() )
            {
                nPortion++;
                pPortion = GetParaPortions().SaveGetObject( nPortion );
            }
            DBG_ASSERT( pPortion, "Keinen sichtbaren Absatz gefunden: GetPaM" );
            aPaM = GetPaM( pPortion, aDocPos, bSmart );
            return aPaM;

        }
    }
    // Dann den letzten sichtbaren Suchen:
    nPortion = GetParaPortions().Count()-1;
    while ( nPortion && !GetParaPortions()[nPortion]->IsVisible() )
        nPortion--;

    DBG_ASSERT( GetParaPortions()[nPortion]->IsVisible(), "Keinen sichtbaren Absatz gefunden: GetPaM" );
    aPaM.SetNode( GetParaPortions()[nPortion]->GetNode() );
    aPaM.SetIndex( GetParaPortions()[nPortion]->GetNode()->Len() );
    return aPaM;
}

ULONG ImpEditEngine::GetTextHeight() const
{
    DBG_ASSERT( GetUpdateMode(), "Sollte bei Update=FALSE nicht verwendet werden: GetTextHeight" );
    DBG_ASSERT( IsFormatted() || IsFormatting(), "GetTextHeight: Nicht formatiert" );
    return nCurTextHeight;
}

ULONG ImpEditEngine::CalcTextWidth()
{
    // Wenn noch nicht formatiert und nicht gerade dabei.
    // Wird in der Formatierung bei AutoPageSize gerufen.
    if ( !IsFormatted() && !IsFormatting() )
        FormatDoc();

    EditLine* pLine;

    long nMaxWidth = 0;
    long nCurWidth = 0;

    // --------------------------------------------------
    // Ueber alle Absaetze...
    // --------------------------------------------------
    USHORT nParas = GetParaPortions().Count();
    USHORT nBiggestPara = 0;
    USHORT nBiggestLine = 0;
    for ( USHORT nPara = 0; nPara < nParas; nPara++ )
    {
        ParaPortion* pPortion = GetParaPortions().GetObject( nPara );
//      SvxAdjust eJustification = SVX_ADJUST_LEFT;
//      if ( !aStatus.IsOutliner() )
//          eJustification = ((const SvxAdjustItem&)pPortion->GetNode()->GetContentAttribs().GetItem( EE_PARA_JUST)).GetAdjust();
        const SvxLRSpaceItem& rLRItem = GetLRSpaceItem( pPortion->GetNode() );

        if ( pPortion->IsVisible() )
        {
            // --------------------------------------------------
            // Ueber die Zeilen des Absatzes...
            // --------------------------------------------------
            ULONG nLines = pPortion->GetLines().Count();
            for ( USHORT nLine = 0; nLine < nLines; nLine++ )
            {
                pLine = pPortion->GetLines().GetObject( nLine );
                DBG_ASSERT( pLine, "NULL-Pointer im Zeileniterator in CalcWidth" );
                // nCurWidth = pLine->GetStartPosX();
                // Bei Center oder Right haengt die breite von der
                // Papierbreite ab, hier nicht erwuenscht.
                // Am besten generell nicht auf StartPosX verlassen,
                // es muss auch die rechte Einrueckung beruecksichtigt werden!
                // if ( ( eJustification == SVX_ADJUST_CENTER ) || ( eJustification == SVX_ADJUST_RIGHT ) )
                // ...
                nCurWidth = GetXValue( rLRItem.GetTxtLeft() );
                if ( nLine == 0 )
                {
                    long nFI = GetXValue( rLRItem.GetTxtFirstLineOfst() );
                    nCurWidth += nFI;
                    if ( pPortion->GetBulletX() > nCurWidth )
                    {
                        nCurWidth -= nFI;   // LI?
                        if ( pPortion->GetBulletX() > nCurWidth )
                            nCurWidth = pPortion->GetBulletX();
                    }
                }
                nCurWidth += GetXValue( rLRItem.GetRight() );
                nCurWidth += CalcLineWidth( pPortion, pLine );
                if ( nCurWidth > nMaxWidth )
                {
                    nMaxWidth = nCurWidth;
                }
            }
        }
    }
    if ( nMaxWidth < 0 )
        nMaxWidth = 0;

    nMaxWidth++; // Ein breiter, da in CreateLines bei >= umgebrochen wird.
    return (ULONG)nMaxWidth;
}

ULONG ImpEditEngine::CalcLineWidth( ParaPortion* pPortion, EditLine* pLine )
{
    // Berechnung der Breite ohne die Indents...
    ULONG nWidth = 0;
    for ( USHORT nTP = pLine->GetStartPortion(); nTP <= pLine->GetEndPortion(); nTP++ )
    {
        TextPortion* pTextPortion = pPortion->GetTextPortions().GetObject( nTP );
        switch ( pTextPortion->GetKind() )
        {
            case PORTIONKIND_TEXT:
            case PORTIONKIND_FIELD:
            case PORTIONKIND_HYPHENATOR:
            case PORTIONKIND_TAB:
            case PORTIONKIND_EXTRASPACE:
            {
                nWidth += pTextPortion->GetSize().Width();
            }
            break;
        }
    }
    return nWidth;
}

ULONG ImpEditEngine::CalcTextHeight()
{
    DBG_ASSERT( GetUpdateMode(), "Sollte bei Update=FALSE nicht verwendet werden: CalcTextHeight" );
    ULONG nY = 0;
    for ( USHORT nPortion = 0; nPortion < GetParaPortions().Count(); nPortion++ )
        nY += GetParaPortions()[nPortion]->GetHeight();
    return nY;
}

USHORT ImpEditEngine::GetLineCount( USHORT nParagraph ) const
{
    DBG_ASSERT( nParagraph < GetParaPortions().Count(), "GetLineCount: Out of range" );
    ParaPortion* pPPortion = GetParaPortions().SaveGetObject( nParagraph );
    DBG_ASSERT( pPPortion, "Absatz nicht gefunden: GetLineCount" );
    if ( pPPortion )
        return pPPortion->GetLines().Count();

    return 0xFFFF;
}

xub_StrLen ImpEditEngine::GetLineLen( USHORT nParagraph, USHORT nLine ) const
{
    DBG_ASSERT( nParagraph < GetParaPortions().Count(), "GetLineCount: Out of range" );
    ParaPortion* pPPortion = GetParaPortions().SaveGetObject( nParagraph );
    DBG_ASSERT( pPPortion, "Absatz nicht gefunden: GetLineHeight" );
    if ( pPPortion && ( nLine < pPPortion->GetLines().Count() ) )
    {
        EditLine* pLine = pPPortion->GetLines().GetObject( nLine );
        DBG_ASSERT( pLine, "Zeile nicht gefunden: GetLineHeight" );
        return pLine->GetLen();
    }

    return 0xFFFF;
}

USHORT ImpEditEngine::GetLineHeight( USHORT nParagraph, USHORT nLine )
{
    DBG_ASSERT( nParagraph < GetParaPortions().Count(), "GetLineCount: Out of range" );
    ParaPortion* pPPortion = GetParaPortions().SaveGetObject( nParagraph );
    DBG_ASSERT( pPPortion, "Absatz nicht gefunden: GetLineHeight" );
    if ( pPPortion && ( nLine < pPPortion->GetLines().Count() ) )
    {
        EditLine* pLine = pPPortion->GetLines().GetObject( nLine );
        DBG_ASSERT( pLine, "Zeile nicht gefunden: GetLineHeight" );
        return pLine->GetHeight();
    }

    return 0xFFFF;
}

ULONG ImpEditEngine::GetParaHeight( USHORT nParagraph )
{
    ULONG nHeight = 0;

    ParaPortion* pPPortion = GetParaPortions().SaveGetObject( nParagraph );
    DBG_ASSERT( pPPortion, "Absatz nicht gefunden: GetParaHeight" );

    if ( pPPortion )
        nHeight = pPPortion->GetHeight();

    return nHeight;
}

void ImpEditEngine::UpdateSelections()
{
    USHORT nInvNodes = aDeletedNodes.Count();

    // Pruefen, ob eine der Selektionen auf einem geloeschten Node steht...
    // Wenn der Node gueltig ist, muss noch der Index geprueft werden!
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
                // ParaPortions verwenden, da jetzt auch versteckte
                // Absaetze beruecksichtigt werden muessen!
                USHORT nPara = pInf->GetPosition();
                ParaPortion* pPPortion = GetParaPortions().SaveGetObject( nPara );
                if ( !pPPortion ) // letzter Absatz
                {
                    nPara = GetParaPortions().Count()-1;
                    pPPortion = GetParaPortions().GetObject( nPara );
                }
                DBG_ASSERT( pPPortion, "Leeres Document in UpdateSelections ?" );
                // Nicht aus einem verstecktem Absatz landen:
                USHORT nCurPara = nPara;
                USHORT nLastPara = GetParaPortions().Count()-1;
                while ( nPara <= nLastPara && !GetParaPortions()[nPara]->IsVisible() )
                    nPara++;
                if ( nPara > nLastPara ) // dann eben rueckwaerts...
                {
                    nPara = nCurPara;
                    while ( nPara && !GetParaPortions()[nPara]->IsVisible() )
                        nPara--;
                }
                DBG_ASSERT( GetParaPortions()[nPara]->IsVisible(), "Keinen sichtbaren Absatz gefunden: UpdateSelections" );

                ParaPortion* pParaPortion = GetParaPortions()[nPara];
                EditSelection aTmpSelection( EditPaM( pParaPortion->GetNode(), 0 ) );
                pView->pImpEditView->SetEditSelection( aTmpSelection );
                bChanged=TRUE;
                break;  // for-Schleife
            }
        }
        if ( !bChanged )
        {
            // Index prueffen, falls Node geschrumpft.
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

    // Loeschen...
    for ( USHORT n = 0; n < nInvNodes; n++ )
    {
        DeletedNodeInfo* pInf = aDeletedNodes.GetObject( n );
        delete pInf;
    }
    aDeletedNodes.Remove( 0, aDeletedNodes.Count() );
}

BOOL ImpEditEngine::SetCursorAtPoint( const Point& rPointPixel, EditView* pView )
{
    CheckIdleFormatter();

    Point aMousePos( rPointPixel );

    // Logische Einheiten...
    aMousePos = pView->GetWindow()->PixelToLogic( aMousePos );

    if ( ( !pView->GetOutputArea().IsInside( aMousePos ) ) && !IsInSelectionMode() )
    {
        return FALSE;
    }

    Point aDocPos( pView->pImpEditView->GetDocPos( aMousePos ) );

    // Kann optimiert werden: Erst innerhalb eines Absatzes die Zeilen
    // fuer den PaM durchwuehlen, dann nochmal mit dem PaM fuer das Rect,
    // obwohl die Zeile schon bekannt ist....
    // Das muss doch nicht sein !

    EditPaM aPaM = GetPaM( aDocPos );
    BOOL bGotoCursor = pView->pImpEditView->DoAutoScroll();

    // aTmpNewSel: Diff zwischen alt und neu, nicht die neue Selektion
    EditSelection aTmpNewSel( pView->pImpEditView->GetEditSelection().Max(), aPaM );

    pView->pImpEditView->GetEditSelection().Max() = aPaM;
    if ( !aSelEngine.HasAnchor() )
    {
        if ( pView->pImpEditView->GetEditSelection().Min() != aPaM )
            CursorMoved( pView->pImpEditView->GetEditSelection().Min().GetNode() );
        pView->pImpEditView->GetEditSelection().Min() = aPaM;
    }
    else
    {
        pView->pImpEditView->DrawSelection( aTmpNewSel );
    }

    BOOL bForceCursor = ( pDragAndDropInfo ? FALSE : TRUE ) && !IsInSelectionMode();
    pView->ShowCursor( bGotoCursor, bForceCursor );
    return TRUE;
}

BOOL ImpEditEngine::IsSelectionAtPoint( const Point& rPointPixel, EditView* pView )
{
    if ( pDragAndDropInfo && pDragAndDropInfo->pField )
        return TRUE;

    Point aMousePos( rPointPixel );

    // Logische Einheiten...
    aMousePos = pView->GetWindow()->PixelToLogic( aMousePos );

    if ( ( !pView->GetOutputArea().IsInside( aMousePos ) ) && !IsInSelectionMode() )
    {
        return FALSE;
    }

    Point aDocPos( pView->pImpEditView->GetDocPos( aMousePos ) );
    EditPaM aPaM = GetPaM( aDocPos, FALSE );
    return IsInSelection( aPaM, pView );
}

BOOL ImpEditEngine::IsInSelection( EditPaM aPaM, EditView* pView )
{
    EditSelection aSel = pView->pImpEditView->GetEditSelection();
    if ( !aSel.HasRange() )
        return FALSE;

    aSel.Adjust( aEditDoc );

    USHORT nStartNode = aEditDoc.GetPos( aSel.Min().GetNode() );
    USHORT nEndNode = aEditDoc.GetPos( aSel.Max().GetNode() );
    USHORT nCurNode = aEditDoc.GetPos( aPaM.GetNode() );

    if ( ( nCurNode > nStartNode ) && ( nCurNode < nEndNode ) )
        return TRUE;

    if ( nStartNode == nEndNode )
    {
        if ( nCurNode == nStartNode )
            if ( ( aPaM.GetIndex() >= aSel.Min().GetIndex() ) && ( aPaM.GetIndex() < aSel.Max().GetIndex() ) )
                return TRUE;
    }
    else if ( ( nCurNode == nStartNode ) && ( aPaM.GetIndex() >= aSel.Min().GetIndex() ) )
        return TRUE;
    else if ( ( nCurNode == nEndNode ) && ( aPaM.GetIndex() < aSel.Max().GetIndex() ) )
        return TRUE;

    return FALSE;
}

void ImpEditEngine::CreateAnchor( EditView* pView )
{
    bInSelection = TRUE;
    // Min() setzen, da in SetCursorAtPoint nicht initialisiert:
    pView->pImpEditView->GetEditSelection().Min() = pView->pImpEditView->GetEditSelection().Max();
}

void ImpEditEngine::DeselectAll( EditView* pView )
{
    bInSelection = FALSE;
    // Min() setzen, da in SetCursorAtPoint nicht initialisiert:
    pView->pImpEditView->DrawSelection();
    pView->pImpEditView->GetEditSelection().Min() = pView->pImpEditView->GetEditSelection().Max();
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
            // Vorwaerts suchen...
            xub_Unicode nSC = aGroupChars.GetChar( nMatchChar );
            DBG_ASSERT( aGroupChars.Len() > (nMatchChar+1), "Ungueltige Gruppe von MatchChars!" );
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
            if ( nLevel == 0 )  // gefunden
            {
                aMatchSel.Min() = aTmpSel.Min();
                aMatchSel.Max() = EditPaM( pNode, nCur+1 );
            }
        }
        else
        {
            // Rueckwaerts suchen...
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
                        nCur = pNode->Len()-1;  // egal ob negativ, weil if Len()
                }
            }

            if ( nLevel == 0 )  // gefunden
            {
                aMatchSel.Min() = aTmpSel.Min();
                aMatchSel.Min().GetIndex()++;   // hinter das Zeichen
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
        pActiveView->pImpEditView->DrawSelection(); // Wegzeichnen...
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
    // Eigentlich waere jetzt ein bHasVisSel und HideSelection notwendig !!!

    if ( pView == pActiveView )
        return;

    if ( pActiveView && pActiveView->HasSelection() )
        pActiveView->pImpEditView->DrawSelection(); // Wegzeichnen...

    pActiveView = pView;

    if ( pActiveView && pActiveView->HasSelection() )
        pActiveView->pImpEditView->DrawSelection(); // Wegzeichnen...

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

BOOL ImpEditEngine::HasData( ExchangeType eExchange )
{
    BOOL bData = FALSE;

    if ( eExchange == EXCHANGE_CLIPBOARD )
    {
        uno::Reference< lang::XMultiServiceFactory > xMSF( ::comphelper::getProcessServiceFactory() );
        uno::Reference< datatransfer::clipboard::XClipboard > xClipboard( xMSF->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.datatransfer.clipboard.SystemClipboard" ) ), uno::UNO_QUERY );
        if ( xClipboard.is() )
        {
            uno::Reference< datatransfer::XTransferable > xDataObj = xClipboard->getContents();
            if ( xDataObj.is() )
            {
                datatransfer::DataFlavor aFlavor;
                SotExchange::GetFormatDataFlavor( SOT_FORMAT_STRING, aFlavor );
                bData = xDataObj->isDataFlavorSupported( aFlavor );
            }
        }
    }
    else
    {
        bData = DragServer::HasFormat ( 0, SOT_FORMAT_STRING );
    }

    return bData;
}

void ImpEditEngine::CopyData( EditSelection aSelection, ExchangeType nType ) const
{
    aSelection.Adjust( GetEditDoc() );

    EditDataObject* pDataObj = new EditDataObject;
    uno::Reference< datatransfer::XTransferable > xDataObj;
    xDataObj = pDataObj;

    XubString aText( GetSelected( aSelection ) );
    aText.ConvertLineEnd(); // Systemspezifisch
    pDataObj->GetString() = aText;

    WriteBin( pDataObj->GetStream(), aSelection, TRUE );
    pDataObj->GetStream().Seek( 0 );

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

    if ( nType == EXCHANGE_CLIPBOARD )
    {
        // HACK: Hold Ref to System Clipboard
        static uno::Reference< datatransfer::clipboard::XClipboard > xClipboard;
        if ( !xClipboard.is() )
        {
            uno::Reference< lang::XMultiServiceFactory > xMSF( ::comphelper::getProcessServiceFactory() );
            xClipboard = uno::Reference< datatransfer::clipboard::XClipboard >( xMSF->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.datatransfer.clipboard.SystemClipboard" ) ), uno::UNO_QUERY );
        }
        const sal_uInt32 nRef = Application::ReleaseSolarMutex();
        xClipboard->setContents( pDataObj, NULL );
        Application::AcquireSolarMutex( nRef );
    }
    else // DRAGSERVER
    {
        // OLD, remove when new D&D is finished...
        DragServer::Clear();
        pDataObj->GetStream().Seek( STREAM_SEEK_TO_END );
        long nBinLen = pDataObj->GetStream().Tell();
        pDataObj->GetStream().Seek( 0 );
        DragServer::CopyData( pDataObj->GetStream().GetData(), nBinLen, EditEngine::RegisterClipboardFormatName() );
        DragServer::CopyString( pDataObj->GetString() );
    }
}

EditSelection ImpEditEngine::PasteData( EditPaM aPaM, ExchangeType eExchange, BOOL bSpecial )
{
    EditSelection aNewSelection( aPaM, aPaM );

    if ( eExchange == EXCHANGE_CLIPBOARD )
    {
        uno::Reference< datatransfer::XTransferable > xDataObj;

        uno::Reference< lang::XMultiServiceFactory > xMSF( ::comphelper::getProcessServiceFactory() );
        uno::Reference< datatransfer::clipboard::XClipboard > xClipboard( xMSF->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.datatransfer.clipboard.SystemClipboard" ) ), uno::UNO_QUERY );
        if ( xClipboard.is() )
            xDataObj = xClipboard->getContents();

        if ( xDataObj.is() )
        {
            datatransfer::DataFlavor aFlavor;
            BOOL bDone = FALSE;

            if ( bSpecial )
            {
                // BIN
                SotExchange::GetFormatDataFlavor( SOT_FORMATSTR_ID_EDITENGINE, aFlavor );
                if ( xDataObj->isDataFlavorSupported( aFlavor ) )
                {
                    uno::Any aData = xDataObj->getTransferData( aFlavor );
                    uno::Sequence< sal_Int8 > aSeq;
                    aData >>= aSeq;
                    {
                        SvMemoryStream aBinStream( aSeq.getArray(), aSeq.getLength(), STREAM_READ );
                        aNewSelection = ReadBin( aBinStream, aPaM );
                    }
                    bDone = TRUE;
                }

                if ( !bDone )
                {
                    // Bookmark
                    /*
                    String aURL = ...;
                    String aTxt = ...;
                    // Feld nur einfuegen, wenn Factory vorhanden.
                    if ( ITEMDATA() && ITEMDATA()->GetClassManager().Get( SVX_URLFIELD ) )
                    {
                        SvxFieldItem aField( SvxURLField( aURL, aTxt, SVXURLFORMAT_URL ), EE_FEATURE_FIELD  );
                        aNewSelection = InsertField( aPaM, aField );
                        UpdateFields();
                    }
                    else
                        aNewSelection = ImpInsertText( aPaM, aURL );
                    }
                    */
                }
                if ( !bDone )
                {
                    // RTF
                    SotExchange::GetFormatDataFlavor( SOT_FORMAT_RTF, aFlavor );
                    if ( xDataObj->isDataFlavorSupported( aFlavor ) )
                    {
                        uno::Any aData = xDataObj->getTransferData( aFlavor );
                        uno::Sequence< sal_Int8 > aSeq;
                        aData >>= aSeq;
                        {
                            SvMemoryStream aRTFStream( aSeq.getArray(), aSeq.getLength(), STREAM_READ );
                            aNewSelection = ReadRTF( aRTFStream, aPaM );
                        }
                        bDone = TRUE;
                    }
                }
            }
            if ( !bDone )
            {
                SotExchange::GetFormatDataFlavor( SOT_FORMAT_STRING, aFlavor );
                if ( xDataObj->isDataFlavorSupported( aFlavor ) )
                {
                    uno::Any aData = xDataObj->getTransferData( aFlavor );
                    ::rtl::OUString aText;
                    aData >>= aText;
                    aNewSelection = ImpInsertText( aPaM, aText );
                    bDone = TRUE;
                }
            }
        }

    }
    else
    {
        ULONG nBinReg = EditEngine::RegisterClipboardFormatName();

        if ( bSpecial && DragServer::HasFormat ( 0, nBinReg ) )
        {
            ULONG nLen; BYTE* pData;
            nLen = DragServer::GetDataLen( 0, nBinReg );
            pData = new BYTE[ nLen ];
            DragServer::PasteData( 0, pData, nLen, nBinReg );
            SvMemoryStream aStream( pData, nLen, STREAM_READ );
            aNewSelection = ReadBin( aStream, aPaM );
            delete pData;
        }
        else if ( bSpecial && DragServer::HasFormat ( 0, SOT_FORMAT_RTF ) )
        {
            ULONG nLen; BYTE* pData;
            nLen = DragServer::GetDataLen( 0, SOT_FORMAT_RTF );
            pData = new BYTE[ nLen+1 ];
            DragServer::PasteData( 0, pData, nLen, FORMAT_RTF );
            *( pData + nLen ) = '\0';   // Falls RTF kaputt
            SvMemoryStream aStream( pData, nLen, STREAM_READ );
            aNewSelection = ReadRTF( aStream, aPaM );
            delete pData;
        }
        else if ( DragServer::HasFormat ( 0, FORMAT_STRING ) )
        {
            XubString aText;
            if( eExchange == EXCHANGE_CLIPBOARD )
                aText = Clipboard::PasteString();
            else
                aText = DragServer::PasteString( 0 );
            aNewSelection = ImpInsertText( aPaM, aText );
        }
    }
    return aNewSelection;
}

Range ImpEditEngine::GetInvalidYOffsets( ParaPortion* pPortion )
{
    DBG_ASSERT( pPortion->IsVisible(), "Wozu GetInvalidYOffset() bei einem unsichtbaren Absatz?" );

    Range aRange( 0, 0 );

    const SvxULSpaceItem& rULSpace = (const SvxULSpaceItem&)pPortion->GetNode()->GetContentAttribs().GetItem( EE_PARA_ULSPACE );
    const SvxLineSpacingItem& rLSItem = (const SvxLineSpacingItem&)pPortion->GetNode()->GetContentAttribs().GetItem( EE_PARA_SBL );
    USHORT nSBL = ( rLSItem.GetInterLineSpaceRule() == SVX_INTER_LINE_SPACE_FIX )
                        ? GetYValue( rLSItem.GetInterLineSpace() ) : 0;

    // erst von vorne...
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
        if ( nLine && !aStatus.IsOutliner() )   // nicht die erste Zeile
            aRange.Min() += nSBL;
        aRange.Min() += pL->GetHeight();
    }
    DBG_ASSERT( nFirstInvalid != 0xFFFF, "Keine ungueltige Zeile gefunden in GetInvalidYOffset(1)" );


    // Abgleichen und weiter...
    aRange.Max() = aRange.Min();
    aRange.Max() += pPortion->GetFirstLineOffset();
    if ( nFirstInvalid != 0 )   // Nur wenn nicht die erste Zeile ungueltig
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

    // MT 07/00 SBL kann jetzt kleiner 100% sein => ggf. die Zeile davor neu ausgeben.
    if( ( rLSItem.GetInterLineSpaceRule() == SVX_INTER_LINE_SPACE_PROP ) &&
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

    return aRange;
}

EditPaM ImpEditEngine::GetPaM( ParaPortion* pPortion, Point aDocPos, BOOL bSmart )
{
    DBG_ASSERT( pPortion->IsVisible(), "Wozu GetPaM() bei einem unsichtbaren Absatz?" );
    DBG_ASSERT( IsFormatted(), "GetPaM: Nicht formatiert" );

    USHORT nCurIndex = 0;
    EditPaM aPaM;
    aPaM.SetNode( pPortion->GetNode() );

    const SvxLineSpacingItem& rLSItem = (const SvxLineSpacingItem&)pPortion->GetNode()->GetContentAttribs().GetItem( EE_PARA_SBL );
    USHORT nSBL = ( rLSItem.GetInterLineSpaceRule() == SVX_INTER_LINE_SPACE_FIX )
                        ? GetYValue( rLSItem.GetInterLineSpace() ) : 0;

    long nY = pPortion->GetFirstLineOffset();

    DBG_ASSERT( pPortion->GetLines().Count(), "Leere ParaPortion in GetPaM!" );

    EditLine* pLine = 0;
    for ( USHORT nLine = 0; nLine < pPortion->GetLines().Count(); nLine++ )
    {
        EditLine* pTmpLine = pPortion->GetLines().GetObject( nLine );
        nY += pTmpLine->GetHeight();
        if ( !aStatus.IsOutliner() )
            nY += nSBL;
        if ( nY > aDocPos.Y() )     // das war 'se
        {
            pLine = pTmpLine;
            break;                  // richtige Y-Position intressiert nicht
        }

        nCurIndex += pTmpLine->GetLen();
    }

    if ( !pLine ) // darf nur im Bereich von SA passieren!
    {
        #ifdef DBG_UTIL
         const SvxULSpaceItem& rULSpace =(const SvxULSpaceItem&)pPortion->GetNode()->GetContentAttribs().GetItem( EE_PARA_ULSPACE );
         DBG_ASSERT( nY+GetYValue( rULSpace.GetLower() ) >= aDocPos.Y() , "Index in keiner Zeile, GetPaM ?" );
        #endif
        aPaM.SetIndex( pPortion->GetNode()->Len() );
        return aPaM;
    }

    // Wenn Zeile gefunden, nur noch X-Position => Index
    nCurIndex = pPortion->GetChar( pLine, aDocPos.X(), bSmart );

    if ( ( nCurIndex == pLine->GetEnd() ) &&
         ( pLine != pPortion->GetLines().GetObject( pPortion->GetLines().Count()-1) ) )
     {
        if ( nCurIndex )
            nCurIndex--;
     }

    aPaM.SetIndex( nCurIndex );
    return aPaM;
}

void ImpEditEngine::CalcHeight( ParaPortion* pPortion )
{
    pPortion->nHeight = 0;
    pPortion->nFirstLineOffset = 0;

    if ( pPortion->IsVisible() )
    {
        DBG_ASSERT( pPortion->GetLines().Count(), "Absatz ohne Zeilen in ParaPortion::CalcHeight" );
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
                pPortion->nHeight += GetYValue( rULItem.GetLower() );   // nicht in letzter
            }


            if ( nPortion && !aStatus.ULSpaceSummation() )
            {
                ParaPortion* pPrev = GetParaPortions().SaveGetObject( nPortion-1 );
                const SvxULSpaceItem& rPrevULItem = (const SvxULSpaceItem&)pPrev->GetNode()->GetContentAttribs().GetItem( EE_PARA_ULSPACE );
                const SvxLineSpacingItem& rPrevLSItem = (const SvxLineSpacingItem&)pPrev->GetNode()->GetContentAttribs().GetItem( EE_PARA_SBL );

                // Verhalten WinWord6/Writer3:
                // Bei einem proportionalen Zeilenabstand wird auch der Absatzabstand
                // manipuliert.
                // Nur Writer3: Nicht aufaddieren, sondern Mindestabstand.

                // Pruefen, ob Abstand durch LineSpacing > Upper:
                USHORT nExtraSpace = GetYValue( lcl_CalcExtraSpace( pPortion, rLSItem ) );
                if ( nExtraSpace > pPortion->nFirstLineOffset )
                {
                    // Absatz wird 'groesser':
                    pPortion->nHeight += ( nExtraSpace - pPortion->nFirstLineOffset );
                    pPortion->nFirstLineOffset = nExtraSpace;
                }

                // nFirstLineOffset jetzt f(pNode) => jetzt f(pNode, pPrev) ermitteln:
                USHORT nPrevLower = GetYValue( rPrevULItem.GetLower() );

                // Dieser PrevLower steckt noch in der Hoehe der PrevPortion...
                if ( nPrevLower > pPortion->nFirstLineOffset )
                {
                    // Absatz wird 'kleiner':
                    pPortion->nHeight -= pPortion->nFirstLineOffset;
                    pPortion->nFirstLineOffset = 0;
                }
                else if ( nPrevLower )
                {
                    // Absatz wird 'etwas kleiner':
                    pPortion->nHeight -= nPrevLower;
                    pPortion->nFirstLineOffset -= nPrevLower;
                }

                // Finde ich zwar nicht so gut, aber Writer3-Feature:
                // Pruefen, ob Abstand durch LineSpacing > Lower:
                // Dieser Wert steckt nicht in der Hoehe der PrevPortion.
                if ( !pPrev->IsInvalid() )
                {
                    nExtraSpace = GetYValue( lcl_CalcExtraSpace( pPrev, rPrevLSItem ) );
                    if ( nExtraSpace > nPrevLower )
                    {
                        USHORT nMoreLower = nExtraSpace - nPrevLower;
                        // Absatz wird 'groesser', 'waechst' nach unten:
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
    DBG_ASSERT( pPortion->IsVisible(), "Wozu GetEditCursor() bei einem unsichtbaren Absatz?" );
    DBG_ASSERT( IsFormatted() || GetTextRanger(), "GetEditCursor: Nicht formatiert" );

    /*
     GETCRSR_ENDOFLINE: Wenn hinter dem letzten Zeichen einer umgebrochenen Zeile,
     am Ende der Zeile bleiben, nicht am Anfang der naechsten.
     Zweck:     - END => wirklich hinter das letzte Zeichen
                - Selektion....
    */

    long nY = pPortion->GetFirstLineOffset();

    const SvxLineSpacingItem& rLSItem = (const SvxLineSpacingItem&)pPortion->GetNode()->GetContentAttribs().GetItem( EE_PARA_SBL );
    USHORT nSBL = ( rLSItem.GetInterLineSpaceRule() == SVX_INTER_LINE_SPACE_FIX )
                        ? GetYValue( rLSItem.GetInterLineSpace() ) : 0;

    USHORT nCurIndex = 0;
    DBG_ASSERT( pPortion->GetLines().Count(), "Leere ParaPortion in GetEditCursor!" );
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

        nCurIndex +=  pTmpLine->GetLen();
        nY += pTmpLine->GetHeight();
        if ( !aStatus.IsOutliner() )
            nY += nSBL;
    }
    if ( !pLine )
    {
        // Cursor am Ende des Absatzes.
        DBG_ASSERT( nIndex == nCurIndex, "Index voll daneben in GetEditCursor!" );

        pLine = pPortion->GetLines().GetObject( pPortion->GetLines().Count()-1 );
        nY -= pLine->GetHeight();
        if ( !aStatus.IsOutliner() )
            nY -= nSBL;
        nCurIndex -=  pLine->GetLen();
    }

    Rectangle aEditCursor;

    aEditCursor.Top() = nY;
    nY += pLine->GetHeight();
    aEditCursor.Bottom() = nY-1;

    // innerhalb der Zeile suchen....
    long nX = pPortion->GetXPos( pLine, nIndex );
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

    // Minimale/Maximale Breite:
    if ( aPaperSize.Width() < nMinWidth )
        aPaperSize.Width() = nMinWidth;
    else if ( aPaperSize.Width() > nMaxWidth )
        aPaperSize.Width() = nMaxWidth;

    // Minimale/Maximale Hoehe:
    if ( aPaperSize.Height() < nMinHeight )
        aPaperSize.Height() = nMinHeight;
    else if ( aPaperSize.Height() > nMaxHeight )
        aPaperSize.Height() = nMaxHeight;
}

void ImpEditEngine::IndentBlock( EditView* pEditView, BOOL bRight )
{
    ESelection aESel( CreateESel( pEditView->pImpEditView->GetEditSelection() ) );
    aESel.Adjust();

    // Nur wenn mehrere selektierte Absaetze...
    if ( aESel.nEndPara > aESel.nStartPara )
    {
        ESelection aNewSel = aESel;
        aNewSel.nStartPos = 0;
        aNewSel.nEndPos = 0xFFFF;

        if ( aESel.nEndPos == 0 )
        {
            aESel.nEndPara--;       // dann diesen Absatz nicht...
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
                // Tabs hinzufuegen
                EditPaM aPaM( pNode, 0 );
                InsertTab( aPaM );
            }
            else
            {
                // Tabs entfernen
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

