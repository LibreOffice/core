/*************************************************************************
 *
 *  $RCSfile: inputwin.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2000-09-22 18:39:19 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------

#include "scitems.hxx"
#include <svx/editview.hxx>
#include <svx/editstat.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/event.hxx>
#include <sfx2/imgmgr.hxx>
#include <offmgr/app.hxx>
#include <vcl/system.hxx>
#include <stdlib.h>     // qsort
#include <svx/eeitem.hxx>

#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif

#include "inputwin.hxx"
#include "scmod.hxx"
#include "uiitems.hxx"
#include "global.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "globstr.hrc"
#include "editutil.hxx"
#include "inputhdl.hxx"
#include "tabvwsh.hxx"
#include "document.hxx"
#include "docsh.hxx"
#include "appoptio.hxx"
#include "rangenam.hxx"
#include "compiler.hrc"

#ifndef _SVX_FONTITEM_HXX //autogen
#include <svx/fontitem.hxx>
#endif



#define TEXT_STARTPOS       3
#define THESIZE             1000000 //!!! langt... :-)
#define TBX_WINDOW_HEIGHT   22 // in Pixeln - fuer alle Systeme gleich?

#if defined OS2 || defined MAC
#define INPUTWIN_APPFONT
#else
#undef INPUTWIN_APPFONT
#endif

#ifdef INPUTWIN_APPFONT
//  Unter OS/2 wird die Fontgroesse vom App-Font genommen
#else
#define TEXT_FONT_HEIGHT    180         // 9pt
#endif

// STATIC DATA -----------------------------------------------------------

//==================================================================
//  class ScInputWindowWrapper
//==================================================================

SFX_IMPL_CHILDWINDOW(ScInputWindowWrapper,FID_INPUTLINE_STATUS)

ScInputWindowWrapper::ScInputWindowWrapper( Window*          pParent,
                                            USHORT           nId,
                                            SfxBindings*     pBindings,
                                            SfxChildWinInfo* pInfo )
    :   SfxChildWindow( pParent, nId )
{
    ScInputWindow* pWin=new ScInputWindow( pParent );
    pWindow = pWin;

    pWin->Show();

    pWin->SetSizePixel( pWin->CalcWindowSizePixel() );

    eChildAlignment = SFX_ALIGN_LOWESTTOP;
    pBindings->Invalidate( FID_TOGGLEINPUTLINE );
}

//  GetInfo fliegt wieder raus, wenn es ein SFX_IMPL_TOOLBOX gibt !!!!

SfxChildWinInfo __EXPORT ScInputWindowWrapper::GetInfo() const
{
    SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();
    return aInfo;
}

//==================================================================

#define IMAGE(id) pImgMgr->SeekImage(id,pScMod)

//==================================================================
//  class ScInputWindow
//==================================================================

ScInputWindow::ScInputWindow( Window* pParent ) :
#ifdef OS2
// #37192# ohne WB_CLIPCHILDREN wg. os/2 Paintproblem
        ToolBox         ( pParent, WinBits(WB_BORDER|WB_3DLOOK) ),
#else
// mit WB_CLIPCHILDREN, sonst Flicker
        ToolBox         ( pParent, WinBits(WB_BORDER|WB_3DLOOK|WB_CLIPCHILDREN) ),
#endif
        aWndPos         ( this ),
        aTextWindow     ( this ),
        aTextOk         ( ScResId( SCSTR_QHELP_BTNOK ) ),       // nicht immer neu aus Resource
        aTextCancel     ( ScResId( SCSTR_QHELP_BTNCANCEL ) ),
        aTextSum        ( ScResId( SCSTR_QHELP_BTNSUM ) ),
        aTextEqual      ( ScResId( SCSTR_QHELP_BTNEQUAL ) ),
        bIsOkCancelMode ( FALSE ),
        pInputHdl       ( NULL )
{
    SfxImageManager* pImgMgr = SFX_IMAGEMANAGER();
    ScTabViewShell*  pViewSh = PTR_CAST( ScTabViewShell, SfxViewShell::Current() );
    ScModule*        pScMod  = SC_MOD();

    // Positionsfenster, 3 Buttons, Eingabefenster
    InsertWindow    ( 1, &aWndPos, 0,                                     0 );
    InsertSeparator (                                                     1 );
    InsertItem      ( SID_INPUT_FUNCTION, IMAGE( SID_INPUT_FUNCTION ), 0, 2 );
    InsertItem      ( SID_INPUT_SUM,      IMAGE( SID_INPUT_SUM ), 0,      3 );
    InsertItem      ( SID_INPUT_EQUAL,    IMAGE( SID_INPUT_EQUAL ), 0,    4 );
    InsertSeparator (                                                     5 );
    InsertWindow    ( 7, &aTextWindow, 0,                                 6 );

    aWndPos    .SetQuickHelpText( ScResId( SCSTR_QHELP_POSWND ) );
    aWndPos    .SetHelpId       ( HID_INSWIN_POS );
    aTextWindow.SetQuickHelpText( ScResId( SCSTR_QHELP_INPUTWND ) );
    aTextWindow.SetHelpId       ( HID_INSWIN_INPUT );

    //  kein SetHelpText, die Hilfetexte kommen aus der Hilfe

    SetItemText ( SID_INPUT_FUNCTION, ScResId( SCSTR_QHELP_BTNCALC ) );
    SetHelpId   ( SID_INPUT_FUNCTION, HID_INSWIN_CALC );

    SetItemText ( SID_INPUT_SUM, aTextSum );
    SetHelpId   ( SID_INPUT_SUM, HID_INSWIN_SUMME );

    SetItemText ( SID_INPUT_EQUAL, aTextEqual );
    SetHelpId   ( SID_INPUT_EQUAL, HID_INSWIN_FUNC );

    SetHelpId( HID_SC_INPUTWIN );   // fuer die ganze Eingabezeile

    aWndPos     .Show();
    aTextWindow .Show();

    pInputHdl = SC_MOD()->GetInputHdl();
    if (pInputHdl)
        pInputHdl->SetInputWindow( this );

    if ( pInputHdl && pInputHdl->GetFormString().Len() )
    {
        //  Umschalten waehrend der Funktionsautopilot aktiv ist
        //  -> Inhalt des Funktionsautopiloten wieder anzeigen
        //! auch Selektion (am InputHdl gemerkt) wieder anzeigen

        aTextWindow.SetTextString( pInputHdl->GetFormString() );
    }
    else if ( pInputHdl && pInputHdl->IsInputMode() )
    {
        //  wenn waehrend des Editierens die Eingabezeile weg war
        //  (Editieren einer Formel, dann umschalten zu fremdem Dokument/Hilfe),
        //  wieder den gerade editierten Text aus dem InputHandler anzeigen

        aTextWindow.SetTextString( pInputHdl->GetEditString() );    // Text anzeigen
        if ( pInputHdl->IsTopMode() )
            pInputHdl->SetMode( SC_INPUT_TABLE );       // Focus kommt eh nach unten
    }
    else if ( pViewSh )
        pViewSh->UpdateInputHandler( TRUE ); // unbedingtes Update

    pImgMgr->RegisterToolBox( this, SC_MOD() );
}

__EXPORT ScInputWindow::~ScInputWindow()
{
    BOOL bDown = ( ScGlobal::pScInternational == NULL );        // schon nach Clear?

    //  if any view's input handler has a pointer to this input window, reset it
    //  (may be several ones, #74522#)
    //  member pInputHdl is not used here

    if ( !bDown )
    {
        TypeId aScType = TYPE(ScTabViewShell);
        SfxViewShell* pSh = SfxViewShell::GetFirst( &aScType );
        while ( pSh )
        {
            ScInputHandler* pHdl = ((ScTabViewShell*)pSh)->GetInputHandler();
            if ( pHdl && pHdl->GetInputWindow() == this )
                pHdl->SetInputWindow( NULL );
            pSh = SfxViewShell::GetNext( *pSh, &aScType );
        }
    }

    SFX_IMAGEMANAGER()->ReleaseToolBox( this );
}

void ScInputWindow::SetInputHandler( ScInputHandler* pNew )
{
    //  wird im Activate der View gerufen...

    DBG_ASSERT(pNew,"SetInputHandler: NULL");
    if ( pNew != pInputHdl )
    {
        //  Bei Reload (letzte Version) ist pInputHdl der Input-Handler der alten,
        //  geloeschten ViewShell, darum hier auf keinen Fall anfassen!

        pInputHdl = pNew;
        if (pInputHdl)
            pInputHdl->SetInputWindow( this );
    }
}

void __EXPORT ScInputWindow::Select()
{
    ScModule* pScMod = SC_MOD();
    ToolBox::Select();

    switch ( GetCurItemId() )
    {
        case SID_INPUT_FUNCTION:
            {
                //! new method at ScModule to query if function autopilot is open
                SfxViewFrame* pViewFrm = SfxViewFrame::Current();
                if ( pViewFrm && !pViewFrm->GetChildWindow( SID_OPENDLG_FUNCTION ) )
                {
                    pViewFrm->GetDispatcher()->Execute( SID_OPENDLG_FUNCTION,
                                              SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD );

                    //  die Toolbox wird sowieso disabled, also braucht auch nicht umgeschaltet
                    //  zu werden, egal ob's geklappt hat oder nicht
//                  SetOkCancelMode();
                }
            }
            break;

        case SID_INPUT_CANCEL:
            pScMod->InputCancelHandler();
            SetSumAssignMode();
            break;

        case SID_INPUT_OK:
            pScMod->InputEnterHandler();
            SetSumAssignMode();
            aTextWindow.Invalidate();       // sonst bleibt Selektion stehen
            break;

        case SID_INPUT_SUM:
            {
                ScTabViewShell* pViewSh = PTR_CAST( ScTabViewShell, SfxViewShell::Current() );
                if ( pViewSh )
                {
                    ScMarkData& rMark = pViewSh->GetViewData()->GetMarkData();

                    ScRangeList* pRangeList = new ScRangeList;
                    BOOL bDataFound = pViewSh->GetAutoSumArea( *pRangeList );

                    if ((rMark.IsMarked() || rMark.IsMultiMarked()) && bDataFound)
                    {
                        pViewSh->EnterAutoSum( *pRangeList );   // Block mit Summen fuellen
                    }
                    else                                    // nur in Eingabezeile einfuegen
                    {
                        String aFormula = '=';
                        ScFunctionMgr* pFuncMgr = ScGlobal::GetStarCalcFunctionMgr();
                        ScFuncDesc* pDesc = pFuncMgr->Get( SC_OPCODE_SUM );
                        if ( pDesc && pDesc->pFuncName )
                        {
                            aFormula += *pDesc->pFuncName;
                            aFormula.AppendAscii(RTL_CONSTASCII_STRINGPARAM( "()" ));
                        }

                        ScDocument* pDoc = pViewSh->GetViewData()->GetDocument();
                        xub_StrLen nPos = aFormula.Len() - 1;
                        String aRef;
                        pRangeList->Format( aRef, SCA_VALID, pDoc );
                        aFormula.Insert( aRef, nPos );

                        SetFuncString( aFormula );

                        if ( bDataFound && pScMod->IsEditMode() )
                        {
                            ScInputHandler* pHdl = pScMod->GetInputHdl( pViewSh );
                            if ( pHdl )
                            {
                                pHdl->InitRangeFinder( aFormula );

                                //! SetSelection am InputHandler ???
                                //! bSelIsRef setzen ???
                                xub_StrLen nOpen = aFormula.Search('(');
                                xub_StrLen nLen = aFormula.Len();
                                if ( nOpen != STRING_NOTFOUND && nLen > nOpen )
                                {
                                    ESelection aSel(0,nOpen+1,0,nLen-1);
                                    EditView* pTableView = pHdl->GetTableView();
                                    if (pTableView)
                                        pTableView->SetSelection(aSel);
                                    EditView* pTopView = pHdl->GetTopView();
                                    if (pTopView)
                                        pTopView->SetSelection(aSel);
                                }
                            }
                        }
                    }
                    delete pRangeList;
                }
            }
            break;

        case SID_INPUT_EQUAL:
        {
            aTextWindow.StartEditEngine();
            if ( pScMod->IsEditMode() )         // nicht, wenn z.B. geschuetzt
            {
                aTextWindow.GrabFocus();
                aTextWindow.SetTextString( '=' );

                EditView* pView = aTextWindow.GetEditView();
                if (pView)
                {
                    pView->SetSelection( ESelection(0,1, 0,1) );
                    pScMod->InputChanged(pView);
                    SetOkCancelMode();
                    pView->SetEditEngineUpdateMode(TRUE);
                }
            }
            break;
        }
    }
}

void __EXPORT ScInputWindow::Resize()
{
    ToolBox::Resize();

    long nWidth = GetSizePixel().Width();
    long nLeft  = aTextWindow.GetPosPixel().X();
    Size aSize  = aTextWindow.GetSizePixel();

    aSize.Width() = Max( ((long)(nWidth - nLeft - 5)), (long)0 );
    aTextWindow.SetSizePixel( aSize );
    aTextWindow.Invalidate();
}

void ScInputWindow::SetFuncString( const String& rString, BOOL bDoEdit )
{
    //! new method at ScModule to query if function autopilot is open
    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    EnableButtons( pViewFrm && !pViewFrm->GetChildWindow( SID_OPENDLG_FUNCTION ) );
    aTextWindow.StartEditEngine();

    ScModule* pScMod = SC_MOD();
    if ( pScMod->IsEditMode() )
    {
        aTextWindow.GrabFocus();
        aTextWindow.SetTextString( rString );
        EditView* pView = aTextWindow.GetEditView();
        if (pView)
        {
            xub_StrLen nLen = rString.Len();

            if ( nLen > 0 )
            {
                nLen--;
                pView->SetSelection( ESelection( 0, nLen, 0, nLen ) );
            }

            pScMod->InputChanged(pView);
            if ( bDoEdit )
                SetOkCancelMode();          // nicht, wenn gleich hinterher Enter/Cancel

            pView->SetEditEngineUpdateMode(TRUE);
        }
    }
}

void ScInputWindow::SetPosString( const String& rStr )
{
    aWndPos.SetPos( rStr );
}

void ScInputWindow::SetTextString( const String& rString )
{
    if (rString.Len() <= 32767)
        aTextWindow.SetTextString(rString);
    else
    {
        String aNew = rString;
        aNew.Erase(32767);
        aTextWindow.SetTextString(aNew);
    }
}

const String& ScInputWindow::GetTextString()
{
    return aTextWindow.GetTextString();
}

void ScInputWindow::SetOkCancelMode()
{
    //! new method at ScModule to query if function autopilot is open
    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    EnableButtons( pViewFrm && !pViewFrm->GetChildWindow( SID_OPENDLG_FUNCTION ) );

    ScModule* pScMod = SC_MOD();
    SfxImageManager* pImgMgr = SFX_IMAGEMANAGER();
    if (!bIsOkCancelMode)
    {
        RemoveItem( 3 ); // SID_INPUT_SUM und SID_INPUT_EQUAL entfernen
        RemoveItem( 3 );
        InsertItem( SID_INPUT_CANCEL, IMAGE( SID_INPUT_CANCEL ), 0, 3 );
        InsertItem( SID_INPUT_OK,     IMAGE( SID_INPUT_OK ),     0, 4 );
        SetItemText ( SID_INPUT_CANCEL, aTextCancel );
        SetHelpId   ( SID_INPUT_CANCEL, HID_INSWIN_CANCEL );
        SetItemText ( SID_INPUT_OK,     aTextOk );
        SetHelpId   ( SID_INPUT_OK,     HID_INSWIN_OK );
        bIsOkCancelMode = TRUE;
    }
}

void ScInputWindow::SetSumAssignMode()
{
    //! new method at ScModule to query if function autopilot is open
    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    EnableButtons( pViewFrm && !pViewFrm->GetChildWindow( SID_OPENDLG_FUNCTION ) );

    ScModule* pScMod = SC_MOD();
    SfxImageManager* pImgMgr = SFX_IMAGEMANAGER();
    if (bIsOkCancelMode)
    {
        // SID_INPUT_CANCEL, und SID_INPUT_OK entfernen
        RemoveItem( 3 );
        RemoveItem( 3 );
        InsertItem( SID_INPUT_SUM,   IMAGE( SID_INPUT_SUM ),     0, 3 );
        InsertItem( SID_INPUT_EQUAL, IMAGE( SID_INPUT_EQUAL ),   0, 4 );
        SetItemText ( SID_INPUT_SUM,   aTextSum );
        SetHelpId   ( SID_INPUT_SUM,   HID_INSWIN_SUMME );
        SetItemText ( SID_INPUT_EQUAL, aTextEqual );
        SetHelpId   ( SID_INPUT_EQUAL, HID_INSWIN_FUNC );
        bIsOkCancelMode = FALSE;

        SetFormulaMode(FALSE);      // kein editieren -> keine Formel
    }
}

void ScInputWindow::SetFormulaMode( BOOL bSet )
{
    aWndPos.SetFormulaMode(bSet);
    aTextWindow.SetFormulaMode(bSet);
}

void __EXPORT ScInputWindow::SetText( const String& rString )
{
    ToolBox::SetText(rString);
}

String __EXPORT ScInputWindow::GetText() const
{
    return ToolBox::GetText();
}


EditView* ScInputWindow::ActivateEdit( const String&     rText,
                                       const ESelection& rSel )
{
    if ( !aTextWindow.IsActive() )
    {
        aTextWindow.StartEditEngine();
        aTextWindow.GrabFocus();
        aTextWindow.SetTextString( rText );
        aTextWindow.GetEditView()->SetSelection( rSel );
    }

    return aTextWindow.GetEditView();
}

BOOL ScInputWindow::IsActive()
{
    return aTextWindow.IsActive();
}

EditView* ScInputWindow::GetEditView()
{
    return aTextWindow.GetEditView();
}

void ScInputWindow::MakeDialogEditView()
{
    aTextWindow.MakeDialogEditView();
}

void ScInputWindow::StopEditEngine()
{
    aTextWindow.StopEditEngine();
}

void ScInputWindow::TextGrabFocus()
{
    aTextWindow.GrabFocus();
}

void ScInputWindow::TextInvalidate()
{
    aTextWindow.Invalidate();
}

void ScInputWindow::PosGrabFocus()
{
    aWndPos.GrabFocus();
}

void ScInputWindow::EnableButtons( BOOL bEnable )
{
    EnableItem( SID_INPUT_FUNCTION,                                   bEnable );
    EnableItem( bIsOkCancelMode ? SID_INPUT_CANCEL : SID_INPUT_SUM,   bEnable );
    EnableItem( bIsOkCancelMode ? SID_INPUT_OK     : SID_INPUT_EQUAL, bEnable );
//  Invalidate();
}

void ScInputWindow::StateChanged( StateChangedType nType )
{
    ToolBox::StateChanged( nType );

    if ( nType == STATE_CHANGE_INITSHOW ) Resize();
}
//========================================================================
//                          Eingabefenster
//========================================================================

ScTextWnd::ScTextWnd( Window* pParent )
    :   Window       ( pParent, WinBits(WB_HIDE | WB_BORDER) ),
        pEditEngine  ( NULL ),
        pEditView    ( NULL ),
        bIsInsertMode( TRUE ),
        bFormulaMode ( FALSE )
{
    //  Unter OS/2 sieht alles andere als der App-Font bei kleinen Aufloesungen
    //  voellig daneben aus. Unter Windows kann der App-Font z.B. die typographischen
    //  Anfuehrungszeichen nicht darstellen, darum hier die Unterscheidung:
#ifdef INPUTWIN_APPFONT
    Font aAppFont = GetFont();
    aTextFont = aAppFont;
    aTextFont.SetSize( PixelToLogic( aAppFont.GetSize(), MAP_TWIP ) );  // AppFont ist in Pixeln
#else
    aTextFont = System::GetStandardFont( STDFONT_SWISS );
    aTextFont.SetSize( Size(0,TEXT_FONT_HEIGHT) );
#endif

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

    Color aBgColor= rStyleSettings.GetWindowColor();
    Color aTxtColor= rStyleSettings.GetWindowTextColor();

    aTextFont.SetTransparent ( TRUE );
    aTextFont.SetFillColor   ( aBgColor );
    //aTextFont.SetColor         ( COL_FIELDTEXT );
    aTextFont.SetColor       (aTxtColor);
    aTextFont.SetWeight      ( WEIGHT_NORMAL );

    SetSizePixel        ( Size(1,TBX_WINDOW_HEIGHT) );
    SetBackground       ( aBgColor );
    SetLineColor        ( COL_BLACK );
    SetMapMode          ( MAP_TWIP );
    SetPointer          ( POINTER_TEXT );
}

__EXPORT ScTextWnd::~ScTextWnd()
{
    delete pEditView;
    delete pEditEngine;
}

void __EXPORT ScTextWnd::Paint( const Rectangle& rRec )
{
    if (pEditView)
        pEditView->Paint( rRec );
    else
    {
        SetFont( aTextFont );

        long nDiff =  GetOutputSizePixel().Height()
                    - LogicToPixel( Size( 0, GetTextHeight() ) ).Height();
//      if (nDiff<2) nDiff=2;       // mind. 1 Pixel

        //! Clipping am rechten Rand?
        DrawText( PixelToLogic( Point( TEXT_STARTPOS, nDiff/2 ) ), aString );
    }
}

void __EXPORT ScTextWnd::Resize()
{
    if (pEditView)
    {
        Size aSize = GetOutputSizePixel();
        long nDiff =  aSize.Height()
                    - LogicToPixel( Size( 0, GetTextHeight() ) ).Height();

#ifdef OS2_DOCH_NICHT
        nDiff-=2;       // wird durch 2 geteilt
                        // passt sonst nicht zur normalen Textausgabe
#endif

        aSize.Width() -= 4;

        pEditView->SetOutputArea(
            PixelToLogic( Rectangle( Point( TEXT_STARTPOS, (nDiff > 0) ? nDiff/2 : 1 ),
                                     aSize ) ) );
    }
}

void __EXPORT ScTextWnd::MouseMove( const MouseEvent& rMEvt )
{
    if (pEditView)
        pEditView->MouseMove( rMEvt );
}

void __EXPORT ScTextWnd::MouseButtonDown( const MouseEvent& rMEvt )
{
    if (!HasFocus())
    {
        StartEditEngine();
        if ( SC_MOD()->IsEditMode() )
            GrabFocus();
    }

    if (pEditView)
    {
        pEditView->SetEditEngineUpdateMode( TRUE );
        pEditView->MouseButtonDown( rMEvt );
    }
}

void __EXPORT ScTextWnd::MouseButtonUp( const MouseEvent& rMEvt )
{
    if (pEditView)
        if (pEditView->MouseButtonUp( rMEvt ))
            SC_MOD()->InputSelection( pEditView );
}

void __EXPORT ScTextWnd::Command( const CommandEvent& rCEvt )
{
    USHORT nCommand = rCEvt.GetCommand();
    if ( pEditView /* && ( nCommand == COMMAND_STARTDRAG || nCommand == COMMAND_VOICE ) */ )
    {
        ScModule* pScMod = SC_MOD();
        ScTabViewShell* pStartViewSh = ScTabViewShell::GetActiveViewShell();

        if ( nCommand == COMMAND_STARTEXTTEXTINPUT )
        {
            SfxItemSet* pSet = new SfxItemSet( pEditView->GetEditEngine()->GetEmptyItemSet() );
            Font aFont = GetSettings().GetStyleSettings().GetAppFont();
            pSet->Put( SvxFontItem( aFont.GetFamily(), aFont.GetName(), EMPTY_STRING,
                                    aFont.GetPitch(), aFont.GetCharSet(), EE_CHAR_FONTINFO ) );
            ((ScEditEngineDefaulter*)pEditView->GetEditEngine())->SetDefaults( pSet );
        }

        // #63263# verhindern, dass die EditView beim View-Umschalten wegkommt
        pScMod->SetInEditCommand( TRUE );
        pEditView->Command( rCEvt );
        pScMod->SetInEditCommand( FALSE );

        //  #48929# COMMAND_STARTDRAG heiss noch lange nicht, dass der Inhalt geaendert wurde
        //  darum in dem Fall kein InputChanged
        //! erkennen, ob mit Move gedraggt wurde, oder Drag&Move irgendwie verbieten

        if ( nCommand == COMMAND_STARTDRAG )
        {
            //  ist auf eine andere View gedraggt worden?
            ScTabViewShell* pEndViewSh = ScTabViewShell::GetActiveViewShell();
            if ( pEndViewSh != pStartViewSh && pStartViewSh != NULL )
            {
                ScViewData* pViewData = pStartViewSh->GetViewData();
                ScInputHandler* pHdl = pScMod->GetInputHdl( pStartViewSh );
                if ( pHdl && pViewData->HasEditView( pViewData->GetActivePart() ) )
                {
                    pHdl->CancelHandler();
                    pViewData->GetView()->ShowCursor();     // fehlt bei KillEditView, weil nicht aktiv
                }
            }
        }
        else
            SC_MOD()->InputChanged( pEditView );
    }
    else
        Window::Command(rCEvt);     //  sonst soll sich die Basisklasse drum kuemmern...
}

void __EXPORT ScTextWnd::KeyInput(const KeyEvent& rKEvt)
{
    if (!SC_MOD()->InputKeyEvent( rKEvt ))
    {
        BOOL bUsed = FALSE;
        ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
        if ( pViewSh )
            bUsed = pViewSh->SfxKeyInput(rKEvt);    // nur Acceleratoren, keine Eingabe
        if (!bUsed)
            Window::KeyInput( rKEvt );
    }
}

void __EXPORT ScTextWnd::GetFocus()
{
}

void __EXPORT ScTextWnd::LoseFocus()
{
}

String __EXPORT ScTextWnd::GetText() const
{
    //  ueberladen, um per Testtool an den Text heranzukommen

    if ( pEditEngine )
        return pEditEngine->GetText();
    else
        return GetTextString();
}

void ScTextWnd::SetFormulaMode( BOOL bSet )
{
    if ( bSet != bFormulaMode )
    {
        bFormulaMode = bSet;
        UpdateAutoCorrFlag();
    }
}

void ScTextWnd::UpdateAutoCorrFlag()
{
    if ( pEditEngine )
    {
        ULONG nControl = pEditEngine->GetControlWord();
        ULONG nOld = nControl;
        if ( bFormulaMode )
            nControl &= ~EE_CNTRL_AUTOCORRECT;      // keine Autokorrektur in Formeln
        else
            nControl |= EE_CNTRL_AUTOCORRECT;       // sonst schon
        if ( nControl != nOld )
            pEditEngine->SetControlWord( nControl );
    }
}

void ScTextWnd::StartEditEngine()
{
    //  #31147# Bei "eigener Modalitaet" (Doc-modale Dialoge) nicht aktivieren
    SfxObjectShell* pObjSh = SfxObjectShell::Current();
    if ( pObjSh && pObjSh->IsInModalMode() )
        return;

    if ( !pEditView || !pEditEngine )
    {
        ScFieldEditEngine* pNew;
        ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
        if ( pViewSh )
        {
            const ScDocument* pDoc = pViewSh->GetViewData()->GetDocument();
            pNew = new ScFieldEditEngine( pDoc->GetEnginePool(), pDoc->GetEditPool() );
        }
        else
            pNew = new ScFieldEditEngine( EditEngine::CreatePool(), NULL, TRUE );
        pNew->SetExecuteURL( FALSE );
        pEditEngine = pNew;

        pEditEngine->SetUpdateMode( FALSE );
        pEditEngine->SetPaperSize( Size( THESIZE, 300 ) );          //!!! size
        pEditEngine->SetWordDelimiters(
                        ScEditUtil::ModifyDelimiters( pEditEngine->GetWordDelimiters() ) );

        UpdateAutoCorrFlag();

        {
            SfxItemSet* pSet = new SfxItemSet( pEditEngine->GetEmptyItemSet() );
            pEditEngine->SetFontInfoInItemSet( *pSet, aTextFont );
            pEditEngine->SetDefaults( pSet );
        }

        //  #57254# Wenn in der Zelle URL-Felder enthalten sind, muessen die auch in
        //  die Eingabezeile uebernommen werden, weil sonst die Positionen nicht stimmen.

        BOOL bFilled = FALSE;
        ScInputHandler* pHdl = SC_MOD()->GetInputHdl();
        if ( pHdl )         //! Testen, ob's der richtige InputHdl ist?
            bFilled = pHdl->GetTextAndFields( *pEditEngine );

        pEditEngine->SetUpdateMode( TRUE );

        //  aString ist die Wahrheit...
        if ( bFilled && pEditEngine->GetText() == aString )
            Invalidate();                       // Repaint fuer (hinterlegte) Felder
        else
            pEditEngine->SetText(aString);      // dann wenigstens den richtigen Text

        pEditView = new EditView( pEditEngine, this );
        pEditView->SetInsertMode(bIsInsertMode);

        // Text aus Clipboard wird als ASCII einzeilig uebernommen
        ULONG n = pEditView->GetControlWord();
        pEditView->SetControlWord( n | EV_CNTRL_SINGLELINEPASTE );

        pEditEngine->InsertView( pEditView, EE_APPEND );

        Resize();
    }

    SC_MOD()->SetInputMode( SC_INPUT_TOP );

    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    if (pViewFrm)
        pViewFrm->GetBindings().Invalidate( SID_ATTR_INSERT );
}

void ScTextWnd::StopEditEngine()
{
    if (pEditView)
    {
        ScModule* pScMod = SC_MOD();

        pScMod->InputSelection( pEditView );
        aString = pEditEngine->GetText();
        bIsInsertMode = pEditView->IsInsertMode();
        BOOL bSelection = pEditView->HasSelection();
        DELETEZ(pEditView);
        DELETEZ(pEditEngine);

        if (pScMod->IsEditMode())
            pScMod->SetInputMode(SC_INPUT_TABLE);

        SfxViewFrame* pViewFrm = SfxViewFrame::Current();
        if (pViewFrm)
            pViewFrm->GetBindings().Invalidate( SID_ATTR_INSERT );

        if (bSelection)
            Invalidate();           // damit Selektion nicht stehenbleibt
    }
}

void ScTextWnd::SetTextString( const String& rNewString )
{
    if ( rNewString != aString )
    {
        //  Position der Aenderung suchen, nur Rest painten

        long nInvPos = 0;
        long nStartPos = 0;
        long nYPos = 0;
        long nTextSize = 0;

        if (!pEditEngine)
        {
            xub_StrLen nDifPos;
            if (rNewString.Len() > aString.Len())
                nDifPos = rNewString.Match(aString);
            else
                nDifPos = aString.Match(rNewString);

            long nSize1 = GetTextWidth(aString);
            long nSize2 = GetTextWidth(rNewString);
            if ( nSize1>0 && nSize2>0 )
                nTextSize = Max( nSize1, nSize2 );
            else
                nTextSize = GetOutputSize().Width();        // Ueberlauf

            if (nDifPos == STRING_MATCH)
                nDifPos = 0;

                                            // -1 wegen Rundung und "A"
            Point aLogicStart = PixelToLogic(Point(TEXT_STARTPOS-1,0));
            nStartPos = aLogicStart.X();
            nInvPos = nStartPos;
            if (nDifPos)
                nInvPos += GetTextWidth(aString,0,nDifPos);

            USHORT nFlags = 0;
            if ( nDifPos == aString.Len() )         // only new characters appended
                nFlags = INVALIDATE_NOERASE;        // then background is already clear

            Invalidate( Rectangle( nInvPos, 0,
                                    nStartPos+nTextSize, GetOutputSize().Height()-1 ),
                        nFlags );
        }
        else
        {
            pEditEngine->SetText(rNewString);
        }

        aString = rNewString;
    }
}

const String& ScTextWnd::GetTextString() const
{
    return aString;
}

BOOL ScTextWnd::IsActive()
{
    return HasFocus();
}

EditView* ScTextWnd::GetEditView()
{
    return pEditView;
}

void ScTextWnd::MakeDialogEditView()
{
    if ( pEditView ) return;

    ScFieldEditEngine* pNew;
    ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
    if ( pViewSh )
    {
        const ScDocument* pDoc = pViewSh->GetViewData()->GetDocument();
        pNew = new ScFieldEditEngine( pDoc->GetEnginePool(), pDoc->GetEditPool() );
    }
    else
        pNew = new ScFieldEditEngine( EditEngine::CreatePool(), NULL, TRUE );
    pNew->SetExecuteURL( FALSE );
    pEditEngine = pNew;

    pEditEngine->SetUpdateMode( FALSE );
    pEditEngine->SetWordDelimiters( pEditEngine->GetWordDelimiters() += '=' );
    pEditEngine->SetPaperSize( Size( THESIZE, 300 ) );

    SfxItemSet* pSet = new SfxItemSet( pEditEngine->GetEmptyItemSet() );
    pEditEngine->SetFontInfoInItemSet( *pSet, aTextFont );
    pEditEngine->SetDefaults( pSet );
    pEditEngine->SetUpdateMode( TRUE );

    pEditView   = new EditView( pEditEngine, this );
    pEditEngine->InsertView( pEditView, EE_APPEND );

    Resize();
}

void ScTextWnd::ImplInitSettings()
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

    Color aBgColor= rStyleSettings.GetWindowColor();
    Color aTxtColor= rStyleSettings.GetWindowTextColor();

    aTextFont.SetFillColor   ( aBgColor );
    aTextFont.SetColor       (aTxtColor);
    SetBackground           ( aBgColor );
    Invalidate();
}
// -----------------------------------------------------------------------

void ScTextWnd::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        ImplInitSettings();
        Invalidate();
    }
    else
        Window::DataChanged( rDCEvt );
}


//========================================================================
//                          Positionsfenster
//========================================================================

ScPosWnd::ScPosWnd( Window* pParent ) :
    ComboBox    ( pParent, WinBits(WB_HIDE | WB_DROPDOWN) ),
    pAccel      ( NULL ),
    bFormulaMode( FALSE )
{
    Size aSize( GetTextWidth( String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("GW99999:GW99999")) ),
                GetTextHeight() );
    aSize.Width() += 25;    // ??
    aSize.Height() = CalcWindowSizePixel(11);       // Funktionen: 10 MRU + "andere..."
    SetSizePixel( aSize );

    FillRangeNames();

    StartListening( *SFX_APP() );       // fuer Navigator-Bereichsnamen-Updates
}

__EXPORT ScPosWnd::~ScPosWnd()
{
    EndListening( *SFX_APP() );

    delete pAccel;
}

void ScPosWnd::SetFormulaMode( BOOL bSet )
{
    if ( bSet != bFormulaMode )
    {
        bFormulaMode = bSet;

        if ( bSet )
            FillFunctions();
        else
            FillRangeNames();
    }
}

void ScPosWnd::SetPos( const String& rPosStr )
{
    if ( aPosStr != rPosStr )
    {
        aPosStr = rPosStr;
        SetText(aPosStr);
    }
}

void ScPosWnd::FillRangeNames()
{
    Clear();

    SfxObjectShell* pObjSh = SfxObjectShell::Current();
    if ( pObjSh && pObjSh->ISA(ScDocShell) )
    {
        ScDocument* pDoc = ((ScDocShell*)pObjSh)->GetDocument();

        //  per Hand sortieren, weil Funktionen nicht sortiert werden:

        ScRangeName* pRangeNames = pDoc->GetRangeName();
        USHORT nCount = pRangeNames->GetCount();
        if ( nCount > 0 )
        {
            USHORT nValidCount = 0;
            ScRange aDummy;
            USHORT i;
            for ( i=0; i<nCount; i++ )
            {
                ScRangeData* pData = (*pRangeNames)[i];
                if (pData->IsReference(aDummy))
                    nValidCount++;
            }
            if ( nValidCount )
            {
                ScRangeData** ppSortArray = new ScRangeData* [ nValidCount ];
                USHORT j;
                for ( i=0, j=0; i<nCount; i++ )
                {
                    ScRangeData* pData = (*pRangeNames)[i];
                    if (pData->IsReference(aDummy))
                        ppSortArray[j++] = pData;
                }
#ifndef ICC
                qsort( (void*)ppSortArray, nValidCount, sizeof(ScRangeData*),
                    &ScRangeData::QsortNameCompare );
#else
                qsort( (void*)ppSortArray, nValidCount, sizeof(ScRangeData*),
                    ICCQsortNameCompare );
#endif
                for ( j=0; j<nValidCount; j++ )
                    InsertEntry( ppSortArray[j]->GetName() );
                delete [] ppSortArray;
            }
        }
    }
    SetText(aPosStr);
}

void ScPosWnd::FillFunctions()
{
    Clear();

    String aFirstName;
    const ScAppOptions& rOpt = SC_MOD()->GetAppOptions();
    USHORT nMRUCount = rOpt.GetLRUFuncListCount();
    const USHORT* pMRUList = rOpt.GetLRUFuncList();
    if (pMRUList)
    {
        const ScFunctionList* pFuncList = ScGlobal::GetStarCalcFunctionList();
        ULONG nListCount = pFuncList->GetCount();
        ScFunctionMgr* pFuncMgr = ScGlobal::GetStarCalcFunctionMgr();
        for (USHORT i=0; i<nMRUCount; i++)
        {
            USHORT nId = pMRUList[i];
            for (ULONG j=0; j<nListCount; j++)
            {
                const ScFuncDesc* pDesc = pFuncList->GetFunction( j );
                if ( pDesc->nFIndex == nId && pDesc->pFuncName )
                {
                    InsertEntry( *pDesc->pFuncName );
                    if (!aFirstName.Len())
                        aFirstName = *pDesc->pFuncName;
                    break;  // nicht weitersuchen
                }
            }
        }
    }

    //! Eintrag "Andere..." fuer Funktions-Autopilot wieder aufnehmen,
    //! wenn der Funktions-Autopilot mit dem bisher eingegebenen Text arbeiten kann!

//  InsertEntry( ScGlobal::GetRscString(STR_FUNCTIONLIST_MORE) );

    SetText(aFirstName);
}

void __EXPORT ScPosWnd::SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                                      const SfxHint& rHint, const TypeId& rHintType )
{
    if ( !bFormulaMode )
    {
        //  muss die Liste der Bereichsnamen updgedated werden?

        if ( rHint.ISA(SfxSimpleHint) )
        {
            ULONG nHintId = ((SfxSimpleHint&)rHint).GetId();
            if ( nHintId == SC_HINT_AREAS_CHANGED || nHintId == SC_HINT_NAVIGATOR_UPDATEALL)
                FillRangeNames();
        }
        else if ( rHint.ISA(SfxEventHint) )
        {
            ULONG nEventId = ((SfxEventHint&)rHint).GetEventId();
            if ( nEventId == SFX_EVENT_ACTIVATEDOC )
                FillRangeNames();
        }
    }
}

void __EXPORT ScPosWnd::Select()
{
    ComboBox::Select();     //  in VCL gibt GetText() erst danach den ausgewaehlten Eintrag

    if (!IsTravelSelect())
        DoEnter();
}

void ScPosWnd::DoEnter()
{
    String aText = GetText();
    if ( aText.Len() )
    {
        if ( bFormulaMode )
        {
            ScModule* pScMod = SC_MOD();
            if ( aText == ScGlobal::GetRscString(STR_FUNCTIONLIST_MORE) )
            {
                //  Funktions-Autopilot
                //! mit dem bisher eingegebenen Text weiterarbeiten !!!

                //! new method at ScModule to query if function autopilot is open
                SfxViewFrame* pViewFrm = SfxViewFrame::Current();
                if ( pViewFrm && !pViewFrm->GetChildWindow( SID_OPENDLG_FUNCTION ) )
                    pViewFrm->GetDispatcher()->Execute( SID_OPENDLG_FUNCTION,
                                              SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD );
            }
            else
            {
                ScTabViewShell* pViewSh = PTR_CAST( ScTabViewShell, SfxViewShell::Current() );
                ScInputHandler* pHdl = pScMod->GetInputHdl( pViewSh );
                if (pHdl)
                    pHdl->InsertFunction( aText );
            }
        }
        else
        {
            //  Position (Zelle oder Namen) setzen
            SfxStringItem aPosItem( SID_CURRENTCELL, aText );
            SfxBoolItem aUnmarkItem( FN_PARAM_1, TRUE );        // Selektion aufheben
            SfxViewFrame* pViewFrm = SfxViewFrame::Current();
            if ( pViewFrm )
                pViewFrm->GetDispatcher()->Execute( SID_CURRENTCELL,
                                      SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD,
                                      &aPosItem, &aUnmarkItem, 0L );

            //! Fehler erkennen und meckern
        }
    }
    else
        SetText( aPosStr );

    ReleaseFocus_Impl();
}

#ifdef VCL

long __EXPORT ScPosWnd::Notify( NotifyEvent& rNEvt )
{
    //  VCL geht ohne Acceleratoren - AccelSelectHdl kann demnaechst raus

    ComboBox::Notify( rNEvt );
    long nHandled = 0;

    if ( rNEvt.GetType() == EVENT_KEYUP )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();

        switch ( pKEvt->GetKeyCode().GetCode() )
        {
            case KEY_RETURN:
                DoEnter();
                nHandled = 1;
                break;

            case KEY_ESCAPE:
                if (!bFormulaMode)
                    SetText( aPosStr );
                ReleaseFocus_Impl();
                nHandled = 1;
                break;
        }
    }
    return nHandled;
}

#else

void __EXPORT ScPosWnd::GetFocus()
{
    DBG_ASSERT( !pAccel, "Accelerator not deleted!" );
    pAccel = new Accelerator;
    pAccel->InsertItem( 1, KeyCode( KEY_RETURN ) );
    pAccel->InsertItem( 2, KeyCode( KEY_ESCAPE ) );
    pAccel->PushSelectHdl( LINK( this, ScPosWnd, AccelSelectHdl ) );

    Application::InsertAccel( pAccel, ACCEL_ALWAYS );
}

void __EXPORT ScPosWnd::LoseFocus()
{
    DBG_ASSERT( pAccel, "Accelerator not found!" );
    Application::RemoveAccel( pAccel );
    delete pAccel;
    pAccel = NULL;
}

#endif

IMPL_LINK( ScPosWnd, AccelSelectHdl, Accelerator *, pAccel )
{
    if ( !pAccel )
        return 0;

    switch ( pAccel->GetCurKeyCode().GetCode() )
    {
        case KEY_RETURN:
            DoEnter();
            break;

        case KEY_ESCAPE:
            if (!bFormulaMode)
                SetText( aPosStr );
            ReleaseFocus_Impl();
            break;

        default:
        break;
    }
    return TRUE;
}

void ScPosWnd::ReleaseFocus_Impl()
{
    SfxViewShell* pCurSh = SfxViewShell::Current();
    ScInputHandler* pHdl = SC_MOD()->GetInputHdl( PTR_CAST( ScTabViewShell, pCurSh ) );
    if ( pHdl && pHdl->IsTopMode() )
    {
        //  Focus wieder in die Eingabezeile?

        ScInputWindow* pInputWin = pHdl->GetInputWindow();
        if (pInputWin)
        {
            pInputWin->TextGrabFocus();
            return;
        }
    }

    //  Focus auf die aktive View

    if ( pCurSh )
    {
        Window* pShellWnd = pCurSh->GetWindow();

        if ( pShellWnd )
            pShellWnd->GrabFocus();
    }
}






