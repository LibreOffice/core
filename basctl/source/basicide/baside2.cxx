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
#include "baside3.hxx"
#include "brkdlg.hxx"
#include "iderdll.hxx"
#include "moduldlg.hxx"
#include "docsignature.hxx"

#include "helpid.hrc"
#include "baside2.hrc"

#include <basic/basmgr.hxx>
#include <basic/basrdll.hxx>
#include <basic/sbmeth.hxx>
#include <com/sun/star/script/ModuleType.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/FilePicker.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <comphelper/string.hxx>
#include <sfx2/dinfdlg.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/request.hxx>
#include <sot/exchange.hxx>
#include <svl/aeitem.hxx>
#include <svl/srchitem.hxx>
#include <svl/visitem.hxx>
#include <svl/whiter.hxx>
#include <vcl/xtextedt.hxx>
#include <vcl/settings.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <cassert>

namespace basctl
{

namespace
{

namespace Print
{
    long const nLeftMargin = 1700;
    long const nRightMargin = 900;
    long const nTopMargin = 2000;
    long const nBottomMargin = 1000;
    long const nBorder = 300;
}

short const ValidWindow = 0x1234;

// What (who) are OW and MTF? Compare to baside3.cxx where an
// identically named variable, used in the same way, has the value
// "*.*" on Windows, "*" otherwise. Is that what should be done here,
// too?

#if defined(OW) || defined(MTF)
char const FilterMask_All[] = "*";
#else
char const FilterMask_All[] = "*.*";
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

    pPrinter->SetLineColor( Color( COL_BLACK ) );
    pPrinter->SetFillColor();

    vcl::Font aFont( aOldFont );
    aFont.SetWeight( WEIGHT_BOLD );
    aFont.SetAlignment( ALIGN_BOTTOM );
    pPrinter->SetFont( aFont );

    long nFontHeight = pPrinter->GetTextHeight();

    // 1st Border => line, 2+3 Border = free space
    long nYTop = Print::nTopMargin - 3*Print::nBorder - nFontHeight;

    long nXLeft = Print::nLeftMargin - Print::nBorder;
    long nXRight = aSz.Width() - Print::nRightMargin + Print::nBorder;

    if( bOutput )
        pPrinter->DrawRect(Rectangle(
            Point(nXLeft, nYTop),
            Size(nXRight - nXLeft, aSz.Height() - nYTop - Print::nBottomMargin + Print::nBorder)
        ));


    long nY = Print::nTopMargin - 2*Print::nBorder;
    Point aPos(Print::nLeftMargin, nY);
    if( bOutput )
        pPrinter->DrawText( aPos, rTitle );
    if ( nPages != 1 )
    {
        aFont.SetWeight( WEIGHT_NORMAL );
        pPrinter->SetFont( aFont );
        aPos.X() += pPrinter->GetTextWidth( rTitle );

        if( bOutput )
        {
            OUString aPageStr = " [" + IDE_RESSTR(RID_STR_PAGE) + " " + OUString::number( nCurPage ) + "]";
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
    if ( !rLine.isEmpty() )
    {
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
                aResult.insert( nPos, aBlanker.makeStringAndClear() );
                nMax = aResult.getLength();
            }
            ++nPos;
        }
        rLine = aResult.makeStringAndClear();
    }
}

// until we have some configuration lets just keep
// persist this value for the process lifetime
bool bSourceLinesEnabled = false;

} // namespace

ModulWindow::ModulWindow (ModulWindowLayout* pParent, ScriptDocument const& rDocument,
                          const OUString& aLibName, const OUString& aName, OUString& aModule)
    : BaseWindow(pParent, rDocument, aLibName, aName)
    , m_rLayout(*pParent)
    , m_nValid(ValidWindow)
    , m_aXEditorWindow(VclPtr<ComplexEditorWindow>::Create(this))
    , m_aModule(aModule)
{
    m_aXEditorWindow->Show();
    SetBackground();
}

SbModuleRef ModulWindow::XModule()
{
    // ModuleWindows can now be created as a result of the
    // modules getting created via the api. This is a result of an
    // elementInserted event from the BasicLibrary container.
    // However the SbModule is also created from a different listener to
    // the same event ( in basmgr ) Therefore it is possible when we look
    // for m_xModule it may not yet be available, here we keep trying to access
    // the module until such time as it exists

    if ( !m_xModule.Is() )
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
    if (GetVScrollBar())
        GetVScrollBar()->Hide();
    GetHScrollBar()->Show();
    GetEditorWindow().InitScrollBars();
}

void ModulWindow::Paint(vcl::RenderContext& /*rRenderContext*/, const Rectangle&)
{
}

void ModulWindow::Resize()
{
    m_aXEditorWindow->SetPosSizePixel( Point( 0, 0 ),
                                    Size( GetOutputSizePixel() ) );
}

void ModulWindow::CheckCompileBasic()
{
    if ( XModule().Is() )
    {
        // never compile while running!
        bool const bRunning = StarBASIC::IsRunning();
        bool const bModified = ( !m_xModule->IsCompiled() ||
            ( GetEditEngine() && GetEditEngine()->IsModified() ) );

        if ( !bRunning && bModified )
        {
            bool bDone = false;

            GetShell()->GetViewFrame()->GetWindow().EnterWait();

            if( bModified )
            {
                AssertValidEditEngine();
                GetEditorWindow().SetSourceInBasic();
            }

            bool bWasModified = GetBasic()->IsModified();

            bDone = m_xModule->Compile();
            if ( !bWasModified )
                GetBasic()->SetModified(false);

            if ( bDone )
            {
                GetBreakPoints().SetBreakPointsInBasic( m_xModule );
            }

            GetShell()->GetViewFrame()->GetWindow().LeaveWait();

            m_aStatus.bError = !bDone;
            m_aStatus.bIsRunning = false;
        }
    }
}

void ModulWindow::BasicExecute()
{
    // #116444# check security settings before macro execution
    ScriptDocument aDocument( GetDocument() );
    if ( aDocument.isDocument() )
    {
        if ( !aDocument.allowMacros() )
        {
            ScopedVclPtrInstance<MessageDialog>(this, IDE_RESSTR(RID_STR_CANNOTRUNMACRO), VclMessageType::Warning)->Execute();
            return;
        }
    }

    CheckCompileBasic();

    if ( XModule().Is() && m_xModule->IsCompiled() && !m_aStatus.bError )
    {
        if ( GetBreakPoints().size() )
            m_aStatus.nBasicFlags = m_aStatus.nBasicFlags | SbDEBUG_BREAK;

        if ( !m_aStatus.bIsRunning )
        {
            DBG_ASSERT( m_xModule.Is(), "Kein Modul!" );
            AddStatus( BASWIN_RUNNINGBASIC );
            sal_uInt16 nStart, nEnd;
            TextSelection aSel = GetEditView()->GetSelection();
            // Init cursor to top
            const sal_uInt32 nCurMethodStart = aSel.GetStart().GetPara() + 1;
            SbMethod* pMethod = nullptr;
            // first Macro, else blind "Main" (ExtSearch?)
            for ( sal_uInt16 nMacro = 0; nMacro < m_xModule->GetMethods()->Count(); nMacro++ )
            {
                SbMethod* pM = static_cast<SbMethod*>(m_xModule->GetMethods()->Get( nMacro ));
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
                ChooseMacro( uno::Reference< frame::XModel >(), false, OUString() );
                return;
            }
            if ( pMethod )
            {
                pMethod->SetDebugFlags( m_aStatus.nBasicFlags );
                BasicDLL::SetDebugMode( true );
                RunMethod( pMethod );
                BasicDLL::SetDebugMode( false );
                // if cancelled during Interactive=false
                BasicDLL::EnableBreak( true );
            }
            ClearStatus( BASWIN_RUNNINGBASIC );
        }
        else
            m_aStatus.bIsRunning = false; // cancel of Reschedule()
    }
}

void ModulWindow::CompileBasic()
{
    CheckCompileBasic();

    XModule().Is() && m_xModule->IsCompiled();
}

void ModulWindow::BasicRun()
{
    m_aStatus.nBasicFlags = 0;
    BasicExecute();
}

void ModulWindow::BasicStepOver()
{
    m_aStatus.nBasicFlags = SbDEBUG_STEPINTO | SbDEBUG_STEPOVER;
    BasicExecute();
}


void ModulWindow::BasicStepInto()
{
    m_aStatus.nBasicFlags = SbDEBUG_STEPINTO;
    BasicExecute();
}

void ModulWindow::BasicStepOut()
{
    m_aStatus.nBasicFlags = SbDEBUG_STEPOUT;
    BasicExecute();
}


void ModulWindow::BasicStop()
{
    StarBASIC::Stop();
    m_aStatus.bIsRunning = false;
}

void ModulWindow::LoadBasic()
{
    Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    Reference < XFilePicker3 > xFP = FilePicker::createWithMode(xContext, TemplateDescription::FILEOPEN_SIMPLE);

    if ( !m_sCurPath.isEmpty() )
        xFP->setDisplayDirectory ( m_sCurPath );

    xFP->appendFilter( "BASIC" , "*.bas" );
    xFP->appendFilter( IDE_RESSTR(RID_STR_FILTER_ALLFILES), FilterMask_All );
    xFP->setCurrentFilter( "BASIC" );

    if( xFP->execute() == RET_OK )
    {
        Sequence< OUString > aPaths = xFP->getSelectedFiles();
        m_sCurPath = aPaths[0];
        SfxMedium aMedium( m_sCurPath, StreamMode::READ | StreamMode::SHARE_DENYWRITE | StreamMode::NOCREATE );
        SvStream* pStream = aMedium.GetInStream();
        if ( pStream )
        {
            AssertValidEditEngine();
            sal_uLong nLines = CalcLineCount( *pStream );
            // nLines*4: ReadText/Formatting/Highlighting/Formatting
            GetEditorWindow().CreateProgress( IDEResId(RID_STR_GENERATESOURCE).toString(), nLines*4 );
            GetEditEngine()->SetUpdateMode( false );
            GetEditView()->Read( *pStream );
            GetEditEngine()->SetUpdateMode( true );
            GetEditorWindow().Update();
            GetEditorWindow().ForceSyntaxTimeout();
            GetEditorWindow().DestroyProgress();
            sal_uLong nError = aMedium.GetError();
            if ( nError )
                ErrorHandler::HandleError( nError );
        }
        else
            ScopedVclPtrInstance<MessageDialog>(this, IDE_RESSTR(RID_STR_COULDNTREAD))->Execute();
    }
}


void ModulWindow::SaveBasicSource()
{
    Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    Reference < XFilePicker3 > xFP = FilePicker::createWithMode(xContext, TemplateDescription::FILESAVE_AUTOEXTENSION_PASSWORD);

    Reference< XFilePickerControlAccess > xFPControl(xFP, UNO_QUERY);
    xFPControl->enableControl(ExtendedFilePickerElementIds::CHECKBOX_PASSWORD, false);
    Any aValue;
    aValue <<= true;
    xFPControl->setValue(ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION, 0, aValue);

    if ( !m_sCurPath.isEmpty() )
        xFP->setDisplayDirectory ( m_sCurPath );

    xFP->appendFilter( "BASIC", "*.bas" );
    xFP->appendFilter( IDE_RESSTR(RID_STR_FILTER_ALLFILES), FilterMask_All );
    xFP->setCurrentFilter( "BASIC" );

    if( xFP->execute() == RET_OK )
    {
        Sequence< OUString > aPaths = xFP->getSelectedFiles();
        m_sCurPath = aPaths[0];
        SfxMedium aMedium( m_sCurPath, StreamMode::WRITE | StreamMode::SHARE_DENYWRITE | StreamMode::TRUNC );
        SvStream* pStream = aMedium.GetOutStream();
        if ( pStream )
        {
            EnterWait();
            AssertValidEditEngine();
            GetEditEngine()->Write( *pStream );
            aMedium.Commit();
            LeaveWait();
            sal_uLong nError = aMedium.GetError();
            if ( nError )
                ErrorHandler::HandleError( nError );
        }
        else
            ScopedVclPtrInstance<MessageDialog>(this, IDEResId(RID_STR_COULDNTWRITE))->Execute();
    }
}

void ModulWindow::ImportDialog()
{
    const ScriptDocument& rDocument = GetDocument();
    OUString aLibName = GetLibName();
    implImportDialog( this, m_sCurPath, rDocument, aLibName );
}

void ModulWindow::ToggleBreakPoint( sal_uLong nLine )
{
    DBG_ASSERT( XModule().Is(), "Kein Modul!" );

    if ( XModule().Is() )
    {
        CheckCompileBasic();
        if ( m_aStatus.bError )
        {
            return;
        }

        BreakPoint* pBrk = GetBreakPoints().FindBreakPoint( nLine );
        if ( pBrk ) // remove
        {
            m_xModule->ClearBP( (sal_uInt16)nLine );
            delete GetBreakPoints().remove( pBrk );
        }
        else // create one
        {
            if ( m_xModule->SetBP( (sal_uInt16)nLine) )
            {
                GetBreakPoints().InsertSorted( new BreakPoint( nLine ) );
                if ( StarBASIC::IsRunning() )
                {
                    for ( sal_uInt16 nMethod = 0; nMethod < m_xModule->GetMethods()->Count(); nMethod++ )
                    {
                        SbMethod* pMethod = static_cast<SbMethod*>(m_xModule->GetMethods()->Get( nMethod ));
                        assert(pMethod && "Methode nicht gefunden! (NULL)");
                        pMethod->SetDebugFlags( pMethod->GetDebugFlags() | SbDEBUG_BREAK );
                    }
                }
            }
        }
    }
}

void ModulWindow::UpdateBreakPoint( const BreakPoint& rBrk )
{
    DBG_ASSERT( XModule().Is(), "Kein Modul!" );

    if ( XModule().Is() )
    {
        CheckCompileBasic();

        if ( rBrk.bEnabled )
            m_xModule->SetBP( (sal_uInt16)rBrk.nLine );
        else
            m_xModule->ClearBP( (sal_uInt16)rBrk.nLine );
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

    ExtTextView* pView = GetEditView();
    if ( pView )
    {
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
}


void ModulWindow::ManageBreakPoints()
{
    BreakPointWindow& rBrkWin = GetBreakPointWindow();
    ScopedVclPtrInstance< BreakPointDialog > aBrkDlg( &rBrkWin, GetBreakPoints() );
    aBrkDlg->Execute();
    rBrkWin.Invalidate();
}


bool ModulWindow::BasicErrorHdl( StarBASIC * pBasic )
{
    GoOnTop();

    // ReturnWert: BOOL
    //  FALSE:  cancel
    //  TRUE:   go on....
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

    ErrorHandler::HandleError( StarBASIC::GetErrorCode() );

    // #i47002#
    vcl::Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
    if ( !pWindow )
        return false;

    if ( bMarkError )
        m_aXEditorWindow->GetBrkWindow().SetNoMarker();
    return false;
}

long ModulWindow::BasicBreakHdl( StarBASIC* pBasic )
{
    // #i69280 Required in Window despite normal usage in next command!
    (void)pBasic;

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

    while( m_aStatus.bIsRunning )
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
        TextPaM aWordStart;
        OUString aWord = GetEditEngine()->GetWord( GetEditView()->GetSelection().GetEnd(), &aWordStart );
        if ( !aWord.isEmpty() )
        {
            TextSelection aSel( aWordStart );
            aSel.GetEnd().GetIndex() += aWord.getLength();
            GetEditView()->SetSelection( aSel );
            bAdd = true;
        }
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
    DBG_ASSERT( XModule().Is(), "Kein Modul!" );

    if ( XModule().Is() )
    {
        CheckCompileBasic();

        if ( !m_aStatus.bError )
        {
            sal_uInt16 nStart, nEnd;
            SbMethod* pMethod = static_cast<SbMethod*>(m_xModule->Find( rMacroName, SbxClassType::Method ));
            if ( pMethod )
            {
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
                long nVisHeight = GetOutputSizePixel().Height();
                if ( pView->GetTextEngine()->GetTextHeight() > nVisHeight )
                {
                    long nMaxY = pView->GetTextEngine()->GetTextHeight() - nVisHeight;
                    long nOldStartY = pView->GetStartDocPos().Y();
                    long nNewStartY = (long)nStart * pView->GetTextEngine()->GetCharHeight();
                    nNewStartY = std::min( nNewStartY, nMaxY );
                    pView->Scroll( 0, -(nNewStartY-nOldStartY) );
                    pView->ShowCursor( false );
                    GetEditVScrollBar().SetThumbPos( pView->GetStartDocPos().Y() );
                }
                pView->SetSelection( aSel );
                pView->ShowCursor();
                pView->GetWindow()->GrabFocus();
            }
        }
    }
}

void ModulWindow::StoreData()
{
    // StoreData is called when the BasicManager is destroyed or
    // this window is closed.
    // => interrupts undesired!
    GetEditorWindow().SetSourceInBasic();
}

bool ModulWindow::CanClose()
{
    return true;
}

bool ModulWindow::AllowUndo()
{
    return GetEditorWindow().CanModify();
}

void ModulWindow::UpdateData()
{
    DBG_ASSERT( XModule().Is(), "Kein Modul!" );
    // UpdateData is called when the source has changed from outside
    // => interrupts undesired!

    if ( XModule().Is() )
    {
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
}

sal_Int32 ModulWindow::countPages( Printer* pPrinter )
{
    return FormatAndPrint( pPrinter );
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
    pPrinter->SetMapMode( MAP_100TH_MM );

    OUString aTitle( CreateQualifiedName() );

    sal_Int32 nLineHeight = pPrinter->GetTextHeight();
    if(nLineHeight == 0)
    {
        nLineHeight = 1;
    }
    sal_Int32 nParaSpace = 10;

    Size aPaperSz = pPrinter->GetOutputSize();
    aPaperSz.Width() -= (Print::nLeftMargin + Print::nRightMargin);
    aPaperSz.Height() -= (Print::nTopMargin + Print::nBottomMargin);

    // nLinepPage is not correct if there's a line break
    sal_Int32 nLinespPage = aPaperSz.Height()/nLineHeight;
    long nXTextWidth = pPrinter->approximate_char_width();

    sal_Int32 nCharspLine = aPaperSz.Width() / (nXTextWidth > 1 ? nXTextWidth : 1);
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
            aPos.Y() += nLineHeight;
            if ( aPos.Y() > ( aPaperSz.Height() + Print::nTopMargin ) )
            {
                nCurPage++;
                lcl_PrintHeader( pPrinter, nPages, nCurPage, aTitle, nCurPage-1 == nPrintPage );
                aPos = Point(Print::nLeftMargin, Print::nTopMargin + nLineHeight);
            }
            if( nCurPage-1 == nPrintPage )
                pPrinter->DrawText( aPos, aTmpLine );
        }
        aPos.Y() += nParaSpace;
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
            bSourceLinesEnabled = pItem && pItem->GetValue();
            m_aXEditorWindow->SetLineNumberDisplay(bSourceLinesEnabled);
        }
        break;
        case SID_BASICIDE_DELETECURRENT:
        {
            if (QueryDelModule(m_aName, this))
                if (m_aDocument.removeModule(m_aLibName, m_aName))
                    MarkDocumentModified(m_aDocument);
        }
        break;
        case FID_SEARCH_OFF:
            GrabFocus();
            break;
        case SID_GOTOLINE:
        {
            ScopedVclPtrInstance< GotoLineDialog > aGotoDlg(this);
            if (aGotoDlg->Execute())
                if (sal_Int32 const nLine = aGotoDlg->GetLineNumber())
                {
                    TextSelection const aSel(TextPaM(nLine - 1, 0), TextPaM(nLine - 1, 0));
                    GetEditView()->SetSelection(aSel);
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
                aSignature.signScriptingContent();
                if (SfxBindings* pBindings = GetBindingsPtr())
                    pBindings->Invalidate(SID_SIGNATURE);
            }
        }
        break;
    }
}


void ModulWindow::GetState( SfxItemSet &rSet )
{
    SfxWhichIter aIter(rSet);
    for ( sal_uInt16 nWh = aIter.FirstWhich(); 0 != nWh; nWh = aIter.NextWhich() )
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
                    OUString aPos = OUString( IDEResId( RID_STR_LINE ) ) +
                                   " " +
                                   OUString::number(aSel.GetEnd().GetPara()+1) +
                                   ", " +
                                   OUString( IDEResId( RID_STR_COLUMN ) ) +
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
                rSet.Put(SfxBoolItem(nWh, bSourceLinesEnabled));
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

void ModulWindow::DoScroll( ScrollBar* pCurScrollBar )
{
    if ( ( pCurScrollBar == GetHScrollBar() ) && GetEditView() )
    {
        // don't scroll with the value but rather use the Thumb-Pos for the VisArea:
        long nDiff = GetEditView()->GetStartDocPos().X() - pCurScrollBar->GetThumbPos();
        GetEditView()->Scroll( nDiff, 0 );
        GetEditView()->ShowCursor( false );
        pCurScrollBar->SetThumbPos( GetEditView()->GetStartDocPos().X() );

    }
}

bool ModulWindow::IsModified()
{
    return GetEditEngine() && GetEditEngine()->IsModified();
}

void ModulWindow::GoOnTop()
{
    GetShell()->GetViewFrame()->ToTop();
}

OUString ModulWindow::GetSbModuleName()
{
    OUString aModuleName;
    if ( XModule().Is() )
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
    m_aXEditorWindow->SetLineNumberDisplay(bSourceLinesEnabled);
    Show();
}

void ModulWindow::Deactivating()
{
    Hide();
    if ( GetEditView() )
        GetEditView()->EraseVirtualDevice();
}

sal_uInt16 ModulWindow::StartSearchAndReplace( const SvxSearchItem& rSearchItem, bool bFromStart )
{
    if (IsSuspended())
        return 0;

    // one could also relinquish syntaxhighlighting/formatting instead of the stupid replace-everything...
    AssertValidEditEngine();
    ExtTextView* pView = GetEditView();
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

svl::IUndoManager* ModulWindow::GetUndoManager()
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
    if ( XModule().Is() )
    {
        m_aStatus.bIsRunning = true;
        BreakPointList& rList = GetBreakPoints();
        if ( rList.size() )
        {
            rList.ResetHitCount();
            rList.SetBreakPointsInBasic( m_xModule );
            for ( sal_uInt16 nMethod = 0; nMethod < m_xModule->GetMethods()->Count(); nMethod++ )
            {
                SbMethod* pMethod = static_cast<SbMethod*>(m_xModule->GetMethods()->Get( nMethod ));
                assert(pMethod && "Methode nicht gefunden! (NULL)");
                pMethod->SetDebugFlags( pMethod->GetDebugFlags() | SbDEBUG_BREAK );
            }
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
    if( m_xBasic.Is() && aDocument.isInVBAMode() && XModule().Is() )
    {
        switch( m_xModule->GetModuleType() )
        {
            case script::ModuleType::DOCUMENT:
            {
                aLibSubName = OUString( IDEResId( RID_STR_DOCUMENT_OBJECTS ) );
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
                aLibSubName = OUString( IDEResId( RID_STR_USERFORMS ) );
                break;
            case script::ModuleType::NORMAL:
                aLibSubName = OUString( IDEResId( RID_STR_NORMAL_MODULES ) );
                break;
            case script::ModuleType::CLASS:
                aLibSubName = OUString( IDEResId( RID_STR_CLASS_MODULES ) );
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

        Reference< datatransfer::XTransferable > xTransf;
        {
            SolarMutexReleaser aReleaser;
            // get clipboard content
            xTransf = xClipboard->getContents();
        }
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
    m_aXEditorWindow->SetLineNumberDisplay(bSourceLinesEnabled);
}

char const* ModulWindow::GetHid () const
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
    assert(m_xModule);

    // update module in module window
    SetModule(aModule);

    // update module in library
    OSL_VERIFY(m_aDocument.updateModule(m_aLibName, m_aName, aModule));

    GetEditEngine()->SetModified(false);
    MarkDocumentModified(m_aDocument);
}

ModulWindowLayout::ModulWindowLayout (vcl::Window* pParent, ObjectCatalog& rObjectCatalog_) :
    Layout(pParent),
    pChild(nullptr),
    aWatchWindow(VclPtr<WatchWindow>::Create(this)),
    aStackWindow(VclPtr<StackWindow>::Create(this)),
    rObjectCatalog(rObjectCatalog_)
{ }

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

void ModulWindowLayout::Paint (vcl::RenderContext& rRenderContext, Rectangle const&)
{
    rRenderContext.DrawText(Point(), IDEResId(RID_STR_NOMODULE).toString());
}

// virtual
void ModulWindowLayout::DataChanged (DataChangedEvent const& rDCEvt)
{
    Layout::DataChanged(rDCEvt);
    if (rDCEvt.GetType() == DataChangedEventType::SETTINGS && (rDCEvt.GetFlags() & AllSettingsFlags::STYLE))
        aSyntaxColors.SettingsChanged();
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

void ModulWindowLayout::OnFirstSize (long const nWidth, long const nHeight)
{
    AddToLeft(&rObjectCatalog, Size(nWidth * 0.20, nHeight * 0.75));
    AddToBottom(aWatchWindow.get(), Size(nWidth * 0.67, nHeight * 0.25));
    AddToBottom(aStackWindow.get(), Size(nWidth * 0.33, nHeight * 0.25));
}

ModulWindowLayout::SyntaxColors::SyntaxColors () :
    pEditor(nullptr)
{
    aConfig.AddListener(this);

    aColors[TokenType::Unknown] =
    aColors[TokenType::Whitespace] =
    aColors[TokenType::EOL] =
        Application::GetSettings().GetStyleSettings().GetFieldTextColor();

    NewConfig(true);
}

ModulWindowLayout::SyntaxColors::~SyntaxColors ()
{
    aConfig.RemoveListener(this);
}

void ModulWindowLayout::SyntaxColors::SettingsChanged ()
{
    Color const aColor = Application::GetSettings().GetStyleSettings().GetFieldTextColor();
    if (aColor != aColors[TokenType::Unknown])
    {
        aColors[TokenType::Unknown] =
        aColors[TokenType::Whitespace] =
        aColors[TokenType::EOL] =
            aColor;
        if (pEditor)
            pEditor->UpdateSyntaxHighlighting();
    }
}

// virtual
void ModulWindowLayout::SyntaxColors::ConfigurationChanged (utl::ConfigurationBroadcaster*, sal_uInt32)
{
    NewConfig(false);
}

// when a new configuration has to be set
void ModulWindowLayout::SyntaxColors::NewConfig (bool bFirst)
{
    static struct
    {
        TokenType eTokenType;
        svtools::ColorConfigEntry eEntry;
    }
    const vIds[] =
    {
        { TokenType::Identifier,  svtools::BASICIDENTIFIER },
        { TokenType::Number,      svtools::BASICNUMBER },
        { TokenType::String,      svtools::BASICSTRING },
        { TokenType::Comment,     svtools::BASICCOMMENT },
        { TokenType::Error,       svtools::BASICERROR },
        { TokenType::Operator,    svtools::BASICOPERATOR },
        { TokenType::Keywords,    svtools::BASICKEYWORD },
    };

    bool bChanged = false;
    for (unsigned i = 0; i != SAL_N_ELEMENTS(vIds); ++i)
    {
        Color const aColor = aConfig.GetColorValue(vIds[i].eEntry).nColor;
        Color& rMyColor = aColors[vIds[i].eTokenType];
        if (bFirst || aColor != rMyColor)
        {
            rMyColor = aColor;
            bChanged = true;
        }
    }
    if (bChanged && !bFirst && pEditor)
        pEditor->UpdateSyntaxHighlighting();
}


} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
