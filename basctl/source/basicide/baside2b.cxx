/*************************************************************************
 *
 *  $RCSfile: baside2b.cxx,v $
 *
 *  $Revision: 1.33 $
 *
 *  last change: $Author: hr $ $Date: 2003-11-05 12:38:14 $
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


#include <svtools/svarray.hxx>
#define _BASIC_TEXTPORTIONS
#include <basic/sbdef.hxx>
#include <ide_pch.hxx>

#pragma hdrstop

#ifndef _SBXCLASS_HXX //autogen
#include <svtools/sbx.hxx>
#endif
#include <vcl/sound.hxx>
#include <svtools/xtextedt.hxx>
#include <svtools/txtattr.hxx>
#include <svtools/textwindowaccessibility.hxx>

#include <helpid.hrc>
#include <baside2.hrc>
#include <baside2.hxx>
#include <brkdlg.hxx>
#include <objdlg.hxx>
#include <basobj.hxx>
#include <iderdll.hxx>
#include <iderdll2.hxx>

#include <vcl/system.hxx>

#ifndef _SV_TASKPANELIST_HXX
#include <vcl/taskpanelist.hxx>
#endif

#ifndef _SV_HELP_HXX //autogen
#include <vcl/help.hxx>
#endif

//#ifndef _SFX_HELP_HXX //autogen
//#include <sfx2/sfxhelp.hxx>
//#endif

#ifndef _SVTOOLS_SOURCEVIEWCONFIG_HXX
#include <svtools/sourceviewconfig.hxx>
#endif

#ifndef _COM_SUN_STAR_SCRIPT_XLIBRYARYCONTAINER2_HPP_
#include <com/sun/star/script/XLibraryContainer2.hpp>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


long nVirtToolBoxHeight;    // wird im WatchWindow init., im Stackwindow verw.

#define SCROLL_LINE     12
#define SCROLL_PAGE     60

#define DWBORDER        3

static const char cSuffixes[] = "%&!#@$";

MapUnit eEditMapUnit = MAP_100TH_MM;


// #108672 Helper functions to get/set text in TextEngine
// using the stream interface (get/setText() only supports
// tools Strings limited to 64K).
::rtl::OUString getTextEngineText( ExtTextEngine* pEngine )
{
    SvMemoryStream aMemStream;
    aMemStream.SetStreamCharSet( RTL_TEXTENCODING_UTF8 );
    aMemStream.SetLineDelimiter( LINEEND_LF );
    pEngine->Write( aMemStream );
    ULONG nSize = aMemStream.Tell();
    ::rtl::OUString aText( (const sal_Char*)aMemStream.GetData(),
        nSize, RTL_TEXTENCODING_UTF8 );
    return aText;
}

void setTextEngineText( ExtTextEngine* pEngine, const ::rtl::OUString aStr )
{
    pEngine->SetText( String() );
    ::rtl::OString aUTF8Str = ::rtl::OUStringToOString( aStr, RTL_TEXTENCODING_UTF8 );
    SvMemoryStream aMemStream( (void*)aUTF8Str.getStr(), aUTF8Str.getLength(),
        STREAM_READ | STREAM_SEEK_TO_BEGIN );
    aMemStream.SetStreamCharSet( RTL_TEXTENCODING_UTF8 );
    aMemStream.SetLineDelimiter( LINEEND_LF );
    pEngine->Read( aMemStream );
}


void lcl_GetValues( String& rWatchStr, SbxDimArray* pArray,
                            short* pIdx, USHORT nCurrentDim )
{
    USHORT nDims = pArray->GetDims();
    short nMin, nMax;
    pArray->GetDim( nCurrentDim, nMin, nMax );
    for ( USHORT n = nMin; n <= nMax; n++ )
    {
//      pIdx[nDims-nCurrentDim] = n;
        pIdx[nCurrentDim-1] = n;
//      if ( nCurrentDim > 1 )
        if ( nCurrentDim < nDims )
            rWatchStr += String( RTL_CONSTASCII_USTRINGPARAM( "[ " ) );

//      if ( nCurrentDim > 1 )
        if ( nCurrentDim < nDims )
//          lcl_GetValues( rWatchStr, pArray, pIdx, nCurrentDim-1 );
            lcl_GetValues( rWatchStr, pArray, pIdx, nCurrentDim+1 );
        else
        {
            SbxBase* pElem = pArray->Get( pIdx );
            if ( pElem )
                rWatchStr += ((SbxVariable*)pElem)->GetString();
            if ( n < nMax )
                rWatchStr += String( RTL_CONSTASCII_USTRINGPARAM( "; " ) );
        }
//      if ( nCurrentDim > 1 )
        if ( nCurrentDim < nDims )
            rWatchStr += String( RTL_CONSTASCII_USTRINGPARAM( " ]" ) );
    }
}


SbxBase* lcl_FindElement( SbxDimArray* pArray, const String& rIndex, BOOL& rbValidIndex )
{
    rbValidIndex = FALSE;
    SbxBase* pElem = 0;

    USHORT nTokens = rIndex.GetTokenCount( ',' );
    USHORT nDims = pArray->GetDims();
    if ( nDims == nTokens )
    {
        short* pIdx = new short[ nDims ];
        for ( USHORT n = nDims; n; )
            pIdx[--n] = 0;

        rbValidIndex = TRUE;
        for ( USHORT nDim = nTokens; nDim; nDim--,rbValidIndex )
        {
            String aTmpIndex( rIndex.GetToken( nDims-nDim, ',' ) );
            aTmpIndex.EraseLeadingChars();
            aTmpIndex.EraseTrailingChars();
            short nIndex = 0x7FFF;

            if ( ByteString( aTmpIndex, RTL_TEXTENCODING_UTF8 ).IsNumericAscii() )
                nIndex = (USHORT) aTmpIndex.ToInt32();
            else
            {
                SbxBase* pIndexSBX = StarBASIC::FindSBXInCurrentScope( aTmpIndex );
                if ( pIndexSBX && pIndexSBX->ISA( SbxVariable ) && !pIndexSBX->ISA( SbxMethod ) )
                {
                    SbxVariable* pIndexVar = (SbxVariable*)pIndexSBX;
                    SbxDataType eType = pIndexVar->GetType();
                    if ( ( (BYTE)eType != (BYTE)SbxOBJECT )
                            && !( eType & SbxARRAY ) )
                    {
                        nIndex = (USHORT) pIndexVar->GetString().ToInt32();
                    }
                }
            }

            short nMin, nMax;
            pArray->GetDim( nDim, nMin, nMax );
            if ( ( nIndex >= nMin ) && ( nIndex <= nMax ) )
                pIdx[nDims-nDim] = nIndex;
            else
                rbValidIndex = FALSE;   // Und Abbruch...
        }

        if ( rbValidIndex )
            pElem = pArray->Get( pIdx );
    }

    return pElem;
}

void lcl_FormatArrayString( String& rResult )
{
    USHORT nLastSep = 0xFFFF;

    // Format eines mehrdimensionalen Arrays:
    // Beliebig tief geschachtelt.
    // {[ [ 1;2 ][ 3;4 ] ] [ 5;6 ][ 7;8 ] ] ]}

    // Geschw. Klammern nur dann entfernen, wenn sie genau am Anfang und Ende
    // stehen, falls es einfach nur char's in einem String sind.
    if ( ( rResult.Len() > 1 ) && ( rResult.GetChar( 0 ) == '{' ) && ( rResult.GetChar( rResult.Len() - 1 ) == '}') )
    {
        rResult.Erase( 0, 1 );
        rResult.Erase( rResult.Len()-1, 1 );
        rResult.EraseLeadingChars();
        rResult.EraseTrailingChars();
    }

    // Jetzt die Klammern abarbeiten:
    // Vor einer '[' muss ein Separator gewesen sein (oder Start)
    // String stammen.
    // Nach einer ']' muss ein Blank oder ein ']' oder ein '[' stehen.
    USHORT nIndex = 0;
    USHORT nLevel = 0;
    char cLastSep = ' ';
    BOOL bLastWasSep = TRUE;
    while ( nIndex < rResult.Len() )
    {
        switch ( rResult.GetChar( nIndex ) )
        {
            case '[':
            {
                if ( bLastWasSep )
                {
                    rResult.Erase( nIndex, 1 );
                    while ( ( nIndex < rResult.Len() ) && ( rResult.GetChar( nIndex ) == ' ' ) )
                        rResult.Erase( nIndex, 1 );
                    nLevel++;
                    cLastSep = '[';
                    bLastWasSep = TRUE;
                }
                else
                    nIndex++;
            }
            break;
            case ']':
            {
                USHORT nNext = nIndex+1;
                if ( nLevel &&
                    ( ( nNext == rResult.Len() ) || ( rResult.GetChar( nNext ) == ' ' )
                        || ( rResult.GetChar( nNext ) == '[' ) || ( rResult.GetChar( nNext ) == ']' ) ) )
                {
                    rResult.Erase( nIndex, 1 );
                    while ( ( nIndex < rResult.Len() ) && ( rResult.GetChar( nIndex ) == ' ' ) )
                        rResult.Erase( nIndex, 1 );
                    nLevel--;
                    // vor einer ']' steht keine ';':
                    if ( cLastSep == '[' )
                        rResult.Insert( ';', nIndex++ );
                    cLastSep = ']';
                    bLastWasSep = TRUE;
                }
                else
                    nIndex++;
            }
            break;
            default:
            {
                nIndex++;
                bLastWasSep = FALSE;
            }
        }
    }
//  if ( rResult.Len() && rResult[(USHORT)(rResult.Len()-1)] == ';' )
//      rResult.Erase( rResult.Len()-1 );
}

void lcl_DrawIDEWindowFrame( DockingWindow* pWin )
{
    // The result of using explicit colors here appears to be harmless when
    // switching to high contrast mode:
    if ( !pWin->IsFloatingMode() )
    {
        Size aSz = pWin->GetOutputSizePixel();
        long nY = nVirtToolBoxHeight - 1;
        const Color aOldLineColor( pWin->GetLineColor() );
        pWin->SetLineColor( Color( COL_WHITE ) );
        // oben eine weisse..
        pWin->DrawLine( Point( 0, 0 ), Point( aSz.Width(), 0 ) );
        // unten eine schwarze...
        pWin->SetLineColor( Color( COL_BLACK ) );
        pWin->DrawLine( Point( 0, aSz.Height() - 1 ), Point( aSz.Width(), aSz.Height() - 1 ) );
        pWin->SetLineColor( aOldLineColor );
    }
}

void lcl_SeparateNameAndIndex( const String& rVName, String& rVar, String& rIndex )
{
    rVar = rVName;
    rIndex.Erase();
    USHORT nIndexStart = rVar.Search( '(' );
    if ( nIndexStart != STRING_NOTFOUND )
    {
        USHORT nIndexEnd = rVar.Search( ')', nIndexStart );
        if ( nIndexStart != STRING_NOTFOUND )
        {
            rIndex = rVar.Copy( nIndexStart+1, nIndexEnd-nIndexStart-1 );
            rVar.Erase( nIndexStart );
            rVar.EraseTrailingChars();
            rIndex.EraseLeadingChars();
            rIndex.EraseTrailingChars();
        }
    }

    if ( rVar.Len() )
    {
        USHORT nLastChar = rVar.Len()-1;
        if ( strchr( cSuffixes, rVar.GetChar( nLastChar ) ) )
            rVar.Erase( nLastChar, 1 );
    }
    if ( rIndex.Len() )
    {
        USHORT nLastChar = rIndex.Len()-1;
        if ( strchr( cSuffixes, rIndex.GetChar( nLastChar ) ) )
            rIndex.Erase( nLastChar, 1 );
    }
}


EditorWindow::EditorWindow( Window* pParent ) :
    Window( pParent, WB_BORDER )
{
    bDoSyntaxHighlight = TRUE;
    bDelayHighlight = TRUE;
    pModulWindow = 0;
    pEditView = 0;
    pEditEngine = 0;
    pSourceViewConfig = new svt::SourceViewConfig;
    bHighlightning = FALSE;
    pProgress = 0;
    nCurTextWidth = 0;
    SetBackground(
        Wallpaper(GetSettings().GetStyleSettings().GetFieldColor()));
    SetPointer( Pointer( POINTER_TEXT ) );

    SetHelpId( HID_BASICIDE_EDITORWINDOW );

    StartListening( *pSourceViewConfig );
}



__EXPORT EditorWindow::~EditorWindow()
{
    EndListening( *pSourceViewConfig );
    delete pSourceViewConfig;

    aSyntaxIdleTimer.Stop();
    aHelpAgentTimer.Stop();

    if ( pEditEngine )
    {
        EndListening( *pEditEngine );
        pEditEngine->RemoveView( pEditView );
//      pEditEngine->SetViewWin( 0 );
        delete pEditView;
        delete pEditEngine;
    }
}

String EditorWindow::GetWordAtCursor()
{
//  ESelection aESel( pEditView->GetSelection() );
    // Nicht den Selektierten Bereich, sondern an der CursorPosition,
    // falls Teil eines Worts markiert.
    String aWord( pEditView->GetTextEngine()->GetWord( pEditView->GetSelection().GetEnd() ) );
    // Kann leer sein, wenn komplettes Word markiert, da Cursor dahinter.
    if ( !aWord.Len() && pEditView->HasSelection() )
        aWord = pEditView->GetTextEngine()->GetWord( pEditView->GetSelection().GetStart() );
    return aWord ;
}

void __EXPORT EditorWindow::RequestHelp( const HelpEvent& rHEvt )
{
    BOOL bDone = FALSE;

    // Sollte eigentlich mal aktiviert werden...
    if ( pEditEngine )
    {
        if ( rHEvt.GetMode() & HELPMODE_CONTEXT )
        {
            String aKeyword = GetWordAtCursor();
            Application::GetHelp()->Start( aKeyword, this );
            bDone = TRUE;
        }
        else if ( rHEvt.GetMode() & HELPMODE_QUICK )
        {
            String aHelpText;
            Point aTopLeft;
            if ( StarBASIC::IsRunning() )
            {
                Point aWindowPos = rHEvt.GetMousePosPixel();
                aWindowPos = ScreenToOutputPixel( aWindowPos );
                Point aDocPos = GetEditView()->GetDocPos( aWindowPos );
                TextPaM aCursor = GetEditView()->GetTextEngine()->GetPaM( aDocPos, FALSE );
                TextPaM aStartOfWord;
                String aWord = GetEditView()->GetTextEngine()->GetWord( aCursor, &aStartOfWord );
                if ( aWord.Len() && !ByteString( aWord, RTL_TEXTENCODING_UTF8 ).IsNumericAscii() )
                {
                    USHORT nLastChar =aWord.Len()-1;
                    if ( strchr( cSuffixes, aWord.GetChar( nLastChar ) ) )
                        aWord.Erase( nLastChar, 1 );
                    SbxBase* pSBX = StarBASIC::FindSBXInCurrentScope( aWord );
                    if ( pSBX && pSBX->ISA( SbxVariable ) && !pSBX->ISA( SbxMethod ) )
                    {
                        SbxVariable* pVar = (SbxVariable*)pSBX;
                        SbxDataType eType = pVar->GetType();
                        if ( (BYTE)eType == (BYTE)SbxOBJECT )
                            // Kann zu Absturz, z.B. bei Selections-Objekt fuehren
                            // Type == Object heisst nicht, dass pVar == Object!
                            ; // aHelpText = ((SbxObject*)pVar)->GetClassName();
                        else if ( eType & SbxARRAY )
                            ; // aHelpText = "{...}";
                        else if ( (BYTE)eType != (BYTE)SbxEMPTY )
                        {
                            aHelpText = pVar->GetName();
                            if ( !aHelpText.Len() )     // Bei Uebergabeparametern wird der Name nicht kopiert
                                aHelpText = aWord;
                            aHelpText += '=';
                            aHelpText += pVar->GetString();
                        }
                    }
                    if ( aHelpText.Len() )
                    {
                        aTopLeft = GetEditView()->GetTextEngine()->PaMtoEditCursor( aStartOfWord ).BottomLeft();
                        aTopLeft = GetEditView()->GetWindowPos( aTopLeft );
                        aTopLeft.X() += 5;
                        aTopLeft.Y() += 5;
                        aTopLeft = OutputToScreenPixel( aTopLeft );
                    }
                }
            }
            Help::ShowQuickHelp( this, Rectangle( aTopLeft, Size( 1, 1 ) ), aHelpText, QUICKHELP_TOP|QUICKHELP_LEFT);
            bDone = TRUE;
        }
    }

    if ( !bDone )
        Window::RequestHelp( rHEvt );
}


void __EXPORT EditorWindow::Resize()
{
    // ScrollBars, etc. passiert in Adjust...
    if ( pEditView )
    {
        long nVisY = pEditView->GetStartDocPos().Y();
//      pEditView->SetOutputArea( Rectangle( Point( 0, 0 ), GetOutputSize() ) );
        pEditView->ShowCursor();
        Size aOutSz( GetOutputSizePixel() );
        long nMaxVisAreaStart = pEditView->GetTextEngine()->GetTextHeight() - aOutSz.Height();
        if ( nMaxVisAreaStart < 0 )
            nMaxVisAreaStart = 0;
        if ( pEditView->GetStartDocPos().Y() > nMaxVisAreaStart )
        {
            Point aStartDocPos( pEditView->GetStartDocPos() );
            aStartDocPos.Y() = nMaxVisAreaStart;
            pEditView->SetStartDocPos( aStartDocPos );
            pEditView->ShowCursor();
            pModulWindow->GetBreakPointWindow().GetCurYOffset() = aStartDocPos.Y();
        }
        InitScrollBars();
        if ( nVisY != pEditView->GetStartDocPos().Y() )
            Invalidate();
    }
}



void __EXPORT EditorWindow::MouseMove( const MouseEvent &rEvt )
{
    if ( pEditView )
        pEditView->MouseMove( rEvt );
}



void __EXPORT EditorWindow::MouseButtonUp( const MouseEvent &rEvt )
{
    if ( pEditView )
    {
        pEditView->MouseButtonUp( rEvt );
        BasicIDE::GetBindings().Invalidate( SID_BASICIDE_STAT_POS );
    }
}

void __EXPORT EditorWindow::MouseButtonDown( const MouseEvent &rEvt )
{
    GrabFocus();
    if ( pEditView )
    {
        pEditView->MouseButtonDown( rEvt );
        if ( SFX_APP()->GetHelpPI() )
            aHelpAgentTimer.Start();
    }
}

void __EXPORT EditorWindow::Command( const CommandEvent& rCEvt )
{
    if ( pEditView )
    {
        pEditView->Command( rCEvt );
        if ( ( rCEvt.GetCommand() == COMMAND_WHEEL ) ||
             ( rCEvt.GetCommand() == COMMAND_STARTAUTOSCROLL ) ||
             ( rCEvt.GetCommand() == COMMAND_AUTOSCROLL ) )
        {
            HandleScrollCommand( rCEvt, pModulWindow->GetHScrollBar(), &pModulWindow->GetEditVScrollBar() );
        }
    }
}

/*
BOOL __EXPORT EditorWindow::Drop( const DropEvent& rEvt )
{
    BOOL bDone = FALSE;
    if ( pEditView && ImpCanModify() )
    {
        bDone = pEditView->Drop( rEvt );
        if ( bDone )
        {
            SfxBindings& rBindings = BasicIDE::GetBindings();
            rBindings.Invalidate( SID_BASICIDE_STAT_POS );
            rBindings.Invalidate( SID_SAVEDOC );
            rBindings.Invalidate( SID_DOC_MODIFIED );
        }
    }
    return bDone;
}

BOOL __EXPORT EditorWindow::QueryDrop( DropEvent& rEvt )
{
    if ( pEditView )
        return pEditView->QueryDrop( rEvt );
    return FALSE;
}
*/

BOOL EditorWindow::ImpCanModify()
{
    BOOL bCanModify = TRUE;
    if ( StarBASIC::IsRunning() )
    {
        // Wenn im Trace-Mode, entweder Trace abbrechen oder
        // Eingabe verweigern
        // Im Notify bei Basic::Stoped die Markierungen in den Modulen
        // entfernen!
        if ( QueryBox( 0, WB_OK_CANCEL, String( IDEResId( RID_STR_WILLSTOPPRG ) ) ).Execute() == RET_OK )
        {
            pModulWindow->GetBasicStatus().bIsRunning = FALSE;
            BasicIDE::StopBasic();
        }
        else
            bCanModify = FALSE;
    }
    return bCanModify;
}

void __EXPORT EditorWindow::KeyInput( const KeyEvent& rKEvt )
{
    if ( !pEditView )   // Passiert unter W95 bei letzte Version, Ctrl-Tab
        return;

#if OSL_DEBUG_LEVEL > 1
    Range aRange = pModulWindow->GetHScrollBar()->GetRange();
    long nVisSz = pModulWindow->GetHScrollBar()->GetVisibleSize();
    long nPapSz = pModulWindow->GetHScrollBar()->GetPageSize();
    long nLinSz = pModulWindow->GetHScrollBar()->GetLineSize();
    long nThumb = pModulWindow->GetHScrollBar()->GetThumbPos();
#endif
    BOOL bDone = FALSE;
    BOOL bWasModified = pEditEngine->IsModified();
    if ( !TextEngine::DoesKeyChangeText( rKEvt ) || ImpCanModify() )
    {
        if ( ( rKEvt.GetKeyCode().GetCode() == KEY_A) && rKEvt.GetKeyCode().IsMod1() )
            pEditView->SetSelection( TextSelection( TextPaM( 0, 0 ), TextPaM( 0xFFFFFFFF, 0xFFFF ) ) );
        else if ( ( rKEvt.GetKeyCode().GetCode() == KEY_Y ) && rKEvt.GetKeyCode().IsMod1() )
            bDone = TRUE; // CTRL-Y schlucken, damit kein Vorlagenkatalog
        else
        {
            if ( ( rKEvt.GetKeyCode().GetCode() == KEY_TAB ) && !rKEvt.GetKeyCode().IsMod1() && !rKEvt.GetKeyCode().IsMod2() )
            {
                TextSelection aSel( pEditView->GetSelection() );
                if ( aSel.GetStart().GetPara() != aSel.GetEnd().GetPara() )
                {
                    bDelayHighlight = FALSE;
                    if ( !rKEvt.GetKeyCode().IsShift() )
                        pEditView->IndentBlock();
                    else
                        pEditView->UnindentBlock();
                    bDelayHighlight = TRUE;
                    bDone = TRUE;
                }
            }
            if ( !bDone )
                bDone = pEditView->KeyInput( rKEvt );
        }
    }
#ifndef PRODUCT
    if( !bDone )
    {
        if ( ( rKEvt.GetKeyCode().GetCode() == KEY_H ) &&
            rKEvt.GetKeyCode().IsMod1() && rKEvt.GetKeyCode().IsMod2() )
        {
            ToggleHighlightMode();
        }
    }
#endif
    if ( !bDone )
    {
        if ( !SfxViewShell::Current()->KeyInput( rKEvt ) )
            Window::KeyInput( rKEvt );
    }
    else
    {
        SfxBindings& rBindings = BasicIDE::GetBindings();
        rBindings.Invalidate( SID_BASICIDE_STAT_POS );
        if ( rKEvt.GetKeyCode().GetGroup() == KEYGROUP_CURSOR )
            rBindings.Update( SID_BASICIDE_STAT_POS );
        if ( !bWasModified && pEditEngine->IsModified() )
        {
            rBindings.Invalidate( SID_SAVEDOC );
            rBindings.Invalidate( SID_DOC_MODIFIED );
        }
        if( rKEvt.GetKeyCode().GetCode() == KEY_INSERT )
            rBindings.Invalidate( SID_ATTR_INSERT );
        if ( SFX_APP()->GetHelpPI() )
            aHelpAgentTimer.Start();
    }
}

void __EXPORT EditorWindow::Paint( const Rectangle& rRect )
{
    if ( !pEditEngine )     // spaetestens jetzt brauche ich sie...
        CreateEditEngine();

    pEditView->Paint( rRect );
}

void __EXPORT EditorWindow::LoseFocus()
{
    SetSourceInBasic();
    Window::LoseFocus();
}

BOOL EditorWindow::SetSourceInBasic( BOOL bQuiet )
{
    BOOL bChanged = FALSE;
    if ( pEditEngine && pEditEngine->IsModified() )
    {
        if ( !StarBASIC::IsRunning() ) // Nicht zur Laufzeit!
        {
            ::rtl::OUString aModule = getTextEngineText( pEditEngine );

            // update module in basic
            SbModule* pModule = pModulWindow->GetSbModule();
            DBG_ASSERT(pModule, "EditorWindow::SetSourceInBasic: No Module found!");

            // update module in module window
            pModulWindow->SetModule( aModule );

            // update module in library
            SfxObjectShell* pShell = pModulWindow->GetShell();
            String aLibName = pModulWindow->GetLibName();
            String aName = pModulWindow->GetName();
            BasicIDE::UpdateModule( pShell, aLibName, aName, aModule );

            pEditEngine->SetModified( FALSE );
            BasicIDE::MarkDocShellModified( pShell );
            bChanged = TRUE;
        }
    }
    return bChanged;
}


void EditorWindow::CreateEditEngine()
{
    if ( pEditEngine )
        return;

    pEditEngine = new ExtTextEngine;
    pEditView = new ExtTextView( pEditEngine, this );
    pEditView->SetAutoIndentMode( TRUE );
    pEditEngine->SetUpdateMode( FALSE );
    pEditEngine->InsertView( pEditView );

    ImplSetFont();

    aSyntaxIdleTimer.SetTimeout( 200 );
    aSyntaxIdleTimer.SetTimeoutHdl( LINK( this, EditorWindow, SyntaxTimerHdl ) );

    aHelpAgentTimer.SetTimeout( 2000 );
    aHelpAgentTimer.SetTimeoutHdl( LINK( this, EditorWindow, HelpAgentTimerHdl ) );

    aHighlighter.initialize( HIGHLIGHT_BASIC );

    bDoSyntaxHighlight = FALSE; // Bei grossen Texten zu langsam...
    ::rtl::OUString aOUSource( pModulWindow->GetModule() );
    sal_Int32 nLines = 0;
    sal_Int32 nIndex = -1;
    do
    {
        nLines++;
        nIndex = aOUSource.indexOf( LINE_SEP, nIndex+1 );
    }
    while ( nIndex >= 0 );

    // nLines*4: SetText+Formatting+DoHighlight+Formatting
    // 1 Formatting koennte eingespart werden, aber dann wartet man
    // bei einem langen Sourcecode noch laenger auf den Text...
    pProgress = new ProgressInfo( IDE_DLL()->GetShell()->GetViewFrame()->GetObjectShell(), String( IDEResId( RID_STR_GENERATESOURCE ) ), nLines*4 );
    setTextEngineText( pEditEngine, aOUSource );

    pEditView->SetStartDocPos( Point( 0, 0 ) );
    pEditView->SetSelection( TextSelection() );
    pModulWindow->GetBreakPointWindow().GetCurYOffset() = 0;
    pEditEngine->SetUpdateMode( TRUE );
    Update();   // Es wurde bei UpdateMode = TRUE nur Invalidiert

    // Die anderen Fenster auch, damit keine halben Sachen auf dem Bildschirm!
    pModulWindow->GetLayout()->GetWatchWindow().Update();
    pModulWindow->GetLayout()->GetStackWindow().Update();
    pModulWindow->GetBreakPointWindow().Update();

    pEditView->ShowCursor( TRUE, TRUE );

    StartListening( *pEditEngine );

    // Das Syntax-Highlightning legt ein rel. groesse VDev an.
    aSyntaxIdleTimer.Stop();
    bDoSyntaxHighlight = TRUE;


    for ( USHORT nLine = 0; nLine < nLines; nLine++ )
        aSyntaxLineTable.Insert( nLine, (void*)(USHORT)1 );
    ForceSyntaxTimeout();

    DELETEZ( pProgress );

    pEditView->EraseVirtualDevice();
    pEditEngine->SetModified( FALSE );
    pEditEngine->EnableUndo( TRUE );

    InitScrollBars();

    BasicIDE::GetBindings().Invalidate( SID_BASICIDE_STAT_POS );

    DBG_ASSERT( pModulWindow->GetBreakPointWindow().GetCurYOffset() == 0, "CreateEditEngine: Brechpunkte verschoben?" );

    // set readonly mode for readonly libraries
    SfxObjectShell* pShell = pModulWindow->GetShell();
    ::rtl::OUString aOULibName( pModulWindow->GetLibName() );
    Reference< script::XLibraryContainer2 > xModLibContainer( BasicIDE::GetModuleLibraryContainer( pShell ), UNO_QUERY );
    if ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) && xModLibContainer->isLibraryReadOnly( aOULibName ) )
    {
        pModulWindow->SetReadOnly( TRUE );
    }

    if ( pShell && pShell->IsReadOnly() )
        pModulWindow->SetReadOnly( TRUE );
}

// virtual
void EditorWindow::DataChanged(DataChangedEvent const & rDCEvt)
{
    Window::DataChanged(rDCEvt);
    if (rDCEvt.GetType() == DATACHANGED_SETTINGS
        && (rDCEvt.GetFlags() & SETTINGS_STYLE) != 0)
    {
        Color aColor(GetSettings().GetStyleSettings().GetFieldColor());
        if (aColor
            != rDCEvt.GetOldSettings()->GetStyleSettings().GetFieldColor())
        {
            SetBackground(Wallpaper(aColor));
            Invalidate();
        }
        if (pEditEngine != 0)
        {
            aColor = GetSettings().GetStyleSettings().GetFieldTextColor();
            if (aColor != rDCEvt.GetOldSettings()->
                GetStyleSettings().GetFieldTextColor())
            {
                Font aFont(pEditEngine->GetFont());
                aFont.SetColor(aColor);
                pEditEngine->SetFont(aFont);
            }
        }
    }
}

void EditorWindow::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.ISA( TextHint ) )
    {
        const TextHint& rTextHint = (const TextHint&)rHint;
        if( rTextHint.GetId() == TEXT_HINT_VIEWSCROLLED )
        {
            if ( pModulWindow->GetHScrollBar() )
                pModulWindow->GetHScrollBar()->SetThumbPos( pEditView->GetStartDocPos().X() );
            pModulWindow->GetEditVScrollBar().SetThumbPos( pEditView->GetStartDocPos().Y() );
            pModulWindow->GetBreakPointWindow().Scroll( 0, pModulWindow->GetBreakPointWindow().GetCurYOffset() - pEditView->GetStartDocPos().Y() );
        }
        else if( rTextHint.GetId() == TEXT_HINT_TEXTHEIGHTCHANGED )
        {
            if ( pEditView->GetStartDocPos().Y() )
            {
                long nOutHeight = GetOutputSizePixel().Height();
                long nTextHeight = pEditEngine->GetTextHeight();
                if ( nTextHeight < nOutHeight )
                    pEditView->Scroll( 0, pEditView->GetStartDocPos().Y() );
            }

            SetScrollBarRanges();
        }
        else if( rTextHint.GetId() == TEXT_HINT_TEXTFORMATTED )
        {
            if ( pModulWindow->GetHScrollBar() )
            {
                ULONG nWidth = pEditEngine->CalcTextWidth();
                if ( (long)nWidth != nCurTextWidth )
                {
                    nCurTextWidth = nWidth;
                    pModulWindow->GetHScrollBar()->SetRange( Range( 0, (long)nCurTextWidth-1) );
                    pModulWindow->GetHScrollBar()->SetThumbPos( pEditView->GetStartDocPos().X() );
                }
            }
        }
        else if( rTextHint.GetId() == TEXT_HINT_PARAINSERTED )
        {
            ParagraphInsertedDeleted( rTextHint.GetValue(), TRUE );
        }
        else if( rTextHint.GetId() == TEXT_HINT_PARAREMOVED )
        {
            ParagraphInsertedDeleted( rTextHint.GetValue(), FALSE );
        }
        else if( rTextHint.GetId() == TEXT_HINT_FORMATPARA )
        {
            DoDelayedSyntaxHighlight( rTextHint.GetValue() );
        }
    }
    else if ( &rBC == pSourceViewConfig )
    {
        ImplSetFont();
    }
}

void EditorWindow::SetScrollBarRanges()
{
    // Extra-Methode, nicht InitScrollBars, da auch fuer EditEngine-Events.
    if ( !pEditEngine )
        return;

    if ( pModulWindow->GetHScrollBar() )
        pModulWindow->GetHScrollBar()->SetRange( Range( 0, nCurTextWidth-1 ) );

    pModulWindow->GetEditVScrollBar().SetRange( Range( 0, pEditEngine->GetTextHeight()-1 ) );
}

void EditorWindow::InitScrollBars()
{
    if ( !pEditEngine )
        return;

    SetScrollBarRanges();
    Size aOutSz( GetOutputSizePixel() );
    pModulWindow->GetEditVScrollBar().SetVisibleSize( aOutSz.Height() );
    pModulWindow->GetEditVScrollBar().SetPageSize( aOutSz.Height() * 8 / 10 );
    pModulWindow->GetEditVScrollBar().SetLineSize( GetTextHeight() );
    pModulWindow->GetEditVScrollBar().SetThumbPos( pEditView->GetStartDocPos().Y() );
    pModulWindow->GetEditVScrollBar().Show();

    if ( pModulWindow->GetHScrollBar() )
    {
        pModulWindow->GetHScrollBar()->SetVisibleSize( aOutSz.Width() );
        pModulWindow->GetHScrollBar()->SetPageSize( aOutSz.Width() * 8 / 10 );
        pModulWindow->GetHScrollBar()->SetLineSize( GetTextWidth( 'x' ) );
        pModulWindow->GetHScrollBar()->SetThumbPos( pEditView->GetStartDocPos().X() );
        pModulWindow->GetHScrollBar()->Show();
    }
}

void EditorWindow::ImpDoHighlight( ULONG nLine )
{
    String aLine( pEditEngine->GetText( nLine ) );
    Range aChanges = aHighlighter.notifyChange( nLine, 0, &aLine, 1 );
    if ( aChanges.Len() )
    {
        for ( long n = aChanges.Min() + 1; n <= aChanges.Max(); n++ )
            aSyntaxLineTable.Insert( n, (void*)(ULONG)1 );
        aSyntaxIdleTimer.Start();
    }

    BOOL bWasModified = pEditEngine->IsModified();

    HighlightPortions aPortions;
    aHighlighter.getHighlightPortions( nLine, aLine, aPortions );
    USHORT nCount = aPortions.Count();
    for ( USHORT i = 0; i < nCount; i++ )
    {
        HighlightPortion& r = aPortions[i];
        const Color& rColor = ((ModulWindowLayout*)pModulWindow->GetLayoutWindow())->getSyntaxColor(r.tokenType);
        pEditEngine->SetAttrib( TextAttribFontColor( rColor ), nLine, r.nBegin, r.nEnd );
    }

    // Das Highlighten soll kein Modify setzen
    pEditEngine->SetModified( bWasModified );
}

void EditorWindow::ImplSetFont()
{
    if ( pSourceViewConfig )
    {
        String sFontName = pSourceViewConfig->GetFontName();
        if ( !sFontName.Len() )
        {
            Font aTmpFont( OutputDevice::GetDefaultFont( DEFAULTFONT_FIXED, Application::GetSettings().GetUILanguage(), 0 , this ) );
            sFontName = aTmpFont.GetName();
        }
        Size aFontSize( 0, pSourceViewConfig->GetFontHeight() );
        Font aFont( sFontName, aFontSize );
        aFont.SetColor( GetSettings().GetStyleSettings().GetFieldTextColor() );
        SetPointFont( aFont );
        aFont = GetFont();

        if ( pModulWindow )
            pModulWindow->GetBreakPointWindow().SetFont( aFont );

        if ( pEditEngine )
        {
            BOOL bModified = pEditEngine->IsModified();
            pEditEngine->SetFont( aFont );
            pEditEngine->SetModified( bModified );
        }
    }
}

void EditorWindow::DoSyntaxHighlight( ULONG nPara )
{
    // Durch das DelayedSyntaxHighlight kann es passieren,
    // dass die Zeile nicht mehr existiert!
    if ( nPara < pEditEngine->GetParagraphCount() )
    {
        // leider weis ich nicht, ob genau diese Zeile Modified() ...
        if ( pProgress )
            pProgress->StepProgress();
        pEditEngine->RemoveAttribs( nPara );
        ImpDoHighlight( nPara );
    }
}

void EditorWindow::DoDelayedSyntaxHighlight( ULONG nPara )
{
    // Zeile wird nur in 'Liste' aufgenommen, im TimerHdl abgearbeitet.
    // => Nicht Absaetze manipulieren, waehrend EditEngine formatiert.
    if ( pProgress )
        pProgress->StepProgress();

    if ( !bHighlightning && bDoSyntaxHighlight )
    {
        if ( bDelayHighlight )
        {
            aSyntaxLineTable.Insert( nPara, (void*)(ULONG)1 );
            aSyntaxIdleTimer.Start();
        }
        else
            DoSyntaxHighlight( nPara );
    }
}

IMPL_LINK( EditorWindow, HelpAgentTimerHdl, Timer *, EMPTYARG )
{
    //SfxHelpPI* pHelpAgent = SFX_APP()->GetHelpPI();
    //if ( pHelpAgent)
    //{
        //pHelpAgent->LoadTopic( GetWordAtCursor() );
    //}
    return 0;
}

IMPL_LINK( EditorWindow, SyntaxTimerHdl, Timer *, EMPTYARG )
{
    DBG_ASSERT( pEditView, "Noch keine View, aber Syntax-Highlight ?!" );

    BOOL bWasModified = pEditEngine->IsModified();
    pEditEngine->SetUpdateMode( FALSE );

    bHighlightning = TRUE;
    USHORT nLine;
    void* p = aSyntaxLineTable.First();
    while ( p )
    {
        nLine = (USHORT)aSyntaxLineTable.GetCurKey();
        DoSyntaxHighlight( nLine );
        p = aSyntaxLineTable.Next();
    }

    pEditView->SetAutoScroll( FALSE );  // #101043# Don't scroll because of syntax highlight
    pEditEngine->SetUpdateMode( TRUE );
    pEditView->ShowCursor( FALSE, TRUE );
    pEditView->SetAutoScroll( TRUE );

    pEditEngine->SetModified( bWasModified );

    aSyntaxLineTable.Clear();
    // SyntaxTimerHdl wird gerufen, wenn Text-Aenderung
    // => gute Gelegenheit, Textbreite zu ermitteln!
    long nPrevTextWidth = nCurTextWidth;
    nCurTextWidth = pEditEngine->CalcTextWidth();
    if ( nCurTextWidth != nPrevTextWidth )
        SetScrollBarRanges();
    bHighlightning = FALSE;


    return 0;
}

void EditorWindow::ParagraphInsertedDeleted( ULONG nPara, BOOL bInserted )
{
    if ( pProgress )
        pProgress->StepProgress();

    if ( !bInserted && ( nPara == TEXT_PARA_ALL ) )
    {
        pModulWindow->GetBreakPoints().reset();
        pModulWindow->GetBreakPointWindow().Invalidate();
        aHighlighter.initialize( HIGHLIGHT_BASIC );
    }
    else
    {
        // Brechpunkte Aktualisieren...
        // keine Sonderbehandlung fuer EditEngine-CTOR ( Erste-Zeile-Problem ),
        // da in diesem Moment noch keine BreakPoints.
        // +1: Basic-Zeilen beginnen bei 1!
        pModulWindow->GetBreakPoints().AdjustBreakPoints( (USHORT)nPara+1, bInserted );

        // Im BreakPointWindow invalidieren...
        long nLineHeight = GetTextHeight();
        Size aSz = pModulWindow->GetBreakPointWindow().GetOutputSize();
        Rectangle aInvRec( Point( 0, 0 ), aSz );
        long nY = nPara*nLineHeight - pModulWindow->GetBreakPointWindow().GetCurYOffset();
        aInvRec.Top() = nY;
        pModulWindow->GetBreakPointWindow().Invalidate( aInvRec );

        String aDummy;
        aHighlighter.notifyChange( nPara, bInserted ? 1 : (-1), &aDummy, 1 );
    }
}

void EditorWindow::ToggleHighlightMode()
{
    bDoSyntaxHighlight = !bDoSyntaxHighlight;
    if ( !pEditEngine )
        return;


    if ( bDoSyntaxHighlight )
    {
        for ( ULONG i = 0; i < pEditEngine->GetParagraphCount(); i++ )
            DoDelayedSyntaxHighlight( i );
    }
    else
    {
        aSyntaxIdleTimer.Stop();
        pEditEngine->SetUpdateMode( FALSE );
        for ( ULONG i = 0; i < pEditEngine->GetParagraphCount(); i++ )
            pEditEngine->RemoveAttribs( i );

//      pEditEngine->QuickFormatDoc();
        pEditEngine->SetUpdateMode( TRUE );
        pEditView->ShowCursor(TRUE, TRUE );
    }
}


void EditorWindow::CreateProgress( const String& rText, ULONG nRange )
{
    DBG_ASSERT( !pProgress, "ProgressInfo existiert schon" );
    pProgress = new ProgressInfo( IDE_DLL()->GetShell()->GetViewFrame()->GetObjectShell(), rText, nRange );
}

void EditorWindow::DestroyProgress()
{
    DELETEZ( pProgress );
}

void EditorWindow::ForceSyntaxTimeout()
{
    aSyntaxIdleTimer.Stop();
    ((Link&)aSyntaxIdleTimer.GetTimeoutHdl()).Call( &aSyntaxIdleTimer );
}



BreakPointWindow::BreakPointWindow( Window* pParent ) :
    Window( pParent, WB_BORDER )
{
    pModulWindow = 0;
    nCurYOffset = 0;
    setBackgroundColor(GetSettings().GetStyleSettings().GetFieldColor());
    nMarkerPos = MARKER_NOMARKER;

    // nCurYOffset merken und nicht von EditEngine holen.
    // Falls in EditEngine autom. gescrollt wurde, wuesste ich sonst nicht,
    // wo ich gerade stehe.

    SetHelpId( HID_BASICIDE_BREAKPOINTWINDOW );
}



__EXPORT BreakPointWindow::~BreakPointWindow()
{
}



void __EXPORT BreakPointWindow::Resize()
{
/// Invalidate();
}



void __EXPORT BreakPointWindow::Paint( const Rectangle& )
{
    if ( SyncYOffset() )
        return;

    Size aOutSz( GetOutputSize() );
    long nLineHeight = GetTextHeight();

    Image aBrk1(((ModulWindowLayout *) pModulWindow->GetLayoutWindow())->
                getImage(IMGID_BRKENABLED, m_bHighContrastMode));
    Image aBrk0(((ModulWindowLayout *) pModulWindow->GetLayoutWindow())->
                getImage(IMGID_BRKDISABLED, m_bHighContrastMode));
    Size aBmpSz( aBrk1.GetSizePixel() );
    aBmpSz = PixelToLogic( aBmpSz );
    Point aBmpOff( 0, 0 );
    aBmpOff.X() = ( aOutSz.Width() - aBmpSz.Width() ) / 2;
    aBmpOff.Y() = ( nLineHeight - aBmpSz.Height() ) / 2;

    BreakPoint* pBrk = GetBreakPoints().First();
    while ( pBrk )
    {
        ULONG nLine = pBrk->nLine-1;
        ULONG nY = nLine*nLineHeight - nCurYOffset;
        DrawImage( Point( 0, nY ) + aBmpOff, pBrk->bEnabled ? aBrk1 : aBrk0 );
        pBrk = GetBreakPoints().Next();
    }
    ShowMarker( TRUE );
}



void BreakPointWindow::Scroll( long nHorzScroll, long nVertScroll )
{
    nCurYOffset -= nVertScroll;
    Window::Scroll( nHorzScroll, nVertScroll );
}



void BreakPointWindow::SetMarkerPos( USHORT nLine, BOOL bError )
{
    if ( SyncYOffset() )
        Update();

    ShowMarker( FALSE );    // Alten wegzeichen...
    nMarkerPos = nLine;
    bErrorMarker = bError;
    ShowMarker( TRUE );     // Neuen zeichnen...
}

void BreakPointWindow::ShowMarker( BOOL bShow )
{
    if ( nMarkerPos == MARKER_NOMARKER )
        return;

    Size aOutSz( GetOutputSize() );
    long nLineHeight = GetTextHeight();

    Image aMarker(((ModulWindowLayout*)pModulWindow->GetLayoutWindow())->
                  getImage(bErrorMarker
                           ? IMGID_ERRORMARKER : IMGID_STEPMARKER,
                           m_bHighContrastMode));

    Size aMarkerSz( aMarker.GetSizePixel() );
    aMarkerSz = PixelToLogic( aMarkerSz );
    Point aMarkerOff( 0, 0 );
    aMarkerOff.X() = ( aOutSz.Width() - aMarkerSz.Width() ) / 2;
    aMarkerOff.Y() = ( nLineHeight - aMarkerSz.Height() ) / 2;

    ULONG nY = nMarkerPos*nLineHeight - nCurYOffset;
    Point aPos( 0, nY );
    aPos += aMarkerOff;
    if ( bShow )
        DrawImage( aPos, aMarker );
    else
        Invalidate( Rectangle( aPos, aMarkerSz ) );
}




BreakPoint* BreakPointWindow::FindBreakPoint( const Point& rMousePos )
{
    long nLineHeight = GetTextHeight();
    long nYPos = rMousePos.Y() + nCurYOffset;
//  Image aBrk( ((ModulWindowLayout*)pModulWindow->GetLayoutWindow())->GetImage( IMGID_BRKENABLED ) );
//  Size aBmpSz( aBrk.GetSizePixel() );
//  aBmpSz = PixelToLogic( aBmpSz );

    BreakPoint* pBrk = GetBreakPoints().First();
    while ( pBrk )
    {
        ULONG nLine = pBrk->nLine-1;
        long nY = nLine*nLineHeight;
        if ( ( nYPos > nY ) && ( nYPos < ( nY + nLineHeight ) ) )
            return pBrk;
        pBrk = GetBreakPoints().Next();
    }
    return 0;
}

void __EXPORT BreakPointWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.GetClicks() == 2 )
    {
        Point aMousePos( PixelToLogic( rMEvt.GetPosPixel() ) );
        long nLineHeight = GetTextHeight();
        long nYPos = aMousePos.Y() + nCurYOffset;
        long nLine = nYPos / nLineHeight + 1;
        pModulWindow->ToggleBreakPoint( (ULONG)nLine );
        // vielleicht mal etwas genauer...
        Invalidate();
    }
}



void __EXPORT BreakPointWindow::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() == COMMAND_CONTEXTMENU )
    {
        Point aPos( rCEvt.IsMouseEvent() ? rCEvt.GetMousePosPixel() : Point(1,1) );
        Point aEventPos( PixelToLogic( aPos ) );
        BreakPoint* pBrk = rCEvt.IsMouseEvent() ? FindBreakPoint( aEventPos ) : 0;
        if ( pBrk )
        {
            // prueffen, ob Brechpunkt enabled....
            PopupMenu aBrkPropMenu( IDEResId( RID_POPUP_BRKPROPS ) );
            aBrkPropMenu.CheckItem( RID_ACTIV, pBrk->bEnabled );
            switch ( aBrkPropMenu.Execute( this, aPos ) )
            {
                case RID_ACTIV:
                {
                    pBrk->bEnabled = pBrk->bEnabled ? FALSE : TRUE;
                    pModulWindow->UpdateBreakPoint( *pBrk );
                    Invalidate();
                }
                break;
                case RID_BRKPROPS:
                {
                    BreakPointDialog aBrkDlg( this, GetBreakPoints() );
                    aBrkDlg.SetCurrentBreakPoint( pBrk );
                    aBrkDlg.Execute();
                    Invalidate();
                }
                break;
            }
        }
        else
        {
            PopupMenu aBrkListMenu( IDEResId( RID_POPUP_BRKDLG ) );
            switch ( aBrkListMenu.Execute( this, aPos ) )
            {
                case RID_BRKDLG:
                {
                    BreakPointDialog aBrkDlg( this, GetBreakPoints() );
                    aBrkDlg.Execute();
                    Invalidate();
                }
                break;
            }
        }
    }
}

BOOL BreakPointWindow::SyncYOffset()
{
    TextView* pView = pModulWindow->GetEditView();
    if ( pView )
    {
        long nViewYOffset = pView->GetStartDocPos().Y();
        if ( nCurYOffset != nViewYOffset )
        {
            nCurYOffset = nViewYOffset;
            Invalidate();
            return TRUE;
        }
    }
    return FALSE;
}

// virtual
void BreakPointWindow::DataChanged(DataChangedEvent const & rDCEvt)
{
    Window::DataChanged(rDCEvt);
    if (rDCEvt.GetType() == DATACHANGED_SETTINGS
        && (rDCEvt.GetFlags() & SETTINGS_STYLE) != 0)
    {
        Color aColor(GetSettings().GetStyleSettings().GetFieldColor());
        if (aColor
            != rDCEvt.GetOldSettings()->GetStyleSettings().GetFieldColor())
        {
            setBackgroundColor(aColor);
            Invalidate();
        }
    }
}

void BreakPointWindow::setBackgroundColor(Color aColor)
{
    SetBackground(Wallpaper(aColor));
    m_bHighContrastMode = aColor.IsDark();
}

WatchWindow::WatchWindow( Window* pParent ) :
    BasicDockingWindow( pParent ),
    aTreeListBox( this, WB_BORDER | WB_3DLOOK | WB_HASBUTTONS | WB_HASLINES | WB_HSCROLL | WB_TABSTOP ),
    aXEdit( this, IDEResId( RID_EDT_WATCHEDIT ) ),
    aWatchStr( IDEResId( RID_STR_REMOVEWATCH ) ),
    aRemoveWatchButton( this, IDEResId( RID_IMGBTN_REMOVEWATCH ) )
{
    nVirtToolBoxHeight = aXEdit.GetSizePixel().Height() + 7;

    aTreeListBox.SetHelpId(HID_BASICIDE_WATCHWINDOW_LIST);
    aTreeListBox.EnableInplaceEditing( TRUE );
    aTreeListBox.SetSelectHdl( LINK( this, WatchWindow, TreeListHdl ) );
    aTreeListBox.SetPosPixel( Point( DWBORDER, nVirtToolBoxHeight ) );
    aTreeListBox.SetHighlightRange();

    aRemoveWatchButton.Disable();

    aTreeListBox.Show();

    long nTextLen = GetTextWidth( aWatchStr ) + DWBORDER;
    aXEdit.SetPosPixel( Point( nTextLen, 3 ) );
    aXEdit.SetAccHdl( LINK( this, WatchWindow, EditAccHdl ) );
    aXEdit.GetAccelerator().InsertItem( 1, KeyCode( KEY_RETURN ) );
    aXEdit.GetAccelerator().InsertItem( 2, KeyCode( KEY_ESCAPE ) );
    aXEdit.Show();

    aRemoveWatchButton.SetModeImage(Image(IDEResId(RID_IMG_REMOVEWATCH_HC)),
                                    BMP_COLOR_HIGHCONTRAST);
    aRemoveWatchButton.SetClickHdl( LINK( this, WatchWindow, ButtonHdl ) );
    aRemoveWatchButton.SetPosPixel( Point( nTextLen + aXEdit.GetSizePixel().Width() + 4, 2 ) );
    Size aSz( aRemoveWatchButton.GetImage().GetSizePixel() );
    aSz.Width() += 6;
    aSz.Height() += 6;
    aRemoveWatchButton.SetSizePixel( aSz );
    aRemoveWatchButton.Show();

    SetText( String( IDEResId( RID_STR_WATCHNAME ) ) );

    SetHelpId( HID_BASICIDE_WATCHWINDOW );

    // make watch window keyboard accessible
    GetSystemWindow()->GetTaskPaneList()->AddWindow( this );
}



__EXPORT WatchWindow::~WatchWindow()
{
    GetSystemWindow()->GetTaskPaneList()->RemoveWindow( this );
}



void __EXPORT WatchWindow::Paint( const Rectangle& )
{
    DrawText( Point( DWBORDER, 7 ), aWatchStr );
    lcl_DrawIDEWindowFrame( this );
}



void __EXPORT WatchWindow::Resize()
{
    Size aSz = GetOutputSizePixel();
    Size aBoxSz( aSz.Width() - 2*DWBORDER, aSz.Height() - nVirtToolBoxHeight - DWBORDER );

    if ( aBoxSz.Width() < 4 )   // < 4, weil noch Border...
        aBoxSz.Width() = 0;
    if ( aBoxSz.Height() < 4 )
        aBoxSz.Height() = 0;

    aTreeListBox.SetSizePixel( aBoxSz );
    aTreeListBox.GetHScroll()->SetPageSize( aTreeListBox.GetHScroll()->GetVisibleSize() );

    Invalidate();   //Wegen DrawLine im Paint...
}



void WatchWindow::AddWatch( const String& rVName )
{
    // TRUE/FALSE: ChildsOnDemand => Fuer properties
    SvLBoxEntry* pNewEntry = aTreeListBox.InsertEntry( rVName, 0, FALSE, LIST_APPEND );
    // Den Variablen-Namen ranhaengen, da der Text des Entries
    // immer um den Wert der Wariablen ergaenzt wird.
    pNewEntry->SetUserData( new String( rVName ) );
    aTreeListBox.Select( pNewEntry, TRUE );
    aTreeListBox.MakeVisible( pNewEntry );
    aRemoveWatchButton.Enable();
}



BOOL WatchWindow::RemoveSelectedWatch()
{
    SvLBoxEntry* pEntry = aTreeListBox.GetCurEntry();
    if ( pEntry )
    {
        aTreeListBox.GetModel()->Remove( pEntry );
        pEntry = aTreeListBox.GetCurEntry();
        if ( pEntry )
            aXEdit.SetText( *((String*)pEntry->GetUserData()) );
        else
            aXEdit.SetText( String() );
        if ( !aTreeListBox.GetEntryCount() )
            aRemoveWatchButton.Disable();
        return TRUE;
    }
    else
        return FALSE;
}


IMPL_LINK_INLINE_START( WatchWindow, ButtonHdl, ImageButton *, pButton )
{
    if ( pButton == &aRemoveWatchButton )
    {
        BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
        SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
        SfxDispatcher* pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;
        if( pDispatcher )
        {
            pDispatcher->Execute( SID_BASICIDE_REMOVEWATCH );
        }
    }
    return 0;
}
IMPL_LINK_INLINE_END( WatchWindow, ButtonHdl, ImageButton *, pButton )



IMPL_LINK_INLINE_START( WatchWindow, TreeListHdl, SvTreeListBox *, EMPTYARG )
{
    SvLBoxEntry* pCurEntry = aTreeListBox.GetCurEntry();
    if ( pCurEntry && pCurEntry->GetUserData() )
        aXEdit.SetText( *((String*)pCurEntry->GetUserData()) );

    return 0;
}
IMPL_LINK_INLINE_END( WatchWindow, TreeListHdl, SvTreeListBox *, EMPTYARG )



IMPL_LINK( WatchWindow, EditAccHdl, Accelerator *, pAcc )
{
    switch ( pAcc->GetCurKeyCode().GetCode() )
    {
        case KEY_RETURN:
        {
            String aCurText( aXEdit.GetText() );
            if ( aCurText.Len() )
            {
                AddWatch( aCurText );
                aXEdit.SetSelection( Selection( 0, 0xFFFF ) );
                UpdateWatches();
            }
            else
                Sound::Beep();
        }
        break;
        case KEY_ESCAPE:
        {
            aXEdit.SetText( String() );
        }
        break;
    }

    return 0;
}

void WatchWindow::UpdateWatches()
{
    aTreeListBox.UpdateWatches();
}


StackWindow::StackWindow( Window* pParent ) :
    BasicDockingWindow( pParent ),
    aGotoCallButton( this, IDEResId( RID_IMGBTN_GOTOCALL ) ),
    aTreeListBox( this, WB_BORDER | WB_3DLOOK | WB_HSCROLL | WB_TABSTOP ),
    aStackStr( IDEResId( RID_STR_STACK ) )
{
    aTreeListBox.SetHelpId(HID_BASICIDE_STACKWINDOW_LIST);
    aTreeListBox.SetPosPixel( Point( DWBORDER, nVirtToolBoxHeight ) );
    aTreeListBox.SetHighlightRange();
    aTreeListBox.SetSelectionMode( NO_SELECTION );
    aTreeListBox.InsertEntry( String(), 0, FALSE, LIST_APPEND );
    aTreeListBox.Show();

    SetText( String( IDEResId( RID_STR_STACKNAME ) ) );

    SetHelpId( HID_BASICIDE_STACKWINDOW );

    aGotoCallButton.SetClickHdl( LINK( this, StackWindow, ButtonHdl ) );
    aGotoCallButton.SetPosPixel( Point( DWBORDER, 2 ) );
    Size aSz( aGotoCallButton.GetImage().GetSizePixel() );
    aSz.Width() += 6;
    aSz.Height() += 6;
    aGotoCallButton.SetSizePixel( aSz );
//  aGotoCallButton.Show(); // wird vom Basic noch nicht unterstuetzt!
    aGotoCallButton.Hide();

    // make stack window keyboard accessible
    GetSystemWindow()->GetTaskPaneList()->AddWindow( this );
}



__EXPORT StackWindow::~StackWindow()
{
    GetSystemWindow()->GetTaskPaneList()->RemoveWindow( this );
}



void __EXPORT StackWindow::Paint( const Rectangle& )
{
    DrawText( Point( DWBORDER, 7 ), aStackStr );
    lcl_DrawIDEWindowFrame( this );
}



void __EXPORT StackWindow::Resize()
{
    Size aSz = GetOutputSizePixel();
    Size aBoxSz( aSz.Width() - 2*DWBORDER, aSz.Height() - nVirtToolBoxHeight - DWBORDER );

    if ( aBoxSz.Width() < 4 )   // < 4, weil noch Border...
        aBoxSz.Width() = 0;
    if ( aBoxSz.Height() < 4 )
        aBoxSz.Height() = 0;

    aTreeListBox.SetSizePixel( aBoxSz );

    Invalidate();   //Wegen DrawLine im Paint...
}



IMPL_LINK_INLINE_START( StackWindow, ButtonHdl, ImageButton *, pButton )
{
    if ( pButton == &aGotoCallButton )
    {
        BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
        SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
        SfxDispatcher* pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;
        if( pDispatcher )
        {
            pDispatcher->Execute( SID_BASICIDE_GOTOCALL );
        }
    }
    return 0;
}
IMPL_LINK_INLINE_END( StackWindow, ButtonHdl, ImageButton *, pButton )



void __EXPORT StackWindow::UpdateCalls()
{
    aTreeListBox.SetUpdateMode( FALSE );
    aTreeListBox.Clear();

    if ( StarBASIC::IsRunning() )
    {
        SbxError eOld = SbxBase::GetError();
        aTreeListBox.SetSelectionMode( SINGLE_SELECTION );

        USHORT nScope = 0;
        SbMethod* pMethod = StarBASIC::GetActiveMethod( nScope );
        while ( pMethod )
        {
            String aEntry( String::CreateFromInt32(nScope ));
            if ( aEntry.Len() < 2 )
                aEntry.Insert( ' ', 0 );
            aEntry += String( RTL_CONSTASCII_USTRINGPARAM( ": " ) );
            aEntry += pMethod->GetName();
            SbxArray* pParams = pMethod->GetParameters();
            SbxInfo* pInfo = pMethod->GetInfo();
            if ( pParams )
            {
                aEntry += '(';
                // 0 ist der Name der Sub...
                for ( USHORT nParam = 1; nParam < pParams->Count(); nParam++ )
                {
                    SbxVariable* pVar = pParams->Get( nParam );
                    DBG_ASSERT( pVar, "Parameter?!" );
                    if ( pVar->GetName().Len() )
                        aEntry += pVar->GetName();
                    else if ( pInfo )
                    {
                        const SbxParamInfo* pParam = pInfo->GetParam( nParam );
                        if ( pParam )
                            aEntry += pParam->aName;
                    }
                    aEntry += '=';
                    if( pVar->GetType() & SbxARRAY )
                        aEntry += String( RTL_CONSTASCII_USTRINGPARAM( "..." ) );
                    else
                        aEntry += pVar->GetString();
                    if ( nParam < ( pParams->Count() - 1 ) )
                        aEntry += String( RTL_CONSTASCII_USTRINGPARAM( ", " ) );
                }
                aEntry += ')';
            }
            aTreeListBox.InsertEntry( aEntry, 0, FALSE, LIST_APPEND );
            nScope++;
            pMethod = StarBASIC::GetActiveMethod( nScope );
        }

        SbxBase::ResetError();
        if( eOld != SbxERR_OK )
            SbxBase::SetError( eOld );
    }
    else
    {
        aTreeListBox.SetSelectionMode( NO_SELECTION );
        aTreeListBox.InsertEntry( String(), 0, FALSE, LIST_APPEND );
    }

    aTreeListBox.SetUpdateMode( TRUE );
}




ComplexEditorWindow::ComplexEditorWindow( ModulWindow* pParent ) :
    Window( pParent, WB_3DLOOK | WB_CLIPCHILDREN ),
    aEWVScrollBar( this, WB_VSCROLL | WB_DRAG ),
    aBrkWindow( this ),
    aEdtWindow( this )
{
    aEdtWindow.SetModulWindow( pParent );
    aBrkWindow.SetModulWindow( pParent );
    aEdtWindow.Show();
    aBrkWindow.Show();

    aEWVScrollBar.SetLineSize( SCROLL_LINE );
    aEWVScrollBar.SetPageSize( SCROLL_PAGE );
    aEWVScrollBar.SetScrollHdl( LINK( this, ComplexEditorWindow, ScrollHdl ) );
    aEWVScrollBar.Show();
}



void __EXPORT ComplexEditorWindow::Resize()
{
    Size aOutSz = GetOutputSizePixel();
    Size aSz( aOutSz );
    aSz.Width() -= 2*DWBORDER;
    aSz.Height() -= 2*DWBORDER;
    long nBrkWidth = 20;
    long nSBWidth = aEWVScrollBar.GetSizePixel().Width();

    Size aBrkSz( Size( nBrkWidth, aSz.Height() ) );
    aBrkWindow.SetPosSizePixel( Point( DWBORDER, DWBORDER ), aBrkSz );

    Size aEWSz( Size( aSz.Width() - nBrkWidth - nSBWidth + 2, aSz.Height() ) );
    aEdtWindow.SetPosSizePixel( Point( DWBORDER+aBrkSz.Width()-1, DWBORDER ), aEWSz );

    aEWVScrollBar.SetPosSizePixel( Point( aOutSz.Width()-DWBORDER-nSBWidth, DWBORDER ), Size( nSBWidth, aSz.Height() ) );

    // Macht das EditorWindow, ausserdem hier falsch, da Pixel
//  aEWVScrollBar.SetPageSize( aEWSz.Height() * 8 / 10 );
//  aEWVScrollBar.SetVisibleSize( aSz.Height() );
//  Invalidate();
}

IMPL_LINK( ComplexEditorWindow, ScrollHdl, ScrollBar *, pCurScrollBar )
{
    if ( aEdtWindow.GetEditView() )
    {
        DBG_ASSERT( pCurScrollBar == &aEWVScrollBar, "Wer scrollt hier ?" );
        long nDiff = aEdtWindow.GetEditView()->GetStartDocPos().Y() - pCurScrollBar->GetThumbPos();
        aEdtWindow.GetEditView()->Scroll( 0, nDiff );
        aBrkWindow.Scroll( 0, nDiff );
        aEdtWindow.GetEditView()->ShowCursor( FALSE, TRUE );
        pCurScrollBar->SetThumbPos( aEdtWindow.GetEditView()->GetStartDocPos().Y() );
    }

    return 0;
}

// virtual
void ComplexEditorWindow::DataChanged(DataChangedEvent const & rDCEvt)
{
    Window::DataChanged(rDCEvt);
    if (rDCEvt.GetType() == DATACHANGED_SETTINGS
        && (rDCEvt.GetFlags() & SETTINGS_STYLE) != 0)
    {
        Color aColor(GetSettings().GetStyleSettings().GetFaceColor());
        if (aColor
            != rDCEvt.GetOldSettings()->GetStyleSettings().GetFaceColor())
        {
            SetBackground(Wallpaper(aColor));
            Invalidate();
        }
    }
}

// virtual
uno::Reference< awt::XWindowPeer >
EditorWindow::GetComponentInterface(BOOL bCreate)
{
    uno::Reference< awt::XWindowPeer > xPeer(
        Window::GetComponentInterface(false));
    if (!xPeer.is() && bCreate)
    {
        // Make sure edit engine and view are available:
        if (!pEditEngine)
            CreateEditEngine();

        xPeer = new svtools::TextWindowAccessibility(*GetEditView());
        SetComponentInterface(xPeer);
    }
    return xPeer;
}

WatchTreeListBox::WatchTreeListBox( Window* pParent, WinBits nWinBits )
    : SvTreeListBox( pParent, nWinBits )
{
}

WatchTreeListBox::~WatchTreeListBox()
{
    // User-Daten zerstoeren...
    SvLBoxEntry* pEntry = First();
    while ( pEntry )
    {
        delete (String*)pEntry->GetUserData();
        pEntry = Next( pEntry );
    }
}

BOOL __EXPORT WatchTreeListBox::EditingEntry( SvLBoxEntry* pEntry, Selection& rSel  )
{
    BOOL bEdit = FALSE;
    if ( StarBASIC::IsRunning() && StarBASIC::GetActiveMethod() && !SbxBase::IsError() )
    {
        String aEntryText( GetEntryText( pEntry ) );
        USHORT nPos = aEntryText.Search( '=' );
        if ( nPos != STRING_NOTFOUND )
            aEditingRes = aEntryText.Copy( nPos+1);
        else
            aEditingRes.Erase();
        aEditingRes.EraseLeadingChars();
        aEditingRes.EraseTrailingChars();

        bEdit = TRUE;
    }

    if ( !bEdit )
        Sound::Beep();

    return bEdit;
}

BOOL __EXPORT WatchTreeListBox::EditedEntry( SvLBoxEntry* pEntry, const String& rNewText )
{
    USHORT nPos = rNewText.Search( '=' );
    String aVName, aResult;
    aVName = rNewText.Copy( 0, nPos );
    if ( nPos != STRING_NOTFOUND )
        aResult = rNewText.Copy( nPos+1);
    aVName.EraseLeadingChars();
    aVName.EraseTrailingChars();
    aResult.EraseLeadingChars();
    aResult.EraseTrailingChars();

    BOOL bVarModified = ( aVName != *((String*)pEntry->GetUserData()) ) ? TRUE : FALSE;
    BOOL bResModified = ( aResult != aEditingRes ) ? TRUE : FALSE;

    BOOL bError = FALSE;

    if ( !aVName.Len() )
        bError = TRUE;

    if ( bVarModified && !bError )
    {
        delete (String*)pEntry->GetUserData();
        pEntry->SetUserData( new String( aVName ) );
    }

    if ( aVName.Len() && strchr( cSuffixes, aVName.GetChar( aVName.Len() - 1 ) ) )
    {
        aVName.Erase( aVName.Len()-1, 1 );
        if ( !aVName.Len() )
            bError = TRUE;
    }

    BOOL bRet = FALSE;

    if ( bError )
        Sound::Beep();
    else if ( bResModified )
    {
        bRet = ImplBasicEntryEdited( pEntry, aVName, aResult );
    }

    return bRet;
}

BOOL WatchTreeListBox::ImplBasicEntryEdited( SvLBoxEntry* pEntry, const String& rVName, const String& rResult )
{
    BOOL bError = FALSE;
    String aResult( rResult );
    String aVar, aIndex;
    lcl_SeparateNameAndIndex( rVName, aVar, aIndex );
    SbxBase* pToBeChanged  = 0;
    SbxBase* pSBX = StarBASIC::FindSBXInCurrentScope( aVar );
    if ( pSBX && pSBX->ISA( SbxVariable ) && !pSBX->ISA( SbxMethod ) )
    {
        SbxVariable* pVar = (SbxVariable*)pSBX;
        SbxDataType eType = pVar->GetType();
        if ( (BYTE)eType == (BYTE)SbxOBJECT )
            bError = TRUE;
        else if ( eType & SbxARRAY )
        {
            SbxBase* pBase = pVar->GetObject();
            if ( pBase && pBase->ISA( SbxDimArray ) )
            {
                SbxDimArray* pArray = (SbxDimArray*)pBase;
                if ( !aIndex.Len() )
                {
                    // Das Array auf das Format XX;XX;XX formatieren lassen,
                    // wenn TokenCount == Array-Groesse, Werte reinplaetten.
                    lcl_FormatArrayString( aResult );
                    USHORT nVars = pArray->Count();
                    if ( aResult.GetTokenCount() == (nVars+1) )
                    {
                        for ( USHORT nVar = 0; nVar < nVars; nVar++,bError )
                        {
                            SbxBase* pElem = pArray->SbxArray::Get( nVar );
                            if ( pElem && pElem->ISA( SbxVariable ) )
                            {
                                String aVar = aResult.GetToken( nVar, ';' );
                                // Falls die Variablen als String geputtet werden
                                // addieren sich sonst die Zwischenraeume...
                                aVar.EraseLeadingChars();
                                aVar.EraseTrailingChars();
                                ((SbxVariable*)pElem)->PutStringExt( aVar );
                            }
                            else
                                bError = TRUE;
                        }
                    }
                    else
                        bError = TRUE;
                }
                else
                {
                    BOOL bValidIndex;
                    pToBeChanged = lcl_FindElement( pArray, aIndex, bValidIndex );
                    if ( pToBeChanged )
                        lcl_FormatArrayString( aResult );
                    else
                        bError = TRUE;
                }
            }
            else
                bError = TRUE;
        }
        else
            pToBeChanged = pSBX;
    }

    if ( pToBeChanged )
    {
        if ( pToBeChanged->ISA( SbxVariable ) )
        {
            // Wenn der Typ variabel ist, macht die Konvertierung des SBX nichts,
            // bei festem Typ wird der String konvertiert.
            ((SbxVariable*)pToBeChanged)->PutStringExt( aResult );
        }
        else
            bError = TRUE;
    }

    // Wenn jemand z.B. einen zu grossen Wert fuer ein Int eingegeben hat,
    // folgt beim naechsten Step() ein Runtime-Error.
    if ( SbxBase::IsError() )
    {
        bError = TRUE;
        SbxBase::ResetError();
    }

    if ( bError )
        Sound::Beep();

    UpdateWatches();

    // Der Text soll niemals 1-zu-1 uebernommen werden, weil dann das
    // UpdateWatches verlorengeht.
    return FALSE;
}


void WatchTreeListBox::UpdateWatches()
{
    SbMethod* pCurMethod = StarBASIC::GetActiveMethod();
    SbModule* pModule = pCurMethod ? pCurMethod->GetModule() : 0;

    SbxError eOld = SbxBase::GetError();

    SvLBoxEntry* pEntry = First();
    while ( pEntry )
    {
        String aVName( *( (String*)pEntry->GetUserData() ) );
        DBG_ASSERT( aVName.Len(), "Var? - Darf nicht leer sein!" );
        String aWatchStr( aVName );
        aWatchStr += String( RTL_CONSTASCII_USTRINGPARAM( " = " ) );
        if ( pCurMethod )
        {
            String aVar, aIndex;
            lcl_SeparateNameAndIndex( aVName, aVar, aIndex );
            SbxBase* pSBX = StarBASIC::FindSBXInCurrentScope( aVar );
            if ( pSBX && pSBX->ISA( SbxVariable ) && !pSBX->ISA( SbxMethod ) )
            {
                SbxVariable* pVar = (SbxVariable*)pSBX;
                // Sonderbehandlung fuer Arrays:
                SbxDataType eType = pVar->GetType();
                if ( (BYTE)eType == (BYTE)SbxOBJECT )
                    aWatchStr += String( RTL_CONSTASCII_USTRINGPARAM( "<?>" ) );
                else if ( eType & SbxARRAY )
                {
                    // Mehrdimensionale Arrays beruecksichtigen!
                    SbxBase* pBase = pVar->GetObject();
                    if ( pBase && pBase->ISA( SbxDimArray ) )
                    {
                        SbxDimArray* pArray = (SbxDimArray*)pBase;
                        aWatchStr += '{';
                        USHORT nDims = pArray->GetDims();
                        if ( !aIndex.Len() )
                        {
                            short* pIdx = new short[ nDims ];
                            for ( USHORT n = nDims; n; )
                                pIdx[--n] = 0;
                            // hoechste Dimension festhalen.
//                          lcl_GetValues( aWatchStr, pArray, pIdx, nDims );
                            lcl_GetValues( aWatchStr, pArray, pIdx, 1 );
                            delete pIdx;
                        }
                        else
                        {
                            BOOL bValidIndex;
                            SbxBase* pElem = lcl_FindElement( pArray, aIndex, bValidIndex );
                            if ( pElem )
                                aWatchStr += ((SbxVariable*)pElem)->GetString();
                            else if ( !bValidIndex )
                                aWatchStr += String( RTL_CONSTASCII_USTRINGPARAM( "<Invalid index>" ) );
                        }
                        aWatchStr += '}';
                    }
                    else
                        aWatchStr += String( RTL_CONSTASCII_USTRINGPARAM( "<?>" ) );
                }
                else
                    aWatchStr += pVar->GetString();
            }
            else
                aWatchStr += String( RTL_CONSTASCII_USTRINGPARAM( "<Out of Scope>" ) );
        }
        SetEntryText( pEntry, aWatchStr );

        pEntry = Next( pEntry );
    }

    SbxBase::ResetError();
    if( eOld != SbxERR_OK )
        SbxBase::SetError( eOld );

}

