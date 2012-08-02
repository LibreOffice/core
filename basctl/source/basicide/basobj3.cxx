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

#include <vcl/msgbox.hxx>
#include <vector>
#include <algorithm>
#include <basic/basmgr.hxx>
#include <basic/sbmeth.hxx>
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
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
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
    BasicIDEShell* pIDEShell = BasicIDEGlobals::GetShell();
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
            bool bValid = false;
            String aStdMacroText( RTL_CONSTASCII_USTRINGPARAM( "Macro" ) );
            sal_uInt16 nMacro = 1;
            while ( !bValid )
            {
                aMacroName = aStdMacroText;
                aMacroName += String::CreateFromInt32( nMacro );
                // test whether existing...
                bValid = pModule->GetMethods()->Find( aMacroName, SbxCLASS_METHOD ) ? false : true;
                nMacro++;
            }
        }
    }

    ::rtl::OUString aOUSource( pModule->GetSource32() );

    // don't produce too many empty lines...
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

bool RenameDialog( Window* pErrorParent, const ScriptDocument& rDocument, const ::rtl::OUString& rLibName, const ::rtl::OUString& rOldName, const ::rtl::OUString& rNewName )
    throw(ElementExistException, NoSuchElementException)
{
    if ( !rDocument.hasDialog( rLibName, rOldName ) )
    {
        OSL_FAIL( "BasicIDE::RenameDialog: old module name is invalid!" );
        return false;
    }

    if ( rDocument.hasDialog( rLibName, rNewName ) )
    {
        ErrorBox aError( pErrorParent, WB_OK | WB_DEF_OK, IDE_RESSTR(RID_STR_SBXNAMEALLREADYUSED2) );
        aError.Execute();
        return false;
    }

    // #i74440
    if ( rNewName.isEmpty() )
    {
        ErrorBox aError( pErrorParent, WB_OK | WB_DEF_OK, IDE_RESSTR(RID_STR_BADSBXNAME) );
        aError.Execute();
        return false;
    }

    BasicIDEShell* pIDEShell = BasicIDEGlobals::GetShell();
    IDEBaseWindow* pWin = pIDEShell ? pIDEShell->FindWindow( rDocument, rLibName, rOldName, BASICIDE_TYPE_DIALOG, false ) : NULL;
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
        sal_uInt16 nId = pIDEShell->GetIDEWindowId( pWin );
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

bool RemoveDialog( const ScriptDocument& rDocument, const ::rtl::OUString& rLibName, const ::rtl::OUString& rDlgName )
{
    BasicIDEShell* pIDEShell = BasicIDEGlobals::GetShell();
    if ( pIDEShell )
    {
        DialogWindow* pDlgWin = pIDEShell->FindDlgWin( rDocument, rLibName, rDlgName, false );
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
    // does not have to come from a document...
    if ( rDocument.isApplication() )
    {
        BasicIDEShell* pIDEShell = BasicIDEGlobals::GetShell();
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
    BasicIDEShell* pIDEShell = BasicIDEGlobals::GetShell();
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
    BasicIDEShell* pShell = BasicIDEGlobals::GetShell();
    if ( pShell )
    {
        IDEWindowTable& rWindows = pShell->GetIDEWindowTable();
        for( IDEWindowTable::const_iterator it = rWindows.begin(); it != rWindows.end(); ++it )
        {
            IDEBaseWindow* pWin = it->second;
            // call BasicStopped manually because the Stop-Notify
            // might not get through otherwise
            pWin->BasicStopped();
        }
    }
    BasicIDE::BasicStopped();
}

//----------------------------------------------------------------------------

void BasicStopped( bool* pbAppWindowDisabled,
        bool* pbDispatcherLocked, sal_uInt16* pnWaitCount,
        SfxUInt16Item** ppSWActionCount, SfxUInt16Item** ppSWLockViewCount )
{
    // maybe there are some locks to be removed after an error
    // or an explicit cancelling of the basic...

    if ( pbAppWindowDisabled )
        *pbAppWindowDisabled = false;
    if ( pbDispatcherLocked )
        *pbDispatcherLocked = false;
    if ( pnWaitCount )
        *pnWaitCount = 0;
    if ( ppSWActionCount )
        *ppSWActionCount = 0;
    if ( ppSWLockViewCount )
        *ppSWLockViewCount = 0;

    // AppWait ?
    BasicIDEShell* pIDEShell = BasicIDEGlobals::GetShell();
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
            *pbAppWindowDisabled = true;
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
    BasicIDEGlobals::ensure();
    BasicIDE::BasicStopped();

    // no error output during macro choosing
    if ( BasicIDEGlobals::GetExtraData()->ChoosingMacro() )
        return 1;
    if ( BasicIDEGlobals::GetExtraData()->ShellInCriticalSection() )
        return 2;

    long nRet = 0;
    BasicIDEShell* pIDEShell = 0;
    if ( SvtModuleOptions().IsBasicIDE() )
    {
        BasicManager* pBasMgr = BasicIDE::FindBasicManager( pBasic );
        if ( pBasMgr )
        {
            bool bProtected = false;
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
                        bProtected = true;
                    }
                }
            }

            if ( !bProtected )
            {
                pIDEShell = BasicIDEGlobals::GetShell();
                if ( !pIDEShell )
                {
                    SfxAllItemSet aArgs( SFX_APP()->GetPool() );
                    SfxRequest aRequest( SID_BASICIDE_APPEAR, SFX_CALLMODE_SYNCHRON, aArgs );
                    SFX_APP()->ExecuteSlot( aRequest );
                    pIDEShell = BasicIDEGlobals::GetShell();
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
    if ( BasicIDEGlobals::GetShell() )
    {
        pFrame = BasicIDEGlobals::GetShell()->GetViewFrame();
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
