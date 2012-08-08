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
#include "brkdlg.hxx"
#include "iderdll.hxx"
#include "iderdll2.hxx"
#include "objdlg.hxx"

#include "baside2.hrc"

#include <basic/basmgr.hxx>
#include <basic/basrdll.hxx>
#include <basic/sbmeth.hxx>
#include <com/sun/star/script/ModuleType.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#include <comphelper/processfactory.hxx>
#include <sfx2/dinfdlg.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/request.hxx>
#include <svl/aeitem.hxx>
#include <svl/srchitem.hxx>
#include <svl/whiter.hxx>
#include <vcl/xtextedt.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/msgbox.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

#define LMARGPRN        1700
#define RMARGPRN         900
#define TMARGPRN        2000
#define BMARGPRN        1000
#define BORDERPRN       300

#define VALIDWINDOW     0x1234

#if defined(OW) || defined(MTF)
#define FILTERMASK_ALL "*"
#else
#define FILTERMASK_ALL "*.*"
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ui::dialogs;
using namespace utl;
using namespace comphelper;


DBG_NAME( ModulWindow )

TYPEINIT1( ModulWindow , IDEBaseWindow );

void lcl_PrintHeader( Printer* pPrinter, sal_uInt16 nPages, sal_uInt16 nCurPage, const ::rtl::OUString& rTitle, bool bOutput )
{
    short nLeftMargin   = LMARGPRN;
    Size aSz = pPrinter->GetOutputSize();
    short nBorder = BORDERPRN;

    const Color aOldLineColor( pPrinter->GetLineColor() );
    const Color aOldFillColor( pPrinter->GetFillColor() );
    const Font  aOldFont( pPrinter->GetFont() );

    pPrinter->SetLineColor( Color( COL_BLACK ) );
    pPrinter->SetFillColor();

    Font aFont( aOldFont );
    aFont.SetWeight( WEIGHT_BOLD );
    aFont.SetAlign( ALIGN_BOTTOM );
    pPrinter->SetFont( aFont );

    long nFontHeight = pPrinter->GetTextHeight();

    // 1st Border => line, 2+3 Border = free space
    long nYTop = TMARGPRN-3*nBorder-nFontHeight;

    long nXLeft = nLeftMargin-nBorder;
    long nXRight = aSz.Width()-RMARGPRN+nBorder;

    if( bOutput )
        pPrinter->DrawRect( Rectangle(
            Point( nXLeft, nYTop ),
            Size( nXRight-nXLeft, aSz.Height() - nYTop - BMARGPRN + nBorder ) ) );


    long nY = TMARGPRN-2*nBorder;
    Point aPos( nLeftMargin, nY );
    if( bOutput )
        pPrinter->DrawText( aPos, rTitle );
    if ( nPages != 1 )
    {
        aFont.SetWeight( WEIGHT_NORMAL );
        pPrinter->SetFont( aFont );
        aPos.X() += pPrinter->GetTextWidth( rTitle );

        if( bOutput )
        {
            ::rtl::OUStringBuffer aPageStr;
            aPageStr.appendAscii( RTL_CONSTASCII_STRINGPARAM( " [" ) );
            aPageStr.append(IDE_RESSTR(RID_STR_PAGE));
            aPageStr.append(' ');
            aPageStr.append( nCurPage );
            aPageStr.append(']');
            pPrinter->DrawText( aPos, aPageStr.makeStringAndClear() );
        }
    }

    nY = TMARGPRN-nBorder;

    if( bOutput )
        pPrinter->DrawLine( Point( nXLeft, nY ), Point( nXRight, nY ) );

    pPrinter->SetFont( aOldFont );
    pPrinter->SetFillColor( aOldFillColor );
    pPrinter->SetLineColor( aOldLineColor );
}

void lcl_ConvertTabsToSpaces( String& rLine )
{
    if ( rLine.Len() )
    {
        sal_uInt16 nPos = 0;
        sal_uInt16 nMax = rLine.Len();
        while ( nPos < nMax )
        {
            if ( rLine.GetChar( nPos ) == '\t' )
            {
                // not 4 Blanks, but at 4 TabPos:
                String aBlanker;
                aBlanker.Fill( ( 4 - ( nPos % 4 ) ), ' ' );
                rLine.Erase( nPos, 1 );
                rLine.Insert( aBlanker, nPos );
                nMax = rLine.Len();
            }
            nPos++;
        }
    }
}

ModulWindow::ModulWindow( ModulWindowLayout* pParent, const ScriptDocument& rDocument, ::rtl::OUString aLibName,
                          ::rtl::OUString aName, ::rtl::OUString& aModule )
        :IDEBaseWindow( pParent, rDocument, aLibName, aName )
        ,aXEditorWindow( this )
        ,m_aModule( aModule )
{
    DBG_CTOR( ModulWindow, 0 );
    nValid = VALIDWINDOW;
    pLayout = pParent;
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
                xModule = (SbModule*)pBasic->FindModule( GetName() );
            }
        }
    }
    return xModule;
}

ModulWindow::~ModulWindow()
{
    DBG_DTOR( ModulWindow, 0 );
    nValid = 0;

    StarBASIC::Stop();
}


void ModulWindow::GetFocus()
{
    if ( nValid != VALIDWINDOW  )
        return;
    DBG_CHKTHIS( ModulWindow, 0 );
    aXEditorWindow.GetEdtWindow().GrabFocus();
    // don't call basic calls because focus is somewhere else...
}

void ModulWindow::DoInit()
{
    DBG_CHKTHIS( ModulWindow, 0 );

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


// "Import" of baside4.cxx
void CreateEngineForBasic( StarBASIC* pBasic );

void ModulWindow::CheckCompileBasic()
{
    DBG_CHKTHIS( ModulWindow, 0 );

    if ( XModule().Is() )
    {
        // never compile while running!
        bool const bRunning = StarBASIC::IsRunning();
        bool const bModified = ( !xModule->IsCompiled() ||
            ( GetEditEngine() && GetEditEngine()->IsModified() ) );

        if ( !bRunning && bModified )
        {
            bool bDone = false;

            BasicIDEShell* pIDEShell = BasicIDEGlobals::GetShell();
            pIDEShell->GetViewFrame()->GetWindow().EnterWait();

            if( bModified )
            {
                AssertValidEditEngine();
                GetEditorWindow().SetSourceInBasic();
            }

            bool bWasModified = GetBasic()->IsModified();

            bDone = GetBasic()->Compile( xModule );
            if ( !bWasModified )
                GetBasic()->SetModified(false);

            if ( bDone )
            {
                GetBreakPoints().SetBreakPointsInBasic( xModule );
            }

            pIDEShell->GetViewFrame()->GetWindow().LeaveWait();

            aStatus.bError = !bDone;
            aStatus.bIsRunning = false;
        }
    }
}

bool ModulWindow::BasicExecute()
{
    DBG_CHKTHIS( ModulWindow, 0 );

    // #116444# check security settings before macro execution
    ScriptDocument aDocument( GetDocument() );
    if ( aDocument.isDocument() )
    {
        if ( !aDocument.allowMacros() )
        {
            WarningBox( this, WB_OK, IDE_RESSTR(RID_STR_CANNOTRUNMACRO)).Execute();
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
                SbMethod* pM = (SbMethod*)xModule->GetMethods()->Get( nMacro );
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
                return ( !BasicIDE::ChooseMacro( uno::Reference< frame::XModel >(), false, rtl::OUString() ).isEmpty() );
            }
            if ( pMethod )
            {
                pMethod->SetDebugFlags( aStatus.nBasicFlags );
                BasicDLL::SetDebugMode( true );
                BasicIDE::RunMethod( pMethod );
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
    DBG_CHKTHIS( ModulWindow, 0 );
    CheckCompileBasic();

    return XModule().Is() && xModule->IsCompiled();
}

bool ModulWindow::BasicRun()
{
    DBG_CHKTHIS( ModulWindow, 0 );

    aStatus.nBasicFlags = 0;
    return BasicExecute();
}

bool ModulWindow::BasicStepOver()
{
    DBG_CHKTHIS( ModulWindow, 0 );
    aStatus.nBasicFlags = SbDEBUG_STEPINTO | SbDEBUG_STEPOVER;
    return BasicExecute();
}


bool ModulWindow::BasicStepInto()
{
    DBG_CHKTHIS( ModulWindow, 0 );

    aStatus.nBasicFlags = SbDEBUG_STEPINTO;
    return BasicExecute();
}

bool ModulWindow::BasicStepOut()
{
    DBG_CHKTHIS( ModulWindow, 0 );

    aStatus.nBasicFlags = SbDEBUG_STEPOUT;
    return BasicExecute();
}



void ModulWindow::BasicStop()
{
    DBG_CHKTHIS( ModulWindow, 0 );

    GetBasic()->Stop();
    aStatus.bIsRunning = false;
}

bool ModulWindow::LoadBasic()
{
    DBG_CHKTHIS( ModulWindow, 0 );
    bool bDone = false;

    Reference< lang::XMultiServiceFactory > xMSF( ::comphelper::getProcessServiceFactory() );
    Reference < XFilePicker > xFP;
    if( xMSF.is() )
    {
        Sequence <Any> aServiceType(1);
        aServiceType[0] <<= TemplateDescription::FILEOPEN_SIMPLE;
        xFP = Reference< XFilePicker >( xMSF->createInstanceWithArguments(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ui.dialogs.FilePicker" ) ), aServiceType ), UNO_QUERY );
    }

    if ( !aCurPath.isEmpty() )
        xFP->setDisplayDirectory ( aCurPath );

    Reference< XFilterManager > xFltMgr(xFP, UNO_QUERY);
    xFltMgr->appendFilter( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "BASIC" ) ), ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "*.bas" ) ) );
    xFltMgr->appendFilter( IDE_RESSTR(RID_STR_FILTER_ALLFILES), ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( FILTERMASK_ALL ) ) );
    xFltMgr->setCurrentFilter( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "BASIC" ) ) );

    if( xFP->execute() == RET_OK )
    {
        Sequence< ::rtl::OUString > aPaths = xFP->getFiles();
        aCurPath = aPaths[0];
        SfxMedium aMedium( aCurPath, STREAM_READ | STREAM_SHARE_DENYWRITE | STREAM_NOCREATE );
        SvStream* pStream = aMedium.GetInStream();
        if ( pStream )
        {
            AssertValidEditEngine();
            sal_uLong nLines = CalcLineCount( *pStream );
            // nLines*4: ReadText/Formatting/Highlighting/Formatting
            GetEditorWindow().CreateProgress( String( IDEResId( RID_STR_GENERATESOURCE ) ), nLines*4 );
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
            ErrorBox( this, WB_OK | WB_DEF_OK, IDE_RESSTR(RID_STR_COULDNTREAD) ).Execute();
    }
    return bDone;
}


bool ModulWindow::SaveBasicSource()
{
    DBG_CHKTHIS( ModulWindow, 0 );
    bool bDone = false;

    Reference< lang::XMultiServiceFactory > xMSF( ::comphelper::getProcessServiceFactory() );
    Reference < XFilePicker > xFP;
    if( xMSF.is() )
    {
        Sequence <Any> aServiceType(1);
        aServiceType[0] <<= TemplateDescription::FILESAVE_AUTOEXTENSION_PASSWORD;
        xFP = Reference< XFilePicker >( xMSF->createInstanceWithArguments(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ui.dialogs.FilePicker" ) ), aServiceType ), UNO_QUERY );
    }

    Reference< XFilePickerControlAccess > xFPControl(xFP, UNO_QUERY);
    xFPControl->enableControl(ExtendedFilePickerElementIds::CHECKBOX_PASSWORD, false);
    Any aValue;
    aValue <<= sal_True;
    xFPControl->setValue(ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION, 0, aValue);

    if ( !aCurPath.isEmpty() )
        xFP->setDisplayDirectory ( aCurPath );

    Reference< XFilterManager > xFltMgr(xFP, UNO_QUERY);
    xFltMgr->appendFilter( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "BASIC" ) ), ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "*.bas" ) ) );
    xFltMgr->appendFilter( IDE_RESSTR(RID_STR_FILTER_ALLFILES), ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( FILTERMASK_ALL ) ) );
    xFltMgr->setCurrentFilter( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "BASIC" ) ) );

    if( xFP->execute() == RET_OK )
    {
        Sequence< ::rtl::OUString > aPaths = xFP->getFiles();
        aCurPath = aPaths[0];
        SfxMedium aMedium( aCurPath, STREAM_WRITE | STREAM_SHARE_DENYWRITE | STREAM_TRUNC );
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
            ErrorBox( this, WB_OK | WB_DEF_OK, String( IDEResId( RID_STR_COULDNTWRITE) ) ).Execute();
    }

    return bDone;
}

bool implImportDialog( Window* pWin, const ::rtl::OUString& rCurPath, const ScriptDocument& rDocument, const ::rtl::OUString& aLibName );

bool ModulWindow::ImportDialog()
{
    const ScriptDocument& rDocument = GetDocument();
    ::rtl::OUString aLibName = GetLibName();
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
                        SbMethod* pMethod = (SbMethod*)xModule->GetMethods()->Get( nMethod );
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
    DBG_CHKTHIS( ModulWindow, 0 );
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
    DBG_CHKTHIS( ModulWindow, 0 );
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


long ModulWindow::BasicErrorHdl( StarBASIC * pBasic )
{
    DBG_CHKTHIS( ModulWindow, 0 );
    GoOnTop();

    // ReturnWert: BOOL
    //  FALSE:  cancel
    //  TRUE:   go on....
    ::rtl::OUString aErrorText( pBasic->GetErrorText() );
    sal_uInt16 nErrorLine = pBasic->GetLine() - 1;
    sal_uInt16 nErrCol1 = pBasic->GetCol1();
    sal_uInt16 nErrCol2 = pBasic->GetCol2();
    if ( nErrCol2 != 0xFFFF )
        nErrCol2++;

    AssertValidEditEngine();
    GetEditView()->SetSelection( TextSelection( TextPaM( nErrorLine, nErrCol1 ), TextPaM( nErrorLine, nErrCol2 ) ) );

    ::rtl::OUStringBuffer aErrorTextPrefixBuf;
    if( pBasic->IsCompilerError() )
        aErrorTextPrefixBuf.append(IDE_RESSTR(RID_STR_COMPILEERROR));
    else
    {
        aErrorTextPrefixBuf.append(IDE_RESSTR(RID_STR_RUNTIMEERROR));
        aErrorTextPrefixBuf.append(StarBASIC::GetVBErrorCode(pBasic->GetErrorCode()));
        aErrorTextPrefixBuf.append(' ');
        pLayout->GetStackWindow().UpdateCalls();
    }
    ::rtl::OUString aErrorTextPrefix(aErrorTextPrefixBuf.makeStringAndClear());
    // if other basic, the IDE should try to display the correct module
    bool const bMarkError = pBasic == GetBasic();
    if ( bMarkError )
        aXEditorWindow.GetBrkWindow().SetMarkerPos(nErrorLine, true);

    // #i47002#
    Reference< awt::XWindow > xWindow = VCLUnoHelper::GetInterface( this );

    ErrorHandler::HandleError( StarBASIC::GetErrorCode() );

    // #i47002#
    Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
    if ( !pWindow )
        return false;

    if ( bMarkError )
        aXEditorWindow.GetBrkWindow().SetMarkerPos( MARKER_NOMARKER );
    return false;
}

long ModulWindow::BasicBreakHdl( StarBASIC* pBasic )
{
    DBG_CHKTHIS( ModulWindow, 0 );

    // #i69280 Required in Window despite normal usage in next command!
    (void)pBasic;

    // Return value: sal_uInt16 => see SB-Debug-Flags
    sal_uInt16 nErrorLine = pBasic->GetLine();


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

    pLayout->GetWatchWindow().UpdateWatches();
    pLayout->GetStackWindow().UpdateCalls();

    aStatus.bIsInReschedule = true;
    aStatus.bIsRunning = true;

    AddStatus( BASWIN_INRESCHEDULE );

    BasicIDE::InvalidateDebuggerSlots();

    while( aStatus.bIsRunning )
        Application::Yield();

    aStatus.bIsInReschedule = false;
    aXEditorWindow.GetBrkWindow().SetMarkerPos( MARKER_NOMARKER );

    ClearStatus( BASWIN_INRESCHEDULE );

    return aStatus.nBasicFlags;
}

void ModulWindow::BasicAddWatch()
{
    DBG_CHKTHIS( ModulWindow, 0 );
    String aWatchStr;
    AssertValidEditEngine();
    bool bAdd = true;
    if ( !GetEditView()->HasSelection() )
    {
        TextPaM aWordStart;
        ::rtl::OUString aWord = GetEditEngine()->GetWord( GetEditView()->GetSelection().GetEnd(), &aWordStart );
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
        if ( aSel.GetStart().GetPara() == aSel.GetEnd().GetPara() )
        {
            aWatchStr = GetEditView()->GetSelected();
            pLayout->GetWatchWindow().AddWatch( aWatchStr );
            pLayout->GetWatchWindow().UpdateWatches();
        }
    }
}



void ModulWindow::BasicRemoveWatch()
{
    DBG_CHKTHIS( ModulWindow, 0 );
    pLayout->GetWatchWindow().RemoveSelectedWatch();
}


void ModulWindow::EditMacro( const String& rMacroName )
{
    DBG_CHKTHIS( ModulWindow, 0 );
    DBG_ASSERT( XModule().Is(), "Kein Modul!" );

    if ( XModule().Is() )
    {
        CheckCompileBasic();

        if ( !aStatus.bError )
        {
            sal_uInt16 nStart, nEnd;
            SbMethod* pMethod = (SbMethod*)xModule->Find( rMacroName, SbxCLASS_METHOD );
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
                    long nMaxY = pView->GetTextEngine()->GetTextHeight() - nVisHeight;
                    long nOldStartY = pView->GetStartDocPos().Y();
                    long nNewStartY = nStart * pView->GetTextEngine()->GetCharHeight();
                    nNewStartY = Min( nNewStartY, nMaxY );
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
    DBG_CHKTHIS( ModulWindow, 0 );
    // StoreData is called when the BasicManager is destroyed or
    // this window is closed.
    // => interrupts undesired!
    GetEditorWindow().SetSourceInBasic();
}

bool ModulWindow::CanClose()
{
    DBG_CHKTHIS( ModulWindow, 0 );
    return true;
}


bool ModulWindow::AllowUndo()
{
    return GetEditorWindow().CanModify();
}


void ModulWindow::UpdateData()
{
    DBG_CHKTHIS( ModulWindow, 0 );
    DBG_ASSERT( XModule().Is(), "Kein Modul!" );
    // UpdateData is called when the source has changed from outside
    // => interrupts undesired!

    if ( XModule().Is() )
    {
        SetModule( xModule->GetSource32() );

        if ( GetEditView() )
        {
            TextSelection aSel = GetEditView()->GetSelection();
            setTextEngineText( GetEditEngine(), xModule->GetSource32() );
            GetEditView()->SetSelection( aSel );
            GetEditEngine()->SetModified( false );
            BasicIDE::MarkDocumentModified( GetDocument() );
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
    DBG_CHKTHIS( ModulWindow, 0 );

    AssertValidEditEngine();

    MapMode eOldMapMode( pPrinter->GetMapMode() );
    Font aOldFont( pPrinter->GetFont() );

    Font aFont( GetEditEngine()->GetFont() );
    aFont.SetAlign( ALIGN_BOTTOM );
    aFont.SetTransparent( true );
    aFont.SetSize( Size( 0, 360 ) );
    pPrinter->SetFont( aFont );
    pPrinter->SetMapMode( MAP_100TH_MM );

    String aTitle( CreateQualifiedName() );

    sal_uInt16 nLineHeight = (sal_uInt16) pPrinter->GetTextHeight(); // etwas mehr.
    sal_uInt16 nParaSpace = 10;

    Size aPaperSz = pPrinter->GetOutputSize();
    aPaperSz.Width() -= (LMARGPRN+RMARGPRN);
    aPaperSz.Height() -= (TMARGPRN+BMARGPRN);

    // nLinepPage is not correct if there's a line break
    sal_uInt16 nLinespPage = (sal_uInt16) (aPaperSz.Height()/nLineHeight);
    sal_uInt16 nCharspLine = (sal_uInt16) (aPaperSz.Width() / pPrinter->GetTextWidth(rtl::OUString('X')) );
    sal_uLong nParas = GetEditEngine()->GetParagraphCount();

    sal_uInt16 nPages = (sal_uInt16) (nParas/nLinespPage+1 );
    sal_uInt16 nCurPage = 1;

    lcl_PrintHeader( pPrinter, nPages, nCurPage, aTitle, nPrintPage == 0 );
    Point aPos( LMARGPRN, TMARGPRN );
    for ( sal_uLong nPara = 0; nPara < nParas; nPara++ )
    {
        String aLine( GetEditEngine()->GetText( nPara ) );
        lcl_ConvertTabsToSpaces( aLine );
        sal_uInt16 nLines = aLine.Len()/nCharspLine+1;
        for ( sal_uInt16 nLine = 0; nLine < nLines; nLine++ )
        {
            String aTmpLine( aLine, nLine*nCharspLine, nCharspLine );
            aPos.Y() += nLineHeight;
            if ( aPos.Y() > ( aPaperSz.Height()+TMARGPRN ) )
            {
                nCurPage++;
                lcl_PrintHeader( pPrinter, nPages, nCurPage, aTitle, nCurPage-1 == nPrintPage );
                aPos = Point( LMARGPRN, TMARGPRN+nLineHeight );
            }
            if( nCurPage-1 == nPrintPage )
                pPrinter->DrawText( aPos, aTmpLine );
        }
        aPos.Y() += nParaSpace;
    }

    pPrinter->SetFont( aOldFont );
    pPrinter->SetMapMode( eOldMapMode );

    return sal_Int32(nCurPage);
}


void ModulWindow::ExecuteCommand( SfxRequest& rReq )
{
    DBG_CHKTHIS( ModulWindow, 0 );
    AssertValidEditEngine();
    sal_uInt16 nSlot = rReq.GetSlot();
    switch ( nSlot )
    {
        case SID_DELETE:
        {
            KeyEvent aFakeDelete( 0, KEY_DELETE );
            GetEditView()->KeyInput( aFakeDelete );
            break;
        }
        case SID_SELECTALL:
            GetEditView()->SetSelection( TextSelection( TextPaM( 0, 0 ), TextPaM( 0xFFFFFFFF, 0xFFFF ) ) );
            break;
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
            BasicRemoveWatch();
        }
        break;
        case SID_CUT:
        {
            if ( !IsReadOnly() )
            {
                GetEditView()->Cut();
                SfxBindings* pBindings = BasicIDE::GetBindingsPtr();
                if ( pBindings )
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
                SfxBindings* pBindings = BasicIDE::GetBindingsPtr();
                if ( pBindings )
                    pBindings->Invalidate( SID_DOC_MODIFIED );
            }
        }
        break;
        case SID_BASICIDE_BRKPNTSCHANGED:
        {
            GetBreakPointWindow().Invalidate();
        }
        break;
    }
}



void ModulWindow::GetState( SfxItemSet &rSet )
{
    DBG_CHKTHIS( ModulWindow, 0 );
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
                    String aPos( IDEResId( RID_STR_LINE ) );
                    aPos += ' ';
                    aPos += String::CreateFromInt32( aSel.GetEnd().GetPara()+1 );
                    aPos += String( RTL_CONSTASCII_USTRINGPARAM( ", " ) );
                    aPos += String( IDEResId( RID_STR_COLUMN ) );
                    aPos += ' ';
                    aPos += String::CreateFromInt32( aSel.GetEnd().GetIndex()+1 );
                    SfxStringItem aItem( SID_BASICIDE_STAT_POS, aPos );
                    rSet.Put( aItem );
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
        }
    }
}


void ModulWindow::DoScroll( ScrollBar* pCurScrollBar )
{
    DBG_CHKTHIS( ModulWindow, 0 );
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
    BasicIDEGlobals::GetShell()->GetViewFrame()->ToTop();
}

::rtl::OUString ModulWindow::GetSbModuleName()
{
    ::rtl::OUString aModuleName;
    if ( XModule().Is() )
        aModuleName = xModule->GetName();
    return aModuleName;
}

::rtl::OUString ModulWindow::GetTitle()
{
    return GetSbModuleName();
}

void ModulWindow::FrameWindowMoved()
{
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


Window* ModulWindow::GetLayoutWindow()
{
    return pLayout;
}

void ModulWindow::AssertValidEditEngine()
{
    if ( !GetEditEngine() )
        GetEditorWindow().CreateEditEngine();
}

void ModulWindow::Deactivating()
{
    if ( GetEditView() )
        GetEditView()->EraseVirtualDevice();
}

sal_uInt16 ModulWindow::StartSearchAndReplace( const SvxSearchItem& rSearchItem, bool bFromStart )
{
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
    if ( ( rSearchItem.GetCommand() == SVX_SEARCHCMD_FIND ) ||
         ( rSearchItem.GetCommand() == SVX_SEARCHCMD_FIND_ALL ) )
    {
        nFound = pView->Search( rSearchItem.GetSearchOptions() , bForward );
    }
    else if ( ( rSearchItem.GetCommand() == SVX_SEARCHCMD_REPLACE ) ||
              ( rSearchItem.GetCommand() == SVX_SEARCHCMD_REPLACE_ALL ) )
    {
        if ( !IsReadOnly() )
        {
            bool const bAll = rSearchItem.GetCommand() == SVX_SEARCHCMD_REPLACE_ALL;
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
                SbMethod* pMethod = (SbMethod*)xModule->GetMethods()->Get( nMethod );
                DBG_ASSERT( pMethod, "Methode nicht gefunden! (NULL)" );
                pMethod->SetDebugFlags( pMethod->GetDebugFlags() | SbDEBUG_BREAK );
            }
        }
    }
}

void ModulWindow::BasicStopped()
{
    aStatus.bIsRunning = false;
    GetBreakPointWindow().SetMarkerPos( MARKER_NOMARKER );
}

BasicEntryDescriptor ModulWindow::CreateEntryDescriptor()
{
    ScriptDocument aDocument( GetDocument() );
    String aLibName( GetLibName() );
    LibraryLocation eLocation = aDocument.getLibraryLocation( aLibName );
    String aModName( GetName() );
    String aLibSubName;
    if( xBasic.Is() && aDocument.isInVBAMode() && XModule().Is() )
    {
        switch( xModule->GetModuleType() )
        {
            case script::ModuleType::DOCUMENT:
            {
                aLibSubName = String( IDEResId( RID_STR_DOCUMENT_OBJECTS ) );
                uno::Reference< container::XNameContainer > xLib = aDocument.getOrCreateLibrary( E_SCRIPTS, aLibName );
                if( xLib.is() )
                {
                    ::rtl::OUString sObjName;
                    ModuleInfoHelper::getObjectName( xLib, aModName, sObjName );
                    if( !sObjName.isEmpty() )
                    {
                        aModName.AppendAscii(" (").Append(sObjName).AppendAscii(")");
                    }
                }
                break;
            }
            case script::ModuleType::FORM:
                aLibSubName = String( IDEResId( RID_STR_USERFORMS ) );
                break;
            case script::ModuleType::NORMAL:
                aLibSubName = String( IDEResId( RID_STR_NORMAL_MODULES ) );
                break;
            case script::ModuleType::CLASS:
                aLibSubName = String( IDEResId( RID_STR_CLASS_MODULES ) );
                break;
        }
    }
    return BasicEntryDescriptor( aDocument, eLocation, aLibName, aLibSubName, aModName, OBJ_TYPE_MODULE );
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

void ModulWindow::SetLineNumberDisplay(bool b)
{
    aXEditorWindow.SetLineNumberDisplay(b);
}

void ModulWindow::SetObjectCatalogDisplay(bool b)
{
    aXEditorWindow.SetObjectCatalogDisplay(b);
}

bool ModulWindow::IsPasteAllowed()
{
    bool bPaste = false;

    // get clipboard
    Reference< datatransfer::clipboard::XClipboard > xClipboard = GetClipboard();
    if ( xClipboard.is() )
    {
        // get clipboard content
        const sal_uInt32 nRef = Application::ReleaseSolarMutex();
        Reference< datatransfer::XTransferable > xTransf = xClipboard->getContents();
        Application::AcquireSolarMutex( nRef );
        if ( xTransf.is() )
        {
            datatransfer::DataFlavor aFlavor;
            SotExchange::GetFormatDataFlavor( SOT_FORMAT_STRING, aFlavor );
            if ( xTransf->isDataFlavorSupported( aFlavor ) )
                bPaste = true;
        }
    }

    return bPaste;
}

ModulWindowLayout::ModulWindowLayout( Window* pParent ) :
    Window(pParent, WB_CLIPCHILDREN),
    bFirstArrange(true),
    aLeftSplit(this, WB_HSCROLL),
    aBottomSplit(this, WB_VSCROLL),
    aVertSplit(this, WB_HSCROLL),
    aObjectCatalog(this),
    aWatchWindow(this),
    aStackWindow(this),
    m_pModulWindow(0),
    m_aImagesNormal(IDEResId(RID_IMGLST_LAYOUT))
{
    SetBackground(GetSettings().GetStyleSettings().GetWindowColor());

    aLeftSplit.Show();
    aBottomSplit.Show();
    aVertSplit.Show();
    aLeftSplit.SetSplitHdl( LINK(this, ModulWindowLayout, SplitHdl) );
    aBottomSplit.SetSplitHdl( LINK(this, ModulWindowLayout, SplitHdl) );
    aVertSplit.SetSplitHdl( LINK(this, ModulWindowLayout, SplitHdl) );

    aWatchWindow.Show();
    aStackWindow.Show();
    aObjectCatalog.Show();

    Color aColor(GetSettings().GetStyleSettings().GetFieldTextColor());
    m_aSyntaxColors[TT_UNKNOWN] = aColor;
    m_aSyntaxColors[TT_WHITESPACE] = aColor;
    m_aSyntaxColors[TT_EOL] = aColor;
    m_aColorConfig.AddListener(this);
    m_aSyntaxColors[TT_IDENTIFIER]
        = Color(m_aColorConfig.GetColorValue(svtools::BASICIDENTIFIER).nColor);
    m_aSyntaxColors[TT_NUMBER]
        = Color(m_aColorConfig.GetColorValue(svtools::BASICNUMBER).nColor);
    m_aSyntaxColors[TT_STRING]
        = Color(m_aColorConfig.GetColorValue(svtools::BASICSTRING).nColor);
    m_aSyntaxColors[TT_COMMENT]
        = Color(m_aColorConfig.GetColorValue(svtools::BASICCOMMENT).nColor);
    m_aSyntaxColors[TT_ERROR]
        = Color(m_aColorConfig.GetColorValue(svtools::BASICERROR).nColor);
    m_aSyntaxColors[TT_OPERATOR]
        = Color(m_aColorConfig.GetColorValue(svtools::BASICOPERATOR).nColor);
    m_aSyntaxColors[TT_KEYWORDS]
        = Color(m_aColorConfig.GetColorValue(svtools::BASICKEYWORD).nColor);

    Font aFont( GetFont() );
    Size aSz( aFont.GetSize() );
    aSz.Height() *= 3;
    aSz.Height() /= 2;
    aFont.SetSize( aSz );
    aFont.SetWeight( WEIGHT_BOLD );
    aFont.SetColor(GetSettings().GetStyleSettings().GetWindowTextColor());
    SetFont( aFont );
}

ModulWindowLayout::~ModulWindowLayout()
{
    m_aColorConfig.RemoveListener(this);
}

void ModulWindowLayout::Resize()
{
    // ScrollBars, etc. happens in BasicIDEShell:Adjust...
    ArrangeWindows();
}

void ModulWindowLayout::Paint( const Rectangle& )
{
    DrawText( Point(), String( IDEResId( RID_STR_NOMODULE ) ) );
}


void ModulWindowLayout::ArrangeWindows()
{
    static long const nSplitThickness = 2;
    static double const
        fDefaultLeftSplit = 0.2,
        fDefaultBottomSplit = 0.75,
        fDefaultVertSplit = 0.67;

    Size const aSize = GetOutputSizePixel();
    long const nWidth = aSize.Width(), nHeight = aSize.Height();
    if (!nWidth || !nHeight) // empty size
        return;

    // When ArrangeWindows() is called first,
    // the initial positions of the splitter lines are set.
    if (bFirstArrange)
    {
        aLeftSplit.SetSplitPosPixel(aSize.Width()  * fDefaultLeftSplit);
        aBottomSplit.SetSplitPosPixel(aSize.Height() * fDefaultBottomSplit);
        aVertSplit.SetSplitPosPixel(aSize.Width()  * fDefaultVertSplit);
        bFirstArrange = false;
    }

    // resizing windows to the splitting lines
    long const nLeftSplitPos = aLeftSplit.GetSplitPosPixel();
    long const nBottomSplitPos = aBottomSplit.GetSplitPosPixel();
    long const nVertSplitPos = aVertSplit.GetSplitPosPixel();
    // which window is docked?
    bool const bObjCat = !aObjectCatalog.IsFloatingMode() && aObjectCatalog.IsVisible();
    bool const bWatchWin = !aWatchWindow.IsFloatingMode() && aWatchWindow.IsVisible();
    bool const bStackWin = !aStackWindow.IsFloatingMode() && aStackWindow.IsVisible();
    long const nBottom = bStackWin || bWatchWin ? nBottomSplitPos : nHeight;
    // left splitting line
    if (bObjCat)
    {
        aLeftSplit.SetDragRectPixel(Rectangle(Point(0, 0), Size(nWidth, nBottom)));
        aLeftSplit.SetPosPixel(Point(nLeftSplitPos, 0));
        aLeftSplit.SetSizePixel(Size(nSplitThickness, nBottom));
        aLeftSplit.Show();
    }
    else
        aLeftSplit.Hide();
    // bottom splitting line
    if (bWatchWin || bStackWin)
    {
        aBottomSplit.SetDragRectPixel(Rectangle(Point(0, 0), aSize));
        aBottomSplit.SetPosPixel(Point(0, nBottomSplitPos));
        aBottomSplit.SetSizePixel(Size(nWidth, nSplitThickness));
        aBottomSplit.Show();
    }
    else
        aBottomSplit.Hide();
    // vertical (bottom) splitting line
    if (bWatchWin || bStackWin)
    {
        Point const aPos(nVertSplitPos, nBottomSplitPos + nSplitThickness);
        aVertSplit.SetDragRectPixel(Rectangle(Point(0, aPos.Y()), Size(nWidth, nHeight - aPos.Y())));
        aVertSplit.SetPosSizePixel(aPos, Size(nSplitThickness, nHeight - aPos.Y()));
        aVertSplit.Show();
    }
    else
        aVertSplit.Hide();
    // editor window
    if (m_pModulWindow)
    {
        DBG_CHKOBJ(m_pModulWindow, ModulWindow, 0);
        long const nLeft = bObjCat ? nLeftSplitPos + nSplitThickness : 0;
        m_pModulWindow->SetPosSizePixel(
            Point(nLeft, 0),
            Size(nWidth - nLeft, nBottom)
        );
    }
    // object catalog (left)
    if (bObjCat)
    {
        aObjectCatalog.SetPosPixel(Point(0, 0));
        aObjectCatalog.SetSizePixel(Size(nLeftSplitPos, nBottom));
    }
    // watch (bottom left)
    if (bWatchWin)
    {
        Point const aPos(0, nBottomSplitPos + nSplitThickness);
        aWatchWindow.SetPosPixel(aPos);
        aWatchWindow.SetSizePixel(Size(nVertSplitPos, nHeight - aPos.Y()));
    }
    // call stack (bottom right)
    if (bStackWin)
    {
        Point const aPos(nVertSplitPos + nSplitThickness, nBottomSplitPos + nSplitThickness);
        aStackWindow.SetPosPixel(aPos);
        aStackWindow.SetSizePixel(Size(nWidth - aPos.X(), nHeight - aPos.Y()));
    }
}

IMPL_LINK( ModulWindowLayout, SplitHdl, Splitter *, pSplitter )
{
    // The split line cannot be closer to the edges than nMargin pixels.
    static long const nMargin = 16;
    // Checking margins:
    if (long const nSize = pSplitter->IsHorizontal() ?
        GetOutputSizePixel().Width() :
        GetOutputSizePixel().Height()
    ) {
        long const nPos = pSplitter->GetSplitPosPixel();
        if (nPos < nMargin)
            pSplitter->SetSplitPosPixel(nMargin);
        if (nPos > nSize - nMargin)
            pSplitter->SetSplitPosPixel(nSize - nMargin);
    }
    ArrangeWindows();
    return 0;
}

//
// IsToBeDocked() -- test whether dock or child:
// true:  Floating
// false: Child
//
bool ModulWindowLayout::IsToBeDocked (
    DockingWindow* pDockingWindow, Point const& rPos, Rectangle& rRect
) {
    Point const aPos = ScreenToOutputPixel(rPos);
    Size const aSize = GetOutputSizePixel();
    long const nWidth = aSize.Width(), nHeight = aSize.Height();

    if (aPos.X() > 0 && aPos.X() < nWidth && aPos.Y() > 0 && aPos.Y() < nHeight)
    {
        long const nLeftSplit = aLeftSplit.GetSplitPosPixel();
        long const nBottomSplit = aBottomSplit.GetSplitPosPixel();
        long const nVertSplit = aVertSplit.GetSplitPosPixel();
        if (pDockingWindow == &aObjectCatalog)
        {
            if (aPos.Y() < nBottomSplit && aPos.X() < nLeftSplit)
            {
                rRect = Rectangle(
                    OutputToScreenPixel(Point(0, 0)),
                    Size(nLeftSplit, nBottomSplit)
                );
                return true;
            }
        }
        else if (pDockingWindow == &aWatchWindow)
        {
            if (aPos.Y() > nBottomSplit && aPos.X() < nVertSplit)
            {
                rRect = Rectangle(
                    OutputToScreenPixel(Point(0, nBottomSplit)),
                    Size(nVertSplit, nHeight - nBottomSplit)
                );
                return true;
            }
        }
        else if (pDockingWindow == &aStackWindow)
        {
            if (aPos.Y() > nBottomSplit && aPos.X() > nVertSplit)
            {
                rRect = Rectangle(
                    OutputToScreenPixel(Point(nVertSplit, nBottomSplit)),
                    Size(nWidth - nVertSplit, nHeight - nBottomSplit)
                );
                return true;
            }
        }
    }
    return false;
}

void ModulWindowLayout::DockaWindow (DockingWindow*)
{
    ArrangeWindows();
}

void ModulWindowLayout::SetModulWindow (ModulWindow* pModulWindow)
{
    m_pModulWindow = pModulWindow;
    ArrangeWindows();
}

// virtual
void ModulWindowLayout::DataChanged(DataChangedEvent const & rDCEvt)
{
    Window::DataChanged(rDCEvt);
    if (rDCEvt.GetType() == DATACHANGED_SETTINGS
        && (rDCEvt.GetFlags() & SETTINGS_STYLE) != 0)
    {
        bool bInvalidate = false;
        Color aColor(GetSettings().GetStyleSettings().GetWindowColor());
        if (aColor
            != rDCEvt.GetOldSettings()->GetStyleSettings().GetWindowColor())
        {
            SetBackground(Wallpaper(aColor));
            bInvalidate = true;
        }
        aColor = GetSettings().GetStyleSettings().GetWindowTextColor();
        if (aColor != rDCEvt.GetOldSettings()->
            GetStyleSettings().GetWindowTextColor())
        {
            Font aFont(GetFont());
            aFont.SetColor(aColor);
            SetFont(aFont);
            bInvalidate = true;
        }
        if (bInvalidate)
            Invalidate();
        aColor = GetSettings().GetStyleSettings().GetFieldTextColor();
        if (aColor != m_aSyntaxColors[TT_UNKNOWN])
        {
            m_aSyntaxColors[TT_UNKNOWN] = aColor;
            m_aSyntaxColors[TT_WHITESPACE] = aColor;
            m_aSyntaxColors[TT_EOL] = aColor;
            updateSyntaxHighlighting();
        }
    }
}

// virtual
void ModulWindowLayout::ConfigurationChanged( utl::ConfigurationBroadcaster*, sal_uInt32 )
{
    {
        Color aColor(m_aColorConfig.GetColorValue(svtools::BASICIDENTIFIER).
                     nColor);
        bool bChanged = aColor != m_aSyntaxColors[TT_IDENTIFIER];
        m_aSyntaxColors[TT_IDENTIFIER] = aColor;
        aColor = Color(m_aColorConfig.GetColorValue(svtools::BASICNUMBER).nColor);
    if (bChanged || aColor != m_aSyntaxColors[TT_NUMBER])
            bChanged = true;
        m_aSyntaxColors[TT_NUMBER] = aColor;
        aColor = Color(m_aColorConfig.GetColorValue(svtools::BASICSTRING).nColor);
        if (bChanged || aColor != m_aSyntaxColors[TT_STRING])
            bChanged = true;
        m_aSyntaxColors[TT_STRING] = aColor;
        aColor = Color(m_aColorConfig.GetColorValue(svtools::BASICCOMMENT).
                       nColor);
        if (bChanged || aColor != m_aSyntaxColors[TT_COMMENT])
            bChanged = true;
        m_aSyntaxColors[TT_COMMENT] = aColor;
        aColor = Color(m_aColorConfig.GetColorValue(svtools::BASICERROR).nColor);
        if (bChanged || aColor != m_aSyntaxColors[TT_ERROR])
            bChanged = true;
        m_aSyntaxColors[TT_ERROR] = aColor;
        aColor = Color(m_aColorConfig.GetColorValue(svtools::BASICOPERATOR).
                       nColor);
        if (bChanged || aColor != m_aSyntaxColors[TT_OPERATOR])
            bChanged = true;
        m_aSyntaxColors[TT_OPERATOR] = aColor;
        aColor = Color(m_aColorConfig.GetColorValue(svtools::BASICKEYWORD).
                       nColor);
        if (bChanged || aColor != m_aSyntaxColors[TT_KEYWORDS])
            bChanged = true;
        m_aSyntaxColors[TT_KEYWORDS] = aColor;
        if (bChanged)
            updateSyntaxHighlighting();
    }
}

void ModulWindowLayout::updateSyntaxHighlighting()
{
    if (m_pModulWindow != 0)
    {
        EditorWindow & rEditor = m_pModulWindow->GetEditorWindow();
        sal_uLong nCount = rEditor.GetEditEngine()->GetParagraphCount();
        for (sal_uLong i = 0; i < nCount; ++i)
            rEditor.DoDelayedSyntaxHighlight(i);
    }
}

Image ModulWindowLayout::getImage(sal_uInt16 nId) const
{
    return m_aImagesNormal.GetImage(nId);
}

// shows or hides the Object Catalog window (depending on its state)
void ModulWindowLayout::ToggleObjectCatalog ()
{
    // show or hide?
    bool const bShow = !aObjectCatalog.IsVisible();
    bShow ? aObjectCatalog.Show() : aObjectCatalog.Hide();
    if (m_pModulWindow)
        m_pModulWindow->SetObjectCatalogDisplay(bShow);
    // refreshing
    ArrangeWindows();
}

// Updates the Object Catalog window.
void ModulWindowLayout::UpdateObjectCatalog ()
{
    aObjectCatalog.UpdateEntries();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
