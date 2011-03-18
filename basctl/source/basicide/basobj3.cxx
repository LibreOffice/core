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

#include <ide_pch.hxx>

#include <vector>
#include <algorithm>
#include <basic/sbx.hxx>
#include <unotools/moduleoptions.hxx>

#include <iderdll.hxx>
#include <iderdll2.hxx>
#include <basobj.hxx>
#include <basidesh.hxx>
#include <objdlg.hxx>
#include <bastypes.hxx>
#include <basdoc.hxx>
#include <basidesh.hrc>

#include <baside2.hxx>
#include <baside3.hxx>
#include <basicmod.hxx>
#include <localizationmgr.hxx>
#include "dlged.hxx"
#include <dlgeddef.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/script/XLibraryContainerPassword.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <xmlscript/xmldlg_imexp.hxx>
#include <rtl/uri.hxx>
#include <osl/process.h>
#include <osl/file.hxx>

using namespace comphelper;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;


//----------------------------------------------------------------------------

extern "C" {
    SAL_DLLPUBLIC_EXPORT long basicide_handle_basic_error( void* pPtr )
    {
        return BasicIDE::HandleBasicError( (StarBASIC*)pPtr );
    }
}

namespace BasicIDE
{
//----------------------------------------------------------------------------

SbMethod* CreateMacro( SbModule* pModule, const String& rMacroName )
{
    BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
    SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
    SfxDispatcher* pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;

    if( pDispatcher )
    {
        pDispatcher->Execute( SID_BASICIDE_STOREALLMODULESOURCES );
    }

    if ( pModule->GetMethods()->Find( rMacroName, SbxCLASS_METHOD ) )
        return 0;

    String aMacroName( rMacroName );
    if ( aMacroName.Len() == 0 )
    {
        if ( !pModule->GetMethods()->Count() )
            aMacroName = String( RTL_CONSTASCII_USTRINGPARAM( "Main" ) );
        else
        {
            sal_Bool bValid = sal_False;
            String aStdMacroText( RTL_CONSTASCII_USTRINGPARAM( "Macro" ) );
            sal_uInt16 nMacro = 1;
            while ( !bValid )
            {
                aMacroName = aStdMacroText;
                aMacroName += String::CreateFromInt32( nMacro );
                // Pruefen, ob vorhanden...
                bValid = pModule->GetMethods()->Find( aMacroName, SbxCLASS_METHOD ) ? sal_False : sal_True;
                nMacro++;
            }
        }
    }

    ::rtl::OUString aOUSource( pModule->GetSource32() );

    // Nicht zu viele Leerzeilen erzeugen...
    sal_Int32 nSourceLen = aOUSource.getLength();
    if ( nSourceLen > 2 )
    {
        const sal_Unicode* pStr = aOUSource.getStr();
        if ( pStr[ nSourceLen - 1 ]  != LINE_SEP )
            aOUSource += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "\n\n" ) );
        else if ( pStr[ nSourceLen - 2 ] != LINE_SEP )
            aOUSource += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "\n" ) );
        else if ( pStr[ nSourceLen - 3 ] == LINE_SEP )
            aOUSource = aOUSource.copy( 0, nSourceLen-1 );
    }

    ::rtl::OUString aSubStr;
    aSubStr = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Sub " ) );
    aSubStr += aMacroName;
    aSubStr += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "\n\nEnd Sub" ) );

    aOUSource += aSubStr;

    // update module in library
    ScriptDocument aDocument( ScriptDocument::NoDocument );
    SbxObject* pParent = pModule->GetParent();
    StarBASIC* pBasic = PTR_CAST(StarBASIC,pParent);
    DBG_ASSERT(pBasic, "BasicIDE::CreateMacro: No Basic found!");
    if ( pBasic )
    {
        BasicManager* pBasMgr = BasicIDE::FindBasicManager( pBasic );
        DBG_ASSERT(pBasMgr, "BasicIDE::CreateMacro: No BasicManager found!");
        if ( pBasMgr )
        {
            aDocument = ScriptDocument::getDocumentForBasicManager( pBasMgr );
            OSL_ENSURE( aDocument.isValid(), "BasicIDE::CreateMacro: no document for the given BasicManager!" );
            if ( aDocument.isValid() )
            {
                String aLibName = pBasic->GetName();
                String aModName = pModule->GetName();
                OSL_VERIFY( aDocument.updateModule( aLibName, aModName, aOUSource ) );
            }
        }
    }

    SbMethod* pMethod = (SbMethod*)pModule->GetMethods()->Find( aMacroName, SbxCLASS_METHOD );

    if( pDispatcher )
    {
        pDispatcher->Execute( SID_BASICIDE_UPDATEALLMODULESOURCES );
    }

    if ( aDocument.isAlive() )
        BasicIDE::MarkDocumentModified( aDocument );

    return pMethod;
}

//----------------------------------------------------------------------------

bool RenameDialog( Window* pErrorParent, const ScriptDocument& rDocument, const String& rLibName, const String& rOldName, const String& rNewName )
    throw(ElementExistException, NoSuchElementException)
{
    if ( !rDocument.hasDialog( rLibName, rOldName ) )
    {
        OSL_FAIL( "BasicIDE::RenameDialog: old module name is invalid!" );
        return false;
    }

    if ( rDocument.hasDialog( rLibName, rNewName ) )
    {
        ErrorBox aError( pErrorParent, WB_OK | WB_DEF_OK, String( IDEResId( RID_STR_SBXNAMEALLREADYUSED2 ) ) );
        aError.Execute();
        return false;
    }

    // #i74440
    if ( rNewName.Len() == 0 )
    {
        ErrorBox aError( pErrorParent, WB_OK | WB_DEF_OK, String( IDEResId( RID_STR_BADSBXNAME ) ) );
        aError.Execute();
        return false;
    }

    BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
    IDEBaseWindow* pWin = pIDEShell ? pIDEShell->FindWindow( rDocument, rLibName, rOldName, BASICIDE_TYPE_DIALOG, sal_False ) : NULL;
    Reference< XNameContainer > xExistingDialog;
    if ( pWin )
        xExistingDialog = ((DialogWindow*)pWin)->GetEditor()->GetDialog();

    if ( xExistingDialog.is() )
        LocalizationMgr::renameStringResourceIDs( rDocument, rLibName, rNewName, xExistingDialog );

    if ( !rDocument.renameDialog( rLibName, rOldName, rNewName, xExistingDialog ) )
        return false;

    if ( pWin )
    {
        // set new name in window
        pWin->SetName( rNewName );

        // update property browser
        ((DialogWindow*)pWin)->UpdateBrowser();

        // update tabwriter
        sal_uInt16 nId = (sal_uInt16)(pIDEShell->GetIDEWindowTable()).GetKey( pWin );
        DBG_ASSERT( nId, "No entry in Tabbar!" );
        if ( nId )
        {
            BasicIDETabBar* pTabBar = (BasicIDETabBar*)pIDEShell->GetTabBar();
            pTabBar->SetPageText( nId, rNewName );
            pTabBar->Sort();
            pTabBar->MakeVisible( pTabBar->GetCurPageId() );
        }
    }
    return true;
}

//----------------------------------------------------------------------------

bool RemoveDialog( const ScriptDocument& rDocument, const String& rLibName, const String& rDlgName )
{
    BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
    if ( pIDEShell )
    {
        DialogWindow* pDlgWin = pIDEShell->FindDlgWin( rDocument, rLibName, rDlgName, sal_False );
        if( pDlgWin )
        {
            Reference< container::XNameContainer > xDialogModel = pDlgWin->GetDialog();
            LocalizationMgr::removeResourceForDialog( rDocument, rLibName, rDlgName, xDialogModel );
        }
    }

    return rDocument.removeDialog( rLibName, rDlgName );
}

//----------------------------------------------------------------------------

StarBASIC* FindBasic( const SbxVariable* pVar )
{
    const SbxVariable* pSbx = pVar;
    while ( pSbx && !pSbx->ISA( StarBASIC ) )
        pSbx = pSbx->GetParent();

    DBG_ASSERT( !pSbx || pSbx->ISA( StarBASIC ), "Find Basic: Kein Basic!" );
    return (StarBASIC*)pSbx;
}

//----------------------------------------------------------------------------

BasicManager* FindBasicManager( StarBASIC* pLib )
{
    ScriptDocuments aDocuments( ScriptDocument::getAllScriptDocuments( ScriptDocument::AllWithApplication ) );
    for (   ScriptDocuments::const_iterator doc = aDocuments.begin();
            doc != aDocuments.end();
            ++doc
        )
    {
        BasicManager* pBasicMgr = doc->getBasicManager();
        OSL_ENSURE( pBasicMgr, "BasicIDE::FindBasicManager: no basic manager for the document!" );
        if ( !pBasicMgr )
            continue;

        Sequence< ::rtl::OUString > aLibNames( doc->getLibraryNames() );
        sal_Int32 nLibCount = aLibNames.getLength();
        const ::rtl::OUString* pLibNames = aLibNames.getConstArray();

        for ( sal_Int32 i = 0 ; i < nLibCount ; i++ )
        {
            StarBASIC* pL = pBasicMgr->GetLib( pLibNames[ i ] );
            if ( pL == pLib )
                return pBasicMgr;
        }
    }
    return NULL;
}

//----------------------------------------------------------------------------

void MarkDocumentModified( const ScriptDocument& rDocument )
{
    // Muss ja nicht aus einem Document kommen...
    if ( rDocument.isApplication() )
    {
        BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
        if ( pIDEShell )
            pIDEShell->SetAppBasicModified();
    }
    else
    {
        rDocument.setDocumentModified();
    }

    SfxBindings* pBindings = BasicIDE::GetBindingsPtr();
    if ( pBindings )
    {
        pBindings->Invalidate( SID_SIGNATURE );
        pBindings->Invalidate( SID_SAVEDOC );
        pBindings->Update( SID_SAVEDOC );
    }

    // Objectcatalog updaten...
    BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
    ObjectCatalog* pObjCatalog = pIDEShell ? pIDEShell->GetObjectCatalog() : 0;
    if ( pObjCatalog )
        pObjCatalog->UpdateEntries();
}

//----------------------------------------------------------------------------

void RunMethod( SbMethod* pMethod )
{
    SbxValues aRes;
    aRes.eType = SbxVOID;
    pMethod->Get( aRes );
}

//----------------------------------------------------------------------------

void StopBasic()
{
    StarBASIC::Stop();
    BasicIDEShell* pShell = IDE_DLL()->GetShell();
    if ( pShell )
    {
        IDEWindowTable& rWindows = pShell->GetIDEWindowTable();
        IDEBaseWindow* pWin = rWindows.First();
        while ( pWin )
        {
            // BasicStopped von Hand rufen, da das Stop-Notify ggf. sonst nicht
            // durchkommen kann.
            pWin->BasicStopped();
            pWin = rWindows.Next();
        }
    }
    BasicIDE::BasicStopped();
}

//----------------------------------------------------------------------------

void BasicStopped( sal_Bool* pbAppWindowDisabled,
        sal_Bool* pbDispatcherLocked, sal_uInt16* pnWaitCount,
        SfxUInt16Item** ppSWActionCount, SfxUInt16Item** ppSWLockViewCount )
{
    // Nach einem Error oder dem expliziten abbrechen des Basics muessen
    // ggf. einige Locks entfernt werden...

    if ( pbAppWindowDisabled )
        *pbAppWindowDisabled = sal_False;
    if ( pbDispatcherLocked )
        *pbDispatcherLocked = sal_False;
    if ( pnWaitCount )
        *pnWaitCount = 0;
    if ( ppSWActionCount )
        *ppSWActionCount = 0;
    if ( ppSWLockViewCount )
        *ppSWLockViewCount = 0;

    // AppWait ?
    BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
    if( pIDEShell )
    {
        sal_uInt16 nWait = 0;
        while ( pIDEShell->GetViewFrame()->GetWindow().IsWait() )
        {
            pIDEShell->GetViewFrame()->GetWindow().LeaveWait();
            nWait++;
        }
        if ( pnWaitCount )
            *pnWaitCount = nWait;
    }

    Window* pDefParent = Application::GetDefDialogParent();
    if ( pDefParent && !pDefParent->IsEnabled() )
    {
        pDefParent->Enable( sal_True );
        if ( pbAppWindowDisabled )
            *pbAppWindowDisabled = sal_True;
    }

}

//----------------------------------------------------------------------------

void InvalidateDebuggerSlots()
{
    SfxBindings* pBindings = BasicIDE::GetBindingsPtr();
    if ( pBindings )
    {
        pBindings->Invalidate( SID_BASICSTOP );
        pBindings->Update( SID_BASICSTOP );
        pBindings->Invalidate( SID_BASICRUN );
        pBindings->Update( SID_BASICRUN );
        pBindings->Invalidate( SID_BASICCOMPILE );
        pBindings->Update( SID_BASICCOMPILE );
        pBindings->Invalidate( SID_BASICSTEPOVER );
        pBindings->Update( SID_BASICSTEPOVER );
        pBindings->Invalidate( SID_BASICSTEPINTO );
        pBindings->Update( SID_BASICSTEPINTO );
        pBindings->Invalidate( SID_BASICSTEPOUT );
        pBindings->Update( SID_BASICSTEPOUT );
        pBindings->Invalidate( SID_BASICIDE_TOGGLEBRKPNT );
        pBindings->Update( SID_BASICIDE_TOGGLEBRKPNT );
        pBindings->Invalidate( SID_BASICIDE_STAT_POS );
        pBindings->Update( SID_BASICIDE_STAT_POS );
    }
}

//----------------------------------------------------------------------------

long HandleBasicError( StarBASIC* pBasic )
{
    BasicIDEDLL::Init();
    BasicIDE::BasicStopped();

    // no error output during macro choosing
    if ( IDE_DLL()->GetExtraData()->ChoosingMacro() )
        return 1;
    if ( IDE_DLL()->GetExtraData()->ShellInCriticalSection() )
        return 2;

    long nRet = 0;
    BasicIDEShell* pIDEShell = 0;
    if ( SvtModuleOptions().IsBasicIDE() )
    {
        BasicManager* pBasMgr = BasicIDE::FindBasicManager( pBasic );
        if ( pBasMgr )
        {
            sal_Bool bProtected = sal_False;
            ScriptDocument aDocument( ScriptDocument::getDocumentForBasicManager( pBasMgr ) );
            OSL_ENSURE( aDocument.isValid(), "BasicIDE::HandleBasicError: no document for the given BasicManager!" );
            if ( aDocument.isValid() )
            {
                ::rtl::OUString aOULibName( pBasic->GetName() );
                Reference< script::XLibraryContainer > xModLibContainer( aDocument.getLibraryContainer( E_SCRIPTS ) );
                if ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) )
                {
                    Reference< script::XLibraryContainerPassword > xPasswd( xModLibContainer, UNO_QUERY );
                    if ( xPasswd.is() && xPasswd->isLibraryPasswordProtected( aOULibName ) && !xPasswd->isLibraryPasswordVerified( aOULibName ) )
                    {
                        bProtected = sal_True;
                    }
                }
            }

            if ( !bProtected )
            {
                pIDEShell = IDE_DLL()->GetShell();
                if ( !pIDEShell )
                {
                    SfxAllItemSet aArgs( SFX_APP()->GetPool() );
                    SfxRequest aRequest( SID_BASICIDE_APPEAR, SFX_CALLMODE_SYNCHRON, aArgs );
                    SFX_APP()->ExecuteSlot( aRequest );
                    pIDEShell = IDE_DLL()->GetShell();
                }
            }
        }
    }

    if ( pIDEShell )
        nRet = pIDEShell->CallBasicErrorHdl( pBasic );
    else
        ErrorHandler::HandleError( StarBASIC::GetErrorCode() );

    return nRet;
}

//----------------------------------------------------------------------------

SfxBindings* GetBindingsPtr()
{
    SfxBindings* pBindings = NULL;

    SfxViewFrame* pFrame = NULL;
    BasicIDEDLL* pIDEDLL = IDE_DLL();
    if ( pIDEDLL && pIDEDLL->GetShell() )
    {
        pFrame = pIDEDLL->GetShell()->GetViewFrame();
    }
    else
    {
        SfxViewFrame* pView = SfxViewFrame::GetFirst();
        while ( pView )
        {
            SfxObjectShell* pObjShell = pView->GetObjectShell();
            if ( pObjShell && pObjShell->ISA( BasicDocShell ) )
            {
                pFrame = pView;
                break;
            }
            pView = SfxViewFrame::GetNext( *pView );
        }
    }
    if ( pFrame != NULL )
        pBindings = &pFrame->GetBindings();

    return pBindings;
}

} //namespace BasicIDE

//----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
