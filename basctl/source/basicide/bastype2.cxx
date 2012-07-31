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


#include <memory>

#include "vcl/bitmap.hxx"

#include "basidesh.hxx"
#include "iderdll.hxx"
#include "iderdll2.hxx"
#include <basidesh.hrc>
#include <bastypes.hxx>
#include <bastype2.hxx>
#include <basobj.hxx>
#include <baside2.hrc>
#include <iderid.hxx>
#include <tools/urlobj.hxx>
#include <tools/diagnose_ex.h>
#include <basic/sbx.hxx>
#include <svtools/imagemgr.hxx>
#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/script/XLibraryContainerPassword.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/componentcontext.hxx>
#include <sfx2/dispatch.hxx>

#include <map>
#include <com/sun/star/script/ModuleType.hpp>
#include <com/sun/star/script/vba/XVBAModuleInfo.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XNamed.hpp>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;

void ModuleInfoHelper::getObjectName( const uno::Reference< container::XNameContainer >& rLib, const ::rtl::OUString& rModName, ::rtl::OUString& rObjName )
{
    try
    {
                uno::Reference< script::vba::XVBAModuleInfo > xVBAModuleInfo( rLib, uno::UNO_QUERY );
                if ( xVBAModuleInfo.is() && xVBAModuleInfo->hasModuleInfo( rModName ) )
                {
            script::ModuleInfo aModuleInfo = xVBAModuleInfo->getModuleInfo( rModName );
            uno::Any aObject( aModuleInfo.ModuleObject );
            uno::Reference< lang::XServiceInfo > xServiceInfo( aObject, uno::UNO_QUERY );
            if( xServiceInfo.is() && xServiceInfo->supportsService( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ooo.vba.excel.Worksheet" )) ) )
            {
                uno::Reference< container::XNamed > xNamed( aObject, uno::UNO_QUERY );
                if( xNamed.is() )
                    rObjName = xNamed->getName();
            }
        }
    }
    catch(const uno::Exception& )
    {
    }
}

sal_Int32 ModuleInfoHelper::getModuleType(  const uno::Reference< container::XNameContainer >& rLib, const ::rtl::OUString& rModName )
{
    sal_Int32 nType = script::ModuleType::NORMAL;
    uno::Reference< script::vba::XVBAModuleInfo > xVBAModuleInfo( rLib, uno::UNO_QUERY );
    if ( xVBAModuleInfo.is() && xVBAModuleInfo->hasModuleInfo( rModName ) )
    {
        script::ModuleInfo aModuleInfo = xVBAModuleInfo->getModuleInfo( rModName );
        nType = aModuleInfo.ModuleType;
    }
    return nType;
}

BasicEntry::~BasicEntry()
{
}

BasicDocumentEntry::BasicDocumentEntry( const ScriptDocument& rDocument, LibraryLocation eLocation, BasicEntryType eType )
    :BasicEntry( eType )
    ,m_aDocument( rDocument )
    ,m_eLocation( eLocation )
{
    OSL_ENSURE( m_aDocument.isValid(), "BasicDocumentEntry::BasicDocumentEntry: illegal document!" );
}

BasicDocumentEntry::~BasicDocumentEntry()
{
}

BasicLibEntry::BasicLibEntry( const ScriptDocument& rDocument, LibraryLocation eLocation, const ::rtl::OUString& rLibName, BasicEntryType eType )
    :BasicDocumentEntry( rDocument, eLocation, eType )
    ,m_aLibName( rLibName )
{
}

BasicLibEntry::~BasicLibEntry()
{
}

BasicEntryDescriptor::BasicEntryDescriptor()
    :m_aDocument( ScriptDocument::getApplicationScriptDocument() )
    ,m_eLocation( LIBRARY_LOCATION_UNKNOWN )
    ,m_eType( OBJ_TYPE_UNKNOWN )
{
}

BasicEntryDescriptor::BasicEntryDescriptor( const ScriptDocument& rDocument, LibraryLocation eLocation, const ::rtl::OUString& rLibName, const ::rtl::OUString& rLibSubName, const ::rtl::OUString& rName, BasicEntryType eType )
    :m_aDocument( rDocument )
    ,m_eLocation( eLocation )
    ,m_aLibName( rLibName )
    ,m_aLibSubName( rLibSubName )
    ,m_aName( rName )
    ,m_eType( eType )
{
    OSL_ENSURE( m_aDocument.isValid(), "BasicEntryDescriptor::BasicEntryDescriptor: invalid document!" );
}

BasicEntryDescriptor::BasicEntryDescriptor( const ScriptDocument& rDocument, LibraryLocation eLocation, const ::rtl::OUString& rLibName, const ::rtl::OUString& rLibSubName, const ::rtl::OUString& rName, const ::rtl::OUString& rMethodName, BasicEntryType eType )
    :m_aDocument( rDocument )
    ,m_eLocation( eLocation )
    ,m_aLibName( rLibName )
    ,m_aLibSubName( rLibSubName )
    ,m_aName( rName )
    ,m_aMethodName( rMethodName )
    ,m_eType( eType )
{
    OSL_ENSURE( m_aDocument.isValid(), "BasicEntryDescriptor::BasicEntryDescriptor: invalid document!" );
}

BasicEntryDescriptor::~BasicEntryDescriptor()
{
}

BasicEntryDescriptor::BasicEntryDescriptor( const BasicEntryDescriptor& rDesc )
    :m_aDocument( rDesc.m_aDocument )
    ,m_eLocation( rDesc.m_eLocation )
    ,m_aLibName( rDesc.m_aLibName )
    ,m_aLibSubName( rDesc.m_aLibSubName )
    ,m_aName( rDesc.m_aName )
    ,m_aMethodName( rDesc.m_aMethodName )
    ,m_eType( rDesc.m_eType )
{
}

BasicEntryDescriptor& BasicEntryDescriptor::operator=( const BasicEntryDescriptor& rDesc )
{
    m_aDocument = rDesc.m_aDocument;
    m_eLocation = rDesc.m_eLocation;
    m_aLibName = rDesc.m_aLibName;
    m_aLibSubName = rDesc.m_aLibSubName;
    m_aName = rDesc.m_aName;
    m_aMethodName = rDesc.m_aMethodName;
    m_eType = rDesc.m_eType;

    return *this;
}

bool BasicEntryDescriptor::operator==( const BasicEntryDescriptor& rDesc ) const
{
    return m_aDocument == rDesc.m_aDocument &&
           m_eLocation == rDesc.m_eLocation &&
           m_aLibName == rDesc.m_aLibName &&
           m_aLibSubName == rDesc.m_aLibSubName &&
           m_aName == rDesc.m_aName &&
           m_aMethodName == rDesc.m_aMethodName &&
           m_eType == rDesc.m_eType;
}

BasicTreeListBox::BasicTreeListBox( Window* pParent, const ResId& rRes ) :
    SvTreeListBox( pParent, IDEResId( sal::static_int_cast<sal_uInt16>( rRes.GetId() ) ) ),
    m_aNotifier( *this )
{
    SetNodeDefaultImages();
    SetSelectionMode( SINGLE_SELECTION );
    nMode = 0xFF;   // everything
}



BasicTreeListBox::~BasicTreeListBox()
{
    m_aNotifier.dispose();

    // destroy user data
    SvLBoxEntry* pEntry = First();
    while ( pEntry )
    {
        delete (BasicEntry*)pEntry->GetUserData();
        pEntry = Next( pEntry );
    }
}

void BasicTreeListBox::ScanEntry( const ScriptDocument& rDocument, LibraryLocation eLocation )
{
    OSL_ENSURE( rDocument.isAlive(), "BasicTreeListBox::ScanEntry: illegal document!" );
    if ( !rDocument.isAlive() )
        return;

    // can be called multiple times for updating!

    // actually test if basic's in the tree already?!
    SetUpdateMode( sal_False );

    // level 1: BasicManager (application, document, ...)
    SvLBoxEntry* pDocumentRootEntry = FindRootEntry( rDocument, eLocation );
    if ( pDocumentRootEntry && IsExpanded( pDocumentRootEntry ) )
        ImpCreateLibEntries( pDocumentRootEntry, rDocument, eLocation );
    if ( !pDocumentRootEntry )
    {
        ::rtl::OUString aRootName( GetRootEntryName( rDocument, eLocation ) );
        Image aImage;
        GetRootEntryBitmaps( rDocument, aImage );
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        pDocumentRootEntry = AddEntry(
            aRootName,
            aImage,
            0, true,
            std::auto_ptr< BasicEntry >( new BasicDocumentEntry( rDocument, eLocation ) ) );
        SAL_WNODEPRECATED_DECLARATIONS_POP
    }

    SetUpdateMode( sal_True );
}

void BasicTreeListBox::ImpCreateLibEntries( SvLBoxEntry* pDocumentRootEntry, const ScriptDocument& rDocument, LibraryLocation eLocation )
{
    // get a sorted list of library names
    Sequence< ::rtl::OUString > aLibNames( rDocument.getLibraryNames() );
    sal_Int32 nLibCount = aLibNames.getLength();
    const ::rtl::OUString* pLibNames = aLibNames.getConstArray();

    for ( sal_Int32 i = 0 ; i < nLibCount ; i++ )
    {
        ::rtl::OUString aLibName = pLibNames[ i ];

        if ( eLocation == rDocument.getLibraryLocation( aLibName ) )
        {
            // check, if the module library is loaded
            bool bModLibLoaded = false;
            ::rtl::OUString aOULibName( aLibName );
            Reference< script::XLibraryContainer > xModLibContainer( rDocument.getLibraryContainer( E_SCRIPTS ) );
            if ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) && xModLibContainer->isLibraryLoaded( aOULibName ) )
                bModLibLoaded = true;

            // check, if the dialog library is loaded
            bool bDlgLibLoaded = false;
            Reference< script::XLibraryContainer > xDlgLibContainer( rDocument.getLibraryContainer( E_DIALOGS ) );
            if ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aOULibName ) && xDlgLibContainer->isLibraryLoaded( aOULibName ) )
                bDlgLibLoaded = true;

            bool bLoaded = bModLibLoaded || bDlgLibLoaded;

            // if only one of the libraries is loaded, load also the other
            if ( bLoaded )
            {
                if ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) && !xModLibContainer->isLibraryLoaded( aOULibName ) )
                    xModLibContainer->loadLibrary( aOULibName );

                if ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aOULibName ) && !xDlgLibContainer->isLibraryLoaded( aOULibName ) )
                    xDlgLibContainer->loadLibrary( aOULibName );
            }

            // create tree list box entry
            sal_uInt16 nId;
            if ( ( nMode & BROWSEMODE_DIALOGS ) && !( nMode & BROWSEMODE_MODULES ) )
                nId = bLoaded ? RID_IMG_DLGLIB : RID_IMG_DLGLIBNOTLOADED;
            else
                nId = bLoaded ? RID_IMG_MODLIB : RID_IMG_MODLIBNOTLOADED;
            SvLBoxEntry* pLibRootEntry = FindEntry( pDocumentRootEntry, aLibName, OBJ_TYPE_LIBRARY );
            if ( pLibRootEntry )
            {
                SetEntryBitmaps( pLibRootEntry, Image( IDEResId( nId ) ) );
                if ( IsExpanded( pLibRootEntry ) )
                    ImpCreateLibSubEntries( pLibRootEntry, rDocument, aLibName );
            }
            else
            {
                pLibRootEntry = AddEntry(
                    aLibName,
                    Image( IDEResId( nId ) ),
                    pDocumentRootEntry, true,
                    std::auto_ptr< BasicEntry >( new BasicEntry( OBJ_TYPE_LIBRARY ) ) );
            }
        }
    }
}

void BasicTreeListBox::ImpCreateLibSubEntries( SvLBoxEntry* pLibRootEntry, const ScriptDocument& rDocument, const ::rtl::OUString& rLibName )
{
    // modules
    if ( nMode & BROWSEMODE_MODULES )
    {
        Reference< script::XLibraryContainer > xModLibContainer( rDocument.getLibraryContainer( E_SCRIPTS ) );

        if ( xModLibContainer.is() && xModLibContainer->hasByName( rLibName ) && xModLibContainer->isLibraryLoaded( rLibName ) )
        {
            try
            {
                if( rDocument.isInVBAMode() )
                    ImpCreateLibSubEntriesInVBAMode( pLibRootEntry, rDocument, rLibName );
                else
                {
                    // get a sorted list of module names
                    Sequence< ::rtl::OUString > aModNames = rDocument.getObjectNames( E_SCRIPTS, rLibName );
                    sal_Int32 nModCount = aModNames.getLength();
                    const ::rtl::OUString* pModNames = aModNames.getConstArray();

                    for ( sal_Int32 i = 0 ; i < nModCount ; i++ )
                    {
                        ::rtl::OUString aModName = pModNames[ i ];
                        SvLBoxEntry* pModuleEntry = FindEntry( pLibRootEntry, aModName, OBJ_TYPE_MODULE );
                        if ( !pModuleEntry )
                            pModuleEntry = AddEntry(
                                aModName,
                                Image( IDEResId( RID_IMG_MODULE ) ),
                                pLibRootEntry, false,
                                std::auto_ptr< BasicEntry >( new BasicEntry( OBJ_TYPE_MODULE ) ) );

                        // methods
                        if ( nMode & BROWSEMODE_SUBS )
                        {
                            Sequence< ::rtl::OUString > aNames = BasicIDE::GetMethodNames( rDocument, rLibName, aModName );
                            sal_Int32 nCount = aNames.getLength();
                            const ::rtl::OUString* pNames = aNames.getConstArray();

                            for ( sal_Int32 j = 0 ; j < nCount ; j++ )
                            {
                                ::rtl::OUString aName = pNames[ j ];
                                SvLBoxEntry* pEntry = FindEntry( pModuleEntry, aName, OBJ_TYPE_METHOD );
                                if ( !pEntry )
                                    pEntry = AddEntry(
                                        aName,
                                        Image( IDEResId( RID_IMG_MACRO ) ),
                                        pModuleEntry, false,
                                        std::auto_ptr< BasicEntry >( new BasicEntry( OBJ_TYPE_METHOD ) ) );
                            }
                        }
                    }
                }
            }
            catch ( const container::NoSuchElementException& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
    }

    // dialogs
    if ( nMode & BROWSEMODE_DIALOGS )
    {
         Reference< script::XLibraryContainer > xDlgLibContainer( rDocument.getLibraryContainer( E_DIALOGS ) );

         if ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( rLibName ) && xDlgLibContainer->isLibraryLoaded( rLibName ) )
         {
            try
            {
                // get a sorted list of dialog names
                Sequence< ::rtl::OUString > aDlgNames( rDocument.getObjectNames( E_DIALOGS, rLibName ) );
                sal_Int32 nDlgCount = aDlgNames.getLength();
                const ::rtl::OUString* pDlgNames = aDlgNames.getConstArray();

                for ( sal_Int32 i = 0 ; i < nDlgCount ; i++ )
                {
                    ::rtl::OUString aDlgName = pDlgNames[ i ];
                    SvLBoxEntry* pDialogEntry = FindEntry( pLibRootEntry, aDlgName, OBJ_TYPE_DIALOG );
                    if ( !pDialogEntry )
                        pDialogEntry = AddEntry(
                            aDlgName,
                            Image( IDEResId( RID_IMG_DIALOG ) ),
                            pLibRootEntry, false,
                            std::auto_ptr< BasicEntry >( new BasicEntry( OBJ_TYPE_DIALOG ) ) );
                }
            }
            catch (const container::NoSuchElementException& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
    }
}

void BasicTreeListBox::ImpCreateLibSubEntriesInVBAMode( SvLBoxEntry* pLibRootEntry, const ScriptDocument& rDocument, const ::rtl::OUString& rLibName )
{

    ::std::vector< std::pair< BasicEntryType, ::rtl::OUString > > aEntries;
    aEntries.push_back( ::std::make_pair( OBJ_TYPE_DOCUMENT_OBJECTS, IDE_RESSTR(RID_STR_DOCUMENT_OBJECTS) ) );
    aEntries.push_back( ::std::make_pair( OBJ_TYPE_USERFORMS, IDE_RESSTR(RID_STR_USERFORMS) ) );
    aEntries.push_back( ::std::make_pair( OBJ_TYPE_NORMAL_MODULES, IDE_RESSTR(RID_STR_NORMAL_MODULES) ) );
    aEntries.push_back( ::std::make_pair( OBJ_TYPE_CLASS_MODULES, IDE_RESSTR(RID_STR_CLASS_MODULES) ) );

    ::std::vector< std::pair< BasicEntryType, ::rtl::OUString > >::iterator iter;
    for( iter = aEntries.begin(); iter != aEntries.end(); ++iter )
    {
        BasicEntryType eType = iter->first;
        ::rtl::OUString aEntryName = iter->second;
        SvLBoxEntry* pLibSubRootEntry = FindEntry( pLibRootEntry, aEntryName, eType );
        if( pLibSubRootEntry )
        {
            SetEntryBitmaps( pLibSubRootEntry, Image( IDEResId( RID_IMG_MODLIB ) ) );
            if ( IsExpanded( pLibSubRootEntry ) )
                ImpCreateLibSubSubEntriesInVBAMode( pLibSubRootEntry, rDocument, rLibName );
        }
        else
        {
            pLibSubRootEntry = AddEntry(
                aEntryName,
                Image( IDEResId( RID_IMG_MODLIB ) ),
                pLibRootEntry, true,
                std::auto_ptr< BasicEntry >( new BasicEntry( eType ) ) );
        }
    }
}

void BasicTreeListBox::ImpCreateLibSubSubEntriesInVBAMode( SvLBoxEntry* pLibSubRootEntry, const ScriptDocument& rDocument, const ::rtl::OUString& rLibName )
{
    uno::Reference< container::XNameContainer > xLib = rDocument.getOrCreateLibrary( E_SCRIPTS, rLibName );
    if( !xLib.is() )
        return;

    try
    {
        // get a sorted list of module names
        Sequence< ::rtl::OUString > aModNames = rDocument.getObjectNames( E_SCRIPTS, rLibName );
        sal_Int32 nModCount = aModNames.getLength();
        const ::rtl::OUString* pModNames = aModNames.getConstArray();

        BasicEntryDescriptor aDesc( GetEntryDescriptor( pLibSubRootEntry ) );
        BasicEntryType eCurrentType( aDesc.GetType() );

        for ( sal_Int32 i = 0 ; i < nModCount ; i++ )
        {
            ::rtl::OUString aModName = pModNames[ i ];
            BasicEntryType eType = OBJ_TYPE_UNKNOWN;
            switch( ModuleInfoHelper::getModuleType( xLib, aModName ) )
            {
                case script::ModuleType::DOCUMENT:
                    eType = OBJ_TYPE_DOCUMENT_OBJECTS;
                    break;
                case script::ModuleType::FORM:
                    eType = OBJ_TYPE_USERFORMS;
                    break;
                case script::ModuleType::NORMAL:
                    eType = OBJ_TYPE_NORMAL_MODULES;
                    break;
                case script::ModuleType::CLASS:
                    eType = OBJ_TYPE_CLASS_MODULES;
                    break;
            }
            if( eType != eCurrentType )
                continue;

            // display a nice friendly name in the ObjectModule tab,
               // combining the objectname and module name, e.g. Sheet1 ( Financials )
            ::rtl::OUStringBuffer aEntryNameBuf( aModName );
            if( eType == OBJ_TYPE_DOCUMENT_OBJECTS )
            {
                ::rtl::OUString sObjName;
                ModuleInfoHelper::getObjectName( xLib, aModName, sObjName );
                if( !sObjName.isEmpty() )
                {
                    aEntryNameBuf.appendAscii(RTL_CONSTASCII_STRINGPARAM(" ("));
                    aEntryNameBuf.append(sObjName);
                    aEntryNameBuf.append(')');
                }
            }
            ::rtl::OUString aEntryName(aEntryNameBuf.makeStringAndClear());
            SvLBoxEntry* pModuleEntry = FindEntry( pLibSubRootEntry, aEntryName, OBJ_TYPE_MODULE );
            if ( !pModuleEntry )
                pModuleEntry = AddEntry(
                    aEntryName,
                    Image( IDEResId( RID_IMG_MODULE ) ),
                    pLibSubRootEntry, false,
                    std::auto_ptr< BasicEntry >( new BasicEntry( OBJ_TYPE_MODULE ) ) );

            // methods
            if ( nMode & BROWSEMODE_SUBS )
            {
                Sequence< ::rtl::OUString > aNames = BasicIDE::GetMethodNames( rDocument, rLibName, aModName );
                sal_Int32 nCount = aNames.getLength();
                const ::rtl::OUString* pNames = aNames.getConstArray();

                for ( sal_Int32 j = 0 ; j < nCount ; j++ )
                {
                    ::rtl::OUString aName = pNames[ j ];
                    SvLBoxEntry* pEntry = FindEntry( pModuleEntry, aName, OBJ_TYPE_METHOD );
                    if ( !pEntry )
                        pEntry = AddEntry(
                            aName,
                            Image( IDEResId( RID_IMG_MACRO ) ),
                            pModuleEntry, false,
                            std::auto_ptr< BasicEntry >( new BasicEntry( OBJ_TYPE_METHOD ) ) );
                }
            }
        }
    }
    catch ( const container::NoSuchElementException& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

SvLBoxEntry* BasicTreeListBox::ImpFindEntry( SvLBoxEntry* pParent, const ::rtl::OUString& rText )
{
    sal_uLong nRootPos = 0;
    SvLBoxEntry* pEntry = pParent ? FirstChild( pParent ) : GetEntry( nRootPos );
    while ( pEntry )
    {
        if (  rText.equals(GetEntryText( pEntry )) )
            return pEntry;

        pEntry = pParent ? NextSibling( pEntry ) : GetEntry( ++nRootPos );
    }
    return 0;
}

void BasicTreeListBox::onDocumentCreated( const ScriptDocument& /*_rDocument*/ )
{
    UpdateEntries();
}

void BasicTreeListBox::onDocumentOpened( const ScriptDocument& /*_rDocument*/ )
{
    UpdateEntries();
}

void BasicTreeListBox::onDocumentSave( const ScriptDocument& /*_rDocument*/ )
{
    // not interested in
}

void BasicTreeListBox::onDocumentSaveDone( const ScriptDocument& /*_rDocument*/ )
{
    // not interested in
}

void BasicTreeListBox::onDocumentSaveAs( const ScriptDocument& /*_rDocument*/ )
{
    // not interested in
}

void BasicTreeListBox::onDocumentSaveAsDone( const ScriptDocument& /*_rDocument*/ )
{
    UpdateEntries();
}

void BasicTreeListBox::onDocumentClosed( const ScriptDocument& rDocument )
{
    UpdateEntries();
    // The document is not yet actually deleted, so we need to remove its entry
    // manually.
    RemoveEntry(rDocument);
}

void BasicTreeListBox::onDocumentTitleChanged( const ScriptDocument& /*_rDocument*/ )
{
    // not interested in
}

void BasicTreeListBox::onDocumentModeChanged( const ScriptDocument& /*_rDocument*/ )
{
    // not interested in
}

void BasicTreeListBox::UpdateEntries()
{
    BasicEntryDescriptor aCurDesc( GetEntryDescriptor( FirstSelected() ) );

    // removing the invalid entries
    SvLBoxEntry* pLastValid = 0;
    SvLBoxEntry* pEntry = First();
    while ( pEntry )
    {
        if ( IsValidEntry( pEntry ) )
            pLastValid = pEntry;
        else
            RemoveEntry(pEntry);
        pEntry = pLastValid ? Next( pLastValid ) : First();
    }

    ScanAllEntries();

    SetCurrentEntry( aCurDesc );
}

// Removes the entry from the tree.
void BasicTreeListBox::RemoveEntry (SvLBoxEntry* pEntry)
{
    // removing the associated user data
    delete (BasicEntry*)pEntry->GetUserData();
    // removing the entry
    GetModel()->Remove( pEntry );
}

// Removes the entry of rDocument.
void BasicTreeListBox::RemoveEntry (ScriptDocument const& rDocument)
{
    // finding the entry of rDocument
    for (SvLBoxEntry* pEntry = First(); pEntry; pEntry = Next(pEntry))
        if (rDocument == GetEntryDescriptor(pEntry).GetDocument())
        {
            RemoveEntry(pEntry);
            break;
        }
}

SvLBoxEntry* BasicTreeListBox::CloneEntry( SvLBoxEntry* pSource )
{
    SvLBoxEntry* pNew = SvTreeListBox::CloneEntry( pSource );
    BasicEntry* pUser = (BasicEntry*)pSource->GetUserData();

    DBG_ASSERT( pUser, "User-Daten?!" );
    DBG_ASSERT( pUser->GetType() != OBJ_TYPE_DOCUMENT, "BasicTreeListBox::CloneEntry: document?!" );

    BasicEntry* pNewUser = new BasicEntry( *pUser );
    pNew->SetUserData( pNewUser );
    return pNew;
}

SvLBoxEntry* BasicTreeListBox::FindEntry( SvLBoxEntry* pParent, const ::rtl::OUString& rText, BasicEntryType eType )
{
    sal_uLong nRootPos = 0;
    SvLBoxEntry* pEntry = pParent ? FirstChild( pParent ) : GetEntry( nRootPos );
    while ( pEntry )
    {
        BasicEntry* pBasicEntry = (BasicEntry*)pEntry->GetUserData();
        DBG_ASSERT( pBasicEntry, "FindEntry: Kein BasicEntry ?!" );
        if ( ( pBasicEntry->GetType() == eType  ) && ( rText.equals(GetEntryText( pEntry )) ) )
            return pEntry;

        pEntry = pParent ? NextSibling( pEntry ) : GetEntry( ++nRootPos );
    }
    return 0;
}

long BasicTreeListBox::ExpandingHdl()
{
    // expanding or collapsing?
    sal_Bool bOK = sal_True;
    if ( GetModel()->GetDepth( GetHdlEntry() ) == 1 )
    {
        SvLBoxEntry* pCurEntry = GetCurEntry();
        BasicEntryDescriptor aDesc( GetEntryDescriptor( pCurEntry ) );
        ScriptDocument aDocument( aDesc.GetDocument() );
        OSL_ENSURE( aDocument.isAlive(), "BasicTreeListBox::ExpandingHdl: no document, or document is dead!" );
        if ( aDocument.isAlive() )
        {
            ::rtl::OUString aLibName( aDesc.GetLibName() );
            ::rtl::OUString aLibSubName( aDesc.GetLibSubName() );
            ::rtl::OUString aName( aDesc.GetName() );
            ::rtl::OUString aMethodName( aDesc.GetMethodName() );

            if ( !aLibName.isEmpty() && aLibSubName.isEmpty() && aName.isEmpty() && aMethodName.isEmpty() )
            {
                // check password, if library is password protected and not verified
                Reference< script::XLibraryContainer > xModLibContainer( aDocument.getLibraryContainer( E_SCRIPTS ) );
                if ( xModLibContainer.is() && xModLibContainer->hasByName( aLibName ) )
                {
                    Reference< script::XLibraryContainerPassword > xPasswd( xModLibContainer, UNO_QUERY );
                    if ( xPasswd.is() && xPasswd->isLibraryPasswordProtected( aLibName ) && !xPasswd->isLibraryPasswordVerified( aLibName ) )
                    {
                        ::rtl::OUString aPassword;
                        bOK = QueryPassword( xModLibContainer, aLibName, aPassword );
                    }
                }
            }
        }
    }
    return bOK;
}

bool BasicTreeListBox::IsEntryProtected( SvLBoxEntry* pEntry )
{
    bool bProtected = false;
    if ( pEntry && ( GetModel()->GetDepth( pEntry ) == 1 ) )
    {
        BasicEntryDescriptor aDesc( GetEntryDescriptor( pEntry ) );
        ScriptDocument aDocument( aDesc.GetDocument() );
        OSL_ENSURE( aDocument.isAlive(), "BasicTreeListBox::IsEntryProtected: no document, or document is dead!" );
        if ( aDocument.isAlive() )
        {
            ::rtl::OUString aOULibName( aDesc.GetLibName() );
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
    }
    return bProtected;
}

SAL_WNODEPRECATED_DECLARATIONS_PUSH
SvLBoxEntry* BasicTreeListBox::AddEntry(
                                        const ::rtl::OUString& rText,
    const Image& rImage,
    SvLBoxEntry* pParent,
    bool bChildrenOnDemand,
    std::auto_ptr< BasicEntry > aUserData
)
{
    SvLBoxEntry* p = InsertEntry(
        rText, rImage, rImage, pParent, bChildrenOnDemand, LIST_APPEND,
        aUserData.release() ); // XXX possible leak
    return p;
}
SAL_WNODEPRECATED_DECLARATIONS_POP

void BasicTreeListBox::SetEntryBitmaps( SvLBoxEntry * pEntry, const Image& rImage )
{
    SetExpandedEntryBmp(  pEntry, rImage );
    SetCollapsedEntryBmp( pEntry, rImage );
}

LibraryType BasicTreeListBox::GetLibraryType() const
{
    LibraryType eType = LIBRARY_TYPE_ALL;
    if ( ( nMode & BROWSEMODE_MODULES ) && !( nMode & BROWSEMODE_DIALOGS ) )
        eType = LIBRARY_TYPE_MODULE;
    else if ( !( nMode & BROWSEMODE_MODULES ) && ( nMode & BROWSEMODE_DIALOGS ) )
        eType = LIBRARY_TYPE_DIALOG;
    return eType;
}

::rtl::OUString BasicTreeListBox::GetRootEntryName( const ScriptDocument& rDocument, LibraryLocation eLocation ) const
{
    return rDocument.getTitle( eLocation, GetLibraryType() );
}

void BasicTreeListBox::GetRootEntryBitmaps( const ScriptDocument& rDocument, Image& rImage )
{
    OSL_ENSURE( rDocument.isValid(), "BasicTreeListBox::GetRootEntryBitmaps: illegal document!" );
    if ( !rDocument.isValid() )
        return;

    if ( rDocument.isDocument() )
    {
        ::rtl::OUString sFactoryURL;
        ::comphelper::ComponentContext aContext( ::comphelper::getProcessServiceFactory() );
        Reference< ::com::sun::star::frame::XModuleManager > xModuleManager;
        if ( aContext.createComponent( "com.sun.star.frame.ModuleManager", xModuleManager ) )
        {
            try
            {
                ::rtl::OUString sModule( xModuleManager->identify( rDocument.getDocument() ) );
                Reference< container::XNameAccess > xModuleConfig( xModuleManager, UNO_QUERY );
                if ( xModuleConfig.is() )
                {
                    Sequence< beans::PropertyValue > aModuleDescr;
                    xModuleConfig->getByName( sModule ) >>= aModuleDescr;
                    sal_Int32 nCount = aModuleDescr.getLength();
                    const beans::PropertyValue* pModuleDescr = aModuleDescr.getConstArray();
                    for ( sal_Int32 i = 0; i < nCount; ++i )
                    {
                        if ( pModuleDescr[ i ].Name == "ooSetupFactoryEmptyDocumentURL" )
                        {
                            pModuleDescr[ i ].Value >>= sFactoryURL;
                            break;
                        }
                    }
                }
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }

        if ( !sFactoryURL.isEmpty() )
        {
            rImage = SvFileInformationManager::GetFileImage( INetURLObject( sFactoryURL ), sal_False );
        }
        else
        {
            // default icon
            rImage = Image( IDEResId( RID_IMG_DOCUMENT ) );
        }
    }
    else
    {
        rImage = Image( IDEResId( RID_IMG_INSTALLATION ) );
    }
}

void BasicTreeListBox::SetCurrentEntry( BasicEntryDescriptor& rDesc )
{
    SvLBoxEntry* pCurEntry = 0;
    BasicEntryDescriptor aDesc( rDesc );
    if ( aDesc.GetType() == OBJ_TYPE_UNKNOWN )
    {
        aDesc = BasicEntryDescriptor(
            ScriptDocument::getApplicationScriptDocument(),
            LIBRARY_LOCATION_USER, "Standard",
            ::rtl::OUString(), ".", OBJ_TYPE_UNKNOWN );
    }
    ScriptDocument aDocument( aDesc.GetDocument() );
    OSL_ENSURE( aDocument.isValid(), "BasicTreeListBox::SetCurrentEntry: invalid document!" );
    LibraryLocation eLocation( aDesc.GetLocation() );
    SvLBoxEntry* pRootEntry = FindRootEntry( aDocument, eLocation );
    if ( pRootEntry )
    {
        pCurEntry = pRootEntry;
        ::rtl::OUString aLibName( aDesc.GetLibName() );
        if ( !aLibName.isEmpty() )
        {
            Expand( pRootEntry );
            SvLBoxEntry* pLibEntry = FindEntry( pRootEntry, aLibName, OBJ_TYPE_LIBRARY );
            if ( pLibEntry )
            {
                pCurEntry = pLibEntry;
                ::rtl::OUString aLibSubName( aDesc.GetLibSubName() );
                if( !aLibSubName.isEmpty() )
                {
                    Expand( pLibEntry );
                    SvLBoxEntry* pLibSubEntry = ImpFindEntry( pLibEntry, aLibSubName );
                    if( pLibSubEntry )
                    {
                        pCurEntry = pLibSubEntry;
                    }
                }
                ::rtl::OUString aName( aDesc.GetName() );
                if ( !aName.isEmpty() )
                {
                    Expand( pCurEntry );
                    BasicEntryType eType = OBJ_TYPE_MODULE;
                    if ( aDesc.GetType() == OBJ_TYPE_DIALOG )
                        eType = OBJ_TYPE_DIALOG;
                    SvLBoxEntry* pEntry = FindEntry( pCurEntry, aName, eType );
                    if ( pEntry )
                    {
                        pCurEntry = pEntry;
                        ::rtl::OUString aMethodName( aDesc.GetMethodName() );
                        if ( !aMethodName.isEmpty() )
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

void BasicTreeListBox::MouseButtonDown( const MouseEvent& rMEvt )
{
    SvTreeListBox::MouseButtonDown( rMEvt );
    if ( rMEvt.IsLeft() && ( rMEvt.GetClicks() == 2 ) )
    {
        BasicEntryDescriptor aDesc( GetEntryDescriptor( GetCurEntry() ) );

        if ( aDesc.GetType() == OBJ_TYPE_METHOD )
        {
            BasicIDEShell* pIDEShell = BasicIDEGlobals::GetShell();
            SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
            SfxDispatcher* pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;
            if( pDispatcher )
            {
                SbxItem aSbxItem( SID_BASICIDE_ARG_SBX, aDesc.GetDocument(), aDesc.GetLibName(), aDesc.GetName(),
                aDesc.GetMethodName(), ConvertType( aDesc.GetType() ) );
                pDispatcher->Execute( SID_BASICIDE_SHOWSBX,
                SFX_CALLMODE_SYNCHRON, &aSbxItem, 0L );
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
