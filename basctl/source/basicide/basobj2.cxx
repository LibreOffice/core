/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: basobj2.cxx,v $
 *
 *  $Revision: 1.35 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 19:13:26 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basctl.hxx"

#include <ide_pch.hxx>

#include <vector>
#include <algorithm>

#ifndef _SBXCLASS_HXX //autogen
#include <basic/sbx.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_MODULEOPTIONS_HXX
#include <svtools/moduleoptions.hxx>
#endif
#include <com/sun/star/document/XEmbeddedScripts.hpp>
#include <com/sun/star/document/XScriptInvocationContext.hpp>
#include <basobj.hxx>
#include <iderdll.hxx>
#include <iderdll2.hxx>
#include <iderid.hxx>
#include <macrodlg.hxx>
#include <moduldlg.hxx>
#include <basidesh.hxx>
#include <basidesh.hrc>
#include <baside2.hxx>
#include <basicmod.hxx>
#include <basdoc.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;


//----------------------------------------------------------------------------

extern "C" {
    rtl_uString* basicide_choose_macro( void* pOnlyInDocument_AsXModel, BOOL bChooseOnly, rtl_uString* pMacroDesc )
    {
        ::rtl::OUString aMacroDesc( pMacroDesc );
        Reference< frame::XModel > aDocument( static_cast< frame::XModel* >( pOnlyInDocument_AsXModel ) );
        ::rtl::OUString aScriptURL = BasicIDE::ChooseMacro( aDocument, bChooseOnly, aMacroDesc );
        rtl_uString* pScriptURL = aScriptURL.pData;
        rtl_uString_acquire( pScriptURL );

        return pScriptURL;
    }
    void basicide_macro_organizer( INT16 nTabId )
    {
        OSL_TRACE("in basicide_macro_organizer");
        BasicIDE::Organize( nTabId );
    }
}

//----------------------------------------------------------------------------

SfxMacro* BasicIDE::CreateMacro()
{
    DBG_ERROR( "BasicIDE::CreateMacro() - war eigentlich nur fuer Macro-Recording!" );
    IDE_DLL()->GetExtraData()->ChoosingMacro() = TRUE;
    SFX_APP()->EnterBasicCall();
    Window* pParent = Application::GetDefDialogParent();
    SfxMacro* pMacro = 0;
    MacroChooser* pChooser = new MacroChooser( pParent, TRUE );
    Window* pOldModalDialogParent = Application::GetDefDialogParent();
    Application::SetDefDialogParent( pChooser );
    //pChooser->SetMode( MACROCHOOSER_RECORDING );
    short nRetValue = pChooser->Execute();
    (void)nRetValue;

    Application::SetDefDialogParent( pOldModalDialogParent );
    delete pChooser;

    SFX_APP()->LeaveBasicCall();
    IDE_DLL()->GetExtraData()->ChoosingMacro() = FALSE;

    return pMacro;
}

//----------------------------------------------------------------------------

void BasicIDE::Organize( INT16 tabId )
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

BOOL BasicIDE::IsValidSbxName( const String& rName )
{
    for ( USHORT nChar = 0; nChar < rName.Len(); nChar++ )
    {
        BOOL bValid = ( ( rName.GetChar(nChar) >= 'A' && rName.GetChar(nChar) <= 'Z' ) ||
                        ( rName.GetChar(nChar) >= 'a' && rName.GetChar(nChar) <= 'z' ) ||
                        ( rName.GetChar(nChar) >= '0' && rName.GetChar(nChar) <= '9' && nChar ) ||
                        ( rName.GetChar(nChar) == '_' ) );
        if ( !bValid )
            return FALSE;
    }
    return TRUE;
}

//----------------------------------------------------------------------------

SbMethod* BasicIDE::FindMacro( SbModule* pModule, const String& rMacroName )
{
    return (SbMethod*)pModule->GetMethods()->Find( rMacroName, SbxCLASS_METHOD );
}

//----------------------------------------------------------------------------

USHORT BasicIDE::GetBasicDialogCount()
{
    return IDE_DLL()->GetExtraData()->GetBasicDialogCount();
}

//----------------------------------------------------------------------------

void BasicIDE::IncBasicDialogCount()
{
    IDE_DLL()->GetExtraData()->IncBasicDialogCount();
}

//----------------------------------------------------------------------------

void BasicIDE::DecBasicDialogCount()
{
    DBG_ASSERT( GetBasicDialogCount(), "DecBasicDialogCount() - Count allready 0!" );
    IDE_DLL()->GetExtraData()->DecBasicDialogCount();
}

//----------------------------------------------------------------------------

Sequence< ::rtl::OUString > BasicIDE::GetMergedLibraryNames( const Reference< script::XLibraryContainer >& xModLibContainer, const Reference< script::XLibraryContainer >& xDlgLibContainer )
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

bool BasicIDE::RenameModule( Window* pErrorParent, const ScriptDocument& rDocument, const String& rLibName, const String& rOldName, const String& rNewName )
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
        IDEBaseWindow* pWin = pIDEShell->FindWindow( rDocument, rLibName, rOldName, BASICIDE_TYPE_MODULE, FALSE );
        if ( pWin )
        {
            // set new name in window
            pWin->SetName( rNewName );

            // set new module in module window
            ModulWindow* pModWin = (ModulWindow*)pWin;
            pModWin->SetSbModule( (SbModule*)pModWin->GetBasic()->FindModule( rNewName ) );

            // update tabwriter
            USHORT nId = (USHORT)(pIDEShell->GetIDEWindowTable()).GetKey( pWin );
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

::rtl::OUString BasicIDE::ChooseMacro( const uno::Reference< frame::XModel >& rxLimitToDocument, BOOL bChooseOnly, const ::rtl::OUString& rMacroDesc )
{
    (void)rMacroDesc;

    BasicIDEDLL::Init();

    IDE_DLL()->GetExtraData()->ChoosingMacro() = TRUE;
    SFX_APP()->EnterBasicCall();

    String aScriptURL;
    BOOL bError = FALSE;
    SbMethod* pMethod = NULL;

    MacroChooser* pChooser = new MacroChooser( NULL, TRUE );
    if ( bChooseOnly || !SvtModuleOptions().IsBasicIDE() )
        pChooser->SetMode( MACROCHOOSER_CHOOSEONLY );

    if ( !bChooseOnly && rxLimitToDocument.is() )
        // Hack!
        pChooser->SetMode( MACROCHOOSER_RECORDING );

    short nRetValue = pChooser->Execute();

    IDE_DLL()->GetExtraData()->ChoosingMacro() = FALSE;

    switch ( nRetValue )
    {
        case MACRO_OK_RUN:
        {
            pMethod = pChooser->GetMacro();
            if ( !pMethod && pChooser->GetMode() == MACROCHOOSER_RECORDING )
                pMethod = pChooser->CreateMacro();

            if ( pMethod )
            {
                SbModule* pModule = pMethod->GetModule();
                DBG_ASSERT(pModule, "BasicIDE::ChooseMacro: No Module found!");
                if ( pModule )
                {
                    StarBASIC* pBasic = (StarBASIC*)pModule->GetParent();
                    DBG_ASSERT(pBasic, "BasicIDE::ChooseMacro: No Basic found!");
                    if ( pBasic )
                    {
                        BasicManager* pBasMgr = BasicIDE::FindBasicManager( pBasic );
                        DBG_ASSERT(pBasMgr, "BasicIDE::ChooseMacro: No BasicManager found!");
                        if ( pBasMgr )
                        {
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
                        }
                    }
                }
            }

            if ( pMethod && !rxLimitToDocument.is() )
            {
                pMethod->AddRef();  // festhalten, bis Event abgearbeitet.
                Application::PostUserEvent( LINK( IDE_DLL()->GetExtraData(), BasicIDEData, ExecuteMacroEvent ), pMethod );
            }
        }
        break;
    }

    delete pChooser;

    SFX_APP()->LeaveBasicCall();

    return ::rtl::OUString( aScriptURL );
}

//----------------------------------------------------------------------------

Sequence< ::rtl::OUString > BasicIDE::GetMethodNames( const ScriptDocument& rDocument, const String& rLibName, const String& rModName )
    throw(NoSuchElementException )
{
    Sequence< ::rtl::OUString > aSeqMethods;

    // get module
    ::rtl::OUString aOUSource;
    if ( rDocument.getModule( rLibName, rModName, aOUSource ) )
    {
        SbModuleRef xModule = new SbModule( rModName );
        xModule->SetSource32( aOUSource );
        USHORT nCount = xModule->GetMethods()->Count();
        aSeqMethods.realloc( nCount );

        for ( USHORT i = 0; i < nCount; i++ )
        {
            SbMethod* pMethod = (SbMethod*)xModule->GetMethods()->Get( i );
            DBG_ASSERT( pMethod, "Method not found! (NULL)" );
            aSeqMethods.getArray()[ i ] = pMethod->GetName();
        }
    }

    return aSeqMethods;
}

//----------------------------------------------------------------------------

BOOL BasicIDE::HasMethod( const ScriptDocument& rDocument, const String& rLibName, const String& rModName, const String& rMethName )
{
    BOOL bHasMethod = FALSE;

    ::rtl::OUString aOUSource;
    if ( rDocument.hasModule( rLibName, rModName ) && rDocument.getModule( rLibName, rModName, aOUSource ) )
    {
        SbModuleRef xModule = new SbModule( rModName );
        xModule->SetSource32( aOUSource );
        SbxArray* pMethods = xModule->GetMethods();
        if ( pMethods )
        {
            SbMethod* pMethod = (SbMethod*)pMethods->Find( rMethName, SbxCLASS_METHOD );
            if ( pMethod )
                bHasMethod = TRUE;
        }
    }

    return bHasMethod;
}

//----------------------------------------------------------------------------
