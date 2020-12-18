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
#include <string_view>

#include <helpids.h>
#include <iderid.hxx>
#include <strings.hrc>
#include <bitmaps.hlst>

#include "baside2.hxx"
#include "brkdlg.hxx"
#include <basidesh.hxx>
#include <basobj.hxx>
#include <iderdll.hxx>

#include <basic/sbmeth.hxx>
#include <basic/sbuno.hxx>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XPropertiesChangeListener.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/script/XLibraryContainer2.hpp>
#include <comphelper/string.hxx>
#include <officecfg/Office/Common.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/progress.hxx>
#include <sfx2/viewfrm.hxx>
#include <tools/debug.hxx>
#include <vcl/weld.hxx>
#include <vcl/weldutils.hxx>
#include <svl/urihelper.hxx>
#include <svx/svxids.hrc>
#include <vcl/commandevent.hxx>
#include <vcl/xtextedt.hxx>
#include <vcl/textview.hxx>
#include <vcl/txtattr.hxx>
#include <vcl/settings.hxx>
#include <vcl/ptrstyle.hxx>
#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/taskpanelist.hxx>
#include <vcl/help.hxx>
#include <cppuhelper/implbase.hxx>
#include <vector>
#include <com/sun/star/reflection/theCoreReflection.hpp>
#include <unotools/charclass.hxx>
#include "textwindowpeer.hxx"
#include "uiobject.hxx"

namespace basctl
{

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace
{

sal_uInt16 const NoMarker = 0xFFFF;
tools::Long const nBasePad = 2;
tools::Long const nCursorPad = 5;

tools::Long nVirtToolBoxHeight;    // inited in WatchWindow, used in Stackwindow

// Returns pBase converted to SbxVariable if valid and is not an SbxMethod.
SbxVariable* IsSbxVariable (SbxBase* pBase)
{
    if (SbxVariable* pVar = dynamic_cast<SbxVariable*>(pBase))
        if (!dynamic_cast<SbxMethod*>(pVar))
            return pVar;
    return nullptr;
}

Image GetImage(const OUString& rId)
{
    return Image(StockImage::Yes, rId);
}

int const nScrollLine = 12;
int const nScrollPage = 60;
int const DWBORDER = 3;

std::u16string_view const cSuffixes = u"%&!#@$";

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
    std::size_t nSize = aMemStream.Tell();
    OUString aText( static_cast<const char*>(aMemStream.GetData()),
        nSize, RTL_TEXTENCODING_UTF8 );
    return aText;
}

void setTextEngineText (ExtTextEngine& rEngine, std::u16string_view aStr)
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

void lcl_DrawIDEWindowFrame(DockingWindow const * pWin, vcl::RenderContext& rRenderContext)
{
    if (pWin->IsFloatingMode())
        return;

    Size aSz(pWin->GetOutputSizePixel());
    const Color aOldLineColor(rRenderContext.GetLineColor());
    rRenderContext.SetLineColor(COL_WHITE);
    // White line on top
    rRenderContext.DrawLine(Point(0, 0), Point(aSz.Width(), 0));
    // Black line at bottom
    rRenderContext.SetLineColor(COL_BLACK);
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
        if (nIndexEnd != -1)
        {
            rIndex = rVar.copy(nIndexStart + 1, nIndexEnd - nIndexStart - 1);
            rVar = rVar.copy(0, nIndexStart);
            rVar = comphelper::string::stripEnd(rVar, ' ');
            rIndex = comphelper::string::strip(rIndex, ' ');
        }
    }

    if ( !rVar.isEmpty() )
    {
        sal_uInt16 nLastChar = rVar.getLength()-1;
        if ( cSuffixes.find(rVar[ nLastChar ] ) != std::u16string_view::npos )
            rVar = rVar.replaceAt( nLastChar, 1, "" );
    }
    if ( !rIndex.isEmpty() )
    {
        sal_uInt16 nLastChar = rIndex.getLength()-1;
        if ( cSuffixes.find(rIndex[ nLastChar ] ) != std::u16string_view::npos )
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
    virtual ~ChangesListener() override {}

    virtual void SAL_CALL disposing(lang::EventObject const &) override
    {
        osl::MutexGuard g(editor_.mutex_);
        editor_.notifier_.clear();
    }

    virtual void SAL_CALL propertiesChange(
        Sequence< beans::PropertyChangeEvent > const &) override
    {
        SolarMutexGuard g;
        editor_.ImplSetFont();
    }

    EditorWindow & editor_;
};

class EditorWindow::ProgressInfo : public SfxProgress
{
public:
    ProgressInfo (SfxObjectShell* pObjSh, OUString const& rText, sal_uInt32 nRange) :
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
    m_nSetSourceInBasicId(nullptr),
    aHighlighter(HighlighterLanguage::Basic),
    bHighlighting(false),
    bDoSyntaxHighlight(true),
    bDelayHighlight(true),
    pCodeCompleteWnd(VclPtr<CodeCompleteWindow>::Create(this))
{
    set_id("EditorWindow");
    SetBackground(Wallpaper(rModulWindow.GetLayout().GetSyntaxBackgroundColor()));
    SetPointer( PointerStyle::Text );
    SetHelpId( HID_BASICIDE_EDITORWINDOW );

    listener_ = new ChangesListener(*this);
    Reference< beans::XMultiPropertySet > n(
        officecfg::Office::Common::Font::SourceViewFont::get(),
        UNO_QUERY_THROW);
    {
        osl::MutexGuard g(mutex_);
        notifier_ = n;
    }
    const Sequence<OUString> aPropertyNames{"FontHeight", "FontName"};
    n->addPropertiesChangeListener(aPropertyNames, listener_.get());
}


EditorWindow::~EditorWindow()
{
    disposeOnce();
}

void EditorWindow::dispose()
{
    if (m_nSetSourceInBasicId)
    {
        Application::RemoveUserEvent(m_nSetSourceInBasicId);
        m_nSetSourceInBasicId = nullptr;
    }

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
            tools::Rectangle aHelpRect;
            if ( StarBASIC::IsRunning() )
            {
                Point aWindowPos = rHEvt.GetMousePosPixel();
                aWindowPos = ScreenToOutputPixel( aWindowPos );
                Point aDocPos = GetEditView()->GetDocPos( aWindowPos );
                TextPaM aCursor = GetEditView()->GetTextEngine()->GetPaM(aDocPos);
                TextPaM aStartOfWord;
                OUString aWord = GetEditView()->GetTextEngine()->GetWord( aCursor, &aStartOfWord );
                if ( !aWord.isEmpty() && !comphelper::string::isdigitAsciiString(aWord) )
                {
                    sal_uInt16 nLastChar = aWord.getLength() - 1;
                    if ( cSuffixes.find(aWord[ nLastChar ] ) != std::u16string_view::npos )
                        aWord = aWord.replaceAt( nLastChar, 1, "" );
                    SbxBase* pSBX = StarBASIC::FindSBXInCurrentScope( aWord );
                    if (SbxVariable const* pVar = IsSbxVariable(pSBX))
                    {
                        SbxDataType eType = pVar->GetType();
                        if ( static_cast<sal_uInt8>(eType) == sal_uInt8(SbxOBJECT) )
                            // might cause a crash e. g. at the selections-object
                            // Type == Object does not mean pVar == Object!
                            ; // aHelpText = ((SbxObject*)pVar)->GetClassName();
                        else if ( eType & SbxARRAY )
                            ; // aHelpText = "{...}";
                        else if ( static_cast<sal_uInt8>(eType) != sal_uInt8(SbxEMPTY) )
                        {
                            aHelpText = pVar->GetName();
                            if ( aHelpText.isEmpty() )     // name is not copied with the passed parameters
                                aHelpText = aWord;
                            aHelpText += "=" + pVar->GetOUString();
                        }
                    }
                    if ( !aHelpText.isEmpty() )
                    {
                        tools::Rectangle aStartWordRect(GetEditView()->GetTextEngine()->PaMtoEditCursor(aStartOfWord));
                        TextPaM aEndOfWord(aStartOfWord.GetPara(), aStartOfWord.GetIndex() + aWord.getLength());
                        tools::Rectangle aEndWordRect(GetEditView()->GetTextEngine()->PaMtoEditCursor(aEndOfWord));
                        aHelpRect = aStartWordRect.GetUnion(aEndWordRect);

                        Point aTopLeft = GetEditView()->GetWindowPos(aHelpRect.TopLeft());
                        aTopLeft = GetEditView()->GetWindow()->OutputToScreenPixel(aTopLeft);

                        aHelpRect.setX(aTopLeft.X());
                        aHelpRect.setY(aTopLeft.Y());
                    }
                }
            }
            Help::ShowQuickHelp( this, aHelpRect, aHelpText, QuickHelpFlags::NONE);
            bDone = true;
        }
    }

    if ( !bDone )
        Window::RequestHelp( rHEvt );
}


void EditorWindow::Resize()
{
    // ScrollBars, etc. happens in Adjust...
    if ( !pEditView )
        return;

    tools::Long nVisY = pEditView->GetStartDocPos().Y();

    pEditView->ShowCursor();
    Size aOutSz( GetOutputSizePixel() );
    tools::Long nMaxVisAreaStart = pEditView->GetTextEngine()->GetTextHeight() - aOutSz.Height();
    if ( nMaxVisAreaStart < 0 )
        nMaxVisAreaStart = 0;
    if ( pEditView->GetStartDocPos().Y() > nMaxVisAreaStart )
    {
        Point aStartDocPos( pEditView->GetStartDocPos() );
        aStartDocPos.setY( nMaxVisAreaStart );
        pEditView->SetStartDocPos( aStartDocPos );
        pEditView->ShowCursor();
        rModulWindow.GetBreakPointWindow().GetCurYOffset() = aStartDocPos.Y();
        rModulWindow.GetLineNumberWindow().GetCurYOffset() = aStartDocPos.Y();
    }
    InitScrollBars();
    if ( nVisY != pEditView->GetStartDocPos().Y() )
        Invalidate();
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
        if (pEditView->GetSelection() != pCodeCompleteWnd->GetTextSelection())
        {
            //selection changed, code complete window should be hidden
            pCodeCompleteWnd->HideAndRestoreFocus();
        }
    }
}

void EditorWindow::Command( const CommandEvent& rCEvt )
{
    if ( !pEditView )
        return;

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

bool EditorWindow::ImpCanModify()
{
    bool bCanModify = true;
    if ( StarBASIC::IsRunning() && rModulWindow.GetBasicStatus().bIsRunning )
    {
        // If in Trace-mode, abort the trace or refuse input
        // Remove markers in the modules in Notify at Basic::Stopped
        std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(nullptr,
                                                       VclMessageType::Question, VclButtonsType::OkCancel,
                                                       IDEResId(RID_STR_WILLSTOPPRG)));
        if (xQueryBox->run() == RET_OK)
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

    if (pCodeCompleteWnd->IsVisible() && CodeCompleteOptions::IsCodeCompleteOn())
    {
        if (pCodeCompleteWnd->HandleKeyInput(rKEvt))
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
        for (auto const& portion : aPortions)
        {
            if( portion.nEnd == nIndex )
            {
                r = portion;
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
    if( r.tokenType != TokenType::Identifier )
        return;

// correct variables
    if( !aCodeCompleteCache.GetCorrectCaseVarName( sStr, sActSubName ).isEmpty() )
    {
        sStr = aCodeCompleteCache.GetCorrectCaseVarName( sStr, sActSubName );
        pEditEngine->ReplaceText( sTextSelection, sStr );
        pEditView->SetSelection( aSel );
    }
    else
    {
        //autocorrect procedures
        SbxArray* pArr = rModulWindow.GetSbModule()->GetMethods().get();
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

TextSelection EditorWindow::GetLastHighlightPortionTextSelection() const
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
        for (auto const& portion : aPortions)
        {
            if( portion.nEnd == nIndex )
            {
                r = portion;
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
            {//at least 3 tokens: (sub|function) whitespace identifier...
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

bool EditorWindow::GetProcedureName(OUString const & rLine, OUString& rProcType, OUString& rProcName) const
{
    std::vector<HighlightPortion> aPortions;
    aHighlighter.getHighlightPortions(rLine, aPortions);

    if( aPortions.empty() )
        return false;

    bool bFoundType = false;
    bool bFoundName = false;

    for (auto const& portion : aPortions)
    {
        OUString sTokStr = rLine.copy(portion.nBegin, portion.nEnd - portion.nBegin);

        if( portion.tokenType == TokenType::Keywords && ( sTokStr.equalsIgnoreAsciiCase("sub")
            || sTokStr.equalsIgnoreAsciiCase("function")) )
        {
            rProcType = sTokStr;
            bFoundType = true;
        }
        if( portion.tokenType == TokenType::Identifier && bFoundType )
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
    if( aPortions.empty() )
        return;

    //use the syntax highlighter to grab out nested reflection calls, eg. aVar.aMethod("aa").aOtherMethod ..
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

    if( !aTypeCompletor.CanCodeComplete() )
        return;

    std::vector< OUString > aEntryVect;//entries to be inserted into the list
    std::vector< OUString > aFieldVect = aTypeCompletor.GetXIdlClassFields();//fields
    aEntryVect.insert(aEntryVect.end(), aFieldVect.begin(), aFieldVect.end() );
    if( CodeCompleteOptions::IsExtendedTypeDeclaration() )
    {// if extended types on, reflect classes, else just the structs (XIdlClass without methods)
        std::vector< OUString > aMethVect = aTypeCompletor.GetXIdlClassMethods();//methods
        aEntryVect.insert(aEntryVect.end(), aMethVect.begin(), aMethVect.end() );
    }
    if( !aEntryVect.empty() )
        SetupAndShowCodeCompleteWnd( aEntryVect, aSel );
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

void EditorWindow::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    if (!pEditEngine)     // We need it now at latest
        CreateEditEngine();

    pEditView->Paint(rRenderContext, rRect);
}

void EditorWindow::LoseFocus()
{
    // tdf#114258 wait until the next event loop cycle to do this so it doesn't
    // happen during a mouse down/up selection in the treeview whose contents
    // this may update
    if (!m_nSetSourceInBasicId)
        m_nSetSourceInBasicId = Application::PostUserEvent(LINK(this, EditorWindow, SetSourceInBasicHdl));
    Window::LoseFocus();
}

IMPL_LINK_NOARG(EditorWindow, SetSourceInBasicHdl, void*, void)
{
    m_nSetSourceInBasicId = nullptr;
    SetSourceInBasic();
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
    pEditView.reset(new TextView(pEditEngine.get(), this));
    pEditView->SetAutoIndentMode(true);
    pEditEngine->SetUpdateMode(false);
    pEditEngine->InsertView(pEditView.get());

    ImplSetFont();

    aSyntaxIdle.SetInvokeHandler( LINK( this, EditorWindow, SyntaxTimerHdl ) );
    aSyntaxIdle.SetDebugName( "basctl EditorWindow aSyntaxIdle" );

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
                                     IDEResId(RID_STR_GENERATESOURCE),
                                     nLines * 4));
    setTextEngineText(*pEditEngine, aOUSource);

    pEditView->SetStartDocPos(Point(0, 0));
    pEditView->SetSelection(TextSelection());
    rModulWindow.GetBreakPointWindow().GetCurYOffset() = 0;
    rModulWindow.GetLineNumberWindow().GetCurYOffset() = 0;
    pEditEngine->SetUpdateMode(true);
    rModulWindow.PaintImmediately();   // has only been invalidated at UpdateMode = true

    pEditView->ShowCursor();

    StartListening(*pEditEngine);

    aSyntaxIdle.Stop();
    bDoSyntaxHighlight = bWasDoSyntaxHighlight;

    for (sal_Int32 nLine = 0; nLine < nLines; nLine++)
        aSyntaxLineTable.insert(nLine);
    ForceSyntaxTimeout();

    pProgress.reset();

    pEditEngine->SetModified( false );
    pEditEngine->EnableUndo( true );

    InitScrollBars();

    if (SfxBindings* pBindings = GetBindingsPtr())
    {
        pBindings->Invalidate(SID_BASICIDE_STAT_POS);
        pBindings->Invalidate(SID_BASICIDE_STAT_TITLE);
    }

    DBG_ASSERT(rModulWindow.GetBreakPointWindow().GetCurYOffset() == 0, "CreateEditEngine: breakpoints moved?");

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

void EditorWindow::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint )
{
    TextHint const* pTextHint = dynamic_cast<TextHint const*>(&rHint);
    if (!pTextHint)
        return;

    TextHint const& rTextHint = *pTextHint;
    if( rTextHint.GetId() == SfxHintId::TextViewScrolled )
    {
        if ( rModulWindow.GetHScrollBar() )
            rModulWindow.GetHScrollBar()->SetThumbPos( pEditView->GetStartDocPos().X() );
        rModulWindow.GetEditVScrollBar().SetThumbPos( pEditView->GetStartDocPos().Y() );
        rModulWindow.GetBreakPointWindow().DoScroll
            ( rModulWindow.GetBreakPointWindow().GetCurYOffset() - pEditView->GetStartDocPos().Y() );
        rModulWindow.GetLineNumberWindow().DoScroll
            ( rModulWindow.GetLineNumberWindow().GetCurYOffset() - pEditView->GetStartDocPos().Y() );
    }
    else if( rTextHint.GetId() == SfxHintId::TextHeightChanged )
    {
        if ( pEditView->GetStartDocPos().Y() )
        {
            tools::Long nOutHeight = GetOutputSizePixel().Height();
            tools::Long nTextHeight = pEditEngine->GetTextHeight();
            if ( nTextHeight < nOutHeight )
                pEditView->Scroll( 0, pEditView->GetStartDocPos().Y() );

            rModulWindow.GetLineNumberWindow().Invalidate();
        }

        SetScrollBarRanges();
    }
    else if( rTextHint.GetId() == SfxHintId::TextFormatted )
    {
        if ( rModulWindow.GetHScrollBar() )
        {
            const tools::Long nWidth = pEditEngine->CalcTextWidth();
            if ( nWidth != nCurTextWidth )
            {
                nCurTextWidth = nWidth;
                rModulWindow.GetHScrollBar()->SetRange( Range( 0, nCurTextWidth-1) );
                rModulWindow.GetHScrollBar()->SetThumbPos( pEditView->GetStartDocPos().X() );
            }
        }
        tools::Long nPrevTextWidth = nCurTextWidth;
        nCurTextWidth = pEditEngine->CalcTextWidth();
        if ( nCurTextWidth != nPrevTextWidth )
            SetScrollBarRanges();
    }
    else if( rTextHint.GetId() == SfxHintId::TextParaInserted )
    {
        ParagraphInsertedDeleted( rTextHint.GetValue(), true );
        DoDelayedSyntaxHighlight( rTextHint.GetValue() );
    }
    else if( rTextHint.GetId() == SfxHintId::TextParaRemoved )
    {
        ParagraphInsertedDeleted( rTextHint.GetValue(), false );
    }
    else if( rTextHint.GetId() == SfxHintId::TextParaContentChanged )
    {
        DoDelayedSyntaxHighlight( rTextHint.GetValue() );
    }
    else if( rTextHint.GetId() == SfxHintId::TextViewSelectionChanged )
    {
        if (SfxBindings* pBindings = GetBindingsPtr())
        {
            pBindings->Invalidate( SID_CUT );
            pBindings->Invalidate( SID_COPY );
        }
    }
}

OUString EditorWindow::GetActualSubName( sal_uLong nLine )
{
    SbxArrayRef pMethods = rModulWindow.GetSbModule()->GetMethods();
    for( sal_uInt32 i=0; i < pMethods->Count32(); i++ )
    {
        SbMethod* pMeth = dynamic_cast<SbMethod*>( pMethods->Get32( i )  );
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
    return OUString();
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
    if ( !bDoSyntaxHighlight )
        return;

    OUString aLine( pEditEngine->GetText( nLine ) );
    bool const bWasModified = pEditEngine->IsModified();
    pEditEngine->RemoveAttribs( nLine );
    std::vector<HighlightPortion> aPortions;
    aHighlighter.getHighlightPortions( aLine, aPortions );

    for (auto const& portion : aPortions)
    {
        Color const aColor = rModulWindow.GetLayout().GetSyntaxColor(portion.tokenType);
        pEditEngine->SetAttrib(TextAttribFontColor(aColor), nLine, portion.nBegin, portion.nEnd);
    }

    pEditEngine->SetModified(bWasModified);
}

void EditorWindow::ChangeFontColor( Color aColor )
{
    if (pEditEngine)
    {
        vcl::Font aFont(pEditEngine->GetFont());
        aFont.SetColor(aColor);
        pEditEngine->SetFont(aFont);
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
    OUString sFontName(officecfg::Office::Common::Font::SourceViewFont::FontName::get().value_or(OUString()));
    if (sFontName.isEmpty())
    {
        vcl::Font aTmpFont(OutputDevice::GetDefaultFont(DefaultFontType::FIXED,
                                                        Application::GetSettings().GetUILanguageTag().getLanguageType(),
                                                        GetDefaultFontFlags::NONE, this));
        sFontName = aTmpFont.GetFamilyName();
    }
    Size aFontSize(0, officecfg::Office::Common::Font::SourceViewFont::FontHeight::get());
    vcl::Font aFont(sFontName, aFontSize);
    aFont.SetColor(rModulWindow.GetLayout().GetFontColor());
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
        // unfortunately I'm not sure that exactly this line does Modified()...
        if ( pProgress )
            pProgress->StepProgress();
        ImpDoHighlight( nPara );
    }
}

void EditorWindow::DoDelayedSyntaxHighlight( sal_uLong nPara )
{
    // line is only added to list, processed in TimerHdl
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

IMPL_LINK_NOARG(EditorWindow, SyntaxTimerHdl, Timer *, void)
{
    DBG_ASSERT( pEditView, "Not yet a View, but Syntax-Highlight?!" );

    bool const bWasModified = pEditEngine->IsModified();
    //pEditEngine->SetUpdateMode(false);

    bHighlighting = true;
    for (auto const& syntaxLine : aSyntaxLineTable)
    {
        DoSyntaxHighlight(syntaxLine);
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
        rModulWindow.GetBreakPoints().AdjustBreakPoints( static_cast<sal_uInt16>(nPara)+1, bInserted );

        tools::Long nLineHeight = GetTextHeight();
        Size aSz = rModulWindow.GetBreakPointWindow().GetOutputSize();
        tools::Rectangle aInvRect( Point( 0, 0 ), aSz );
        tools::Long nY = nPara*nLineHeight - rModulWindow.GetBreakPointWindow().GetCurYOffset();
        aInvRect.SetTop( nY );
        rModulWindow.GetBreakPointWindow().Invalidate( aInvRect );

        Size aLnSz(rModulWindow.GetLineNumberWindow().GetWidth(),
                   GetOutputSizePixel().Height() - 2 * DWBORDER);
        rModulWindow.GetLineNumberWindow().SetPosSizePixel(Point(DWBORDER + 19, DWBORDER), aLnSz);
        rModulWindow.GetLineNumberWindow().Invalidate();
    }
}

void EditorWindow::CreateProgress( const OUString& rText, sal_uInt32 nRange )
{
    DBG_ASSERT( !pProgress, "ProgressInfo exists already" );
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
    aSyntaxIdle.Invoke();
}

FactoryFunction EditorWindow::GetUITestFactory() const
{
    return EditorWindowUIObject::create;
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

void BreakPointWindow::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    if (SyncYOffset())
        return;

    Size const aOutSz = rRenderContext.GetOutputSize();
    tools::Long const nLineHeight = rRenderContext.GetTextHeight();

    Image const aBrk[2] =
    {
        GetImage(RID_BMP_BRKDISABLED),
        GetImage(RID_BMP_BRKENABLED)
    };

    Size const aBmpSz = rRenderContext.PixelToLogic(aBrk[1].GetSizePixel());
    Point const aBmpOff((aOutSz.Width() - aBmpSz.Width()) / 2,
                        (nLineHeight - aBmpSz.Height()) / 2);

    for (size_t i = 0, n = GetBreakPoints().size(); i < n; ++i)
    {
        BreakPoint& rBrk = GetBreakPoints().at(i);
        sal_uInt16 const nLine = rBrk.nLine - 1;
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
    tools::Long const nLineHeight = GetTextHeight();

    Image aMarker = GetImage(bErrorMarker ? std::u16string_view(u"" RID_BMP_ERRORMARKER) : std::u16string_view(u"" RID_BMP_STEPMARKER));

    Size aMarkerSz(aMarker.GetSizePixel());
    aMarkerSz = rRenderContext.PixelToLogic(aMarkerSz);
    Point aMarkerOff(0, 0);
    aMarkerOff.setX( (aOutSz.Width() - aMarkerSz.Width()) / 2 );
    aMarkerOff.setY( (nLineHeight - aMarkerSz.Height()) / 2 );

    sal_uLong nY = nMarkerPos * nLineHeight - nCurYOffset;
    Point aPos(0, nY);
    aPos += aMarkerOff;

    rRenderContext.DrawImage(aPos, aMarker);
}

void BreakPointWindow::DoScroll( tools::Long nVertScroll )
{
    nCurYOffset -= nVertScroll;
    Window::Scroll( 0, nVertScroll );
}

void BreakPointWindow::SetMarkerPos( sal_uInt16 nLine, bool bError )
{
    if ( SyncYOffset() )
        PaintImmediately();

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
        BreakPoint& rBrk = GetBreakPoints().at( i );
        sal_uInt16 nLine = rBrk.nLine-1;
        size_t nY = nLine*nLineHeight;
        if ( ( nYPos > nY ) && ( nYPos < ( nY + nLineHeight ) ) )
            return &rBrk;
    }
    return nullptr;
}

void BreakPointWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.GetClicks() == 2 )
    {
        Point aMousePos( PixelToLogic( rMEvt.GetPosPixel() ) );
        tools::Long nLineHeight = GetTextHeight();
        if(nLineHeight)
        {
            tools::Long nYPos = aMousePos.Y() + nCurYOffset;
            tools::Long nLine = nYPos / nLineHeight + 1;
            rModulWindow.ToggleBreakPoint( static_cast<sal_uLong>(nLine) );
            Invalidate();
        }
    }
}

void BreakPointWindow::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() != CommandEventId::ContextMenu )
        return;

    Point aPos( rCEvt.IsMouseEvent() ? rCEvt.GetMousePosPixel() : Point(1,1) );
    tools::Rectangle aRect(aPos, Size(1, 1));
    weld::Window* pPopupParent = weld::GetPopupParent(*this, aRect);

    std::unique_ptr<weld::Builder> xUIBuilder(Application::CreateBuilder(pPopupParent, "modules/BasicIDE/ui/breakpointmenus.ui"));

    Point aEventPos( PixelToLogic( aPos ) );
    BreakPoint* pBrk = rCEvt.IsMouseEvent() ? FindBreakPoint( aEventPos ) : nullptr;
    if ( pBrk )
    {
        // test if break point is enabled...
        std::unique_ptr<weld::Menu> xBrkPropMenu = xUIBuilder->weld_menu("breakmenu");
        xBrkPropMenu->set_active("active", pBrk->bEnabled);
        OString sCommand = xBrkPropMenu->popup_at_rect(pPopupParent, aRect);
        if (sCommand == "active")
        {
            pBrk->bEnabled = !pBrk->bEnabled;
            rModulWindow.UpdateBreakPoint( *pBrk );
            Invalidate();
        }
        else if (sCommand == "properties")
        {
            BreakPointDialog aBrkDlg(pPopupParent, GetBreakPoints());
            aBrkDlg.SetCurrentBreakPoint( *pBrk );
            aBrkDlg.run();
            Invalidate();
        }
    }
    else
    {
        std::unique_ptr<weld::Menu> xBrkListMenu = xUIBuilder->weld_menu("breaklistmenu");
        OString sCommand = xBrkListMenu->popup_at_rect(pPopupParent, aRect);
        if (sCommand == "manage")
        {
            BreakPointDialog aBrkDlg(pPopupParent, GetBreakPoints());
            aBrkDlg.run();
            Invalidate();
        }
    }
}

bool BreakPointWindow::SyncYOffset()
{
    TextView* pView = rModulWindow.GetEditView();
    if ( pView )
    {
        tools::Long nViewYOffset = pView->GetStartDocPos().Y();
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

namespace {

struct WatchItem
{
    OUString        maName;
    OUString        maDisplayName;
    SbxObjectRef    mpObject;
    std::vector<OUString> maMemberList;

    SbxDimArrayRef  mpArray;
    int             nDimLevel;  // 0 = Root
    int             nDimCount;
    std::vector<sal_Int32> vIndices;

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

}

WatchWindow::WatchWindow(Layout* pParent)
    : DockingWindow(pParent, "modules/BasicIDE/ui/dockingwatch.ui", "DockingWatch")
    , m_nUpdateWatchesId(nullptr)
{
    m_xTitleArea = m_xBuilder->weld_container("titlearea");

    nVirtToolBoxHeight = m_xTitleArea->get_preferred_size().Height();

    m_xTitle = m_xBuilder->weld_label("title");
    m_xTitle->set_label(IDEResId(RID_STR_REMOVEWATCH));

    m_xEdit = m_xBuilder->weld_entry("edit");
    m_xRemoveWatchButton = m_xBuilder->weld_button("remove");
    m_xTreeListBox = m_xBuilder->weld_tree_view("treeview");

    m_xEdit->set_accessible_name(IDEResId(RID_STR_WATCHNAME));
    m_xEdit->set_help_id(HID_BASICIDE_WATCHWINDOW_EDIT);
    m_xEdit->set_size_request(LogicToPixel(Size(80, 0), MapMode(MapUnit::MapAppFont)).Width(), -1);
    m_xEdit->connect_activate(LINK( this, WatchWindow, ActivateHdl));
    m_xEdit->connect_key_press(LINK( this, WatchWindow, KeyInputHdl));
    m_xTreeListBox->set_accessible_name(IDEResId(RID_STR_WATCHNAME));

    m_xRemoveWatchButton->set_sensitive(false);
    m_xRemoveWatchButton->connect_clicked(LINK( this, WatchWindow, ButtonHdl));
    m_xRemoveWatchButton->set_help_id(HID_BASICIDE_REMOVEWATCH);
    m_xRemoveWatchButton->set_tooltip_text(IDEResId(RID_STR_REMOVEWATCHTIP));

    m_xTreeListBox->set_help_id(HID_BASICIDE_WATCHWINDOW_LIST);
    m_xTreeListBox->connect_editing(LINK(this, WatchWindow, EditingEntryHdl),
                                    LINK(this, WatchWindow, EditedEntryHdl));
    m_xTreeListBox->connect_changed( LINK( this, WatchWindow, TreeListHdl ) );
    m_xTreeListBox->connect_expanding(LINK(this, WatchWindow, RequestingChildrenHdl));

    std::vector<int> aWidths;
    std::vector<bool> aEditables;
    aWidths.push_back(220);  // VarTabWidth
    aEditables.push_back(false);
    aWidths.push_back(100);  // ValueTabWidth
    aEditables.push_back(true);
    aWidths.push_back(1250); // TypeTabWidth
    aEditables.push_back(false);
    m_xTreeListBox->set_column_fixed_widths(aWidths);
    m_xTreeListBox->set_column_editables(aEditables);

    SetText(IDEResId(RID_STR_WATCHNAME));

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
    if (m_nUpdateWatchesId)
    {
        Application::RemoveUserEvent(m_nUpdateWatchesId);
        m_nUpdateWatchesId = nullptr;
    }

    // Destroy user data
    m_xTreeListBox->all_foreach([this](weld::TreeIter& rEntry){
        WatchItem* pItem = reinterpret_cast<WatchItem*>(m_xTreeListBox->get_id(rEntry).toInt64());
        delete pItem;
        return false;
    });

    m_xTitle.reset();
    m_xEdit.reset();
    m_xRemoveWatchButton.reset();
    m_xTitleArea.reset();
    m_xTreeListBox.reset();
    if (!IsDisposed())
        GetSystemWindow()->GetTaskPaneList()->RemoveWindow( this );
    DockingWindow::dispose();
}

void WatchWindow::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    lcl_DrawIDEWindowFrame(this, rRenderContext);
}

void WatchWindow::Resize()
{
    Size aSz = GetOutputSizePixel();
    Size aBoxSz(aSz.Width() - 2*DWBORDER, aSz.Height() - 2*DWBORDER);

    if ( aBoxSz.Width() < 4 )
        aBoxSz.setWidth( 0 );
    if ( aBoxSz.Height() < 4 )
        aBoxSz.setHeight( 0 );

    m_xVclContentArea->SetPosSizePixel(Point(DWBORDER, DWBORDER), aBoxSz);

    Invalidate();
}

WatchItem* WatchItem::GetRootItem()
{
    WatchItem* pItem = mpArrayParentItem;
    while( pItem )
    {
        if( pItem->mpArray.is() )
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
        pRet = pRootItem->mpArray.get();
    return pRet;
}

void WatchWindow::AddWatch( const OUString& rVName )
{
    OUString aVar, aIndex;
    lcl_SeparateNameAndIndex( rVName, aVar, aIndex );
    WatchItem* pWatchItem = new WatchItem(aVar);

    OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pWatchItem)));
    std::unique_ptr<weld::TreeIter> xRet = m_xTreeListBox->make_iterator();
    m_xTreeListBox->insert(nullptr, -1, &aVar, &sId, nullptr, nullptr, false, xRet.get());
    m_xTreeListBox->set_text(*xRet, "", 1);
    m_xTreeListBox->set_text(*xRet, "", 2);

    m_xTreeListBox->set_cursor(*xRet);
    m_xTreeListBox->select(*xRet);
    m_xTreeListBox->scroll_to_row(*xRet);
    m_xRemoveWatchButton->set_sensitive(true);

    UpdateWatches(false);
}

void WatchWindow::RemoveSelectedWatch()
{
    std::unique_ptr<weld::TreeIter> xEntry = m_xTreeListBox->make_iterator();
    bool bEntry = m_xTreeListBox->get_cursor(xEntry.get());
    if (bEntry)
    {
        m_xTreeListBox->remove(*xEntry);
        bEntry = m_xTreeListBox->get_cursor(xEntry.get());
        if (bEntry)
            m_xEdit->set_text(reinterpret_cast<WatchItem*>(m_xTreeListBox->get_id(*xEntry).toInt64())->maName);
        else
            m_xEdit->set_text(OUString());
        if ( !m_xTreeListBox->n_children() )
            m_xRemoveWatchButton->set_sensitive(false);
    }
}

IMPL_STATIC_LINK_NOARG(WatchWindow, ButtonHdl, weld::Button&, void)
{
    if (SfxDispatcher* pDispatcher = GetDispatcher())
        pDispatcher->Execute(SID_BASICIDE_REMOVEWATCH);
}

IMPL_LINK_NOARG(WatchWindow, TreeListHdl, weld::TreeView&, void)
{
    std::unique_ptr<weld::TreeIter> xCurEntry = m_xTreeListBox->make_iterator();
    bool bCurEntry = m_xTreeListBox->get_cursor(xCurEntry.get());
    if (!bCurEntry)
        return;
    WatchItem* pItem = reinterpret_cast<WatchItem*>(m_xTreeListBox->get_id(*xCurEntry).toInt64());
    if (!pItem)
        return;
    m_xEdit->set_text(pItem->maName);
}

IMPL_LINK_NOARG(WatchWindow, ActivateHdl, weld::Entry&, bool)
{
    OUString aCurText(m_xEdit->get_text());
    if (!aCurText.isEmpty())
    {
        AddWatch(aCurText);
        m_xEdit->select_region(0, -1);
    }
    return true;
}

IMPL_LINK(WatchWindow, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    bool bHandled = false;

    sal_uInt16 nKeyCode = rKEvt.GetKeyCode().GetCode();
    if (nKeyCode == KEY_ESCAPE)
    {
        m_xEdit->set_text(OUString());
        bHandled = true;
    }

    return bHandled;
}

// StackWindow
StackWindow::StackWindow(Layout* pParent)
    : DockingWindow(pParent, "modules/BasicIDE/ui/dockingstack.ui", "DockingStack")
{
    m_xTitle = m_xBuilder->weld_label("title");
    m_xTitle->set_label(IDEResId(RID_STR_STACK));

    m_xTitle->set_size_request(-1, nVirtToolBoxHeight); // so the two title areas are the same height

    m_xTreeListBox = m_xBuilder->weld_tree_view("stack");

    m_xTreeListBox->set_help_id(HID_BASICIDE_STACKWINDOW_LIST);
    m_xTreeListBox->set_accessible_name(IDEResId(RID_STR_STACKNAME));
    m_xTreeListBox->set_selection_mode(SelectionMode::NONE);
    m_xTreeListBox->append_text(OUString());

    SetText(IDEResId(RID_STR_STACKNAME));

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
    m_xTitle.reset();
    m_xTreeListBox.reset();
    DockingWindow::dispose();
}

void StackWindow::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    lcl_DrawIDEWindowFrame(this, rRenderContext);
}

void StackWindow::Resize()
{
    Size aSz = GetOutputSizePixel();
    Size aBoxSz(aSz.Width() - 2*DWBORDER, aSz.Height() - 2*DWBORDER);

    if ( aBoxSz.Width() < 4 )
        aBoxSz.setWidth( 0 );
    if ( aBoxSz.Height() < 4 )
        aBoxSz.setHeight( 0 );

    m_xVclContentArea->SetPosSizePixel(Point(DWBORDER, DWBORDER), aBoxSz);

    Invalidate();
}

void StackWindow::UpdateCalls()
{
    m_xTreeListBox->freeze();
    m_xTreeListBox->clear();

    if (StarBASIC::IsRunning())
    {
        ErrCode eOld = SbxBase::GetError();
        m_xTreeListBox->set_selection_mode(SelectionMode::Single);

        sal_Int32 nScope = 0;
        SbMethod* pMethod = StarBASIC::GetActiveMethod( nScope );
        while ( pMethod )
        {
            OUStringBuffer aEntry( OUString::number(nScope ));
            if ( aEntry.getLength() < 2 )
                aEntry.insert(0, " ");
            aEntry.append(": ").append(pMethod->GetName());
            SbxArray* pParams = pMethod->GetParameters();
            SbxInfo* pInfo = pMethod->GetInfo();
            if ( pParams )
            {
                aEntry.append("(");
                // 0 is the sub's name...
                for ( sal_uInt32 nParam = 1; nParam < pParams->Count32(); nParam++ )
                {
                    SbxVariable* pVar = pParams->Get32( nParam );
                    assert(pVar && "Parameter?!");
                    if ( !pVar->GetName().isEmpty() )
                    {
                        aEntry.append(pVar->GetName());
                    }
                    else if ( pInfo )
                    {
                        assert(nParam <= std::numeric_limits<sal_uInt16>::max());
                        const SbxParamInfo* pParam = pInfo->GetParam( sal::static_int_cast<sal_uInt16>(nParam) );
                        if ( pParam )
                        {
                            aEntry.append(pParam->aName);
                        }
                    }
                    aEntry.append("=");
                    SbxDataType eType = pVar->GetType();
                    if( eType & SbxARRAY )
                    {
                        aEntry.append("...");
                    }
                    else if( eType != SbxOBJECT )
                    {
                        aEntry.append(pVar->GetOUString());
                    }
                    if ( nParam < ( pParams->Count32() - 1 ) )
                    {
                        aEntry.append(", ");
                    }
                }
                aEntry.append(")");
            }
            m_xTreeListBox->append_text(aEntry.makeStringAndClear());
            nScope++;
            pMethod = StarBASIC::GetActiveMethod( nScope );
        }

        SbxBase::ResetError();
        if( eOld != ERRCODE_NONE )
            SbxBase::SetError( eOld );
    }
    else
    {
        m_xTreeListBox->set_selection_mode(SelectionMode::NONE);
        m_xTreeListBox->append_text(OUString());
    }

    m_xTreeListBox->thaw();
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
    aSz.AdjustWidth( -(2*DWBORDER) );
    aSz.AdjustHeight( -(2*DWBORDER) );
    tools::Long nBrkWidth = 20;
    tools::Long nSBWidth = aEWVScrollBar->GetSizePixel().Width();

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

IMPL_LINK(ComplexEditorWindow, ScrollHdl, ScrollBar *, pCurScrollBar, void )
{
    if (aEdtWindow->GetEditView())
    {
        DBG_ASSERT( pCurScrollBar == aEWVScrollBar.get(), "Who is scrolling?" );
        tools::Long nDiff = aEdtWindow->GetEditView()->GetStartDocPos().Y() - pCurScrollBar->GetThumbPos();
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

        xPeer = createTextWindowPeer(*GetEditView());
        SetComponentInterface(xPeer);
    }
    return xPeer;
}

static sal_uInt32 getCorrectedPropCount(SbxArray* p)
{
    sal_uInt32 nPropCount = p->Count32();
    if (nPropCount >= 3
        && p->Get32(nPropCount - 1)->GetName().equalsIgnoreAsciiCase("Dbg_Methods")
        && p->Get32(nPropCount - 2)->GetName().equalsIgnoreAsciiCase("Dbg_Properties")
        && p->Get32(nPropCount - 3)->GetName().equalsIgnoreAsciiCase("Dbg_SupportedInterfaces"))
    {
        nPropCount -= 3;
    }
    return nPropCount;
}

IMPL_LINK(WatchWindow, RequestingChildrenHdl, const weld::TreeIter&, rParent, bool)
{
    if( !StarBASIC::IsRunning() )
        return true;

    if (m_xTreeListBox->iter_has_child(rParent))
        return true;

    WatchItem* pItem = reinterpret_cast<WatchItem*>(m_xTreeListBox->get_id(rParent).toInt64());
    std::unique_ptr<weld::TreeIter> xRet = m_xTreeListBox->make_iterator();

    SbxDimArray* pArray = pItem->mpArray.get();
    SbxDimArray* pRootArray = pItem->GetRootArray();
    bool bArrayIsRootArray = false;
    if( !pArray && pRootArray )
    {
        pArray = pRootArray;
        bArrayIsRootArray = true;
    }

    SbxObject* pObj = pItem->mpObject.get();
    if( pObj )
    {
        createAllObjectProperties( pObj );
        SbxArray* pProps = pObj->GetProperties();
        const sal_uInt32 nPropCount = getCorrectedPropCount(pProps);
        pItem->maMemberList.reserve(nPropCount);

        for( sal_uInt32 i = 0 ; i < nPropCount ; ++i )
        {
            SbxVariable* pVar = pProps->Get32( i );

            pItem->maMemberList.push_back(pVar->GetName());
            OUString const& rName = pItem->maMemberList.back();

            WatchItem* pWatchItem = new WatchItem(rName);
            OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pWatchItem)));

            m_xTreeListBox->insert(&rParent, -1, &rName, &sId, nullptr, nullptr, false, xRet.get());
            m_xTreeListBox->set_text(*xRet, "", 1);
            m_xTreeListBox->set_text(*xRet, "", 2);
        }

        if (nPropCount > 0 && !m_nUpdateWatchesId)
        {
            m_nUpdateWatchesId = Application::PostUserEvent(LINK(this, WatchWindow, ExecuteUpdateWatches));
        }
    }
    else if( pArray )
    {
        sal_uInt16 nElementCount = 0;

        // Loop through indices of current level
        int nParentLevel = bArrayIsRootArray ? pItem->nDimLevel : 0;
        int nThisLevel = nParentLevel + 1;
        sal_Int32 nMin, nMax;
        if (pArray->GetDim32(nThisLevel, nMin, nMax))
        {
            for (sal_Int32 i = nMin; i <= nMax; i++)
            {
                WatchItem* pChildItem = new WatchItem(pItem->maName);

                // Copy data and create name

                OUStringBuffer aIndexStr = "(";
                pChildItem->mpArrayParentItem = pItem;
                pChildItem->nDimLevel = nThisLevel;
                pChildItem->nDimCount = pItem->nDimCount;
                pChildItem->vIndices.resize(pChildItem->nDimCount);
                sal_Int32 j;
                for (j = 0; j < nParentLevel; j++)
                {
                    sal_Int32 n = pChildItem->vIndices[j] = pItem->vIndices[j];
                    aIndexStr.append(OUString::number(n)).append(",");
                }
                pChildItem->vIndices[nParentLevel] = i;
                aIndexStr.append(OUString::number(i)).append(")");

                OUString aDisplayName;
                WatchItem* pArrayRootItem = pChildItem->GetRootItem();
                if (pArrayRootItem && pArrayRootItem->mpArrayParentItem)
                    aDisplayName = pItem->maDisplayName;
                else
                    aDisplayName = pItem->maName;
                aDisplayName += aIndexStr;
                pChildItem->maDisplayName = aDisplayName;

                OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pChildItem)));

                m_xTreeListBox->insert(&rParent, -1, &aDisplayName, &sId, nullptr, nullptr, false,
                                       xRet.get());
                m_xTreeListBox->set_text(*xRet, "", 1);
                m_xTreeListBox->set_text(*xRet, "", 2);

                nElementCount++;
            }
        }
        if (nElementCount > 0 && !m_nUpdateWatchesId)
        {
            m_nUpdateWatchesId = Application::PostUserEvent(LINK(this, WatchWindow, ExecuteUpdateWatches));
        }
    }

    return true;
}

IMPL_LINK_NOARG(WatchWindow, ExecuteUpdateWatches, void*, void)
{
    m_nUpdateWatchesId = nullptr;
    UpdateWatches();
}

SbxBase* WatchWindow::ImplGetSBXForEntry(const weld::TreeIter& rEntry, bool& rbArrayElement)
{
    SbxBase* pSBX = nullptr;
    rbArrayElement = false;

    WatchItem* pItem = reinterpret_cast<WatchItem*>(m_xTreeListBox->get_id(rEntry).toInt64());
    OUString aVName( pItem->maName );

    std::unique_ptr<weld::TreeIter> xParentEntry = m_xTreeListBox->make_iterator(&rEntry);
    bool bParentEntry = m_xTreeListBox->iter_parent(*xParentEntry);
    WatchItem* pParentItem = bParentEntry ? reinterpret_cast<WatchItem*>(m_xTreeListBox->get_id(*xParentEntry).toInt64()) : nullptr;
    if( pParentItem )
    {
        SbxObject* pObj = pParentItem->mpObject.get();
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
                pSBX = pArray->Get32(pItem->vIndices.empty() ? nullptr : &*pItem->vIndices.begin());
        }
    }
    else
    {
        pSBX = StarBASIC::FindSBXInCurrentScope( aVName );
    }
    return pSBX;
}

IMPL_LINK(WatchWindow, EditingEntryHdl, const weld::TreeIter&, rIter, bool)
{
    WatchItem* pItem = reinterpret_cast<WatchItem*>(m_xTreeListBox->get_id(rIter).toInt64());

    bool bEdit = false;
    if (StarBASIC::IsRunning() && StarBASIC::GetActiveMethod() && !SbxBase::IsError())
    {
        // No out of scope entries
        bool bArrayElement;
        SbxBase* pSbx = ImplGetSBXForEntry(rIter, bArrayElement);
        if (IsSbxVariable(pSbx) || bArrayElement)
        {
            // Accept no objects and only end nodes of arrays for editing
            if( !pItem->mpObject.is() && ( !pItem->mpArray.is() || pItem->nDimLevel == pItem->nDimCount ) )
            {
                aEditingRes = m_xTreeListBox->get_text(rIter, 1);
                aEditingRes = comphelper::string::strip(aEditingRes, ' ');
                bEdit = true;
            }
        }
    }

    return bEdit;
}

IMPL_LINK(WatchWindow, EditedEntryHdl, const IterString&, rIterString, bool)
{
    const weld::TreeIter& rIter = rIterString.first;
    OUString aResult = comphelper::string::strip(rIterString.second, ' ');

    sal_uInt16 nResultLen = aResult.getLength();
    sal_Unicode cFirst = aResult[0];
    sal_Unicode cLast  = aResult[ nResultLen - 1 ];
    if( cFirst == '\"' && cLast == '\"' )
        aResult = aResult.copy( 1, nResultLen - 2 );

    if (aResult == aEditingRes)
        return false;

    bool bArrayElement;
    SbxBase* pSBX = ImplGetSBXForEntry(rIter, bArrayElement);

    if (SbxVariable* pVar = IsSbxVariable(pSBX))
    {
        SbxDataType eType = pVar->GetType();
        if ( static_cast<sal_uInt8>(eType) != sal_uInt8(SbxOBJECT)
             && ( eType & SbxARRAY ) == 0 )
        {
            // If the type is variable, the conversion of the SBX does not matter,
            // else the string is converted.
            pVar->PutStringExt( aResult );
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

void implCollapseModifiedObjectEntry(weld::TreeIter& rParent, weld::TreeView& rTree)
{
    rTree.collapse_row(rParent);

    std::unique_ptr<weld::TreeIter> xDeleteEntry = rTree.make_iterator(&rParent);

    while (rTree.iter_children(*xDeleteEntry))
    {
        implCollapseModifiedObjectEntry(*xDeleteEntry, rTree);

        WatchItem* pItem = reinterpret_cast<WatchItem*>(rTree.get_id(*xDeleteEntry).toInt64());
        delete pItem;
        rTree.remove(*xDeleteEntry);
        rTree.copy_iterator(rParent, *xDeleteEntry);
    }
}

OUString implCreateTypeStringForDimArray( WatchItem* pItem, SbxDataType eType )
{
    OUString aRetStr = getBasicTypeName( eType );

    SbxDimArray* pArray = pItem->mpArray.get();
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
                sal_Int32 nMin, nMax;
                pArray->GetDim32( sal::static_int_cast<sal_Int32>( i+1 ), nMin, nMax );
                aRetStr += OUString::number(nMin) + " to "  + OUString::number(nMax);
                if( i < nDims - 1 )
                    aRetStr += ", ";
            }
            aRetStr += ")";
        }
    }
    return aRetStr;
}

} // namespace

void WatchWindow::implEnableChildren(const weld::TreeIter& rEntry, bool bEnable)
{
    if (bEnable)
    {
        if (!m_xTreeListBox->get_row_expanded(rEntry))
            m_xTreeListBox->set_children_on_demand(rEntry, true);
    }
    else
    {
        assert(!m_xTreeListBox->get_row_expanded(rEntry));
        m_xTreeListBox->set_children_on_demand(rEntry, false);
    }
}

void WatchWindow::UpdateWatches(bool bBasicStopped)
{
    SbMethod* pCurMethod = StarBASIC::GetActiveMethod();

    ErrCode eOld = SbxBase::GetError();
    setBasicWatchMode( true );

    m_xTreeListBox->all_foreach([this, pCurMethod, bBasicStopped](weld::TreeIter& rEntry){
        WatchItem* pItem = reinterpret_cast<WatchItem*>(m_xTreeListBox->get_id(rEntry).toInt64());
        DBG_ASSERT( !pItem->maName.isEmpty(), "Var? - Must not be empty!" );
        OUString aWatchStr;
        OUString aTypeStr;
        if ( pCurMethod )
        {
            bool bCollapse = false;
            TriState eEnableChildren = TRISTATE_INDET;

            bool bArrayElement;
            SbxBase* pSBX = ImplGetSBXForEntry(rEntry, bArrayElement);

            // Array? If no end node create type string
            if( bArrayElement && pItem->nDimLevel < pItem->nDimCount )
            {
                SbxDimArray* pRootArray = pItem->GetRootArray();
                SbxDataType eType = pRootArray->GetType();
                aTypeStr = implCreateTypeStringForDimArray( pItem, eType );
                eEnableChildren = TRISTATE_TRUE;
            }

            if (SbxVariable const* pVar = IsSbxVariable(pSBX))
            {
                // extra treatment of arrays
                SbxDataType eType = pVar->GetType();
                if ( eType & SbxARRAY )
                {
                    // consider multidimensional arrays!
                    if (SbxDimArray* pNewArray = dynamic_cast<SbxDimArray*>(pVar->GetObject()))
                    {
                        SbxDimArray* pOldArray = pItem->mpArray.get();

                        bool bArrayChanged = false;
                        if (pOldArray != nullptr)
                        {
                            // Compare Array dimensions to see if array has changed
                            // Can be a copy, so comparing pointers does not work
                            sal_Int32 nOldDims = pOldArray->GetDims32();
                            sal_Int32 nNewDims = pNewArray->GetDims32();
                            if( nOldDims != nNewDims )
                            {
                                bArrayChanged = true;
                            }
                            else
                            {
                                for( sal_Int32 i = 0 ; i < nOldDims ; i++ )
                                {
                                    sal_Int32 nOldMin, nOldMax;
                                    sal_Int32 nNewMin, nNewMax;

                                    pOldArray->GetDim32( i+1, nOldMin, nOldMax );
                                    pNewArray->GetDim32( i+1, nNewMin, nNewMax );
                                    if( nOldMin != nNewMin || nOldMax != nNewMax )
                                    {
                                        bArrayChanged = true;
                                        break;
                                    }
                                }
                            }
                        }
                        else
                        {
                            bArrayChanged = true;
                        }
                        eEnableChildren = TRISTATE_TRUE;
                        // #i37227 Clear always and replace array
                        if( pNewArray != pOldArray )
                        {
                            pItem->clearWatchItem();
                            eEnableChildren = TRISTATE_TRUE;

                            pItem->mpArray = pNewArray;
                            sal_Int32 nDims = pNewArray->GetDims32();
                            pItem->nDimLevel = 0;
                            pItem->nDimCount = nDims;
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
                else if ( static_cast<sal_uInt8>(eType) == sal_uInt8(SbxOBJECT) )
                {
                    if (SbxObject* pObj = dynamic_cast<SbxObject*>(pVar->GetObject()))
                    {
                        if ( pItem->mpObject.is() && !pItem->maMemberList.empty() )
                        {
                            createAllObjectProperties(pObj);
                            SbxArray* pProps = pObj->GetProperties();
                            const sal_uInt32 nPropCount = getCorrectedPropCount(pProps);
                            // Check if member list has changed
                            bCollapse = pItem->maMemberList.size() != nPropCount;
                            for( sal_uInt32 i = 0 ; !bCollapse && i < nPropCount ; i++ )
                            {
                                SbxVariable* pVar_ = pProps->Get32( i );
                                if( pItem->maMemberList[i] != pVar_->GetName() )
                                    bCollapse = true;
                            }
                        }

                        pItem->mpObject = pObj;
                        eEnableChildren = TRISTATE_TRUE;
                        aTypeStr = getBasicObjectTypeName( pObj );
                    }
                    else
                    {
                        aWatchStr = "Null";
                        if( pItem->mpObject.is() )
                        {
                            bCollapse = true;
                            eEnableChildren = TRISTATE_FALSE;
                        }
                    }
                }
                else
                {
                    if( pItem->mpObject.is() )
                    {
                        bCollapse = true;
                        eEnableChildren = TRISTATE_FALSE;
                    }

                    bool bString = (static_cast<sal_uInt8>(eType) == sal_uInt8(SbxSTRING));
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
                implCollapseModifiedObjectEntry(rEntry, *m_xTreeListBox);
                pItem->clearWatchItem();
            }

            if (eEnableChildren != TRISTATE_INDET)
                implEnableChildren(rEntry, eEnableChildren == TRISTATE_TRUE);
        }
        else if( bBasicStopped )
        {
            if( pItem->mpObject.is() || pItem->mpArray.is() )
            {
                implCollapseModifiedObjectEntry(rEntry, *m_xTreeListBox);
                pItem->mpObject.clear();
                pItem->mpArray.clear();
            }
            pItem->clearWatchItem();
        }

        m_xTreeListBox->set_text(rEntry, aWatchStr, 1);
        m_xTreeListBox->set_text(rEntry, aTypeStr, 2);

        return false;
    });

    SbxBase::ResetError();
    if( eOld != ERRCODE_NONE )
        SbxBase::SetError( eOld );
    setBasicWatchMode( false );
}

IMPL_LINK_NOARG(CodeCompleteWindow, ImplDoubleClickHdl, weld::TreeView&, bool)
{
    InsertSelectedEntry();
    return true;
}

IMPL_LINK_NOARG(CodeCompleteWindow, ImplSelectHdl, weld::TreeView&, void)
{
    //give back the focus to the parent
    pParent->GrabFocus();
}

TextView* CodeCompleteWindow::GetParentEditView()
{
    return pParent->GetEditView();
}

void CodeCompleteWindow::InsertSelectedEntry()
{
    OUString sSelectedEntry = m_xListBox->get_selected_text();

    if( !aFuncBuffer.isEmpty() )
    {
        // if the user typed in something: remove, and insert
        GetParentEditView()->SetSelection(pParent->GetLastHighlightPortionTextSelection());
        GetParentEditView()->DeleteSelected();

        if (!sSelectedEntry.isEmpty())
        {
            // if the user selected something
            GetParentEditView()->InsertText(sSelectedEntry);
        }
    }
    else
    {
        if (!sSelectedEntry.isEmpty())
        {
            // if the user selected something
            GetParentEditView()->InsertText(sSelectedEntry);
        }
    }
    HideAndRestoreFocus();
}

void CodeCompleteWindow::SetMatchingEntries()
{
    for (sal_Int32 i = 0, nEntryCount = m_xListBox->n_children(); i< nEntryCount; ++i)
    {
        OUString sEntry = m_xListBox->get_text(i);
        if (sEntry.startsWithIgnoreAsciiCase(aFuncBuffer.toString()))
        {
            m_xListBox->select(i);
            break;
        }
    }
}

IMPL_LINK(CodeCompleteWindow, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    return HandleKeyInput(rKEvt);
}

bool CodeCompleteWindow::HandleKeyInput( const KeyEvent& rKeyEvt )
{
    bool bHandled = true;

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
            case KEY_POINT:
                break;
            case KEY_ESCAPE: // hide, do nothing
                HideAndRestoreFocus();
                break;
            case KEY_RIGHT:
            {
                TextSelection aTextSelection( GetParentEditView()->GetSelection() );
                if( aTextSelection.GetEnd().GetPara() != GetTextSelection().GetEnd().GetPara()-1 )
                {
                    HideAndRestoreFocus();
                }
                break;
            }
            case KEY_LEFT:
            {
                TextSelection aTextSelection( GetParentEditView()->GetSelection() );
                if( aTextSelection.GetStart().GetIndex()-1 < GetTextSelection().GetStart().GetIndex() )
                {//leave the cursor where it is
                    HideAndRestoreFocus();
                }
                break;
            }
            case KEY_TAB:
            {
                TextSelection aTextSelection = pParent->GetLastHighlightPortionTextSelection();
                OUString sTypedText = pParent->GetEditEngine()->GetText(aTextSelection);
                if( !aFuncBuffer.isEmpty() )
                {
                    sal_Int32 nInd = m_xListBox->get_selected_index();
                    if (nInd != -1)
                    {
                        int nEntryCount = m_xListBox->n_children();
                        //if there is something selected
                        bool bFound = false;
                        for (sal_Int32 i = nInd; i != nEntryCount; ++i)
                        {
                            OUString sEntry = m_xListBox->get_text(i);
                            if( sEntry.startsWithIgnoreAsciiCase( aFuncBuffer.toString() )
                                && (aFuncBuffer.toString() != sTypedText) && (i != nInd) )
                            {
                                m_xListBox->select(i);
                                bFound = true;
                                break;
                            }
                        }
                        if( !bFound )
                            SetMatchingEntries();

                        GetParentEditView()->SetSelection( aTextSelection );
                        GetParentEditView()->DeleteSelected();
                        GetParentEditView()->InsertText(m_xListBox->get_selected_text());
                    }
                }
                break;
            }
            case KEY_SPACE:
                HideAndRestoreFocus();
                break;
            case KEY_BACKSPACE: case KEY_DELETE:
                if( !aFuncBuffer.isEmpty() )
                {
                    //if there was something inserted by tab: add it to aFuncBuffer
                    TextSelection aSel( GetParentEditView()->GetSelection() );
                    TextPaM aEnd( GetParentEditView()->CursorEndOfLine(GetTextSelection().GetEnd()) );
                    GetParentEditView()->SetSelection(TextSelection(GetTextSelection().GetStart(), aEnd ) );
                    OUString aTabInsertedStr( GetParentEditView()->GetSelected() );
                    GetParentEditView()->SetSelection( aSel );

                    if( !aTabInsertedStr.isEmpty() && aTabInsertedStr != aFuncBuffer.toString() )
                    {
                        aFuncBuffer = aTabInsertedStr;
                    }
                    aFuncBuffer.remove(aFuncBuffer.getLength()-1, 1);
                    SetMatchingEntries();
                }
                else
                {
                    ClearAndHide();
                    bHandled = false;
                }
                break;
            case KEY_RETURN:
                InsertSelectedEntry();
                break;
            case KEY_UP:
            {
                int nInd = m_xListBox->get_selected_index();
                if (nInd)
                    m_xListBox->select(nInd - 1);
                break;
            }
            case KEY_DOWN:
            {
                int nInd = m_xListBox->get_selected_index();
                if (nInd + 1 < m_xListBox->n_children())
                    m_xListBox->select(nInd + 1);
                break;
            }
            default:
                bHandled = false;
                break;
        }
    }

    return bHandled;
}

void CodeCompleteWindow::HideAndRestoreFocus()
{
    Hide();
    pParent->GrabFocus();
}

CodeCompleteWindow::CodeCompleteWindow(EditorWindow* pPar)
    : InterimItemWindow(pPar, "modules/BasicIDE/ui/codecomplete.ui", "CodeComplete")
    , pParent(pPar)
    , m_xListBox(m_xBuilder->weld_tree_view("treeview"))
{
    m_xListBox->connect_row_activated(LINK(this, CodeCompleteWindow, ImplDoubleClickHdl));
    m_xListBox->connect_changed(LINK(this, CodeCompleteWindow, ImplSelectHdl));
    m_xListBox->connect_key_press(LINK(this, CodeCompleteWindow, KeyInputHdl));
    m_xListBox->make_sorted();

    m_xListBox->set_size_request(150, 150); // default, this will adopt the line length
    SetSizePixel(m_xContainer->get_preferred_size());
}

CodeCompleteWindow::~CodeCompleteWindow()
{
    disposeOnce();
}

void CodeCompleteWindow::dispose()
{
    m_xListBox.reset();
    pParent.clear();
    InterimItemWindow::dispose();
}

void CodeCompleteWindow::InsertEntry( const OUString& aStr )
{
    m_xListBox->append_text(aStr);
}

void CodeCompleteWindow::ClearListBox()
{
    m_xListBox->clear();
    aFuncBuffer.setLength(0);
}

void CodeCompleteWindow::SetTextSelection( const TextSelection& aSel )
{
    m_aTextSelection = aSel;
}

void CodeCompleteWindow::ResizeAndPositionListBox()
{
    if (m_xListBox->n_children() < 1)
        return;

    // if there is at least one element inside
    // calculate basic position: under the current line
    tools::Rectangle aRect = static_cast<TextEngine*>(pParent->GetEditEngine())->PaMtoEditCursor( pParent->GetEditView()->GetSelection().GetEnd() );
    tools::Long nViewYOffset = pParent->GetEditView()->GetStartDocPos().Y();
    Point aPos = aRect.BottomRight();// this variable will be used later (if needed)
    aPos.setY( (aPos.Y() - nViewYOffset) + nBasePad );

    // get line count
    const sal_uInt16 nLines = static_cast<sal_uInt16>(std::min(6, m_xListBox->n_children()));

    m_xListBox->set_size_request(-1, m_xListBox->get_height_rows(nLines));

    Size aSize = m_xContainer->get_preferred_size();
    //set the size
    SetSizePixel( aSize );

    //calculate position
    const tools::Rectangle aVisArea( pParent->GetEditView()->GetStartDocPos(), pParent->GetOutputSizePixel() ); //the visible area
    const Point& aBottomPoint = aVisArea.BottomRight();

    if( aVisArea.TopRight().getY() + aPos.getY() + aSize.getHeight() > aBottomPoint.getY() )
    {//clipped at the bottom: move it up
        const tools::Long& nParentFontHeight = pParent->GetEditEngine()->GetFont().GetFontHeight(); //parent's font (in the IDE): needed for height
        aPos.AdjustY( -(aSize.getHeight() + nParentFontHeight + nCursorPad) );
    }

    if( aVisArea.TopLeft().getX() + aPos.getX() + aSize.getWidth() > aBottomPoint.getX() )
    {//clipped at the right side, move it a bit left
        aPos.AdjustX( -(aSize.getWidth() + aVisArea.TopLeft().getX()) );
    }
    //set the position
    SetPosPixel( aPos );
}

void CodeCompleteWindow::SelectFirstEntry()
{
    if (m_xListBox->n_children() > 0)
        m_xListBox->select(0);
}

void CodeCompleteWindow::ClearAndHide()
{
    ClearListBox();
    HideAndRestoreFocus();
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

    //start from aVect[1]: aVect[0] is the variable name
    bCanComplete = std::none_of(aVect.begin() + 1, aVect.end(), [this](const OUString& rMethName) {
        return (!CodeCompleteOptions::IsExtendedTypeDeclaration() || !CheckMethod(rMethName)) && !CheckField(rMethName); });
}

std::vector< OUString > UnoTypeCodeCompletetor::GetXIdlClassMethods() const
{
    std::vector< OUString > aRetVect;
    if( bCanComplete && ( xClass != nullptr ) )
    {
        const Sequence< Reference< reflection::XIdlMethod > > aMethods = xClass->getMethods();
        for(Reference< reflection::XIdlMethod > const & rMethod : aMethods)
        {
            aRetVect.push_back( rMethod->getName() );
        }
    }
    return aRetVect;//this is empty when cannot code complete
}

std::vector< OUString > UnoTypeCodeCompletetor::GetXIdlClassFields() const
{
    std::vector< OUString > aRetVect;
    if( bCanComplete && ( xClass != nullptr ) )
    {
        const Sequence< Reference< reflection::XIdlField > > aFields = xClass->getFields();
        for(Reference< reflection::XIdlField > const & rxField : aFields)
        {
            aRetVect.push_back( rxField->getName() );
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
