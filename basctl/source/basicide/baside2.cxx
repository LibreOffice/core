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

#include "baside2.hxx"
#include <baside3.hxx>
#include <basobj.hxx>
#include <basidesh.hxx>
#include "brkdlg.hxx"
#include <iderdll.hxx>
#include <iderid.hxx>
#include "moduldlg.hxx"
#include <sfx2/dispatch.hxx>
#include <docsignature.hxx>
#include <colorscheme.hxx>
#include <officecfg/Office/BasicIDE.hxx>

#include <helpids.h>
#include <strings.hrc>

#include <basic/basmgr.hxx>
#include <basic/basrdll.hxx>
#include <basic/sbmeth.hxx>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/script/ModuleType.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker3.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <comphelper/SetFlagContextHelper.hxx>
#include <comphelper/string.hxx>
#include <svl/srchdefs.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <sot/exchange.hxx>
#include <svl/eitem.hxx>
#include <svl/srchitem.hxx>
#include <svl/stritem.hxx>
#include <svl/visitem.hxx>
#include <svl/whiter.hxx>
#include <svx/svxids.hrc>
#include <tools/debug.hxx>
#include <utility>
#include <vcl/locktoplevels.hxx>
#include <vcl/errinf.hxx>
#include <vcl/event.hxx>
#include <vcl/print.hxx>
#include <vcl/svapp.hxx>
#include <vcl/textview.hxx>
#include <vcl/weld.hxx>
#include <vcl/xtextedt.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <cassert>
#include <osl/diagnose.h>
#include <officecfg/Office/Common.hxx>
#include <BasicColorConfig.hxx>

namespace basctl
{

namespace
{

namespace Print
{
    tools::Long const nLeftMargin = 1700;
    tools::Long const nRightMargin = 900;
    tools::Long const nTopMargin = 2000;
    tools::Long const nBottomMargin = 1000;
    tools::Long const nBorder = 300;
}

short const ValidWindow = 0x1234;

// What (who) are OW and MTF? Compare to baside3.cxx where an
// identically named variable, used in the same way, has the value
// "*.*" on Windows, "*" otherwise. Is that what should be done here,
// too?

#if defined(OW) || defined(MTF)
char const FilterMask_All[] = "*";
#else
constexpr OUString FilterMask_All = u"*.*"_ustr;
#endif

} // end anonymous namespace

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ui::dialogs;
using namespace utl;
using namespace comphelper;

namespace
{

void lcl_PrintHeader( Printer* pPrinter, sal_uInt16 nPages, sal_uInt16 nCurPage, const OUString& rTitle, bool bOutput )
{
    Size const aSz = pPrinter->GetOutputSize();

    const Color aOldLineColor( pPrinter->GetLineColor() );
    const Color aOldFillColor( pPrinter->GetFillColor() );
    const vcl::Font aOldFont( pPrinter->GetFont() );

    pPrinter->SetLineColor( COL_BLACK );
    pPrinter->SetFillColor();

    vcl::Font aFont( aOldFont );
    aFont.SetWeight( WEIGHT_BOLD );
    aFont.SetAlignment( ALIGN_BOTTOM );
    pPrinter->SetFont( aFont );

    tools::Long nFontHeight = pPrinter->GetTextHeight();

    // 1st Border => line, 2+3 Border = free space
    tools::Long nYTop = Print::nTopMargin - 3*Print::nBorder - nFontHeight;

    tools::Long nXLeft = Print::nLeftMargin - Print::nBorder;
    tools::Long nXRight = aSz.Width() - Print::nRightMargin + Print::nBorder;

    if( bOutput )
        pPrinter->DrawRect(tools::Rectangle(
            Point(nXLeft, nYTop),
            Size(nXRight - nXLeft, aSz.Height() - nYTop - Print::nBottomMargin + Print::nBorder)
        ));


    tools::Long nY = Print::nTopMargin - 2*Print::nBorder;
    Point aPos(Print::nLeftMargin, nY);
    if( bOutput )
        pPrinter->DrawText( aPos, rTitle );
    if ( nPages != 1 )
    {
        aFont.SetWeight( WEIGHT_NORMAL );
        pPrinter->SetFont( aFont );
        aPos.AdjustX(pPrinter->GetTextWidth( rTitle ) );

        if( bOutput )
        {
            OUString aPageStr = " [" + IDEResId(RID_STR_PAGE) + " " + OUString::number( nCurPage ) + "]";
            pPrinter->DrawText( aPos, aPageStr );
        }
    }

    nY = Print::nTopMargin - Print::nBorder;

    if( bOutput )
        pPrinter->DrawLine( Point( nXLeft, nY ), Point( nXRight, nY ) );

    pPrinter->SetFont( aOldFont );
    pPrinter->SetFillColor( aOldFillColor );
    pPrinter->SetLineColor( aOldLineColor );
}

void lcl_ConvertTabsToSpaces( OUString& rLine )
{
    if ( rLine.isEmpty() )
        return;

    OUStringBuffer aResult( rLine );
    sal_Int32 nPos = 0;
    sal_Int32 nMax = aResult.getLength();
    while ( nPos < nMax )
    {
        if ( aResult[nPos] == '\t' )
        {
            // not 4 Blanks, but at 4 TabPos:
            OUStringBuffer aBlanker;
            string::padToLength(aBlanker, ( 4 - ( nPos % 4 ) ), ' ');
            aResult.remove( nPos, 1 );
            aResult.insert( nPos, aBlanker );
            nMax = aResult.getLength();
        }
        ++nPos;
    }
    rLine = aResult.makeStringAndClear();
}

} // namespace

ModulWindow::ModulWindow (ModulWindowLayout* pParent, ScriptDocument const& rDocument,
                          const OUString& aLibName, const OUString& aName, OUString aModule)
    : BaseWindow(pParent, rDocument, aLibName, aName)
    , m_rLayout(*pParent)
    , m_nValid(ValidWindow)
    , m_aXEditorWindow(VclPtr<ComplexEditorWindow>::Create(this))
    , m_aModule(std::move(aModule))
{
    // Active editor color scheme
    m_sWinColorScheme = GetShell()->GetColorConfig()->GetCurrentColorSchemeName();

    m_aXEditorWindow->Show();
    SetBackground();
}

SbModuleRef const & ModulWindow::XModule()
{
    // ModuleWindows can now be created as a result of the
    // modules getting created via the api. This is a result of an
    // elementInserted event from the BasicLibrary container.
    // However the SbModule is also created from a different listener to
    // the same event ( in basmgr ) Therefore it is possible when we look
    // for m_xModule it may not yet be available, here we keep trying to access
    // the module until such time as it exists

    if ( !m_xModule.is() )
    {
        BasicManager* pBasMgr = GetDocument().getBasicManager();
        if ( pBasMgr )
        {
            StarBASIC* pBasic = pBasMgr->GetLib( GetLibName() );
            if ( pBasic )
            {
                m_xBasic = pBasic;
                m_xModule = pBasic->FindModule( GetName() );
            }
        }
    }
    return m_xModule;
}

ModulWindow::~ModulWindow()
{
    disposeOnce();
}

void ModulWindow::dispose()
{
    m_nValid = 0;
    StarBASIC::Stop();
    m_aXEditorWindow.disposeAndClear();
    BaseWindow::dispose();
}


void ModulWindow::GetFocus()
{
    if (m_nValid != ValidWindow)
        return;
    m_aXEditorWindow->GetEdtWindow().GrabFocus();
    // don't call basic calls because focus is somewhere else...
}

void ModulWindow::DoInit()
{
    GetEditorWindow().InitScrollBars();
}

void ModulWindow::Paint(vcl::RenderContext& /*rRenderContext*/, const tools::Rectangle&)
{
}

void ModulWindow::Resize()
{
    m_aXEditorWindow->SetPosSizePixel( Point( 0, 0 ), GetOutputSizePixel() );
}

void ModulWindow::CheckCompileBasic()
{
    if ( !XModule().is() )
        return;

    // never compile while running!
    bool const bRunning = StarBASIC::IsRunning();
    bool const bModified = ( !m_xModule->IsCompiled() ||
        ( GetEditEngine() && GetEditEngine()->IsModified() ) );

    if ( bRunning || !bModified )
        return;

    bool bDone = false;

    GetShell()->GetViewFrame().GetWindow().EnterWait();

    AssertValidEditEngine();
    GetEditorWindow().SetSourceInBasic();

    bool bWasModified = GetBasic()->IsModified();

    {
        // tdf#106529: only use strict compilation mode when compiling from the IDE
        css::uno::ContextLayer layer(comphelper::NewFlagContext(u"BasicStrict"_ustr));
        bDone = m_xModule->Compile();
    }
    if ( !bWasModified )
        GetBasic()->SetModified(false);

    if ( bDone )
    {
        GetBreakPoints().SetBreakPointsInBasic( m_xModule.get() );
    }

    GetShell()->GetViewFrame().GetWindow().LeaveWait();

    m_aStatus.bError = !bDone;
    m_aStatus.bIsRunning = false;
}

void ModulWindow::BasicExecute()
{
    // #116444# check security settings before macro execution
    ScriptDocument aDocument( GetDocument() );
    bool bMacrosDisabled = officecfg::Office::Common::Security::Scripting::DisableMacrosExecution::get();
    if (bMacrosDisabled || (aDocument.isDocument() && !aDocument.allowMacros()))
    {
        std::unique_ptr<weld::MessageDialog> xBox(
            Application::CreateMessageDialog(GetFrameWeld(), VclMessageType::Warning,
                                             VclButtonsType::Ok, IDEResId(RID_STR_CANNOTRUNMACRO)));
        xBox->run();
        return;
    }

    CheckCompileBasic();

    if ( !XModule().is() || !m_xModule->IsCompiled() || m_aStatus.bError )
        return;

    if ( GetBreakPoints().size() )
        m_aStatus.nBasicFlags = m_aStatus.nBasicFlags | BasicDebugFlags::Break;

    if ( !m_aStatus.bIsRunning )
    {
        DBG_ASSERT( m_xModule.is(), "No Module!" );
        AddStatus( BASWIN_RUNNINGBASIC );
        sal_uInt16 nStart, nEnd;
        TextSelection aSel = GetEditView()->GetSelection();
        // Init cursor to top
        const sal_uInt32 nCurMethodStart = aSel.GetStart().GetPara() + 1;
        SbMethod* pMethod = nullptr;
        // first Macro, else blind "Main" (ExtSearch?)
        for (sal_uInt32 nMacro = 0; nMacro < m_xModule->GetMethods()->Count(); nMacro++)
        {
            SbMethod* pM = static_cast<SbMethod*>(m_xModule->GetMethods()->Get(nMacro));
            assert(pM && "Method?");
            pM->GetLineRange( nStart, nEnd );
            if (  nCurMethodStart >= nStart && nCurMethodStart <= nEnd )
            {
                // matched a method to the cursor position
                pMethod = pM;
                break;
            }
        }
        if ( !pMethod )
        {
            // If not in a method then prompt the user
            ChooseMacro(GetFrameWeld(), uno::Reference<frame::XModel>());
            return;
        }
        pMethod->SetDebugFlags(m_aStatus.nBasicFlags);
        BasicDLL::SetDebugMode(true);
        RunMethod(pMethod);
        BasicDLL::SetDebugMode(false);
        // if cancelled during Interactive=false
        BasicDLL::EnableBreak(true);
        ClearStatus( BASWIN_RUNNINGBASIC );
    }
    else
        m_aStatus.bIsRunning = false; // cancel of Reschedule()
}

void ModulWindow::CompileBasic()
{
    CheckCompileBasic();

    XModule().is() && m_xModule->IsCompiled();
}

void ModulWindow::BasicRun()
{
    m_aStatus.nBasicFlags = BasicDebugFlags::NONE;
    BasicExecute();
}

void ModulWindow::BasicStepOver()
{
    m_aStatus.nBasicFlags = BasicDebugFlags::StepInto | BasicDebugFlags::StepOver;
    BasicExecute();
}


void ModulWindow::BasicStepInto()
{
    m_aStatus.nBasicFlags = BasicDebugFlags::StepInto;
    BasicExecute();
}

void ModulWindow::BasicStepOut()
{
    m_aStatus.nBasicFlags = BasicDebugFlags::StepOut;
    BasicExecute();
}


void ModulWindow::BasicStop()
{
    StarBASIC::Stop();
    m_aStatus.bIsRunning = false;
}

void ModulWindow::LoadBasic()
{
    sfx2::FileDialogHelper aDlg(ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE,
                                FileDialogFlags::NONE, this->GetFrameWeld());
    aDlg.SetContext(sfx2::FileDialogHelper::BasicImportSource);
    Reference<XFilePicker3> xFP = aDlg.GetFilePicker();

    xFP->appendFilter( u"BASIC"_ustr , u"*.bas"_ustr );
    xFP->appendFilter( IDEResId(RID_STR_FILTER_ALLFILES), FilterMask_All );
    xFP->setCurrentFilter( u"BASIC"_ustr );

    if( aDlg.Execute() != ERRCODE_NONE )
        return;

    Sequence< OUString > aPaths = xFP->getSelectedFiles();
    SfxMedium aMedium( aPaths[0], StreamMode::READ | StreamMode::SHARE_DENYWRITE | StreamMode::NOCREATE );
    SvStream* pStream = aMedium.GetInStream();
    if ( pStream )
    {
        AssertValidEditEngine();
        sal_uInt32 nLines = CalcLineCount( *pStream );
        // nLines*4: ReadText/Formatting/Highlighting/Formatting
        GetEditorWindow().CreateProgress( IDEResId(RID_STR_GENERATESOURCE), nLines*4 );
        GetEditEngine()->SetUpdateMode( false );
        // tdf#139196 - import macros using either default or utf-8 text encoding
        pStream->StartReadingUnicodeText(RTL_TEXTENCODING_UTF8);
        if (pStream->Tell() == 3)
            pStream->SetStreamCharSet(RTL_TEXTENCODING_UTF8);
        GetEditView()->Read( *pStream );
        GetEditEngine()->SetUpdateMode( true );
        GetEditorWindow().PaintImmediately();
        GetEditorWindow().ForceSyntaxTimeout();
        GetEditorWindow().DestroyProgress();
        ErrCodeMsg nError = aMedium.GetErrorIgnoreWarning();
        if ( nError )
            ErrorHandler::HandleError( nError );
    }
    else
    {
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                  VclMessageType::Warning, VclButtonsType::Ok, IDEResId(RID_STR_COULDNTREAD)));
        xBox->run();
    }
}


void ModulWindow::SaveBasicSource()
{
    sfx2::FileDialogHelper aDlg(ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION,
                                FileDialogFlags::NONE, this->GetFrameWeld());
    aDlg.SetContext(sfx2::FileDialogHelper::BasicExportSource);
    const Reference<XFilePicker3>& xFP = aDlg.GetFilePicker();

    xFP.queryThrow<XFilePickerControlAccess>()->setValue(ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION, 0, Any(true));

    xFP->appendFilter( u"BASIC"_ustr, u"*.bas"_ustr );
    xFP->appendFilter( IDEResId(RID_STR_FILTER_ALLFILES), FilterMask_All );
    xFP->setCurrentFilter( u"BASIC"_ustr );

    if( aDlg.Execute() != ERRCODE_NONE )
        return;

    Sequence< OUString > aPaths = xFP->getSelectedFiles();
    SfxMedium aMedium( aPaths[0], StreamMode::WRITE | StreamMode::SHARE_DENYWRITE | StreamMode::TRUNC );
    SvStream* pStream = aMedium.GetOutStream();
    if ( pStream )
    {
        EnterWait();
        AssertValidEditEngine();
        // tdf#139196 - export macros using utf-8 including BOM
        pStream->SetStreamCharSet(RTL_TEXTENCODING_UTF8);
        pStream->WriteUChar(0xEF).WriteUChar(0xBB).WriteUChar(0xBF);
        GetEditEngine()->Write( *pStream );
        aMedium.Commit();
        LeaveWait();
        ErrCodeMsg nError = aMedium.GetErrorIgnoreWarning();
        if ( nError )
            ErrorHandler::HandleError( nError );
    }
    else
    {
        std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                       VclMessageType::Warning, VclButtonsType::Ok, IDEResId(RID_STR_COULDNTWRITE)));
        xErrorBox->run();
    }
}

void ModulWindow::ImportDialog()
{
    const ScriptDocument& rDocument = GetDocument();
    OUString aLibName = GetLibName();
    implImportDialog(GetFrameWeld(), rDocument, aLibName);
}

void ModulWindow::ToggleBreakPoint( sal_uInt16 nLine )
{
    DBG_ASSERT( XModule().is(), "No Module!" );

    if ( !XModule().is() )
        return;

    CheckCompileBasic();
    if ( m_aStatus.bError )
    {
        return;
    }

    BreakPoint* pBrk = GetBreakPoints().FindBreakPoint( nLine );
    if ( pBrk ) // remove
    {
        m_xModule->ClearBP( nLine );
        GetBreakPoints().remove( pBrk );
    }
    else // create one
    {
        if ( m_xModule->SetBP( nLine ))
        {
            GetBreakPoints().InsertSorted( BreakPoint( nLine ) );
            if ( StarBASIC::IsRunning() )
            {
                for (sal_uInt32 nMethod = 0; nMethod < m_xModule->GetMethods()->Count(); nMethod++)
                {
                    SbMethod* pMethod
                        = static_cast<SbMethod*>(m_xModule->GetMethods()->Get(nMethod));
                    assert(pMethod && "Method not found! (NULL)");
                    pMethod->SetDebugFlags( pMethod->GetDebugFlags() | BasicDebugFlags::Break );
                }
            }
        }
    }
}

void ModulWindow::UpdateBreakPoint( const BreakPoint& rBrk )
{
    DBG_ASSERT( XModule().is(), "No Module!" );

    if ( XModule().is() )
    {
        CheckCompileBasic();

        if ( rBrk.bEnabled )
            m_xModule->SetBP( rBrk.nLine );
        else
            m_xModule->ClearBP( rBrk.nLine );
    }
}


void ModulWindow::BasicToggleBreakPoint()
{
    AssertValidEditEngine();

    TextSelection aSel = GetEditView()->GetSelection();
    aSel.GetStart().GetPara()++;    // Basic lines start at 1!
    aSel.GetEnd().GetPara()++;

    for ( sal_uInt32 nLine = aSel.GetStart().GetPara(); nLine <= aSel.GetEnd().GetPara(); ++nLine )
    {
        ToggleBreakPoint( nLine );
    }

    m_aXEditorWindow->GetBrkWindow().Invalidate();
}


void ModulWindow::BasicToggleBreakPointEnabled()
{
    AssertValidEditEngine();

    TextView* pView = GetEditView();
    if ( !pView )
        return;

    TextSelection aSel = pView->GetSelection();
    BreakPointList& rList = GetBreakPoints();

    for ( sal_uInt32 nLine = ++aSel.GetStart().GetPara(), nEnd = ++aSel.GetEnd().GetPara(); nLine <= nEnd; ++nLine )
    {
        BreakPoint* pBrk = rList.FindBreakPoint( nLine );
        if ( pBrk )
        {
            pBrk->bEnabled = !pBrk->bEnabled;
            UpdateBreakPoint( *pBrk );
        }
    }

    GetBreakPointWindow().Invalidate();
}

void ModulWindow::ManageBreakPoints()
{
    BreakPointWindow& rBrkWin = GetBreakPointWindow();
    BreakPointDialog aBrkDlg(rBrkWin.GetFrameWeld(), GetBreakPoints());
    aBrkDlg.run();
    rBrkWin.Invalidate();
}

void ModulWindow::BasicErrorHdl( StarBASIC const * pBasic )
{
    GetShell()->GetViewFrame().ToTop();

    // Return value: BOOL
    //  FALSE:  cancel
    //  TRUE:   go on...
    sal_uInt16 nErrorLine = StarBASIC::GetLine() - 1;
    sal_uInt16 nErrCol1 = StarBASIC::GetCol1();
    sal_uInt16 nErrCol2 = StarBASIC::GetCol2();
    if ( nErrCol2 != 0xFFFF )
        nErrCol2++;

    AssertValidEditEngine();
    GetEditView()->SetSelection( TextSelection( TextPaM( nErrorLine, nErrCol1 ), TextPaM( nErrorLine, nErrCol2 ) ) );

    // if other basic, the IDE should try to display the correct module
    bool const bMarkError = pBasic == GetBasic();
    if ( bMarkError )
        m_aXEditorWindow->GetBrkWindow().SetMarkerPos(nErrorLine, true);

    // #i47002#
    Reference< awt::XWindow > xWindow = VCLUnoHelper::GetInterface( this );

    // tdf#118572 make a currently running dialog, regardless of what its modal
    // to, insensitive to user input until after this error dialog goes away.
    TopLevelWindowLocker aBusy;
    aBusy.incBusy(nullptr);

    ErrorHandler::HandleError(StarBASIC::GetErrorCode(), GetFrameWeld());

    aBusy.decBusy();

    // #i47002#
    VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow( xWindow );
    if ( !pWindow )
        return;

    if ( bMarkError )
        m_aXEditorWindow->GetBrkWindow().SetNoMarker();
    return;
}

BasicDebugFlags ModulWindow::BasicBreakHdl()
{
    // Return value: sal_uInt16 => see SB-Debug-Flags
    sal_uInt16 nErrorLine = StarBASIC::GetLine();


    BreakPoint* pBrk = GetBreakPoints().FindBreakPoint( nErrorLine );
    if ( pBrk )
    {
        pBrk->nHitCount++;
        if ( pBrk->nHitCount <= pBrk->nStopAfter && GetBasic()->IsBreak() )
            return m_aStatus.nBasicFlags; // go on...
    }

    nErrorLine--;   // EditEngine starts at 0, Basic at 1

    AssertValidEditEngine();
    GetEditView()->SetSelection( TextSelection( TextPaM( nErrorLine, 0 ), TextPaM( nErrorLine, 0 ) ) );
    m_aXEditorWindow->GetBrkWindow().SetMarkerPos( nErrorLine );

    m_rLayout.UpdateDebug(false);

    m_aStatus.bIsInReschedule = true;
    m_aStatus.bIsRunning = true;

    AddStatus( BASWIN_INRESCHEDULE );

    InvalidateDebuggerSlots();

    while( m_aStatus.bIsRunning && !Application::IsQuit())
        Application::Yield();

    m_aStatus.bIsInReschedule = false;
    m_aXEditorWindow->GetBrkWindow().SetNoMarker();

    ClearStatus( BASWIN_INRESCHEDULE );

    return m_aStatus.nBasicFlags;
}

void ModulWindow::BasicAddWatch()
{
    AssertValidEditEngine();
    bool bAdd = true;
    if ( !GetEditView()->HasSelection() )
    {
        // tdf#57307 - expand selection to include connector punctuations
        TextSelection aSel;
        OUString aWord = GetEditEngine()->GetWord( GetEditView()->GetSelection().GetEnd(), &aSel.GetStart(), &aSel.GetEnd() );
        if ( !aWord.isEmpty() )
            GetEditView()->SetSelection( aSel );
        else
            bAdd = false;
    }
    if ( bAdd )
    {
        TextSelection aSel = GetEditView()->GetSelection();
        if ( aSel.GetStart().GetPara() == aSel.GetEnd().GetPara() ) // single line selection
            m_rLayout.BasicAddWatch(GetEditView()->GetSelected());
    }
}


void ModulWindow::EditMacro( const OUString& rMacroName )
{
    DBG_ASSERT( XModule().is(), "No Module!" );

    if ( !XModule().is() )
        return;

    CheckCompileBasic();

    if ( m_aStatus.bError )
        return;

    sal_uInt16 nStart, nEnd;
    SbMethod* pMethod = static_cast<SbMethod*>(m_xModule->Find( rMacroName, SbxClassType::Method ));
    if ( !pMethod )
        return;

    pMethod->GetLineRange( nStart, nEnd );
    if ( nStart )
    {
        nStart--;
        nEnd--;
    }
    TextSelection aSel( TextPaM( nStart, 0 ), TextPaM( nStart, 0 ) );
    AssertValidEditEngine();
    TextView * pView = GetEditView();
    // scroll if applicable so that first line is at the top
    tools::Long nVisHeight = GetOutputSizePixel().Height();
    if ( pView->GetTextEngine()->GetTextHeight() > nVisHeight )
    {
        tools::Long nMaxY = pView->GetTextEngine()->GetTextHeight() - nVisHeight;
        tools::Long nOldStartY = pView->GetStartDocPos().Y();
        tools::Long nNewStartY = static_cast<tools::Long>(nStart) * pView->GetTextEngine()->GetCharHeight();
        nNewStartY = std::min( nNewStartY, nMaxY );
        pView->Scroll( 0, -(nNewStartY-nOldStartY) );
        pView->ShowCursor( false );
        GetEditVScrollBar().SetThumbPos( pView->GetStartDocPos().Y() );
    }
    pView->SetSelection( aSel );
    pView->ShowCursor();
    pView->GetWindow()->GrabFocus();
}

void ModulWindow::StoreData()
{
    // StoreData is called when the BasicManager is destroyed or
    // this window is closed.
    // => interrupts undesired!
    GetEditorWindow().SetSourceInBasic();
}

bool ModulWindow::AllowUndo()
{
    return GetEditorWindow().CanModify();
}

void ModulWindow::UpdateData()
{
    DBG_ASSERT( XModule().is(), "No Module!" );
    // UpdateData is called when the source has changed from outside
    // => interrupts undesired!

    if ( !XModule().is() )
        return;

    SetModule( m_xModule->GetSource32() );

    if ( GetEditView() )
    {
        TextSelection aSel = GetEditView()->GetSelection();
        setTextEngineText(*GetEditEngine(), m_xModule->GetSource32());
        GetEditView()->SetSelection( aSel );
        GetEditEngine()->SetModified( false );
        MarkDocumentModified( GetDocument() );
    }
}

sal_Int32 ModulWindow::countPages( Printer* pPrinter )
{
    return FormatAndPrint( pPrinter, -1 );
}

void ModulWindow::printPage( sal_Int32 nPage, Printer* pPrinter )
{
    FormatAndPrint( pPrinter, nPage );
}

/* implementation note: this is totally inefficient for the XRenderable interface
   usage since the whole "document" will be format for every page. Should this ever
   become a problem we should
   - format only once for every new printer
   - keep an index list for each page which is the starting paragraph
*/
sal_Int32 ModulWindow::FormatAndPrint( Printer* pPrinter, sal_Int32 nPrintPage )
{
    AssertValidEditEngine();

    MapMode eOldMapMode( pPrinter->GetMapMode() );
    vcl::Font aOldFont( pPrinter->GetFont() );

    vcl::Font aFont( GetEditEngine()->GetFont() );
    aFont.SetAlignment( ALIGN_BOTTOM );
    aFont.SetTransparent( true );
    aFont.SetFontSize( Size( 0, 360 ) );
    pPrinter->SetFont( aFont );
    pPrinter->SetMapMode(MapMode(MapUnit::Map100thMM));

    OUString aTitle( CreateQualifiedName() );

    sal_Int32 nLineHeight = pPrinter->GetTextHeight();
    if(nLineHeight == 0)
    {
        nLineHeight = 1;
    }

    Size aPaperSz = pPrinter->GetOutputSize();
    aPaperSz.AdjustWidth( -(Print::nLeftMargin + Print::nRightMargin) );
    aPaperSz.AdjustHeight( -(Print::nTopMargin + Print::nBottomMargin) );

    // nLinepPage is not correct if there's a line break
    sal_Int32 nLinespPage = aPaperSz.Height()/nLineHeight;
    tools::Long nXTextWidth = pPrinter->approximate_digit_width();

    sal_Int32 nCharspLine = aPaperSz.Width() / std::max<tools::Long>(nXTextWidth, 1);
    const sal_uInt32 nParas = GetEditEngine()->GetParagraphCount();

    sal_Int32 nPages = nParas/nLinespPage+1;
    sal_Int32 nCurPage = 1;

    lcl_PrintHeader( pPrinter, nPages, nCurPage, aTitle, nPrintPage == 0 );
    Point aPos( Print::nLeftMargin, Print::nTopMargin );
    for ( sal_uInt32 nPara = 0; nPara < nParas; ++nPara )
    {
        OUString aLine( GetEditEngine()->GetText( nPara ) );
        lcl_ConvertTabsToSpaces( aLine );
        sal_Int32 nLines = aLine.getLength()/nCharspLine+1;
        for (sal_Int32 nLine = 0; nLine < nLines; ++nLine)
        {
            sal_Int32 nBeginIndex = nLine*nCharspLine;
            sal_Int32 nCopyCount = std::min<sal_Int32>(nCharspLine, aLine.getLength()-nBeginIndex);
            OUString aTmpLine = aLine.copy(nBeginIndex, nCopyCount);
            aPos.AdjustY(nLineHeight );
            if ( aPos.Y() > ( aPaperSz.Height() + Print::nTopMargin ) )
            {
                nCurPage++;
                lcl_PrintHeader( pPrinter, nPages, nCurPage, aTitle, nCurPage-1 == nPrintPage );
                aPos = Point(Print::nLeftMargin, Print::nTopMargin + nLineHeight);
            }
            if( nCurPage-1 == nPrintPage )
                pPrinter->DrawText( aPos, aTmpLine );
        }
        aPos.AdjustY(10 ); // nParaSpace
    }

    pPrinter->SetFont( aOldFont );
    pPrinter->SetMapMode( eOldMapMode );

    return nCurPage;
}

void ModulWindow::ExecuteCommand (SfxRequest& rReq)
{
    AssertValidEditEngine();
    switch (rReq.GetSlot())
    {
        case SID_DELETE:
        {
            if (!IsReadOnly())
            {
                KeyEvent aFakeDelete(0, KEY_DELETE);
                (void)GetEditView()->KeyInput(aFakeDelete);
            }
            break;
        }
        case SID_SELECTALL:
        {
            TextSelection aSel( TextPaM( 0, 0 ), TextPaM( TEXT_PARA_ALL, TEXT_INDEX_ALL ) );
            TextView * pView = GetEditView();
            pView->SetSelection( aSel );
            pView->GetWindow()->GrabFocus();
            break;
        }
        case SID_BASICRUN:
        {
            BasicRun();
        }
        break;
        case SID_BASICCOMPILE:
        {
            CompileBasic();
        }
        break;
        case SID_BASICSTEPOVER:
        {
            BasicStepOver();
        }
        break;
        case SID_BASICSTEPINTO:
        {
            BasicStepInto();
        }
        break;
        case SID_BASICSTEPOUT:
        {
            BasicStepOut();
        }
        break;
        case SID_BASICLOAD:
        {
            LoadBasic();
        }
        break;
        case SID_BASICSAVEAS:
        {
            SaveBasicSource();
        }
        break;
        case SID_IMPORT_DIALOG:
        {
            ImportDialog();
        }
        break;
        case SID_BASICIDE_MATCHGROUP:
        {
            GetEditView()->MatchGroup();
        }
        break;
        case SID_BASICIDE_TOGGLEBRKPNT:
        {
            BasicToggleBreakPoint();
        }
        break;
        case SID_BASICIDE_MANAGEBRKPNTS:
        {
            ManageBreakPoints();
        }
        break;
        case SID_BASICIDE_TOGGLEBRKPNTENABLED:
        {
            BasicToggleBreakPointEnabled();
        }
        break;
        case SID_BASICIDE_ADDWATCH:
        {
            BasicAddWatch();
        }
        break;
        case SID_BASICIDE_REMOVEWATCH:
        {
            m_rLayout.BasicRemoveWatch();
        }
        break;
        case SID_CUT:
        {
            if ( !IsReadOnly() )
            {
                GetEditView()->Cut();
                if (SfxBindings* pBindings = GetBindingsPtr())
                    pBindings->Invalidate( SID_DOC_MODIFIED );
            }
        }
        break;
        case SID_COPY:
        {
            GetEditView()->Copy();
        }
        break;
        case SID_PASTE:
        {
            if ( !IsReadOnly() )
            {
                GetEditView()->Paste();
                if (SfxBindings* pBindings = GetBindingsPtr())
                    pBindings->Invalidate( SID_DOC_MODIFIED );
            }
        }
        break;
        case SID_BASICIDE_BRKPNTSCHANGED:
        {
            GetBreakPointWindow().Invalidate();
        }
        break;
        case SID_SHOWLINES:
        {
            const SfxBoolItem* pItem = rReq.GetArg<SfxBoolItem>(rReq.GetSlot());
            bool bLineNumbers = pItem && pItem->GetValue();
            m_aXEditorWindow->SetLineNumberDisplay(bLineNumbers);

            std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
            officecfg::Office::BasicIDE::EditorSettings::LineNumbering::set(bLineNumbers, batch);
            batch->commit();
        }
        break;
        case SID_BASICIDE_DELETECURRENT:
        {
            if (QueryDelModule(m_aName, GetFrameWeld()))
            {
                // tdf#134551 don't delete the window if last module is removed until this block
                // is complete
                VclPtr<ModulWindow> xKeepRef(this);
                if (m_aDocument.removeModule(m_aLibName, m_aName))
                    MarkDocumentModified(m_aDocument);
            }
        }
        break;
        case FID_SEARCH_OFF:
            GrabFocus();
            break;
        case SID_GOTOLINE:
        {
            sal_uInt32 nCurLine = GetEditView()->GetSelection().GetStart().GetPara() + 1;
            sal_uInt32 nLineCount = GetEditEngine()->GetParagraphCount();
            GotoLineDialog aGotoDlg(GetFrameWeld(), nCurLine, nLineCount);
            if (aGotoDlg.run() == RET_OK)
            {
                if (sal_Int32 const nLine = aGotoDlg.GetLineNumber())
                {
                    TextSelection const aSel(TextPaM(nLine - 1, 0), TextPaM(nLine - 1, 0));
                    GrabFocus();
                    GetEditView()->SetSelection(aSel);
                }
            }
            break;
        }
    }
}

void ModulWindow::ExecuteGlobal (SfxRequest& rReq)
{
    switch (rReq.GetSlot())
    {
        case SID_SIGNATURE:
        {
            DocumentSignature aSignature(m_aDocument);
            if (aSignature.supportsSignatures())
            {
                aSignature.signScriptingContent(rReq.GetFrameWeld());
                if (SfxBindings* pBindings = GetBindingsPtr())
                    pBindings->Invalidate(SID_SIGNATURE);
            }
        }
        break;

        case SID_BASICIDE_STAT_POS:
        {
            GetDispatcher()->Execute(SID_GOTOLINE);
        }
        break;

        case SID_TOGGLE_COMMENT:
        {
            GetEditView()->ToggleComment();
        }
        break;
    }
}

void ModulWindow::GetState( SfxItemSet &rSet )
{
    SfxWhichIter aIter(rSet);
    for ( sal_uInt16 nWh = aIter.FirstWhich(); nWh != 0; nWh = aIter.NextWhich() )
    {
        switch ( nWh )
        {
            case SID_CUT:
            {
                if ( !GetEditView() || !GetEditView()->HasSelection() )
                    rSet.DisableItem( nWh );

                if ( IsReadOnly() )
                    rSet.DisableItem( nWh );
            }
            break;
            case SID_COPY:
            {
                if ( !GetEditView() || !GetEditView()->HasSelection() )
                    rSet.DisableItem( nWh );
            }
            break;
            case SID_PASTE:
            {
                if ( !IsPasteAllowed() )
                    rSet.DisableItem( nWh );

                if ( IsReadOnly() )
                    rSet.DisableItem( nWh );
            }
            break;
            case SID_BASICIDE_STAT_POS:
            {
                TextView* pView = GetEditView();
                if ( pView )
                {
                    TextSelection aSel = pView->GetSelection();
                    OUString aPos = IDEResId( RID_STR_LINE ) +
                                   " " +
                                   OUString::number(aSel.GetEnd().GetPara()+1) +
                                   ", " +
                                   IDEResId( RID_STR_COLUMN ) +
                                   " " +
                                   OUString::number(aSel.GetEnd().GetIndex()+1);
                    SfxStringItem aItem( SID_BASICIDE_STAT_POS, aPos );
                    rSet.Put( aItem );
                }
            }
            break;
            case SID_BASICIDE_STAT_TITLE:
            {
                // search for current procedure name (Sub or Function)
                TextView* pView = GetEditView();
                if ( pView )
                {
                    OUString sProcName;

                    TextSelection aSel = pView->GetSelection();

                    sal_uInt32 i = aSel.GetStart().GetPara();
                    do
                    {
                        OUString aCurrLine = GetEditEngine()->GetText( i );
                        OUString sProcType;
                        if (GetEditorWindow().GetProcedureName(aCurrLine, sProcType, sProcName))
                            break;
                    } while (i--);

                    OUString aTitle = CreateQualifiedName();
                    if (!sProcName.isEmpty())
                        aTitle += "." + sProcName;

                    if (IsReadOnly())
                        aTitle += " (" + IDEResId(RID_STR_READONLY) + ")";

                    SfxStringItem aTitleItem( SID_BASICIDE_STAT_TITLE, aTitle );
                    rSet.Put( aTitleItem );
                }
            }
            break;
            case SID_ATTR_INSERT:
            {
                TextView* pView = GetEditView();
                if ( pView )
                {
                    SfxBoolItem aItem( SID_ATTR_INSERT, pView->IsInsertMode() );
                    rSet.Put( aItem );
                }
            }
            break;
            case SID_SHOWLINES:
            {
                bool bLineNumbers = ::officecfg::Office::BasicIDE::EditorSettings::LineNumbering::get();
                rSet.Put(SfxBoolItem(nWh, bLineNumbers));
                break;
            }
            case SID_SELECTALL:
            {
                if ( !GetEditView() )
                    rSet.DisableItem( nWh );
            }
            break;
        }
    }
}

void ModulWindow::DoScroll( Scrollable* pCurScrollBar )
{
    if ( ( pCurScrollBar == GetHScrollBar() ) && GetEditView() )
    {
        // don't scroll with the value but rather use the Thumb-Pos for the VisArea:
        tools::Long nDiff = GetEditView()->GetStartDocPos().X() - pCurScrollBar->GetThumbPos();
        GetEditView()->Scroll( nDiff, 0 );
        GetEditView()->ShowCursor( false );
        pCurScrollBar->SetThumbPos( GetEditView()->GetStartDocPos().X() );

    }
}

bool ModulWindow::IsModified()
{
    return GetEditEngine() && GetEditEngine()->IsModified();
}

OUString ModulWindow::GetSbModuleName()
{
    OUString aModuleName;
    if ( XModule().is() )
        aModuleName = m_xModule->GetName();
    return aModuleName;
}

OUString ModulWindow::GetTitle()
{
    return GetSbModuleName();
}

void ModulWindow::ShowCursor( bool bOn )
{
    if ( GetEditEngine() )
    {
        TextView* pView = GetEditEngine()->GetActiveView();
        if ( pView )
        {
            if ( bOn )
                pView->ShowCursor();
            else
                pView->HideCursor();
        }
    }
}

void ModulWindow::AssertValidEditEngine()
{
    if ( !GetEditEngine() )
        GetEditorWindow().CreateEditEngine();
}

void ModulWindow::Activating ()
{
    bool bLineNumbers = ::officecfg::Office::BasicIDE::EditorSettings::LineNumbering::get();
    m_aXEditorWindow->SetLineNumberDisplay(bLineNumbers);
    Show();
}

void ModulWindow::Deactivating()
{
    Hide();
}

sal_uInt16 ModulWindow::StartSearchAndReplace( const SvxSearchItem& rSearchItem, bool bFromStart )
{
    if (IsSuspended())
        return 0;

    // one could also relinquish syntaxhighlighting/formatting instead of the stupid replace-everything...
    AssertValidEditEngine();
    TextView* pView = GetEditView();
    TextSelection aSel;
    if ( bFromStart )
    {
        aSel = pView->GetSelection();
        if ( !rSearchItem.GetBackward() )
            pView->SetSelection( TextSelection() );
        else
            pView->SetSelection( TextSelection( TextPaM( TEXT_PARA_ALL, TEXT_INDEX_ALL ), TextPaM( TEXT_PARA_ALL, TEXT_INDEX_ALL ) ) );
    }

    bool const bForward = !rSearchItem.GetBackward();
    sal_uInt16 nFound = 0;
    if ( ( rSearchItem.GetCommand() == SvxSearchCmd::FIND ) ||
         ( rSearchItem.GetCommand() == SvxSearchCmd::FIND_ALL ) )
    {
        nFound = pView->Search( rSearchItem.GetSearchOptions() , bForward ) ? 1 : 0;
    }
    else if ( ( rSearchItem.GetCommand() == SvxSearchCmd::REPLACE ) ||
              ( rSearchItem.GetCommand() == SvxSearchCmd::REPLACE_ALL ) )
    {
        if ( !IsReadOnly() )
        {
            bool const bAll = rSearchItem.GetCommand() == SvxSearchCmd::REPLACE_ALL;
            nFound = pView->Replace( rSearchItem.GetSearchOptions() , bAll , bForward );
        }
    }

    if ( bFromStart && !nFound )
        pView->SetSelection( aSel );

    return nFound;
}

SfxUndoManager* ModulWindow::GetUndoManager()
{
    if ( GetEditEngine() )
        return &GetEditEngine()->GetUndoManager();
    return nullptr;
}

SearchOptionFlags ModulWindow::GetSearchOptions()
{
    SearchOptionFlags nOptions = SearchOptionFlags::SEARCH |
                      SearchOptionFlags::WHOLE_WORDS |
                      SearchOptionFlags::BACKWARDS |
                      SearchOptionFlags::REG_EXP |
                      SearchOptionFlags::EXACT |
                      SearchOptionFlags::SELECTION |
                      SearchOptionFlags::SIMILARITY;

    if ( !IsReadOnly() )
    {
        nOptions |= SearchOptionFlags::REPLACE;
        nOptions |= SearchOptionFlags::REPLACE_ALL;
    }

    return nOptions;
}

void ModulWindow::BasicStarted()
{
    if ( !XModule().is() )
        return;

    m_aStatus.bIsRunning = true;
    BreakPointList& rList = GetBreakPoints();
    if ( rList.size() )
    {
        rList.ResetHitCount();
        rList.SetBreakPointsInBasic( m_xModule.get() );
        for (sal_uInt32 nMethod = 0; nMethod < m_xModule->GetMethods()->Count(); nMethod++)
        {
            SbMethod* pMethod = static_cast<SbMethod*>(m_xModule->GetMethods()->Get(nMethod));
            assert(pMethod && "Method not found! (NULL)");
            pMethod->SetDebugFlags( pMethod->GetDebugFlags() | BasicDebugFlags::Break );
        }
    }
}

void ModulWindow::BasicStopped()
{
    m_aStatus.bIsRunning = false;
    GetBreakPointWindow().SetNoMarker();
}

EntryDescriptor ModulWindow::CreateEntryDescriptor()
{
    ScriptDocument aDocument( GetDocument() );
    OUString aLibName( GetLibName() );
    LibraryLocation eLocation = aDocument.getLibraryLocation( aLibName );
    OUString aModName( GetName() );
    OUString aLibSubName;
    if( m_xBasic.is() && aDocument.isInVBAMode() && XModule().is() )
    {
        switch( m_xModule->GetModuleType() )
        {
            case script::ModuleType::DOCUMENT:
            {
                aLibSubName = IDEResId( RID_STR_DOCUMENT_OBJECTS );
                uno::Reference< container::XNameContainer > xLib = aDocument.getOrCreateLibrary( E_SCRIPTS, aLibName );
                if( xLib.is() )
                {
                    OUString sObjName;
                    ModuleInfoHelper::getObjectName( xLib, aModName, sObjName );
                    if( !sObjName.isEmpty() )
                    {
                        aModName += " (" + sObjName + ")";
                    }
                }
                break;
            }
            case script::ModuleType::FORM:
                aLibSubName = IDEResId( RID_STR_USERFORMS );
                break;
            case script::ModuleType::NORMAL:
                aLibSubName = IDEResId( RID_STR_NORMAL_MODULES );
                break;
            case script::ModuleType::CLASS:
                aLibSubName = IDEResId( RID_STR_CLASS_MODULES );
                break;
        }
    }
    return EntryDescriptor( aDocument, eLocation, aLibName, aLibSubName, aModName, OBJ_TYPE_MODULE );
}

void ModulWindow::SetReadOnly (bool b)
{
    if ( GetEditView() )
        GetEditView()->SetReadOnly( b );
}

bool ModulWindow::IsReadOnly()
{
    return GetEditView() && GetEditView()->IsReadOnly();
}

bool ModulWindow::IsPasteAllowed()
{
    bool bPaste = false;

    // get clipboard
    Reference< datatransfer::clipboard::XClipboard > xClipboard = GetClipboard();
    if ( xClipboard.is() )
    {

        Reference< datatransfer::XTransferable > xTransf = xClipboard->getContents();
        if ( xTransf.is() )
        {
            datatransfer::DataFlavor aFlavor;
            SotExchange::GetFormatDataFlavor( SotClipboardFormatId::STRING, aFlavor );
            if ( xTransf->isDataFlavorSupported( aFlavor ) )
                bPaste = true;
        }
    }

    return bPaste;
}

void ModulWindow::OnNewDocument ()
{
    bool bLineNumbers = ::officecfg::Office::BasicIDE::EditorSettings::LineNumbering::get();
    m_aXEditorWindow->SetLineNumberDisplay(bLineNumbers);
}

OUString ModulWindow::GetHid () const
{
    return HID_BASICIDE_MODULWINDOW;
}
ItemType ModulWindow::GetType () const
{
    return TYPE_MODULE;
}

bool ModulWindow::HasActiveEditor () const
{
    return !IsSuspended();
}


void ModulWindow::UpdateModule ()
{
    OUString const aModule = getTextEngineText(*GetEditEngine());

    // update module in basic
    assert(m_xModule.is());

    // update module in module window
    SetModule(aModule);

    // update module in library
    OSL_VERIFY(m_aDocument.updateModule(m_aLibName, m_aName, aModule));

    GetEditEngine()->SetModified(false);
    MarkDocumentModified(m_aDocument);
}

void ModulWindow::SetEditorColorScheme(const OUString& rColorScheme)
{
    m_sWinColorScheme = rColorScheme;
    EditorWindow& rEditWindow = GetEditorWindow();
    Wallpaper aBackgroundColor(GetLayout().GetSyntaxBackgroundColor());
    rEditWindow.SetBackground(aBackgroundColor);
    rEditWindow.GetWindow(GetWindowType::Border)->SetBackground(aBackgroundColor);
    rEditWindow.SetLineHighlightColor(GetShell()->GetColorConfig()->GetColorScheme(rColorScheme).m_aLineHighlightColor);

    // The EditEngine is created only when the module is actually opened for the first time,
    // therefore we need to check if it actually exists before updating its syntax highlighting
    ExtTextEngine* pEditEngine = GetEditEngine();
    if (pEditEngine)
        rEditWindow.UpdateSyntaxHighlighting();
}

ModulWindowLayout::ModulWindowLayout (vcl::Window* pParent, ObjectCatalog& rObjectCatalog_) :
    Layout(pParent),
    pChild(nullptr),
    aWatchWindow(VclPtr<WatchWindow>::Create(this)),
    aStackWindow(VclPtr<StackWindow>::Create(this)),
    rObjectCatalog(rObjectCatalog_)
{
    // Get active color scheme from the registry
    m_sColorSchemeId = GetShell()->GetColorConfig()->GetCurrentColorSchemeName();
    aSyntaxColors.ApplyColorScheme(m_sColorSchemeId, true);

    // Initialize the visibility of the Stack Window
    bool bStackVisible = ::officecfg::Office::BasicIDE::EditorSettings::StackWindow::get();
    if (!bStackVisible)
        aStackWindow->Show(bStackVisible);

    // Initialize the visibility of the Watched Expressions window
    bool bWatchVisible = ::officecfg::Office::BasicIDE::EditorSettings::WatchWindow::get();
    if (!bWatchVisible)
        aWatchWindow->Show(bWatchVisible);
}

ModulWindowLayout::~ModulWindowLayout()
{
    disposeOnce();
}

void ModulWindowLayout::dispose()
{
    aWatchWindow.disposeAndClear();
    aStackWindow.disposeAndClear();
    pChild.clear();
    Layout::dispose();
}

void ModulWindowLayout::UpdateDebug (bool bBasicStopped)
{
    aWatchWindow->UpdateWatches(bBasicStopped);
    aStackWindow->UpdateCalls();
}

void ModulWindowLayout::Paint (vcl::RenderContext& rRenderContext, tools::Rectangle const&)
{
    rRenderContext.DrawText(Point(), IDEResId(RID_STR_NOMODULE));
}

void ModulWindowLayout::Activating (BaseWindow& rChild)
{
    assert(dynamic_cast<ModulWindow*>(&rChild));
    pChild = &static_cast<ModulWindow&>(rChild);
    aWatchWindow->Show();
    aStackWindow->Show();
    rObjectCatalog.Show();
    rObjectCatalog.SetLayoutWindow(this);
    rObjectCatalog.UpdateEntries();
    Layout::Activating(rChild);
    aSyntaxColors.SetActiveEditor(&pChild->GetEditorWindow());
    aSyntaxColors.SetActiveColorSchemeId(m_sColorSchemeId);
}

void ModulWindowLayout::Deactivating ()
{
    aSyntaxColors.SetActiveEditor(nullptr);
    Layout::Deactivating();
    aWatchWindow->Hide();
    aStackWindow->Hide();
    rObjectCatalog.Hide();
    pChild = nullptr;
}

void ModulWindowLayout::GetState (SfxItemSet &rSet, unsigned nWhich)
{
    switch (nWhich)
    {
        case SID_SHOW_PROPERTYBROWSER:
            rSet.Put(SfxVisibilityItem(nWhich, false));
            break;

        case SID_BASICIDE_CHOOSEMACRO:
            rSet.Put(SfxVisibilityItem(nWhich, true));
            break;
    }
}

void ModulWindowLayout::BasicAddWatch (OUString const& rWatchStr)
{
    aWatchWindow->AddWatch(rWatchStr);
}

void ModulWindowLayout::BasicRemoveWatch ()
{
    aWatchWindow->RemoveSelectedWatch();
}

void ModulWindowLayout::ShowWatchWindow(bool bVisible)
{
    aWatchWindow->Show(bVisible);
    ArrangeWindows();
}

void ModulWindowLayout::ShowStackWindow(bool bVisible)
{
    aStackWindow->Show(bVisible);
    ArrangeWindows();
}

void ModulWindowLayout::OnFirstSize (tools::Long const nWidth, tools::Long const nHeight)
{
    AddToLeft(&rObjectCatalog, Size(nWidth * 0.20, nHeight * 0.75));
    AddToBottom(aWatchWindow.get(), Size(nWidth * 0.67, nHeight * 0.25));
    AddToBottom(aStackWindow.get(), Size(nWidth * 0.33, nHeight * 0.25));
}

// Applies the color scheme to the current window and updates color definitions;
// note that other ModulWindow instances are not affected by calling this method
void ModulWindowLayout::ApplyColorSchemeToCurrentWindow(const OUString& rSchemeId)
{
    // Apply new color scheme to the UI
    m_sColorSchemeId = rSchemeId;
    aSyntaxColors.ApplyColorScheme(m_sColorSchemeId, false);
}

ModulWindowLayout::SyntaxColors::SyntaxColors ()
    : pEditor(nullptr)
{
    aConfig.AddListener(this);
}

ModulWindowLayout::SyntaxColors::~SyntaxColors ()
{
    aConfig.RemoveListener(this);
}

// virtual
void ModulWindowLayout::SyntaxColors::ConfigurationChanged (utl::ConfigurationBroadcaster*, ConfigurationHints)
{
    // The color scheme only needs to be applied when configuration changed if the "default" color
    // scheme (based on Application Colors) is being used
    if (m_sActiveSchemeId == DEFAULT_SCHEME)
        ApplyColorScheme(DEFAULT_SCHEME, false);
}

// Applies an entire new color scheme; when bFirst is true, then the checks to see if the color scheme
// has changed are ignored to make sure the color scheme is applied
void ModulWindowLayout::SyntaxColors::ApplyColorScheme(OUString aSchemeId, bool bFirst)
{
    const TokenType vTokenTypes[] =
    {
        TokenType::Unknown,
        TokenType::Identifier,
        TokenType::Whitespace,
        TokenType::Number,
        TokenType::String,
        TokenType::EOL,
        TokenType::Comment,
        TokenType::Error,
        TokenType::Operator,
        TokenType::Keywords
    };

    m_sActiveSchemeId = aSchemeId;
    ColorScheme aColorScheme = GetShell()->GetColorConfig()->GetColorScheme(aSchemeId);
    Color aFontColor = aColorScheme.m_aGenericFontColor;
    m_aFontColor = aFontColor;
    Color aDocColor = aColorScheme.m_aBackgroundColor;
    m_aBackgroundColor = aDocColor;
    if (!bFirst && pEditor)
    {
        pEditor->ChangeFontColor(aFontColor);
        pEditor->SetBackground(Wallpaper(aDocColor));
        pEditor->SetLineHighlightColor(aColorScheme.m_aLineHighlightColor);
        pEditor->Invalidate();
    }

    for (const auto& aToken: vTokenTypes)
    {
        // Retrieves the new color to be set from the color scheme
        Color aNewColor;
        switch (aToken)
        {
        case TokenType::EOL:
        case TokenType::Whitespace:
        case TokenType::Unknown:
            aNewColor = aColorScheme.m_aGenericFontColor;
            break;
        case TokenType::Identifier:
            aNewColor = aColorScheme.m_aIdentifierColor;
            break;
        case TokenType::Number:
            aNewColor = aColorScheme.m_aNumberColor;
            break;
        case TokenType::String:
            aNewColor = aColorScheme.m_aStringColor;
            break;
        case TokenType::Comment:
            aNewColor = aColorScheme.m_aCommentColor;
            break;
        case TokenType::Error:
            aNewColor = aColorScheme.m_aErrorColor;
            break;
        case TokenType::Operator:
            aNewColor = aColorScheme.m_aOperatorColor;
            break;
        case TokenType::Keywords:
            aNewColor = aColorScheme.m_aKeywordColor;
            break;
        default:
            SAL_WARN("basctl.basicide", "Unexpected token type for color scheme");
            aNewColor = aColorScheme.m_aGenericFontColor;
        }

        Color& rCurrentColor = aColors[aToken];
        rCurrentColor = aNewColor;
    }

    // This check is needed because the EditEngine will only exist in the EditorWindow when the
    // module is actually opened
    if (!bFirst && pEditor)
        pEditor->UpdateSyntaxHighlighting();
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
