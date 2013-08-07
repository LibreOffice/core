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

#include "helpid.hrc"
#include "baside2.hrc"

#include "baside2.hxx"
#include "brkdlg.hxx"
#include "iderdll.hxx"
#include "iderdll2.hxx"
#include "objdlg.hxx"

#include <basic/sbmeth.hxx>
#include <basic/sbuno.hxx>
#include <com/sun/star/script/XLibraryContainer2.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XPropertiesChangeListener.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <officecfg/Office/Common.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/msgbox.hxx>
#include <svl/urihelper.hxx>
#include <vcl/xtextedt.hxx>
#include <vcl/txtattr.hxx>
#include <svtools/textwindowpeer.hxx>
#include <tools/stream.hxx>
#include <comphelper/syntaxhighlight.hxx>
#include "svtools/treelistentry.hxx"
#include <vcl/taskpanelist.hxx>
#include <vcl/help.hxx>

#include <vector>
#include "com/sun/star/reflection/XInterfaceMemberTypeDescription.hpp"
#include "com/sun/star/reflection/XIdlMethod.hpp"
#include "com/sun/star/reflection/XIdlField.hpp"
#include "com/sun/star/uno/Exception.hpp"

namespace basctl
{

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace
{

sal_uInt16 const NoMarker = 0xFFFF;

long nVirtToolBoxHeight;    // inited in WatchWindow, used in Stackwindow
long nHeaderBarHeight;

// Returns pBase converted to SbxVariable if valid and is not an SbxMethod.
SbxVariable* IsSbxVariable (SbxBase* pBase)
{
    if (SbxVariable* pVar = dynamic_cast<SbxVariable*>(pBase))
        if (!dynamic_cast<SbxMethod*>(pVar))
            return pVar;
    return 0;
}

Image GetImage (unsigned nId)
{
    static ImageList const aImagesNormal(IDEResId(RID_IMGLST_LAYOUT));
    return aImagesNormal.GetImage(nId);
}

int const nScrollLine = 12;
int const nScrollPage = 60;
int const DWBORDER = 3;

char const cSuffixes[] = "%&!#@$";

} // namespace


/**
 * Helper functions to get/set text in TextEngine using
 * the stream interface.
 *
 * get/setText() only supports tools Strings limited to 64K).
 */
OUString getTextEngineText (ExtTextEngine& rEngine)
{
    SvMemoryStream aMemStream;
    aMemStream.SetStreamCharSet( RTL_TEXTENCODING_UTF8 );
    aMemStream.SetLineDelimiter( LINEEND_LF );
    rEngine.Write( aMemStream );
    sal_uLong nSize = aMemStream.Tell();
    OUString aText( (const sal_Char*)aMemStream.GetData(),
        nSize, RTL_TEXTENCODING_UTF8 );
    return aText;
}

void setTextEngineText (ExtTextEngine& rEngine, OUString const& aStr)
{
    rEngine.SetText(OUString());
    OString aUTF8Str = OUStringToOString( aStr, RTL_TEXTENCODING_UTF8 );
    SvMemoryStream aMemStream( (void*)aUTF8Str.getStr(), aUTF8Str.getLength(),
        STREAM_READ | STREAM_SEEK_TO_BEGIN );
    aMemStream.SetStreamCharSet( RTL_TEXTENCODING_UTF8 );
    aMemStream.SetLineDelimiter( LINEEND_LF );
    rEngine.Read(aMemStream);
}

namespace
{

void lcl_DrawIDEWindowFrame( DockingWindow* pWin )
{
    if ( pWin->IsFloatingMode() )
        return;

    Size aSz = pWin->GetOutputSizePixel();
    const Color aOldLineColor( pWin->GetLineColor() );
    pWin->SetLineColor( Color( COL_WHITE ) );
    // White line on top
    pWin->DrawLine( Point( 0, 0 ), Point( aSz.Width(), 0 ) );
    // Black line at bottom
    pWin->SetLineColor( Color( COL_BLACK ) );
    pWin->DrawLine( Point( 0, aSz.Height() - 1 ),
                    Point( aSz.Width(), aSz.Height() - 1 ) );
    pWin->SetLineColor( aOldLineColor );
}

void lcl_SeparateNameAndIndex( const String& rVName, String& rVar, String& rIndex )
{
    rVar = rVName;
    rIndex.Erase();
    sal_uInt16 nIndexStart = rVar.Search( '(' );
    if ( nIndexStart != STRING_NOTFOUND )
    {
        sal_uInt16 nIndexEnd = rVar.Search( ')', nIndexStart );
        if ( nIndexStart != STRING_NOTFOUND )
        {
            rIndex = rVar.Copy( nIndexStart+1, nIndexEnd-nIndexStart-1 );
            rVar.Erase( nIndexStart );
            rVar = comphelper::string::stripEnd(rVar, ' ');
            rIndex = comphelper::string::strip(rIndex, ' ');
        }
    }

    if ( rVar.Len() )
    {
        sal_uInt16 nLastChar = rVar.Len()-1;
        if ( strchr( cSuffixes, rVar.GetChar( nLastChar ) ) )
            rVar.Erase( nLastChar, 1 );
    }
    if ( rIndex.Len() )
    {
        sal_uInt16 nLastChar = rIndex.Len()-1;
        if ( strchr( cSuffixes, rIndex.GetChar( nLastChar ) ) )
            rIndex.Erase( nLastChar, 1 );
    }
}

} // namespace


//
// EditorWindow
// ============
//

class EditorWindow::ChangesListener:
    public cppu::WeakImplHelper1< beans::XPropertiesChangeListener >
{
public:
    ChangesListener(EditorWindow & editor): editor_(editor) {}

private:
    virtual ~ChangesListener() {}

    virtual void SAL_CALL disposing(lang::EventObject const &) throw (RuntimeException)
    {
        osl::MutexGuard g(editor_.mutex_);
        editor_.notifier_.clear();
    }

    virtual void SAL_CALL propertiesChange(
        Sequence< beans::PropertyChangeEvent > const &) throw (RuntimeException)
    {
        SolarMutexGuard g;
        editor_.ImplSetFont();
    }

    EditorWindow & editor_;
};

class EditorWindow::ProgressInfo : public SfxProgress
{
public:
    ProgressInfo (SfxObjectShell* pObjSh, String const& rText, sal_uLong nRange) :
        SfxProgress(pObjSh, rText, nRange),
        nCurState(0)
    { }

    void StepProgress ()
    {
        SetState(++nCurState);
    }

private:
    sal_uLong nCurState;
};

EditorWindow::EditorWindow (Window* pParent, ModulWindow* pModulWindow) :
    Window(pParent, WB_BORDER),
    pEditView(0),
    pEditEngine(0),
    rModulWindow(*pModulWindow),
    nCurTextWidth(0),
    bHighlightning(false),
    bDoSyntaxHighlight(true),
    bDelayHighlight(true),
    pCodeCompleteWnd(new CodeCompleteWindow(this))
{
    SetBackground(Wallpaper(GetSettings().GetStyleSettings().GetFieldColor()));
    SetPointer( Pointer( POINTER_TEXT ) );
    SetHelpId( HID_BASICIDE_EDITORWINDOW );

    listener_ = new ChangesListener(*this);
    Reference< beans::XMultiPropertySet > n(
        officecfg::Office::Common::Font::SourceViewFont::get(),
        UNO_QUERY_THROW);
    {
        osl::MutexGuard g(mutex_);
        notifier_ = n;
    }
    Sequence< OUString > s(2);
    s[0] = OUString( "FontHeight" );
    s[1] = OUString( "FontName" );
    n->addPropertiesChangeListener(s, listener_.get());
}


EditorWindow::~EditorWindow()
{
    Reference< beans::XMultiPropertySet > n;
    {
        osl::MutexGuard g(mutex_);
        n = notifier_;
    }
    if (n.is()) {
        n->removePropertiesChangeListener(listener_.get());
    }

    aSyntaxIdleTimer.Stop();

    if ( pEditEngine )
    {
        EndListening( *pEditEngine );
        pEditEngine->RemoveView(pEditView.get());
    }
}

OUString EditorWindow::GetWordAtCursor()
{
    String aWord;

    if ( pEditView )
    {
        TextEngine* pTextEngine = pEditView->GetTextEngine();
        if ( pTextEngine )
        {
            // check first, if the cursor is at a help URL
            const TextSelection& rSelection = pEditView->GetSelection();
            const TextPaM& rSelStart = rSelection.GetStart();
            const TextPaM& rSelEnd = rSelection.GetEnd();
            OUString aText = pTextEngine->GetText( rSelEnd.GetPara() );
            CharClass aClass( ::comphelper::getProcessComponentContext() , Application::GetSettings().GetLanguageTag() );
            sal_Int32 nSelStart = rSelStart.GetIndex();
            sal_Int32 nSelEnd = rSelEnd.GetIndex();
            sal_Int32 nLength = aText.getLength();
            sal_Int32 nStart = 0;
            sal_Int32 nEnd = nLength;
            while ( nStart < nLength )
            {
                OUString aURL( URIHelper::FindFirstURLInText( aText, nStart, nEnd, aClass ) );
                INetURLObject aURLObj( aURL );
                if ( aURLObj.GetProtocol() == INET_PROT_VND_SUN_STAR_HELP
                     && nSelStart >= nStart && nSelStart <= nEnd && nSelEnd >= nStart && nSelEnd <= nEnd )
                {
                    aWord = aURL;
                    break;
                }
                nStart = nEnd;
                nEnd = nLength;
            }

            // Not the selected range, but at the CursorPosition,
            // if a word is partially selected.
            if ( !aWord.Len() )
                aWord = pTextEngine->GetWord( rSelEnd );

            // Can be empty when full word selected, as Cursor behing it
            if ( !aWord.Len() && pEditView->HasSelection() )
                aWord = pTextEngine->GetWord( rSelStart );
        }
    }

    return aWord;
}

void EditorWindow::RequestHelp( const HelpEvent& rHEvt )
{
    bool bDone = false;

    // Should have been activated at some point
    if ( pEditEngine )
    {
        if ( rHEvt.GetMode() & HELPMODE_CONTEXT )
        {
            String aKeyword = GetWordAtCursor();
            Application::GetHelp()->SearchKeyword( aKeyword );
            bDone = true;
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
                TextPaM aCursor = GetEditView()->GetTextEngine()->GetPaM(aDocPos, false);
                TextPaM aStartOfWord;
                String aWord = GetEditView()->GetTextEngine()->GetWord( aCursor, &aStartOfWord );
                if ( aWord.Len() && !comphelper::string::isdigitAsciiString(aWord) )
                {
                    sal_uInt16 nLastChar =aWord.Len()-1;
                    if ( strchr( cSuffixes, aWord.GetChar( nLastChar ) ) )
                        aWord.Erase( nLastChar, 1 );
                    SbxBase* pSBX = StarBASIC::FindSBXInCurrentScope( aWord );
                    if (SbxVariable const* pVar = IsSbxVariable(pSBX))
                    {
                        SbxDataType eType = pVar->GetType();
                        if ( (sal_uInt8)eType == (sal_uInt8)SbxOBJECT )
                            // might cause a crash e. g. at the selections-object
                            // Type == Object does not mean pVar == Object!
                            ; // aHelpText = ((SbxObject*)pVar)->GetClassName();
                        else if ( eType & SbxARRAY )
                            ; // aHelpText = "{...}";
                        else if ( (sal_uInt8)eType != (sal_uInt8)SbxEMPTY )
                        {
                            aHelpText = pVar->GetName();
                            if ( !aHelpText.Len() )     // name is not copied with the passed parameters
                                aHelpText = aWord;
                            aHelpText += '=';
                            aHelpText += pVar->GetOUString();
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
            bDone = true;
        }
    }

    if ( !bDone )
        Window::RequestHelp( rHEvt );
}


void EditorWindow::Resize()
{
    // ScrollBars, etc. happens in Adjust...
    if ( pEditView )
    {
        long nVisY = pEditView->GetStartDocPos().Y();

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
            rModulWindow.GetBreakPointWindow().GetCurYOffset() = aStartDocPos.Y();
            rModulWindow.GetLineNumberWindow().GetCurYOffset() = aStartDocPos.Y();
        }
        InitScrollBars();
        if ( nVisY != pEditView->GetStartDocPos().Y() )
            Invalidate();
    }
}


void EditorWindow::MouseMove( const MouseEvent &rEvt )
{
    if ( pEditView )
        pEditView->MouseMove( rEvt );
}


void EditorWindow::MouseButtonUp( const MouseEvent &rEvt )
{
    if ( pEditView )
    {
        pEditView->MouseButtonUp( rEvt );
        if (SfxBindings* pBindings = GetBindingsPtr())
        {
            pBindings->Invalidate( SID_BASICIDE_STAT_POS );
        }
    }
}

void EditorWindow::MouseButtonDown( const MouseEvent &rEvt )
{
    GrabFocus();
    if ( pEditView )
        pEditView->MouseButtonDown( rEvt );
    if( pCodeCompleteWnd->IsVisible() )
    {
        if( pEditView->GetSelection() != pCodeCompleteWnd->GetTextSelection() )
        {//selection changed, code complete window should be hidden
            pCodeCompleteWnd->GetListBox()->HideAndRestoreFocus();
        }
    }
}

void EditorWindow::Command( const CommandEvent& rCEvt )
{
    if ( pEditView )
    {
        pEditView->Command( rCEvt );
        if ( ( rCEvt.GetCommand() == COMMAND_WHEEL ) ||
             ( rCEvt.GetCommand() == COMMAND_STARTAUTOSCROLL ) ||
             ( rCEvt.GetCommand() == COMMAND_AUTOSCROLL ) )
        {
            HandleScrollCommand( rCEvt, rModulWindow.GetHScrollBar(), &rModulWindow.GetEditVScrollBar() );
        } else if ( rCEvt.GetCommand() == COMMAND_CONTEXTMENU ) {
            SfxDispatcher* pDispatcher = GetDispatcher();
            if ( pDispatcher )
            {
                pDispatcher->ExecutePopup();
            }
            if( pCodeCompleteWnd->IsVisible() ) // hide the code complete window
                pCodeCompleteWnd->ClearAndHide();
        }
    }
}

bool EditorWindow::ImpCanModify()
{
    bool bCanModify = true;
    if ( StarBASIC::IsRunning() )
    {
        // If in Trace-mode, abort the trace or refuse input
        // Remove markers in the modules in Notify at Basic::Stoped
        if ( QueryBox( 0, WB_OK_CANCEL, IDEResId(RID_STR_WILLSTOPPRG).toString() ).Execute() == RET_OK )
        {
            rModulWindow.GetBasicStatus().bIsRunning = false;
            StopBasic();
        }
        else
            bCanModify = false;
    }
    return bCanModify;
}

void EditorWindow::KeyInput( const KeyEvent& rKEvt )
{
    if ( !pEditView )   // Happens in Win95
        return;

#if OSL_DEBUG_LEVEL > 1
    Range aRange = rModulWindow.GetHScrollBar()->GetRange(); (void)aRange;
    long nVisSz = rModulWindow.GetHScrollBar()->GetVisibleSize(); (void)nVisSz;
    long nPapSz = rModulWindow.GetHScrollBar()->GetPageSize(); (void)nPapSz;
    long nLinSz = rModulWindow.GetHScrollBar()->GetLineSize(); (void)nLinSz;
    long nThumb = rModulWindow.GetHScrollBar()->GetThumbPos(); (void)nThumb;
#endif
    bool const bWasModified = pEditEngine->IsModified();
    // see if there is an accelerator to be processed first
    bool bDone = SfxViewShell::Current()->KeyInput( rKEvt );

    if( pCodeCompleteWnd->IsVisible() && CodeCompleteOptions::IsCodeCompleteOn() )
    {
        //std::cerr << "EditorWindow::KeyInput" << std::endl;
        pCodeCompleteWnd->GetListBox()->KeyInput(rKEvt);
        if( rKEvt.GetKeyCode().GetCode() == KEY_UP
            || rKEvt.GetKeyCode().GetCode() == KEY_DOWN
            || rKEvt.GetKeyCode().GetCode() == KEY_TAB )
            return;
    }

    if( (rKEvt.GetKeyCode().GetCode() == KEY_SPACE ||
        rKEvt.GetKeyCode().GetCode() == KEY_TAB ||
        rKEvt.GetKeyCode().GetCode() == KEY_RETURN ) && CodeCompleteOptions::IsAutoCorrectKeywordsOn() )
    {
        HandleAutoCorrect();
    }

    if( rKEvt.GetCharCode() == '"' && CodeCompleteOptions::IsAutoCloseQuotesOn() )
    {//autoclose double quotes
        HandleAutoCloseDoubleQuotes();
    }

    if( rKEvt.GetCharCode() == '(' && CodeCompleteOptions::IsAutoCloseParenthesisOn() )
    {//autoclose parenthesis
        HandleAutoCloseParen();
    }

    if( rKEvt.GetKeyCode().GetCode() == KEY_RETURN && CodeCompleteOptions::IsProcedureAutoCompleteOn() )
    {//autoclose implementation
       HandleProcedureCompletition();
    }

    if( rKEvt.GetKeyCode().GetCode() == KEY_POINT &&
        (CodeCompleteOptions::IsCodeCompleteOn() || CodeCompleteOptions::IsExtendedTypeDeclaration()) )
    {
        HandleCodeCompletition();
    }
    if ( !bDone && ( !TextEngine::DoesKeyChangeText( rKEvt ) || ImpCanModify()  ) )
    {
        if ( ( rKEvt.GetKeyCode().GetCode() == KEY_TAB ) && !rKEvt.GetKeyCode().IsMod1() &&
              !rKEvt.GetKeyCode().IsMod2() && !GetEditView()->IsReadOnly() )
        {
            TextSelection aSel( pEditView->GetSelection() );
            if ( aSel.GetStart().GetPara() != aSel.GetEnd().GetPara() )
            {
                bDelayHighlight = false;
                if ( !rKEvt.GetKeyCode().IsShift() )
                    pEditView->IndentBlock();
                else
                    pEditView->UnindentBlock();
                bDelayHighlight = true;
                bDone = true;
            }
        }
        if ( !bDone )
            bDone = pEditView->KeyInput( rKEvt );
    }
    if ( !bDone )
    {
            Window::KeyInput( rKEvt );
    }
    else
    {
        if (SfxBindings* pBindings = GetBindingsPtr())
        {
            pBindings->Invalidate( SID_BASICIDE_STAT_POS );
            if ( rKEvt.GetKeyCode().GetGroup() == KEYGROUP_CURSOR )
                pBindings->Update( SID_BASICIDE_STAT_POS );
            if ( !bWasModified && pEditEngine->IsModified() )
            {
                pBindings->Invalidate( SID_SAVEDOC );
                pBindings->Invalidate( SID_DOC_MODIFIED );
                pBindings->Invalidate( SID_UNDO );
            }
            if ( rKEvt.GetKeyCode().GetCode() == KEY_INSERT )
                pBindings->Invalidate( SID_ATTR_INSERT );
        }
    }
}

void EditorWindow::HandleAutoCorrect()
{
    TextSelection aSel = GetEditView()->GetSelection();
    sal_uLong nLine =  aSel.GetStart().GetPara();
    OUString aLine( pEditEngine->GetText( nLine ) ); // the line being modified

    HighlightPortions aPortions;
    aHighlighter.getHighlightPortions( nLine, aLine, aPortions );

    if( aPortions.size() == 0 )
        return;

    HighlightPortion& r = aPortions[aPortions.size()-1];
    if( r.tokenType == 9 ) // correct the last entered keyword
    {
        OUString sStr = aLine.copy(r.nBegin, r.nEnd - r.nBegin);
        if( !sStr.isEmpty() )
        {
            //capitalize first letter and replace
            sStr = sStr.toAsciiLowerCase();
            sStr = sStr.replaceAt( 0, 1, OUString(sStr[0]).toAsciiUpperCase() );

            TextPaM aStart(nLine, aSel.GetStart().GetIndex() - sStr.getLength() );
            TextSelection sTextSelection(aStart, TextPaM(nLine, aSel.GetStart().GetIndex()));
            pEditEngine->ReplaceText( sTextSelection, sStr );
            pEditView->SetSelection( aSel );
        }
    }
}

void EditorWindow::HandleAutoCloseParen()
{
    TextSelection aSel = GetEditView()->GetSelection();
    sal_uLong nLine =  aSel.GetStart().GetPara();
    OUString aLine( pEditEngine->GetText( nLine ) ); // the line being modified

    if( aLine.getLength() > 0 && aLine[aSel.GetEnd().GetIndex()-1] != '(' )
    {
        GetEditView()->InsertText(OUString(")"));
        //leave the cursor on it's place: inside the parenthesis
        TextPaM aEnd(nLine, aSel.GetEnd().GetIndex());
        GetEditView()->SetSelection( TextSelection( aEnd, aEnd ) );
    }
}

void EditorWindow::HandleAutoCloseDoubleQuotes()
{
    TextSelection aSel = GetEditView()->GetSelection();
    sal_uLong nLine =  aSel.GetStart().GetPara();
    OUString aLine( pEditEngine->GetText( nLine ) ); // the line being modified

    HighlightPortions aPortions;
    aHighlighter.getHighlightPortions( nLine, aLine, aPortions );

    if( aPortions.size() == 0 )
        return;

    if( aLine.getLength() > 0 && aLine[aLine.getLength()-1] != '"' && (aPortions[aPortions.size()-1].tokenType != 4) )
    {
        GetEditView()->InsertText(OUString("\""));
        //leave the cursor on it's place: inside the two double quotes
        TextPaM aEnd(nLine, aSel.GetEnd().GetIndex());
        GetEditView()->SetSelection( TextSelection( aEnd, aEnd ) );
    }
}

void EditorWindow::HandleProcedureCompletition()
{
    TextSelection aSel = GetEditView()->GetSelection();
    sal_uLong nLine = aSel.GetStart().GetPara();
    OUString aLine( pEditEngine->GetText( nLine ) );

    HighlightPortions aPortions;
    aHighlighter.getHighlightPortions( nLine, aLine, aPortions );

    if( aPortions.size() == 0 )
        return;

    OUString sProcType;
    OUString sProcName;
    bool bFoundType = false;
    bool bFoundName = false;

    for ( size_t i = 0; i < aPortions.size(); i++ )
    {
        HighlightPortion& r = aPortions[i];
        OUString sTokStr = aLine.copy(r.nBegin, r.nEnd - r.nBegin);

        if( r.tokenType == 9 && ( sTokStr.equalsIgnoreAsciiCase("sub")
            || sTokStr.equalsIgnoreAsciiCase("function")) )
        {
            sProcType = sTokStr;
            bFoundType = true;
        }
        if( r.tokenType == 1 && bFoundType )
        {
            sProcName = sTokStr;
            bFoundName = true;
            break;
        }
    }

    if( !bFoundType || !bFoundName )
        return;// no sub/function keyword or there is no identifier

    OUString sText("\nEnd ");
    if( sProcType.equalsIgnoreAsciiCase("function") )
        sText += OUString( "Function\n" );
    if( sProcType.equalsIgnoreAsciiCase("sub") )
        sText += OUString( "Sub\n" );

    if( nLine+1 == pEditEngine->GetParagraphCount() )
        pEditView->InsertText( sText );//append to the end
    else
    {
        for( sal_uLong i = nLine+1; i < pEditEngine->GetParagraphCount(); ++i )
        {//searching forward for end token, or another sub/function definition
            OUString aCurrLine = pEditEngine->GetText( i );
            HighlightPortions aCurrPortions;
            aHighlighter.getHighlightPortions( i, aCurrLine, aCurrPortions );

            if( aCurrPortions.size() >= 3 )
            {//at least 3 tokens: (sub|function) whitespace idetifier ....
                HighlightPortion& r = aCurrPortions[0];
                OUString sStr = aCurrLine.copy(r.nBegin, r.nEnd - r.nBegin);

                if( r.tokenType == 9 )
                {
                    if( sStr.equalsIgnoreAsciiCase("sub") || sStr.equalsIgnoreAsciiCase("function") )
                    {
                        pEditView->InsertText( sText );
                        break;
                    }
                    if( sStr.equalsIgnoreAsciiCase("end") )
                        break;
                }
            }
        }
    }
}

void EditorWindow::HandleCodeCompletition()
{
    rModulWindow.UpdateModule();
    rModulWindow.GetSbModule()->GetCodeCompleteDataFromParse(aCodeCompleteCache);
    TextSelection aSel = GetEditView()->GetSelection();
    sal_uLong nLine =  aSel.GetStart().GetPara();
    OUString aLine( pEditEngine->GetText( nLine ) ); // the line being modified
    std::vector< OUString > aVect;

    HighlightPortions aPortions;
    aHighlighter.getHighlightPortions( nLine, aLine, aPortions );
    if( aPortions.size() != 0 )
    {//use the syntax highlighter to grab out nested reflection calls, eg. aVar.aMethod("aa").aOtherMethod ..
        for ( size_t i = 0; i < aPortions.size(); i++ )
        {
            HighlightPortion& r = aPortions[i];
            if( r.tokenType == 1 || r.tokenType == 9) // extract the identifers(methods, base variable)
            /* an example: Dim aLocVar2 as com.sun.star.beans.PropertyValue
             * here, aLocVar2.Name, and PropertyValue's Name field is treated as a keyword(?!)
             * */
                aVect.push_back( aLine.copy(r.nBegin, r.nEnd - r.nBegin) );
        }

        OUString sBaseName = aVect[0];//variable name
        OUString sVarType = aCodeCompleteCache.GetVarType( sBaseName );
        if( !sVarType.isEmpty() && CodeCompleteOptions::IsAutoCorrectKeywordsOn() )//correct variable name
        {
            TextPaM aStart(nLine, aSel.GetStart().GetIndex() - sBaseName.getLength() );
            TextSelection sTextSelection(aStart, TextPaM(nLine, aSel.GetStart().GetIndex()));
            pEditEngine->ReplaceText( sTextSelection, aCodeCompleteCache.GetCorrectCaseVarName(sBaseName) );
            pEditView->SetSelection( aSel );
        }

        UnoTypeCodeCompletetor aTypeCompletor( aVect, sVarType );

        if( aTypeCompletor.CanCodeComplete() )
        {
            std::vector< OUString > aEntryVect;//entries to be inserted into the list
            std::vector< OUString > aFieldVect = aTypeCompletor.GetXIdlClassFields();//fields
            aEntryVect.insert(aEntryVect.end(), aFieldVect.begin(), aFieldVect.end() );
            if( CodeCompleteOptions::IsExtendedTypeDeclaration() )
            {// if extended types on, reflect classes, else just the structs (XIdlClass without methods)
                std::vector< OUString > aMethVect = aTypeCompletor.GetXIdlClassMethods();//methods
                aEntryVect.insert(aEntryVect.end(), aMethVect.begin(), aMethVect.end() );
            }
            if( aEntryVect.size() > 0 )
            {
                SetupAndShowCodeCompleteWnd( aEntryVect, aSel );
            }
        }
    }
}

void EditorWindow::SetupAndShowCodeCompleteWnd(const std::vector< OUString >& aEntryVect, TextSelection aSel )
{
    // calculate position
    Rectangle aRect = ( (TextEngine*) GetEditEngine() )->PaMtoEditCursor( aSel.GetEnd() , false );
    long nViewYOffset = pEditView->GetStartDocPos().Y();
    Point aPoint = aRect.BottomRight();
    aPoint.Y() = (aPoint.Y() - nViewYOffset) + 2;
    aSel.GetStart().GetIndex() += 1;
    aSel.GetEnd().GetIndex() += 1;
    pCodeCompleteWnd->ClearListBox();
    pCodeCompleteWnd->SetTextSelection(aSel);
    //fill the listbox
    for(unsigned int l = 0; l < aEntryVect.size(); ++l)
    {
        pCodeCompleteWnd->InsertEntry( aEntryVect[l] );
    }
    //show it
    pCodeCompleteWnd->SetPosPixel( aPoint );
    pCodeCompleteWnd->Show();
    pCodeCompleteWnd->ResizeListBox();
    pCodeCompleteWnd->SelectFirstEntry();
    pEditView->GetWindow()->GrabFocus();
}

void EditorWindow::Paint( const Rectangle& rRect )
{
    if ( !pEditEngine )     // We need it now at latest
        CreateEditEngine();

    pEditView->Paint( rRect );
}

void EditorWindow::LoseFocus()
{
    SetSourceInBasic();
    Window::LoseFocus();
}

bool EditorWindow::SetSourceInBasic()
{
    bool bChanged = false;
    if ( pEditEngine && pEditEngine->IsModified()
        && !GetEditView()->IsReadOnly() )   // Added for #i60626, otherwise
            // any read only bug in the text engine could lead to a crash later
    {
        if ( !StarBASIC::IsRunning() ) // Not at runtime!
        {
            rModulWindow.UpdateModule();
            bChanged = true;
        }
    }
    return bChanged;
}

// Returns the position of the last character of any of the following
// EOL char combinations: CR, CR/LF, LF, return -1 if no EOL is found
sal_Int32 searchEOL( const OUString& rStr, sal_Int32 fromIndex )
{
    sal_Int32 iRetPos = -1;

    sal_Int32 iLF = rStr.indexOf( LINE_SEP, fromIndex );
    if( iLF != -1 )
    {
        iRetPos = iLF;
    }
    else
    {
        iRetPos = rStr.indexOf( LINE_SEP_CR, fromIndex );
    }
    return iRetPos;
}

void EditorWindow::CreateEditEngine()
{
    if ( pEditEngine )
        return;

    pEditEngine.reset(new ExtTextEngine);
    pEditView.reset(new ExtTextView(pEditEngine.get(), this));
    pEditView->SetAutoIndentMode(true);
    pEditEngine->SetUpdateMode(false);
    pEditEngine->InsertView(pEditView.get());

    ImplSetFont();

    aSyntaxIdleTimer.SetTimeout( 200 );
    aSyntaxIdleTimer.SetTimeoutHdl( LINK( this, EditorWindow, SyntaxTimerHdl ) );

    aHighlighter.initialize( HIGHLIGHT_BASIC );

    bool bWasDoSyntaxHighlight = bDoSyntaxHighlight;
    bDoSyntaxHighlight = false; // too slow for large texts...
    OUString aOUSource(rModulWindow.GetModule());
    sal_Int32 nLines = 0;
    sal_Int32 nIndex = -1;
    do
    {
        nLines++;
        nIndex = searchEOL( aOUSource, nIndex+1 );
    }
    while ( nIndex >= 0 );

    // nLines*4: SetText+Formatting+DoHighlight+Formatting
    // it could be cut down on one formatting but you would wait even longer
    // for the text then if the source code is long...
    pProgress.reset(new ProgressInfo(
        GetShell()->GetViewFrame()->GetObjectShell(),
        IDEResId(RID_STR_GENERATESOURCE).toString(),
        nLines*4
    ));
    setTextEngineText(*pEditEngine, aOUSource);

    pEditView->SetStartDocPos( Point( 0, 0 ) );
    pEditView->SetSelection( TextSelection() );
    rModulWindow.GetBreakPointWindow().GetCurYOffset() = 0;
    rModulWindow.GetLineNumberWindow().GetCurYOffset() = 0;
    pEditEngine->SetUpdateMode(true);
    rModulWindow.Update();   // has only been invalidated at UpdateMode = true

    pEditView->ShowCursor( true, true );

    StartListening( *pEditEngine );

    aSyntaxIdleTimer.Stop();
    bDoSyntaxHighlight = bWasDoSyntaxHighlight;


    for ( sal_uInt16 nLine = 0; nLine < nLines; nLine++ )
        aSyntaxLineTable.insert( nLine );
    ForceSyntaxTimeout();

    pProgress.reset();

    pEditView->EraseVirtualDevice();
    pEditEngine->SetModified( false );
    pEditEngine->EnableUndo( true );

    InitScrollBars();

    if (SfxBindings* pBindings = GetBindingsPtr())
        pBindings->Invalidate( SID_BASICIDE_STAT_POS );

    DBG_ASSERT( rModulWindow.GetBreakPointWindow().GetCurYOffset() == 0, "CreateEditEngine: Brechpunkte verschoben?" );

    // set readonly mode for readonly libraries
    ScriptDocument aDocument(rModulWindow.GetDocument());
    OUString aOULibName(rModulWindow.GetLibName());
    Reference< script::XLibraryContainer2 > xModLibContainer( aDocument.getLibraryContainer( E_SCRIPTS ), UNO_QUERY );
    if ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) && xModLibContainer->isLibraryReadOnly( aOULibName ) )
    {
        rModulWindow.SetReadOnly(true);
    }

    if ( aDocument.isDocument() && aDocument.isReadOnly() )
        rModulWindow.SetReadOnly(true);
}

// virtual
void EditorWindow::DataChanged(DataChangedEvent const & rDCEvt)
{
    Window::DataChanged(rDCEvt);
    if (rDCEvt.GetType() == DATACHANGED_SETTINGS
        && (rDCEvt.GetFlags() & SETTINGS_STYLE) != 0)
    {
        Color aColor(GetSettings().GetStyleSettings().GetFieldColor());
        const AllSettings* pOldSettings = rDCEvt.GetOldSettings();
        if (!pOldSettings || aColor != pOldSettings->GetStyleSettings().GetFieldColor())
        {
            SetBackground(Wallpaper(aColor));
            Invalidate();
        }
        if (pEditEngine != 0)
        {
            aColor = GetSettings().GetStyleSettings().GetFieldTextColor();
            if (!pOldSettings || aColor !=
                    pOldSettings-> GetStyleSettings().GetFieldTextColor())
            {
                Font aFont(pEditEngine->GetFont());
                aFont.SetColor(aColor);
                pEditEngine->SetFont(aFont);
            }
        }
    }
}

void EditorWindow::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint )
{
    if (TextHint const* pTextHint = dynamic_cast<TextHint const*>(&rHint))
    {
        TextHint const& rTextHint = *pTextHint;
        if( rTextHint.GetId() == TEXT_HINT_VIEWSCROLLED )
        {
            if ( rModulWindow.GetHScrollBar() )
                rModulWindow.GetHScrollBar()->SetThumbPos( pEditView->GetStartDocPos().X() );
            rModulWindow.GetEditVScrollBar().SetThumbPos( pEditView->GetStartDocPos().Y() );
            rModulWindow.GetBreakPointWindow().DoScroll
                ( 0, rModulWindow.GetBreakPointWindow().GetCurYOffset() - pEditView->GetStartDocPos().Y() );
            rModulWindow.GetLineNumberWindow().DoScroll
                ( 0, rModulWindow.GetLineNumberWindow().GetCurYOffset() - pEditView->GetStartDocPos().Y() );
        }
        else if( rTextHint.GetId() == TEXT_HINT_TEXTHEIGHTCHANGED )
        {
            if ( pEditView->GetStartDocPos().Y() )
            {
                long nOutHeight = GetOutputSizePixel().Height();
                long nTextHeight = pEditEngine->GetTextHeight();
                if ( nTextHeight < nOutHeight )
                    pEditView->Scroll( 0, pEditView->GetStartDocPos().Y() );

                rModulWindow.GetLineNumberWindow().Invalidate();
            }

            SetScrollBarRanges();
        }
        else if( rTextHint.GetId() == TEXT_HINT_TEXTFORMATTED )
        {
            if ( rModulWindow.GetHScrollBar() )
            {
                sal_uLong nWidth = pEditEngine->CalcTextWidth();
                if ( (long)nWidth != nCurTextWidth )
                {
                    nCurTextWidth = nWidth;
                    rModulWindow.GetHScrollBar()->SetRange( Range( 0, (long)nCurTextWidth-1) );
                    rModulWindow.GetHScrollBar()->SetThumbPos( pEditView->GetStartDocPos().X() );
                }
            }
            long nPrevTextWidth = nCurTextWidth;
            nCurTextWidth = pEditEngine->CalcTextWidth();
            if ( nCurTextWidth != nPrevTextWidth )
                SetScrollBarRanges();
        }
        else if( rTextHint.GetId() == TEXT_HINT_PARAINSERTED )
        {
            ParagraphInsertedDeleted( rTextHint.GetValue(), true );
            DoDelayedSyntaxHighlight( rTextHint.GetValue() );
        }
        else if( rTextHint.GetId() == TEXT_HINT_PARAREMOVED )
        {
            ParagraphInsertedDeleted( rTextHint.GetValue(), false );
        }
        else if( rTextHint.GetId() == TEXT_HINT_PARACONTENTCHANGED )
        {
            DoDelayedSyntaxHighlight( rTextHint.GetValue() );
        }
        else if( rTextHint.GetId() == TEXT_HINT_VIEWSELECTIONCHANGED )
        {
            if (SfxBindings* pBindings = GetBindingsPtr())
            {
                pBindings->Invalidate( SID_CUT );
                pBindings->Invalidate( SID_COPY );
            }
        }
    }
}

OUString EditorWindow::GetActualSubName( sal_uLong nLine )
{
    SbxArrayRef pMethods = rModulWindow.GetSbModule()->GetMethods();
    for( sal_uInt16 i=0; i < pMethods->Count(); i++ )
    {
        SbxVariable* p = PTR_CAST( SbMethod, pMethods->Get( i ) );
        OUString sName = p->GetName();
        SbMethod* pMeth = p ? PTR_CAST( SbMethod, p ) : NULL;
        if( pMeth )
        {
            sal_uInt16 l1,l2;
            pMeth->GetLineRange(l1,l2);
            if( (l1 <= nLine+1) && (nLine+1 <= l2) )
            {
                return sName;
            }
        }
    }
    return OUString("");
}

void EditorWindow::SetScrollBarRanges()
{
    // extra method, not InitScrollBars, because for EditEngine events too
    if ( !pEditEngine )
        return;

    if ( rModulWindow.GetHScrollBar() )
        rModulWindow.GetHScrollBar()->SetRange( Range( 0, nCurTextWidth-1 ) );

    rModulWindow.GetEditVScrollBar().SetRange( Range( 0, pEditEngine->GetTextHeight()-1 ) );
}

void EditorWindow::InitScrollBars()
{
    if ( !pEditEngine )
        return;

    SetScrollBarRanges();
    Size aOutSz( GetOutputSizePixel() );
    rModulWindow.GetEditVScrollBar().SetVisibleSize( aOutSz.Height() );
    rModulWindow.GetEditVScrollBar().SetPageSize( aOutSz.Height() * 8 / 10 );
    rModulWindow.GetEditVScrollBar().SetLineSize( GetTextHeight() );
    rModulWindow.GetEditVScrollBar().SetThumbPos( pEditView->GetStartDocPos().Y() );
    rModulWindow.GetEditVScrollBar().Show();

    if ( rModulWindow.GetHScrollBar() )
    {
        rModulWindow.GetHScrollBar()->SetVisibleSize( aOutSz.Width() );
        rModulWindow.GetHScrollBar()->SetPageSize( aOutSz.Width() * 8 / 10 );
        rModulWindow.GetHScrollBar()->SetLineSize( GetTextWidth( "x" ) );
        rModulWindow.GetHScrollBar()->SetThumbPos( pEditView->GetStartDocPos().X() );
        rModulWindow.GetHScrollBar()->Show();
    }
}

void EditorWindow::ImpDoHighlight( sal_uLong nLine )
{
    if ( bDoSyntaxHighlight )
    {
        OUString aLine( pEditEngine->GetText( nLine ) );
        aHighlighter.notifyChange( nLine, 0, &aLine, 1 );

        bool const bWasModified = pEditEngine->IsModified();
        pEditEngine->RemoveAttribs( nLine, true );
        HighlightPortions aPortions;
        aHighlighter.getHighlightPortions( nLine, aLine, aPortions );

        for ( size_t i = 0; i < aPortions.size(); i++ )
        {
            HighlightPortion& r = aPortions[i];
            Color const aColor = rModulWindow.GetLayout().GetSyntaxColor(r.tokenType);
            pEditEngine->SetAttrib( TextAttribFontColor(aColor), nLine, r.nBegin, r.nEnd, true );
        }

        pEditEngine->SetModified( bWasModified );
    }
}

void EditorWindow::UpdateSyntaxHighlighting ()
{
    unsigned nCount = pEditEngine->GetParagraphCount();
    for (unsigned i = 0; i < nCount; ++i)
        DoDelayedSyntaxHighlight(i);
}

void EditorWindow::ImplSetFont()
{
    OUString sFontName(
        officecfg::Office::Common::Font::SourceViewFont::FontName::get().
        get_value_or( OUString() ) );
    if ( sFontName.isEmpty() )
    {
        Font aTmpFont( OutputDevice::GetDefaultFont( DEFAULTFONT_FIXED, Application::GetSettings().GetUILanguageTag().getLanguageType(), 0 , this ) );
        sFontName = aTmpFont.GetName();
    }
    Size aFontSize(0, officecfg::Office::Common::Font::SourceViewFont::FontHeight::get());
    Font aFont( sFontName, aFontSize );
    aFont.SetColor( GetSettings().GetStyleSettings().GetFieldTextColor() );
    SetPointFont( aFont );
    aFont = GetFont();

    rModulWindow.GetBreakPointWindow().SetFont( aFont );
    rModulWindow.GetLineNumberWindow().SetFont( aFont );

    if ( pEditEngine )
    {
        bool const bModified = pEditEngine->IsModified();
        pEditEngine->SetFont( aFont );
        pEditEngine->SetModified( bModified );
    }
}

void EditorWindow::DoSyntaxHighlight( sal_uLong nPara )
{
    // because of the DelayedSyntaxHighlight it's possible
    // that this line does not exist anymore!
    if ( nPara < pEditEngine->GetParagraphCount() )
    {
        // unfortunately I'm not sure that excactly this line does Modified() ...
        if ( pProgress )
            pProgress->StepProgress();
        ImpDoHighlight( nPara );
    }
}

void EditorWindow::DoDelayedSyntaxHighlight( sal_uLong nPara )
{
    // line is only added to 'Liste' (list), processed in TimerHdl
    // => don't manipulate breaks while EditEngine is formatting
    if ( pProgress )
        pProgress->StepProgress();

    if ( !bHighlightning && bDoSyntaxHighlight )
    {
        if ( bDelayHighlight )
        {
            aSyntaxLineTable.insert( nPara );
            aSyntaxIdleTimer.Start();
        }
        else
            DoSyntaxHighlight( nPara );
    }
}

IMPL_LINK_NOARG(EditorWindow, SyntaxTimerHdl)
{
    DBG_ASSERT( pEditView, "Noch keine View, aber Syntax-Highlight ?!" );

    bool const bWasModified = pEditEngine->IsModified();
    //pEditEngine->SetUpdateMode(false);

    bHighlightning = true;
    for ( SyntaxLineSet::const_iterator it = aSyntaxLineTable.begin();
          it != aSyntaxLineTable.end(); ++it )
    {
        sal_uInt16 nLine = *it;
        DoSyntaxHighlight( nLine );
    }

    // #i45572#
    if ( pEditView )
        pEditView->ShowCursor( false, true );

    pEditEngine->SetModified( bWasModified );

    aSyntaxLineTable.clear();
    bHighlightning = false;

    return 0;
}

void EditorWindow::ParagraphInsertedDeleted( sal_uLong nPara, bool bInserted )
{
    if ( pProgress )
        pProgress->StepProgress();

    if ( !bInserted && ( nPara == TEXT_PARA_ALL ) )
    {
        rModulWindow.GetBreakPoints().reset();
        rModulWindow.GetBreakPointWindow().Invalidate();
        rModulWindow.GetLineNumberWindow().Invalidate();
        aHighlighter.initialize( HIGHLIGHT_BASIC );
    }
    else
    {
        rModulWindow.GetBreakPoints().AdjustBreakPoints( (sal_uInt16)nPara+1, bInserted );

        long nLineHeight = GetTextHeight();
        Size aSz = rModulWindow.GetBreakPointWindow().GetOutputSize();
        Rectangle aInvRect( Point( 0, 0 ), aSz );
        long nY = nPara*nLineHeight - rModulWindow.GetBreakPointWindow().GetCurYOffset();
        aInvRect.Top() = nY;
        rModulWindow.GetBreakPointWindow().Invalidate( aInvRect );

        Size aLnSz(rModulWindow.GetLineNumberWindow().GetWidth(),
                   GetOutputSizePixel().Height() - 2 * DWBORDER);
        rModulWindow.GetLineNumberWindow().SetPosSizePixel(Point(DWBORDER + 19, DWBORDER), aLnSz);
        rModulWindow.GetLineNumberWindow().Invalidate();

        if ( bDoSyntaxHighlight )
        {
            OUString aDummy;
            aHighlighter.notifyChange( nPara, bInserted ? 1 : (-1), &aDummy, 1 );
        }
    }
}

void EditorWindow::CreateProgress( const OUString& rText, sal_uLong nRange )
{
    DBG_ASSERT( !pProgress, "ProgressInfo existiert schon" );
    pProgress.reset(new ProgressInfo(
        GetShell()->GetViewFrame()->GetObjectShell(),
        rText,
        nRange
    ));
}

void EditorWindow::DestroyProgress()
{
    pProgress.reset();
}

void EditorWindow::ForceSyntaxTimeout()
{
    aSyntaxIdleTimer.Stop();
    aSyntaxIdleTimer.GetTimeoutHdl().Call(&aSyntaxIdleTimer);
}


//
// BreakPointWindow
// ================
//

BreakPointWindow::BreakPointWindow (Window* pParent, ModulWindow* pModulWindow) :
    Window(pParent, WB_BORDER),
    rModulWindow(*pModulWindow),
    nCurYOffset(0), // memorize nCurYOffset and not take it from EditEngine
    nMarkerPos(NoMarker)
{
    setBackgroundColor(GetSettings().GetStyleSettings().GetFieldColor());
    SetHelpId(HID_BASICIDE_BREAKPOINTWINDOW);
}

BreakPointWindow::~BreakPointWindow()
{
}



void BreakPointWindow::Paint( const Rectangle& )
{
    if ( SyncYOffset() )
        return;

    Size const aOutSz = GetOutputSize();
    long const nLineHeight = GetTextHeight();

    Image const aBrk[2] = { GetImage(IMGID_BRKDISABLED), GetImage(IMGID_BRKENABLED) };
    Size const aBmpSz = PixelToLogic(aBrk[1].GetSizePixel());
    Point const aBmpOff(
        (aOutSz.Width() - aBmpSz.Width()) / 2,
        (nLineHeight - aBmpSz.Height()) / 2
    );

    for (size_t i = 0, n = GetBreakPoints().size(); i < n; ++i)
    {
        BreakPoint& rBrk = *GetBreakPoints().at(i);
        size_t const nLine = rBrk.nLine - 1;
        size_t const nY = nLine*nLineHeight - nCurYOffset;
        DrawImage(Point(0, nY) + aBmpOff, aBrk[rBrk.bEnabled]);
    }
    ShowMarker(true);
}



void BreakPointWindow::DoScroll( long nHorzScroll, long nVertScroll )
{
    nCurYOffset -= nVertScroll;
    Window::Scroll( nHorzScroll, nVertScroll );
}



void BreakPointWindow::SetMarkerPos( sal_uInt16 nLine, bool bError )
{
    if ( SyncYOffset() )
        Update();

    ShowMarker( false );
    nMarkerPos = nLine;
    bErrorMarker = bError;
    ShowMarker( true );
}

void BreakPointWindow::SetNoMarker ()
{
    SetMarkerPos(NoMarker);
}

void BreakPointWindow::ShowMarker( bool bShow )
{
    if ( nMarkerPos == NoMarker )
        return;

    Size const aOutSz = GetOutputSize();
    long const nLineHeight = GetTextHeight();

    Image aMarker = GetImage(bErrorMarker ? IMGID_ERRORMARKER : IMGID_STEPMARKER);

    Size aMarkerSz( aMarker.GetSizePixel() );
    aMarkerSz = PixelToLogic( aMarkerSz );
    Point aMarkerOff( 0, 0 );
    aMarkerOff.X() = ( aOutSz.Width() - aMarkerSz.Width() ) / 2;
    aMarkerOff.Y() = ( nLineHeight - aMarkerSz.Height() ) / 2;

    sal_uLong nY = nMarkerPos*nLineHeight - nCurYOffset;
    Point aPos( 0, nY );
    aPos += aMarkerOff;
    if ( bShow )
        DrawImage( aPos, aMarker );
    else
        Invalidate( Rectangle( aPos, aMarkerSz ) );
}




BreakPoint* BreakPointWindow::FindBreakPoint( const Point& rMousePos )
{
    size_t nLineHeight = GetTextHeight();
    size_t nYPos = rMousePos.Y() + nCurYOffset;

    for ( size_t i = 0, n = GetBreakPoints().size(); i < n ; ++i )
    {
        BreakPoint* pBrk = GetBreakPoints().at( i );
        size_t nLine = pBrk->nLine-1;
        size_t nY = nLine*nLineHeight;
        if ( ( nYPos > nY ) && ( nYPos < ( nY + nLineHeight ) ) )
            return pBrk;
    }
    return 0;
}

void BreakPointWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.GetClicks() == 2 )
    {
        Point aMousePos( PixelToLogic( rMEvt.GetPosPixel() ) );
        long nLineHeight = GetTextHeight();
        long nYPos = aMousePos.Y() + nCurYOffset;
        long nLine = nYPos / nLineHeight + 1;
        rModulWindow.ToggleBreakPoint( (sal_uLong)nLine );
        Invalidate();
    }
}



void BreakPointWindow::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() == COMMAND_CONTEXTMENU )
    {
        Point aPos( rCEvt.IsMouseEvent() ? rCEvt.GetMousePosPixel() : Point(1,1) );
        Point aEventPos( PixelToLogic( aPos ) );
        BreakPoint* pBrk = rCEvt.IsMouseEvent() ? FindBreakPoint( aEventPos ) : 0;
        if ( pBrk )
        {
            // test if break point is enabled...
            PopupMenu aBrkPropMenu( IDEResId( RID_POPUP_BRKPROPS ) );
            aBrkPropMenu.CheckItem( RID_ACTIV, pBrk->bEnabled );
            switch ( aBrkPropMenu.Execute( this, aPos ) )
            {
                case RID_ACTIV:
                {
                    pBrk->bEnabled = !pBrk->bEnabled;
                    rModulWindow.UpdateBreakPoint( *pBrk );
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

bool BreakPointWindow::SyncYOffset()
{
    TextView* pView = rModulWindow.GetEditView();
    if ( pView )
    {
        long nViewYOffset = pView->GetStartDocPos().Y();
        if ( nCurYOffset != nViewYOffset )
        {
            nCurYOffset = nViewYOffset;
            Invalidate();
            return true;
        }
    }
    return false;
}

// virtual
void BreakPointWindow::DataChanged(DataChangedEvent const & rDCEvt)
{
    Window::DataChanged(rDCEvt);
    if (rDCEvt.GetType() == DATACHANGED_SETTINGS
        && (rDCEvt.GetFlags() & SETTINGS_STYLE) != 0)
    {
        Color aColor(GetSettings().GetStyleSettings().GetFieldColor());
        const AllSettings* pOldSettings = rDCEvt.GetOldSettings();
        if (!pOldSettings || aColor != pOldSettings->GetStyleSettings().GetFieldColor())
        {
            setBackgroundColor(aColor);
            Invalidate();
        }
    }
}

void BreakPointWindow::setBackgroundColor(Color aColor)
{
    SetBackground(Wallpaper(aColor));
}


//
// WatchWindow
// ===========
//

namespace
{
    const sal_uInt16 ITEM_ID_VARIABLE = 1;
    const sal_uInt16 ITEM_ID_VALUE = 2;
    const sal_uInt16 ITEM_ID_TYPE = 3;
}

WatchWindow::WatchWindow (Layout* pParent) :
    DockingWindow(pParent),
    aWatchStr( IDEResId( RID_STR_REMOVEWATCH ) ),
    aXEdit( this, IDEResId( RID_EDT_WATCHEDIT ) ),
    aRemoveWatchButton( this, IDEResId( RID_IMGBTN_REMOVEWATCH ) ),
    aTreeListBox( this, WB_BORDER | WB_3DLOOK | WB_HASBUTTONS | WB_HASLINES | WB_HSCROLL | WB_TABSTOP
                                  | WB_HASLINESATROOT | WB_HASBUTTONSATROOT ),
    aHeaderBar( this, WB_BUTTONSTYLE | WB_BORDER )
{
    aXEdit.SetAccessibleName(IDEResId(RID_STR_WATCHNAME).toString());
    aTreeListBox.SetAccessibleName(IDEResId(RID_STR_WATCHNAME).toString());

    long nTextLen = GetTextWidth( aWatchStr ) + DWBORDER + 3;
    aXEdit.SetPosPixel( Point( nTextLen, 3 ) );
    aXEdit.SetAccHdl( LINK( this, WatchWindow, EditAccHdl ) );
    aXEdit.GetAccelerator().InsertItem( 1, KeyCode( KEY_RETURN ) );
    aXEdit.GetAccelerator().InsertItem( 2, KeyCode( KEY_ESCAPE ) );
    aXEdit.Show();

    aRemoveWatchButton.Disable();
    aRemoveWatchButton.SetClickHdl( LINK( this, WatchWindow, ButtonHdl ) );
    aRemoveWatchButton.SetPosPixel( Point( nTextLen + aXEdit.GetSizePixel().Width() + 4, 2 ) );
    Size aSz( aRemoveWatchButton.GetModeImage().GetSizePixel() );
    aSz.Width() += 6;
    aSz.Height() += 6;
    aRemoveWatchButton.SetSizePixel( aSz );
    aRemoveWatchButton.Show();

    long nRWBtnSize = aRemoveWatchButton.GetModeImage().GetSizePixel().Height() + 10;
    nVirtToolBoxHeight = aXEdit.GetSizePixel().Height() + 7;

    if ( nRWBtnSize > nVirtToolBoxHeight )
        nVirtToolBoxHeight = nRWBtnSize;

    nHeaderBarHeight = 16;

    aTreeListBox.SetHelpId(HID_BASICIDE_WATCHWINDOW_LIST);
    aTreeListBox.EnableInplaceEditing(true);
    aTreeListBox.SetSelectHdl( LINK( this, WatchWindow, TreeListHdl ) );
    aTreeListBox.SetPosPixel( Point( DWBORDER, nVirtToolBoxHeight + nHeaderBarHeight ) );
    aTreeListBox.SetHighlightRange( 1, 5 );

    Point aPnt( DWBORDER, nVirtToolBoxHeight + 1 );
    aHeaderBar.SetPosPixel( aPnt );
    aHeaderBar.SetEndDragHdl( LINK( this, WatchWindow, implEndDragHdl ) );

    long nVarTabWidth = 220;
    long nValueTabWidth = 100;
    long nTypeTabWidth = 1250;
    aHeaderBar.InsertItem( ITEM_ID_VARIABLE, IDEResId(RID_STR_WATCHVARIABLE).toString(), nVarTabWidth );
    aHeaderBar.InsertItem( ITEM_ID_VALUE, IDEResId(RID_STR_WATCHVALUE).toString(), nValueTabWidth );
    aHeaderBar.InsertItem( ITEM_ID_TYPE, IDEResId(RID_STR_WATCHTYPE).toString(), nTypeTabWidth );

    long tabs[ 4 ];
    tabs[ 0 ] = 3; // two tabs
    tabs[ 1 ] = 0;
    tabs[ 2 ] = nVarTabWidth;
    tabs[ 3 ] = nVarTabWidth + nValueTabWidth;
    aTreeListBox.SvHeaderTabListBox::SetTabs( tabs, MAP_PIXEL );
    aTreeListBox.InitHeaderBar( &aHeaderBar );

    aTreeListBox.SetNodeDefaultImages( );

    aHeaderBar.Show();

    aTreeListBox.Show();

    SetText(IDEResId(RID_STR_WATCHNAME).toString());

    SetHelpId( HID_BASICIDE_WATCHWINDOW );

    // make watch window keyboard accessible
    GetSystemWindow()->GetTaskPaneList()->AddWindow( this );
}



WatchWindow::~WatchWindow()
{
    GetSystemWindow()->GetTaskPaneList()->RemoveWindow( this );
}



void WatchWindow::Paint( const Rectangle& )
{
    DrawText( Point( DWBORDER, 7 ), aWatchStr );
    lcl_DrawIDEWindowFrame( this );
}



void WatchWindow::Resize()
{
    Size aSz = GetOutputSizePixel();
    Size aBoxSz( aSz.Width() - 2*DWBORDER, aSz.Height() - nVirtToolBoxHeight - DWBORDER );

    if ( aBoxSz.Width() < 4 )
        aBoxSz.Width() = 0;
    if ( aBoxSz.Height() < 4 )
        aBoxSz.Height() = 0;

    aBoxSz.Height() -= nHeaderBarHeight;
    aTreeListBox.SetSizePixel( aBoxSz );
    aTreeListBox.GetHScroll()->SetPageSize( aTreeListBox.GetHScroll()->GetVisibleSize() );

    aBoxSz.Height() = nHeaderBarHeight;
    aHeaderBar.SetSizePixel( aBoxSz );

    Invalidate();
}

struct WatchItem
{
    String          maName;
    String          maDisplayName;
    SbxObjectRef    mpObject;
    std::vector<String> maMemberList;

    SbxDimArrayRef  mpArray;
    int             nDimLevel;  // 0 = Root
    int             nDimCount;
    std::vector<short> vIndices;

    WatchItem*      mpArrayParentItem;

    WatchItem (String const& rName):
        maName(rName),
        nDimLevel(0),
        nDimCount(0),
        mpArrayParentItem(0)
    { }

    void clearWatchItem ()
    {
        maMemberList.clear();
    }

    WatchItem* GetRootItem( void );
    SbxDimArray* GetRootArray( void );
};

WatchItem* WatchItem::GetRootItem( void )
{
    WatchItem* pItem = mpArrayParentItem;
    while( pItem )
    {
        if( pItem->mpArray.Is() )
            break;
        pItem = pItem->mpArrayParentItem;
    }
    return pItem;
}

SbxDimArray* WatchItem::GetRootArray( void )
{
    WatchItem* pRootItem = GetRootItem();
    SbxDimArray* pRet = NULL;
    if( pRootItem )
        pRet = pRootItem->mpArray;
    return pRet;
}

void WatchWindow::AddWatch( const OUString& rVName )
{
    String aVar, aIndex;
    lcl_SeparateNameAndIndex( rVName, aVar, aIndex );
    WatchItem* pWatchItem = new WatchItem(aVar);

    OUString aWatchStr_( aVar );
    aWatchStr_ += "\t\t";
    SvTreeListEntry* pNewEntry = aTreeListBox.InsertEntry( aWatchStr_, 0, true, LIST_APPEND );
    pNewEntry->SetUserData( pWatchItem );

    aTreeListBox.Select(pNewEntry, true);
    aTreeListBox.MakeVisible(pNewEntry);
    aRemoveWatchButton.Enable();

    UpdateWatches();
}

bool WatchWindow::RemoveSelectedWatch()
{
    SvTreeListEntry* pEntry = aTreeListBox.GetCurEntry();
    if ( pEntry )
    {
        aTreeListBox.GetModel()->Remove( pEntry );
        pEntry = aTreeListBox.GetCurEntry();
        if ( pEntry )
            aXEdit.SetText( ((WatchItem*)pEntry->GetUserData())->maName );
        else
            aXEdit.SetText( String() );
        if ( !aTreeListBox.GetEntryCount() )
            aRemoveWatchButton.Disable();
        return true;
    }
    else
        return false;
}


IMPL_LINK_INLINE_START( WatchWindow, ButtonHdl, ImageButton *, pButton )
{
    if (pButton == &aRemoveWatchButton)
        if (SfxDispatcher* pDispatcher = GetDispatcher())
            pDispatcher->Execute(SID_BASICIDE_REMOVEWATCH);
    return 0;
}
IMPL_LINK_INLINE_END( WatchWindow, ButtonHdl, ImageButton *, pButton )



IMPL_LINK_NOARG_INLINE_START(WatchWindow, TreeListHdl)
{
    SvTreeListEntry* pCurEntry = aTreeListBox.GetCurEntry();
    if ( pCurEntry && pCurEntry->GetUserData() )
        aXEdit.SetText( ((WatchItem*)pCurEntry->GetUserData())->maName );

    return 0;
}
IMPL_LINK_NOARG_INLINE_END(WatchWindow, TreeListHdl)


IMPL_LINK_INLINE_START( WatchWindow, implEndDragHdl, HeaderBar *, pBar )
{
    (void)pBar;

    const sal_Int32 TAB_WIDTH_MIN = 10;
    sal_Int32 nMaxWidth =
        aHeaderBar.GetSizePixel().getWidth() - 2 * TAB_WIDTH_MIN;

    sal_Int32 nVariableWith = aHeaderBar.GetItemSize( ITEM_ID_VARIABLE );
    if( nVariableWith < TAB_WIDTH_MIN )
        aHeaderBar.SetItemSize( ITEM_ID_VARIABLE, TAB_WIDTH_MIN );
    else if( nVariableWith > nMaxWidth )
        aHeaderBar.SetItemSize( ITEM_ID_VARIABLE, nMaxWidth );

    sal_Int32 nValueWith = aHeaderBar.GetItemSize( ITEM_ID_VALUE );
    if( nValueWith < TAB_WIDTH_MIN )
        aHeaderBar.SetItemSize( ITEM_ID_VALUE, TAB_WIDTH_MIN );
    else if( nValueWith > nMaxWidth )
        aHeaderBar.SetItemSize( ITEM_ID_VALUE, nMaxWidth );

    if (aHeaderBar.GetItemSize( ITEM_ID_TYPE ) < TAB_WIDTH_MIN)
        aHeaderBar.SetItemSize( ITEM_ID_TYPE, TAB_WIDTH_MIN );

    sal_Int32 nPos = 0;
    sal_uInt16 nTabs = aHeaderBar.GetItemCount();
    for( sal_uInt16 i = 1 ; i < nTabs ; ++i )
    {
        nPos += aHeaderBar.GetItemSize( i );
        aTreeListBox.SetTab( i, nPos, MAP_PIXEL );
    }
    return 0;
}
IMPL_LINK_INLINE_END( WatchWindow, implEndDragHdl, HeaderBar *, pBar )


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
            }
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

void WatchWindow::UpdateWatches( bool bBasicStopped )
{
    aTreeListBox.UpdateWatches( bBasicStopped );
}


//
// StackWindow
// ===========
//

StackWindow::StackWindow (Layout* pParent) :
    DockingWindow(pParent),
    aTreeListBox( this, WB_BORDER | WB_3DLOOK | WB_HSCROLL | WB_TABSTOP ),
    aStackStr( IDEResId( RID_STR_STACK ) )
{
    aTreeListBox.SetHelpId(HID_BASICIDE_STACKWINDOW_LIST);
    aTreeListBox.SetAccessibleName(IDEResId(RID_STR_STACKNAME).toString());
    aTreeListBox.SetPosPixel( Point( DWBORDER, nVirtToolBoxHeight ) );
    aTreeListBox.SetHighlightRange();
    aTreeListBox.SetSelectionMode( NO_SELECTION );
    aTreeListBox.InsertEntry( String(), 0, false, LIST_APPEND );
    aTreeListBox.Show();

    SetText(IDEResId(RID_STR_STACKNAME).toString());

    SetHelpId( HID_BASICIDE_STACKWINDOW );

    // make stack window keyboard accessible
    GetSystemWindow()->GetTaskPaneList()->AddWindow( this );
}



StackWindow::~StackWindow()
{
    GetSystemWindow()->GetTaskPaneList()->RemoveWindow( this );
}



void StackWindow::Paint( const Rectangle& )
{
    DrawText( Point( DWBORDER, 7 ), aStackStr );
    lcl_DrawIDEWindowFrame( this );
}



void StackWindow::Resize()
{
    Size aSz = GetOutputSizePixel();
    Size aBoxSz( aSz.Width() - 2*DWBORDER, aSz.Height() - nVirtToolBoxHeight - DWBORDER );

    if ( aBoxSz.Width() < 4 )
        aBoxSz.Width() = 0;
    if ( aBoxSz.Height() < 4 )
        aBoxSz.Height() = 0;

    aTreeListBox.SetSizePixel( aBoxSz );

    Invalidate();
}

void StackWindow::UpdateCalls()
{
    aTreeListBox.SetUpdateMode(false);
    aTreeListBox.Clear();

    if ( StarBASIC::IsRunning() )
    {
        SbxError eOld = SbxBase::GetError();
        aTreeListBox.SetSelectionMode( SINGLE_SELECTION );

        sal_Int32 nScope = 0;
        SbMethod* pMethod = StarBASIC::GetActiveMethod( nScope );
        while ( pMethod )
        {
            OUString aEntry( OUString::number(nScope ));
            if ( aEntry.getLength() < 2 )
                aEntry = " " + aEntry;
            aEntry += ": "  + pMethod->GetName();
            SbxArray* pParams = pMethod->GetParameters();
            SbxInfo* pInfo = pMethod->GetInfo();
            if ( pParams )
            {
                aEntry += "(";
                // 0 is the sub's name...
                for ( sal_uInt16 nParam = 1; nParam < pParams->Count(); nParam++ )
                {
                    SbxVariable* pVar = pParams->Get( nParam );
                    DBG_ASSERT( pVar, "Parameter?!" );
                    if ( !pVar->GetName().isEmpty() )
                    {
                        aEntry += pVar->GetName();
                    }
                    else if ( pInfo )
                    {
                        const SbxParamInfo* pParam = pInfo->GetParam( nParam );
                        if ( pParam )
                        {
                            aEntry += pParam->aName;
                        }
                    }
                    aEntry += "=";
                    SbxDataType eType = pVar->GetType();
                    if( eType & SbxARRAY )
                    {
                        aEntry += "..." ;
                    }
                    else if( eType != SbxOBJECT )
                    {
                        aEntry += pVar->GetOUString();
                    }
                    if ( nParam < ( pParams->Count() - 1 ) )
                    {
                        aEntry += ", ";
                    }
                }
                aEntry += ")";
            }
            aTreeListBox.InsertEntry( aEntry, 0, false, LIST_APPEND );
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
        aTreeListBox.InsertEntry( String(), 0, false, LIST_APPEND );
    }

    aTreeListBox.SetUpdateMode(true);
}


//
// ComplexEditorWindow
// ===================
//

ComplexEditorWindow::ComplexEditorWindow( ModulWindow* pParent ) :
    Window( pParent, WB_3DLOOK | WB_CLIPCHILDREN ),
    aBrkWindow(this, pParent),
    aLineNumberWindow(this, pParent),
    aEdtWindow(this, pParent),
    aEWVScrollBar( this, WB_VSCROLL | WB_DRAG )
{
    aEdtWindow.Show();
    aBrkWindow.Show();

    aEWVScrollBar.SetLineSize(nScrollLine);
    aEWVScrollBar.SetPageSize(nScrollPage);
    aEWVScrollBar.SetScrollHdl( LINK( this, ComplexEditorWindow, ScrollHdl ) );
    aEWVScrollBar.Show();
}

void ComplexEditorWindow::Resize()
{
    Size aOutSz = GetOutputSizePixel();
    Size aSz( aOutSz );
    aSz.Width() -= 2*DWBORDER;
    aSz.Height() -= 2*DWBORDER;
    long nBrkWidth = 20;
    long nSBWidth = aEWVScrollBar.GetSizePixel().Width();

    Size aBrkSz(nBrkWidth, aSz.Height());

    Size aLnSz(aLineNumberWindow.GetWidth(), aSz.Height());

    if (aLineNumberWindow.IsVisible())
    {
        aBrkWindow.SetPosSizePixel( Point( DWBORDER, DWBORDER ), aBrkSz );
        aLineNumberWindow.SetPosSizePixel(Point(DWBORDER + aBrkSz.Width() - 1, DWBORDER), aLnSz);
        Size aEWSz(aSz.Width() - nBrkWidth - aLineNumberWindow.GetWidth() - nSBWidth + 2, aSz.Height());
        aEdtWindow.SetPosSizePixel( Point( DWBORDER + aBrkSz.Width() + aLnSz.Width() - 1, DWBORDER ), aEWSz );
    }
    else
    {
        aBrkWindow.SetPosSizePixel( Point( DWBORDER, DWBORDER ), aBrkSz );
        Size aEWSz(aSz.Width() - nBrkWidth - nSBWidth + 2, aSz.Height());
        aEdtWindow.SetPosSizePixel(Point(DWBORDER + aBrkSz.Width() - 1, DWBORDER), aEWSz);
    }

    aEWVScrollBar.SetPosSizePixel( Point( aOutSz.Width() - DWBORDER - nSBWidth, DWBORDER ), Size( nSBWidth, aSz.Height() ) );
}

IMPL_LINK( ComplexEditorWindow, ScrollHdl, ScrollBar *, pCurScrollBar )
{
    if ( aEdtWindow.GetEditView() )
    {
        DBG_ASSERT( pCurScrollBar == &aEWVScrollBar, "Wer scrollt hier ?" );
        long nDiff = aEdtWindow.GetEditView()->GetStartDocPos().Y() - pCurScrollBar->GetThumbPos();
        aEdtWindow.GetEditView()->Scroll( 0, nDiff );
        aBrkWindow.DoScroll( 0, nDiff );
        aLineNumberWindow.DoScroll(0, nDiff);
        aEdtWindow.GetEditView()->ShowCursor(false, true);
        pCurScrollBar->SetThumbPos( aEdtWindow.GetEditView()->GetStartDocPos().Y() );
    }

    return 0;
}

void ComplexEditorWindow::DataChanged(DataChangedEvent const & rDCEvt)
{
    Window::DataChanged(rDCEvt);
    if (rDCEvt.GetType() == DATACHANGED_SETTINGS
        && (rDCEvt.GetFlags() & SETTINGS_STYLE) != 0)
    {
        Color aColor(GetSettings().GetStyleSettings().GetFaceColor());
        const AllSettings* pOldSettings = rDCEvt.GetOldSettings();
        if (!pOldSettings || aColor != pOldSettings->GetStyleSettings().GetFaceColor())
        {
            SetBackground(Wallpaper(aColor));
            Invalidate();
        }
    }
}

void ComplexEditorWindow::SetLineNumberDisplay(bool b)
{
    aLineNumberWindow.Show(b);
    Resize();
}

uno::Reference< awt::XWindowPeer >
EditorWindow::GetComponentInterface(sal_Bool bCreate)
{
    uno::Reference< awt::XWindowPeer > xPeer(
        Window::GetComponentInterface(false));
    if (!xPeer.is() && bCreate)
    {
        // Make sure edit engine and view are available:
        if (!pEditEngine)
            CreateEditEngine();

        xPeer = new ::svt::TextWindowPeer(*GetEditView());
        SetComponentInterface(xPeer);
    }
    return xPeer;
}


//
// WatchTreeListBox
// ================
//

WatchTreeListBox::WatchTreeListBox( Window* pParent, WinBits nWinBits )
    : SvHeaderTabListBox( pParent, nWinBits )
{}

WatchTreeListBox::~WatchTreeListBox()
{
    // Destroy user data
    SvTreeListEntry* pEntry = First();
    while ( pEntry )
    {
        delete (WatchItem*)pEntry->GetUserData();
        pEntry = Next( pEntry );
    }
}

void WatchTreeListBox::SetTabs()
{
    SvHeaderTabListBox::SetTabs();
    sal_uInt16 nTabCount_ = aTabs.size();
    for( sal_uInt16 i = 0 ; i < nTabCount_ ; i++ )
    {
        SvLBoxTab* pTab = aTabs[i];
        if( i == 2 )
            pTab->nFlags |= SV_LBOXTAB_EDITABLE;
        else
            pTab->nFlags &= ~SV_LBOXTAB_EDITABLE;
    }
}

void WatchTreeListBox::RequestingChildren( SvTreeListEntry * pParent )
{
    if( !StarBASIC::IsRunning() )
        return;

    if( GetChildCount( pParent ) > 0 )
        return;

    SvTreeListEntry* pEntry = pParent;
    WatchItem* pItem = (WatchItem*)pEntry->GetUserData();

    SbxDimArray* pArray = pItem->mpArray;
    SbxDimArray* pRootArray = pItem->GetRootArray();
    bool bArrayIsRootArray = false;
    if( !pArray && pRootArray )
    {
        pArray = pRootArray;
        bArrayIsRootArray = true;
    }

    SbxObject* pObj = pItem->mpObject;
    if( pObj )
    {
        createAllObjectProperties( pObj );
        SbxArray* pProps = pObj->GetProperties();
        sal_uInt16 nPropCount = pProps->Count();
        pItem->maMemberList.reserve(nPropCount);

        for( sal_uInt16 i = 0 ; i < nPropCount - 3 ; i++ )
        {
            SbxVariable* pVar = pProps->Get( i );

            pItem->maMemberList.push_back(String(pVar->GetName()));
            String const& rName = pItem->maMemberList.back();
            SvTreeListEntry* pChildEntry = SvTreeListBox::InsertEntry( rName, pEntry );
            pChildEntry->SetUserData(new WatchItem(rName));
        }
        if( nPropCount > 0 )
        {
            UpdateWatches();
        }
    }
    else if( pArray )
    {
        sal_uInt16 nElementCount = 0;

        // Loop through indices of current level
        int nParentLevel = bArrayIsRootArray ? pItem->nDimLevel : 0;
        int nThisLevel = nParentLevel + 1;
        sal_Int32 nMin, nMax;
        pArray->GetDim32( nThisLevel, nMin, nMax );
        for( sal_Int32 i = nMin ; i <= nMax ; i++ )
        {
            WatchItem* pChildItem = new WatchItem(pItem->maName);

            // Copy data and create name

            OUString aIndexStr = "(";
            pChildItem->mpArrayParentItem = pItem;
            pChildItem->nDimLevel = nThisLevel;
            pChildItem->nDimCount = pItem->nDimCount;
            pChildItem->vIndices.resize(pChildItem->nDimCount);
            sal_Int32 j;
            for( j = 0 ; j < nParentLevel ; j++ )
            {
                short n = pChildItem->vIndices[j] = pItem->vIndices[j];
                aIndexStr += OUString::number( n ) + ",";
            }
            pChildItem->vIndices[nParentLevel] = sal::static_int_cast<short>( i );
            aIndexStr += OUString::number( i ) + ")";

            OUString aDisplayName;
            WatchItem* pArrayRootItem = pChildItem->GetRootItem();
            if( pArrayRootItem && pArrayRootItem->mpArrayParentItem )
                aDisplayName = pItem->maDisplayName;
            else
                aDisplayName = pItem->maName;
            aDisplayName += aIndexStr;
            pChildItem->maDisplayName = aDisplayName;

            SvTreeListEntry* pChildEntry = SvTreeListBox::InsertEntry( aDisplayName, pEntry );
            nElementCount++;
            pChildEntry->SetUserData( pChildItem );
        }
        if( nElementCount > 0 )
        {
            UpdateWatches();
        }
    }
}

SbxBase* WatchTreeListBox::ImplGetSBXForEntry( SvTreeListEntry* pEntry, bool& rbArrayElement )
{
    SbxBase* pSBX = NULL;
    rbArrayElement = false;

    WatchItem* pItem = (WatchItem*)pEntry->GetUserData();
    String aVName( pItem->maName );

    SvTreeListEntry* pParentEntry = GetParent( pEntry );
    WatchItem* pParentItem = pParentEntry ? (WatchItem*)pParentEntry->GetUserData() : NULL;
    if( pParentItem )
    {
        SbxObject* pObj = pParentItem->mpObject;
        SbxDimArray* pArray;
        if( pObj )
        {
            pSBX = pObj->Find( aVName, SbxCLASS_DONTCARE );
            if (SbxVariable const* pVar = IsSbxVariable(pSBX))
            {
                // Force getting value
                SbxValues aRes;
                aRes.eType = SbxVOID;
                pVar->Get( aRes );
            }
        }
        // Array?
        else if( (pArray = pItem->GetRootArray()) != NULL )
        {
            rbArrayElement = true;
            if( pParentItem->nDimLevel + 1 == pParentItem->nDimCount )
                pSBX = pArray->Get(pItem->vIndices.empty() ? 0 : &*pItem->vIndices.begin());
        }
    }
    else
    {
        pSBX = StarBASIC::FindSBXInCurrentScope( aVName );
    }
    return pSBX;
}

sal_Bool WatchTreeListBox::EditingEntry( SvTreeListEntry* pEntry, Selection& )
{
    WatchItem* pItem = (WatchItem*)pEntry->GetUserData();

    bool bEdit = false;
    if ( StarBASIC::IsRunning() && StarBASIC::GetActiveMethod() && !SbxBase::IsError() )
    {
        // No out of scope entries
        bool bArrayElement;
        SbxBase* pSbx = ImplGetSBXForEntry( pEntry, bArrayElement );
        if (IsSbxVariable(pSbx) || bArrayElement)
        {
            // Accept no objects and only end nodes of arrays for editing
            if( !pItem->mpObject && (pItem->mpArray == NULL || pItem->nDimLevel == pItem->nDimCount) )
            {
                aEditingRes = SvHeaderTabListBox::GetEntryText( pEntry, ITEM_ID_VALUE-1 );
                aEditingRes = comphelper::string::strip(aEditingRes, ' ');
                bEdit = true;
            }
        }
    }

    return bEdit;
}

sal_Bool WatchTreeListBox::EditedEntry( SvTreeListEntry* pEntry, const OUString& rNewText )
{
    String aResult = comphelper::string::strip(rNewText, ' ');

    sal_uInt16 nResultLen = aResult.Len();
    sal_Unicode cFirst = aResult.GetChar( 0 );
    sal_Unicode cLast  = aResult.GetChar( nResultLen - 1 );
    if( cFirst == '\"' && cLast == '\"' )
        aResult = aResult.Copy( 1, nResultLen - 2 );

    return aResult != aEditingRes && ImplBasicEntryEdited(pEntry, aResult);
}

bool WatchTreeListBox::ImplBasicEntryEdited( SvTreeListEntry* pEntry, const OUString& rResult )
{
    bool bArrayElement;
    SbxBase* pSBX = ImplGetSBXForEntry( pEntry, bArrayElement );

    if (SbxVariable* pVar = IsSbxVariable(pSBX))
    {
        SbxDataType eType = pVar->GetType();
        if ( (sal_uInt8)eType != (sal_uInt8)SbxOBJECT
             && ( eType & SbxARRAY ) == 0 )
        {
            // If the type is variable, the conversion of the SBX does not matter,
            // else the string is converted.
            pVar->PutStringExt( rResult );
        }
    }

    if ( SbxBase::IsError() )
    {
        SbxBase::ResetError();
    }

    UpdateWatches();

    // The text should never be taken/copied 1:1,
    // as the UpdateWatches will be lost
    return false;
}


namespace
{

void implCollapseModifiedObjectEntry( SvTreeListEntry* pParent, WatchTreeListBox* pThis )
{
    pThis->Collapse( pParent );

    SvTreeList* pModel = pThis->GetModel();
    SvTreeListEntry* pDeleteEntry;
    while( (pDeleteEntry = pThis->SvTreeListBox::GetEntry( pParent, 0 )) != NULL )
    {
        implCollapseModifiedObjectEntry( pDeleteEntry, pThis );

        delete (WatchItem*)pDeleteEntry->GetUserData();
        pModel->Remove( pDeleteEntry );
    }
}

String implCreateTypeStringForDimArray( WatchItem* pItem, SbxDataType eType )
{
    OUString aRetStr = getBasicTypeName( eType );

    SbxDimArray* pArray = pItem->mpArray;
    if( !pArray )
        pArray = pItem->GetRootArray();
    if( pArray )
    {
        int nDimLevel = pItem->nDimLevel;
        int nDims = pItem->nDimCount;
        if( nDimLevel < nDims )
        {
            aRetStr += "(";
            for( int i = nDimLevel ; i < nDims ; i++ )
            {
                short nMin, nMax;
                pArray->GetDim( sal::static_int_cast<short>( i+1 ), nMin, nMax );
                aRetStr += OUString::number(nMin) + " to "  + OUString::number(nMax);
                if( i < nDims - 1 )
                    aRetStr += ", ";
            }
            aRetStr += ")";
        }
    }
    return aRetStr;
}

void implEnableChildren( SvTreeListEntry* pEntry, bool bEnable )
{
    if( bEnable )
    {
        pEntry->SetFlags(
            (pEntry->GetFlags() &
            ~(SV_ENTRYFLAG_NO_NODEBMP | SV_ENTRYFLAG_HAD_CHILDREN))
            | SV_ENTRYFLAG_CHILDREN_ON_DEMAND );
    }
    else
    {
        pEntry->SetFlags(
            (pEntry->GetFlags() & ~(SV_ENTRYFLAG_CHILDREN_ON_DEMAND)) );
    }
}

} // namespace

void WatchTreeListBox::UpdateWatches( bool bBasicStopped )
{
    SbMethod* pCurMethod = StarBASIC::GetActiveMethod();

    SbxError eOld = SbxBase::GetError();
    setBasicWatchMode( true );

    SvTreeListEntry* pEntry = First();
    while ( pEntry )
    {
        WatchItem* pItem = (WatchItem*)pEntry->GetUserData();
        String aVName( pItem->maName );
        DBG_ASSERT( aVName.Len(), "Var? - Must not be empty!" );
        String aWatchStr;
        String aTypeStr;
        if ( pCurMethod )
        {
            bool bArrayElement;
            SbxBase* pSBX = ImplGetSBXForEntry( pEntry, bArrayElement );

            // Array? If no end node create type string
            if( bArrayElement && pItem->nDimLevel < pItem->nDimCount )
            {
                SbxDimArray* pRootArray = pItem->GetRootArray();
                SbxDataType eType = pRootArray->GetType();
                aTypeStr = implCreateTypeStringForDimArray( pItem, eType );
                implEnableChildren( pEntry, true );
            }

            bool bCollapse = false;
            if (SbxVariable const* pVar = IsSbxVariable(pSBX))
            {
                // extra treatment of arrays
                SbxDataType eType = pVar->GetType();
                if ( eType & SbxARRAY )
                {
                    // consider multidimensinal arrays!
                    if (SbxDimArray* pNewArray = dynamic_cast<SbxDimArray*>(pVar->GetObject()))
                    {
                        SbxDimArray* pOldArray = pItem->mpArray;

                        bool bArrayChanged = false;
                        if( pNewArray != NULL && pOldArray != NULL )
                        {
                            // Compare Array dimensions to see if array has changed
                            // Can be a copy, so comparing pointers does not work
                            sal_uInt16 nOldDims = pOldArray->GetDims();
                            sal_uInt16 nNewDims = pNewArray->GetDims();
                            if( nOldDims != nNewDims )
                            {
                                bArrayChanged = true;
                            }
                            else
                            {
                                for( int i = 0 ; i < nOldDims ; i++ )
                                {
                                    short nOldMin, nOldMax;
                                    short nNewMin, nNewMax;

                                    pOldArray->GetDim( sal::static_int_cast<short>( i+1 ), nOldMin, nOldMax );
                                    pNewArray->GetDim( sal::static_int_cast<short>( i+1 ), nNewMin, nNewMax );
                                    if( nOldMin != nNewMin || nOldMax != nNewMax )
                                    {
                                        bArrayChanged = true;
                                        break;
                                    }
                                }
                            }
                        }
                        else if( pNewArray == NULL || pOldArray == NULL )
                        {
                            bArrayChanged = true;
                        }
                        if( pNewArray )
                        {
                            implEnableChildren( pEntry, true );
                        }
                        // #i37227 Clear always and replace array
                        if( pNewArray != pOldArray )
                        {
                            pItem->clearWatchItem();
                            if( pNewArray )
                            {
                                implEnableChildren( pEntry, true );

                                pItem->mpArray = pNewArray;
                                sal_uInt16 nDims = pNewArray->GetDims();
                                pItem->nDimLevel = 0;
                                pItem->nDimCount = nDims;
                            }
                        }
                        if( bArrayChanged && pOldArray != NULL )
                        {
                            bCollapse = true;
                        }
                        aTypeStr = implCreateTypeStringForDimArray( pItem, eType );
                    }
                    else
                    {
                        aWatchStr += "<?>";
                    }
                }
                else if ( (sal_uInt8)eType == (sal_uInt8)SbxOBJECT )
                {
                    if (SbxObject* pObj = dynamic_cast<SbxObject*>(pVar->GetObject()))
                    {
                        // Check if member list has changed
                        bool bObjChanged = false;
                        if (pItem->mpObject && !pItem->maMemberList.empty())
                        {
                            SbxArray* pProps = pObj->GetProperties();
                            sal_uInt16 nPropCount = pProps->Count();
                            for( sal_uInt16 i = 0 ; i < nPropCount - 3 ; i++ )
                            {
                                SbxVariable* pVar_ = pProps->Get( i );
                                String aName( pVar_->GetName() );
                                if( pItem->maMemberList[i] != aName )
                                {
                                    bObjChanged = true;
                                    break;
                                }
                            }
                            if( bObjChanged )
                            {
                                bCollapse = true;
                            }
                        }

                        pItem->mpObject = pObj;
                        implEnableChildren( pEntry, true );
                        aTypeStr = getBasicObjectTypeName( pObj );
                    }
                    else
                    {
                        aWatchStr = OUString( "Null" );
                        if( pItem->mpObject != NULL )
                        {
                            bCollapse = true;
                            pItem->clearWatchItem();

                            implEnableChildren( pEntry, false );
                        }
                    }
                }
                else
                {
                    if( pItem->mpObject != NULL )
                    {
                        bCollapse = true;
                        pItem->clearWatchItem();

                        implEnableChildren( pEntry, false );
                    }

                    bool bString = ((sal_uInt8)eType == (sal_uInt8)SbxSTRING);
                    OUString aStrStr( "\"" );
                    if( bString )
                    {
                        aWatchStr += aStrStr;
                    }
                    aWatchStr += pVar->GetOUString();
                    if( bString )
                    {
                        aWatchStr += aStrStr;
                    }
                }
                if( !aTypeStr.Len() )
                {
                    if( !pVar->IsFixed() )
                    {
                        aTypeStr = OUString( "Variant/" );
                    }
                    aTypeStr += getBasicTypeName( pVar->GetType() );
                }
            }
            else if( !bArrayElement )
            {
                aWatchStr += "<Out of Scope>";
            }

            if( bCollapse )
            {
                implCollapseModifiedObjectEntry( pEntry, this );
            }

        }
        else if( bBasicStopped )
        {
            if( pItem->mpObject || pItem->mpArray )
            {
                implCollapseModifiedObjectEntry( pEntry, this );
                pItem->mpObject = NULL;
            }
        }

        SvHeaderTabListBox::SetEntryText( aWatchStr, pEntry, ITEM_ID_VALUE-1 );
        SvHeaderTabListBox::SetEntryText( aTypeStr, pEntry, ITEM_ID_TYPE-1 );

        pEntry = Next( pEntry );
    }

    // Force redraw
    Invalidate();

    SbxBase::ResetError();
    if( eOld != SbxERR_OK )
        SbxBase::SetError( eOld );
    setBasicWatchMode( false );
}

CodeCompleteListBox::CodeCompleteListBox( CodeCompleteWindow* pPar )
: ListBox(pPar, WB_SORT | WB_BORDER ),
pCodeCompleteWindow( pPar )
{
    SetDoubleClickHdl(LINK(this, CodeCompleteListBox, ImplDoubleClickHdl));
    SetSelectHdl(LINK(this, CodeCompleteListBox, ImplSelectHdl));
}

IMPL_LINK_NOARG(CodeCompleteListBox, ImplDoubleClickHdl)
{
    InsertSelectedEntry();
    return 0;
}

IMPL_LINK_NOARG(CodeCompleteListBox, ImplSelectHdl)
{//give back the focus to the parent
    pCodeCompleteWindow->pParent->GrabFocus();
    return 0;
}

void CodeCompleteListBox::InsertSelectedEntry()
{
    if( !aFuncBuffer.toString().isEmpty() )
    {
        // if the user typed in something: remove, and insert
        TextPaM aEnd(pCodeCompleteWindow->aTextSelection.GetEnd().GetPara(), pCodeCompleteWindow->GetTextSelection().GetEnd().GetIndex() + aFuncBuffer.getLength());
        TextPaM aStart(pCodeCompleteWindow->aTextSelection.GetEnd().GetPara(), pCodeCompleteWindow->GetTextSelection().GetEnd().GetIndex() );
        pCodeCompleteWindow->pParent->GetEditView()->SetSelection(TextSelection(aStart, aEnd));
        pCodeCompleteWindow->pParent->GetEditView()->DeleteSelected();

        if( !((OUString) GetEntry( GetSelectEntryPos() )).isEmpty() )
        {//if the user selected something
            pCodeCompleteWindow->pParent->GetEditView()->InsertText( (OUString) GetEntry(GetSelectEntryPos()), sal_True );
            HideAndRestoreFocus();
        }
        else
        {
            HideAndRestoreFocus();
        }
    }
    else
    {
        if( !((OUString) GetEntry( GetSelectEntryPos() )).isEmpty() )
        {//if the user selected something
            pCodeCompleteWindow->pParent->GetEditView()->InsertText( (OUString) GetEntry(GetSelectEntryPos()), sal_True );
            HideAndRestoreFocus();
        }
    }
}

void CodeCompleteListBox::SetMatchingEntries()
{
    for(sal_uInt16 i=0; i< GetEntryCount(); ++i)
    {
        OUString sEntry = (OUString) GetEntry(i);
        if( sEntry.startsWithIgnoreAsciiCase( aFuncBuffer.toString() ) )
        {
            SelectEntry(sEntry);
            break;
        }
    }
}

void CodeCompleteListBox::KeyInput( const KeyEvent& rKeyEvt )
{
    sal_Unicode aChar = rKeyEvt.GetKeyCode().GetCode();
    if( ( aChar >= KEY_A ) && ( aChar <= KEY_Z ) )
    {
        aFuncBuffer.append(rKeyEvt.GetCharCode());
        SetMatchingEntries();
    }
    else
    {
        switch( aChar )
        {
            case KEY_ESCAPE: // hide, do nothing
                HideAndRestoreFocus();
                break;
            case KEY_TAB: case KEY_SPACE:
                HideAndRestoreFocus();
                break;
            case KEY_BACKSPACE: case KEY_DELETE:
                if( aFuncBuffer.toString() != OUString("") )
                {
                    aFuncBuffer = aFuncBuffer.remove(aFuncBuffer.getLength()-1, 1);
                    SetMatchingEntries();
                }
                else
                {
                    pCodeCompleteWindow->ClearAndHide();
                }
                break;
            case KEY_RETURN:
                InsertSelectedEntry();
                break;
            case KEY_UP: case KEY_DOWN:
                NotifyEvent nEvt( EVENT_KEYINPUT, NULL, &rKeyEvt );
                PreNotify(nEvt);
                break;
        }
    }
    ListBox::KeyInput(rKeyEvt);
}

void CodeCompleteListBox::HideAndRestoreFocus()
{
    pCodeCompleteWindow->Hide();
    pCodeCompleteWindow->pParent->GetEditView()->SetSelection( pCodeCompleteWindow->pParent->GetEditView()->CursorEndOfLine(pCodeCompleteWindow->GetTextSelection().GetStart()) );
    pCodeCompleteWindow->pParent->GrabFocus();
}

CodeCompleteWindow::CodeCompleteWindow( EditorWindow* pPar )
: Window( pPar ),
pParent( pPar ),
pListBox( new CodeCompleteListBox(this) )
{
    SetSizePixel( Size(151,151) ); //default, later it changes
    InitListBox();
}

void CodeCompleteWindow::InitListBox()
{
    pListBox->SetSizePixel( Size(150,150) ); //default, this will adopt the line length
    pListBox->Show();
    pListBox->GrabFocus();
    pListBox->EnableQuickSelection( false );
}

void CodeCompleteWindow::InsertEntry( const OUString& aStr )
{
    pListBox->InsertEntry( aStr );
}

void CodeCompleteWindow::ClearListBox()
{
    pListBox->Clear();
    pListBox->aFuncBuffer.makeStringAndClear();
}

void CodeCompleteWindow::SetTextSelection( const TextSelection& aSel )
{
    aTextSelection = aSel;
}

const TextSelection& CodeCompleteWindow::GetTextSelection() const
{
    return aTextSelection;
}

void CodeCompleteWindow::ResizeListBox()
{
    if( pListBox->GetEntryCount() > 0 )
    {// if there is at least one element inside
        OUString aLongestEntry = pListBox->GetEntry( 0 );//grab the longest one: max search
        if( pListBox->GetEntryCount() > 0 )
        {
            for( sal_uInt16 i=1; i< pListBox->GetEntryCount(); ++i )
            {
                if( ((OUString) pListBox->GetEntry( i )).getLength() > aLongestEntry.getLength() )
                    aLongestEntry = pListBox->GetEntry( i );
            }
        }
        //long nWidth = GetTextWidth(aLongestEntry);
        sal_uInt16 nColumns = aLongestEntry.getLength();
        sal_uInt16 nLines = std::min( (sal_uInt16) 6, pListBox->GetEntryCount() );
        const Font& aFont = pListBox->GetUnzoomedControlPointFont();

        Rectangle aVisArea( pParent->GetEditView()->GetStartDocPos(), pParent->GetOutputSizePixel() );
        Size aSize = pListBox->GetOptimalSize();//this sets the correct width
        aSize.setHeight( pListBox->CalcSize( nColumns, nLines ).getHeight() );

        Point aBottomPoint = aVisArea.BottomRight();
        Point aTopPoint = aVisArea.TopRight();
        long nYDiff = std::abs((aBottomPoint.Y() - aTopPoint.Y()) - GetPosPixel().Y());

        if( (nYDiff + aFont.GetSize().getHeight()) < aSize.Height() )
        {//bottom part is clipped, fix the visibility by placing it over the line (not under)
            Point aPos = GetPosPixel();
            Font aParFont = pParent->GetEditEngine()->GetFont();
            aPos.Y() = aPos.Y() - (aSize.getHeight() + aParFont.GetSize().getHeight()+5);
            SetPosPixel(aPos);
        }
        long nXDiff = std::abs(aTopPoint.X() - GetPosPixel().X());
        if( nXDiff < aSize.Width() )
        {//clipped at the right side, move it a bit left
            Point aPos = GetPosPixel();
            aPos.X() = aPos.X() - aSize.Width() + nXDiff;
            SetPosPixel(aPos);
        }
        pListBox->SetSizePixel( aSize );
        aSize.setWidth( aSize.getWidth() + 1 );
        aSize.setHeight( aSize.getHeight() + 1 );
        SetSizePixel( aSize );
        pListBox->GrabFocus();
    }
}

void CodeCompleteWindow::SelectFirstEntry()
{
    if( pListBox->GetEntryCount() > 0 )
    {
         pListBox->SelectEntryPos( 0 );
    }
}

void CodeCompleteWindow::ClearAndHide()
{
    TextPaM aEnd(aTextSelection.GetEnd().GetPara(), GetTextSelection().GetEnd().GetIndex() + pListBox->aFuncBuffer.getLength());
    TextPaM aStart(aTextSelection.GetEnd().GetPara(), GetTextSelection().GetEnd().GetIndex() );
    pParent->GetEditView()->SetSelection(TextSelection(aStart, aEnd));
    pParent->GetEditView()->DeleteSelected();
    Hide();
    ClearListBox();
    pParent->GrabFocus();
}

UnoTypeCodeCompletetor::UnoTypeCodeCompletetor( const std::vector< OUString >& aVect, const OUString& sVarType )
: bCanComplete( true )
{
    if( aVect.size() == 0 || sVarType.isEmpty() )
    {
        bCanComplete = false;//invalid parameters, nothing to code complete
        return;
    }

    try
    {
        xFactory = Reference< lang::XMultiServiceFactory >( comphelper::getProcessServiceFactory(), UNO_QUERY_THROW );
        xRefl = Reference< reflection::XIdlReflection >( xFactory->createInstance("com.sun.star.reflection.CoreReflection"), UNO_QUERY_THROW );
        if( xRefl.is() )
            xClass = xRefl->forName( sVarType );//get the base class for reflection
    }
    catch( const Exception& ex)
    {
        bCanComplete = false;
        return;
    }

    unsigned int j = 1;//start from aVect[1]: aVect[0] is the variable name
    OUString sMethName;
    while( j != aVect.size() )
    {
        sMethName = aVect[j];

        if( !CheckField(sMethName) )//check field
            break;
        else
        {
            if( CodeCompleteOptions::IsExtendedTypeDeclaration() )
            {// if extended types on, check methods
                if( !CheckMethod(sMethName) )
                {
                    bCanComplete = false;
                    break;
                }
            }
            bCanComplete = false;
            break;
        }
        j++;
    }
}

std::vector< OUString > UnoTypeCodeCompletetor::GetXIdlClassMethods() const
{

    std::vector< OUString > aRetVect;
    if( bCanComplete )
    {
        Sequence< Reference< reflection::XIdlMethod > > aMethods = xClass->getMethods();
        if( aMethods.getLength() != 0 )
        {
            for(sal_Int32 l = 0; l < aMethods.getLength(); ++l)
            {
                aRetVect.push_back(OUString(aMethods[l]->getName()));
            }
        }
    }
    return aRetVect;//this is empty when cannot code complete
}

std::vector< OUString > UnoTypeCodeCompletetor::GetXIdlClassFields() const
{

    std::vector< OUString > aRetVect;
    if( bCanComplete )
    {
        Sequence< Reference< reflection::XIdlField > > aFields = xClass->getFields();
        if( aFields.getLength() != 0 )
        {
            for(sal_Int32 l = 0; l < aFields.getLength(); ++l)
            {
                aRetVect.push_back(OUString(aFields[l]->getName()));
            }
        }
    }
    return aRetVect;//this is empty when cannot code complete
}

bool UnoTypeCodeCompletetor::CanCodeComplete() const
{
    return bCanComplete;
}

bool UnoTypeCodeCompletetor::CheckField( const OUString& sFieldName )
{
    Reference< reflection::XIdlField> xField = xClass->getField( sFieldName );
    if( xField != NULL )
    {
        xClass = xField->getType();
        if( xClass == NULL )
        {
            return true;
        }
    }
    return false;
}

bool UnoTypeCodeCompletetor::CheckMethod( const OUString& sMethName )
{
    Reference< reflection::XIdlMethod> xMethod = xClass->getMethod( sMethName );
    if( xMethod != NULL ) //method OK
    {
        xClass = xMethod->getReturnType();
        if( xClass == NULL )
        {
            return true;
        }
    }
    return false;
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
