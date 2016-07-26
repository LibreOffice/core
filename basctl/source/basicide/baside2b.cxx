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

#include <sal/config.h>

#include <cassert>

#include "helpid.hrc"
#include "baside2.hrc"

#include "baside2.hxx"
#include "brkdlg.hxx"
#include "iderdll.hxx"

#include <basic/sbmeth.hxx>
#include <basic/sbuno.hxx>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XPropertiesChangeListener.hpp>
#include <com/sun/star/script/XLibraryContainer2.hpp>
#include <comphelper/string.hxx>
#include <officecfg/Office/Common.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/msgbox.hxx>
#include <svl/urihelper.hxx>
#include <vcl/xtextedt.hxx>
#include <vcl/txtattr.hxx>
#include <vcl/settings.hxx>
#include <svtools/textwindowpeer.hxx>
#include <svtools/treelistentry.hxx>
#include <vcl/taskpanelist.hxx>
#include <vcl/help.hxx>
#include <cppuhelper/implbase.hxx>
#include <vector>
#include <com/sun/star/reflection/theCoreReflection.hpp>

namespace basctl
{

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace
{

sal_uInt16 const NoMarker = 0xFFFF;
long const nBasePad = 2;
long const nCursorPad = 5;

long nVirtToolBoxHeight;    // inited in WatchWindow, used in Stackwindow
long nHeaderBarHeight;

// Returns pBase converted to SbxVariable if valid and is not an SbxMethod.
SbxVariable* IsSbxVariable (SbxBase* pBase)
{
    if (SbxVariable* pVar = dynamic_cast<SbxVariable*>(pBase))
        if (!dynamic_cast<SbxMethod*>(pVar))
            return pVar;
    return nullptr;
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
    sal_Size nSize = aMemStream.Tell();
    OUString aText( static_cast<const sal_Char*>(aMemStream.GetData()),
        nSize, RTL_TEXTENCODING_UTF8 );
    return aText;
}

void setTextEngineText (ExtTextEngine& rEngine, OUString const& aStr)
{
    rEngine.SetText(OUString());
    OString aUTF8Str = OUStringToOString( aStr, RTL_TEXTENCODING_UTF8 );
    SvMemoryStream aMemStream( const_cast<char *>(aUTF8Str.getStr()), aUTF8Str.getLength(),
        StreamMode::READ );
    aMemStream.SetStreamCharSet( RTL_TEXTENCODING_UTF8 );
    aMemStream.SetLineDelimiter( LINEEND_LF );
    rEngine.Read(aMemStream);
}

namespace
{

void lcl_DrawIDEWindowFrame(DockingWindow* pWin, vcl::RenderContext& rRenderContext)
{
    if (pWin->IsFloatingMode())
        return;

    Size aSz(pWin->GetOutputSizePixel());
    const Color aOldLineColor(rRenderContext.GetLineColor());
    rRenderContext.SetLineColor(Color(COL_WHITE));
    // White line on top
    rRenderContext.DrawLine(Point(0, 0), Point(aSz.Width(), 0));
    // Black line at bottom
    rRenderContext.SetLineColor(Color(COL_BLACK));
    rRenderContext.DrawLine(Point(0, aSz.Height() - 1),
                            Point(aSz.Width(), aSz.Height() - 1));
    rRenderContext.SetLineColor(aOldLineColor);
}

void lcl_SeparateNameAndIndex( const OUString& rVName, OUString& rVar, OUString& rIndex )
{
    rVar = rVName;
    rIndex.clear();
    sal_Int32 nIndexStart = rVar.indexOf( '(' );
    if ( nIndexStart != -1 )
    {
        sal_Int32 nIndexEnd = rVar.indexOf( ')', nIndexStart );
        if ( nIndexStart != -1 )
        {
            rIndex = rVar.copy( nIndexStart+1, nIndexEnd-nIndexStart-1 );
            rVar = rVar.copy( 0, nIndexStart );
            rVar = comphelper::string::stripEnd(rVar, ' ');
            rIndex = comphelper::string::strip(rIndex, ' ');
        }
    }

    if ( !rVar.isEmpty() )
    {
        sal_uInt16 nLastChar = rVar.getLength()-1;
        if ( strchr( cSuffixes, rVar[ nLastChar ] ) )
            rVar = rVar.replaceAt( nLastChar, 1, "" );
    }
    if ( !rIndex.isEmpty() )
    {
        sal_uInt16 nLastChar = rIndex.getLength()-1;
        if ( strchr( cSuffixes, rIndex[ nLastChar ] ) )
            rIndex = rIndex.replaceAt( nLastChar, 1, "" );
    }
}

} // namespace


// EditorWindow


class EditorWindow::ChangesListener:
    public cppu::WeakImplHelper< beans::XPropertiesChangeListener >
{
public:
    explicit ChangesListener(EditorWindow & editor): editor_(editor) {}

private:
    virtual ~ChangesListener() {}

    virtual void SAL_CALL disposing(lang::EventObject const &) throw (RuntimeException, std::exception) override
    {
        osl::MutexGuard g(editor_.mutex_);
        editor_.notifier_.clear();
    }

    virtual void SAL_CALL propertiesChange(
        Sequence< beans::PropertyChangeEvent > const &) throw (RuntimeException, std::exception) override
    {
        SolarMutexGuard g;
        editor_.ImplSetFont();
    }

    EditorWindow & editor_;
};

class EditorWindow::ProgressInfo : public SfxProgress
{
public:
    ProgressInfo (SfxObjectShell* pObjSh, OUString const& rText, sal_uLong nRange) :
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

EditorWindow::EditorWindow (vcl::Window* pParent, ModulWindow* pModulWindow) :
    Window(pParent, WB_BORDER),
    rModulWindow(*pModulWindow),
    nCurTextWidth(0),
    aHighlighter(HighlighterLanguage::Basic),
    bHighlighting(false),
    bDoSyntaxHighlight(true),
    bDelayHighlight(true),
    pCodeCompleteWnd(VclPtr<CodeCompleteWindow>::Create(this))
{
    SetBackground(Wallpaper(GetSettings().GetStyleSettings().GetFieldColor()));
    SetPointer( Pointer( PointerStyle::Text ) );
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
    s[0] = "FontHeight";
    s[1] = "FontName";
    n->addPropertiesChangeListener(s, listener_.get());
}


EditorWindow::~EditorWindow()
{
    disposeOnce();
}

void EditorWindow::dispose()
{
    Reference< beans::XMultiPropertySet > n;
    {
        osl::MutexGuard g(mutex_);
        n = notifier_;
    }
    if (n.is()) {
        n->removePropertiesChangeListener(listener_.get());
    }

    aSyntaxIdle.Stop();

    if ( pEditEngine )
    {
        EndListening( *pEditEngine );
        pEditEngine->RemoveView(pEditView.get());
    }
    pCodeCompleteWnd.disposeAndClear();
    vcl::Window::dispose();
}

OUString EditorWindow::GetWordAtCursor()
{
    OUString aWord;

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
                if ( aURLObj.GetProtocol() == INetProtocol::VndSunStarHelp
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
            if ( aWord.isEmpty() )
                aWord = pTextEngine->GetWord( rSelEnd );

            // Can be empty when full word selected, as Cursor behind it
            if ( aWord.isEmpty() && pEditView->HasSelection() )
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
        if ( rHEvt.GetMode() & HelpEventMode::CONTEXT )
        {
            OUString aKeyword = GetWordAtCursor();
            Application::GetHelp()->SearchKeyword( aKeyword );
            bDone = true;
        }
        else if ( rHEvt.GetMode() & HelpEventMode::QUICK )
        {
            OUString aHelpText;
            Point aTopLeft;
            if ( StarBASIC::IsRunning() )
            {
                Point aWindowPos = rHEvt.GetMousePosPixel();
                aWindowPos = ScreenToOutputPixel( aWindowPos );
                Point aDocPos = GetEditView()->GetDocPos( aWindowPos );
                TextPaM aCursor = GetEditView()->GetTextEngine()->GetPaM(aDocPos, false);
                TextPaM aStartOfWord;
                OUString aWord = GetEditView()->GetTextEngine()->GetWord( aCursor, &aStartOfWord );
                if ( !aWord.isEmpty() && !comphelper::string::isdigitAsciiString(aWord) )
                {
                    sal_uInt16 nLastChar = aWord.getLength() - 1;
                    if ( strchr( cSuffixes, aWord[ nLastChar ] ) )
                        aWord = aWord.replaceAt( nLastChar, 1, "" );
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
                            if ( aHelpText.isEmpty() )     // name is not copied with the passed parameters
                                aHelpText = aWord;
                            aHelpText += "=" + pVar->GetOUString();
                        }
                    }
                    if ( !aHelpText.isEmpty() )
                    {
                        aTopLeft = GetEditView()->GetTextEngine()->PaMtoEditCursor( aStartOfWord ).BottomLeft();
                        aTopLeft = GetEditView()->GetWindowPos( aTopLeft );
                        aTopLeft.X() += 5;
                        aTopLeft.Y() += 5;
                        aTopLeft = OutputToScreenPixel( aTopLeft );
                    }
                }
            }
            Help::ShowQuickHelp( this, Rectangle( aTopLeft, Size( 1, 1 ) ), aHelpText, QuickHelpFlags::Top|QuickHelpFlags::Left);
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
            pBindings->Invalidate( SID_BASICIDE_STAT_TITLE );
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
        if ( ( rCEvt.GetCommand() == CommandEventId::Wheel ) ||
             ( rCEvt.GetCommand() == CommandEventId::StartAutoScroll ) ||
             ( rCEvt.GetCommand() == CommandEventId::AutoScroll ) )
        {
            HandleScrollCommand( rCEvt, rModulWindow.GetHScrollBar(), &rModulWindow.GetEditVScrollBar() );
        } else if ( rCEvt.GetCommand() == CommandEventId::ContextMenu ) {
            SfxDispatcher* pDispatcher = GetDispatcher();
            if ( pDispatcher )
            {
                SfxDispatcher::ExecutePopup();
            }
            if( pCodeCompleteWnd->IsVisible() ) // hide the code complete window
                pCodeCompleteWnd->ClearAndHide();
        }
    }
}

bool EditorWindow::ImpCanModify()
{
    bool bCanModify = true;
    if ( StarBASIC::IsRunning() && rModulWindow.GetBasicStatus().bIsRunning )
    {
        // If in Trace-mode, abort the trace or refuse input
        // Remove markers in the modules in Notify at Basic::Stoped
        if (ScopedVclPtrInstance<QueryBox>(nullptr, WB_OK_CANCEL, IDEResId(RID_STR_WILLSTOPPRG).toString())->Execute() == RET_OK)
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

    bool const bWasModified = pEditEngine->IsModified();
    // see if there is an accelerator to be processed first
    SfxViewShell *pVS( SfxViewShell::Current());
    bool bDone = pVS && pVS->KeyInput( rKEvt );

    if( pCodeCompleteWnd->IsVisible() && CodeCompleteOptions::IsCodeCompleteOn() )
    {
        pCodeCompleteWnd->GetListBox()->KeyInput(rKEvt);
        if( rKEvt.GetKeyCode().GetCode() == KEY_UP
            || rKEvt.GetKeyCode().GetCode() == KEY_DOWN
            || rKEvt.GetKeyCode().GetCode() == KEY_TAB
            || rKEvt.GetKeyCode().GetCode() == KEY_POINT)
            return;
    }

    if( (rKEvt.GetKeyCode().GetCode() == KEY_SPACE ||
        rKEvt.GetKeyCode().GetCode() == KEY_TAB ||
        rKEvt.GetKeyCode().GetCode() == KEY_RETURN ) && CodeCompleteOptions::IsAutoCorrectOn() )
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
       HandleProcedureCompletion();
    }

    if( rKEvt.GetKeyCode().GetCode() == KEY_POINT && CodeCompleteOptions::IsCodeCompleteOn() )
    {
        HandleCodeCompletion();
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
            pBindings->Invalidate( SID_BASICIDE_STAT_TITLE );
            if ( rKEvt.GetKeyCode().GetGroup() == KEYGROUP_CURSOR )
            {
                pBindings->Update( SID_BASICIDE_STAT_POS );
                pBindings->Update( SID_BASICIDE_STAT_TITLE );
            }
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
    const sal_uInt32 nLine =  aSel.GetStart().GetPara();
    const sal_Int32 nIndex =  aSel.GetStart().GetIndex();
    OUString aLine( pEditEngine->GetText( nLine ) ); // the line being modified
    const OUString& sActSubName = GetActualSubName( nLine ); // the actual procedure

    std::vector<HighlightPortion> aPortions;
    aHighlighter.getHighlightPortions( aLine, aPortions );

    if( aPortions.empty() )
        return;

    HighlightPortion& r = aPortions.back();
    if( static_cast<size_t>(nIndex) != aPortions.size()-1 )
    {//cursor is not standing at the end of the line
        for (std::vector<HighlightPortion>::iterator i(aPortions.begin());
             i != aPortions.end(); ++i)
        {
            if( i->nEnd == nIndex )
            {
                r = *i;
                break;
            }
        }
    }

    OUString sStr = aLine.copy( r.nBegin, r.nEnd - r.nBegin );
    //if WS or empty string: stop, nothing to do
    if( ( r.tokenType == TokenType::Whitespace ) || sStr.isEmpty() )
        return;
    //create the appropriate TextSelection, and update the cache
    TextPaM aStart( nLine, r.nBegin );
    TextPaM aEnd( nLine, r.nBegin + sStr.getLength() );
    TextSelection sTextSelection( aStart, aEnd );
    rModulWindow.UpdateModule();
    rModulWindow.GetSbModule()->GetCodeCompleteDataFromParse( aCodeCompleteCache );
    // correct the last entered keyword
    if( r.tokenType == TokenType::Keywords )
    {
        sStr = sStr.toAsciiLowerCase();
        if( !SbModule::GetKeywordCase(sStr).isEmpty() )
        // if it is a keyword, get its correct case
            sStr = SbModule::GetKeywordCase(sStr);
        else
        // else capitalize first letter/select the correct one, and replace
            sStr = sStr.replaceAt( 0, 1, OUString(sStr[0]).toAsciiUpperCase() );

        pEditEngine->ReplaceText( sTextSelection, sStr );
        pEditView->SetSelection( aSel );
    }
    if( r.tokenType == TokenType::Identifier )
    {// correct variables
        if( !aCodeCompleteCache.GetCorrectCaseVarName( sStr, sActSubName ).isEmpty() )
        {
            sStr = aCodeCompleteCache.GetCorrectCaseVarName( sStr, sActSubName );
            pEditEngine->ReplaceText( sTextSelection, sStr );
            pEditView->SetSelection( aSel );
        }
        else
        {
            //autocorrect procedures
            SbxArray* pArr = rModulWindow.GetSbModule()->GetMethods();
            for( sal_uInt32 i=0; i < pArr->Count32(); ++i )
            {
                if( pArr->Get32(i)->GetName().equalsIgnoreAsciiCase( sStr ) )
                {
                    sStr = pArr->Get32(i)->GetName(); //if found, get the correct case
                    pEditEngine->ReplaceText( sTextSelection, sStr );
                    pEditView->SetSelection( aSel );
                    return;
                }
            }
        }
    }
}

TextSelection EditorWindow::GetLastHighlightPortionTextSelection()
{//creates a text selection from the highlight portion on the cursor
    const sal_uInt32 nLine = GetEditView()->GetSelection().GetStart().GetPara();
    const sal_Int32 nIndex = GetEditView()->GetSelection().GetStart().GetIndex();
    OUString aLine( pEditEngine->GetText( nLine ) ); // the line being modified
    std::vector<HighlightPortion> aPortions;
    aHighlighter.getHighlightPortions( aLine, aPortions );

    assert(!aPortions.empty());
    HighlightPortion& r = aPortions.back();
    if( static_cast<size_t>(nIndex) != aPortions.size()-1 )
    {//cursor is not standing at the end of the line
        for (std::vector<HighlightPortion>::iterator i(aPortions.begin());
             i != aPortions.end(); ++i)
        {
            if( i->nEnd == nIndex )
            {
                r = *i;
                break;
            }
        }
    }

    if( aPortions.empty() )
        return TextSelection();

    OUString sStr = aLine.copy( r.nBegin, r.nEnd - r.nBegin );
    TextPaM aStart( nLine, r.nBegin );
    TextPaM aEnd( nLine, r.nBegin + sStr.getLength() );
    return TextSelection( aStart, aEnd );
}

void EditorWindow::HandleAutoCloseParen()
{
    TextSelection aSel = GetEditView()->GetSelection();
    const sal_uInt32 nLine =  aSel.GetStart().GetPara();
    OUString aLine( pEditEngine->GetText( nLine ) ); // the line being modified

    if( aLine.getLength() > 0 && aLine[aSel.GetEnd().GetIndex()-1] != '(' )
    {
        GetEditView()->InsertText(")");
        //leave the cursor on its place: inside the parenthesis
        TextPaM aEnd(nLine, aSel.GetEnd().GetIndex());
        GetEditView()->SetSelection( TextSelection( aEnd, aEnd ) );
    }
}

void EditorWindow::HandleAutoCloseDoubleQuotes()
{
    TextSelection aSel = GetEditView()->GetSelection();
    const sal_uInt32 nLine =  aSel.GetStart().GetPara();
    OUString aLine( pEditEngine->GetText( nLine ) ); // the line being modified

    std::vector<HighlightPortion> aPortions;
    aHighlighter.getHighlightPortions( aLine, aPortions );

    if( aPortions.empty() )
        return;

    if( aLine.getLength() > 0 && !aLine.endsWith("\"") && (aPortions.back().tokenType != TokenType::String) )
    {
        GetEditView()->InsertText("\"");
        //leave the cursor on its place: inside the two double quotes
        TextPaM aEnd(nLine, aSel.GetEnd().GetIndex());
        GetEditView()->SetSelection( TextSelection( aEnd, aEnd ) );
    }
}

void EditorWindow::HandleProcedureCompletion()
{

    TextSelection aSel = GetEditView()->GetSelection();
    const sal_uInt32 nLine = aSel.GetStart().GetPara();
    OUString aLine( pEditEngine->GetText( nLine ) );

    OUString sProcType;
    OUString sProcName;
    bool bFoundName = GetProcedureName(aLine, sProcType, sProcName);
    if (!bFoundName)
      return;

    OUString sText("\nEnd ");
    aSel = GetEditView()->GetSelection();
    if( sProcType.equalsIgnoreAsciiCase("function") )
        sText += "Function\n";
    if( sProcType.equalsIgnoreAsciiCase("sub") )
        sText += "Sub\n";

    if( nLine+1 == pEditEngine->GetParagraphCount() )
    {
        pEditView->InsertText( sText );//append to the end
        GetEditView()->SetSelection(aSel);
    }
    else
    {
        for( sal_uInt32 i = nLine+1; i < pEditEngine->GetParagraphCount(); ++i )
        {//searching forward for end token, or another sub/function definition
            OUString aCurrLine = pEditEngine->GetText( i );
            std::vector<HighlightPortion> aCurrPortions;
            aHighlighter.getHighlightPortions( aCurrLine, aCurrPortions );

            if( aCurrPortions.size() >= 3 )
            {//at least 3 tokens: (sub|function) whitespace idetifier ....
                HighlightPortion& r = aCurrPortions.front();
                OUString sStr = aCurrLine.copy(r.nBegin, r.nEnd - r.nBegin);

                if( r.tokenType == TokenType::Keywords )
                {
                    if( sStr.equalsIgnoreAsciiCase("sub") || sStr.equalsIgnoreAsciiCase("function") )
                    {
                        pEditView->InsertText( sText );//append to the end
                        GetEditView()->SetSelection(aSel);
                        break;
                    }
                    if( sStr.equalsIgnoreAsciiCase("end") )
                        break;
                }
            }
        }
    }
}

bool EditorWindow::GetProcedureName(OUString& rLine, OUString& rProcType, OUString& rProcName) const
{
    std::vector<HighlightPortion> aPortions;
    aHighlighter.getHighlightPortions(rLine, aPortions);

    if( aPortions.empty() )
        return false;

    bool bFoundType = false;
    bool bFoundName = false;

    for (std::vector<HighlightPortion>::iterator i(aPortions.begin());
         i != aPortions.end(); ++i)
    {
        OUString sTokStr = rLine.copy(i->nBegin, i->nEnd - i->nBegin);

        if( i->tokenType == TokenType::Keywords && ( sTokStr.equalsIgnoreAsciiCase("sub")
            || sTokStr.equalsIgnoreAsciiCase("function")) )
        {
            rProcType = sTokStr;
            bFoundType = true;
        }
        if( i->tokenType == TokenType::Identifier && bFoundType )
        {
            rProcName = sTokStr;
            bFoundName = true;
            break;
        }
    }

    if( !bFoundType || !bFoundName )
        return false;// no sub/function keyword or there is no identifier

    return true;

}

void EditorWindow::HandleCodeCompletion()
{
    rModulWindow.UpdateModule();
    rModulWindow.GetSbModule()->GetCodeCompleteDataFromParse(aCodeCompleteCache);
    TextSelection aSel = GetEditView()->GetSelection();
    const sal_uInt32 nLine =  aSel.GetStart().GetPara();
    OUString aLine( pEditEngine->GetText( nLine ) ); // the line being modified
    std::vector< OUString > aVect; //vector to hold the base variable+methods for the nested reflection

    std::vector<HighlightPortion> aPortions;
    aLine = aLine.copy(0, aSel.GetEnd().GetIndex());
    aHighlighter.getHighlightPortions( aLine, aPortions );
    if( !aPortions.empty() )
    {//use the syntax highlighter to grab out nested reflection calls, eg. aVar.aMethod("aa").aOtherMethod ..
        for( std::vector<HighlightPortion>::reverse_iterator i(
                 aPortions.rbegin());
             i != aPortions.rend(); ++i)
        {
            if( i->tokenType == TokenType::Whitespace ) // a whitespace: stop; if there is no ws, it goes to the beginning of the line
                break;
            if( i->tokenType == TokenType::Identifier || i->tokenType == TokenType::Keywords ) // extract the identifiers(methods, base variable)
            /* an example: Dim aLocVar2 as com.sun.star.beans.PropertyValue
             * here, aLocVar2.Name, and PropertyValue's Name field is treated as a keyword(?!)
             * */
                aVect.insert( aVect.begin(), aLine.copy(i->nBegin, i->nEnd - i->nBegin) );
        }

        if( aVect.empty() )//nothing to do
            return;

        OUString sBaseName = aVect[aVect.size()-1];//variable name
        OUString sVarType = aCodeCompleteCache.GetVarType( sBaseName );

        if( !sVarType.isEmpty() && CodeCompleteOptions::IsAutoCorrectOn() )
        {//correct variable name, if autocorrection on
            const OUString& sStr = aCodeCompleteCache.GetCorrectCaseVarName( sBaseName, GetActualSubName(nLine) );
            if( !sStr.isEmpty() )
            {
                TextPaM aStart(nLine, aSel.GetStart().GetIndex() - sStr.getLength() );
                TextSelection sTextSelection(aStart, TextPaM(nLine, aSel.GetStart().GetIndex()));
                pEditEngine->ReplaceText( sTextSelection, sStr );
                pEditView->SetSelection( aSel );
            }
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
                SetupAndShowCodeCompleteWnd( aEntryVect, aSel );
        }
    }
}

void EditorWindow::SetupAndShowCodeCompleteWnd( const std::vector< OUString >& aEntryVect, TextSelection aSel )
{
    // clear the listbox
    pCodeCompleteWnd->ClearListBox();
    // fill the listbox
    for(const auto & l : aEntryVect)
    {
        pCodeCompleteWnd->InsertEntry( l );
    }
    // show it
    pCodeCompleteWnd->Show();
    pCodeCompleteWnd->ResizeAndPositionListBox();
    pCodeCompleteWnd->SelectFirstEntry();
    // correct text selection, and set it
    ++aSel.GetStart().GetIndex();
    ++aSel.GetEnd().GetIndex();
    pCodeCompleteWnd->SetTextSelection( aSel );
    //give the focus to the EditView
    pEditView->GetWindow()->GrabFocus();
}

void EditorWindow::Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect)
{
    if (!pEditEngine)     // We need it now at latest
        CreateEditEngine();

    pEditView->Paint(rRenderContext, rRect);
}

void EditorWindow::LoseFocus()
{
    SetSourceInBasic();
    Window::LoseFocus();
}

void EditorWindow::SetSourceInBasic()
{
    if ( pEditEngine && pEditEngine->IsModified()
        && !GetEditView()->IsReadOnly() )   // Added for #i60626, otherwise
            // any read only bug in the text engine could lead to a crash later
    {
        if ( !StarBASIC::IsRunning() ) // Not at runtime!
        {
            rModulWindow.UpdateModule();
        }
    }
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
    if (pEditEngine)
        return;

    pEditEngine.reset(new ExtTextEngine);
    pEditView.reset(new ExtTextView(pEditEngine.get(), this));
    pEditView->SetAutoIndentMode(true);
    pEditEngine->SetUpdateMode(false);
    pEditEngine->InsertView(pEditView.get());

    ImplSetFont();

    aSyntaxIdle.SetPriority( SchedulerPriority::LOWER );
    aSyntaxIdle.SetIdleHdl( LINK( this, EditorWindow, SyntaxTimerHdl ) );

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
    while (nIndex >= 0);

    // nLines*4: SetText+Formatting+DoHighlight+Formatting
    // it could be cut down on one formatting but you would wait even longer
    // for the text then if the source code is long...
    pProgress.reset(new ProgressInfo(GetShell()->GetViewFrame()->GetObjectShell(),
                                     IDEResId(RID_STR_GENERATESOURCE).toString(),
                                     nLines * 4));
    setTextEngineText(*pEditEngine, aOUSource);

    pEditView->SetStartDocPos(Point(0, 0));
    pEditView->SetSelection(TextSelection());
    rModulWindow.GetBreakPointWindow().GetCurYOffset() = 0;
    rModulWindow.GetLineNumberWindow().GetCurYOffset() = 0;
    pEditEngine->SetUpdateMode(true);
    rModulWindow.Update();   // has only been invalidated at UpdateMode = true

    pEditView->ShowCursor();

    StartListening(*pEditEngine);

    aSyntaxIdle.Stop();
    bDoSyntaxHighlight = bWasDoSyntaxHighlight;

    for (sal_Int32 nLine = 0; nLine < nLines; nLine++)
        aSyntaxLineTable.insert(nLine);
    ForceSyntaxTimeout();

    pProgress.reset();

    pEditView->EraseVirtualDevice();
    pEditEngine->SetModified( false );
    pEditEngine->EnableUndo( true );

    InitScrollBars();

    if (SfxBindings* pBindings = GetBindingsPtr())
    {
        pBindings->Invalidate(SID_BASICIDE_STAT_POS);
        pBindings->Invalidate(SID_BASICIDE_STAT_TITLE);
    }

    DBG_ASSERT(rModulWindow.GetBreakPointWindow().GetCurYOffset() == 0, "CreateEditEngine: Brechpunkte verschoben?");

    // set readonly mode for readonly libraries
    ScriptDocument aDocument(rModulWindow.GetDocument());
    OUString aOULibName(rModulWindow.GetLibName());
    Reference< script::XLibraryContainer2 > xModLibContainer( aDocument.getLibraryContainer( E_SCRIPTS ), UNO_QUERY );
    if (xModLibContainer.is()
     && xModLibContainer->hasByName(aOULibName)
     && xModLibContainer->isLibraryReadOnly(aOULibName))
    {
        rModulWindow.SetReadOnly(true);
    }

    if (aDocument.isDocument() && aDocument.isReadOnly())
        rModulWindow.SetReadOnly(true);
}

// virtual
void EditorWindow::DataChanged(DataChangedEvent const & rDCEvt)
{
    Window::DataChanged(rDCEvt);
    if (rDCEvt.GetType() == DataChangedEventType::SETTINGS
        && (rDCEvt.GetFlags() & AllSettingsFlags::STYLE))
    {
        Color aColor(GetSettings().GetStyleSettings().GetFieldColor());
        const AllSettings* pOldSettings = rDCEvt.GetOldSettings();
        if (!pOldSettings || aColor != pOldSettings->GetStyleSettings().GetFieldColor())
        {
            SetBackground(Wallpaper(aColor));
            Invalidate();
        }
        if (pEditEngine != nullptr)
        {
            aColor = GetSettings().GetStyleSettings().GetFieldTextColor();
            if (!pOldSettings || aColor !=
                    pOldSettings-> GetStyleSettings().GetFieldTextColor())
            {
                vcl::Font aFont(pEditEngine->GetFont());
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
                ( rModulWindow.GetBreakPointWindow().GetCurYOffset() - pEditView->GetStartDocPos().Y() );
            rModulWindow.GetLineNumberWindow().DoScroll
                ( rModulWindow.GetLineNumberWindow().GetCurYOffset() - pEditView->GetStartDocPos().Y() );
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
                const long nWidth = pEditEngine->CalcTextWidth();
                if ( nWidth != nCurTextWidth )
                {
                    nCurTextWidth = nWidth;
                    rModulWindow.GetHScrollBar()->SetRange( Range( 0, nCurTextWidth-1) );
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
        SbMethod* pMeth = dynamic_cast<SbMethod*>( pMethods->Get( i )  );
        if( pMeth )
        {
            sal_uInt16 l1,l2;
            pMeth->GetLineRange(l1,l2);
            if( (l1 <= nLine+1) && (nLine+1 <= l2) )
            {
                return pMeth->GetName();
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
    if (!pEditEngine)
        return;

    SetScrollBarRanges();
    Size aOutSz(GetOutputSizePixel());
    rModulWindow.GetEditVScrollBar().SetVisibleSize(aOutSz.Height());
    rModulWindow.GetEditVScrollBar().SetPageSize(aOutSz.Height() * 8 / 10);
    rModulWindow.GetEditVScrollBar().SetLineSize(GetTextHeight());
    rModulWindow.GetEditVScrollBar().SetThumbPos(pEditView->GetStartDocPos().Y());
    rModulWindow.GetEditVScrollBar().Show();

    if (rModulWindow.GetHScrollBar())
    {
        rModulWindow.GetHScrollBar()->SetVisibleSize(aOutSz.Width());
        rModulWindow.GetHScrollBar()->SetPageSize(aOutSz.Width() * 8 / 10);
        rModulWindow.GetHScrollBar()->SetLineSize(GetTextWidth( "x" ) );
        rModulWindow.GetHScrollBar()->SetThumbPos(pEditView->GetStartDocPos().X());
        rModulWindow.GetHScrollBar()->Show();
    }
}

void EditorWindow::ImpDoHighlight( sal_uLong nLine )
{
    if ( bDoSyntaxHighlight )
    {
        OUString aLine( pEditEngine->GetText( nLine ) );
        bool const bWasModified = pEditEngine->IsModified();
        pEditEngine->RemoveAttribs( nLine );
        std::vector<HighlightPortion> aPortions;
        aHighlighter.getHighlightPortions( aLine, aPortions );

        for (std::vector<HighlightPortion>::iterator i(aPortions.begin());
             i != aPortions.end(); ++i)
        {
            Color const aColor = rModulWindow.GetLayout().GetSyntaxColor(i->tokenType);
            pEditEngine->SetAttrib(TextAttribFontColor(aColor), nLine, i->nBegin, i->nEnd);
        }

        pEditEngine->SetModified(bWasModified);
    }
}

void EditorWindow::UpdateSyntaxHighlighting ()
{
    const sal_uInt32 nCount = pEditEngine->GetParagraphCount();
    for (sal_uInt32 i = 0; i < nCount; ++i)
        DoDelayedSyntaxHighlight(i);
}

void EditorWindow::ImplSetFont()
{
    OUString sFontName(officecfg::Office::Common::Font::SourceViewFont::FontName::get().get_value_or(OUString()));
    if (sFontName.isEmpty())
    {
        vcl::Font aTmpFont(OutputDevice::GetDefaultFont(DefaultFontType::FIXED,
                                                        Application::GetSettings().GetUILanguageTag().getLanguageType(),
                                                        GetDefaultFontFlags::NONE, this));
        sFontName = aTmpFont.GetFamilyName();
    }
    Size aFontSize(0, officecfg::Office::Common::Font::SourceViewFont::FontHeight::get());
    vcl::Font aFont(sFontName, aFontSize);
    aFont.SetColor(Application::GetSettings().GetStyleSettings().GetFieldTextColor());
    SetPointFont(*this, aFont); // FIXME RenderContext
    aFont = GetFont();

    rModulWindow.GetBreakPointWindow().SetFont(aFont);
    rModulWindow.GetLineNumberWindow().SetFont(aFont);

    if (pEditEngine)
    {
        bool const bModified = pEditEngine->IsModified();
        pEditEngine->SetFont(aFont);
        pEditEngine->SetModified(bModified);
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

    if ( !bHighlighting && bDoSyntaxHighlight )
    {
        if ( bDelayHighlight )
        {
            aSyntaxLineTable.insert( nPara );
            aSyntaxIdle.Start();
        }
        else
            DoSyntaxHighlight( nPara );
    }
}

IMPL_LINK_NOARG_TYPED(EditorWindow, SyntaxTimerHdl, Idle *, void)
{
    DBG_ASSERT( pEditView, "Noch keine View, aber Syntax-Highlight ?!" );

    bool const bWasModified = pEditEngine->IsModified();
    //pEditEngine->SetUpdateMode(false);

    bHighlighting = true;
    for ( std::set<sal_uInt16>::const_iterator it = aSyntaxLineTable.begin();
          it != aSyntaxLineTable.end(); ++it )
    {
        sal_uInt16 nLine = *it;
        DoSyntaxHighlight( nLine );
    }

    // #i45572#
    if ( pEditView )
        pEditView->ShowCursor( false );

    pEditEngine->SetModified( bWasModified );

    aSyntaxLineTable.clear();
    bHighlighting = false;
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
    aSyntaxIdle.Stop();
    aSyntaxIdle.GetIdleHdl().Call(&aSyntaxIdle);
}

// BreakPointWindow

BreakPointWindow::BreakPointWindow (vcl::Window* pParent, ModulWindow* pModulWindow)
    : Window(pParent, WB_BORDER)
    , rModulWindow(*pModulWindow)
    , nCurYOffset(0) // memorize nCurYOffset and not take it from EditEngine
    , nMarkerPos(NoMarker)
    , bErrorMarker(false)
{
    setBackgroundColor(GetSettings().GetStyleSettings().GetFieldColor());
    SetHelpId(HID_BASICIDE_BREAKPOINTWINDOW);
}

void BreakPointWindow::Paint(vcl::RenderContext& rRenderContext, const Rectangle&)
{
    if (SyncYOffset())
        return;

    Size const aOutSz = rRenderContext.GetOutputSize();
    long const nLineHeight = rRenderContext.GetTextHeight();

    Image const aBrk[2] =
    {
        GetImage(IMGID_BRKDISABLED),
        GetImage(IMGID_BRKENABLED)
    };

    Size const aBmpSz = rRenderContext.PixelToLogic(aBrk[1].GetSizePixel());
    Point const aBmpOff((aOutSz.Width() - aBmpSz.Width()) / 2,
                        (nLineHeight - aBmpSz.Height()) / 2);

    for (size_t i = 0, n = GetBreakPoints().size(); i < n; ++i)
    {
        BreakPoint& rBrk = *GetBreakPoints().at(i);
        size_t const nLine = rBrk.nLine - 1;
        size_t const nY = nLine*nLineHeight - nCurYOffset;
        rRenderContext.DrawImage(Point(0, nY) + aBmpOff, aBrk[rBrk.bEnabled]);
    }

    ShowMarker(rRenderContext);
}

void BreakPointWindow::ShowMarker(vcl::RenderContext& rRenderContext)
{
    if (nMarkerPos == NoMarker)
        return;

    Size const aOutSz = GetOutputSize();
    long const nLineHeight = GetTextHeight();

    Image aMarker = GetImage(bErrorMarker ? IMGID_ERRORMARKER : IMGID_STEPMARKER);

    Size aMarkerSz(aMarker.GetSizePixel());
    aMarkerSz = rRenderContext.PixelToLogic(aMarkerSz);
    Point aMarkerOff(0, 0);
    aMarkerOff.X() = (aOutSz.Width() - aMarkerSz.Width()) / 2;
    aMarkerOff.Y() = (nLineHeight - aMarkerSz.Height()) / 2;

    sal_uLong nY = nMarkerPos * nLineHeight - nCurYOffset;
    Point aPos(0, nY);
    aPos += aMarkerOff;

    rRenderContext.DrawImage(aPos, aMarker);
}

void BreakPointWindow::DoScroll( long nVertScroll )
{
    nCurYOffset -= nVertScroll;
    Window::Scroll( 0, nVertScroll );
}

void BreakPointWindow::SetMarkerPos( sal_uInt16 nLine, bool bError )
{
    if ( SyncYOffset() )
        Update();

    nMarkerPos = nLine;
    bErrorMarker = bError;
    Invalidate();
}

void BreakPointWindow::SetNoMarker ()
{
    SetMarkerPos(NoMarker);
}

BreakPoint* BreakPointWindow::FindBreakPoint( const Point& rMousePos )
{
    size_t nLineHeight = GetTextHeight();
    nLineHeight = nLineHeight > 0 ? nLineHeight : 1;
    size_t nYPos = rMousePos.Y() + nCurYOffset;

    for ( size_t i = 0, n = GetBreakPoints().size(); i < n ; ++i )
    {
        BreakPoint* pBrk = GetBreakPoints().at( i );
        size_t nLine = pBrk->nLine-1;
        size_t nY = nLine*nLineHeight;
        if ( ( nYPos > nY ) && ( nYPos < ( nY + nLineHeight ) ) )
            return pBrk;
    }
    return nullptr;
}

void BreakPointWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.GetClicks() == 2 )
    {
        Point aMousePos( PixelToLogic( rMEvt.GetPosPixel() ) );
        long nLineHeight = GetTextHeight();
        if(nLineHeight)
        {
            long nYPos = aMousePos.Y() + nCurYOffset;
            long nLine = nYPos / nLineHeight + 1;
            rModulWindow.ToggleBreakPoint( (sal_uLong)nLine );
            Invalidate();
        }
    }
}

void BreakPointWindow::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() == CommandEventId::ContextMenu )
    {
        Point aPos( rCEvt.IsMouseEvent() ? rCEvt.GetMousePosPixel() : Point(1,1) );
        Point aEventPos( PixelToLogic( aPos ) );
        BreakPoint* pBrk = rCEvt.IsMouseEvent() ? FindBreakPoint( aEventPos ) : nullptr;
        if ( pBrk )
        {
            // test if break point is enabled...
            ScopedVclPtrInstance<PopupMenu> aBrkPropMenu( IDEResId( RID_POPUP_BRKPROPS ) );
            aBrkPropMenu->CheckItem( RID_ACTIV, pBrk->bEnabled );
            switch ( aBrkPropMenu->Execute( this, aPos ) )
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
                    ScopedVclPtrInstance< BreakPointDialog > aBrkDlg( this, GetBreakPoints() );
                    aBrkDlg->SetCurrentBreakPoint( pBrk );
                    aBrkDlg->Execute();
                    Invalidate();
                }
                break;
            }
        }
        else
        {
            ScopedVclPtrInstance<PopupMenu> aBrkListMenu( IDEResId( RID_POPUP_BRKDLG ) );
            switch ( aBrkListMenu->Execute( this, aPos ) )
            {
                case RID_BRKDLG:
                {
                    ScopedVclPtrInstance< BreakPointDialog > aBrkDlg( this, GetBreakPoints() );
                    aBrkDlg->Execute();
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
    if (rDCEvt.GetType() == DataChangedEventType::SETTINGS
        && (rDCEvt.GetFlags() & AllSettingsFlags::STYLE))
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

namespace
{
    const sal_uInt16 ITEM_ID_VARIABLE = 1;
    const sal_uInt16 ITEM_ID_VALUE = 2;
    const sal_uInt16 ITEM_ID_TYPE = 3;
}

WatchWindow::WatchWindow (Layout* pParent) :
    DockingWindow(pParent),
    aWatchStr( IDEResId( RID_STR_REMOVEWATCH ) ),
    aXEdit( VclPtr<ExtendedEdit>::Create(this, IDEResId( RID_EDT_WATCHEDIT )) ),
    aRemoveWatchButton( VclPtr<ImageButton>::Create(this, IDEResId( RID_IMGBTN_REMOVEWATCH )) ),
    aTreeListBox( VclPtr<WatchTreeListBox>::Create(this, WB_BORDER | WB_3DLOOK | WB_HASBUTTONS | WB_HASLINES | WB_HSCROLL | WB_TABSTOP
                                  | WB_HASLINESATROOT | WB_HASBUTTONSATROOT) ),
    aHeaderBar( VclPtr<HeaderBar>::Create( this, WB_BUTTONSTYLE | WB_BORDER ) )
{
    aXEdit->SetAccessibleName(IDEResId(RID_STR_WATCHNAME).toString());
    aTreeListBox->SetAccessibleName(IDEResId(RID_STR_WATCHNAME).toString());

    long nTextLen = GetTextWidth( aWatchStr ) + DWBORDER + 3;
    aXEdit->SetPosPixel( Point( nTextLen, 3 ) );
    aXEdit->SetAccHdl( LINK( this, WatchWindow, EditAccHdl ) );
    aXEdit->GetAccelerator().InsertItem( 1, vcl::KeyCode( KEY_RETURN ) );
    aXEdit->GetAccelerator().InsertItem( 2, vcl::KeyCode( KEY_ESCAPE ) );
    aXEdit->Show();

    aRemoveWatchButton->Disable();
    aRemoveWatchButton->SetClickHdl( LINK( this, WatchWindow, ButtonHdl ) );
    aRemoveWatchButton->SetPosPixel( Point( nTextLen + aXEdit->GetSizePixel().Width() + 4, 2 ) );
    Size aSz( aRemoveWatchButton->GetModeImage().GetSizePixel() );
    aSz.Width() += 6;
    aSz.Height() += 6;
    aRemoveWatchButton->SetSizePixel( aSz );
    aRemoveWatchButton->Show();

    long nRWBtnSize = aRemoveWatchButton->GetModeImage().GetSizePixel().Height() + 10;
    nVirtToolBoxHeight = aXEdit->GetSizePixel().Height() + 7;

    if ( nRWBtnSize > nVirtToolBoxHeight )
        nVirtToolBoxHeight = nRWBtnSize;

    nHeaderBarHeight = 16;

    aTreeListBox->SetHelpId(HID_BASICIDE_WATCHWINDOW_LIST);
    aTreeListBox->EnableInplaceEditing(true);
    aTreeListBox->SetSelectHdl( LINK( this, WatchWindow, TreeListHdl ) );
    aTreeListBox->SetPosPixel( Point( DWBORDER, nVirtToolBoxHeight + nHeaderBarHeight ) );
    aTreeListBox->SetHighlightRange( 1, 5 );

    Point aPnt( DWBORDER, nVirtToolBoxHeight + 1 );
    aHeaderBar->SetPosPixel( aPnt );
    aHeaderBar->SetEndDragHdl( LINK( this, WatchWindow, implEndDragHdl ) );

    long nVarTabWidth = 220;
    long nValueTabWidth = 100;
    long nTypeTabWidth = 1250;
    aHeaderBar->InsertItem( ITEM_ID_VARIABLE, IDEResId(RID_STR_WATCHVARIABLE).toString(), nVarTabWidth );
    aHeaderBar->InsertItem( ITEM_ID_VALUE, IDEResId(RID_STR_WATCHVALUE).toString(), nValueTabWidth );
    aHeaderBar->InsertItem( ITEM_ID_TYPE, IDEResId(RID_STR_WATCHTYPE).toString(), nTypeTabWidth );

    long tabs[ 4 ];
    tabs[ 0 ] = 3; // two tabs
    tabs[ 1 ] = 0;
    tabs[ 2 ] = nVarTabWidth;
    tabs[ 3 ] = nVarTabWidth + nValueTabWidth;
    aTreeListBox->SvHeaderTabListBox::SetTabs( tabs, MAP_PIXEL );
    aTreeListBox->InitHeaderBar( aHeaderBar.get() );

    aTreeListBox->SetNodeDefaultImages( );

    aHeaderBar->Show();

    aTreeListBox->Show();

    SetText(IDEResId(RID_STR_WATCHNAME).toString());

    SetHelpId( HID_BASICIDE_WATCHWINDOW );

    // make watch window keyboard accessible
    GetSystemWindow()->GetTaskPaneList()->AddWindow( this );
}


WatchWindow::~WatchWindow()
{
    disposeOnce();
}

void WatchWindow::dispose()
{
    aXEdit.disposeAndClear();
    aRemoveWatchButton.disposeAndClear();
    aHeaderBar.disposeAndClear();
    aTreeListBox.disposeAndClear();
    if (!IsDisposed())
        GetSystemWindow()->GetTaskPaneList()->RemoveWindow( this );
    DockingWindow::dispose();
}

void WatchWindow::Paint(vcl::RenderContext& rRenderContext, const Rectangle&)
{
    rRenderContext.DrawText(Point(DWBORDER, 7), aWatchStr);
    lcl_DrawIDEWindowFrame(this, rRenderContext);
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
    aTreeListBox->SetSizePixel( aBoxSz );
    aTreeListBox->GetHScroll()->SetPageSize( aTreeListBox->GetHScroll()->GetVisibleSize() );

    aBoxSz.Height() = nHeaderBarHeight;
    aHeaderBar->SetSizePixel( aBoxSz );

    Invalidate();
}

struct WatchItem
{
    OUString        maName;
    OUString        maDisplayName;
    SbxObjectRef    mpObject;
    std::vector<OUString> maMemberList;

    SbxDimArrayRef  mpArray;
    int             nDimLevel;  // 0 = Root
    int             nDimCount;
    std::vector<short> vIndices;

    WatchItem*      mpArrayParentItem;

    explicit WatchItem (OUString const& rName):
        maName(rName),
        nDimLevel(0),
        nDimCount(0),
        mpArrayParentItem(nullptr)
    { }

    void clearWatchItem ()
    {
        maMemberList.clear();
    }

    WatchItem* GetRootItem();
    SbxDimArray* GetRootArray();
};

WatchItem* WatchItem::GetRootItem()
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

SbxDimArray* WatchItem::GetRootArray()
{
    WatchItem* pRootItem = GetRootItem();
    SbxDimArray* pRet = nullptr;
    if( pRootItem )
        pRet = pRootItem->mpArray;
    return pRet;
}

void WatchWindow::AddWatch( const OUString& rVName )
{
    OUString aVar, aIndex;
    lcl_SeparateNameAndIndex( rVName, aVar, aIndex );
    WatchItem* pWatchItem = new WatchItem(aVar);

    OUString aWatchStr_ = aVar + "\t\t";
    SvTreeListEntry* pNewEntry = aTreeListBox->InsertEntry( aWatchStr_, nullptr, true );
    pNewEntry->SetUserData( pWatchItem );

    aTreeListBox->Select(pNewEntry);
    aTreeListBox->MakeVisible(pNewEntry);
    aRemoveWatchButton->Enable();

    UpdateWatches();
}

void WatchWindow::RemoveSelectedWatch()
{
    SvTreeListEntry* pEntry = aTreeListBox->GetCurEntry();
    if ( pEntry )
    {
        aTreeListBox->GetModel()->Remove( pEntry );
        pEntry = aTreeListBox->GetCurEntry();
        if ( pEntry )
            aXEdit->SetText( static_cast<WatchItem*>(pEntry->GetUserData())->maName );
        else
            aXEdit->SetText( OUString() );
        if ( !aTreeListBox->GetEntryCount() )
            aRemoveWatchButton->Disable();
    }
}


IMPL_LINK_TYPED( WatchWindow, ButtonHdl, Button *, pButton, void )
{
    if (pButton == aRemoveWatchButton.get())
        if (SfxDispatcher* pDispatcher = GetDispatcher())
            pDispatcher->Execute(SID_BASICIDE_REMOVEWATCH);
}

IMPL_LINK_NOARG_TYPED(WatchWindow, TreeListHdl, SvTreeListBox*, void)
{
    SvTreeListEntry* pCurEntry = aTreeListBox->GetCurEntry();
    if ( pCurEntry && pCurEntry->GetUserData() )
        aXEdit->SetText( static_cast<WatchItem*>(pCurEntry->GetUserData())->maName );
}

IMPL_LINK_NOARG_TYPED( WatchWindow, implEndDragHdl, HeaderBar *, void )
{
    const sal_Int32 TAB_WIDTH_MIN = 10;
    sal_Int32 nMaxWidth =
        aHeaderBar->GetSizePixel().getWidth() - 2 * TAB_WIDTH_MIN;

    sal_Int32 nVariableWith = aHeaderBar->GetItemSize( ITEM_ID_VARIABLE );
    if( nVariableWith < TAB_WIDTH_MIN )
        aHeaderBar->SetItemSize( ITEM_ID_VARIABLE, TAB_WIDTH_MIN );
    else if( nVariableWith > nMaxWidth )
        aHeaderBar->SetItemSize( ITEM_ID_VARIABLE, nMaxWidth );

    sal_Int32 nValueWith = aHeaderBar->GetItemSize( ITEM_ID_VALUE );
    if( nValueWith < TAB_WIDTH_MIN )
        aHeaderBar->SetItemSize( ITEM_ID_VALUE, TAB_WIDTH_MIN );
    else if( nValueWith > nMaxWidth )
        aHeaderBar->SetItemSize( ITEM_ID_VALUE, nMaxWidth );

    if (aHeaderBar->GetItemSize( ITEM_ID_TYPE ) < TAB_WIDTH_MIN)
        aHeaderBar->SetItemSize( ITEM_ID_TYPE, TAB_WIDTH_MIN );

    sal_Int32 nPos = 0;
    sal_uInt16 nTabs = aHeaderBar->GetItemCount();
    for( sal_uInt16 i = 1 ; i < nTabs ; ++i )
    {
        nPos += aHeaderBar->GetItemSize( i );
        aTreeListBox->SetTab( i, nPos, MAP_PIXEL );
    }
}

IMPL_LINK_TYPED( WatchWindow, EditAccHdl, Accelerator&, rAcc, void )
{
    switch ( rAcc.GetCurKeyCode().GetCode() )
    {
        case KEY_RETURN:
        {
            OUString aCurText( aXEdit->GetText() );
            if ( !aCurText.isEmpty() )
            {
                AddWatch( aCurText );
                aXEdit->SetSelection( Selection( 0, 0xFFFF ) );
            }
        }
        break;
        case KEY_ESCAPE:
        {
            aXEdit->SetText( OUString() );
        }
        break;
    }
}

void WatchWindow::UpdateWatches( bool bBasicStopped )
{
    aTreeListBox->UpdateWatches( bBasicStopped );
}


// StackWindow


StackWindow::StackWindow (Layout* pParent) :
    DockingWindow(pParent),
    aTreeListBox( VclPtr<SvTreeListBox>::Create(this, WB_BORDER | WB_3DLOOK | WB_HSCROLL | WB_TABSTOP) ),
    aStackStr( IDEResId( RID_STR_STACK ) )
{
    aTreeListBox->SetHelpId(HID_BASICIDE_STACKWINDOW_LIST);
    aTreeListBox->SetAccessibleName(IDEResId(RID_STR_STACKNAME).toString());
    aTreeListBox->SetPosPixel( Point( DWBORDER, nVirtToolBoxHeight ) );
    aTreeListBox->SetHighlightRange();
    aTreeListBox->SetSelectionMode( SelectionMode::NONE );
    aTreeListBox->InsertEntry( OUString() );
    aTreeListBox->Show();

    SetText(IDEResId(RID_STR_STACKNAME).toString());

    SetHelpId( HID_BASICIDE_STACKWINDOW );

    // make stack window keyboard accessible
    GetSystemWindow()->GetTaskPaneList()->AddWindow( this );
}


StackWindow::~StackWindow()
{
    disposeOnce();
}

void StackWindow::dispose()
{
    if (!IsDisposed())
        GetSystemWindow()->GetTaskPaneList()->RemoveWindow( this );
    aTreeListBox.disposeAndClear();
    DockingWindow::dispose();
}

void StackWindow::Paint(vcl::RenderContext& rRenderContext, const Rectangle&)
{
    rRenderContext.DrawText(Point(DWBORDER, 7), aStackStr);
    lcl_DrawIDEWindowFrame(this, rRenderContext);
}

void StackWindow::Resize()
{
    Size aSz = GetOutputSizePixel();
    Size aBoxSz(aSz.Width() - 2*DWBORDER, aSz.Height() - nVirtToolBoxHeight - DWBORDER);

    if ( aBoxSz.Width() < 4 )
        aBoxSz.Width() = 0;
    if ( aBoxSz.Height() < 4 )
        aBoxSz.Height() = 0;

    aTreeListBox->SetSizePixel( aBoxSz );

    Invalidate();
}

void StackWindow::UpdateCalls()
{
    aTreeListBox->SetUpdateMode(false);
    aTreeListBox->Clear();

    if (StarBASIC::IsRunning())
    {
        SbxError eOld = SbxBase::GetError();
        aTreeListBox->SetSelectionMode( SelectionMode::Single );

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
                    assert(pVar && "Parameter?!");
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
            aTreeListBox->InsertEntry( aEntry );
            nScope++;
            pMethod = StarBASIC::GetActiveMethod( nScope );
        }

        SbxBase::ResetError();
        if( eOld != ERRCODE_SBX_OK )
            SbxBase::SetError( eOld );
    }
    else
    {
        aTreeListBox->SetSelectionMode( SelectionMode::NONE );
        aTreeListBox->InsertEntry( OUString() );
    }

    aTreeListBox->SetUpdateMode(true);
}

ComplexEditorWindow::ComplexEditorWindow( ModulWindow* pParent ) :
    Window( pParent, WB_3DLOOK | WB_CLIPCHILDREN ),
    aBrkWindow(VclPtr<BreakPointWindow>::Create(this, pParent)),
    aLineNumberWindow(VclPtr<LineNumberWindow>::Create(this, pParent)),
    aEdtWindow(VclPtr<EditorWindow>::Create(this, pParent)),
    aEWVScrollBar( VclPtr<ScrollBar>::Create(this, WB_VSCROLL | WB_DRAG) )
{
    aEdtWindow->Show();
    aBrkWindow->Show();

    aEWVScrollBar->SetLineSize(nScrollLine);
    aEWVScrollBar->SetPageSize(nScrollPage);
    aEWVScrollBar->SetScrollHdl( LINK( this, ComplexEditorWindow, ScrollHdl ) );
    aEWVScrollBar->Show();
}

ComplexEditorWindow::~ComplexEditorWindow()
{
    disposeOnce();
}

void ComplexEditorWindow::dispose()
{
    aBrkWindow.disposeAndClear();
    aLineNumberWindow.disposeAndClear();
    aEdtWindow.disposeAndClear();
    aEWVScrollBar.disposeAndClear();
    vcl::Window::dispose();
}

void ComplexEditorWindow::Resize()
{
    Size aOutSz = GetOutputSizePixel();
    Size aSz(aOutSz);
    aSz.Width() -= 2*DWBORDER;
    aSz.Height() -= 2*DWBORDER;
    long nBrkWidth = 20;
    long nSBWidth = aEWVScrollBar->GetSizePixel().Width();

    Size aBrkSz(nBrkWidth, aSz.Height());

    Size aLnSz(aLineNumberWindow->GetWidth(), aSz.Height());

    if (aLineNumberWindow->IsVisible())
    {
        aBrkWindow->SetPosSizePixel( Point( DWBORDER, DWBORDER ), aBrkSz );
        aLineNumberWindow->SetPosSizePixel(Point(DWBORDER + aBrkSz.Width() - 1, DWBORDER), aLnSz);
        Size aEWSz(aSz.Width() - nBrkWidth - aLineNumberWindow->GetWidth() - nSBWidth + 2, aSz.Height());
        aEdtWindow->SetPosSizePixel( Point( DWBORDER + aBrkSz.Width() + aLnSz.Width() - 1, DWBORDER ), aEWSz );
    }
    else
    {
        aBrkWindow->SetPosSizePixel( Point( DWBORDER, DWBORDER ), aBrkSz );
        Size aEWSz(aSz.Width() - nBrkWidth - nSBWidth + 2, aSz.Height());
        aEdtWindow->SetPosSizePixel(Point(DWBORDER + aBrkSz.Width() - 1, DWBORDER), aEWSz);
    }

    aEWVScrollBar->SetPosSizePixel( Point( aOutSz.Width() - DWBORDER - nSBWidth, DWBORDER ), Size( nSBWidth, aSz.Height() ) );
}

IMPL_LINK_TYPED(ComplexEditorWindow, ScrollHdl, ScrollBar *, pCurScrollBar, void )
{
    if (aEdtWindow->GetEditView())
    {
        DBG_ASSERT( pCurScrollBar == aEWVScrollBar.get(), "Wer scrollt hier ?" );
        long nDiff = aEdtWindow->GetEditView()->GetStartDocPos().Y() - pCurScrollBar->GetThumbPos();
        aEdtWindow->GetEditView()->Scroll( 0, nDiff );
        aBrkWindow->DoScroll( nDiff );
        aLineNumberWindow->DoScroll( nDiff );
        aEdtWindow->GetEditView()->ShowCursor(false);
        pCurScrollBar->SetThumbPos( aEdtWindow->GetEditView()->GetStartDocPos().Y() );
    }
}

void ComplexEditorWindow::DataChanged(DataChangedEvent const & rDCEvt)
{
    Window::DataChanged(rDCEvt);
    if (rDCEvt.GetType() == DataChangedEventType::SETTINGS
        && (rDCEvt.GetFlags() & AllSettingsFlags::STYLE))
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
    aLineNumberWindow->Show(b);
    Resize();
}

uno::Reference< awt::XWindowPeer >
EditorWindow::GetComponentInterface(bool bCreate)
{
    uno::Reference< awt::XWindowPeer > xPeer(
        Window::GetComponentInterface(false));
    if (!xPeer.is() && bCreate)
    {
        // Make sure edit engine and view are available:
        if (!pEditEngine)
            CreateEditEngine();

        xPeer = svt::createTextWindowPeer(*GetEditView());
        SetComponentInterface(xPeer);
    }
    return xPeer;
}


// WatchTreeListBox


WatchTreeListBox::WatchTreeListBox( vcl::Window* pParent, WinBits nWinBits )
    : SvHeaderTabListBox( pParent, nWinBits )
{}

WatchTreeListBox::~WatchTreeListBox()
{
    disposeOnce();
}

void WatchTreeListBox::dispose()
{
    // Destroy user data
    SvTreeListEntry* pEntry = First();
    while ( pEntry )
    {
        delete static_cast<WatchItem*>(pEntry->GetUserData());
        pEntry->SetUserData(nullptr);
        pEntry = Next( pEntry );
    }
    SvHeaderTabListBox::dispose();
}

void WatchTreeListBox::SetTabs()
{
    SvHeaderTabListBox::SetTabs();
    sal_uInt16 nTabCount_ = aTabs.size();
    for( sal_uInt16 i = 0 ; i < nTabCount_ ; i++ )
    {
        SvLBoxTab* pTab = aTabs[i];
        if( i == 2 )
            pTab->nFlags |= SvLBoxTabFlags::EDITABLE;
        else
            pTab->nFlags &= ~SvLBoxTabFlags::EDITABLE;
    }
}

void WatchTreeListBox::RequestingChildren( SvTreeListEntry * pParent )
{
    if( !StarBASIC::IsRunning() )
        return;

    if( GetChildCount( pParent ) > 0 )
        return;

    SvTreeListEntry* pEntry = pParent;
    WatchItem* pItem = static_cast<WatchItem*>(pEntry->GetUserData());

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
        if ( nPropCount >= 3 &&
             pProps->Get( nPropCount -1 )->GetName().equalsIgnoreAsciiCase( "Dbg_Methods" ) &&
             pProps->Get( nPropCount -2 )->GetName().equalsIgnoreAsciiCase( "Dbg_Properties" ) &&
             pProps->Get( nPropCount -3 )->GetName().equalsIgnoreAsciiCase( "Dbg_SupportedInterfaces" ) )
        {
            nPropCount -= 3;
        }
        pItem->maMemberList.reserve(nPropCount);

        for( sal_uInt16 i = 0 ; i < nPropCount ; ++i )
        {
            SbxVariable* pVar = pProps->Get( i );

            pItem->maMemberList.push_back(pVar->GetName());
            OUString const& rName = pItem->maMemberList.back();
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
    SbxBase* pSBX = nullptr;
    rbArrayElement = false;

    WatchItem* pItem = static_cast<WatchItem*>(pEntry->GetUserData());
    OUString aVName( pItem->maName );

    SvTreeListEntry* pParentEntry = GetParent( pEntry );
    WatchItem* pParentItem = pParentEntry ? static_cast<WatchItem*>(pParentEntry->GetUserData()) : nullptr;
    if( pParentItem )
    {
        SbxObject* pObj = pParentItem->mpObject;
        SbxDimArray* pArray;
        if( pObj )
        {
            pSBX = pObj->Find( aVName, SbxClassType::DontCare );
            if (SbxVariable const* pVar = IsSbxVariable(pSBX))
            {
                // Force getting value
                SbxValues aRes;
                aRes.eType = SbxVOID;
                pVar->Get( aRes );
            }
        }
        // Array?
        else if( (pArray = pItem->GetRootArray()) != nullptr )
        {
            rbArrayElement = true;
            if( pParentItem->nDimLevel + 1 == pParentItem->nDimCount )
                pSBX = pArray->Get(pItem->vIndices.empty() ? nullptr : &*pItem->vIndices.begin());
        }
    }
    else
    {
        pSBX = StarBASIC::FindSBXInCurrentScope( aVName );
    }
    return pSBX;
}

bool WatchTreeListBox::EditingEntry( SvTreeListEntry* pEntry, Selection& )
{
    WatchItem* pItem = static_cast<WatchItem*>(pEntry->GetUserData());

    bool bEdit = false;
    if ( StarBASIC::IsRunning() && StarBASIC::GetActiveMethod() && !SbxBase::IsError() )
    {
        // No out of scope entries
        bool bArrayElement;
        SbxBase* pSbx = ImplGetSBXForEntry( pEntry, bArrayElement );
        if (IsSbxVariable(pSbx) || bArrayElement)
        {
            // Accept no objects and only end nodes of arrays for editing
            if( !pItem->mpObject && (pItem->mpArray == nullptr || pItem->nDimLevel == pItem->nDimCount) )
            {
                aEditingRes = SvHeaderTabListBox::GetEntryText( pEntry, ITEM_ID_VALUE-1 );
                aEditingRes = comphelper::string::strip(aEditingRes, ' ');
                bEdit = true;
            }
        }
    }

    return bEdit;
}

bool WatchTreeListBox::EditedEntry( SvTreeListEntry* pEntry, const OUString& rNewText )
{
    OUString aResult = comphelper::string::strip(rNewText, ' ');

    sal_uInt16 nResultLen = aResult.getLength();
    sal_Unicode cFirst = aResult[0];
    sal_Unicode cLast  = aResult[ nResultLen - 1 ];
    if( cFirst == '\"' && cLast == '\"' )
        aResult = aResult.copy( 1, nResultLen - 2 );

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
    while( (pDeleteEntry = pThis->SvTreeListBox::GetEntry( pParent, 0 )) != nullptr )
    {
        implCollapseModifiedObjectEntry( pDeleteEntry, pThis );

        delete static_cast<WatchItem*>(pDeleteEntry->GetUserData());
        pModel->Remove( pDeleteEntry );
    }
}

OUString implCreateTypeStringForDimArray( WatchItem* pItem, SbxDataType eType )
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
            (pEntry->GetFlags() & ~SvTLEntryFlags(SvTLEntryFlags::NO_NODEBMP | SvTLEntryFlags::HAD_CHILDREN))
            | SvTLEntryFlags::CHILDREN_ON_DEMAND );
    }
    else
    {
        pEntry->SetFlags( pEntry->GetFlags() & ~SvTLEntryFlags::CHILDREN_ON_DEMAND );
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
        WatchItem* pItem = static_cast<WatchItem*>(pEntry->GetUserData());
        DBG_ASSERT( !pItem->maName.isEmpty(), "Var? - Must not be empty!" );
        OUString aWatchStr;
        OUString aTypeStr;
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
                        if( pNewArray != nullptr && pOldArray != nullptr )
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
                        else if( pNewArray == nullptr || pOldArray == nullptr )
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
                        if( bArrayChanged && pOldArray != nullptr )
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
                        if (pItem->mpObject && !pItem->maMemberList.empty())
                        {
                            bool bObjChanged = false; // Check if member list has changed
                            SbxArray* pProps = pObj->GetProperties();
                            sal_uInt16 nPropCount = pProps->Count();
                            for( sal_uInt16 i = 0 ; i < nPropCount - 3 ; i++ )
                            {
                                SbxVariable* pVar_ = pProps->Get( i );
                                OUString aName( pVar_->GetName() );
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
                        aWatchStr = "Null";
                        if( pItem->mpObject != nullptr )
                        {
                            bCollapse = true;
                            pItem->clearWatchItem();

                            implEnableChildren( pEntry, false );
                        }
                    }
                }
                else
                {
                    if( pItem->mpObject != nullptr )
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
                if( aTypeStr.isEmpty() )
                {
                    if( !pVar->IsFixed() )
                    {
                        aTypeStr = "Variant/";
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
                pItem->mpObject = nullptr;
            }
        }

        SvHeaderTabListBox::SetEntryText( aWatchStr, pEntry, ITEM_ID_VALUE-1 );
        SvHeaderTabListBox::SetEntryText( aTypeStr, pEntry, ITEM_ID_TYPE-1 );

        pEntry = Next( pEntry );
    }

    // Force redraw
    Invalidate();

    SbxBase::ResetError();
    if( eOld != ERRCODE_SBX_OK )
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

CodeCompleteListBox::~CodeCompleteListBox()
{
    disposeOnce();
}

void CodeCompleteListBox::dispose()
{
    pCodeCompleteWindow.clear();
    ListBox::dispose();
}

IMPL_LINK_NOARG_TYPED(CodeCompleteListBox, ImplDoubleClickHdl, ListBox&, void)
{
    InsertSelectedEntry();
}

IMPL_LINK_NOARG_TYPED(CodeCompleteListBox, ImplSelectHdl, ListBox&, void)
{//give back the focus to the parent
    pCodeCompleteWindow->pParent->GrabFocus();
}

ExtTextView* CodeCompleteListBox::GetParentEditView()
{
    return pCodeCompleteWindow->pParent->GetEditView();
}

void CodeCompleteListBox::InsertSelectedEntry()
{
    if( !aFuncBuffer.toString().isEmpty() )
    {
        // if the user typed in something: remove, and insert
        GetParentEditView()->SetSelection( pCodeCompleteWindow->pParent->GetLastHighlightPortionTextSelection() );
        GetParentEditView()->DeleteSelected();

        if( !GetSelectEntry().isEmpty() )
        {//if the user selected something
            GetParentEditView()->InsertText( GetSelectEntry() );
        }
    }
    else
    {
        if( !GetSelectEntry().isEmpty() )
        {//if the user selected something
            GetParentEditView()->InsertText( GetSelectEntry() );
        }
    }
    HideAndRestoreFocus();
}

void CodeCompleteListBox::SetMatchingEntries()
{
    for(sal_Int32 i=0; i< GetEntryCount(); ++i)
    {
        OUString sEntry = GetEntry(i);
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
    if( (( aChar >= KEY_A ) && ( aChar <= KEY_Z ))
        || ((aChar >= KEY_0) && (aChar <= KEY_9)) )
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
            case KEY_RIGHT:
            {
                TextSelection aTextSelection( GetParentEditView()->GetSelection() );
                if( aTextSelection.GetEnd().GetPara() != pCodeCompleteWindow->GetTextSelection().GetEnd().GetPara()-1 )
                {
                    HideAndRestoreFocus();
                }
                break;
            }
            case KEY_LEFT:
            {
                TextSelection aTextSelection( GetParentEditView()->GetSelection() );
                if( aTextSelection.GetStart().GetIndex()-1 < pCodeCompleteWindow->GetTextSelection().GetStart().GetIndex() )
                {//leave the cursor where it is
                    HideAndRestoreFocus();
                }
                break;
            }
            case KEY_TAB:
            {
                TextSelection aTextSelection = pCodeCompleteWindow->pParent->GetLastHighlightPortionTextSelection();
                OUString sTypedText = pCodeCompleteWindow->pParent->GetEditEngine()->GetText(aTextSelection);
                if( !aFuncBuffer.isEmpty() )
                {
                    sal_Int32 nInd = GetSelectEntryPos();
                    if( nInd != LISTBOX_ENTRY_NOTFOUND )
                    {//if there is something selected
                        bool bFound = false;
                        if( nInd == GetEntryCount() )
                            nInd = 0;
                        for( sal_Int32 i = nInd; i != GetEntryCount(); ++i )
                        {
                            OUString sEntry = GetEntry(i);
                            if( sEntry.startsWithIgnoreAsciiCase( aFuncBuffer.toString() )
                                && (aFuncBuffer.toString() != sTypedText) && (i != nInd) )
                            {
                                SelectEntry( sEntry );
                                bFound = true;
                                break;
                            }
                        }
                        if( !bFound )
                            SetMatchingEntries();

                        GetParentEditView()->SetSelection( aTextSelection );
                        GetParentEditView()->DeleteSelected();
                        GetParentEditView()->InsertText( GetSelectEntry() );
                    }
                }
                break;
            }
            case KEY_SPACE:
                HideAndRestoreFocus();
                break;
            case KEY_BACKSPACE: case KEY_DELETE:
                if( !aFuncBuffer.toString().isEmpty() )
                {
                    //if there was something inserted by tab: add it to aFuncBuffer
                    TextSelection aSel( GetParentEditView()->GetSelection() );
                    TextPaM aEnd( GetParentEditView()->CursorEndOfLine(pCodeCompleteWindow->GetTextSelection().GetEnd()) );
                    GetParentEditView()->SetSelection(TextSelection(pCodeCompleteWindow->GetTextSelection().GetStart(), aEnd ) );
                    OUString aTabInsertedStr( GetParentEditView()->GetSelected() );
                    GetParentEditView()->SetSelection( aSel );

                    if( !aTabInsertedStr.isEmpty() && aTabInsertedStr != aFuncBuffer.toString() )
                    {
                        aFuncBuffer.makeStringAndClear();
                        aFuncBuffer = aFuncBuffer.append(aTabInsertedStr);
                    }
                    aFuncBuffer = aFuncBuffer.remove(aFuncBuffer.getLength()-1, 1);
                    SetMatchingEntries();
                }
                else
                    pCodeCompleteWindow->ClearAndHide();
                break;
            case KEY_RETURN:
                InsertSelectedEntry();
                break;
            case KEY_UP: case KEY_DOWN:
                NotifyEvent nEvt( MouseNotifyEvent::KEYINPUT, nullptr, &rKeyEvt );
                PreNotify(nEvt);
                break;
        }
    }
    ListBox::KeyInput(rKeyEvt);
}

void CodeCompleteListBox::HideAndRestoreFocus()
{
    pCodeCompleteWindow->Hide();
    pCodeCompleteWindow->pParent->GrabFocus();
}

CodeCompleteWindow::CodeCompleteWindow( EditorWindow* pPar )
: Window( pPar ),
pParent( pPar ),
pListBox( VclPtr<CodeCompleteListBox>::Create(this) )
{
    SetSizePixel( Size(151,151) ); //default, later it changes
    InitListBox();
}

CodeCompleteWindow::~CodeCompleteWindow()
{
    disposeOnce();
}

void CodeCompleteWindow::dispose()
{
    pListBox.disposeAndClear();
    pParent.clear();
    vcl::Window::dispose();
}

void CodeCompleteWindow::InitListBox()
{
    pListBox->SetSizePixel( Size(150,150) ); //default, this will adopt the line length
    pListBox->Show();
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


void CodeCompleteWindow::ResizeAndPositionListBox()
{
    if( pListBox->GetEntryCount() >= 1 )
    {// if there is at least one element inside
        // calculate basic position: under the current line
        Rectangle aRect = static_cast<TextEngine*>(pParent->GetEditEngine())->PaMtoEditCursor( pParent->GetEditView()->GetSelection().GetEnd() );
        long nViewYOffset = pParent->GetEditView()->GetStartDocPos().Y();
        Point aPos = aRect.BottomRight();// this variable will be used later (if needed)
        aPos.Y() = (aPos.Y() - nViewYOffset) + nBasePad;

        OUString aLongestEntry = pListBox->GetEntry( 0 );// grab the longest one: max search
        for( sal_Int32 i=1; i< pListBox->GetEntryCount(); ++i )
        {
            if( pListBox->GetEntry( i ).getLength() > aLongestEntry.getLength() )
                aLongestEntry = pListBox->GetEntry( i );
        }
        // get column/line count
        const sal_uInt16& nColumns = aLongestEntry.getLength();
        const sal_uInt16  nLines = static_cast<sal_uInt16>( std::min( (sal_Int32) 6, pListBox->GetEntryCount() ));

        Size aSize = pListBox->CalcBlockSize( nColumns, nLines );
        //set the size
        SetSizePixel( aSize );
        //1 px smaller, to see the border
        aSize.setWidth( aSize.getWidth() - 1 );
        aSize.setHeight( aSize.getHeight() - 1 );
        pListBox->SetSizePixel( aSize );

        //calculate position
        const Rectangle aVisArea( pParent->GetEditView()->GetStartDocPos(), pParent->GetOutputSizePixel() ); //the visible area
        const Point& aBottomPoint = aVisArea.BottomRight();

        if( aVisArea.TopRight().getY() + aPos.getY() + aSize.getHeight() > aBottomPoint.getY() )
        {//clipped at the bottom: move it up
            const long& nParentFontHeight = pParent->GetEditEngine()->GetFont().GetFontHeight(); //parent's font (in the IDE): needed for height
            aPos.Y() -= aSize.getHeight() + nParentFontHeight + nCursorPad;
        }

        if( aVisArea.TopLeft().getX() + aPos.getX() + aSize.getWidth() > aBottomPoint.getX() )
        {//clipped at the right side, move it a bit left
            aPos.X() -= aSize.getWidth() + aVisArea.TopLeft().getX();
        }
        //set the position
        SetPosPixel( aPos );
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
    ClearListBox();
    pListBox->HideAndRestoreFocus();
}

UnoTypeCodeCompletetor::UnoTypeCodeCompletetor( const std::vector< OUString >& aVect, const OUString& sVarType )
: bCanComplete( true )
{
    if( aVect.empty() || sVarType.isEmpty() )
    {
        bCanComplete = false;//invalid parameters, nothing to code complete
        return;
    }

    try
    {
        // Get the base class for reflection:
        xClass = css::reflection::theCoreReflection::get(
            comphelper::getProcessComponentContext())->forName(sVarType);
    }
    catch( const Exception& )
    {
        bCanComplete = false;
        return;
    }

    unsigned int j = 1;//start from aVect[1]: aVect[0] is the variable name
    OUString sMethName;

    while( j != aVect.size() )
    {
        sMethName = aVect[j];

        if( CodeCompleteOptions::IsExtendedTypeDeclaration() )
        {
            if( !CheckMethod(sMethName) && !CheckField(sMethName) )
            {
                bCanComplete = false;
                break;
            }
        }
        else
        {
            if( !CheckField(sMethName) )
            {
                bCanComplete = false;
                break;
            }
        }

        ++j;
    }
}

std::vector< OUString > UnoTypeCodeCompletetor::GetXIdlClassMethods() const
{
    std::vector< OUString > aRetVect;
    if( bCanComplete && ( xClass != nullptr ) )
    {
        Sequence< Reference< reflection::XIdlMethod > > aMethods = xClass->getMethods();
        if( aMethods.getLength() != 0 )
        {
            for(sal_Int32 l = 0; l < aMethods.getLength(); ++l)
            {
                aRetVect.push_back( OUString(aMethods[l]->getName()) );
            }
        }
    }
    return aRetVect;//this is empty when cannot code complete
}

std::vector< OUString > UnoTypeCodeCompletetor::GetXIdlClassFields() const
{
    std::vector< OUString > aRetVect;
    if( bCanComplete && ( xClass != nullptr ) )
    {
        Sequence< Reference< reflection::XIdlField > > aFields = xClass->getFields();
        if( aFields.getLength() != 0 )
        {
            for(sal_Int32 l = 0; l < aFields.getLength(); ++l)
            {
                aRetVect.push_back( OUString(aFields[l]->getName()) );
            }
        }
    }
    return aRetVect;//this is empty when cannot code complete
}


bool UnoTypeCodeCompletetor::CheckField( const OUString& sFieldName )
{// modifies xClass!!!

    if ( xClass == nullptr )
        return false;

    Reference< reflection::XIdlField> xField = xClass->getField( sFieldName );
    if( xField != nullptr )
    {
        xClass = xField->getType();
        if( xClass != nullptr )
        {
            return true;
        }
    }
    return false;
}

bool UnoTypeCodeCompletetor::CheckMethod( const OUString& sMethName )
{// modifies xClass!!!


    if ( xClass == nullptr )
        return false;

    Reference< reflection::XIdlMethod> xMethod = xClass->getMethod( sMethName );
    if( xMethod != nullptr ) //method OK, check return type
    {
        xClass = xMethod->getReturnType();
        if( xClass != nullptr )
        {
            return true;
        }
    }
    return false;
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
