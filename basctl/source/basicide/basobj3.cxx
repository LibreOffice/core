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

namespace basctl
{

using namespace comphelper;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;


//----------------------------------------------------------------------------

extern "C" {
    SAL_DLLPUBLIC_EXPORT long basicide_handle_basic_error( void* pPtr )
    {
        return HandleBasicError( (StarBASIC*)pPtr );
    }
}

//----------------------------------------------------------------------------

SbMethod* CreateMacro( SbModule* pModule, const String& rMacroName )
{
    SfxDispatcher* pDispatcher = GetDispatcher();
    if( pDispatcher )
    {
        pDispatcher->Execute( SID_BASICIDE_STOREALLMODULESOURCES );
    }

    if ( pModule->GetMethods()->Find( rMacroName, SbxCLASS_METHOD ) )
        return 0;

    OUString aMacroName( rMacroName );
    if ( aMacroName.isEmpty() )
    {
        if ( !pModule->GetMethods()->Count() )
            aMacroName = "Main" ;
        else
        {
            bool bValid = false;
            OUString aStdMacroText( "Macro" );
            sal_Int32 nMacro = 1;
            while ( !bValid )
            {
                aMacroName = aStdMacroText;
                aMacroName += OUString::valueOf( nMacro );
                // test whether existing...
                bValid = pModule->GetMethods()->Find( aMacroName, SbxCLASS_METHOD ) ? false : true;
                nMacro++;
            }
        }
    }

    OUString aOUSource( pModule->GetSource32() );

    // don't produce too many empty lines...
    sal_Int32 nSourceLen = aOUSource.getLength();
    if ( nSourceLen > 2 )
    {
        const sal_Unicode* pStr = aOUSource.getStr();
        if ( pStr[ nSourceLen - 1 ]  != LINE_SEP )
            aOUSource += "\n\n" ;
        else if ( pStr[ nSourceLen - 2 ] != LINE_SEP )
            aOUSource += "\n" ;
        else if ( pStr[ nSourceLen - 3 ] == LINE_SEP )
            aOUSource = aOUSource.copy( 0, nSourceLen-1 );
    }

    OUString aSubStr;
    aSubStr = "Sub " ;
    aSubStr += aMacroName;
    aSubStr += "\n\nEnd Sub" ;

    aOUSource += aSubStr;

    // update module in library
    ScriptDocument aDocument( ScriptDocument::NoDocument );
    StarBASIC* pBasic = dynamic_cast<StarBASIC*>(pModule->GetParent());
    DBG_ASSERT(pBasic, "basctl::CreateMacro: No Basic found!");
    if ( pBasic )
    {
        BasicManager* pBasMgr = FindBasicManager( pBasic );
        DBG_ASSERT(pBasMgr, "basctl::CreateMacro: No BasicManager found!");
        if ( pBasMgr )
        {
            aDocument = ScriptDocument::getDocumentForBasicManager( pBasMgr );
            OSL_ENSURE( aDocument.isValid(), "basctl::CreateMacro: no document for the given BasicManager!" );
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
        MarkDocumentModified( aDocument );

    return pMethod;
}

//----------------------------------------------------------------------------

bool RenameDialog (
    Window* pErrorParent,
    ScriptDocument const& rDocument,
    OUString const& rLibName,
    OUString const& rOldName,
    OUString const& rNewName
)
    throw(ElementExistException, NoSuchElementException)
{
    if ( !rDocument.hasDialog( rLibName, rOldName ) )
    {
        OSL_FAIL( "basctl::RenameDialog: old module name is invalid!" );
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

    Shell* pShell = GetShell();
    DialogWindow* pWin = pShell ? pShell->FindDlgWin(rDocument, rLibName, rOldName) : 0;
    Reference< XNameContainer > xExistingDialog;
    if ( pWin )
        xExistingDialog = pWin->GetEditor().GetDialog();

    if ( xExistingDialog.is() )
        LocalizationMgr::renameStringResourceIDs( rDocument, rLibName, rNewName, xExistingDialog );

    if ( !rDocument.renameDialog( rLibName, rOldName, rNewName, xExistingDialog ) )
        return false;

    if ( pWin )
    {
        // set new name in window
        pWin->SetName( rNewName );

        // update property browser
        pWin->UpdateBrowser();

        // update tabwriter
        sal_uInt16 nId = pShell->GetWindowId( pWin );
        DBG_ASSERT( nId, "No entry in Tabbar!" );
        if ( nId )
        {
            TabBar& rTabBar = pShell->GetTabBar();
            rTabBar.SetPageText( nId, rNewName );
            rTabBar.Sort();
            rTabBar.MakeVisible( rTabBar.GetCurPageId() );
        }
    }
    return true;
}

//----------------------------------------------------------------------------

bool RemoveDialog( const ScriptDocument& rDocument, const OUString& rLibName, const OUString& rDlgName )
{
    if (Shell* pShell = GetShell())
    {
        if (DialogWindow* pDlgWin = pShell->FindDlgWin(rDocument, rLibName, rDlgName))
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
    SbxVariable const* pSbx = pVar;
    while (pSbx && !dynamic_cast<StarBASIC const*>(pSbx))
        pSbx = pSbx->GetParent();
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
        OSL_ENSURE( pBasicMgr, "basctl::FindBasicManager: no basic manager for the document!" );
        if ( !pBasicMgr )
            continue;

        Sequence< OUString > aLibNames( doc->getLibraryNames() );
        sal_Int32 nLibCount = aLibNames.getLength();
        const OUString* pLibNames = aLibNames.getConstArray();

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
        if (Shell* pShell = GetShell())
        {
            pShell->SetAppBasicModified();
            pShell->UpdateObjectCatalog();
        }
    }
    else
    {
        rDocument.setDocumentModified();
    }

    if (SfxBindings* pBindings = GetBindingsPtr())
    {
        pBindings->Invalidate( SID_SIGNATURE );
        pBindings->Invalidate( SID_SAVEDOC );
        pBindings->Update( SID_SAVEDOC );
    }
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
    if (Shell* pShell = GetShell())
    {
        Shell::WindowTable& rWindows = pShell->GetWindowTable();
        for (Shell::WindowTableIt it = rWindows.begin(); it != rWindows.end(); ++it )
        {
            BaseWindow* pWin = it->second;
            // call BasicStopped manually because the Stop-Notify
            // might not get through otherwise
            pWin->BasicStopped();
        }
    }
    BasicStopped();
}

//----------------------------------------------------------------------------

void BasicStopped(
    bool* pbAppWindowDisabled,
    bool* pbDispatcherLocked,
    sal_uInt16* pnWaitCount,
    SfxUInt16Item** ppSWActionCount, SfxUInt16Item** ppSWLockViewCount
)
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
    if (Shell* pShell = GetShell())
    {
        sal_uInt16 nWait = 0;
        while ( pShell->GetViewFrame()->GetWindow().IsWait() )
        {
            pShell->GetViewFrame()->GetWindow().LeaveWait();
            nWait++;
        }
        if ( pnWaitCount )
            *pnWaitCount = nWait;
    }

    Window* pDefParent = Application::GetDefDialogParent();
    if ( pDefParent && !pDefParent->IsEnabled() )
    {
        pDefParent->Enable(true);
        if ( pbAppWindowDisabled )
            *pbAppWindowDisabled = true;
    }

}

//----------------------------------------------------------------------------

void InvalidateDebuggerSlots()
{
    if (SfxBindings* pBindings = GetBindingsPtr())
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
    EnsureIde();
    BasicStopped();

    // no error output during macro choosing
    if (GetExtraData()->ChoosingMacro())
        return 1;
    if (GetExtraData()->ShellInCriticalSection())
        return 2;

    long nRet = 0;
    Shell* pShell = 0;
    if ( SvtModuleOptions().IsBasicIDE() )
    {
        BasicManager* pBasMgr = FindBasicManager( pBasic );
        if ( pBasMgr )
        {
            bool bProtected = false;
            ScriptDocument aDocument( ScriptDocument::getDocumentForBasicManager( pBasMgr ) );
            OSL_ENSURE( aDocument.isValid(), "basctl::HandleBasicError: no document for the given BasicManager!" );
            if ( aDocument.isValid() )
            {
                OUString aOULibName( pBasic->GetName() );
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
                pShell = GetShell();
                if ( !pShell )
                {
                    SfxAllItemSet aArgs( SFX_APP()->GetPool() );
                    SfxRequest aRequest( SID_BASICIDE_APPEAR, SFX_CALLMODE_SYNCHRON, aArgs );
                    SFX_APP()->ExecuteSlot( aRequest );
                    pShell = GetShell();
                }
            }
        }
    }

    if ( pShell )
        nRet = pShell->CallBasicErrorHdl( pBasic );
    else
        ErrorHandler::HandleError( StarBASIC::GetErrorCode() );

    return nRet;
}

//----------------------------------------------------------------------------

SfxBindings* GetBindingsPtr()
{
    SfxBindings* pBindings = NULL;

    SfxViewFrame* pFrame = NULL;
    if (Shell* pShell = GetShell())
    {
        pFrame = pShell->GetViewFrame();
    }
    else
    {
        SfxViewFrame* pView = SfxViewFrame::GetFirst();
        while ( pView )
        {
            if (dynamic_cast<DocShell*>(pView->GetObjectShell()))
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

//----------------------------------------------------------------------------

SfxDispatcher* GetDispatcher ()
{
    if (Shell* pShell = GetShell())
        if (SfxViewFrame* pViewFrame = pShell->GetViewFrame())
            if (SfxDispatcher* pDispatcher = pViewFrame->GetDispatcher())
                return pDispatcher;
    return 0;
}

//----------------------------------------------------------------------------
} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
