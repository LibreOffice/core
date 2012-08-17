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

#include "basobj.hxx"
#include "iderdll.hxx"
#include "iderdll2.hxx"
#include "iderid.hxx"
#include "macrodlg.hxx"
#include "moduldlg.hxx"
#include "basidesh.hxx"
#include "basidesh.hrc"
#include "baside2.hxx"
#include "basicmod.hxx"
#include "basdoc.hxx"

#include <com/sun/star/document/XEmbeddedScripts.hpp>
#include <com/sun/star/document/XScriptInvocationContext.hpp>

#include <basic/sbmeth.hxx>
#include <basic/sbx.hxx>
#include <framework/documentundoguard.hxx>
#include <tools/diagnose_ex.h>
#include <unotools/moduleoptions.hxx>
#include <vcl/msgbox.hxx>

#include <vector>
#include <algorithm>
#include <memory>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;


//----------------------------------------------------------------------------

extern "C" {
    SAL_DLLPUBLIC_EXPORT rtl_uString* basicide_choose_macro( void* pOnlyInDocument_AsXModel, sal_Bool bChooseOnly, rtl_uString* pMacroDesc )
    {
        ::rtl::OUString aMacroDesc( pMacroDesc );
        Reference< frame::XModel > aDocument( static_cast< frame::XModel* >( pOnlyInDocument_AsXModel ) );
        ::rtl::OUString aScriptURL = BasicIDE::ChooseMacro( aDocument, bChooseOnly, aMacroDesc );
        rtl_uString* pScriptURL = aScriptURL.pData;
        rtl_uString_acquire( pScriptURL );

        return pScriptURL;
    }
    SAL_DLLPUBLIC_EXPORT void basicide_macro_organizer( sal_Int16 nTabId )
    {
        OSL_TRACE("in basicide_macro_organizer");
        BasicIDE::Organize( nTabId );
    }
}

namespace BasicIDE
{
//----------------------------------------------------------------------------

void Organize( sal_Int16 tabId )
{
    BasicIDEGlobals::ensure();

    BasicEntryDescriptor aDesc;
    BasicIDEShell* pIDEShell = BasicIDEGlobals::GetShell();
    if ( pIDEShell )
    {
        IDEBaseWindow* pCurWin = pIDEShell->GetCurWindow();
        if ( pCurWin )
            aDesc = pCurWin->CreateEntryDescriptor();
    }

    Window* pParent = Application::GetDefDialogParent();
    OrganizeDialog* pDlg = new OrganizeDialog( pParent, tabId, aDesc );
    pDlg->Execute();
    delete pDlg;
}

//----------------------------------------------------------------------------

bool IsValidSbxName( const String& rName )
{
    for ( sal_uInt16 nChar = 0; nChar < rName.Len(); nChar++ )
    {
        sal_Unicode c = rName.GetChar(nChar);
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

static bool StringCompareLessThan( const String& rStr1, const String& rStr2 )
{
    return (rStr1.CompareIgnoreCaseToAscii( rStr2 ) == COMPARE_LESS);
}

//----------------------------------------------------------------------------

Sequence< ::rtl::OUString > GetMergedLibraryNames( const Reference< script::XLibraryContainer >& xModLibContainer, const Reference< script::XLibraryContainer >& xDlgLibContainer )
{
    // create a sorted list of module library names
    ::std::vector<String> aModLibList;
    if ( xModLibContainer.is() )
    {
        Sequence< ::rtl::OUString > aModLibNames = xModLibContainer->getElementNames();
        sal_Int32 nModLibCount = aModLibNames.getLength();
        const ::rtl::OUString* pModLibNames = aModLibNames.getConstArray();
        for ( sal_Int32 i = 0 ; i < nModLibCount ; i++ )
            aModLibList.push_back( pModLibNames[ i ] );
        ::std::sort( aModLibList.begin() , aModLibList.end() , StringCompareLessThan );
    }

    // create a sorted list of dialog library names
    ::std::vector<String> aDlgLibList;
    if ( xDlgLibContainer.is() )
    {
        Sequence< ::rtl::OUString > aDlgLibNames = xDlgLibContainer->getElementNames();
        sal_Int32 nDlgLibCount = aDlgLibNames.getLength();
        const ::rtl::OUString* pDlgLibNames = aDlgLibNames.getConstArray();
        for ( sal_Int32 i = 0 ; i < nDlgLibCount ; i++ )
            aDlgLibList.push_back( pDlgLibNames[ i ] );
        ::std::sort( aDlgLibList.begin() , aDlgLibList.end() , StringCompareLessThan );
    }

    // merge both lists
    ::std::vector<String> aLibList( aModLibList.size() + aDlgLibList.size() );
    ::std::merge( aModLibList.begin(), aModLibList.end(), aDlgLibList.begin(), aDlgLibList.end(), aLibList.begin(), StringCompareLessThan );
    ::std::vector<String>::iterator aIterEnd = ::std::unique( aLibList.begin(), aLibList.end() );  // move unique elements to the front
    aLibList.erase( aIterEnd, aLibList.end() ); // remove duplicates

    // copy to sequence
    sal_Int32 nLibCount = aLibList.size();
    Sequence< ::rtl::OUString > aSeqLibNames( nLibCount );
    for ( sal_Int32 i = 0 ; i < nLibCount ; i++ )
        aSeqLibNames.getArray()[ i ] = aLibList[ i ];

    return aSeqLibNames;
}

//----------------------------------------------------------------------------

bool RenameModule( Window* pErrorParent, const ScriptDocument& rDocument, const ::rtl::OUString& rLibName, const ::rtl::OUString& rOldName, const ::rtl::OUString& rNewName )
{
    if ( !rDocument.hasModule( rLibName, rOldName ) )
    {
        OSL_FAIL( "BasicIDE::RenameModule: old module name is invalid!" );
        return false;
    }

    if ( rDocument.hasModule( rLibName, rNewName ) )
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

    if ( !rDocument.renameModule( rLibName, rOldName, rNewName ) )
        return false;

    if (BasicIDEShell* pIDEShell = BasicIDEGlobals::GetShell())
    {
        if (basctl::ModulWindow* pWin = pIDEShell->FindBasWin(rDocument, rLibName, rNewName, false, true))
        {
            // set new name in window
            pWin->SetName( rNewName );

            // set new module in module window
            pWin->SetSbModule( (SbModule*)pWin->GetBasic()->FindModule( rNewName ) );

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
    }
    return true;
}


//----------------------------------------------------------------------------

namespace
{
    struct MacroExecutionData
    {
        ScriptDocument  aDocument;
        SbMethodRef     xMethod;

        MacroExecutionData()
            :aDocument( ScriptDocument::NoDocument )
            ,xMethod( NULL )
        {
        }
    };

    class MacroExecution
    {
    public:
        DECL_STATIC_LINK( MacroExecution, ExecuteMacroEvent, MacroExecutionData* );
    };


    IMPL_STATIC_LINK( MacroExecution, ExecuteMacroEvent, MacroExecutionData*, i_pData )
    {
        (void)pThis;
        ENSURE_OR_RETURN( i_pData, "wrong MacroExecutionData", 0L );
        // take ownership of the data
        ::std::auto_ptr< MacroExecutionData > pData( i_pData );

        DBG_ASSERT( pData->xMethod->GetParent()->GetFlags() & SBX_EXTSEARCH, "Kein EXTSEARCH!" );

        // in case this is a document-local macro, try to protect the document's Undo Manager from
        // flawed scripts
        ::std::auto_ptr< ::framework::DocumentUndoGuard > pUndoGuard;
        if ( pData->aDocument.isDocument() )
            pUndoGuard.reset( new ::framework::DocumentUndoGuard( pData->aDocument.getDocument() ) );

        BasicIDE::RunMethod( pData->xMethod );

        return 1L;
    }
}

//----------------------------------------------------------------------------

::rtl::OUString ChooseMacro( const uno::Reference< frame::XModel >& rxLimitToDocument, bool bChooseOnly, const ::rtl::OUString& rMacroDesc )
{
    (void)rMacroDesc;

    BasicIDEGlobals::ensure();

    BasicIDEGlobals::GetExtraData()->ChoosingMacro() = true;

    String aScriptURL;
    bool bError = false;
    SbMethod* pMethod = NULL;

    ::std::auto_ptr< MacroChooser > pChooser( new MacroChooser( NULL, true ) );
    if ( bChooseOnly || !SvtModuleOptions().IsBasicIDE() )
        pChooser->SetMode( MACROCHOOSER_CHOOSEONLY );

    if ( !bChooseOnly && rxLimitToDocument.is() )
        // Hack!
        pChooser->SetMode( MACROCHOOSER_RECORDING );

    short nRetValue = pChooser->Execute();

    BasicIDEGlobals::GetExtraData()->ChoosingMacro() = false;

    switch ( nRetValue )
    {
        case MACRO_OK_RUN:
        {
            pMethod = pChooser->GetMacro();
            if ( !pMethod && pChooser->GetMode() == MACROCHOOSER_RECORDING )
                pMethod = pChooser->CreateMacro();

            if ( !pMethod )
                break;

            SbModule* pModule = pMethod->GetModule();
            if ( !pModule )
            {
                SAL_WARN( "basctl.basicide", "BasicIDE::ChooseMacro: No Module found!" );
                break;
            }

            StarBASIC* pBasic = dynamic_cast<StarBASIC*>(pModule->GetParent());
            if ( !pBasic )
            {
                SAL_WARN( "basctl.basicide", "BasicIDE::ChooseMacro: No Basic found!" );
                break;
            }

            BasicManager* pBasMgr = BasicIDE::FindBasicManager( pBasic );
            if ( !pBasMgr )
            {
                SAL_WARN( "basctl.basicide", "BasicIDE::ChooseMacro: No BasicManager found!" );
                break;
            }

            // name
            String aName;
            aName += pBasic->GetName();
            aName += '.';
            aName += pModule->GetName();
            aName += '.';
            aName += pMethod->GetName();

            // language
            rtl::OUString aLanguage("Basic");

            // location
            rtl::OUString aLocation;
            ScriptDocument aDocument( ScriptDocument::getDocumentForBasicManager( pBasMgr ) );
            if ( aDocument.isDocument() )
            {
                // document basic
                aLocation = rtl::OUString("document");

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
                                OSL_ENSURE( false, "BasicIDE::ChooseMacro: a script container which is no document!?" );
                                xLimitToDocument = rxLimitToDocument;
                            }
                        }
                    }

                    if ( xLimitToDocument != aDocument.getDocument() )
                    {
                        // error
                        bError = true;
                        ErrorBox( NULL, WB_OK | WB_DEF_OK, String( IDEResId( RID_STR_ERRORCHOOSEMACRO ) ) ).Execute();
                    }
                }
            }
            else
            {
                // application basic
                aLocation = rtl::OUString("application");
            }

            // script URL
            if ( !bError )
            {
                aScriptURL = rtl::OUString("vnd.sun.star.script:");
                aScriptURL += aName;
                aScriptURL += rtl::OUString("?language=");
                aScriptURL += aLanguage;
                aScriptURL += rtl::OUString("&location=");
                aScriptURL += aLocation;
            }

            if ( !rxLimitToDocument.is() )
            {
                MacroExecutionData* pExecData = new MacroExecutionData;
                pExecData->aDocument = aDocument;
                pExecData->xMethod = pMethod;   // keep alive until the event has been processed
                Application::PostUserEvent( STATIC_LINK( NULL, MacroExecution, ExecuteMacroEvent ), pExecData );
            }
        }
        break;
    }

    return aScriptURL;
}

//----------------------------------------------------------------------------

Sequence< ::rtl::OUString > GetMethodNames( const ScriptDocument& rDocument, const ::rtl::OUString& rLibName, const ::rtl::OUString& rModName )
    throw(NoSuchElementException )
{
    Sequence< ::rtl::OUString > aSeqMethods;

    // get module
    ::rtl::OUString aOUSource;
    if ( rDocument.getModule( rLibName, rModName, aOUSource ) )
    {
        SbModuleRef xModule = new SbModule( rModName );
        xModule->SetSource32( aOUSource );
        sal_uInt16 nCount = xModule->GetMethods()->Count();
        sal_uInt16 nRealCount = nCount;
        for ( sal_uInt16 i = 0; i < nCount; i++ )
        {
            SbMethod* pMethod = (SbMethod*)xModule->GetMethods()->Get( i );
            if( pMethod->IsHidden() )
                --nRealCount;
        }
        aSeqMethods.realloc( nRealCount );

        sal_uInt16 iTarget = 0;
        for ( sal_uInt16 i = 0 ; i < nCount; ++i )
        {
            SbMethod* pMethod = (SbMethod*)xModule->GetMethods()->Get( i );
            if( pMethod->IsHidden() )
                continue;
            DBG_ASSERT( pMethod, "Method not found! (NULL)" );
            aSeqMethods.getArray()[ iTarget++ ] = pMethod->GetName();
        }
    }

    return aSeqMethods;
}

//----------------------------------------------------------------------------

bool HasMethod( const ScriptDocument& rDocument, const ::rtl::OUString& rLibName, const ::rtl::OUString& rModName, const ::rtl::OUString& rMethName )
{
    bool bHasMethod = false;

    ::rtl::OUString aOUSource;
    if ( rDocument.hasModule( rLibName, rModName ) && rDocument.getModule( rLibName, rModName, aOUSource ) )
    {
        SbModuleRef xModule = new SbModule( rModName );
        xModule->SetSource32( aOUSource );
        SbxArray* pMethods = xModule->GetMethods();
        if ( pMethods )
        {
            SbMethod* pMethod = (SbMethod*)pMethods->Find( rMethName, SbxCLASS_METHOD );
            if ( pMethod && !pMethod->IsHidden() )
                bHasMethod = true;
        }
    }

    return bHasMethod;
}
} //namespace BasicIDE
//----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
