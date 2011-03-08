/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basctl.hxx"

#define _SDR_NOITEMS
#define _SDR_NOTOUCH
#define _SDR_NOTRANSFORM
#define _SDR_NOOBJECTS
#define _SDR_NOVIEWMARKER
#define _SDR_NODRAGMETHODS
#define _SDR_NOXOUTDEV

#include <ide_pch.hxx>


#include <vector>
#include <basidesh.hrc>
#include <baside3.hxx>
#include <localizationmgr.hxx>
#include <accessibledialogwindow.hxx>
#include <dlged.hxx>
#include <dlgedmod.hxx>
#include <dlgedview.hxx>
#include <dlgeddef.hxx>
#include <propbrw.hxx>

#include <basobj.hxx>
#include <iderdll.hxx>
#include <basidesh.hxx>
#include <idetemp.hxx>
#include <helpid.hrc>
#include <bastype2.hxx>
#include <svx/svdview.hxx>
#include <editeng/unolingu.hxx>
#include <tools/diagnose_ex.h>
#include <tools/urlobj.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <xmlscript/xmldlg_imexp.hxx>
#include <com/sun/star/script/XLibraryContainer2.hpp>
#include <svtools/ehdl.hxx>
#include <svtools/langtab.hxx>
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/resource/XStringResourceResolver.hpp>
#include <com/sun/star/resource/StringResourceWithLocation.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/script/vba/XVBACompatibility.hpp>

using namespace comphelper;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::resource;
using namespace ::com::sun::star::ui::dialogs;

#if defined(MAC)
#define FILTERMASK_ALL "****"
#elif defined(OW) || defined(MTF)
#define FILTERMASK_ALL "*"
#elif defined(PM2)
#define FILTERMASK_ALL ""
#else
#define FILTERMASK_ALL "*.*"
#endif

DBG_NAME( DialogWindow )

TYPEINIT1( DialogWindow, IDEBaseWindow );

DialogWindow::DialogWindow( Window* pParent, const ScriptDocument& rDocument, String aLibName, String aName,
    const com::sun::star::uno::Reference< com::sun::star::container::XNameContainer >& xDialogModel )
        :IDEBaseWindow( pParent, rDocument, aLibName, aName )
        ,pUndoMgr(NULL)
{
    InitSettings( TRUE, TRUE, TRUE );

    pEditor = new DlgEditor( rDocument.isDocument() ? rDocument.getDocument() : Reference< frame::XModel >() );
    pEditor->SetWindow( this );
    pEditor->SetDialog( xDialogModel );

    // Undo einrichten
    pUndoMgr = new SfxUndoManager;

    Link aDummyLink;
    aOldNotifyUndoActionHdl = pEditor->GetModel()->GetNotifyUndoActionHdl();
    pEditor->GetModel()->SetNotifyUndoActionHdl(
        LINK(this, DialogWindow, NotifyUndoActionHdl));

    SetHelpId( HID_BASICIDE_DIALOGWINDOW );

    // set readonly mode for readonly libraries
    ::rtl::OUString aOULibName( aLibName );
    Reference< script::XLibraryContainer2 > xDlgLibContainer( GetDocument().getLibraryContainer( E_DIALOGS ), UNO_QUERY );
    if ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aOULibName ) && xDlgLibContainer->isLibraryReadOnly( aOULibName ) )
        SetReadOnly( TRUE );

    if ( rDocument.isDocument() && rDocument.isReadOnly() )
        SetReadOnly( TRUE );
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
        BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
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
    for ( USHORT nWh = aIter.FirstWhich(); 0 != nWh; nWh = aIter.NextWhich() )
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
                        USHORT nObj;
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
                BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
                SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
                if ( pViewFrame && !pViewFrame->HasChildWindow( SID_SHOW_PROPERTYBROWSER ) && !pEditor->GetView()->AreObjectsMarked() )
                    rSet.DisableItem( nWh );

                if ( IsReadOnly() )
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

BOOL DialogWindow::RenameDialog( const String& rNewName )
{
    if ( !BasicIDE::RenameDialog( this, GetDocument(), GetLibName(), GetName(), rNewName ) )
        return FALSE;

    SfxBindings* pBindings = BasicIDE::GetBindingsPtr();
    if ( pBindings )
        pBindings->Invalidate( SID_DOC_MODIFIED );

    return TRUE;
}

void DialogWindow::DisableBrowser()
{
    BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
    SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
    SfxChildWindow* pChildWin = pViewFrame ? pViewFrame->GetChildWindow(SID_SHOW_PROPERTYBROWSER) : NULL;
    if( pChildWin )
        ((PropBrw*)(pChildWin->GetWindow()))->Update( NULL );
}

void DialogWindow::UpdateBrowser()
{
    BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
    SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
    SfxChildWindow* pChildWin = pViewFrame ? pViewFrame->GetChildWindow(SID_SHOW_PROPERTYBROWSER) : NULL;
    if( pChildWin )
        ((PropBrw*)(pChildWin->GetWindow()))->Update( pIDEShell );
}

static ::rtl::OUString aResourceResolverPropName( RTL_CONSTASCII_USTRINGPARAM( "ResourceResolver" ));

BOOL DialogWindow::SaveDialog()
{
    DBG_CHKTHIS( DialogWindow, 0 );
    BOOL bDone = FALSE;

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

    if ( aCurPath.Len() )
        xFP->setDisplayDirectory ( aCurPath );

    xFP->setDefaultName( ::rtl::OUString( GetName() ) );

    String aDialogStr( IDEResId( RID_STR_STDDIALOGNAME ) );
    Reference< XFilterManager > xFltMgr(xFP, UNO_QUERY);
    xFltMgr->appendFilter( aDialogStr, String( RTL_CONSTASCII_USTRINGPARAM( "*.xdl" ) ) );
    xFltMgr->appendFilter( String( IDEResId( RID_STR_FILTER_ALLFILES ) ), String( RTL_CONSTASCII_USTRINGPARAM( FILTERMASK_ALL ) ) );
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

        Reference< XSimpleFileAccess > xSFI( xMSF->createInstance
            ( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ucb.SimpleFileAccess" )) ), UNO_QUERY );

        Reference< XOutputStream > xOutput;
        try
        {
            if( xSFI->exists( aCurPath ) )
                xSFI->kill( aCurPath );
            xOutput = xSFI->openFileWrite( aCurPath );
        }
        catch( Exception& )
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
                catch( beans::UnknownPropertyException& )
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

                        if( aExtension.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "properties" ) ) ||
                            aExtension.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "default" ) ) )
                        {
                            if( aPureName.indexOf( aDialogName_ ) == 0 )
                            {
                                try
                                {
                                    xSFI->kill( aCompleteName );
                                }
                                catch( uno::Exception& )
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
            ErrorBox( this, WB_OK | WB_DEF_OK, String( IDEResId( RID_STR_COULDNTWRITE) ) ).Execute();
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

    AddButton( String( IDEResId( RID_STR_DLGIMP_CLASH_RENAME ) ), RET_YES,
        BUTTONDIALOG_DEFBUTTON | BUTTONDIALOG_OKBUTTON | BUTTONDIALOG_FOCUSBUTTON );
    AddButton( String( IDEResId( RID_STR_DLGIMP_CLASH_REPLACE ) ), RET_NO, 0 );
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
    AddButton( String( IDEResId( RID_STR_DLGIMP_MISMATCH_ADD ) ), RET_YES,
        BUTTONDIALOG_DEFBUTTON | BUTTONDIALOG_OKBUTTON | BUTTONDIALOG_FOCUSBUTTON );
    AddButton( String( IDEResId( RID_STR_DLGIMP_MISMATCH_OMIT ) ), RET_NO, 0 );
    AddButton( BUTTON_CANCEL, RET_CANCEL, BUTTONDIALOG_CANCELBUTTON );
    AddButton( BUTTON_HELP, BUTTONID_HELP, BUTTONDIALOG_HELPBUTTON, 4 );

    SetImage( QueryBox::GetStandardImage() );
}

BOOL implImportDialog( Window* pWin, const String& rCurPath, const ScriptDocument& rDocument, const String& aLibName )
{
    BOOL bDone = FALSE;

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

    String aCurPath( rCurPath );
    if ( aCurPath.Len() )
        xFP->setDisplayDirectory ( aCurPath );

    String aDialogStr( IDEResId( RID_STR_STDDIALOGNAME ) );
    Reference< XFilterManager > xFltMgr(xFP, UNO_QUERY);
    xFltMgr->appendFilter( aDialogStr, String( RTL_CONSTASCII_USTRINGPARAM( "*.xdl" ) ) );
    xFltMgr->appendFilter( String( IDEResId( RID_STR_FILTER_ALLFILES ) ), String( RTL_CONSTASCII_USTRINGPARAM( FILTERMASK_ALL ) ) );
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

            Reference< XSimpleFileAccess > xSFI( xMSF->createInstance
                ( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ucb.SimpleFileAccess" )) ), UNO_QUERY_THROW );

            Reference< XInputStream > xInput;
            if( xSFI->exists( aCurPath ) )
                xInput = xSFI->openFileRead( aCurPath );

            Reference< XComponentContext > xContext;
            Reference< beans::XPropertySet > xProps( xMSF, UNO_QUERY );
            OSL_ASSERT( xProps.is() );
            OSL_VERIFY( xProps->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DefaultContext")) ) >>= xContext );
            ::xmlscript::importDialogModel( xInput, xDialogModel, xContext, rDocument.isDocument() ? rDocument.getDocument() : Reference< frame::XModel >() );

            String aXmlDlgName;
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
                catch( beans::UnknownPropertyException& )
                {}
            }
            bool bValidName = (aXmlDlgName.Len() != 0);
            OSL_ASSERT( bValidName );
            if( !bValidName )
                return bDone;

            bool bDialogAlreadyExists = rDocument.hasDialog( aLibName, aXmlDlgName );

            String aNewDlgName = aXmlDlgName;
            enum NameClashMode
            {
                NO_CLASH,
                CLASH_OVERWRITE_DIALOG,
                CLASH_RENAME_DIALOG,
            };
            NameClashMode eNameClashMode = NO_CLASH;
            if( bDialogAlreadyExists )
            {
                String aQueryBoxTitle( IDEResId( RID_STR_DLGIMP_CLASH_TITLE ) );
                String aQueryBoxText(  IDEResId( RID_STR_DLGIMP_CLASH_TEXT  ) );
                aQueryBoxText.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "$(ARG1)" ) ), aXmlDlgName );

                NameClashQueryBox aQueryBox( pWin, aQueryBoxTitle, aQueryBoxText );
                USHORT nRet = aQueryBox.Execute();
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

            BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
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

            Reference< container::XNameContainer > xDialogLib( rDocument.getLibrary( E_DIALOGS, aLibName, TRUE ) );
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
                String aQueryBoxTitle( IDEResId( RID_STR_DLGIMP_MISMATCH_TITLE ) );
                String aQueryBoxText( IDEResId( RID_STR_DLGIMP_MISMATCH_TEXT ) );
                LanguageMismatchQueryBox aQueryBox( pWin, aQueryBoxTitle, aQueryBoxText );
                USHORT nRet = aQueryBox.Execute();
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
                    LocalizationMgr* pCurMgr = pIDEShell->GetCurLocalizationMgr();

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
                    IDEBaseWindow* pDlgWin = pIDEShell->FindDlgWin( rDocument, aLibName, aNewDlgName, FALSE, TRUE );
                    if( pDlgWin != NULL )
                        pIDEShell->RemoveWindow( pDlgWin, TRUE );
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
                    catch( beans::UnknownPropertyException& )
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
                pIDEShell->SetCurWindow( pNewDlgWin, TRUE );
            }

            bDone = TRUE;
        }
        catch( Exception& )
        {}
    }

    return bDone;
}

BOOL DialogWindow::ImportDialog()
{
    DBG_CHKTHIS( DialogWindow, 0 );

    const ScriptDocument& rDocument = GetDocument();
    String aLibName = GetLibName();
    BOOL bRet = implImportDialog( this, aCurPath, rDocument, aLibName );
    return bRet;
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

BOOL DialogWindow::IsModified()
{
    return pEditor->IsModified();
}

SfxUndoManager* DialogWindow::GetUndoManager()
{
    return pUndoMgr;
}

String DialogWindow::GetTitle()
{
    return GetName();
}

BasicEntryDescriptor DialogWindow::CreateEntryDescriptor()
{
    ScriptDocument aDocument( GetDocument() );
    String aLibName( GetLibName() );
    String aLibSubName;
    LibraryLocation eLocation = aDocument.getLibraryLocation( aLibName );
    return BasicEntryDescriptor( aDocument, eLocation, aLibName, aLibSubName, GetName(), OBJ_TYPE_DIALOG );
}

void DialogWindow::SetReadOnly( BOOL b )
{
    if ( pEditor )
    {
        if ( b )
            pEditor->SetMode( DLGED_READONLY );
        else
            pEditor->SetMode( DLGED_SELECT );
    }
}

BOOL DialogWindow::IsReadOnly()
{
    BOOL bReadOnly = FALSE;

    if ( pEditor && pEditor->GetMode() == DLGED_READONLY )
        bReadOnly = TRUE;

    return bReadOnly;
}

BOOL DialogWindow::IsPasteAllowed()
{
    return pEditor ? pEditor->IsPasteAllowed() : FALSE;
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
        catch ( uno::Exception& )
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

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > DialogWindow::CreateAccessible()
{
    return (::com::sun::star::accessibility::XAccessible*) new AccessibleDialogWindow( this );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
