/*************************************************************************
 *
 *  $RCSfile: basobj2.cxx,v $
 *
 *  $Revision: 1.27 $
 *
 *  last change: $Author: kz $ $Date: 2004-07-23 12:03:15 $
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


#include <ide_pch.hxx>

#pragma hdrstop

#include <vector>
#include <algorithm>

#ifndef _SBXCLASS_HXX //autogen
#include <svtools/sbx.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_MODULEOPTIONS_HXX
#include <svtools/moduleoptions.hxx>
#endif
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


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;


//----------------------------------------------------------------------------

extern "C" {
    rtl_uString* basicide_choose_macro( BOOL bExecute, BOOL bChooseOnly, rtl_uString* pMacroDesc )
    {
        ::rtl::OUString aMacroDesc( pMacroDesc );
        ::rtl::OUString aScriptURL = BasicIDE::ChooseMacro( bExecute, bChooseOnly, aMacroDesc );
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

Sequence< ::rtl::OUString > BasicIDE::GetLibraryNames( SfxObjectShell* pShell )
{
    Reference< script::XLibraryContainer > xModLibContainer( GetModuleLibraryContainer( pShell ), UNO_QUERY );
    Reference< script::XLibraryContainer > xDlgLibContainer( GetDialogLibraryContainer( pShell ), UNO_QUERY );

    return GetMergedLibraryNames( xModLibContainer, xDlgLibContainer );
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

BOOL BasicIDE::HasShell( SfxObjectShell* pShell )
{
    BOOL bHasShell = FALSE;

    if ( pShell )
    {
        SfxObjectShell* pDocShell = SfxObjectShell::GetFirst();
        while ( pDocShell )
        {
            if ( pDocShell == pShell )
            {
                bHasShell = TRUE;
                break;
            }
            pDocShell = SfxObjectShell::GetNext( *pDocShell );
        }
    }
    else
    {
        bHasShell = TRUE;
    }

    return bHasShell;
}

//----------------------------------------------------------------------------

Reference< script::XLibraryContainer > BasicIDE::GetModuleLibraryContainer( SfxObjectShell* pShell )
{
    // get library container
    Reference< script::XLibraryContainer > xLibContainer;
    if( pShell )
    {
        // document
        xLibContainer = Reference< script::XLibraryContainer >( pShell->GetBasicContainer(), UNO_QUERY );
    }
    else
    {
        // application
        xLibContainer = Reference< script::XLibraryContainer >( SFX_APP()->GetBasicContainer(), UNO_QUERY );
    }

    return xLibContainer;
}

//----------------------------------------------------------------------------

BOOL BasicIDE::HasModuleLibrary( SfxObjectShell* pShell, const String& rLibName )
{
    BOOL bHasModuleLibrary = FALSE;

    // get library container
    Reference< script::XLibraryContainer > xLibContainer = GetModuleLibraryContainer( pShell );

    // check if library container has module library
    ::rtl::OUString aOULibName( rLibName );
    if( xLibContainer.is() && xLibContainer->hasByName( aOULibName ) )
    {
        bHasModuleLibrary = TRUE;
    }

    return bHasModuleLibrary;
}

//----------------------------------------------------------------------------

Reference< container::XNameContainer > BasicIDE::GetModuleLibrary( SfxObjectShell* pShell, const String& rLibName, BOOL bLoadLibrary )
    throw(NoSuchElementException)
{
    // get library container
    Reference< script::XLibraryContainer > xLibContainer = GetModuleLibraryContainer( pShell );

    // get library
    Reference< container::XNameContainer > xLib;
    ::rtl::OUString aOULibName( rLibName );
    if( xLibContainer.is() && xLibContainer->hasByName( aOULibName ) )
    {
        Any aElement = xLibContainer->getByName( aOULibName );
        aElement >>= xLib;
    }
    else
    {
        throw NoSuchElementException(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("BasicIDE::GetModuleLibrary: NoSuchElementException!") ),
            Reference<XInterface>() );
    }

    // load library
    if( bLoadLibrary && !xLibContainer->isLibraryLoaded( aOULibName ) )
        xLibContainer->loadLibrary( aOULibName );

    return xLib;
}

//----------------------------------------------------------------------------

Reference< ::com::sun::star::container::XNameContainer > BasicIDE::CreateModuleLibrary( SfxObjectShell* pShell, const String& rLibName )
    throw(ElementExistException)
{
    // get library container
    Reference< script::XLibraryContainer > xLibContainer = GetModuleLibraryContainer( pShell );

    // create module library
    Reference< container::XNameContainer > xLib;
    ::rtl::OUString aOULibName( rLibName );
    if( xLibContainer.is() && !xLibContainer->hasByName( aOULibName ) )
    {
        xLib = xLibContainer->createLibrary( aOULibName );
    }
    else
    {
        throw ElementExistException(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("BasicIDE::CreateModuleLibrary: ElementExistException!") ),
            Reference<XInterface>() );
    }

    return xLib;
}

//----------------------------------------------------------------------------

Sequence< ::rtl::OUString > BasicIDE::GetModuleNames( SfxObjectShell* pShell, const String& rLibName )
    throw(NoSuchElementException)
{
    Sequence< ::rtl::OUString > aSeqModNames;
    Reference< container::XNameContainer > xLib = GetModuleLibrary( pShell, rLibName );
    if( xLib.is() )
    {
        Sequence< ::rtl::OUString > aModNames = xLib->getElementNames();
        sal_Int32 nModCount = aModNames.getLength();
        const ::rtl::OUString* pModNames = aModNames.getConstArray();

        // sort module names
        ::std::vector<String> aModList( nModCount );
        sal_Int32 i;

        for ( i = 0 ; i < nModCount ; i++ )
            aModList[ i ] = pModNames[ i ];
        ::std::sort( aModList.begin() , aModList.end() , StringCompareLessThan );

        // copy to sequence
        aSeqModNames.realloc( nModCount );
        for ( i = 0 ; i < nModCount ; i++ )
            aSeqModNames.getArray()[ i ] = aModList[ i ];
    }

    return aSeqModNames;
}

//----------------------------------------------------------------------------

BOOL BasicIDE::HasModule( SfxObjectShell* pShell, const String& rLibName, const String& rModName )
{
    BOOL bHasModule = FALSE;

    // get library container
    Reference< script::XLibraryContainer > xLibContainer = GetModuleLibraryContainer( pShell );

    // check if library container has module library
    ::rtl::OUString aOULibName( rLibName );
    if ( xLibContainer.is() && xLibContainer->hasByName( aOULibName ) )
    {
        // load library
        if ( !xLibContainer->isLibraryLoaded( aOULibName ) )
            xLibContainer->loadLibrary( aOULibName );

        // get library
        Reference< container::XNameContainer > xLib;
        Any aElement = xLibContainer->getByName( aOULibName );
        aElement >>= xLib;

        // check if library has module
        ::rtl::OUString aOUModName( rModName );
        if ( xLib.is() && xLib->hasByName( aOUModName ) )
            bHasModule = TRUE;
    }

    return bHasModule;
}

//----------------------------------------------------------------------------

::rtl::OUString BasicIDE::GetModule( SfxObjectShell* pShell, const String& rLibName, const String& rModName )
    throw(NoSuchElementException)
{
    // get library
    Reference< container::XNameContainer > xLib = GetModuleLibrary( pShell, rLibName, TRUE );

    // get module
    ::rtl::OUString aOUSource;
    ::rtl::OUString aOUModName( rModName );
    if( xLib.is() && xLib->hasByName( aOUModName ) )
    {
        Any aElement = xLib->getByName( aOUModName );
        aElement >>= aOUSource;
    }
    else
    {
        throw NoSuchElementException(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("BasicIDE::GetModule: NoSuchElementException!") ),
            Reference<XInterface>() );
    }

    return aOUSource;
}

//----------------------------------------------------------------------------

String BasicIDE::CreateModuleName( SfxObjectShell* pShell, const String& rLibName )
{
    String aModName;
    String aModStdName( RTL_CONSTASCII_USTRINGPARAM( "Module" ) );
    //String aModStdName( IDEResId( RID_STR_STDMODULENAME ) );
    BOOL bValid = FALSE;
    USHORT i = 1;
    while ( !bValid )
    {
        aModName = aModStdName;
        aModName += String::CreateFromInt32( i );
        if ( !BasicIDE::HasModule( pShell, rLibName, aModName ) )
            bValid = TRUE;

        i++;
    }

    return aModName;
}

//----------------------------------------------------------------------------

::rtl::OUString BasicIDE::CreateModule( SfxObjectShell* pShell, const String& rLibName, const String& rModName, BOOL bCreateMain )
    throw(ElementExistException, NoSuchElementException)
{
    // get library
    Reference< container::XNameContainer > xLib = GetModuleLibrary( pShell, rLibName, TRUE );

    // create module
    ::rtl::OUString aOUSource;
    ::rtl::OUString aOUModName( rModName );
    if( xLib.is() && !xLib->hasByName( aOUModName ) )
    {
        // create new module
        aOUSource = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "REM  *****  BASIC  *****\n\n" ) );
        if ( bCreateMain )
            aOUSource += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Sub Main\n\nEnd Sub" ) );

        // insert module into library
        Any aElement;
        aElement <<= aOUSource;
        xLib->insertByName( aOUModName, aElement );

        // doc shell modified
        BasicIDE::MarkDocShellModified( pShell );   // here?
    }
    else
    {
        throw ElementExistException(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("BasicIDE::CreateModule: ElementExistException!") ),
            Reference<XInterface>() );
    }

    return aOUSource;
}

//----------------------------------------------------------------------------

void BasicIDE::RenameModule( SfxObjectShell* pShell, const String& rLibName, const String& rOldName, const String& rNewName )
    throw(ElementExistException, NoSuchElementException)
{
    ::rtl::OUString aOUOldName( rOldName );
    ::rtl::OUString aOUNewName( rNewName );

    // get library
    Reference< container::XNameContainer > xLib = GetModuleLibrary( pShell, rLibName, TRUE );

    // rename module
    if( xLib.is() && xLib->hasByName( aOUOldName ) )
    {
        if ( xLib->hasByName( aOUNewName ) )
        {
            throw ElementExistException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("BasicIDE::RenameModule: ElementExistException!") ),
                Reference<XInterface>() );
        }

        // get module
        Any aElement = xLib->getByName( aOUOldName );

        // remove module from module container
        xLib->removeByName( aOUOldName );

        // insert module by new name in module container
        xLib->insertByName( aOUNewName, aElement );
    }
    else
    {
        throw NoSuchElementException(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("BasicIDE::RenameModule: NoSuchElementException!") ),
            Reference<XInterface>() );
    }

    BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
    if ( pIDEShell )
    {
        IDEBaseWindow* pWin = pIDEShell->FindWindow( pShell, rLibName, rOldName, BASICIDE_TYPE_MODULE, FALSE );
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
}

//----------------------------------------------------------------------------

void BasicIDE::RemoveModule( SfxObjectShell* pShell, const String& rLibName, const String& rModName )
    throw(NoSuchElementException)
{
    // get library
    Reference< container::XNameContainer > xLib = GetModuleLibrary( pShell, rLibName, TRUE );

    // remove module
    ::rtl::OUString aOUModName( rModName );
    if( xLib.is() && xLib->hasByName( aOUModName ) )
    {
        xLib->removeByName( aOUModName );
    }
    else
    {
        throw NoSuchElementException(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("BasicIDE::RemoveModule: NoSuchElementException!") ),
            Reference<XInterface>() );
    }
}

//----------------------------------------------------------------------------

void BasicIDE::InsertModule( SfxObjectShell* pShell, const String& rLibName, const String& rModName, ::rtl::OUString& aModule )
    throw(ElementExistException, NoSuchElementException)
{
    // get library
    Reference< container::XNameContainer > xLib;
    if ( BasicIDE::HasModuleLibrary( pShell, rLibName ) )
        xLib = GetModuleLibrary( pShell, rLibName, TRUE );
    else
        xLib = BasicIDE::CreateModuleLibrary( pShell, rLibName );

    // insert module into library
    ::rtl::OUString aOUModName( rModName );
    if( xLib.is() && !xLib->hasByName( aOUModName ) )
    {
        Any aElement;
        aElement <<= aModule;
        xLib->insertByName( aOUModName, aElement );
    }
    else
    {
        throw ElementExistException(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("BasicIDE::InsertModule: ElementExistException!") ),
            Reference<XInterface>() );
    }
}

//----------------------------------------------------------------------------

void BasicIDE::UpdateModule( SfxObjectShell* pShell, const String& rLibName, const String& rModName, ::rtl::OUString& aModule )
    throw(NoSuchElementException)
{
    // get library
    Reference< container::XNameContainer > xLib = GetModuleLibrary( pShell, rLibName, TRUE );

    // update module in library
    ::rtl::OUString aOUModName( rModName );
    if ( xLib.is() && xLib->hasByName( aOUModName ) )
    {
        Any aElement;
        aElement <<= aModule;
        xLib->replaceByName( aOUModName, aElement );
    }
    else
    {
        throw NoSuchElementException(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("BasicIDE::UpdateModule: NoSuchElementException!") ),
            Reference<XInterface>() );
    }
}

//----------------------------------------------------------------------------

::rtl::OUString BasicIDE::ChooseMacro( BOOL bExecute, BOOL bChooseOnly, const ::rtl::OUString& rMacroDesc )
{
    BasicIDEDLL::Init();

    IDE_DLL()->GetExtraData()->ChoosingMacro() = TRUE;
    SFX_APP()->EnterBasicCall();

    String aScriptURL;
    BOOL bError = FALSE;
    SbMethod* pMethod = NULL;

    MacroChooser* pChooser = new MacroChooser( NULL, TRUE );
    if ( bChooseOnly || !SvtModuleOptions().IsBasicIDE() )
        pChooser->SetMode( MACROCHOOSER_CHOOSEONLY );

    if ( !bChooseOnly && !bExecute )
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
                            SfxObjectShell* pShell = BasicIDE::FindDocShell( pBasMgr );
                            if ( pShell )
                            {
                                // document basic
                                aLocation = String::CreateFromAscii("document");

                                SfxObjectShell* pCurrShell = SfxObjectShell::Current();
                                if ( !bExecute && pShell != pCurrShell )
                                {
                                    // error
                                    bError = TRUE;
                                    ErrorBox( NULL, WB_OK | WB_DEF_OK, String( IDEResId( RID_STR_ERRORCHOOSEMACRO ) ) ).Execute();
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

            if ( pMethod && bExecute )
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

Sequence< ::rtl::OUString > BasicIDE::GetMethodNames( SfxObjectShell* pShell, const String& rLibName, const String& rModName )
    throw(NoSuchElementException )
{
    // get module
    ::rtl::OUString aOUSource = GetModule( pShell, rLibName, rModName );

    SbModuleRef xModule = new SbModule( rModName );
    xModule->SetSource32( aOUSource );
    USHORT nCount = xModule->GetMethods()->Count();
    Sequence< ::rtl::OUString > aSeqMethods( nCount );

    for ( USHORT i = 0; i < nCount; i++ )
    {
        SbMethod* pMethod = (SbMethod*)xModule->GetMethods()->Get( i );
        DBG_ASSERT( pMethod, "Method not found! (NULL)" );
        aSeqMethods.getArray()[ i ] = pMethod->GetName();
    }

    return aSeqMethods;
}

//----------------------------------------------------------------------------

BOOL BasicIDE::HasMethod( SfxObjectShell* pShell, const String& rLibName, const String& rModName, const String& rMethName )
{
    BOOL bHasMethod = FALSE;

    if ( HasModule( pShell, rLibName, rModName ) )
    {
        ::rtl::OUString aOUSource = GetModule( pShell, rLibName, rModName );
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
