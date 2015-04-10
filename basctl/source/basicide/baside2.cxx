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
#include <svl/aeitem.hxx>
#include <svl/srchitem.hxx>
#include <svl/visitem.hxx>
#include <svl/whiter.hxx>
#include <vcl/xtextedt.hxx>
#include <vcl/settings.hxx>
#include <toolkit/helper/vclunohelper.hxx>

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

} // namespace

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ui::dialogs;
using namespace utl;
using namespace comphelper;

TYPEINIT1( ModulWindow , BaseWindow );

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
    aFont.SetAlign( ALIGN_BOTTOM );
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



// ModulWindow



ModulWindow::ModulWindow (
    ModulWindowLayout* pParent,
    ScriptDocument const& rDocument,
    const OUString& aLibName, const OUString& aName, OUString& aModule
) :
    BaseWindow(pParent, rDocument, aLibName, aName),
    rLayout(*pParent),
    nValid(ValidWindow),
    aXEditorWindow(this),
    m_aModule(aModule)
{
    aXEditorWindow.Show();
    SetBackground();
}

SbModuleRef ModulWindow::XModule()
{
    // ModuleWindows can now be created as a result of the
    // modules getting created via the api. This is a result of an
    // elementInserted event from the BasicLibrary container.
    // However the SbModule is also created from a different listener to
    // the same event ( in basmgr ) Therefore it is possible when we look
    // for xModule it may not yet be available, here we keep tring to access
    // the module until such time as it exists

    if ( !xModule.Is() )
    {
        BasicManager* pBasMgr = GetDocument().getBasicManager();
        if ( pBasMgr )
        {
            StarBASIC* pBasic = pBasMgr->GetLib( GetLibName() );
            if ( pBasic )
            {
                xBasic = pBasic;
                xModule = pBasic->FindModule( GetName() );
            }
        }
    }
    return xModule;
}

ModulWindow::~ModulWindow()
{
    nValid = 0;

    StarBASIC::Stop();
}


void ModulWindow::GetFocus()
{
    if (nValid != ValidWindow)
        return;
    aXEditorWindow.GetEdtWindow().GrabFocus();
    // don't call basic calls because focus is somewhere else...
}

void ModulWindow::DoInit()
{
    if ( GetVScrollBar() )
        GetVScrollBar()->Hide();
    GetHScrollBar()->Show();
    GetEditorWindow().InitScrollBars();
}


void ModulWindow::Paint( const Rectangle& )
{
}

void ModulWindow::Resize()
{
    aXEditorWindow.SetPosSizePixel( Point( 0, 0 ),
                                    Size( GetOutputSizePixel() ) );
}


void ModulWindow::CheckCompileBasic()
{
    if ( XModule().Is() )
    {
        // never compile while running!
        bool const bRunning = StarBASIC::IsRunning();
        bool const bModified = ( !xModule->IsCompiled() ||
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

            bDone = StarBASIC::Compile( xModule );
            if ( !bWasModified )
                GetBasic()->SetModified(false);

            if ( bDone )
            {
                GetBreakPoints().SetBreakPointsInBasic( xModule );
            }

            GetShell()->GetViewFrame()->GetWindow().LeaveWait();

            aStatus.bError = !bDone;
            aStatus.bIsRunning = false;
        }
    }
}

bool ModulWindow::BasicExecute()
{
    // #116444# check security settings before macro execution
    ScriptDocument aDocument( GetDocument() );
    if ( aDocument.isDocument() )
    {
        if ( !aDocument.allowMacros() )
        {
            MessageDialog(this, IDE_RESSTR(RID_STR_CANNOTRUNMACRO), VCL_MESSAGE_WARNING).Execute();
            return false;
        }
    }

    CheckCompileBasic();

    if ( XModule().Is() && xModule->IsCompiled() && !aStatus.bError )
    {
        if ( GetBreakPoints().size() )
            aStatus.nBasicFlags = aStatus.nBasicFlags | SbDEBUG_BREAK;

        if ( !aStatus.bIsRunning )
        {
            DBG_ASSERT( xModule.Is(), "Kein Modul!" );
            AddStatus( BASWIN_RUNNINGBASIC );
            sal_uInt16 nStart, nEnd, nCurMethodStart = 0;
            TextSelection aSel = GetEditView()->GetSelection();
            // Init cursor to top
            nCurMethodStart = ( aSel.GetStart().GetPara() + 1 );
            SbMethod* pMethod = 0;
            // first Macro, else blind "Main" (ExtSearch?)
            for ( sal_uInt16 nMacro = 0; nMacro < xModule->GetMethods()->Count(); nMacro++ )
            {
                SbMethod* pM = static_cast<SbMethod*>(xModule->GetMethods()->Get( nMacro ));
                DBG_ASSERT( pM, "Method?" );
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
                return ( !ChooseMacro( uno::Reference< frame::XModel >(), false, OUString() ).isEmpty() );
            }
            if ( pMethod )
            {
                pMethod->SetDebugFlags( aStatus.nBasicFlags );
                BasicDLL::SetDebugMode( true );
                RunMethod( pMethod );
                BasicDLL::SetDebugMode( false );
                // if cancelled during Interactive=false
                BasicDLL::EnableBreak( true );
            }
            ClearStatus( BASWIN_RUNNINGBASIC );
        }
        else
            aStatus.bIsRunning = false; // cancel of Reschedule()
    }

    bool bDone = !aStatus.bError;

    return bDone;
}

bool ModulWindow::CompileBasic()
{
    CheckCompileBasic();

    return XModule().Is() && xModule->IsCompiled();
}

bool ModulWindow::BasicRun()
{
    aStatus.nBasicFlags = 0;
    return BasicExecute();
}

bool ModulWindow::BasicStepOver()
{
    aStatus.nBasicFlags = SbDEBUG_STEPINTO | SbDEBUG_STEPOVER;
    return BasicExecute();
}


bool ModulWindow::BasicStepInto()
{
    aStatus.nBasicFlags = SbDEBUG_STEPINTO;
    return BasicExecute();
}

bool ModulWindow::BasicStepOut()
{
    aStatus.nBasicFlags = SbDEBUG_STEPOUT;
    return BasicExecute();
}



void ModulWindow::BasicStop()
{
    StarBASIC::Stop();
    aStatus.bIsRunning = false;
}

bool ModulWindow::LoadBasic()
{
    bool bDone = false;

    Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    Reference < XFilePicker3 > xFP = FilePicker::createWithMode(xContext, TemplateDescription::FILEOPEN_SIMPLE);

    if ( !aCurPath.isEmpty() )
        xFP->setDisplayDirectory ( aCurPath );

    xFP->appendFilter( "BASIC" , "*.bas" );
    xFP->appendFilter( IDE_RESSTR(RID_STR_FILTER_ALLFILES), OUString( FilterMask_All ) );
    xFP->setCurrentFilter( "BASIC" );

    if( xFP->execute() == RET_OK )
    {
        Sequence< OUString > aPaths = xFP->getFiles();
        aCurPath = aPaths[0];
        SfxMedium aMedium( aCurPath, StreamMode::READ | StreamMode::SHARE_DENYWRITE | StreamMode::NOCREATE );
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
            else
                bDone = true;
        }
        else
            MessageDialog(this, IDE_RESSTR(RID_STR_COULDNTREAD)).Execute();
    }
    return bDone;
}


bool ModulWindow::SaveBasicSource()
{
    bool bDone = false;

    Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    Reference < XFilePicker3 > xFP = FilePicker::createWithMode(xContext, TemplateDescription::FILESAVE_AUTOEXTENSION_PASSWORD);

    Reference< XFilePickerControlAccess > xFPControl(xFP, UNO_QUERY);
    xFPControl->enableControl(ExtendedFilePickerElementIds::CHECKBOX_PASSWORD, false);
    Any aValue;
    aValue <<= true;
    xFPControl->setValue(ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION, 0, aValue);

    if ( !aCurPath.isEmpty() )
        xFP->setDisplayDirectory ( aCurPath );

    xFP->appendFilter( "BASIC", "*.bas" );
    xFP->appendFilter( IDE_RESSTR(RID_STR_FILTER_ALLFILES), OUString( FilterMask_All ) );
    xFP->setCurrentFilter( "BASIC" );

    if( xFP->execute() == RET_OK )
    {
        Sequence< OUString > aPaths = xFP->getFiles();
        aCurPath = aPaths[0];
        SfxMedium aMedium( aCurPath, StreamMode::WRITE | StreamMode::SHARE_DENYWRITE | StreamMode::TRUNC );
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
            else
                bDone = true;
        }
        else
            MessageDialog(this, IDEResId(RID_STR_COULDNTWRITE)).Execute();
    }

    return bDone;
}

bool ModulWindow::ImportDialog()
{
    const ScriptDocument& rDocument = GetDocument();
    OUString aLibName = GetLibName();
    return implImportDialog( this, aCurPath, rDocument, aLibName );
}

bool ModulWindow::ToggleBreakPoint( sal_uLong nLine )
{
    DBG_ASSERT( XModule().Is(), "Kein Modul!" );

    bool bNewBreakPoint = false;

    if ( XModule().Is() )
    {
        CheckCompileBasic();
        if ( aStatus.bError )
        {
            return false;
        }

        BreakPoint* pBrk = GetBreakPoints().FindBreakPoint( nLine );
        if ( pBrk ) // remove
        {
            xModule->ClearBP( (sal_uInt16)nLine );
            delete GetBreakPoints().remove( pBrk );
        }
        else // create one
        {
            if ( xModule->SetBP( (sal_uInt16)nLine) )
            {
                GetBreakPoints().InsertSorted( new BreakPoint( nLine ) );
                bNewBreakPoint = true;
                if ( StarBASIC::IsRunning() )
                {
                    for ( sal_uInt16 nMethod = 0; nMethod < xModule->GetMethods()->Count(); nMethod++ )
                    {
                        SbMethod* pMethod = static_cast<SbMethod*>(xModule->GetMethods()->Get( nMethod ));
                        DBG_ASSERT( pMethod, "Methode nicht gefunden! (NULL)" );
                        pMethod->SetDebugFlags( pMethod->GetDebugFlags() | SbDEBUG_BREAK );
                    }
                }
            }
        }
    }

    return bNewBreakPoint;
}

void ModulWindow::UpdateBreakPoint( const BreakPoint& rBrk )
{
    DBG_ASSERT( XModule().Is(), "Kein Modul!" );

    if ( XModule().Is() )
    {
        CheckCompileBasic();

        if ( rBrk.bEnabled )
            xModule->SetBP( (sal_uInt16)rBrk.nLine );
        else
            xModule->ClearBP( (sal_uInt16)rBrk.nLine );
    }
}


bool ModulWindow::BasicToggleBreakPoint()
{
    AssertValidEditEngine();

    TextSelection aSel = GetEditView()->GetSelection();
    aSel.GetStart().GetPara()++;    // Basic lines start at 1!
    aSel.GetEnd().GetPara()++;

    bool bNewBreakPoint = false;

    for ( sal_uLong nLine = aSel.GetStart().GetPara(); nLine <= aSel.GetEnd().GetPara(); nLine++ )
    {
        if ( ToggleBreakPoint( nLine ) )
            bNewBreakPoint = true;
    }

    aXEditorWindow.GetBrkWindow().Invalidate();
    return bNewBreakPoint;
}


void ModulWindow::BasicToggleBreakPointEnabled()
{
    AssertValidEditEngine();

    ExtTextView* pView = GetEditView();
    if ( pView )
    {
        TextSelection aSel = pView->GetSelection();
        BreakPointList& rList = GetBreakPoints();

        for ( sal_uLong nLine = ++aSel.GetStart().GetPara(), nEnd = ++aSel.GetEnd().GetPara(); nLine <= nEnd; ++nLine )
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
    BreakPointDialog aBrkDlg( &rBrkWin, GetBreakPoints() );
    aBrkDlg.Execute();
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
        aXEditorWindow.GetBrkWindow().SetMarkerPos(nErrorLine, true);

    // #i47002#
    Reference< awt::XWindow > xWindow = VCLUnoHelper::GetInterface( this );

    ErrorHandler::HandleError( StarBASIC::GetErrorCode() );

    // #i47002#
    vcl::Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
    if ( !pWindow )
        return false;

    if ( bMarkError )
        aXEditorWindow.GetBrkWindow().SetNoMarker();
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
            return aStatus.nBasicFlags; // go on...
    }

    nErrorLine--;   // EditEngine starts at 0, Basic at 1

    AssertValidEditEngine();
    GetEditView()->SetSelection( TextSelection( TextPaM( nErrorLine, 0 ), TextPaM( nErrorLine, 0 ) ) );
    aXEditorWindow.GetBrkWindow().SetMarkerPos( nErrorLine );

    rLayout.UpdateDebug(false);

    aStatus.bIsInReschedule = true;
    aStatus.bIsRunning = true;

    AddStatus( BASWIN_INRESCHEDULE );

    InvalidateDebuggerSlots();

    while( aStatus.bIsRunning )
        Application::Yield();

    aStatus.bIsInReschedule = false;
    aXEditorWindow.GetBrkWindow().SetNoMarker();

    ClearStatus( BASWIN_INRESCHEDULE );

    return aStatus.nBasicFlags;
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
            sal_uInt16& rIndex = aSel.GetEnd().GetIndex();
            rIndex = rIndex + aWord.getLength();
            GetEditView()->SetSelection( aSel );
            bAdd = true;
        }
    }
    if ( bAdd )
    {
        TextSelection aSel = GetEditView()->GetSelection();
        if ( aSel.GetStart().GetPara() == aSel.GetEnd().GetPara() ) // single line selection
            rLayout.BasicAddWatch(GetEditView()->GetSelected());
    }
}



void ModulWindow::EditMacro( const OUString& rMacroName )
{
    DBG_ASSERT( XModule().Is(), "Kein Modul!" );

    if ( XModule().Is() )
    {
        CheckCompileBasic();

        if ( !aStatus.bError )
        {
            sal_uInt16 nStart, nEnd;
            SbMethod* pMethod = static_cast<SbMethod*>(xModule->Find( rMacroName, SbxCLASS_METHOD ));
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
                // scroll if applicabel so that first line is at the top
                long nVisHeight = GetOutputSizePixel().Height();
                if ( (long)pView->GetTextEngine()->GetTextHeight() > nVisHeight )
                {
                    long nMaxY = (long)pView->GetTextEngine()->GetTextHeight() - nVisHeight;
                    long nOldStartY = pView->GetStartDocPos().Y();
                    long nNewStartY = (long)nStart * (long)pView->GetTextEngine()->GetCharHeight();
                    nNewStartY = std::min( nNewStartY, nMaxY );
                    pView->Scroll( 0, -(nNewStartY-nOldStartY) );
                    pView->ShowCursor( false, true );
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
        SetModule( xModule->GetSource32() );

        if ( GetEditView() )
        {
            TextSelection aSel = GetEditView()->GetSelection();
            setTextEngineText(*GetEditEngine(), xModule->GetSource32());
            GetEditView()->SetSelection( aSel );
            GetEditEngine()->SetModified( false );
            MarkDocumentModified( GetDocument() );
        }
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
    aFont.SetAlign( ALIGN_BOTTOM );
    aFont.SetTransparent( true );
    aFont.SetSize( Size( 0, 360 ) );
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
    sal_uLong nParas = GetEditEngine()->GetParagraphCount();

    sal_Int32 nPages = nParas/nLinespPage+1;
    sal_Int32 nCurPage = 1;

    lcl_PrintHeader( pPrinter, nPages, nCurPage, aTitle, nPrintPage == 0 );
    Point aPos( Print::nLeftMargin, Print::nTopMargin );
    for ( sal_uLong nPara = 0; nPara < nParas; nPara++ )
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
            KeyEvent aFakeDelete( 0, KEY_DELETE );
            bool bDone = GetEditView()->KeyInput( aFakeDelete );
            if (!bDone)
            {
                BaseWindow::KeyInput(aFakeDelete);
            }
            break;
        }
        case SID_SELECTALL:
        {
            TextSelection aSel( TextPaM( 0, 0 ), TextPaM( TEXT_PARA_ALL, 0xFFFF ) );
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
            rLayout.BasicRemoveWatch();
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
            SFX_REQUEST_ARG(rReq, pItem, SfxBoolItem, rReq.GetSlot(), false);
            bSourceLinesEnabled = pItem && pItem->GetValue();
            aXEditorWindow.SetLineNumberDisplay(bSourceLinesEnabled);
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
            GotoLineDialog aGotoDlg(this);
            if (aGotoDlg.Execute())
                if (sal_Int32 const nLine = aGotoDlg.GetLineNumber())
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
                    bool bFound = false;

                    TextSelection aSel = pView->GetSelection();
                    long nLine = aSel.GetStart().GetPara();

                    for (long i = nLine; i >= 0 && !bFound; --i)
                    {
                        OUString aCurrLine = GetEditEngine()->GetText( i );
                        OUString sProcType;
                        bFound = GetEditorWindow().GetProcedureName(aCurrLine, sProcType, sProcName);
                    }

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
        GetEditView()->ShowCursor( false, true );
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
        aModuleName = xModule->GetName();
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
    aXEditorWindow.SetLineNumberDisplay(bSourceLinesEnabled);
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
            pView->SetSelection( TextSelection( TextPaM( 0xFFFFFFFF, 0xFFFF ), TextPaM( 0xFFFFFFFF, 0xFFFF ) ) );
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

::svl::IUndoManager* ModulWindow::GetUndoManager()
{
    if ( GetEditEngine() )
        return &GetEditEngine()->GetUndoManager();
    return NULL;
}

sal_uInt16 ModulWindow::GetSearchOptions()
{
    sal_uInt16 nOptions = SEARCH_OPTIONS_SEARCH |
                      SEARCH_OPTIONS_WHOLE_WORDS |
                      SEARCH_OPTIONS_BACKWARDS |
                      SEARCH_OPTIONS_REG_EXP |
                      SEARCH_OPTIONS_EXACT |
                      SEARCH_OPTIONS_SELECTION |
                      SEARCH_OPTIONS_SIMILARITY;

    if ( !IsReadOnly() )
    {
        nOptions |= SEARCH_OPTIONS_REPLACE;
        nOptions |= SEARCH_OPTIONS_REPLACE_ALL;
    }

    return nOptions;
}

void ModulWindow::BasicStarted()
{
    if ( XModule().Is() )
    {
        aStatus.bIsRunning = true;
        BreakPointList& rList = GetBreakPoints();
        if ( rList.size() )
        {
            rList.ResetHitCount();
            rList.SetBreakPointsInBasic( xModule );
            for ( sal_uInt16 nMethod = 0; nMethod < xModule->GetMethods()->Count(); nMethod++ )
            {
                SbMethod* pMethod = static_cast<SbMethod*>(xModule->GetMethods()->Get( nMethod ));
                DBG_ASSERT( pMethod, "Methode nicht gefunden! (NULL)" );
                pMethod->SetDebugFlags( pMethod->GetDebugFlags() | SbDEBUG_BREAK );
            }
        }
    }
}

void ModulWindow::BasicStopped()
{
    aStatus.bIsRunning = false;
    GetBreakPointWindow().SetNoMarker();
}

EntryDescriptor ModulWindow::CreateEntryDescriptor()
{
    ScriptDocument aDocument( GetDocument() );
    OUString aLibName( GetLibName() );
    LibraryLocation eLocation = aDocument.getLibraryLocation( aLibName );
    OUString aModName( GetName() );
    OUString aLibSubName;
    if( xBasic.Is() && aDocument.isInVBAMode() && XModule().Is() )
    {
        switch( xModule->GetModuleType() )
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
    aXEditorWindow.SetLineNumberDisplay(bSourceLinesEnabled);
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
    assert(xModule);

    // update module in module window
    SetModule(aModule);

    // update module in library
    OSL_VERIFY(m_aDocument.updateModule(m_aLibName, m_aName, aModule));

    GetEditEngine()->SetModified(false);
    MarkDocumentModified(m_aDocument);
}



// ModulWindowLayout



ModulWindowLayout::ModulWindowLayout (vcl::Window* pParent, ObjectCatalog& rObjectCatalog_) :
    Layout(pParent),
    pChild(0),
    aWatchWindow(this),
    aStackWindow(this),
    rObjectCatalog(rObjectCatalog_)
{ }

void ModulWindowLayout::UpdateDebug (bool bBasicStopped)
{
    aWatchWindow.UpdateWatches(bBasicStopped);
    aStackWindow.UpdateCalls();
}

void ModulWindowLayout::Paint (Rectangle const&)
{
    DrawText(Point(), IDEResId(RID_STR_NOMODULE).toString());
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
    aWatchWindow.Show();
    aStackWindow.Show();
    rObjectCatalog.Show();
    rObjectCatalog.SetLayoutWindow(this);
    rObjectCatalog.UpdateEntries();
    Layout::Activating(rChild);
    aSyntaxColors.SetActiveEditor(&pChild->GetEditorWindow());
}

void ModulWindowLayout::Deactivating ()
{
    aSyntaxColors.SetActiveEditor(0);
    Layout::Deactivating();
    aWatchWindow.Hide();
    aStackWindow.Hide();
    rObjectCatalog.Hide();
    pChild = 0;
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
    aWatchWindow.AddWatch(rWatchStr);
}

void ModulWindowLayout::BasicRemoveWatch ()
{
    aWatchWindow.RemoveSelectedWatch();
}

void ModulWindowLayout::OnFirstSize (long const nWidth, long const nHeight)
{
    AddToLeft(&rObjectCatalog, Size(nWidth * 0.20, nHeight * 0.75));
    AddToBottom(&aWatchWindow, Size(nWidth * 0.67, nHeight * 0.25));
    AddToBottom(&aStackWindow, Size(nWidth * 0.33, nHeight * 0.25));
}



// SyntaxColors



ModulWindowLayout::SyntaxColors::SyntaxColors () :
    pEditor(0)
{
    aConfig.AddListener(this);

    aColors[TT_UNKNOWN] =
    aColors[TT_WHITESPACE] =
    aColors[TT_EOL] =
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
    if (aColor != aColors[TT_UNKNOWN])
    {
        aColors[TT_UNKNOWN] =
        aColors[TT_WHITESPACE] =
        aColors[TT_EOL] =
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
        TokenTypes eTokenType;
        svtools::ColorConfigEntry eEntry;
    }
    const vIds[] =
    {
        { TT_IDENTIFIER,  svtools::BASICIDENTIFIER },
        { TT_NUMBER,      svtools::BASICNUMBER },
        { TT_STRING,      svtools::BASICSTRING },
        { TT_COMMENT,     svtools::BASICCOMMENT },
        { TT_ERROR,       svtools::BASICERROR },
        { TT_OPERATOR,    svtools::BASICOPERATOR },
        { TT_KEYWORDS,    svtools::BASICKEYWORD },
    };

    bool bChanged = false;
    for (unsigned i = 0; i != sizeof vIds / sizeof vIds[0]; ++i)
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
