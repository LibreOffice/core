/*************************************************************************
 *
 *  $RCSfile: baside3.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ab $ $Date: 2001-03-03 15:08:22 $
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

#ifdef _DLGEDITOR_
#include <dlged.hxx>
#else
#include <vced.hxx>
#endif

#ifdef _DLGEDITOR_
#ifndef _BASCTL_PROPBRW_HXX
#include <propbrw.hxx>
#endif
#else
#include <vcbrw.hxx>
#endif

#include <basobj.hxx>
#include <iderdll.hxx>
#include <basidesh.hxx>
#include <idetemp.hxx>
#include <helpid.hrc>

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#include <xmlscript/xmldlg_imexp.hxx>

using namespace comphelper;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;



TYPEINIT1( DialogWindow, IDEBaseWindow );

DialogWindow::DialogWindow( Window* pParent, VCSbxDialogRef aDialog,
                                    StarBASIC* pBas ) :
        IDEBaseWindow( pParent, pBas ),
        pUndoMgr(NULL)
{
    // Bitte kein Show() !
    if( aDialog.Is() )
        pDialog = aDialog;
    else
    {
        pDialog = new VCSbxDialog;
        pDialog->SetName( String( RTL_CONSTASCII_USTRINGPARAM( "Dialog" ) ) );
        GetBasic()->Insert( pDialog );
    }

    InitSettings( TRUE, TRUE, TRUE );

    pEditor = new VCDlgEditor( GetBasic() );
    pEditor->SetWindow( this );
    pEditor->SetVCSbxForm( pDialog );

    // Undo einrichten
    pUndoMgr = new SfxUndoManager;

    Link aDummyLink;
    aOldNotifyUndoActionHdl = pEditor->GetModel()->GetNotifyUndoActionHdl();
    pEditor->GetModel()->SetNotifyUndoActionHdl(
        LINK(this, DialogWindow, NotifyUndoActionHdl));

    SetHelpId( HID_BASICIDE_DIALOGWINDOW );
}

#ifdef _DLGEDITOR_
// This Ctor can only be used in a _DLGEDITOR_ environment
DialogWindow::DialogWindow( Window* pParent, VCSbxDialogRef aDialog, StarBASIC* pBasic,
    const Reference< script::XLibraryContainer >& xLibContainer, String aLibName, String aDlgName )
        : IDEBaseWindow( pParent, pBasic )
        , pUndoMgr(NULL)
        , mxLibContainer( xLibContainer )
        , maLibName( aLibName )
        , maDlgName( aDlgName )
{
    // TODO: Remove!
    // Bitte kein Show() !
    if( aDialog.Is() )
        pDialog = aDialog;
    else
    {
        pDialog = new VCSbxDialog;
        pDialog->SetName( String( RTL_CONSTASCII_USTRINGPARAM( "Dialog" ) ) );
        GetBasic()->Insert( pDialog );
    }


    // It has to be checked before that the library exists
    // and that it supports XNameContainer
    Reference< container::XNameContainer > xLib;
    rtl::OUString aOULibName( maLibName );
    Any aElement = mxLibContainer->getByName( aOULibName );
    aElement >>= xLib;
    if( !mxLibContainer->isLibraryLoaded( aOULibName ) )
        mxLibContainer->loadLibrary( aOULibName );

    // Does the dialog exist?
    Reference< container::XNameContainer > xDialogModel;
    rtl::OUString aOUDlgName( maDlgName );
    if( xLib->hasByName( aOUDlgName ) )
    {
        // Get dialog data
        Any aElement = xLib->getByName( aOUDlgName );
        Sequence< sal_Int8 > aDialogsSeq;
        aElement >>= aDialogsSeq;

        // Create a DialogModel
        Sequence< Reference< container::XNameContainer > > aModelSeq;
        xmlscript::importDialogModelsFromByteSequence( &aModelSeq, aDialogsSeq );
        xDialogModel = aModelSeq.getConstArray()[0];
    }
    else
    {
        // Create new uno dialog
        Reference< lang::XMultiServiceFactory > xMSF = getProcessServiceFactory();
        xDialogModel = Reference< container::XNameContainer >( xMSF->createInstance
            ( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlDialogModel" ) ) ), uno::UNO_QUERY );

        // Also set name as property
        Reference< beans::XPropertySet > xDlgPSet( xDialogModel, UNO_QUERY );
        Any aValue;
        aValue <<= aOUDlgName;
        xDlgPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Name" ) ), aValue );

        Sequence< Reference< container::XNameContainer > > aModelSeq( 1 );
        aModelSeq.getArray()[0] = xDialogModel;
        Sequence< sal_Int8 > aSeq;

        xmlscript::exportDialogModelsToByteSequence( &aSeq, aModelSeq );
        Any aSeqAny;
        aSeqAny <<= aSeq;
        xLib->insertByName( aOUDlgName, aSeqAny );
    }

    InitSettings( TRUE, TRUE, TRUE );

    pEditor = new VCDlgEditor( GetBasic() );
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
#endif

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
        BasicIDE::MarkDocShellModified( pEditor->GetBasic() );
        pEditor->ClearModifyFlag();
        pDialog->SetModified( TRUE );
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
    if( (pEditor->GetMode() == VCDLGED_INSERT) && !pEditor->IsCreateOK() )
    {
        pEditor->SetMode( VCDLGED_SELECT );
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
        SfxViewFrame* pCurFrame = SfxViewFrame::Current();
        DBG_ASSERT( pCurFrame != NULL, "No current view frame!" );
        SfxDispatcher* pDispatcher = pCurFrame ? pCurFrame->GetDispatcher() : NULL;
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
    if (pUndoAction)
    {
        pUndoMgr->AddUndoAction( pUndoAction );
        BasicIDE::GetBindings().Invalidate( SID_UNDO );
    }

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




void DialogWindow::SetDialogName( const String& rName )
{
    pDialog->SetName( rName );
}



String DialogWindow::GetDialogName()
{
    return pDialog->GetName();
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
                              (pEditor->GetMode() == VCDLGED_TEST) ? TRUE : FALSE) );
                }
                else
                    rSet.Put( SfxBoolItem( SID_DIALOG_TESTMODE,FALSE ));
            }
            break;

            case SID_CHOOSE_CONTROLS:
            {
                SfxAllEnumItem aItem( SID_CHOOSE_CONTROLS );
                if( GetEditor()->GetMode() == VCDLGED_SELECT )
                    aItem.SetValue( SVX_SNAP_SELECT );
                else
                {
                    USHORT nObj;
                    switch( pEditor->GetInsertObj() )
                    {
                        case OBJ_DLG_CHECKBOX:      nObj = SVX_SNAP_CHECKBOX; break;
                        case OBJ_DLG_RADIOBUTTON:   nObj = SVX_SNAP_RADIOBUTTON; break;
                        case OBJ_DLG_PUSHBUTTON:    nObj = SVX_SNAP_PUSHBUTTON; break;
                        case OBJ_DLG_SPINBUTTON:    nObj = SVX_SNAP_SPINBUTTON; break;
                        case OBJ_DLG_FIXEDTEXT:     nObj = SVX_SNAP_FIXEDTEXT; break;
                        case OBJ_DLG_GROUPBOX:      nObj = SVX_SNAP_GROUPBOX; break;
                        case OBJ_DLG_EDIT:          nObj = SVX_SNAP_EDIT; break;
                        case OBJ_DLG_LISTBOX:       nObj = SVX_SNAP_LISTBOX; break;
                        case OBJ_DLG_COMBOBOX:      nObj = SVX_SNAP_COMBOBOX; break;
                        case OBJ_DLG_HSCROLLBAR:    nObj = SVX_SNAP_HSCROLLBAR; break;
                        case OBJ_DLG_VSCROLLBAR:    nObj = SVX_SNAP_VSCROLLBAR; break;
                        case OBJ_DLG_PREVIEW:       nObj = SVX_SNAP_PREVIEW; break;
                        case OBJ_DLG_URLBUTTON:     nObj = SVX_SNAP_URLBUTTON; break;
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
                    GetEditor()->SetMode( VCDLGED_INSERT );
                    GetEditor()->SetInsertObj( OBJ_DLG_PUSHBUTTON );
                }
                break;
                case SVX_SNAP_RADIOBUTTON:
                {
                    GetEditor()->SetMode( VCDLGED_INSERT );
                    GetEditor()->SetInsertObj( OBJ_DLG_RADIOBUTTON );
                }
                break;
                case SVX_SNAP_CHECKBOX:
                {
                    GetEditor()->SetMode( VCDLGED_INSERT );
                    GetEditor()->SetInsertObj( OBJ_DLG_CHECKBOX);
                }
                break;
                case SVX_SNAP_SPINBUTTON:
                {
                    GetEditor()->SetMode( VCDLGED_INSERT );
                    GetEditor()->SetInsertObj( OBJ_DLG_SPINBUTTON );
                }
                break;
                case SVX_SNAP_FIXEDTEXT:
                {
                    GetEditor()->SetMode( VCDLGED_INSERT );
                    GetEditor()->SetInsertObj( OBJ_DLG_FIXEDTEXT );
                }
                break;
                case SVX_SNAP_GROUPBOX:
                {
                    GetEditor()->SetMode( VCDLGED_INSERT );
                    GetEditor()->SetInsertObj( OBJ_DLG_GROUPBOX );
                }
                break;
                case SVX_SNAP_LISTBOX:
                {
                    GetEditor()->SetMode( VCDLGED_INSERT );
                    GetEditor()->SetInsertObj( OBJ_DLG_LISTBOX );
                }
                break;
                case SVX_SNAP_COMBOBOX:
                {
                    GetEditor()->SetMode( VCDLGED_INSERT );
                    GetEditor()->SetInsertObj( OBJ_DLG_COMBOBOX );
                }
                break;
                case SVX_SNAP_EDIT:
                {
                    GetEditor()->SetMode( VCDLGED_INSERT );
                    GetEditor()->SetInsertObj( OBJ_DLG_EDIT );
                }
                break;
                case SVX_SNAP_HSCROLLBAR:
                {
                    GetEditor()->SetMode( VCDLGED_INSERT );
                    GetEditor()->SetInsertObj( OBJ_DLG_HSCROLLBAR );
                }
                break;
                case SVX_SNAP_VSCROLLBAR:
                {
                    GetEditor()->SetMode( VCDLGED_INSERT );
                    GetEditor()->SetInsertObj( OBJ_DLG_VSCROLLBAR );
                }
                break;
                case SVX_SNAP_PREVIEW:
                {
                    GetEditor()->SetMode( VCDLGED_INSERT );
                    GetEditor()->SetInsertObj( OBJ_DLG_PREVIEW );
                }
                break;
                case SVX_SNAP_URLBUTTON:
                {
                    GetEditor()->SetMode( VCDLGED_INSERT );
                    GetEditor()->SetInsertObj( OBJ_DLG_URLBUTTON );
                }
                break;
                case SVX_SNAP_SELECT:
                {
                    GetEditor()->SetMode( VCDLGED_SELECT );
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
            VCDlgMode eOldMode = GetEditor()->GetMode();
            GetEditor()->SetMode( VCDLGED_TEST );
            GetEditor()->SetMode( eOldMode );
            rReq.Done();
            BasicIDE::GetBindings().Invalidate( SID_DIALOG_TESTMODE );
            return;
        }
        break;
    }

    rReq.Done();
}



void DialogWindow::RenameDialog( const String& rNewName )
{
    pDialog->SetName( rNewName );
    BasicIDE::GetBindings().Invalidate( SID_DOC_MODIFIED );
}




void DialogWindow::DisableBrowser()
{
    SfxViewFrame* pCurFrame = SfxViewFrame::Current();
    SfxChildWindow* pChildWin = pCurFrame ? pCurFrame->GetChildWindow(SID_SHOW_BROWSER) : NULL;
    if( pChildWin )
#ifdef _DLGEDITOR_
        ((PropBrw*)(pChildWin->GetWindow()))->Update( 0 );
#else
        ((VCBrowser*)(pChildWin->GetWindow()))->Update( 0 );
#endif
}




void DialogWindow::UpdateBrowser()
{
    SfxViewFrame* pCurFrame = SfxViewFrame::Current();
    SfxChildWindow* pChildWin = pCurFrame ? pCurFrame->GetChildWindow(SID_SHOW_BROWSER) : NULL;
    if( pChildWin )
#ifdef _DLGEDITOR_
        ((PropBrw*)(pChildWin->GetWindow()))->Update(GetEditor()->GetView());
#else
        ((VCBrowser*)(pChildWin->GetWindow()))->Update(GetEditor()->GetView());
#endif
}




SdrView* DialogWindow::GetView() const
{
    return GetEditor()->GetView();
}



BOOL __EXPORT DialogWindow::IsModified()
{
    // So auf jedenfall nicht verkehrt...
    return pDialog->IsModified() || pEditor->IsModified();
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
    // Die Daten befinden sich schon im Basic...
//  pDialog->SetModified( FALSE );  // dann wird das Modify vom Basic geloescht
#if 1
    // New uno dialogs
    if( mxLibContainer.is() && (pEditor->IsModified() || (pDialog->GetFlags() & SBX_MODIFIED)!=0 ) )
    {
        // It has to be checked before that the library exists
        // and that it supports XNameContainer
        Reference< container::XNameContainer > xLib;
        rtl::OUString aOULibName( maLibName );
        Any aElement = mxLibContainer->getByName( aOULibName );
        aElement >>= xLib;

        Reference< container::XNameContainer > xDialogModel = pEditor->GetDialog();

        if( xDialogModel.is() )
        {
            Sequence< Reference< container::XNameContainer > > aModelSeq( 1 );
            aModelSeq.getArray()[0] = xDialogModel;
            Sequence< sal_Int8 > aSeq;

            xmlscript::exportDialogModelsToByteSequence( &aSeq, aModelSeq );
            Any aSeqAny;
            aSeqAny <<= aSeq;
            xLib->replaceByName( rtl::OUString( maDlgName ), aSeqAny );

            // HACK: Modify via old dialog to force SaveBasicManager()
            pDialog->SetModified( TRUE );
        }
    }

    if( pEditor->IsModified() )
        BasicIDE::MarkDocShellModified( pEditor->GetBasic() );
    pDialog->ResetFlag( SBX_MODIFIED );
    pEditor->ClearModifyFlag();
#else
    if( pEditor->IsModified() )
        pDialog->SetModified( TRUE );
    else
        pDialog->ResetFlag( SBX_MODIFIED );
    pEditor->ClearModifyFlag();
#endif

}

void DialogWindow::Deactivating()
{
    if( pEditor->IsModified() )
        BasicIDE::MarkDocShellModified( pEditor->GetBasic() );
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
