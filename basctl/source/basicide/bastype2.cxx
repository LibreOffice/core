/*************************************************************************
 *
 *  $RCSfile: bastype2.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: kz $ $Date: 2004-07-23 12:03:42 $
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

#include <memory>

#include "vcl/bitmap.hxx"

#include <ide_pch.hxx>

#pragma hdrstop

#include <basidesh.hrc>
#include <bastypes.hxx>
#include <bastype2.hxx>
#include <basobj.hxx>
#include <baside2.hrc>
#include <iderid.hxx>

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _SBXCLASS_HXX //autogen
#include <svtools/sbx.hxx>
#endif
#ifndef _SVTOOLS_IMAGEMGR_HXX
#include <svtools/imagemgr.hxx>
#endif

#ifndef _COM_SUN_STAR_SCRIPT_XLIBRARYCONTAINER_HPP_
#include <com/sun/star/script/XLibraryContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XLIBRARYCONTAINERPASSWORD_HPP_
#include <com/sun/star/script/XLibraryContainerPassword.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_FRAME_XMODULEMANAGER_HPP_
#include <drafts/com/sun/star/frame/XModuleManager.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;


BasicEntry::~BasicEntry()
{
}

BasicShellEntry::BasicShellEntry( SfxObjectShell* pShell, LibraryLocation eLocation, BasicEntryType eType )
    :BasicEntry( eType )
    ,m_pShell( pShell )
    ,m_eLocation( eLocation )
{
}

BasicShellEntry::~BasicShellEntry()
{
}

BasicLibEntry::BasicLibEntry( SfxObjectShell* pShell, LibraryLocation eLocation, const String& rLibName, BasicEntryType eType )
    :BasicShellEntry( pShell, eLocation, eType )
    ,m_aLibName( rLibName )
{
}

BasicLibEntry::~BasicLibEntry()
{
}

BasicEntryDescriptor::BasicEntryDescriptor()
    :m_pShell( 0 )
    ,m_eLocation( LIBRARY_LOCATION_UNKNOWN )
    ,m_eType( OBJ_TYPE_UNKNOWN )
{
}

BasicEntryDescriptor::BasicEntryDescriptor( SfxObjectShell* pShell, LibraryLocation eLocation, const String& rLibName, const String& rName, BasicEntryType eType )
    :m_pShell( pShell )
    ,m_eLocation( eLocation )
    ,m_aLibName( rLibName )
    ,m_aName( rName )
    ,m_eType( eType )
{
}

BasicEntryDescriptor::BasicEntryDescriptor( SfxObjectShell* pShell, LibraryLocation eLocation, const String& rLibName, const String& rName, const String& rMethodName, BasicEntryType eType )
    :m_pShell( pShell )
    ,m_eLocation( eLocation )
    ,m_aLibName( rLibName )
    ,m_aName( rName )
    ,m_aMethodName( rMethodName )
    ,m_eType( eType )
{
}

BasicEntryDescriptor::~BasicEntryDescriptor()
{
}

BasicEntryDescriptor::BasicEntryDescriptor( const BasicEntryDescriptor& rDesc )
    :m_pShell( rDesc.m_pShell )
    ,m_eLocation( rDesc.m_eLocation )
    ,m_aLibName( rDesc.m_aLibName )
    ,m_aName( rDesc.m_aName )
    ,m_aMethodName( rDesc.m_aMethodName )
    ,m_eType( rDesc.m_eType )
{
}

BasicEntryDescriptor& BasicEntryDescriptor::operator=( const BasicEntryDescriptor& rDesc )
{
    m_pShell = rDesc.m_pShell;
    m_eLocation = rDesc.m_eLocation;
    m_aLibName = rDesc.m_aLibName;
    m_aName = rDesc.m_aName;
    m_aMethodName = rDesc.m_aMethodName;
    m_eType = rDesc.m_eType;

    return *this;
}

bool BasicEntryDescriptor::operator==( const BasicEntryDescriptor& rDesc ) const
{
    return m_pShell == rDesc.m_pShell &&
           m_eLocation == rDesc.m_eLocation &&
           m_aLibName == rDesc.m_aLibName &&
           m_aName == rDesc.m_aName &&
           m_aMethodName == rDesc.m_aMethodName &&
           m_eType == rDesc.m_eType;
}

BasicTreeListBox::BasicTreeListBox( Window* pParent, const ResId& rRes ) :
    SvTreeListBox( pParent, IDEResId( rRes.GetId() ) )
{
    SetNodeDefaultImages();
    SetSelectionMode( SINGLE_SELECTION );
    nMode = 0xFF;   // Alles
}



BasicTreeListBox::~BasicTreeListBox()
{
    // UserDaten zerstoeren
    SvLBoxEntry* pEntry = First();
    while ( pEntry )
    {
        delete (BasicEntry*)pEntry->GetUserData();
        pEntry = Next( pEntry );
    }
}

void BasicTreeListBox::ScanEntry( SfxObjectShell* pShell, LibraryLocation eLocation )
{
    // can be called multiple times for updating!

    // eigentlich prueffen, ob Basic bereits im Baum ?!
    SetUpdateMode( FALSE );

    // level 1: BasicManager (application, document, ...)
    String aRootName( GetRootEntryName( pShell, eLocation ) );
    SvLBoxEntry* pShellRootEntry = FindEntry( 0, aRootName, OBJ_TYPE_SHELL );
    if ( pShellRootEntry )
    {
        // #94812 For equally named documents equally named entries are required
        BasicEntryDescriptor aDesc( GetEntryDescriptor( pShellRootEntry ) );
        SfxObjectShell* pEntryShell = aDesc.GetShell();
        if( pEntryShell != pShell )
            pShellRootEntry = NULL;

        if ( pShellRootEntry && IsExpanded( pShellRootEntry ) )
            ImpCreateLibEntries( pShellRootEntry, pShell, eLocation );
    }
    if ( !pShellRootEntry )
    {
        Image aImage;
        Image aImageHC;
        GetRootEntryBitmaps( pShell, aImage, aImageHC );
        pShellRootEntry = AddEntry(
            aRootName,
            aImage,
            aImageHC,
            0, true,
            std::auto_ptr< BasicEntry >( new BasicShellEntry( pShell, eLocation ) ) );
    }

    SetUpdateMode( TRUE );
}

void BasicTreeListBox::ImpCreateLibEntries( SvLBoxEntry* pShellRootEntry, SfxObjectShell* pShell, LibraryLocation eLocation )
{
    // get a sorted list of library names
    Sequence< ::rtl::OUString > aLibNames = BasicIDE::GetLibraryNames( pShell );
    sal_Int32 nLibCount = aLibNames.getLength();
    const ::rtl::OUString* pLibNames = aLibNames.getConstArray();

    for ( sal_Int32 i = 0 ; i < nLibCount ; i++ )
    {
        String aLibName = pLibNames[ i ];

        if ( eLocation == BasicIDE::GetLibraryLocation( pShell, aLibName ) )
        {
            // check, if the module library is loaded
            BOOL bModLibLoaded = FALSE;
            ::rtl::OUString aOULibName( aLibName );
            Reference< script::XLibraryContainer > xModLibContainer = BasicIDE::GetModuleLibraryContainer( pShell );
            if ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) && xModLibContainer->isLibraryLoaded( aOULibName ) )
                bModLibLoaded = TRUE;

            // check, if the dialog library is loaded
            BOOL bDlgLibLoaded = FALSE;
            Reference< script::XLibraryContainer > xDlgLibContainer = BasicIDE::GetDialogLibraryContainer( pShell );
            if ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aOULibName ) && xDlgLibContainer->isLibraryLoaded( aOULibName ) )
                bDlgLibLoaded = TRUE;

            BOOL bLoaded = bModLibLoaded || bDlgLibLoaded;

            // if only one of the libraries is loaded, load also the other
            if ( bLoaded )
            {
                if ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) && !xModLibContainer->isLibraryLoaded( aOULibName ) )
                    xModLibContainer->loadLibrary( aOULibName );

                if ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aOULibName ) && !xDlgLibContainer->isLibraryLoaded( aOULibName ) )
                    xDlgLibContainer->loadLibrary( aOULibName );
            }

            // create tree list box entry
            USHORT nId, nIdHC;
            if ( ( nMode & BROWSEMODE_DIALOGS ) && !( nMode & BROWSEMODE_MODULES ) )
            {
                nId = bLoaded ? RID_IMG_DLGLIB : RID_IMG_DLGLIBNOTLOADED;
                nIdHC = bLoaded ? RID_IMG_DLGLIB_HC : RID_IMG_DLGLIBNOTLOADED_HC;
            }
            else
            {
                nId = bLoaded ? RID_IMG_MODLIB : RID_IMG_MODLIBNOTLOADED;
                nIdHC = bLoaded ? RID_IMG_MODLIB_HC : RID_IMG_MODLIBNOTLOADED_HC;
            }
            SvLBoxEntry* pLibRootEntry = FindEntry( pShellRootEntry, aLibName, OBJ_TYPE_LIBRARY );
            if ( pLibRootEntry )
            {
                SetEntryBitmaps( pLibRootEntry, Image( IDEResId( nId ) ), Image( IDEResId( nIdHC ) ) );
                if ( IsExpanded( pLibRootEntry ) )
                    ImpCreateLibSubEntries( pLibRootEntry, pShell, aLibName );
            }
            else
            {
                pLibRootEntry = AddEntry(
                    aLibName,
                    Image( IDEResId( nId ) ),
                    Image( IDEResId( nIdHC ) ),
                    pShellRootEntry, true,
                    std::auto_ptr< BasicEntry >( new BasicEntry( OBJ_TYPE_LIBRARY ) ) );
            }
        }
    }
}

void BasicTreeListBox::ImpCreateLibSubEntries( SvLBoxEntry* pLibRootEntry, SfxObjectShell* pShell, const String& rLibName )
{
    ::rtl::OUString aOULibName( rLibName );

    // modules
    if ( nMode & BROWSEMODE_MODULES )
    {
        Reference< script::XLibraryContainer > xModLibContainer = BasicIDE::GetModuleLibraryContainer( pShell );

        if ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) && xModLibContainer->isLibraryLoaded( aOULibName ) )
        {
            try
            {
                // get a sorted list of module names
                Sequence< ::rtl::OUString > aModNames = BasicIDE::GetModuleNames( pShell, rLibName );
                sal_Int32 nModCount = aModNames.getLength();
                const ::rtl::OUString* pModNames = aModNames.getConstArray();

                for ( sal_Int32 i = 0 ; i < nModCount ; i++ )
                {
                    String aModName = pModNames[ i ];
                    SvLBoxEntry* pModuleEntry = FindEntry( pLibRootEntry, aModName, OBJ_TYPE_MODULE );
                    if ( !pModuleEntry )
                        pModuleEntry = AddEntry(
                            aModName,
                            Image( IDEResId( RID_IMG_MODULE ) ),
                            Image( IDEResId( RID_IMG_MODULE_HC ) ),
                            pLibRootEntry, false,
                            std::auto_ptr< BasicEntry >( new BasicEntry( OBJ_TYPE_MODULE ) ) );

                    // methods
                    if ( nMode & BROWSEMODE_SUBS )
                    {
                        Sequence< ::rtl::OUString > aNames = BasicIDE::GetMethodNames( pShell, rLibName, aModName );
                        sal_Int32 nCount = aNames.getLength();
                        const ::rtl::OUString* pNames = aNames.getConstArray();

                        for ( sal_Int32 j = 0 ; j < nCount ; j++ )
                        {
                            String aName = pNames[ j ];
                            SvLBoxEntry* pEntry = FindEntry( pModuleEntry, aName, OBJ_TYPE_METHOD );
                            if ( !pEntry )
                                pEntry = AddEntry(
                                    aName,
                                    Image( IDEResId( RID_IMG_MACRO ) ),
                                    Image( IDEResId( RID_IMG_MACRO_HC ) ),
                                    pModuleEntry, false,
                                    std::auto_ptr< BasicEntry >( new BasicEntry( OBJ_TYPE_METHOD ) ) );
                        }
                    }
                }
            }
            catch ( container::NoSuchElementException& e )
            {
                ByteString aBStr( String(e.Message), RTL_TEXTENCODING_ASCII_US );
                DBG_ERROR( aBStr.GetBuffer() );
            }
        }
    }

    // dialogs
    if ( nMode & BROWSEMODE_DIALOGS )
    {
         Reference< script::XLibraryContainer > xDlgLibContainer = BasicIDE::GetDialogLibraryContainer( pShell );

         if ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aOULibName ) && xDlgLibContainer->isLibraryLoaded( aOULibName ) )
         {
            try
            {
                // get a sorted list of dialog names
                Sequence< ::rtl::OUString > aDlgNames = BasicIDE::GetDialogNames( pShell, rLibName );
                sal_Int32 nDlgCount = aDlgNames.getLength();
                const ::rtl::OUString* pDlgNames = aDlgNames.getConstArray();

                for ( sal_Int32 i = 0 ; i < nDlgCount ; i++ )
                {
                    String aDlgName = pDlgNames[ i ];
                    SvLBoxEntry* pDialogEntry = FindEntry( pLibRootEntry, aDlgName, OBJ_TYPE_DIALOG );
                    if ( !pDialogEntry )
                        pDialogEntry = AddEntry(
                            aDlgName,
                            Image( IDEResId( RID_IMG_DIALOG ) ),
                            Image( IDEResId( RID_IMG_DIALOG_HC ) ),
                            pLibRootEntry, false,
                            std::auto_ptr< BasicEntry >( new BasicEntry( OBJ_TYPE_DIALOG ) ) );
                }
            }
            catch ( container::NoSuchElementException& e )
            {
                ByteString aBStr( String(e.Message), RTL_TEXTENCODING_ASCII_US );
                DBG_ERROR( aBStr.GetBuffer() );
            }
        }
    }
}

void BasicTreeListBox::UpdateEntries()
{
    BasicEntryDescriptor aCurDesc( GetEntryDescriptor( FirstSelected() ) );

    // Erstmal die vorhandenen Eintraege auf existens pruefen:
    SvLBoxEntry* pLastValid = 0;
    SvLBoxEntry* pEntry = First();
    while ( pEntry )
    {
        if ( IsValidEntry( pEntry ) )
            pLastValid = pEntry;
        else
        {
            delete (BasicEntry*)pEntry->GetUserData();
            GetModel()->Remove( pEntry );
        }
        pEntry = pLastValid ? Next( pLastValid ) : First();
    }

    // Jetzt ueber die Basics rennen und in die Zweige eintragen
    ScanAllEntries();

    SetCurrentEntry( aCurDesc );
}

SvLBoxEntry* __EXPORT BasicTreeListBox::CloneEntry( SvLBoxEntry* pSource )
{
    SvLBoxEntry* pNew = SvTreeListBox::CloneEntry( pSource );
    BasicEntry* pUser = (BasicEntry*)pSource->GetUserData();

    DBG_ASSERT( pUser, "User-Daten?!" );
    DBG_ASSERT( pUser->GetType() != OBJ_TYPE_SHELL, "Shell?!" );

    BasicEntry* pNewUser = new BasicEntry( *pUser );
    pNew->SetUserData( pNewUser );
    return pNew;
}

SvLBoxEntry* BasicTreeListBox::FindEntry( SvLBoxEntry* pParent, const String& rText, BasicEntryType eType )
{
    ULONG nRootPos = 0;
    SvLBoxEntry* pEntry = pParent ? FirstChild( pParent ) : GetEntry( nRootPos );
    while ( pEntry )
    {
        BasicEntry* pBasicEntry = (BasicEntry*)pEntry->GetUserData();
        DBG_ASSERT( pBasicEntry, "FindEntry: Kein BasicEntry ?!" );
        if ( ( pBasicEntry->GetType() == eType  ) && ( GetEntryText( pEntry ) == rText ) )
            return pEntry;

        pEntry = pParent ? NextSibling( pEntry ) : GetEntry( ++nRootPos );
    }
    return 0;
}

long BasicTreeListBox::ExpandingHdl()
{
    // Expanding oder Collaps?
    BOOL bOK = TRUE;
    if ( GetModel()->GetDepth( GetHdlEntry() ) == 1 )
    {
        SvLBoxEntry* pCurEntry = GetCurEntry();
        BasicEntryDescriptor aDesc( GetEntryDescriptor( pCurEntry ) );
        SfxObjectShell* pShell( aDesc.GetShell() );
        String aLibName( aDesc.GetLibName() );
        String aName( aDesc.GetName() );
        String aMethodName( aDesc.GetMethodName() );

        if ( aLibName.Len() && !aName.Len() && !aMethodName.Len() )
        {
            // check password, if library is password protected and not verified
            ::rtl::OUString aOULibName( aLibName );
            Reference< script::XLibraryContainer > xModLibContainer( BasicIDE::GetModuleLibraryContainer( pShell ), UNO_QUERY );
            if ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) )
            {
                Reference< script::XLibraryContainerPassword > xPasswd( xModLibContainer, UNO_QUERY );
                if ( xPasswd.is() && xPasswd->isLibraryPasswordProtected( aOULibName ) && !xPasswd->isLibraryPasswordVerified( aOULibName ) )
                {
                    String aPassword;
                    bOK = QueryPassword( xModLibContainer, aLibName, aPassword );
                }
            }
         }
    }
    return bOK;
}

BOOL BasicTreeListBox::IsEntryProtected( SvLBoxEntry* pEntry )
{
    BOOL bProtected = FALSE;
    if ( pEntry && ( GetModel()->GetDepth( pEntry ) == 1 ) )
    {
        BasicEntryDescriptor aDesc( GetEntryDescriptor( pEntry ) );
        SfxObjectShell* pShell( aDesc.GetShell() );
        ::rtl::OUString aOULibName( aDesc.GetLibName() );
        Reference< script::XLibraryContainer > xModLibContainer( BasicIDE::GetModuleLibraryContainer( pShell ), UNO_QUERY );
        if ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) )
        {
            Reference< script::XLibraryContainerPassword > xPasswd( xModLibContainer, UNO_QUERY );
            if ( xPasswd.is() && xPasswd->isLibraryPasswordProtected( aOULibName ) && !xPasswd->isLibraryPasswordVerified( aOULibName ) )
            {
                bProtected = TRUE;
            }
        }
    }
    return bProtected;
}

SvLBoxEntry* BasicTreeListBox::AddEntry(
    const String& rText, const Image& rImage, const Image& rImageHC,
    SvLBoxEntry* pParent, bool bChildrenOnDemand, std::auto_ptr< BasicEntry > aUserData )
{
    SvLBoxEntry* p = InsertEntry(
        rText, rImage, rImage, pParent, bChildrenOnDemand, LIST_APPEND,
        aUserData.release() ); // XXX possible leak
    SetExpandedEntryBmp( p, rImageHC, BMP_COLOR_HIGHCONTRAST );
    SetCollapsedEntryBmp( p, rImageHC, BMP_COLOR_HIGHCONTRAST );
    return p;
}

void BasicTreeListBox::SetEntryBitmaps( SvLBoxEntry * pEntry, const Image& rImage, const Image& rImageHC )
{
    SetExpandedEntryBmp( pEntry, rImage, BMP_COLOR_NORMAL );
    SetCollapsedEntryBmp( pEntry, rImage, BMP_COLOR_NORMAL );
    SetExpandedEntryBmp( pEntry, rImageHC, BMP_COLOR_HIGHCONTRAST );
    SetCollapsedEntryBmp( pEntry, rImageHC, BMP_COLOR_HIGHCONTRAST );
}

String BasicTreeListBox::GetRootEntryName( SfxObjectShell* pShell, LibraryLocation eLocation )
{
    LibraryType eType = LIBRARY_TYPE_ALL;
    if ( ( nMode & BROWSEMODE_MODULES ) && !( nMode & BROWSEMODE_DIALOGS ) )
        eType = LIBRARY_TYPE_MODULE;
    else if ( !( nMode & BROWSEMODE_MODULES ) && ( nMode & BROWSEMODE_DIALOGS ) )
        eType = LIBRARY_TYPE_DIALOG;

    return BasicIDE::GetTitle( pShell, eLocation, SFX_TITLE_FILENAME, eType );
}

void BasicTreeListBox::GetRootEntryBitmaps( SfxObjectShell* pShell, Image& rImage, Image& rImageHC )
{
    if ( pShell )
    {
        ::rtl::OUString sFactoryURL;
        Reference< lang::XMultiServiceFactory > xMSF( ::comphelper::getProcessServiceFactory() );
        Reference< ::drafts::com::sun::star::frame::XModuleManager > xModuleManager;
        if ( xMSF.is() )
        {
            xModuleManager.set( xMSF->createInstance( ::rtl::OUString::createFromAscii(
                "drafts.com.sun.star.frame.ModuleManager" ) ) , UNO_QUERY );
        }
        if ( xModuleManager.is() )
        {
            ::rtl::OUString sModule( xModuleManager->identify( pShell->GetModel() ) );
            Reference< container::XNameAccess > xModuleConfig( xModuleManager, UNO_QUERY );
            if ( xModuleConfig.is() )
            {
                Sequence< beans::PropertyValue > aModuleDescr;
                xModuleConfig->getByName( sModule ) >>= aModuleDescr;
                sal_Int32 nCount = aModuleDescr.getLength();
                const beans::PropertyValue* pModuleDescr = aModuleDescr.getConstArray();
                for ( sal_Int32 i = 0; i < nCount; ++i )
                {
                    if ( pModuleDescr[ i ].Name.equalsAsciiL(
                         RTL_CONSTASCII_STRINGPARAM( "ooSetupFactoryEmptyDocumentURL" ) ) )
                    {
                        pModuleDescr[ i ].Value >>= sFactoryURL;
                        break;
                    }
                }
            }
        }

        if ( sFactoryURL.getLength() )
        {
            rImage = SvFileInformationManager::GetFileImage( INetURLObject( sFactoryURL ),
                FALSE /* small */,
                FALSE /* normal */ );

            rImageHC = SvFileInformationManager::GetFileImage( INetURLObject( sFactoryURL ),
                FALSE /* small */,
                TRUE /* high contrast */ );
        }
        else
        {
            // default icon
            rImage = Image( IDEResId( RID_IMG_DOCUMENT ) );
            rImageHC = Image( IDEResId( RID_IMG_DOCUMENT_HC ) );
        }
    }
    else
    {
        rImage = Image( IDEResId( RID_IMG_INSTALLATION ) );
        rImageHC = Image( IDEResId( RID_IMG_INSTALLATION_HC ) );
    }
}

void BasicTreeListBox::SetCurrentEntry( BasicEntryDescriptor& rDesc )
{
    SvLBoxEntry* pCurEntry = 0;
    BasicEntryDescriptor aDesc( rDesc );
    if ( aDesc.GetType() == OBJ_TYPE_UNKNOWN )
    {
        aDesc = BasicEntryDescriptor( 0, LIBRARY_LOCATION_USER, String::CreateFromAscii( "Standard" ),
                                      String::CreateFromAscii( "." ), OBJ_TYPE_UNKNOWN );
    }
    SfxObjectShell* pShell( aDesc.GetShell() );
    LibraryLocation eLocation( aDesc.GetLocation() );
    SvLBoxEntry* pRootEntry = FindRootEntry( pShell, eLocation );
    if ( pRootEntry )
    {
        pCurEntry = pRootEntry;
        String aLibName( aDesc.GetLibName() );
        if ( aLibName.Len() )
        {
            Expand( pRootEntry );
            SvLBoxEntry* pLibEntry = FindEntry( pRootEntry, aLibName, OBJ_TYPE_LIBRARY );
            if ( pLibEntry )
            {
                pCurEntry = pLibEntry;
                String aName( aDesc.GetName() );
                if ( aName.Len() )
                {
                    Expand( pLibEntry );
                    BasicEntryType eType = OBJ_TYPE_MODULE;
                    if ( aDesc.GetType() == OBJ_TYPE_DIALOG )
                        eType = OBJ_TYPE_DIALOG;
                    SvLBoxEntry* pEntry = FindEntry( pLibEntry, aName, eType );
                    if ( pEntry )
                    {
                        pCurEntry = pEntry;
                        String aMethodName( aDesc.GetMethodName() );
                        if ( aMethodName.Len() )
                        {
                            Expand( pEntry );
                            SvLBoxEntry* pSubEntry = FindEntry( pEntry, aMethodName, OBJ_TYPE_METHOD );
                            if ( pSubEntry )
                            {
                                pCurEntry = pSubEntry;
                            }
                            else
                            {
                                pSubEntry = FirstChild( pEntry );
                                if ( pSubEntry )
                                    pCurEntry = pSubEntry;
                            }
                        }
                    }
                    else
                    {
                        pEntry = FirstChild( pLibEntry );
                        if ( pEntry )
                            pCurEntry = pEntry;
                    }
                }
            }
            else
            {
                pLibEntry = FirstChild( pRootEntry );
                if ( pLibEntry )
                    pCurEntry = pLibEntry;
            }
        }
    }
    else
    {
        pRootEntry = First();
        if ( pRootEntry )
            pCurEntry = pRootEntry;
    }

    SetCurEntry( pCurEntry );
}
