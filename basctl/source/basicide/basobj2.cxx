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

#include <basic/sbx.hxx>
#include <framework/documentundoguard.hxx>
#include <tools/diagnose_ex.h>
#include <unotools/moduleoptions.hxx>

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
    BasicIDEDLL::Init();

    BasicEntryDescriptor aDesc;
    BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
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

sal_Bool IsValidSbxName( const String& rName )
{
    for ( sal_uInt16 nChar = 0; nChar < rName.Len(); nChar++ )
    {
        sal_Bool bValid = ( ( rName.GetChar(nChar) >= 'A' && rName.GetChar(nChar) <= 'Z' ) ||
                        ( rName.GetChar(nChar) >= 'a' && rName.GetChar(nChar) <= 'z' ) ||
                        ( rName.GetChar(nChar) >= '0' && rName.GetChar(nChar) <= '9' && nChar ) ||
                        ( rName.GetChar(nChar) == '_' ) );
        if ( !bValid )
            return sal_False;
    }
    return sal_True;
}

static sal_Bool StringCompareLessThan( const String& rStr1, const String& rStr2 )
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

bool RenameModule( Window* pErrorParent, const ScriptDocument& rDocument, const String& rLibName, const String& rOldName, const String& rNewName )
{
    if ( !rDocument.hasModule( rLibName, rOldName ) )
    {
        OSL_ENSURE( false, "BasicIDE::RenameModule: old module name is invalid!" );
        return false;
    }

    if ( rDocument.hasModule( rLibName, rNewName ) )
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

    if ( !rDocument.renameModule( rLibName, rOldName, rNewName ) )
        return false;

    BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
    if ( pIDEShell )
    {
        IDEBaseWindow* pWin = pIDEShell->FindWindow( rDocument, rLibName, rNewName, BASICIDE_TYPE_MODULE, sal_True );
        if ( pWin )
        {
            // set new name in window
            pWin->SetName( rNewName );

            // set new module in module window
            ModulWindow* pModWin = (ModulWindow*)pWin;
            pModWin->SetSbModule( (SbModule*)pModWin->GetBasic()->FindModule( rNewName ) );

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

::rtl::OUString ChooseMacro( const uno::Reference< frame::XModel >& rxLimitToDocument, sal_Bool bChooseOnly, const ::rtl::OUString& rMacroDesc )
{
    (void)rMacroDesc;

    BasicIDEDLL::Init();

    IDE_DLL()->GetExtraData()->ChoosingMacro() = sal_True;

    String aScriptURL;
    sal_Bool bError = sal_False;
    SbMethod* pMethod = NULL;

    ::std::auto_ptr< MacroChooser > pChooser( new MacroChooser( NULL, sal_True ) );
    if ( bChooseOnly || !SvtModuleOptions().IsBasicIDE() )
        pChooser->SetMode( MACROCHOOSER_CHOOSEONLY );

    if ( !bChooseOnly && rxLimitToDocument.is() )
        // Hack!
        pChooser->SetMode( MACROCHOOSER_RECORDING );

    short nRetValue = pChooser->Execute();

    IDE_DLL()->GetExtraData()->ChoosingMacro() = sal_False;

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
            ENSURE_OR_BREAK( pModule, "BasicIDE::ChooseMacro: No Module found!" );

            StarBASIC* pBasic = (StarBASIC*)pModule->GetParent();
            ENSURE_OR_BREAK( pBasic, "BasicIDE::ChooseMacro: No Basic found!" );

            BasicManager* pBasMgr = BasicIDE::FindBasicManager( pBasic );
            ENSURE_OR_BREAK( pBasMgr, "BasicIDE::ChooseMacro: No BasicManager found!" );

            // name
            String aName;
            aName += pBasic->GetName();
            aName += '.';
            aName += pModule->GetName();
            aName += '.';
            aName += pMethod->GetName();

            // language
            String aLanguage = String::CreateFromAscii("Basic");

            // location
            String aLocation;
            ScriptDocument aDocument( ScriptDocument::getDocumentForBasicManager( pBasMgr ) );
            if ( aDocument.isDocument() )
            {
                // document basic
                aLocation = String::CreateFromAscii("document");

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
                        bError = TRUE;
                        ErrorBox( NULL, WB_OK | WB_DEF_OK, String( IDEResId( RID_STR_ERRORCHOOSEMACRO ) ) ).Execute();
                    }
                }
            }
            else
            {
                // application basic
                aLocation = String::CreateFromAscii("application");
            }

            // script URL
            if ( !bError )
            {
                aScriptURL = String::CreateFromAscii("vnd.sun.star.script:");
                aScriptURL += aName;
                aScriptURL += String::CreateFromAscii("?language=");
                aScriptURL += aLanguage;
                aScriptURL += String::CreateFromAscii("&location=");
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

Sequence< ::rtl::OUString > GetMethodNames( const ScriptDocument& rDocument, const String& rLibName, const String& rModName )
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

sal_Bool HasMethod( const ScriptDocument& rDocument, const String& rLibName, const String& rModName, const String& rMethName )
{
    sal_Bool bHasMethod = sal_False;

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
                bHasMethod = sal_True;
        }
    }

    return bHasMethod;
}
} //namespace BasicIDE
//----------------------------------------------------------------------------
