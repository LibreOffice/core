/*************************************************************************
 *
 *  $RCSfile: baside3.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: tbe $ $Date: 2001-09-06 09:17:41 $
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

//svdraw.hxx
//#define _SVDRAW_HXX ***
#define _SDR_NOITEMS
#define _SDR_NOTOUCH
#define _SDR_NOTRANSFORM
#define _SDR_NOOBJECTS
//#define _SDR_NOVIEWS ***
#define _SDR_NOVIEWMARKER
#define _SDR_NODRAGMETHODS
//#define _SDR_NOUNDO ***
#define _SDR_NOXOUTDEV

#include <ide_pch.hxx>

#pragma hdrstop

#include <basidesh.hrc>
#include <baside3.hxx>

#include <dlged.hxx>

#ifndef _BASCTL_PROPBRW_HXX
#include <propbrw.hxx>
#endif

#ifndef _BASCTL_DLGEDDEF_HXX
#include <dlgeddef.hxx>
#endif

#include <basobj.hxx>
#include <iderdll.hxx>
#include <basidesh.hxx>
#include <idetemp.hxx>
#include <helpid.hrc>

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _XMLSCRIPT_XMLDLG_IMEXP_HXX_
#include <xmlscript/xmldlg_imexp.hxx>
#endif

using namespace comphelper;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;


TYPEINIT1( DialogWindow, IDEBaseWindow );

DialogWindow::DialogWindow( Window* pParent, StarBASIC* pBasic,
    SfxObjectShell* pShell, String aLibName, String aDlgName,
    const com::sun::star::uno::Reference< com::sun::star::container::XNameContainer >& xDialogModel )
        :IDEBaseWindow( pParent, pBasic )
        ,pUndoMgr(NULL)
        ,m_pShell( pShell )
        ,m_aLibName( aLibName )
        ,m_aDlgName( aDlgName )
{
    InitSettings( TRUE, TRUE, TRUE );

    pEditor = new DlgEditor();
    pEditor->SetWindow( this );
    pEditor->SetDialog( xDialogModel );

    // Undo einrichten
    pUndoMgr = new SfxUndoManager;

    Link aDummyLink;
    aOldNotifyUndoActionHdl = pEditor->GetModel()->GetNotifyUndoActionHdl();
    pEditor->GetModel()->SetNotifyUndoActionHdl(
        LINK(this, DialogWindow, NotifyUndoActionHdl));

    SetHelpId( HID_BASICIDE_DIALOGWINDOW );
}

DialogWindow::DialogWindow( DialogWindow* pOrgWin ) :
        IDEBaseWindow( pOrgWin->GetParent(), pOrgWin->GetBasic() )
{
    DBG_ERROR( "Dieser CTOR ist nicht erlaubt!" );
}



DialogWindow::~DialogWindow()
{
    delete pEditor;
    delete pUndoMgr;
}

void DialogWindow::LoseFocus()
{
    if( pEditor->IsModified() )
    {
        StoreData();
        //BasicIDE::MarkDocShellModified( m_pShell );   // also done in StoreData
        //pEditor->ClearModifyFlag();                   // also done in StoreData
    }
    Window::LoseFocus();
}



void DialogWindow::Paint( const Rectangle& rRect )
{
    pEditor->Paint( rRect );
}



void DialogWindow::Resize()
{
    if ( GetHScrollBar() && GetVScrollBar() ) {
        pEditor->SetScrollBars( GetHScrollBar(), GetVScrollBar() );
    }
}



void DialogWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    pEditor->MouseButtonDown( rMEvt );
}



void DialogWindow::MouseButtonUp( const MouseEvent& rMEvt )
{
    pEditor->MouseButtonUp( rMEvt );
    SfxBindings& rBindings = BasicIDE::GetBindings();
    if( (pEditor->GetMode() == DLGED_INSERT) && !pEditor->IsCreateOK() )
    {
        pEditor->SetMode( DLGED_SELECT );
        rBindings.Invalidate( SID_CHOOSE_CONTROLS );
    }
    rBindings.Invalidate( SID_DOC_MODIFIED );
    rBindings.Invalidate( SID_SAVEDOC );
}



void DialogWindow::MouseMove( const MouseEvent& rMEvt )
{
    pEditor->MouseMove( rMEvt );
}



void DialogWindow::KeyInput( const KeyEvent& rKEvt )
{
    if( rKEvt.GetKeyCode() == KEY_BACKSPACE )
    {
        BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
        SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
        SfxDispatcher* pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;
        if( pDispatcher )
        {
            pDispatcher->Execute( SID_BACKSPACE );
        }
    }
    else
    {
        if( !pEditor->KeyInput( rKEvt ) )
        {
            if( !SfxViewShell::Current()->KeyInput( rKEvt ) )
                Window::KeyInput( rKEvt );
        }
    }
}

void DialogWindow::Command( const CommandEvent& rCEvt )
{
    if ( ( rCEvt.GetCommand() == COMMAND_WHEEL ) ||
            ( rCEvt.GetCommand() == COMMAND_STARTAUTOSCROLL ) ||
            ( rCEvt.GetCommand() == COMMAND_AUTOSCROLL ) )
    {
        HandleScrollCommand( rCEvt, GetHScrollBar(), GetVScrollBar() );
    }
    else
        IDEBaseWindow::Command( rCEvt );
}




IMPL_LINK( DialogWindow, NotifyUndoActionHdl, SfxUndoAction *, pUndoAction )
{
    // not working yet for unocontrols
    /*
    if (pUndoAction)
    {
        pUndoMgr->AddUndoAction( pUndoAction );
        BasicIDE::GetBindings().Invalidate( SID_UNDO );
    }
    */

    return 0;
}



void __EXPORT DialogWindow::DoInit()
{
    GetHScrollBar()->Show();
    GetVScrollBar()->Show();
    pEditor->SetScrollBars( GetHScrollBar(), GetVScrollBar() );
}



void __EXPORT DialogWindow::DoScroll( ScrollBar* pCurScrollBar )
{
    pEditor->DoScroll( pCurScrollBar );
}

void __EXPORT DialogWindow::GetState( SfxItemSet& rSet )
{
    SfxWhichIter aIter(rSet);
    for ( USHORT nWh = aIter.FirstWhich(); 0 != nWh; nWh = aIter.NextWhich() )
    {
        switch ( nWh )
        {
            case SID_PASTE:
            break;
            case SID_CUT:
            case SID_COPY:
            case SID_DELETE:
            case SID_BACKSPACE:
            {
                // Object selektiert?
            }
            break;
            case SID_REDO:
            {
                if ( !pUndoMgr->GetUndoActionCount() )
                    rSet.DisableItem( nWh );
            }
            break;

            // Nur Dialogfenster:
            case SID_DIALOG_TESTMODE:
            {
                // ist die IDE noch aktiv?
                if( IDE_DLL()->GetShell()->GetFrame() )
                {
                    rSet.Put( SfxBoolItem( SID_DIALOG_TESTMODE,
                              (pEditor->GetMode() == DLGED_TEST) ? TRUE : FALSE) );
                }
                else
                    rSet.Put( SfxBoolItem( SID_DIALOG_TESTMODE,FALSE ));
            }
            break;

            case SID_CHOOSE_CONTROLS:
            {
                SfxAllEnumItem aItem( SID_CHOOSE_CONTROLS );
                if( GetEditor()->GetMode() == DLGED_SELECT )
                    aItem.SetValue( SVX_SNAP_SELECT );
                else
                {
                    USHORT nObj;
                    switch( pEditor->GetInsertObj() )
                    {
                        case OBJ_DLG_PUSHBUTTON:    nObj = SVX_SNAP_PUSHBUTTON; break;
                        case OBJ_DLG_RADIOBUTTON:   nObj = SVX_SNAP_RADIOBUTTON; break;
                        case OBJ_DLG_CHECKBOX:      nObj = SVX_SNAP_CHECKBOX; break;
                        case OBJ_DLG_LISTBOX:       nObj = SVX_SNAP_LISTBOX; break;
                        case OBJ_DLG_COMBOBOX:      nObj = SVX_SNAP_COMBOBOX; break;
                        case OBJ_DLG_GROUPBOX:      nObj = SVX_SNAP_GROUPBOX; break;
                        case OBJ_DLG_EDIT:          nObj = SVX_SNAP_EDIT; break;
                        case OBJ_DLG_FIXEDTEXT:     nObj = SVX_SNAP_FIXEDTEXT; break;
                        case OBJ_DLG_IMAGECONTROL:  nObj = SVX_SNAP_IMAGECONTROL; break;
                        case OBJ_DLG_PROGRESSBAR:   nObj = SVX_SNAP_PROGRESSBAR; break;
                        case OBJ_DLG_HSCROLLBAR:    nObj = SVX_SNAP_HSCROLLBAR; break;
                        case OBJ_DLG_VSCROLLBAR:    nObj = SVX_SNAP_VSCROLLBAR; break;
                        case OBJ_DLG_HFIXEDLINE:    nObj = SVX_SNAP_HFIXEDLINE; break;
                        case OBJ_DLG_VFIXEDLINE:    nObj = SVX_SNAP_VFIXEDLINE; break;
                        default:                    nObj = 0;
                    }
#ifdef DBG_UTIL
                    if( !nObj )
                    {
                        DBG_WARNING( "SID_CHOOSE_CONTROLS: Unbekannt!" );
                    }
#endif
                    aItem.SetValue( nObj );
                }

                rSet.Put( aItem );
            }
            break;
        }
    }
}



void __EXPORT DialogWindow::ExecuteCommand( SfxRequest& rReq )
{
    switch ( rReq.GetSlot() )
    {
        case SID_CUT:
            GetEditor()->Cut();
            BasicIDE::GetBindings().Invalidate( SID_DOC_MODIFIED );
            break;
        case SID_DELETE:
#ifdef MAC
        case SID_BACKSPACE:
#endif
            GetEditor()->Delete();
            BasicIDE::GetBindings().Invalidate( SID_DOC_MODIFIED );
            break;
        case SID_COPY:
            GetEditor()->Copy();
            break;
        case SID_PASTE:
            GetEditor()->Paste();
            BasicIDE::GetBindings().Invalidate( SID_DOC_MODIFIED );
            break;
        case SID_CHOOSE_CONTROLS:
        {
            const SfxItemSet* pArgs = rReq.GetArgs();
            DBG_ASSERT( pArgs, "Nix Args" );

            const SfxAllEnumItem& rItem = (SfxAllEnumItem&)pArgs->Get( SID_CHOOSE_CONTROLS );
            switch( rItem.GetValue() )
            {
                case SVX_SNAP_PUSHBUTTON:
                {
                    GetEditor()->SetMode( DLGED_INSERT );
                    GetEditor()->SetInsertObj( OBJ_DLG_PUSHBUTTON );
                }
                break;
                case SVX_SNAP_RADIOBUTTON:
                {
                    GetEditor()->SetMode( DLGED_INSERT );
                    GetEditor()->SetInsertObj( OBJ_DLG_RADIOBUTTON );
                }
                break;
                case SVX_SNAP_CHECKBOX:
                {
                    GetEditor()->SetMode( DLGED_INSERT );
                    GetEditor()->SetInsertObj( OBJ_DLG_CHECKBOX);
                }
                break;
                case SVX_SNAP_LISTBOX:
                {
                    GetEditor()->SetMode( DLGED_INSERT );
                    GetEditor()->SetInsertObj( OBJ_DLG_LISTBOX );
                }
                break;
                case SVX_SNAP_COMBOBOX:
                {
                    GetEditor()->SetMode( DLGED_INSERT );
                    GetEditor()->SetInsertObj( OBJ_DLG_COMBOBOX );
                }
                break;
                case SVX_SNAP_GROUPBOX:
                {
                    GetEditor()->SetMode( DLGED_INSERT );
                    GetEditor()->SetInsertObj( OBJ_DLG_GROUPBOX );
                }
                break;
                case SVX_SNAP_EDIT:
                {
                    GetEditor()->SetMode( DLGED_INSERT );
                    GetEditor()->SetInsertObj( OBJ_DLG_EDIT );
                }
                break;
                case SVX_SNAP_FIXEDTEXT:
                {
                    GetEditor()->SetMode( DLGED_INSERT );
                    GetEditor()->SetInsertObj( OBJ_DLG_FIXEDTEXT );
                }
                break;
                case SVX_SNAP_IMAGECONTROL:
                {
                    GetEditor()->SetMode( DLGED_INSERT );
                    GetEditor()->SetInsertObj( OBJ_DLG_IMAGECONTROL );
                }
                break;
                case SVX_SNAP_PROGRESSBAR:
                {
                    GetEditor()->SetMode( DLGED_INSERT );
                    GetEditor()->SetInsertObj( OBJ_DLG_PROGRESSBAR );
                }
                break;
                case SVX_SNAP_HSCROLLBAR:
                {
                    GetEditor()->SetMode( DLGED_INSERT );
                    GetEditor()->SetInsertObj( OBJ_DLG_HSCROLLBAR );
                }
                break;
                case SVX_SNAP_VSCROLLBAR:
                {
                    GetEditor()->SetMode( DLGED_INSERT );
                    GetEditor()->SetInsertObj( OBJ_DLG_VSCROLLBAR );
                }
                break;
                case SVX_SNAP_HFIXEDLINE:
                {
                    GetEditor()->SetMode( DLGED_INSERT );
                    GetEditor()->SetInsertObj( OBJ_DLG_HFIXEDLINE );
                }
                break;
                case SVX_SNAP_VFIXEDLINE:
                {
                    GetEditor()->SetMode( DLGED_INSERT );
                    GetEditor()->SetInsertObj( OBJ_DLG_VFIXEDLINE );
                }
                break;
                case SVX_SNAP_SELECT:
                {
                    GetEditor()->SetMode( DLGED_SELECT );
                }
                break;
            }
            BasicIDE::GetBindings().Invalidate( SID_DOC_MODIFIED );
            #ifdef DEBUG
                BOOL bModified = IsModified();
            #endif

        }
        break;

        case SID_DIALOG_TESTMODE:
        {
            DlgEdMode eOldMode = GetEditor()->GetMode();
            GetEditor()->SetMode( DLGED_TEST );
            GetEditor()->SetMode( eOldMode );
            rReq.Done();
            BasicIDE::GetBindings().Invalidate( SID_DIALOG_TESTMODE );
            return;
        }
        break;
    }

    rReq.Done();
}

Reference< container::XNameContainer > DialogWindow::GetDialog() const
{
    return pEditor->GetDialog();
}

BOOL DialogWindow::RenameDialog( const String& rNewName )
{
    BOOL bDone = TRUE;

    try
    {
        BasicIDE::RenameDialog( m_pShell, m_aLibName, m_aDlgName, rNewName );
        BasicIDE::GetBindings().Invalidate( SID_DOC_MODIFIED );
    }
    catch ( container::ElementExistException& )
    {
        ErrorBox( this, WB_OK | WB_DEF_OK, String( IDEResId( RID_STR_SBXNAMEALLREADYUSED2 ) ) ).Execute();
        bDone = FALSE;
    }
    catch ( container::NoSuchElementException& e )
    {
        ByteString aBStr( String(e.Message), RTL_TEXTENCODING_ASCII_US );
        DBG_ERROR( aBStr.GetBuffer() );
        bDone = FALSE;
    }

    return bDone;
}

void DialogWindow::DisableBrowser()
{
    BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
    SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
    SfxChildWindow* pChildWin = pViewFrame ? pViewFrame->GetChildWindow(SID_SHOW_BROWSER) : NULL;
    if( pChildWin )
        ((PropBrw*)(pChildWin->GetWindow()))->Update( 0 );
}

void DialogWindow::UpdateBrowser()
{
    BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
    SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
    SfxChildWindow* pChildWin = pViewFrame ? pViewFrame->GetChildWindow(SID_SHOW_BROWSER) : NULL;
    if( pChildWin )
        ((PropBrw*)(pChildWin->GetWindow()))->Update(GetEditor()->GetView());
}

SdrView* DialogWindow::GetView() const
{
    return GetEditor()->GetView();
}

BOOL __EXPORT DialogWindow::IsModified()
{
    return pEditor->IsModified();
}

SfxUndoManager* __EXPORT DialogWindow::GetUndoManager()
{
    return pUndoMgr;
}

String DialogWindow::GetTitle()
{
    return GetDialogName();
}

void DialogWindow::StoreData()
{
    if( pEditor->IsModified() )
    {
        try
        {
            Reference< container::XNameContainer > xLib = BasicIDE::GetDialogLibrary( m_pShell, m_aLibName, TRUE );

            if( xLib.is() )
            {
                Reference< container::XNameContainer > xDialogModel = pEditor->GetDialog();

                if( xDialogModel.is() )
                {
                    Reference< XInputStreamProvider > xISP = ::xmlscript::exportDialogModel( xDialogModel );
                    Any aAny;
                    aAny <<= xISP;
                    xLib->replaceByName( ::rtl::OUString( m_aDlgName ), aAny );

                    BasicIDE::MarkDocShellModified( m_pShell );
                    pEditor->ClearModifyFlag();
                }
            }
        }
        catch ( container::NoSuchElementException& e )
        {
            ByteString aBStr( String(e.Message), RTL_TEXTENCODING_ASCII_US );
            DBG_ERROR( aBStr.GetBuffer() );
        }
    }
}

void DialogWindow::Deactivating()
{
    if( pEditor->IsModified() )
        BasicIDE::MarkDocShellModified( GetBasic() );
}

void DialogWindow::PrintData( Printer* pPrinter )
{
    pEditor->PrintData( pPrinter, CreateQualifiedName() );
}

void DialogWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    if( (rDCEvt.GetType()==DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        InitSettings( TRUE, TRUE, TRUE );
        Invalidate();
    }
    else
        IDEBaseWindow::DataChanged( rDCEvt );
}

void DialogWindow::InitSettings(BOOL bFont,BOOL bForeground,BOOL bBackground)
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    if( bFont )
    {
        Font aFont;
        aFont = rStyleSettings.GetFieldFont();
        SetPointFont( aFont );
    }

    if( bForeground || bFont )
    {
        SetTextColor( rStyleSettings.GetFieldTextColor() );
        SetTextFillColor();
    }

    if( bBackground )
        SetBackground( rStyleSettings.GetFieldColor() );
}
