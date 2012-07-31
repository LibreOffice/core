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

#include "basidesh.hrc"
#include "helpid.hrc"

#include "accessibledialogwindow.hxx"
#include "baside3.hxx"
#include "basidesh.hxx"
#include "bastype2.hxx"
#include "dlged.hxx"
#include "dlgeddef.hxx"
#include "dlgedmod.hxx"
#include "dlgedview.hxx"
#include "iderdll.hxx"
#include "idetemp.hxx"
#include "localizationmgr.hxx"
#include "propbrw.hxx"

#include <basic/basmgr.hxx>
#include <com/sun/star/resource/StringResourceWithLocation.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess2.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#include <comphelper/processfactory.hxx>
#include <sfx2/dinfdlg.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <svl/aeitem.hxx>
#include <svl/whiter.hxx>
#include <tools/diagnose_ex.h>
#include <tools/urlobj.hxx>
#include <vcl/msgbox.hxx>
#include <xmlscript/xmldlg_imexp.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::resource;
using namespace ::com::sun::star::ui::dialogs;

#if defined(UNX)
#define FILTERMASK_ALL "*"
#else
#define FILTERMASK_ALL "*.*"
#endif

DBG_NAME( DialogWindow )

TYPEINIT1( DialogWindow, IDEBaseWindow );

DialogWindow::DialogWindow( Window* pParent, const ScriptDocument& rDocument, ::rtl::OUString aLibName, ::rtl::OUString aName,
    const com::sun::star::uno::Reference< com::sun::star::container::XNameContainer >& xDialogModel )
        :IDEBaseWindow( pParent, rDocument, aLibName, aName )
        ,pUndoMgr(NULL)
{
    InitSettings( sal_True, sal_True, sal_True );

    pEditor = new DlgEditor( rDocument.isDocument() ? rDocument.getDocument() : Reference< frame::XModel >() );
    pEditor->SetWindow( this );
    pEditor->SetDialog( xDialogModel );

    pUndoMgr = new SfxUndoManager;

    Link aDummyLink;
    aOldNotifyUndoActionHdl = pEditor->GetModel()->GetNotifyUndoActionHdl();
    pEditor->GetModel()->SetNotifyUndoActionHdl(
        LINK(this, DialogWindow, NotifyUndoActionHdl));

    SetHelpId( HID_BASICIDE_DIALOGWINDOW );

    // set readonly mode for readonly libraries
    Reference< script::XLibraryContainer2 > xDlgLibContainer( GetDocument().getLibraryContainer( E_DIALOGS ), UNO_QUERY );
    if ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aLibName ) && xDlgLibContainer->isLibraryReadOnly( aLibName ) )
        SetReadOnly( sal_True );

    if ( rDocument.isDocument() && rDocument.isReadOnly() )
        SetReadOnly( sal_True );
}

DialogWindow::~DialogWindow()
{
    delete pEditor;
    delete pUndoMgr;
}

void DialogWindow::LoseFocus()
{
    if ( IsModified() )
        StoreData();

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

    SfxBindings* pBindings = BasicIDE::GetBindingsPtr();
    if ( pBindings )
        pBindings->Invalidate( SID_SHOW_PROPERTYBROWSER );
}



void DialogWindow::MouseButtonUp( const MouseEvent& rMEvt )
{
    pEditor->MouseButtonUp( rMEvt );
    SfxBindings* pBindings = BasicIDE::GetBindingsPtr();
    if( (pEditor->GetMode() == DLGED_INSERT) && !pEditor->IsCreateOK() )
    {
        pEditor->SetMode( DLGED_SELECT );
        if ( pBindings )
            pBindings->Invalidate( SID_CHOOSE_CONTROLS );
    }
    if ( pBindings )
    {
        pBindings->Invalidate( SID_SHOW_PROPERTYBROWSER );
        pBindings->Invalidate( SID_DOC_MODIFIED );
        pBindings->Invalidate( SID_SAVEDOC );
    }
}



void DialogWindow::MouseMove( const MouseEvent& rMEvt )
{
    pEditor->MouseMove( rMEvt );
}



void DialogWindow::KeyInput( const KeyEvent& rKEvt )
{
    if( rKEvt.GetKeyCode() == KEY_BACKSPACE )
    {
        BasicIDEShell* pIDEShell = BasicIDEGlobals::GetShell();
        SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
        SfxDispatcher* pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;
        if( pDispatcher )
        {
            pDispatcher->Execute( SID_BACKSPACE );
        }
    }
    else
    {
        SfxBindings* pBindings = BasicIDE::GetBindingsPtr();
        if( pBindings && rKEvt.GetKeyCode() == KEY_TAB )
            pBindings->Invalidate( SID_SHOW_PROPERTYBROWSER );

        if( !pEditor->KeyInput( rKEvt ) )
        {
            if( !SfxViewShell::Current()->KeyInput( rKEvt ) )
                Window::KeyInput( rKEvt );
        }
    }
}

void DialogWindow::Command( const CommandEvent& rCEvt )
{
    if ( ( rCEvt.GetCommand() == COMMAND_WHEEL           ) ||
         ( rCEvt.GetCommand() == COMMAND_STARTAUTOSCROLL ) ||
         ( rCEvt.GetCommand() == COMMAND_AUTOSCROLL      ) )
    {
        HandleScrollCommand( rCEvt, GetHScrollBar(), GetVScrollBar() );
    }
    else if ( rCEvt.GetCommand() == COMMAND_CONTEXTMENU )
    {
        BasicIDEShell* pIDEShell = BasicIDEGlobals::GetShell();
        SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
        SfxDispatcher* pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;
        if ( pDispatcher )
        {
            SdrView* pView = GetView();
            if( !rCEvt.IsMouseEvent() && pView->AreObjectsMarked() )
            {
                Rectangle aMarkedRect( pView->GetMarkedRect() );
                Point MarkedCenter( aMarkedRect.Center() );
                Point PosPixel( LogicToPixel( MarkedCenter ) );
                pDispatcher->ExecutePopup( IDEResId(RID_POPUP_DLGED), this, &PosPixel );
            }
            else
            {
                pDispatcher->ExecutePopup( IDEResId(RID_POPUP_DLGED) );
            }

        }
    }
    else
        IDEBaseWindow::Command( rCEvt );
}




IMPL_LINK( DialogWindow, NotifyUndoActionHdl, SfxUndoAction *, pUndoAction )
{
    (void)pUndoAction;

    return 0;
}



void DialogWindow::DoInit()
{
    GetHScrollBar()->Show();
    GetVScrollBar()->Show();
    pEditor->SetScrollBars( GetHScrollBar(), GetVScrollBar() );
}



void DialogWindow::DoScroll( ScrollBar* pCurScrollBar )
{
    pEditor->DoScroll( pCurScrollBar );
}

void DialogWindow::GetState( SfxItemSet& rSet )
{
    SfxWhichIter aIter(rSet);
    bool bIsCalc = false;
    if ( GetDocument().isDocument() )
    {
        Reference< frame::XModel > xModel= GetDocument().getDocument();
        if ( xModel.is() )
        {
            Reference< lang::XServiceInfo > xServiceInfo ( xModel, UNO_QUERY );
            if ( xServiceInfo.is() && xServiceInfo->supportsService( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sheet.SpreadsheetDocument") ) ) )
                bIsCalc = true;
        }
    }

    for ( sal_uInt16 nWh = aIter.FirstWhich(); 0 != nWh; nWh = aIter.NextWhich() )
    {
        switch ( nWh )
        {
            case SID_PASTE:
            {
                if ( !IsPasteAllowed() )
                    rSet.DisableItem( nWh );

                if ( IsReadOnly() )
                    rSet.DisableItem( nWh );
            }
            break;
            case SID_COPY:
            {
                // any object selected?
                if ( !pEditor->GetView()->AreObjectsMarked() )
                    rSet.DisableItem( nWh );
            }
            break;
            case SID_CUT:
            case SID_DELETE:
            case SID_BACKSPACE:
            {
                // any object selected?
                if ( !pEditor->GetView()->AreObjectsMarked() )
                    rSet.DisableItem( nWh );

                if ( IsReadOnly() )
                    rSet.DisableItem( nWh );
            }
            break;
            case SID_REDO:
            {
                if ( !pUndoMgr->GetUndoActionCount() )
                    rSet.DisableItem( nWh );
            }
            break;

            case SID_DIALOG_TESTMODE:
            {
                // is the IDE still active?
                if( BasicIDEGlobals::GetShell()->GetFrame() )
                {
                    rSet.Put( SfxBoolItem( SID_DIALOG_TESTMODE,
                              (pEditor->GetMode() == DLGED_TEST) ? sal_True : sal_False) );
                }
                else
                    rSet.Put( SfxBoolItem( SID_DIALOG_TESTMODE,sal_False ));
            }
            break;

            case SID_CHOOSE_CONTROLS:
            {
                if ( IsReadOnly() )
                {
                    rSet.DisableItem( nWh );
                }
                else
                {
                    SfxAllEnumItem aItem( SID_CHOOSE_CONTROLS );
                    if ( GetEditor()->GetMode() == DLGED_SELECT )
                        aItem.SetValue( SVX_SNAP_SELECT );
                    else
                    {
                        sal_uInt16 nObj;
                        switch( pEditor->GetInsertObj() )
                        {
                            case OBJ_DLG_PUSHBUTTON:        nObj = SVX_SNAP_PUSHBUTTON; break;
                            case OBJ_DLG_RADIOBUTTON:       nObj = SVX_SNAP_RADIOBUTTON; break;
                            case OBJ_DLG_CHECKBOX:          nObj = SVX_SNAP_CHECKBOX; break;
                            case OBJ_DLG_LISTBOX:           nObj = SVX_SNAP_LISTBOX; break;
                            case OBJ_DLG_COMBOBOX:          nObj = SVX_SNAP_COMBOBOX; break;
                            case OBJ_DLG_GROUPBOX:          nObj = SVX_SNAP_GROUPBOX; break;
                            case OBJ_DLG_EDIT:              nObj = SVX_SNAP_EDIT; break;
                            case OBJ_DLG_FIXEDTEXT:         nObj = SVX_SNAP_FIXEDTEXT; break;
                            case OBJ_DLG_IMAGECONTROL:      nObj = SVX_SNAP_IMAGECONTROL; break;
                            case OBJ_DLG_PROGRESSBAR:       nObj = SVX_SNAP_PROGRESSBAR; break;
                            case OBJ_DLG_HSCROLLBAR:        nObj = SVX_SNAP_HSCROLLBAR; break;
                            case OBJ_DLG_VSCROLLBAR:        nObj = SVX_SNAP_VSCROLLBAR; break;
                            case OBJ_DLG_HFIXEDLINE:        nObj = SVX_SNAP_HFIXEDLINE; break;
                            case OBJ_DLG_VFIXEDLINE:        nObj = SVX_SNAP_VFIXEDLINE; break;
                            case OBJ_DLG_DATEFIELD:         nObj = SVX_SNAP_DATEFIELD; break;
                            case OBJ_DLG_TIMEFIELD:         nObj = SVX_SNAP_TIMEFIELD; break;
                            case OBJ_DLG_NUMERICFIELD:      nObj = SVX_SNAP_NUMERICFIELD; break;
                            case OBJ_DLG_CURRENCYFIELD:     nObj = SVX_SNAP_CURRENCYFIELD; break;
                            case OBJ_DLG_FORMATTEDFIELD:    nObj = SVX_SNAP_FORMATTEDFIELD; break;
                            case OBJ_DLG_PATTERNFIELD:      nObj = SVX_SNAP_PATTERNFIELD; break;
                            case OBJ_DLG_FILECONTROL:       nObj = SVX_SNAP_FILECONTROL; break;
                            case OBJ_DLG_SPINBUTTON:        nObj = SVX_SNAP_SPINBUTTON; break;
                            case OBJ_DLG_TREECONTROL:       nObj = SVX_SNAP_TREECONTROL; break;
                            default:                        nObj = 0;
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
            }
            break;

            case SID_SHOW_PROPERTYBROWSER:
            {
                BasicIDEShell* pIDEShell = BasicIDEGlobals::GetShell();
                SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
                if ( pViewFrame && !pViewFrame->HasChildWindow( SID_SHOW_PROPERTYBROWSER ) && !pEditor->GetView()->AreObjectsMarked() )
                    rSet.DisableItem( nWh );

                if ( IsReadOnly() )
                    rSet.DisableItem( nWh );
            }
            break;
            case SID_INSERT_FORM_RADIO:
            case SID_INSERT_FORM_CHECK:
            case SID_INSERT_FORM_LIST:
            case SID_INSERT_FORM_COMBO:
            case SID_INSERT_FORM_VSCROLL:
            case SID_INSERT_FORM_HSCROLL:
            case SID_INSERT_FORM_SPIN:
            {
                if ( !bIsCalc || IsReadOnly() )
                    rSet.DisableItem( nWh );
            }
            break;
        }
    }
}



void DialogWindow::ExecuteCommand( SfxRequest& rReq )
{
    switch ( rReq.GetSlot() )
    {
        case SID_CUT:
            if ( !IsReadOnly() )
            {
                GetEditor()->Cut();
                SfxBindings* pBindings = BasicIDE::GetBindingsPtr();
                if ( pBindings )
                    pBindings->Invalidate( SID_DOC_MODIFIED );
            }
            break;
        case SID_DELETE:
            if ( !IsReadOnly() )
            {
                GetEditor()->Delete();
                SfxBindings* pBindings = BasicIDE::GetBindingsPtr();
                if ( pBindings )
                    pBindings->Invalidate( SID_DOC_MODIFIED );
            }
            break;
        case SID_COPY:
            GetEditor()->Copy();
            break;
        case SID_PASTE:
            if ( !IsReadOnly() )
            {
                GetEditor()->Paste();
                SfxBindings* pBindings = BasicIDE::GetBindingsPtr();
                if ( pBindings )
                    pBindings->Invalidate( SID_DOC_MODIFIED );
            }
            break;
        case SID_INSERT_FORM_RADIO:
            GetEditor()->SetMode( DLGED_INSERT );
            GetEditor()->SetInsertObj( OBJ_DLG_FORMRADIO );
            break;
        case SID_INSERT_FORM_CHECK:
            GetEditor()->SetMode( DLGED_INSERT );
            GetEditor()->SetInsertObj( OBJ_DLG_FORMCHECK );
            break;
        case SID_INSERT_FORM_LIST:
            GetEditor()->SetMode( DLGED_INSERT );
            GetEditor()->SetInsertObj( OBJ_DLG_FORMLIST );
            break;
        case SID_INSERT_FORM_COMBO:
            GetEditor()->SetMode( DLGED_INSERT );
            GetEditor()->SetInsertObj( OBJ_DLG_FORMCOMBO );
            break;
        case SID_INSERT_FORM_SPIN:
            GetEditor()->SetMode( DLGED_INSERT );
            GetEditor()->SetInsertObj( OBJ_DLG_FORMSPIN );
            break;
        case SID_INSERT_FORM_VSCROLL:
            GetEditor()->SetMode( DLGED_INSERT );
            GetEditor()->SetInsertObj( OBJ_DLG_FORMVSCROLL );
            break;
        case SID_INSERT_FORM_HSCROLL:
            GetEditor()->SetMode( DLGED_INSERT );
            GetEditor()->SetInsertObj( OBJ_DLG_FORMHSCROLL );
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
                case SVX_SNAP_DATEFIELD:
                {
                    GetEditor()->SetMode( DLGED_INSERT );
                    GetEditor()->SetInsertObj( OBJ_DLG_DATEFIELD );
                }
                break;
                case SVX_SNAP_TIMEFIELD:
                {
                    GetEditor()->SetMode( DLGED_INSERT );
                    GetEditor()->SetInsertObj( OBJ_DLG_TIMEFIELD );
                }
                break;
                case SVX_SNAP_NUMERICFIELD:
                {
                    GetEditor()->SetMode( DLGED_INSERT );
                    GetEditor()->SetInsertObj( OBJ_DLG_NUMERICFIELD );
                }
                break;
                case SVX_SNAP_CURRENCYFIELD:
                {
                    GetEditor()->SetMode( DLGED_INSERT );
                    GetEditor()->SetInsertObj( OBJ_DLG_CURRENCYFIELD );
                }
                break;
                case SVX_SNAP_FORMATTEDFIELD:
                {
                    GetEditor()->SetMode( DLGED_INSERT );
                    GetEditor()->SetInsertObj( OBJ_DLG_FORMATTEDFIELD );
                }
                break;
                case SVX_SNAP_PATTERNFIELD:
                {
                    GetEditor()->SetMode( DLGED_INSERT );
                    GetEditor()->SetInsertObj( OBJ_DLG_PATTERNFIELD );
                }
                break;
                case SVX_SNAP_FILECONTROL:
                {
                    GetEditor()->SetMode( DLGED_INSERT );
                    GetEditor()->SetInsertObj( OBJ_DLG_FILECONTROL );
                }
                break;
                case SVX_SNAP_SPINBUTTON:
                {
                    GetEditor()->SetMode( DLGED_INSERT );
                    GetEditor()->SetInsertObj( OBJ_DLG_SPINBUTTON );
                }
                break;
                case SVX_SNAP_TREECONTROL:
                {
                    GetEditor()->SetMode( DLGED_INSERT );
                    GetEditor()->SetInsertObj( OBJ_DLG_TREECONTROL );
                }
                break;

                case SVX_SNAP_SELECT:
                {
                    GetEditor()->SetMode( DLGED_SELECT );
                }
                break;
            }

            if ( rReq.GetModifier() & KEY_MOD1 )
            {
                if ( GetEditor()->GetMode() == DLGED_INSERT )
                    GetEditor()->CreateDefaultObject();
            }

            SfxBindings* pBindings = BasicIDE::GetBindingsPtr();
            if ( pBindings )
                pBindings->Invalidate( SID_DOC_MODIFIED );
        }
        break;

        case SID_DIALOG_TESTMODE:
        {
            DlgEdMode eOldMode = GetEditor()->GetMode();
            GetEditor()->SetMode( DLGED_TEST );
            GetEditor()->SetMode( eOldMode );
            rReq.Done();
            SfxBindings* pBindings = BasicIDE::GetBindingsPtr();
            if ( pBindings )
                pBindings->Invalidate( SID_DIALOG_TESTMODE );
            return;
        }
        case SID_EXPORT_DIALOG:
            SaveDialog();
            break;

        case SID_IMPORT_DIALOG:
            ImportDialog();
            break;
    }

    rReq.Done();
}

Reference< container::XNameContainer > DialogWindow::GetDialog() const
{
    return pEditor->GetDialog();
}

sal_Bool DialogWindow::RenameDialog( const ::rtl::OUString& rNewName )
{
    if ( !BasicIDE::RenameDialog( this, GetDocument(), GetLibName(), GetName(), rNewName ) )
        return sal_False;

    SfxBindings* pBindings = BasicIDE::GetBindingsPtr();
    if ( pBindings )
        pBindings->Invalidate( SID_DOC_MODIFIED );

    return sal_True;
}

void DialogWindow::DisableBrowser()
{
    BasicIDEShell* pIDEShell = BasicIDEGlobals::GetShell();
    SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
    SfxChildWindow* pChildWin = pViewFrame ? pViewFrame->GetChildWindow(SID_SHOW_PROPERTYBROWSER) : NULL;
    if( pChildWin )
        ((PropBrw*)(pChildWin->GetWindow()))->Update( NULL );
}

void DialogWindow::UpdateBrowser()
{
    BasicIDEShell* pIDEShell = BasicIDEGlobals::GetShell();
    SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
    SfxChildWindow* pChildWin = pViewFrame ? pViewFrame->GetChildWindow(SID_SHOW_PROPERTYBROWSER) : NULL;
    if( pChildWin )
        ((PropBrw*)(pChildWin->GetWindow()))->Update( pIDEShell );
}

static ::rtl::OUString aResourceResolverPropName( RTL_CONSTASCII_USTRINGPARAM( "ResourceResolver" ));

bool DialogWindow::SaveDialog()
{
    DBG_CHKTHIS( DialogWindow, 0 );
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
    xFPControl->enableControl(ExtendedFilePickerElementIds::CHECKBOX_PASSWORD, sal_False);
    Any aValue;
    aValue <<= (sal_Bool) sal_True;
    xFPControl->setValue(ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION, 0, aValue);

    if ( !aCurPath.isEmpty() )
        xFP->setDisplayDirectory ( aCurPath );

    xFP->setDefaultName( ::rtl::OUString( GetName() ) );

    ::rtl::OUString aDialogStr(IDE_RESSTR(RID_STR_STDDIALOGNAME));
    Reference< XFilterManager > xFltMgr(xFP, UNO_QUERY);
    xFltMgr->appendFilter( aDialogStr, String( RTL_CONSTASCII_USTRINGPARAM( "*.xdl" ) ) );
    xFltMgr->appendFilter( IDE_RESSTR(RID_STR_FILTER_ALLFILES), String( RTL_CONSTASCII_USTRINGPARAM( FILTERMASK_ALL ) ) );
    xFltMgr->setCurrentFilter( aDialogStr );

    if( xFP->execute() == RET_OK )
    {
        Sequence< ::rtl::OUString > aPaths = xFP->getFiles();
        aCurPath = aPaths[0];

        // export dialog model to xml
        Reference< container::XNameContainer > xDialogModel = GetDialog();
        Reference< XComponentContext > xContext;
        Reference< beans::XPropertySet > xProps( ::comphelper::getProcessServiceFactory(), UNO_QUERY );
        OSL_ASSERT( xProps.is() );
        OSL_VERIFY( xProps->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DefaultContext")) ) >>= xContext );
        Reference< XInputStreamProvider > xISP = ::xmlscript::exportDialogModel( xDialogModel, xContext, GetDocument().isDocument() ? GetDocument().getDocument() : Reference< frame::XModel >() );
        Reference< XInputStream > xInput( xISP->createInputStream() );

        Reference< XSimpleFileAccess2 > xSFI( SimpleFileAccess::create(comphelper::getProcessComponentContext()) );

        Reference< XOutputStream > xOutput;
        try
        {
            if( xSFI->exists( aCurPath ) )
                xSFI->kill( aCurPath );
            xOutput = xSFI->openFileWrite( aCurPath );
        }
        catch(const Exception& )
        {}

        if( xOutput.is() )
        {
            Sequence< sal_Int8 > bytes;
            sal_Int32 nRead = xInput->readBytes( bytes, xInput->available() );
            for (;;)
            {
                if( nRead )
                    xOutput->writeBytes( bytes );

                nRead = xInput->readBytes( bytes, 1024 );
                if (! nRead)
                    break;
            }
            bDone = true;

            // With resource?
            Reference< beans::XPropertySet > xDialogModelPropSet( xDialogModel, UNO_QUERY );
            Reference< resource::XStringResourceResolver > xStringResourceResolver;
            if( xDialogModelPropSet.is() )
            {
                try
                {
                    Any aResourceResolver = xDialogModelPropSet->getPropertyValue( aResourceResolverPropName );
                    aResourceResolver >>= xStringResourceResolver;
                }
                catch(const beans::UnknownPropertyException& )
                {}
            }

            bool bResource = false;
            if( xStringResourceResolver.is() )
            {
                Sequence< lang::Locale > aLocaleSeq = xStringResourceResolver->getLocales();
                sal_Int32 nLocaleCount = aLocaleSeq.getLength();
                if( nLocaleCount > 0 )
                    bResource = true;
            }

            if( bResource )
            {
                INetURLObject aURLObj( aCurPath );
                aURLObj.removeExtension();
                ::rtl::OUString aDialogName( aURLObj.getName() );
                aURLObj.removeSegment();
                ::rtl::OUString aURL( aURLObj.GetMainURL( INetURLObject::NO_DECODE ) );
                sal_Bool bReadOnly = sal_False;
                ::rtl::OUString aComment( RTL_CONSTASCII_USTRINGPARAM( "# " ));
                aComment += aDialogName;
                aComment += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( " strings" ));
                Reference< task::XInteractionHandler > xDummyHandler;

                // Remove old properties files in case of overwriting Dialog files
                if( xSFI->isFolder( aURL ) )
                {
                    Sequence< ::rtl::OUString > aContentSeq = xSFI->getFolderContents( aURL, false );

                    ::rtl::OUString aDialogName_( aDialogName );
                    aDialogName_ += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "_" ));
                    sal_Int32 nCount = aContentSeq.getLength();
                    const ::rtl::OUString* pFiles = aContentSeq.getConstArray();
                    for( int i = 0 ; i < nCount ; i++ )
                    {
                        ::rtl::OUString aCompleteName = pFiles[i];
                        rtl::OUString aPureName;
                        rtl::OUString aExtension;
                        sal_Int32 iDot = aCompleteName.lastIndexOf( '.' );
                        sal_Int32 iSlash = aCompleteName.lastIndexOf( '/' );
                        if( iDot != -1 )
                        {
                            sal_Int32 iCopyFrom = (iSlash != -1) ? iSlash + 1 : 0;
                            aPureName = aCompleteName.copy( iCopyFrom, iDot-iCopyFrom );
                            aExtension = aCompleteName.copy( iDot + 1 );
                        }

                        if( aExtension == "properties" || aExtension == "default" )
                        {
                            if( aPureName.indexOf( aDialogName_ ) == 0 )
                            {
                                try
                                {
                                    xSFI->kill( aCompleteName );
                                }
                                catch(const uno::Exception& )
                                {}
                            }
                        }
                    }
                }

                Reference< XStringResourceWithLocation > xStringResourceWithLocation =
                    StringResourceWithLocation::create( xContext, aURL, bReadOnly,
                        xStringResourceResolver->getDefaultLocale(), aDialogName, aComment, xDummyHandler );

                // Add locales
                Sequence< lang::Locale > aLocaleSeq = xStringResourceResolver->getLocales();
                const lang::Locale* pLocales = aLocaleSeq.getConstArray();
                sal_Int32 nLocaleCount = aLocaleSeq.getLength();
                for( sal_Int32 iLocale = 0 ; iLocale < nLocaleCount ; iLocale++ )
                {
                    const lang::Locale& rLocale = pLocales[ iLocale ];
                    xStringResourceWithLocation->newLocale( rLocale );
                }

                Reference< XStringResourceManager > xTargetStringResourceManager( xStringResourceWithLocation, uno::UNO_QUERY );

                LocalizationMgr::copyResourceForDialog( xDialogModel,
                    xStringResourceResolver, xTargetStringResourceManager );

                xStringResourceWithLocation->store();
            }
        }
        else
            ErrorBox( this, WB_OK | WB_DEF_OK, IDE_RESSTR(RID_STR_COULDNTWRITE)).Execute();
    }

    return bDone;
}

extern bool localesAreEqual( const ::com::sun::star::lang::Locale& rLocaleLeft,
                             const ::com::sun::star::lang::Locale& rLocaleRight );

std::vector< lang::Locale > implGetLanguagesOnlyContainedInFirstSeq
    ( Sequence< lang::Locale > aFirstSeq, Sequence< lang::Locale > aSecondSeq )
{
    std::vector< lang::Locale > avRet;

    const lang::Locale* pFirst = aFirstSeq.getConstArray();
    const lang::Locale* pSecond = aSecondSeq.getConstArray();
    sal_Int32 nFirstCount = aFirstSeq.getLength();
    sal_Int32 nSecondCount = aSecondSeq.getLength();

    for( sal_Int32 iFirst = 0 ; iFirst < nFirstCount ; iFirst++ )
    {
        const lang::Locale& rFirstLocale = pFirst[ iFirst ];

        bool bAlsoContainedInSecondSeq = false;
        for( sal_Int32 iSecond = 0 ; iSecond < nSecondCount ; iSecond++ )
        {
            const lang::Locale& rSecondLocale = pSecond[ iSecond ];

            bool bMatch = localesAreEqual( rFirstLocale, rSecondLocale );
            if( bMatch )
            {
                bAlsoContainedInSecondSeq = true;
                break;
            }
        }

        if( !bAlsoContainedInSecondSeq )
            avRet.push_back( rFirstLocale );
    }

    return avRet;
}


class NameClashQueryBox : public MessBox
{
public:
    NameClashQueryBox( Window* pParent,
        const XubString& rTitle, const XubString& rMessage );
};

NameClashQueryBox::NameClashQueryBox( Window* pParent,
    const XubString& rTitle, const XubString& rMessage )
        : MessBox( pParent, 0, rTitle, rMessage )
{
    if ( rTitle.Len() )
        SetText( rTitle );

    maMessText = rMessage;

    AddButton( IDE_RESSTR(RID_STR_DLGIMP_CLASH_RENAME), RET_YES,
        BUTTONDIALOG_DEFBUTTON | BUTTONDIALOG_OKBUTTON | BUTTONDIALOG_FOCUSBUTTON );
    AddButton( IDE_RESSTR(RID_STR_DLGIMP_CLASH_REPLACE), RET_NO, 0 );
    AddButton( BUTTON_CANCEL, RET_CANCEL, BUTTONDIALOG_CANCELBUTTON );

    SetImage( QueryBox::GetStandardImage() );
}


class LanguageMismatchQueryBox : public MessBox
{
public:
    LanguageMismatchQueryBox( Window* pParent,
        const XubString& rTitle, const XubString& rMessage );
};

LanguageMismatchQueryBox::LanguageMismatchQueryBox( Window* pParent,
    const XubString& rTitle, const XubString& rMessage )
        : MessBox( pParent, 0, rTitle, rMessage )
{
    if ( rTitle.Len() )
        SetText( rTitle );

    maMessText = rMessage;
    AddButton( IDE_RESSTR(RID_STR_DLGIMP_MISMATCH_ADD), RET_YES,
        BUTTONDIALOG_DEFBUTTON | BUTTONDIALOG_OKBUTTON | BUTTONDIALOG_FOCUSBUTTON );
    AddButton( IDE_RESSTR(RID_STR_DLGIMP_MISMATCH_OMIT), RET_NO, 0 );
    AddButton( BUTTON_CANCEL, RET_CANCEL, BUTTONDIALOG_CANCELBUTTON );
    AddButton( BUTTON_HELP, BUTTONID_HELP, BUTTONDIALOG_HELPBUTTON, 4 );

    SetImage( QueryBox::GetStandardImage() );
}

bool implImportDialog( Window* pWin, const ::rtl::OUString& rCurPath, const ScriptDocument& rDocument, const ::rtl::OUString& aLibName )
{
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

    Reference< XFilePickerControlAccess > xFPControl(xFP, UNO_QUERY);
    xFPControl->enableControl(ExtendedFilePickerElementIds::CHECKBOX_PASSWORD, sal_False);
    Any aValue;
    aValue <<= (sal_Bool) sal_True;
    xFPControl->setValue(ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION, 0, aValue);

    ::rtl::OUString aCurPath( rCurPath );
    if ( !aCurPath.isEmpty() )
        xFP->setDisplayDirectory ( aCurPath );

    ::rtl::OUString aDialogStr(IDE_RESSTR(RID_STR_STDDIALOGNAME));
    Reference< XFilterManager > xFltMgr(xFP, UNO_QUERY);
    xFltMgr->appendFilter( aDialogStr, String( RTL_CONSTASCII_USTRINGPARAM( "*.xdl" ) ) );
    xFltMgr->appendFilter( IDE_RESSTR(RID_STR_FILTER_ALLFILES), String( RTL_CONSTASCII_USTRINGPARAM( FILTERMASK_ALL ) ) );
    xFltMgr->setCurrentFilter( aDialogStr );

    if( xFP->execute() == RET_OK )
    {
        Sequence< ::rtl::OUString > aPaths = xFP->getFiles();
        aCurPath = aPaths[0];

        ::rtl::OUString aBasePath;
        ::rtl::OUString aOUCurPath( aCurPath );
        sal_Int32 iSlash = aOUCurPath.lastIndexOf( '/' );
        if( iSlash != -1 )
            aBasePath = aOUCurPath.copy( 0, iSlash + 1 );

        try
        {
            // create dialog model
            Reference< container::XNameContainer > xDialogModel( xMSF->createInstance
                ( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlDialogModel" ) ) ), UNO_QUERY_THROW );

            Reference< XSimpleFileAccess2 > xSFI( SimpleFileAccess::create(comphelper::getProcessComponentContext()) );

            Reference< XInputStream > xInput;
            if( xSFI->exists( aCurPath ) )
                xInput = xSFI->openFileRead( aCurPath );

            Reference< XComponentContext > xContext;
            Reference< beans::XPropertySet > xProps( xMSF, UNO_QUERY );
            OSL_ASSERT( xProps.is() );
            OSL_VERIFY( xProps->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DefaultContext")) ) >>= xContext );
            ::xmlscript::importDialogModel( xInput, xDialogModel, xContext, rDocument.isDocument() ? rDocument.getDocument() : Reference< frame::XModel >() );

            ::rtl::OUString aXmlDlgName;
            Reference< beans::XPropertySet > xDialogModelPropSet( xDialogModel, UNO_QUERY );
            if( xDialogModelPropSet.is() )
            {
                try
                {
                    Any aXmlDialogNameAny = xDialogModelPropSet->getPropertyValue( DLGED_PROP_NAME );
                    ::rtl::OUString aOUXmlDialogName;
                    aXmlDialogNameAny >>= aOUXmlDialogName;
                    aXmlDlgName = aOUXmlDialogName;
                }
                catch(const beans::UnknownPropertyException& )
                {}
            }
            bool bValidName = !aXmlDlgName.isEmpty();
            OSL_ASSERT( bValidName );
            if( !bValidName )
                return bDone;

            bool bDialogAlreadyExists = rDocument.hasDialog( aLibName, aXmlDlgName );

            ::rtl::OUString aNewDlgName = aXmlDlgName;
            enum NameClashMode
            {
                NO_CLASH,
                CLASH_OVERWRITE_DIALOG,
                CLASH_RENAME_DIALOG,
            };
            NameClashMode eNameClashMode = NO_CLASH;
            if( bDialogAlreadyExists )
            {
                ::rtl::OUString aQueryBoxTitle(IDE_RESSTR(RID_STR_DLGIMP_CLASH_TITLE));
                ::rtl::OUString aQueryBoxText(IDE_RESSTR(RID_STR_DLGIMP_CLASH_TEXT));
                aQueryBoxText = aQueryBoxText.replaceAll("$(ARG1)", aXmlDlgName);

                NameClashQueryBox aQueryBox( pWin, aQueryBoxTitle, aQueryBoxText );
                sal_uInt16 nRet = aQueryBox.Execute();
                if( RET_YES == nRet )
                {
                    // RET_YES == Rename, see NameClashQueryBox::NameClashQueryBox
                    eNameClashMode = CLASH_RENAME_DIALOG;

                    aNewDlgName = rDocument.createObjectName( E_DIALOGS, aLibName );
                }
                else if( RET_NO == nRet )
                {
                    // RET_NO == Replace, see NameClashQueryBox::NameClashQueryBox
                    eNameClashMode = CLASH_OVERWRITE_DIALOG;
                }
                else if( RET_CANCEL == nRet )
                {
                    return bDone;
                }
            }

            BasicIDEShell* pIDEShell = BasicIDEGlobals::GetShell();
            if( pIDEShell == NULL )
            {
                OSL_ASSERT( pIDEShell != NULL );
                return bDone;
            }

            // Resource?
            ::com::sun::star::lang::Locale aLocale = Application::GetSettings().GetUILocale();
            Reference< task::XInteractionHandler > xDummyHandler;
            bool bReadOnly = true;
            Reference< XStringResourceWithLocation > xImportStringResource =
                StringResourceWithLocation::create( xContext, aBasePath, bReadOnly,
                aLocale, aXmlDlgName, ::rtl::OUString(), xDummyHandler );

            Sequence< lang::Locale > aImportLocaleSeq = xImportStringResource->getLocales();
            sal_Int32 nImportLocaleCount = aImportLocaleSeq.getLength();

            Reference< container::XNameContainer > xDialogLib( rDocument.getLibrary( E_DIALOGS, aLibName, sal_True ) );
            Reference< resource::XStringResourceManager > xLibStringResourceManager = LocalizationMgr::getStringResourceFromDialogLibrary( xDialogLib );
            sal_Int32 nLibLocaleCount = 0;
            Sequence< lang::Locale > aLibLocaleSeq;
            if( xLibStringResourceManager.is() )
            {
                aLibLocaleSeq = xLibStringResourceManager->getLocales();
                nLibLocaleCount = aLibLocaleSeq.getLength();
            }

            // Check language matches
            std::vector< lang::Locale > aOnlyInImportLanguages =
                implGetLanguagesOnlyContainedInFirstSeq( aImportLocaleSeq, aLibLocaleSeq );
            int nOnlyInImportLanguageCount = aOnlyInImportLanguages.size();

            // For now: Keep languages from lib
            bool bLibLocalized = (nLibLocaleCount > 0);
            bool bImportLocalized = (nImportLocaleCount > 0);

            bool bAddDialogLanguagesToLib = false;
            if( nOnlyInImportLanguageCount > 0 )
            {
                ::rtl::OUString aQueryBoxTitle(IDE_RESSTR(RID_STR_DLGIMP_MISMATCH_TITLE));
                ::rtl::OUString aQueryBoxText(IDE_RESSTR(RID_STR_DLGIMP_MISMATCH_TEXT));
                LanguageMismatchQueryBox aQueryBox( pWin, aQueryBoxTitle, aQueryBoxText );
                sal_uInt16 nRet = aQueryBox.Execute();
                if( RET_YES == nRet )
                {
                    // RET_YES == Add, see LanguageMismatchQueryBox::LanguageMismatchQueryBox
                    bAddDialogLanguagesToLib = true;
                }
                // RET_NO == Omit, see LanguageMismatchQueryBox::LanguageMismatchQueryBox
                // -> nothing to do here
                //else if( RET_NO == nRet )
                //{
                //}
                else if( RET_CANCEL == nRet )
                {
                    return bDone;
                }
            }

            if( bImportLocalized )
            {
                bool bCopyResourcesForDialog = true;
                if( bAddDialogLanguagesToLib )
                {
                    boost::shared_ptr<LocalizationMgr> pCurMgr = pIDEShell->GetCurLocalizationMgr();

                    lang::Locale aFirstLocale;
                    aFirstLocale = aOnlyInImportLanguages[0];
                    if( nOnlyInImportLanguageCount > 1 )
                    {
                        // Check if import default belongs to only import languages and use it then
                        lang::Locale aImportDefaultLocale = xImportStringResource->getDefaultLocale();
                        lang::Locale aTmpLocale;
                        for( int i = 0 ; i < nOnlyInImportLanguageCount ; ++i )
                        {
                            aTmpLocale = aOnlyInImportLanguages[i];
                            if( localesAreEqual( aImportDefaultLocale, aTmpLocale ) )
                            {
                                aFirstLocale = aImportDefaultLocale;
                                break;
                            }
                        }
                    }

                    Sequence< lang::Locale > aFirstLocaleSeq( 1 );
                    aFirstLocaleSeq[0] = aFirstLocale;
                    pCurMgr->handleAddLocales( aFirstLocaleSeq );

                    if( nOnlyInImportLanguageCount > 1 )
                    {
                        Sequence< lang::Locale > aRemainingLocaleSeq( nOnlyInImportLanguageCount - 1 );
                        lang::Locale aTmpLocale;
                        int iSeq = 0;
                        for( int i = 0 ; i < nOnlyInImportLanguageCount ; ++i )
                        {
                            aTmpLocale = aOnlyInImportLanguages[i];
                            if( !localesAreEqual( aFirstLocale, aTmpLocale ) )
                                aRemainingLocaleSeq[iSeq++] = aTmpLocale;
                        }
                        pCurMgr->handleAddLocales( aRemainingLocaleSeq );
                    }
                }
                else if( !bLibLocalized )
                {
                    Reference< resource::XStringResourceManager > xImportStringResourceManager( xImportStringResource, UNO_QUERY );
                    LocalizationMgr::resetResourceForDialog( xDialogModel, xImportStringResourceManager );
                    bCopyResourcesForDialog = false;
                }

                if( bCopyResourcesForDialog )
                {
                    Reference< resource::XStringResourceResolver > xImportStringResourceResolver( xImportStringResource, UNO_QUERY );
                    LocalizationMgr::copyResourceForDroppedDialog( xDialogModel, aXmlDlgName,
                        xLibStringResourceManager, xImportStringResourceResolver );
                }
            }
            else if( bLibLocalized )
            {
                LocalizationMgr::setResourceIDsForDialog( xDialogModel, xLibStringResourceManager );
            }


            LocalizationMgr::setStringResourceAtDialog( rDocument, aLibName, aNewDlgName, xDialogModel );

            if( eNameClashMode == CLASH_OVERWRITE_DIALOG )
            {
                if ( BasicIDE::RemoveDialog( rDocument, aLibName, aNewDlgName ) )
                {
                    IDEBaseWindow* pDlgWin = pIDEShell->FindDlgWin( rDocument, aLibName, aNewDlgName, false, true );
                    if( pDlgWin != NULL )
                        pIDEShell->RemoveWindow( pDlgWin, true );
                    BasicIDE::MarkDocumentModified( rDocument );
                }
                else
                {
                    // TODO: Assertion?
                    return bDone;
                }
            }

            if( eNameClashMode == CLASH_RENAME_DIALOG )
            {
                bool bRenamed = false;
                if( xDialogModelPropSet.is() )
                {
                    try
                    {
                        Any aXmlDialogNameAny;
                        aXmlDialogNameAny <<= ::rtl::OUString( aNewDlgName );
                        xDialogModelPropSet->setPropertyValue( DLGED_PROP_NAME, aXmlDialogNameAny );
                        bRenamed = true;
                    }
                    catch(const beans::UnknownPropertyException& )
                    {}
                }


                if( bRenamed )
                {
                    LocalizationMgr::renameStringResourceIDs( rDocument, aLibName, aNewDlgName, xDialogModel );
                }
                else
                {
                    // TODO: Assertion?
                    return bDone;
                }
            }

            Reference< XInputStreamProvider > xISP = ::xmlscript::exportDialogModel( xDialogModel, xContext, rDocument.isDocument() ? rDocument.getDocument() : Reference< frame::XModel >() );
            bool bSuccess = rDocument.insertDialog( aLibName, aNewDlgName, xISP );
            if( bSuccess )
            {
                DialogWindow* pNewDlgWin = pIDEShell->CreateDlgWin( rDocument, aLibName, aNewDlgName );
                pIDEShell->SetCurWindow( pNewDlgWin, true );
            }

            bDone = true;
        }
        catch(const Exception& )
        {}
    }

    return bDone;
}

bool DialogWindow::ImportDialog()
{
    DBG_CHKTHIS( DialogWindow, 0 );

    const ScriptDocument& rDocument = GetDocument();
    ::rtl::OUString aLibName = GetLibName();
    return implImportDialog( this, aCurPath, rDocument, aLibName );
}

DlgEdModel* DialogWindow::GetModel() const
{
    return pEditor ? pEditor->GetModel() : NULL;
}

DlgEdPage* DialogWindow::GetPage() const
{
    return pEditor ? pEditor->GetPage() : NULL;
}

DlgEdView* DialogWindow::GetView() const
{
    return pEditor ? pEditor->GetView() : NULL;
}

sal_Bool DialogWindow::IsModified()
{
    return pEditor->IsModified();
}

::svl::IUndoManager* DialogWindow::GetUndoManager()
{
    return pUndoMgr;
}

::rtl::OUString DialogWindow::GetTitle()
{
    return GetName();
}

BasicEntryDescriptor DialogWindow::CreateEntryDescriptor()
{
    ScriptDocument aDocument( GetDocument() );
    ::rtl::OUString aLibName( GetLibName() );
    ::rtl::OUString aLibSubName;
    LibraryLocation eLocation = aDocument.getLibraryLocation( aLibName );
    return BasicEntryDescriptor( aDocument, eLocation, aLibName, aLibSubName, GetName(), OBJ_TYPE_DIALOG );
}

void DialogWindow::SetReadOnly( sal_Bool b )
{
    if ( pEditor )
    {
        if ( b )
            pEditor->SetMode( DLGED_READONLY );
        else
            pEditor->SetMode( DLGED_SELECT );
    }
}

sal_Bool DialogWindow::IsReadOnly()
{
    sal_Bool bReadOnly = sal_False;

    if ( pEditor && pEditor->GetMode() == DLGED_READONLY )
        bReadOnly = sal_True;

    return bReadOnly;
}

sal_Bool DialogWindow::IsPasteAllowed()
{
    return pEditor ? pEditor->IsPasteAllowed() : sal_False;
}

void DialogWindow::StoreData()
{
    if ( IsModified() )
    {
        try
        {
            Reference< container::XNameContainer > xLib = GetDocument().getLibrary( E_DIALOGS, GetLibName(), true );

            if( xLib.is() )
            {
                Reference< container::XNameContainer > xDialogModel = pEditor->GetDialog();

                if( xDialogModel.is() )
                {
                    Reference< XComponentContext > xContext;
                    Reference< beans::XPropertySet > xProps( ::comphelper::getProcessServiceFactory(), UNO_QUERY );
                    OSL_ASSERT( xProps.is() );
                    OSL_VERIFY( xProps->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DefaultContext")) ) >>= xContext );
                    Reference< XInputStreamProvider > xISP = ::xmlscript::exportDialogModel( xDialogModel, xContext, GetDocument().isDocument() ? GetDocument().getDocument() : Reference< frame::XModel >() );
                    xLib->replaceByName( ::rtl::OUString( GetName() ), makeAny( xISP ) );
                }
            }
        }
        catch (const uno::Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        BasicIDE::MarkDocumentModified( GetDocument() );
        pEditor->ClearModifyFlag();
    }
}

void DialogWindow::Deactivating()
{
    if ( IsModified() )
        BasicIDE::MarkDocumentModified( GetDocument() );
}

sal_Int32 DialogWindow::countPages( Printer* pPrinter )
{
    return pEditor->countPages( pPrinter );
}

void DialogWindow::printPage( sal_Int32 nPage, Printer* pPrinter )
{
    pEditor->printPage( nPage, pPrinter, CreateQualifiedName() );
}

void DialogWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    if( (rDCEvt.GetType()==DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        InitSettings( sal_True, sal_True, sal_True );
        Invalidate();
    }
    else
        IDEBaseWindow::DataChanged( rDCEvt );
}

void DialogWindow::InitSettings(sal_Bool bFont,sal_Bool bForeground,sal_Bool bBackground)
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

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > DialogWindow::CreateAccessible()
{
    return (::com::sun::star::accessibility::XAccessible*) new AccessibleDialogWindow( this );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
