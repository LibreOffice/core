/*************************************************************************
 *
 *  $RCSfile: impedit2.cxx,v $
 *
 *  $Revision: 1.91 $
 *
 *  last change: $Author: hr $ $Date: 2003-11-07 15:07:15 $
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
#include <svtools/colorcfg.hxx>
#include <svtools/ctloptions.hxx>

#include <fhgtitem.hxx>
#include <lrspitem.hxx>
#include <ulspitem.hxx>
#include <wghtitem.hxx>
#include <postitem.hxx>
#include <udlnitem.hxx>
#include <adjitem.hxx>
#include <scripttypeitem.hxx>
#include <frmdiritem.hxx>
#include <fontitem.hxx>

#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif

#ifndef _SFX_FCONTNR_HXX //autogen
#include <sfx2/fcontnr.hxx>
#endif

#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif

#ifndef _VCL_CMDEVT_H
#include <vcl/cmdevt.h>
#endif

#ifndef SVX_LIGHT
#ifndef _SFXFRAME_HXX //autogen
#include <sfx2/frame.hxx>
#endif
#endif

#ifndef _COM_SUN_STAR_I18N_CHARACTERITERATORMODE_HPP_
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#endif

#ifndef _COM_SUN_STAR_I18N_WORDTYPE_HPP_
#include <com/sun/star/i18n/WordType.hpp>
#endif

#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HPP_
#include <com/sun/star/i18n/ScriptType.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_CHARACTERCOMPRESSIONTYPE_HPP_
#include <com/sun/star/text/CharacterCompressionType.hpp>
#endif

#include <comphelper/processfactory.hxx>

#include <sot/formats.hxx>

#include <unicode/ubidi.h>

#if defined(MACOSX) && ( __GNUC__ < 3 )
// moved from editundo.hxx
const EPaM& EditUndoRemoveChars::GetEPaM() { return aEPaM; }
String& EditUndoRemoveChars::GetStr() { return aText; }
#endif

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
#ifndef SVX_LIGHT
    bUndoEnabled        = TRUE;
#else
    bUndoEnabled        = FALSE;
#endif
    bCallParaInsertedOrDeleted = FALSE;

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

    // Ab hier wird schon auf Daten zugegriffen!
    SetRefDevice( pRefDev );
    InitDoc( FALSE );

    bCallParaInsertedOrDeleted = TRUE;

    aEditDoc.SetModifyHdl( LINK( this, ImpEditEngine, DocModified ) );
}

ImpEditEngine::~ImpEditEngine()
{
    aStatusTimer.Stop();
    aOnlineSpellTimer.Stop();
    aIdleFormatter.Stop();

    // das Zerstoeren von Vorlagen kann sonst unnoetiges Formatieren ausloesen,
    // wenn eine Parent-Vorlage zerstoert wird.
    // Und das nach dem Zerstoeren der Daten!
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
    if ( rCEvt.GetCommand() == COMMAND_VOICE )
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
                aSel.Max().GetIndex() += mpIMEInfos->nLen;
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
                        aPaM.GetIndex() += nNewIMETextLen;
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
                        aPaM.GetIndex() += nNewIMETextLen;
                        EditSelection aSel( aPaM );
                        aSel.Max().GetIndex() += nOverwrite;
                        DeleteSelected( aSel );
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
#ifndef SVX_LIGHT
    DBG_ASSERT( !HasUndoManager() || !GetUndoManager().GetUndoActionCount(), "Undo nach SetText?" );
#endif
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
        /*
        else if ( !bCtrl && ( ( nCode == KEY_HOME ) || ( nCode == KEY_END ) ) )
        {
            aPaM = CursorVisualStartEnd( pEditView, aPaM, nCode == KEY_HOME );
            nCode = 0;  // skip switch statement
        }
        */
    }

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
    aSelEngine.CursorPosChanging( aTranslatedKeyEvent.GetKeyCode().IsShift(), aTranslatedKeyEvent.GetKeyCode().IsMod1() );
    EditPaM aOldEnd( pEditView->pImpEditView->GetEditSelection().Max() );
    pEditView->pImpEditView->GetEditSelection().Max() = aPaM;
    if ( aTranslatedKeyEvent.GetKeyCode().IsShift() )
    {
        // Dann wird die Selektion erweitert...
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
        USHORT nPosInLine = aPaM.GetIndex() - pLine->GetStart();

        const sal_Unicode* pLineString = aLine.GetBuffer();

        UErrorCode nError = U_ZERO_ERROR;
        UBiDi* pBidi = ubidi_openSized( aLine.Len(), 0, &nError );

        const BYTE nDefaultDir = IsRightToLeft( nPara ) ? UBIDI_RTL : UBIDI_LTR;
        ubidi_setPara( pBidi, pLineString, aLine.Len(), nDefaultDir, NULL, &nError );

        USHORT nVisPos = bStart ? 0 : aLine.Len()-1;
        USHORT nLogPos = (USHORT)ubidi_getLogicalIndex( pBidi, nVisPos, &nError );

        ubidi_close( pBidi );

        aPaM.GetIndex() = nLogPos + pLine->GetStart();

        USHORT nTmp;
        USHORT nTextPortion = pParaPortion->GetTextPortions().FindPortion( aPaM.GetIndex(), nTmp, TRUE );
        TextPortion* pTextPortion = pParaPortion->GetTextPortions().GetObject( nTextPortion );
        USHORT nRTLLevel = pTextPortion->GetRightToLeft();
        BOOL bParaRTL = IsRightToLeft( nPara );
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

    USHORT nCurrentCursorFlags = pEditView->pImpEditView->nExtraCursorFlags;
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

        const BYTE nDefaultDir = IsRightToLeft( nPara ) ? UBIDI_RTL : UBIDI_LTR;
        ubidi_setPara( pBidi, pLineString, aLine.Len(), nDefaultDir, NULL, &nError );

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
//                if ( bWasBehind && bRTLPortion )
//                    nVisPos++;
            }

            bGotoEndOfPrevLine = nVisPos < 0;
            bGotoStartOfNextLine = nVisPos >= aLine.Len();

            if ( !bGotoEndOfPrevLine && !bGotoStartOfNextLine )
            {
                USHORT nLogPos = (USHORT)ubidi_getLogicalIndex( pBidi, nVisPos, &nError );

/*
                if ( nLogPos == aPaM.GetIndex() )
                {
                    if ( bVisualToLeft )
                        bGotoEndOfPrevLine = TRUE;
                    else
                        bGotoStartOfNextLine = TRUE;
                }
                else
*/
                {
                    aPaM.GetIndex() = pLine->GetStart() + nLogPos;

                    // RTL portion, stay visually on the left side.
                    USHORT nPortionStart;
                    // USHORT nTextPortion = pParaPortion->GetTextPortions().FindPortion( aPaM.GetIndex(), nPortionStart, !bRTLPortion );
                    USHORT nTextPortion = pParaPortion->GetTextPortions().FindPortion( aPaM.GetIndex(), nPortionStart, TRUE );
                    TextPortion* pTextPortion = pParaPortion->GetTextPortions().GetObject( nTextPortion );
                    if ( bVisualToLeft && !bRTLPortion && ( pTextPortion->GetRightToLeft() % 2 ) )
                        aPaM.GetIndex()++;
                    else if ( !bVisualToLeft && bRTLPortion && ( bWasBehind || !(pTextPortion->GetRightToLeft() % 2 )) )
                        aPaM.GetIndex()++;

                    pEditView->pImpEditView->SetCursorBidiLevel( nPortionStart );
                }
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
        uno::Reference < i18n::XBreakIterator > xBI = ImplGetBreakIterator();
         aNewPaM.SetIndex( (USHORT)xBI->previousCharacters( *aNewPaM.GetNode(), aNewPaM.GetIndex(), GetLocale( aNewPaM ), nCharacterIteratorMode, nCount, nCount ) );
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
        uno::Reference < i18n::XBreakIterator > xBI = ImplGetBreakIterator();
        sal_Int32 nCount = 1;
        aNewPaM.SetIndex( (USHORT)xBI->nextCharacters( *aNewPaM.GetNode(), aNewPaM.GetIndex(), GetLocale( aNewPaM ), nCharacterIteratorMode, nCount, nCount ) );
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
    DBG_ASSERT( pView, "Keine View - Keine Cursorbewegung!" );

    ParaPortion* pPPortion = FindParaPortion( rPaM.GetNode() );
    DBG_ASSERT( pPPortion, "Keine passende Portion gefunden: CursorUp" );
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
    if ( nLine )    // gleicher Absatz
    {
        EditLine* pPrevLine = pPPortion->GetLines().GetObject(nLine-1);
        aNewPaM.SetIndex( GetChar( pPPortion, pPrevLine, nX ) );
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
            aNewPaM.SetIndex( GetChar( pPrevPortion, pLine, nX+nOnePixelInRef ) );
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
        if ( aBoundary.startPos >= nCurrentPos )
            aBoundary = xBI->previousWord( *aNewPaM.GetNode(), nCurrentPos, GetLocale( EditPaM( aNewPaM.GetNode(), nCurrentPos ) ), nWordType );
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

EditSelection ImpEditEngine::SelectWord( const EditSelection& rCurSel, sal_Int16 nWordType, BOOL bAcceptStartOfWord )
{
    EditSelection aNewSel( rCurSel );
    EditPaM aPaM( rCurSel.Max() );
    uno::Reference < i18n::XBreakIterator > xBI = ImplGetBreakIterator();
    sal_Int16 nType = xBI->getWordType( *aPaM.GetNode(), aPaM.GetIndex(), GetLocale( aPaM ) );
    if ( nType == i18n::WordType::ANY_WORD )
    {
        i18n::Boundary aBoundary = xBI->getWordBoundary( *aPaM.GetNode(), aPaM.GetIndex(), GetLocale( aPaM ), nWordType, sal_True );
        // don't select when curser at end of word
        if ( ( aBoundary.endPos > aPaM.GetIndex() ) &&
             ( bAcceptStartOfWord || ( aBoundary.startPos < aPaM.GetIndex() ) ) )
        {
            aNewSel.Min().SetIndex( (USHORT)aBoundary.startPos );
            aNewSel.Max().SetIndex( (USHORT)aBoundary.endPos );
        }
    }

    return aNewSel;
}

void ImpEditEngine::InitScriptTypes( USHORT nPara )
{
    ParaPortion* pParaPortion = GetParaPortions().SaveGetObject( nPara );
    ScriptTypePosInfos& rTypes = pParaPortion->aScriptInfos;
    rTypes.Remove( 0, rTypes.Count() );


//  pParaPortion->aExtraCharInfos.Remove( 0, pParaPortion->aExtraCharInfos.Count() );

    ContentNode* pNode = pParaPortion->GetNode();
    if ( pNode->Len() )
    {
        uno::Reference < i18n::XBreakIterator > xBI = ImplGetBreakIterator();

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
                short nFldScriptType = xBI->getScriptType( aFldText, 0 );

                for ( USHORT nCharInField = 1; nCharInField < aFldText.getLength(); nCharInField++ )
                {
                    short nTmpType = xBI->getScriptType( aFldText, nCharInField );

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
            long nEndPos = xBI->endOfScript( aOUText, nPos, nScriptType );

            // #96850# Handle blanks as weak, remove if BreakIterator returns WEAK for spaces.
            if ( ( nScriptType == i18n::ScriptType::LATIN ) && ( aOUText.getStr()[ nPos ] == 0x20 ) )
            {
                BOOL bOnlySpaces = TRUE;
                for ( USHORT n = nPos+1; ( n < nEndPos ) && bOnlySpaces; n++ )
                {
                    if ( aOUText.getStr()[ n ] != 0x20 )
                        bOnlySpaces = FALSE;
                }
                if ( bOnlySpaces )
                    nScriptType = i18n::ScriptType::WEAK;
            }

            if ( ( nScriptType == i18n::ScriptType::WEAK ) || ( nScriptType == rTypes[rTypes.Count()-1].nScriptType ) )
            {
                // Expand last ScriptTypePosInfo, don't create weak or unecessary portions
                rTypes[rTypes.Count()-1].nEndPos = (USHORT)nEndPos;
            }
            else
            {
                rTypes.Insert( ScriptTypePosInfo( nScriptType, (USHORT)nPos, nTextLen ), rTypes.Count() );
            }

            nPos = nEndPos;
        }

        if ( rTypes[0].nScriptType == i18n::ScriptType::WEAK )
            rTypes[0].nScriptType = ( rTypes.Count() > 1 ) ? rTypes[1].nScriptType : GetI18NScriptTypeOfLanguage( GetDefaultLanguage() );
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

        USHORT nS = ( nPara == nStartPara ) ? aSel.Min().GetIndex() : 0;
        USHORT nE = ( nPara == nEndPara ) ? aSel.Max().GetIndex() : pParaPortion->GetNode()->Len();
        for ( USHORT n = 0; n < rTypes.Count(); n++ )
        {
            if ( ( rTypes[n].nStartPos <= nE ) && ( rTypes[n].nEndPos >= nS ) )
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

    const BYTE nDefaultDir = IsRightToLeft( nPara ) ? UBIDI_RTL : UBIDI_LTR;
    if ( ( bCTL || ( nDefaultDir == UBIDI_RTL ) ) && pParaPortion->GetNode()->Len() )
    {

        String aText( *pParaPortion->GetNode() );

        //
        // Bidi functions from icu 2.0
        //
        UErrorCode nError = U_ZERO_ERROR;
        UBiDi* pBidi = ubidi_openSized( aText.Len(), 0, &nError );
        nError = U_ZERO_ERROR;

        ubidi_setPara( pBidi, aText.GetBuffer(), aText.Len(), nDefaultDir, NULL, &nError );
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
//    BYTE nRightToLeft = IsRightToLeft( nPara ) ? 1 : 0;
    BYTE nRightToLeft = 0;

    ContentNode* pNode = aEditDoc.SaveGetObject( nPara );
    if ( pNode && pNode->Len() )
    {
        ParaPortion* pParaPortion = GetParaPortions().SaveGetObject( nPara );
        if ( !pParaPortion->aWritingDirectionInfos.Count() )
            InitWritingDirections( nPara );

        BYTE nType = 0;
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
#ifndef SVX_LIGHT
                EditSelection aSel( rPaM );
                aSel.Max().GetIndex() += nChars;
                EditUndoSetAttribs* pAttrUndo = CreateAttribUndo( aSel, GetEmptyItemSet() );
                InsertUndo( pAttrUndo );
#endif
                break;  // for
            }
        }
        if ( pCurUndo && ( CreateEditPaM( pCurUndo->GetEPaM() ) == rPaM ) )
            pCurUndo->GetStr() += aStr;
#ifndef SVX_LIGHT
        else
            InsertUndo( new EditUndoRemoveChars( this, CreateEPaM( rPaM ), aStr ) );
#endif
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

#ifndef SVX_LIGHT
    if ( IsUndoEnabled() && !IsInUndo())
        InsertUndo( new EditUndoMoveParagraphs( this, aOldPositions, nNewPos ) );
#endif

    MoveParagraphsInfo aMoveParagraphsInfo( aOldPositions.Min(), aOldPositions.Max(), nNewPos );
    aBeginMovingParagraphsHdl.Call( &aMoveParagraphsInfo );

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

#ifndef SVX_LIGHT
    if ( IsUndoEnabled() && !IsInUndo() )
    {
        InsertUndo( new EditUndoConnectParas( this,
            aEditDoc.GetPos( pLeft ), pLeft->Len(),
            pLeft->GetContentAttribs().GetItems(), pRight->GetContentAttribs().GetItems(),
            pLeft->GetStyleSheet(), pRight->GetStyleSheet(), bBackward ) );
    }
#endif

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
    DBG_ASSERT( nParagraphTobeDeleted == GetParaPortions().GetPos( pRightPortion ), "NodePos != PortionPos?" );

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

    if ( IsCallParaInsertedOrDeleted() )
        GetEditEnginePtr()->ParagraphDeleted( nParagraphTobeDeleted );

    EditPaM aPaM = aEditDoc.ConnectParagraphs( pLeft, pRight );
    GetParaPortions().Remove( nParagraphTobeDeleted );
    delete pRightPortion;

    pLeftPortion->MarkSelectionInvalid( aPaM.GetIndex(), pLeft->Len() );

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
    {
        GetEditEnginePtr()->ParagraphDeleted( nPara );
    }

    // Im folgenden muss ggf. Extra-Space neu ermittelt werden.
    // Bei ParaAttribsChanged wird leider der Absatz neu formatiert,
    // aber diese Methode sollte nicht Zeitkritsch sein!
    if ( pNextNode )
        ParaAttribsChanged( pNextNode );

#ifndef SVX_LIGHT
    if ( IsUndoEnabled() && !IsInUndo() )
        InsertUndo( new EditUndoDelContent( this, pNode, nPara ) );
    else
#endif
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
#ifndef SVX_LIGHT
        if ( IsUndoEnabled() && !IsInUndo() )
        {
                EditUndoInsertChars* pNewUndo = new EditUndoInsertChars( this, CreateEPaM( aPaM ), c );
                BOOL bTryMerge = ( !bDoOverwrite && ( c != ' ' ) ) ? TRUE : FALSE;
                InsertUndo( pNewUndo, bTryMerge );
        }
#endif

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
                USHORT nMaxNewChars = MAXCHARSINPARA-aPaM.GetNode()->Len();
                nEnd -= ( aLine.Len() - nMaxNewChars ); // Dann landen die Zeichen im naechsten Absatz.
                aLine.Erase( nMaxNewChars );            // Del Rest...
            }
#ifndef SVX_LIGHT
            if ( IsUndoEnabled() && !IsInUndo() )
                InsertUndo( new EditUndoInsertChars( this, CreateEPaM( aPaM ), aLine ) );
#endif
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
#ifndef SVX_LIGHT
        if ( IsUndoEnabled() && !IsInUndo() )
            InsertUndo( new EditUndoInsertChars( this, CreateEPaM( aPaM ), rStr ) );
#endif

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

#ifndef SVX_LIGHT
    if ( IsUndoEnabled() && !IsInUndo() )
        InsertUndo( new EditUndoInsertFeature( this, CreateEPaM( aPaM ), rItem ) );
#endif
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
#ifndef SVX_LIGHT
    if ( IsUndoEnabled() && !IsInUndo() )
        InsertUndo( new EditUndoSplitPara( this, aEditDoc.GetPos( rPaM.GetNode() ), rPaM.GetIndex() ) );
#endif

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
#ifndef SVX_LIGHT
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
#endif

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

ULONG ImpEditEngine::CalcTextWidth( BOOL bIgnoreExtraSpace )
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

    nMaxWidth++; // Ein breiter, da in CreateLines bei >= umgebrochen wird.
    return (ULONG)nMaxWidth;
}

ULONG ImpEditEngine::CalcLineWidth( ParaPortion* pPortion, EditLine* pLine, BOOL bIgnoreExtraSpace )
{
    USHORT nPara = GetEditDoc().GetPos( pPortion->GetNode() );
    ULONG nOldLayoutMode = GetRefDevice()->GetLayoutMode();

    ImplInitLayoutMode( GetRefDevice(), nPara, 0xFFFF );

    SvxAdjust eJustification = GetJustification( nPara );

    // Berechnung der Breite ohne die Indents...
    ULONG nWidth = 0;
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
                    nWidth += aTmpFont.QuickGetTextSize( GetRefDevice(), *pPortion->GetNode(), nPos, pTextPortion->GetLen(), NULL ).Width();
                }
            }
            break;
        }
        nPos += pTextPortion->GetLen();
    }

    GetRefDevice()->SetLayoutMode( nOldLayoutMode );

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

uno::Reference< datatransfer::XTransferable > ImpEditEngine::CreateTransferable( const EditSelection& rSelection ) const
{
#ifndef SVX_LIGHT
    EditSelection aSelection( rSelection );
    aSelection.Adjust( GetEditDoc() );

    EditDataObject* pDataObj = new EditDataObject;
    uno::Reference< datatransfer::XTransferable > xDataObj;
    xDataObj = pDataObj;

    XubString aText( GetSelected( aSelection ) );
    aText.ConvertLineEnd(); // Systemspezifisch
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
#else
    return uno::Reference< datatransfer::XTransferable >();
#endif
}

EditSelection ImpEditEngine::InsertText( uno::Reference< datatransfer::XTransferable >& rxDataObj, const EditPaM& rPaM, BOOL bUseSpecial )
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
                uno::Any aData = rxDataObj->getTransferData( aFlavor );
                uno::Sequence< sal_Int8 > aSeq;
                aData >>= aSeq;
                {
                    SvMemoryStream aBinStream( aSeq.getArray(), aSeq.getLength(), STREAM_READ );
                    aNewSelection = Read( aBinStream, EE_FORMAT_BIN, rPaM );
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
                if ( rxDataObj->isDataFlavorSupported( aFlavor ) )
                {
                    uno::Any aData = rxDataObj->getTransferData( aFlavor );
                    uno::Sequence< sal_Int8 > aSeq;
                    aData >>= aSeq;
                    {
                        SvMemoryStream aRTFStream( aSeq.getArray(), aSeq.getLength(), STREAM_READ );
                        aNewSelection = Read( aRTFStream, EE_FORMAT_RTF, rPaM );
                    }
                    bDone = TRUE;
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
    DBG_ASSERT( pLine, "Keine Zeile erhalten: GetChar" );

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
                        // pruefen, ob dieser oder der davor...
                        long nTmpPosMin = x ? pLine->GetCharPosArray().GetObject( nTmpCurIndex+x-1 ) : 0;
                        long nDiffLeft = nXInPortion - nTmpPosMin;
                        long nDiffRight = nTmpPosMax - nXInPortion;
                        DBG_ASSERT( nDiffLeft >= 0, "DiffLeft negativ" );
                        DBG_ASSERT( nDiffRight >= 0, "DiffRight negativ" );
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

                // Bei Verwendung des CharPosArray duerfte es keine Ungenauigkeiten geben!
                // Vielleicht bei Kerning ?
                // 0xFFF passiert z.B. bei Outline-Font, wenn ganz hinten.
                if ( nOffset == 0xFFFF )
                    nOffset = nMax;

                DBG_ASSERT( nOffset <= nMax, "nOffset > nMax" );

                nChar += nOffset;

                // Check if index is within a cell:
                if ( nChar && ( nChar < pParaPortion->GetNode()->Len() ) )
                {
                    EditPaM aPaM( pParaPortion->GetNode(), nChar+1 );
                    USHORT nScriptType = GetScriptType( aPaM );
                    if ( nScriptType == i18n::ScriptType::COMPLEX )
                    {
                        uno::Reference < i18n::XBreakIterator > xBI = ImplGetBreakIterator();
                        sal_Int32 nCount = 1;
                        lang::Locale aLocale = GetLocale( aPaM );
                        USHORT nRight = (USHORT)xBI->nextCharacters( *pParaPortion->GetNode(), nChar, aLocale, ::com::sun::star::i18n::CharacterIteratorMode::SKIPCELL, nCount, nCount );
                        USHORT nLeft = (USHORT)xBI->previousCharacters( *pParaPortion->GetNode(), nRight, aLocale, ::com::sun::star::i18n::CharacterIteratorMode::SKIPCELL, nCount, nCount );
                        if ( ( nLeft != nChar ) && ( nRight != nChar ) )
                        {
                            nChar = ( Abs( nRight - nChar ) < Abs( nLeft - nChar ) ) ? nRight : nLeft;
                        }
                    }
                }
            }
        }

        nCurIndex += pPortion->GetLen();
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
//          case PORTIONKIND_EXTRASPACE:
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
            // Portions behind must be ermoved, visual behind this portion
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
    DBG_ASSERT( pLine, "Keine Zeile erhalten: GetXPos" );
    DBG_ASSERT( ( nIndex >= pLine->GetStart() ) && ( nIndex <= pLine->GetEnd() ) , "GetXPos muss richtig gerufen werden!" );

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
                        // DBG_ASSERT( !bPreferPortionStart, "GetXPos - How can we this tab portion here???" );
                        // #109879# We loop if nIndex == pLine->GetEnd, because bPreferPortionStart will be reset
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

            long nPosInPortion = pLine->GetCharPosArray().GetObject( nIndex - 1 - pLine->GetStart() );

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
                        const long* pDXArray = pLine->GetCharPosArray().GetData()+( nTextPortionStart-pLine->GetStart() );
                        long nCharWidth = ( ( (n+1) < pPortion->GetLen() ) ? pDXArray[n] : pPortion->GetSize().Width() )
                                                        - ( n ? pDXArray[n-1] : 0 );
                        if ( (n+1) < pPortion->GetLen() )
                        {
                            // smaller, when char behind is CHAR_PUNCTUATIONRIGHT also
                            nType = GetCharTypeForCompression( pParaPortion->GetNode()->GetChar( nIndex+1 ) );
                            if ( nType == CHAR_PUNCTUATIONRIGHT )
                            {
                                long nNextCharWidth = ( ( (n+2) < pPortion->GetLen() ) ? pDXArray[n+1] : pPortion->GetSize().Width() )
                                                                - pDXArray[n];
                                long nCompressed = nNextCharWidth/2;
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

    // innerhalb der Zeile suchen...
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

vos::ORef<SvxForbiddenCharactersTable> ImpEditEngine::GetForbiddenCharsTable( BOOL bGetInternal ) const
{
    vos::ORef<SvxForbiddenCharactersTable> xF = xForbiddenCharsTable;
    if ( !xF.isValid() && bGetInternal )
        xF = EE_DLL()->GetGlobalData()->GetForbiddenCharsTable();
    return xF;
}

void ImpEditEngine::SetForbiddenCharsTable( vos::ORef<SvxForbiddenCharactersTable> xForbiddenChars )
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

BOOL ImpEditEngine::DoVisualCursorTraveling( const ContentNode* pNode )
{
    // Don't check if it's necessary, because we also need it when leaving the paragraph
    return IsVisualCursorTravelingEnabled();
/*
    BOOL bDoVisualCursorTraveling = FALSE;

    if ( IsVisualCursorTravelingEnabled() && pNode->Len() )
    {
        // Only necessary when RTL text in LTR para or LTR text in RTL para
        bDoVisualCursorTraveling = HasDifferentRTLLevels( pNode );
    }

    return bDoVisualCursorTraveling;
*/
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
