/*************************************************************************
 *
 *  $RCSfile: basobj3.cxx,v $
 *
 *  $Revision: 1.28 $
 *
 *  last change: $Author: kz $ $Date: 2004-07-23 12:03:28 $
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

#ifndef _SFX_IPFRM_HXX
#include <sfx2/ipfrm.hxx>
#endif

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

#include <iderdll.hxx>
#include <iderdll2.hxx>
#include <iderid.hxx>
#include <basobj.hxx>
#include <basidesh.hxx>
#include <basidesh.hrc>
#include <objdlg.hxx>
#include <bastypes.hxx>
#include <basdoc.hxx>

#include <baside2.hxx>
#include <baside3.hxx>
#include <basicmod.hxx>

#ifndef _BASCTL_DLGED_HXX
#include "dlged.hxx"
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XLIBRYARYCONTAINER_HPP_
#include <com/sun/star/script/XLibraryContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XLIBRYARYCONTAINER2_HPP_
#include <com/sun/star/script/XLibraryContainer2.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XLIBRARYCONTAINERPASSWORD_HPP_
#include <com/sun/star/script/XLibraryContainerPassword.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_URI_XURIREFERENCE_HPP_
#include <com/sun/star/uri/XUriReference.hpp>
#endif
#ifndef _COM_SUN_STAR_URI_XURIREFERENCEFACTORY_HPP_
#include <com/sun/star/uri/XUriReferenceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMACROEXPANDER_HPP_
#include <com/sun/star/util/XMacroExpander.hpp>
#endif

#ifndef _XMLSCRIPT_XMLDLG_IMEXP_HXX_
#include <xmlscript/xmldlg_imexp.hxx>
#endif
#ifndef _RTL_URI_HXX_
#include <rtl/uri.hxx>
#endif
#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

using namespace comphelper;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;


//----------------------------------------------------------------------------

extern "C" {
    long basicide_handle_basic_error( void* pPtr )
    {
        return BasicIDE::HandleBasicError( (StarBASIC*)pPtr );
    }
}

//----------------------------------------------------------------------------

SbMethod* BasicIDE::CreateMacro( SbModule* pModule, const String& rMacroName )
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
            BOOL bValid = FALSE;
            String aStdMacroText( RTL_CONSTASCII_USTRINGPARAM( "Macro" ) );
            //String aStdMacroText( IDEResId( RID_STR_STDMACRONAME ) );
            USHORT nMacro = 1;
            while ( !bValid )
            {
                aMacroName = aStdMacroText;
                aMacroName += String::CreateFromInt32( nMacro );
                // Pruefen, ob vorhanden...
                bValid = pModule->GetMethods()->Find( aMacroName, SbxCLASS_METHOD ) ? FALSE : TRUE;
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
    SfxObjectShell* pShell = 0;
    SbxObject* pParent = pModule->GetParent();
    StarBASIC* pBasic = PTR_CAST(StarBASIC,pParent);
    DBG_ASSERT(pBasic, "BasicIDE::CreateMacro: No Basic found!");
    if ( pBasic )
    {
        BasicManager* pBasMgr = BasicIDE::FindBasicManager( pBasic );
        DBG_ASSERT(pBasMgr, "BasicIDE::CreateMacro: No BasicManager found!");
        if ( pBasMgr )
        {
            pShell = BasicIDE::FindDocShell( pBasMgr );
            String aLibName = pBasic->GetName();
            String aModName = pModule->GetName();
            BasicIDE::UpdateModule( pShell, aLibName, aModName, aOUSource );
        }
    }

    SbMethod* pMethod = (SbMethod*)pModule->GetMethods()->Find( aMacroName, SbxCLASS_METHOD );

    if( pDispatcher )
    {
        pDispatcher->Execute( SID_BASICIDE_UPDATEALLMODULESOURCES );
    }

    BasicIDE::MarkDocShellModified( pShell );

    return pMethod;
}

//----------------------------------------------------------------------------

Reference< script::XLibraryContainer > BasicIDE::GetDialogLibraryContainer( SfxObjectShell* pShell )
{
    // get library container
    Reference< script::XLibraryContainer > xLibContainer;
    if( pShell )
    {
        // document
        xLibContainer = Reference< script::XLibraryContainer >( pShell->GetDialogContainer(), UNO_QUERY );
    }
    else
    {
        // application
        xLibContainer = Reference< script::XLibraryContainer >( SFX_APP()->GetDialogContainer(), UNO_QUERY );
    }

    return xLibContainer;
}

//----------------------------------------------------------------------------

BOOL BasicIDE::HasDialogLibrary( SfxObjectShell* pShell, const String& rLibName )
{
    BOOL bHasDialogLibrary = FALSE;

    // get library container
    Reference< script::XLibraryContainer > xLibContainer = GetDialogLibraryContainer( pShell );

    // check if library container has dialog library
    ::rtl::OUString aOULibName( rLibName );
    if( xLibContainer.is() && xLibContainer->hasByName( aOULibName ) )
    {
        bHasDialogLibrary = TRUE;
    }

    return bHasDialogLibrary;
}

//----------------------------------------------------------------------------

Reference< container::XNameContainer > BasicIDE::GetDialogLibrary( SfxObjectShell* pShell, const String& rLibName, BOOL bLoadLibrary )
    throw(NoSuchElementException)
{
    // get library container
    Reference< script::XLibraryContainer > xLibContainer = GetDialogLibraryContainer( pShell );

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
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("BasicIDE::GetDialogLibrary: NoSuchElementException!") ),
            Reference<XInterface>() );
    }

    // load library
    if( bLoadLibrary && !xLibContainer->isLibraryLoaded( aOULibName ) )
        xLibContainer->loadLibrary( aOULibName );

    return xLib;
}

//----------------------------------------------------------------------------

Reference< ::com::sun::star::container::XNameContainer > BasicIDE::CreateDialogLibrary( SfxObjectShell* pShell, const String& rLibName )
    throw(ElementExistException)
{
    // get library container
    Reference< script::XLibraryContainer > xLibContainer = GetDialogLibraryContainer( pShell );

    // create dialog library
    Reference< container::XNameContainer > xLib;
    ::rtl::OUString aOULibName( rLibName );
    if( xLibContainer.is() && !xLibContainer->hasByName( aOULibName ) )
    {
        xLib = xLibContainer->createLibrary( aOULibName );
    }
    else
    {
        throw ElementExistException(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("BasicIDE::CreateDialogLibrary: ElementExistException!") ),
            Reference<XInterface>() );
    }

    return xLib;
}

//----------------------------------------------------------------------------

Sequence< ::rtl::OUString > BasicIDE::GetDialogNames( SfxObjectShell* pShell, const String& rLibName )
    throw(NoSuchElementException)
{
    Sequence< ::rtl::OUString > aSeqDlgNames;
    Reference< container::XNameContainer > xLib = GetDialogLibrary( pShell, rLibName );
    if( xLib.is() )
    {
        Sequence< ::rtl::OUString > aDlgNames = xLib->getElementNames();
        sal_Int32 nDlgCount = aDlgNames.getLength();
        const ::rtl::OUString* pDlgNames = aDlgNames.getConstArray();

        // sort dialog names
        ::std::vector<String> aDlgList( nDlgCount );
        sal_Int32 i;

        for ( i = 0 ; i < nDlgCount ; i++ )
            aDlgList[ i ] = pDlgNames[ i ];
        ::std::sort( aDlgList.begin() , aDlgList.end() , StringCompareLessThan );

        // copy to sequence
        aSeqDlgNames.realloc( nDlgCount );
        for ( i = 0 ; i < nDlgCount ; i++ )
            aSeqDlgNames.getArray()[ i ] = aDlgList[ i ];
    }

    return aSeqDlgNames;
}

//----------------------------------------------------------------------------

BOOL BasicIDE::HasDialog( SfxObjectShell* pShell, const String& rLibName, const String& rDlgName )
{
    BOOL bHasDialog = FALSE;

    // get library container
    Reference< script::XLibraryContainer > xLibContainer = GetDialogLibraryContainer( pShell );

    // check if library container has dialog library
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

        // check if library has dialog
        ::rtl::OUString aOUDlgName( rDlgName );
        if ( xLib.is() && xLib->hasByName( aOUDlgName ) )
            bHasDialog = TRUE;
    }

    return bHasDialog;
}

//----------------------------------------------------------------------------

Reference< io::XInputStreamProvider > BasicIDE::GetDialog( SfxObjectShell* pShell, const String& rLibName, const String& rDlgName )
    throw(NoSuchElementException)
{
    // get library
    Reference< container::XNameContainer > xLib = GetDialogLibrary( pShell, rLibName, TRUE );

    // get dialog
    Reference< io::XInputStreamProvider > xISP;
    ::rtl::OUString aOUDlgName( rDlgName );
    if( xLib.is() && xLib->hasByName( aOUDlgName ) )
    {
        Any aElement = xLib->getByName( aOUDlgName );
        aElement >>= xISP;
    }
    else
    {
        throw NoSuchElementException(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("BasicIDE::GetDialog: NoSuchElementException!") ),
            Reference<XInterface>() );
    }

    return xISP;
}

//----------------------------------------------------------------------------

String BasicIDE::CreateDialogName( SfxObjectShell* pShell, const String& rLibName )
{
    String aDlgName;
    String aDlgStdName( RTL_CONSTASCII_USTRINGPARAM( "Dialog" ) );
    //String aDlgStdName( IDEResId( RID_STR_STDDIALOGNAME ) );
    BOOL bValid = FALSE;
    USHORT i = 1;
    while ( !bValid )
    {
        aDlgName = aDlgStdName;
        aDlgName += String::CreateFromInt32( i );
        if ( !BasicIDE::HasDialog( pShell, rLibName, aDlgName ) )
            bValid = TRUE;

        i++;
    }

    return aDlgName;
}

//----------------------------------------------------------------------------

Reference< io::XInputStreamProvider > BasicIDE::CreateDialog( SfxObjectShell* pShell, const String& rLibName, const String& rDlgName )
    throw(ElementExistException, NoSuchElementException)
{
    // get library
    Reference< container::XNameContainer > xLib = GetDialogLibrary( pShell, rLibName, TRUE );

    // create dialog
    Reference< io::XInputStreamProvider > xISP;
    ::rtl::OUString aOUDlgName( rDlgName );
    if( xLib.is() && !xLib->hasByName( aOUDlgName ) )
    {
        // create new dialog model
        Reference< lang::XMultiServiceFactory > xMSF = getProcessServiceFactory();
        Reference< container::XNameContainer > xDialogModel( xMSF->createInstance
            ( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlDialogModel" ) ) ), UNO_QUERY );

        // set name property
        Reference< beans::XPropertySet > xDlgPSet( xDialogModel, UNO_QUERY );
        Any aName;
        aName <<= aOUDlgName;
        xDlgPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Name" ) ), aName );

        // export dialog model
        Reference< XComponentContext > xContext;
        Reference< beans::XPropertySet > xProps( xMSF, UNO_QUERY );
        OSL_ASSERT( xProps.is() );
        OSL_VERIFY( xProps->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DefaultContext")) ) >>= xContext );
        xISP = ::xmlscript::exportDialogModel( xDialogModel, xContext );
        Any aElement;
        aElement <<= xISP;

        // insert dialog into library
        xLib->insertByName( aOUDlgName, aElement );

        // doc shell modified
        BasicIDE::MarkDocShellModified( pShell );   // here?
    }
    else
    {
        throw ElementExistException(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("BasicIDE::CreateDialog: ElementExistException!") ),
            Reference<XInterface>() );
    }

    return xISP;
}

//----------------------------------------------------------------------------

void BasicIDE::RenameDialog( SfxObjectShell* pShell, const String& rLibName, const String& rOldName, const String& rNewName )
    throw(ElementExistException, NoSuchElementException)
{
    ::rtl::OUString aOUOldName( rOldName );
    ::rtl::OUString aOUNewName( rNewName );

    // get library
    Reference< container::XNameContainer > xLib = GetDialogLibrary( pShell, rLibName, TRUE );

    // rename dialog
    if( xLib.is() && xLib->hasByName( aOUOldName ) )
    {
        if ( xLib->hasByName( aOUNewName ) )
        {
            throw ElementExistException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("BasicIDE::RenameDialog: ElementExistException!") ),
                Reference<XInterface>() );
        }

        // get dialog
        Any aElement = xLib->getByName( aOUOldName );

        // remove dialog from dialog container
        xLib->removeByName( aOUOldName );

        // create dialog model
        Reference< lang::XMultiServiceFactory > xMSF = getProcessServiceFactory();
        Reference< container::XNameContainer > xDialogModel( xMSF->createInstance
            ( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlDialogModel" ) ) ), UNO_QUERY );

        Reference< io::XInputStreamProvider > xISP;
        aElement >>= xISP;
        if( xISP.is() )
        {
            // import dialog model
            Reference< XComponentContext > xContext;
            Reference< beans::XPropertySet > xProps( xMSF, UNO_QUERY );
            OSL_ASSERT( xProps.is() );
            OSL_VERIFY( xProps->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DefaultContext")) ) >>= xContext );
            Reference< io::XInputStream > xInput( xISP->createInputStream() );
            ::xmlscript::importDialogModel( xInput, xDialogModel, xContext );

            // set new name as property
            Reference< beans::XPropertySet > xDlgPSet( xDialogModel, UNO_QUERY );
            Any aName;
            aName <<= aOUNewName;
            xDlgPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Name" ) ), aName );

            // export dialog model
            xISP = ::xmlscript::exportDialogModel( xDialogModel, xContext );
            aElement <<= xISP;
        }

        // insert dialog by new name in dialog container
        xLib->insertByName( aOUNewName, aElement );
    }
    else
    {
        throw NoSuchElementException(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("BasicIDE::RenameDialog: NoSuchElementException!") ),
            Reference<XInterface>() );
    }

    BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
    if ( pIDEShell )
    {
        IDEBaseWindow* pWin = pIDEShell->FindWindow( pShell, rLibName, rOldName, BASICIDE_TYPE_DIALOG, FALSE );
        if ( pWin )
        {
            // set new name in window
            pWin->SetName( rNewName );

            // get dialog model from dialog editor and set new name as property
            Reference< container::XNameContainer > xDlgModel = ((DialogWindow*)pWin)->GetEditor()->GetDialog();
            if( xDlgModel.is() )
            {
                Reference< beans::XPropertySet > xPSet( xDlgModel, UNO_QUERY );
                Any aName;
                aName <<= aOUNewName;
                xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Name" ) ), aName );
            }

            // update property browser
            ((DialogWindow*)pWin)->UpdateBrowser();

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

void BasicIDE::RemoveDialog( SfxObjectShell* pShell, const String& rLibName, const String& rDlgName )
    throw(NoSuchElementException)
{
    // get library
    Reference< container::XNameContainer > xLib = GetDialogLibrary( pShell, rLibName, TRUE );

    // remove dialog
    ::rtl::OUString aOUDlgName( rDlgName );
    if( xLib.is() && xLib->hasByName( aOUDlgName ) )
    {
        xLib->removeByName( aOUDlgName );
    }
    else
    {
        throw NoSuchElementException(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("BasicIDE::RemoveDialog: NoSuchElementException!") ),
            Reference<XInterface>() );
    }
}

//----------------------------------------------------------------------------

void BasicIDE::InsertDialog( SfxObjectShell* pShell, const String& rLibName, const String& rDlgName,
                             const Reference< io::XInputStreamProvider >& xISP )
    throw(ElementExistException, NoSuchElementException)
{
    // get library
    Reference< container::XNameContainer > xLib;
    if ( BasicIDE::HasDialogLibrary( pShell, rLibName ) )
        xLib = GetDialogLibrary( pShell, rLibName, TRUE );
    else
        xLib = BasicIDE::CreateDialogLibrary( pShell, rLibName );

    // insert dialog into library
    ::rtl::OUString aOUDlgName( rDlgName );
    if( xLib.is() && !xLib->hasByName( aOUDlgName ) )
    {
        Any aElement;
        aElement <<= xISP;
        xLib->insertByName( aOUDlgName, aElement );
    }
    else
    {
        throw ElementExistException(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("BasicIDE::InsertDialog: ElementExistException!") ),
            Reference<XInterface>() );
    }
}

//----------------------------------------------------------------------------

StarBASIC* BasicIDE::FindBasic( const SbxVariable* pVar )
{
    const SbxVariable* pSbx = pVar;
    while ( pSbx && !pSbx->ISA( StarBASIC ) )
        pSbx = pSbx->GetParent();

    DBG_ASSERT( !pSbx || pSbx->ISA( StarBASIC ), "Find Basic: Kein Basic!" );
    return (StarBASIC*)pSbx;
}

//----------------------------------------------------------------------------

BasicManager* BasicIDE::FindBasicManager( StarBASIC* pLib )
{
    BasicManager* pBasicMgr = SFX_APP()->GetBasicManager();
    SfxObjectShell* pDocShell = 0;
    while ( pBasicMgr )
    {
        Sequence< ::rtl::OUString > aLibNames = GetLibraryNames( pDocShell );
        sal_Int32 nLibCount = aLibNames.getLength();
        const ::rtl::OUString* pLibNames = aLibNames.getConstArray();

        for ( sal_Int32 i = 0 ; i < nLibCount ; i++ )
        {
            StarBASIC* pL = pBasicMgr->GetLib( pLibNames[ i ] );
            if ( pL == pLib )
                return pBasicMgr;
        }

        if ( pDocShell  )
            pDocShell = SfxObjectShell::GetNext( *pDocShell );
        else
            pDocShell = SfxObjectShell::GetFirst();

        pBasicMgr = ( pDocShell ? pDocShell->GetBasicManager() : 0 );
    }
    return 0;
}

//----------------------------------------------------------------------------

String BasicIDE::GetTitle( SfxObjectShell* pShell, LibraryLocation eLocation, USHORT nSFXTitleType, LibraryType eType )
{
    String aTitle;

    switch ( eLocation )
    {
        case LIBRARY_LOCATION_USER:
        {
            if ( eType == LIBRARY_TYPE_MODULE )
                aTitle = IDEResId( RID_STR_USERMACROS );
            else if ( eType == LIBRARY_TYPE_DIALOG )
                aTitle = IDEResId( RID_STR_USERDIALOGS );
            else if ( eType == LIBRARY_TYPE_ALL )
                aTitle = IDEResId( RID_STR_USERMACROSDIALOGS );
        }
        break;
        case LIBRARY_LOCATION_SHARE:
        {
            if ( eType == LIBRARY_TYPE_MODULE )
                aTitle = IDEResId( RID_STR_SHAREMACROS );
            else if ( eType == LIBRARY_TYPE_DIALOG )
                aTitle = IDEResId( RID_STR_SHAREDIALOGS );
            else if ( eType == LIBRARY_TYPE_ALL )
                aTitle = IDEResId( RID_STR_SHAREMACROSDIALOGS );
        }
        break;
        case LIBRARY_LOCATION_DOCUMENT:
        {
            if ( pShell )
                aTitle = pShell->GetTitle( nSFXTitleType );
        }
        break;
    }

    return aTitle;
}

//----------------------------------------------------------------------------

SfxObjectShell* BasicIDE::FindDocShell( BasicManager* pBasMgr )
{
    SfxObjectShell* pDocShell = SfxObjectShell::GetFirst();
    while ( pDocShell )
    {
        if ( ( pDocShell->GetBasicManager() != SFX_APP()->GetBasicManager() ) &&
             ( pDocShell->GetBasicManager() == pBasMgr ) )
        {
            return pDocShell;
        }
        pDocShell = SfxObjectShell::GetNext( *pDocShell );
    }
    return 0;
}

//----------------------------------------------------------------------------

void BasicIDE::MarkDocShellModified( SfxObjectShell* pShell )
{
    // Muss ja nicht aus einem Document kommen...
    if ( pShell )
    {
        pShell->SetModified();
    }
    else
    {
        BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
        if ( pIDEShell )
            pIDEShell->SetAppBasicModified();
    }

    SfxBindings* pBindings = BasicIDE::GetBindingsPtr();
    if ( pBindings )
    {
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

void BasicIDE::RunMethod( SbMethod* pMethod )
{
    SbxValues aRes;
    aRes.eType = SbxVOID;
    pMethod->Get( aRes );
}

//----------------------------------------------------------------------------

void BasicIDE::StopBasic()
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

void BasicIDE::BasicStopped( BOOL* pbAppWindowDisabled,
        BOOL* pbDispatcherLocked, USHORT* pnWaitCount,
        SfxUInt16Item** ppSWActionCount, SfxUInt16Item** ppSWLockViewCount )
{
    // Nach einem Error oder dem expliziten abbrechen des Basics muessen
    // ggf. einige Locks entfernt werden...

    if ( pbAppWindowDisabled )
        *pbAppWindowDisabled = FALSE;
    if ( pbDispatcherLocked )
        *pbDispatcherLocked = FALSE;
    if ( pnWaitCount )
        *pnWaitCount = 0;
    if ( ppSWActionCount )
        *ppSWActionCount = 0;
    if ( ppSWLockViewCount )
        *ppSWLockViewCount = 0;

    // AppWait ?
    USHORT nWait = 0;
    BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
    if( pIDEShell )
    {
        while ( pIDEShell->GetViewFrame()->GetWindow().IsWait() )
        {
            pIDEShell->GetViewFrame()->GetWindow().LeaveWait();
            nWait++;
        }
        if ( pnWaitCount )
            *pnWaitCount = nWait;
    }

    // Interactive = FALSE ?
    if ( SFX_APP()->IsDispatcherLocked() )
    {
        SFX_APP()->LockDispatcher( FALSE );
        if ( pbDispatcherLocked )
            *pbDispatcherLocked = TRUE;
    }
    Window* pDefParent = Application::GetDefDialogParent();
    if ( pDefParent && !pDefParent->IsEnabled() )
    {
        pDefParent->Enable( TRUE );
        if ( pbAppWindowDisabled )
            *pbAppWindowDisabled = TRUE;
    }

}

//----------------------------------------------------------------------------

void BasicIDE::InvalidateDebuggerSlots()
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

long BasicIDE::HandleBasicError( StarBASIC* pBasic )
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
            BOOL bProtected = FALSE;
            SfxObjectShell* pShell = BasicIDE::FindDocShell( pBasMgr );
            ::rtl::OUString aOULibName( pBasic->GetName() );
            Reference< script::XLibraryContainer > xModLibContainer( BasicIDE::GetModuleLibraryContainer( pShell ), UNO_QUERY );
            if ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) )
            {
                Reference< script::XLibraryContainerPassword > xPasswd( xModLibContainer, UNO_QUERY );
                if ( xPasswd.is() && xPasswd->isLibraryPasswordProtected( aOULibName ) && !xPasswd->isLibraryPasswordVerified( aOULibName ) )
                {
                    bProtected = TRUE;
                }
            }

            if ( !bProtected )
            {
                pIDEShell = IDE_DLL()->GetShell();
                if ( !pIDEShell )
                {
                    SfxViewFrame* pViewFrame = SfxViewFrame::Current();
                    SfxDispatcher* pDispatcher = ( pViewFrame && !pViewFrame->ISA( SfxInPlaceFrame ) ) ? pViewFrame->GetDispatcher() : NULL;
                    if ( pDispatcher )
                    {
                        pDispatcher->Execute( SID_BASICIDE_APPEAR, SFX_CALLMODE_SYNCHRON );
                    }
                    else
                    {
                        SfxAllItemSet aArgs( SFX_APP()->GetPool() );
                        SfxRequest aRequest( SID_BASICIDE_APPEAR, SFX_CALLMODE_SYNCHRON, aArgs );
                        SFX_APP()->ExecuteSlot( aRequest );
                    }
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

SfxBindings& BasicIDE::GetBindings()
{
    BasicIDEDLL* pIDEDLL = IDE_DLL();
    if ( pIDEDLL && pIDEDLL->GetShell() )
    {
        // #63960# fuer BasicIDE die Bindings der APP
        // 07/00 Now the APP Dispatcher is 'dead', SFX changes...
//      return SFX_APP()->GetAppBindings();
        return pIDEDLL->GetShell()->GetFrame()->GetBindings();
    }
    SfxViewFrame* pCurFrame = SfxViewFrame::Current();
    DBG_ASSERT( pCurFrame != NULL, "No current view frame!" );
    return pCurFrame->GetBindings();
}

//----------------------------------------------------------------------------

SfxBindings* BasicIDE::GetBindingsPtr()
{
    SfxViewFrame* pFrame;
    BasicIDEDLL* pIDEDLL = IDE_DLL();
    if ( pIDEDLL && pIDEDLL->GetShell() )
        pFrame = pIDEDLL->GetShell()->GetFrame();
    else
        pFrame = SfxViewFrame::Current();

    SfxBindings* pBindings = NULL;
    if( pFrame != NULL )
        pBindings = &pFrame->GetBindings();
    return pBindings;
}

//----------------------------------------------------------------------------

bool BasicIDE::IsLibraryShared( SfxObjectShell* pShell, const String& rLibName, LibraryType eType )
{
    bool bIsShared = false;
    Reference< script::XLibraryContainer2 > xLibContainer;
    if ( eType == LIBRARY_TYPE_MODULE )
        xLibContainer.set( BasicIDE::GetModuleLibraryContainer( pShell ), UNO_QUERY );
    else if ( eType == LIBRARY_TYPE_DIALOG )
        xLibContainer.set( BasicIDE::GetDialogLibraryContainer( pShell ), UNO_QUERY );

    ::rtl::OUString aOULibName( rLibName );
    if ( xLibContainer.is() && xLibContainer->hasByName( aOULibName ) && xLibContainer->isLibraryLink( aOULibName ) )
    {
        ::rtl::OUString aFileURL;
        Reference< lang::XMultiServiceFactory > xMSF( ::comphelper::getProcessServiceFactory() );
        Reference< uri::XUriReferenceFactory > xUriFac;
        if ( xMSF.is() )
        {
            xUriFac.set( xMSF->createInstance( ::rtl::OUString::createFromAscii(
                "com.sun.star.uri.UriReferenceFactory") ) , UNO_QUERY );
        }

        if ( xUriFac.is() )
        {
            ::rtl::OUString aLinkURL( xLibContainer->getLibraryLinkURL( aOULibName ) );
            Reference<  uri::XUriReference > xUriRef( xUriFac->parse( aLinkURL ), UNO_QUERY );

            if ( xUriRef.is() )
            {
                ::rtl::OUString aScheme = xUriRef->getScheme();
                if ( aScheme.equalsIgnoreAsciiCaseAscii( "file" ) )
                {
                    aFileURL = aLinkURL;
                }
                else if ( aScheme.equalsIgnoreAsciiCaseAscii( "vnd.sun.star.pkg" ) )
                {
                    ::rtl::OUString aAuthority = xUriRef->getAuthority();
                    if ( aAuthority.matchIgnoreAsciiCaseAsciiL( RTL_CONSTASCII_STRINGPARAM( "vnd.sun.star.expand:" ) ) )
                    {
                        ::rtl::OUString aDecodedURL( aAuthority.copy( sizeof ( "vnd.sun.star.expand:" ) - 1 ) );
                        aDecodedURL = ::rtl::Uri::decode( aDecodedURL, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8 );
                        Reference< XComponentContext > xContext;
                        Reference< beans::XPropertySet > xProps( xMSF, UNO_QUERY );
                        OSL_ASSERT( xProps.is() );
                        OSL_VERIFY( xProps->getPropertyValue( ::rtl::OUString::createFromAscii( "DefaultContext" ) ) >>= xContext );
                        if ( xContext.is() )
                        {
                            Reference<util::XMacroExpander> xMacroExpander(
                                xContext->getValueByName(
                                ::rtl::OUString::createFromAscii( "/singletons/com.sun.star.util.theMacroExpander" ) ),
                                UNO_QUERY );
                            if ( xMacroExpander.is() )
                                aFileURL = xMacroExpander->expandMacros( aDecodedURL );
                        }
                    }
                }
            }
        }

        if ( aFileURL.getLength() )
        {
            osl::DirectoryItem aFileItem;
            osl::FileStatus aFileStatus( FileStatusMask_FileURL );
            OSL_VERIFY( osl::DirectoryItem::get( aFileURL, aFileItem ) == osl::FileBase::E_None );
            OSL_VERIFY( aFileItem.getFileStatus( aFileStatus ) == osl::FileBase::E_None );
            ::rtl::OUString aCanonicalFileURL( aFileStatus.getFileURL() );

            ::rtl::OUString aShareURL;
            OSL_VERIFY( osl_getExecutableFile( &aShareURL.pData ) == osl_Process_E_None );
            sal_Int32 nIndex = aShareURL.lastIndexOf( '/' );
            if ( nIndex >= 0 )
            {
                nIndex = aShareURL.lastIndexOf( '/', nIndex );
                if ( nIndex >= 0 )
                {
                    aShareURL = aShareURL.copy( 0, nIndex + 1 );
                    aShareURL += ::rtl::OUString::createFromAscii( "share" );
                }
            }

            osl::DirectoryItem aShareItem;
            osl::FileStatus aShareStatus( FileStatusMask_FileURL );
            OSL_VERIFY( osl::DirectoryItem::get( aShareURL, aShareItem ) == osl::FileBase::E_None );
            OSL_VERIFY( aShareItem.getFileStatus( aShareStatus ) == osl::FileBase::E_None );
            ::rtl::OUString aCanonicalShareURL( aShareStatus.getFileURL() );

            if ( aCanonicalFileURL.match( aCanonicalShareURL ) )
                bIsShared = true;
        }
    }

    return bIsShared;
}

//----------------------------------------------------------------------------

LibraryLocation BasicIDE::GetLibraryLocation( SfxObjectShell* pShell, const String& rLibName )
{
    LibraryLocation eLocation = LIBRARY_LOCATION_UNKNOWN;

    if ( rLibName.Len() )
    {
        if ( pShell )
        {
            eLocation = LIBRARY_LOCATION_DOCUMENT;
        }
        else
        {
            if ( ( HasModuleLibrary( pShell, rLibName ) && !IsLibraryShared( pShell, rLibName, LIBRARY_TYPE_MODULE ) ) ||
                 ( HasDialogLibrary( pShell, rLibName ) && !IsLibraryShared( pShell, rLibName, LIBRARY_TYPE_DIALOG ) ) )
            {
                eLocation = LIBRARY_LOCATION_USER;
            }
            else
            {
                eLocation = LIBRARY_LOCATION_SHARE;
            }
        }
    }

    return eLocation;
}

//----------------------------------------------------------------------------
