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

#include "iderdll.hxx"
#include "iderdll2.hxx"
#include "macrodlg.hxx"
#include "moduldlg.hxx"
#include "basidesh.hrc"
#include "baside2.hxx"

#include <com/sun/star/document/XScriptInvocationContext.hpp>

#include <basic/sbmeth.hxx>
#include <framework/documentundoguard.hxx>
#include <tools/diagnose_ex.h>
#include <unotools/moduleoptions.hxx>

#include <vector>
#include <algorithm>
#include <basic/basmgr.hxx>
namespace basctl
{

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;

extern "C" {
    SAL_DLLPUBLIC_EXPORT rtl_uString* basicide_choose_macro( void* pOnlyInDocument_AsXModel, sal_Bool bChooseOnly, rtl_uString* pMacroDesc )
    {
        OUString aMacroDesc( pMacroDesc );
        Reference< frame::XModel > aDocument( static_cast< frame::XModel* >( pOnlyInDocument_AsXModel ) );
        OUString aScriptURL = basctl::ChooseMacro( aDocument, bChooseOnly, aMacroDesc );
        rtl_uString* pScriptURL = aScriptURL.pData;
        rtl_uString_acquire( pScriptURL );

        return pScriptURL;
    }
    SAL_DLLPUBLIC_EXPORT void basicide_macro_organizer( sal_Int16 nTabId )
    {
        SAL_INFO("basctl.basicide","in basicide_macro_organizer");
        basctl::Organize( nTabId );
    }
}

void Organize( sal_Int16 tabId )
{
    EnsureIde();

    EntryDescriptor aDesc;
    if (Shell* pShell = GetShell())
        if (BaseWindow* pCurWin = pShell->GetCurWindow())
            aDesc = pCurWin->CreateEntryDescriptor();

    vcl::Window* pParent = Application::GetDefDialogParent();
    ScopedVclPtrInstance<OrganizeDialog>(pParent, tabId, aDesc)->Execute();
}

bool IsValidSbxName( const OUString& rName )
{
    for ( sal_Int32 nChar = 0; nChar < rName.getLength(); nChar++ )
    {
        sal_Unicode c = rName[nChar];
        bool bValid = (
            ( c >= 'A' && c <= 'Z' ) ||
            ( c >= 'a' && c <= 'z' ) ||
            ( c >= '0' && c <= '9' && nChar ) ||
            ( c == '_' )
        );
        if ( !bValid )
            return false;
    }
    return true;
}

static bool StringCompareLessThan( const OUString& rStr1, const OUString& rStr2 )
{
    return rStr1.compareToIgnoreAsciiCase( rStr2 ) < 0;
}

Sequence< OUString > GetMergedLibraryNames( const Reference< script::XLibraryContainer >& xModLibContainer, const Reference< script::XLibraryContainer >& xDlgLibContainer )
{
    // create a sorted list of module library names
    ::std::vector<OUString> aModLibList;
    if ( xModLibContainer.is() )
    {
        Sequence< OUString > aModLibNames = xModLibContainer->getElementNames();
        sal_Int32 nModLibCount = aModLibNames.getLength();
        const OUString* pModLibNames = aModLibNames.getConstArray();
        for ( sal_Int32 i = 0 ; i < nModLibCount ; i++ )
            aModLibList.push_back( pModLibNames[ i ] );
        ::std::sort( aModLibList.begin() , aModLibList.end() , StringCompareLessThan );
    }

    // create a sorted list of dialog library names
    ::std::vector<OUString> aDlgLibList;
    if ( xDlgLibContainer.is() )
    {
        Sequence< OUString > aDlgLibNames = xDlgLibContainer->getElementNames();
        sal_Int32 nDlgLibCount = aDlgLibNames.getLength();
        const OUString* pDlgLibNames = aDlgLibNames.getConstArray();
        for ( sal_Int32 i = 0 ; i < nDlgLibCount ; i++ )
            aDlgLibList.push_back( pDlgLibNames[ i ] );
        ::std::sort( aDlgLibList.begin() , aDlgLibList.end() , StringCompareLessThan );
    }

    // merge both lists
    ::std::vector<OUString> aLibList( aModLibList.size() + aDlgLibList.size() );
    ::std::merge( aModLibList.begin(), aModLibList.end(), aDlgLibList.begin(), aDlgLibList.end(), aLibList.begin(), StringCompareLessThan );
    ::std::vector<OUString>::iterator aIterEnd = ::std::unique( aLibList.begin(), aLibList.end() );  // move unique elements to the front
    aLibList.erase( aIterEnd, aLibList.end() ); // remove duplicates

    // copy to sequence
    sal_Int32 nLibCount = aLibList.size();
    Sequence< OUString > aSeqLibNames( nLibCount );
    for ( sal_Int32 i = 0 ; i < nLibCount ; i++ )
        aSeqLibNames.getArray()[ i ] = aLibList[ i ];

    return aSeqLibNames;
}

bool RenameModule (
    vcl::Window* pErrorParent,
    const ScriptDocument& rDocument,
    const OUString& rLibName,
    const OUString& rOldName,
    const OUString& rNewName
)
{
    if ( !rDocument.hasModule( rLibName, rOldName ) )
    {
        SAL_WARN( "basctl.basicide","basctl::RenameModule: old module name is invalid!" );
        return false;
    }

    if ( rDocument.hasModule( rLibName, rNewName ) )
    {
        ScopedVclPtrInstance< MessageDialog > aError(pErrorParent, IDE_RESSTR(RID_STR_SBXNAMEALLREADYUSED2));
        aError->Execute();
        return false;
    }

    // #i74440
    if ( rNewName.isEmpty() )
    {
        ScopedVclPtrInstance< MessageDialog > aError(pErrorParent, IDE_RESSTR(RID_STR_BADSBXNAME));
        aError->Execute();
        return false;
    }

    if ( !rDocument.renameModule( rLibName, rOldName, rNewName ) )
        return false;

    if (Shell* pShell = GetShell())
    {
        if (ModulWindow* pWin = pShell->FindBasWin(rDocument, rLibName, rNewName, false, true))
        {
            // set new name in window
            pWin->SetName( rNewName );

            // set new module in module window
            pWin->SetSbModule( pWin->GetBasic()->FindModule( rNewName ) );

            // update tabwriter
            sal_uInt16 nId = pShell->GetWindowId( pWin );
            SAL_WARN_IF( nId == 0 , "basctl.basicide", "No entry in Tabbar!");
            if ( nId )
            {
                TabBar& rTabBar = pShell->GetTabBar();
                rTabBar.SetPageText(nId, rNewName);
                rTabBar.Sort();
                rTabBar.MakeVisible(rTabBar.GetCurPageId());
            }
        }
    }
    return true;
}

namespace
{
    struct MacroExecutionData
    {
        ScriptDocument  aDocument;
        SbMethodRef     xMethod;

        MacroExecutionData()
            :aDocument( ScriptDocument::NoDocument )
            ,xMethod( nullptr )
        {
        }
    };

    class MacroExecution
    {
    public:
        DECL_STATIC_LINK_TYPED( MacroExecution, ExecuteMacroEvent, void*, void );
    };

    IMPL_STATIC_LINK_TYPED( MacroExecution, ExecuteMacroEvent, void*, p, void )
    {
        MacroExecutionData* i_pData = static_cast<MacroExecutionData*>(p);
        ENSURE_OR_RETURN_VOID( i_pData, "wrong MacroExecutionData" );
        // take ownership of the data
        std::unique_ptr< MacroExecutionData > pData( i_pData );

        SAL_WARN_IF( (pData->xMethod->GetParent()->GetFlags() & SbxFlagBits::ExtSearch) == SbxFlagBits::NONE, "basctl.basicide","No EXTSEARCH!" );

        // in case this is a document-local macro, try to protect the document's Undo Manager from
        // flawed scripts
        std::unique_ptr< ::framework::DocumentUndoGuard > pUndoGuard;
        if ( pData->aDocument.isDocument() )
            pUndoGuard.reset( new ::framework::DocumentUndoGuard( pData->aDocument.getDocument() ) );

        RunMethod(pData->xMethod);
    }
}

OUString ChooseMacro( const uno::Reference< frame::XModel >& rxLimitToDocument, bool bChooseOnly, const OUString& rMacroDesc )
{
    (void)rMacroDesc;

    EnsureIde();

    GetExtraData()->ChoosingMacro() = true;

    OUString aScriptURL;
    SbMethod* pMethod = nullptr;

    ScopedVclPtrInstance< MacroChooser > pChooser( nullptr, true );
    if ( bChooseOnly || !SvtModuleOptions::IsBasicIDE() )
        pChooser->SetMode(MacroChooser::ChooseOnly);

    if ( !bChooseOnly && rxLimitToDocument.is() )
        // Hack!
        pChooser->SetMode(MacroChooser::Recording);

    short nRetValue = pChooser->Execute();

    GetExtraData()->ChoosingMacro() = false;

    switch ( nRetValue )
    {
        case Macro_OkRun:
        {
            bool bError = false;

            pMethod = pChooser->GetMacro();
            if ( !pMethod && pChooser->GetMode() == MacroChooser::Recording )
                pMethod = pChooser->CreateMacro();

            if ( !pMethod )
                break;

            SbModule* pModule = pMethod->GetModule();
            if ( !pModule )
            {
                SAL_WARN( "basctl.basicide", "basctl::ChooseMacro: No Module found!" );
                break;
            }

            StarBASIC* pBasic = dynamic_cast<StarBASIC*>(pModule->GetParent());
            if ( !pBasic )
            {
                SAL_WARN( "basctl.basicide", "basctl::ChooseMacro: No Basic found!" );
                break;
            }

            BasicManager* pBasMgr = FindBasicManager( pBasic );
            if ( !pBasMgr )
            {
                SAL_WARN( "basctl.basicide", "basctl::ChooseMacro: No BasicManager found!" );
                break;
            }

            // name
            OUString aName = pBasic->GetName() + "." + pModule->GetName() + "." + pMethod->GetName();

            // location
            OUString aLocation;
            ScriptDocument aDocument( ScriptDocument::getDocumentForBasicManager( pBasMgr ) );
            if ( aDocument.isDocument() )
            {
                // document basic
                aLocation = "document" ;

                if ( rxLimitToDocument.is() )
                {
                    uno::Reference< frame::XModel > xLimitToDocument( rxLimitToDocument );

                    uno::Reference< document::XEmbeddedScripts > xScripts( rxLimitToDocument, UNO_QUERY );
                    if ( !xScripts.is() )
                    {   // the document itself does not support embedding scripts
                        uno::Reference< document::XScriptInvocationContext > xContext( rxLimitToDocument, UNO_QUERY );
                        if ( xContext.is() )
                            xScripts = xContext->getScriptContainer();
                        if ( xScripts.is() )
                        {   // but it is able to refer to a document which actually does support this
                            xLimitToDocument.set( xScripts, UNO_QUERY );
                            if ( !xLimitToDocument.is() )
                            {
                                SAL_WARN_IF(!xLimitToDocument.is(), "basctl.basicide", "basctl::ChooseMacro: a script container which is no document!?" );
                                xLimitToDocument = rxLimitToDocument;
                            }
                        }
                    }

                    if ( xLimitToDocument != aDocument.getDocument() )
                    {
                        // error
                        bError = true;
                        ScopedVclPtrInstance<MessageDialog>(nullptr, IDEResId(RID_STR_ERRORCHOOSEMACRO))->Execute();
                    }
                }
            }
            else
            {
                // application basic
                aLocation = "application" ;
            }

            // script URL
            if ( !bError )
            {
                aScriptURL = "vnd.sun.star.script:" + aName + "?language=Basic&location=" + aLocation;
            }

            if ( !rxLimitToDocument.is() )
            {
                MacroExecutionData* pExecData = new MacroExecutionData;
                pExecData->aDocument = aDocument;
                pExecData->xMethod = pMethod;   // keep alive until the event has been processed
                Application::PostUserEvent( LINK( nullptr, MacroExecution, ExecuteMacroEvent ), pExecData );
            }
        }
        break;
    }

    return aScriptURL;
}

Sequence< OUString > GetMethodNames( const ScriptDocument& rDocument, const OUString& rLibName, const OUString& rModName )
    throw (NoSuchElementException, RuntimeException, std::exception)
{
    Sequence< OUString > aSeqMethods;

    // get module
    OUString aOUSource;
    if ( rDocument.getModule( rLibName, rModName, aOUSource ) )
    {
        BasicManager* pBasMgr = rDocument.getBasicManager();
        StarBASIC* pSb = pBasMgr ? pBasMgr->GetLib( rLibName ) : nullptr;
        SbModule* pMod = pSb ? pSb->FindModule( rModName ) : nullptr;

        SbModuleRef xModule;
        // Only reparse modules if ScriptDocument source is out of sync
        // with basic's Module
        if ( !pMod || pMod->GetSource() != aOUSource )
        {
            xModule = new SbModule( rModName );
            xModule->SetSource32( aOUSource );
            pMod = xModule;
        }

        sal_uInt16 nCount = pMod->GetMethods()->Count();
        sal_uInt16 nRealCount = nCount;
        for ( sal_uInt16 i = 0; i < nCount; i++ )
        {
            SbMethod* pMethod = static_cast<SbMethod*>(pMod->GetMethods()->Get( i ));
            if( pMethod->IsHidden() )
                --nRealCount;
        }
        aSeqMethods.realloc( nRealCount );

        sal_uInt16 iTarget = 0;
        for ( sal_uInt16 i = 0 ; i < nCount; ++i )
        {
            SbMethod* pMethod = static_cast<SbMethod*>(pMod->GetMethods()->Get( i ));
            if( pMethod->IsHidden() )
                continue;
            SAL_WARN_IF( !pMethod, "basctl.basicide","Method not found! (NULL)" );
            aSeqMethods.getArray()[ iTarget++ ] = pMethod->GetName();
        }
    }

    return aSeqMethods;
}

bool HasMethod (
    ScriptDocument const& rDocument,
    OUString const& rLibName,
    OUString const& rModName,
    OUString const& rMethName
)
{
    bool bHasMethod = false;

    OUString aOUSource;
    if ( rDocument.hasModule( rLibName, rModName ) && rDocument.getModule( rLibName, rModName, aOUSource ) )
    {
        // Check if we really need to scan the source ( again )
        BasicManager* pBasMgr = rDocument.getBasicManager();
        StarBASIC* pSb = pBasMgr ? pBasMgr->GetLib( rLibName ) : nullptr;
        SbModule* pMod = pSb ? pSb->FindModule( rModName ) : nullptr;
        SbModuleRef xModule;
        // Only reparse modules if ScriptDocument source is out of sync
        // with basic's Module
        if ( !pMod || pMod->GetSource() != aOUSource )
        {
            xModule = new SbModule( rModName );
            xModule->SetSource32( aOUSource );
            pMod = xModule;
        }
        SbxArray* pMethods = pMod->GetMethods();
        if ( pMethods )
        {
            SbMethod* pMethod = static_cast<SbMethod*>(pMethods->Find( rMethName, SbxClassType::Method ));
            if ( pMethod && !pMethod->IsHidden() )
                bHasMethod = true;
        }
    }

    return bHasMethod;
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
