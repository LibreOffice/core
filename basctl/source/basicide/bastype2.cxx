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


#include <bastypes.hxx>
#include <bastype2.hxx>
#include <basidesh.hrc>
#include <iderid.hxx>
#include <o3tl/make_unique.hxx>
#include <tools/urlobj.hxx>
#include <tools/diagnose_ex.h>
#include <svtools/imagemgr.hxx>
#include <svtools/treelistentry.hxx>
#include <com/sun/star/script/XLibraryContainerPassword.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <comphelper/processfactory.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/builderfactory.hxx>

#include <initializer_list>
#include <memory>

#include <com/sun/star/script/ModuleType.hpp>
#include <com/sun/star/script/vba/XVBAModuleInfo.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XNamed.hpp>

namespace basctl
{

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;

void ModuleInfoHelper::getObjectName( const uno::Reference< container::XNameContainer >& rLib, const OUString& rModName, OUString& rObjName )
{
    try
    {
        uno::Reference< script::vba::XVBAModuleInfo > xVBAModuleInfo( rLib, uno::UNO_QUERY );
        if ( xVBAModuleInfo.is() && xVBAModuleInfo->hasModuleInfo( rModName ) )
        {
            script::ModuleInfo aModuleInfo = xVBAModuleInfo->getModuleInfo( rModName );
            uno::Any aObject( aModuleInfo.ModuleObject );
            uno::Reference< lang::XServiceInfo > xServiceInfo( aObject, uno::UNO_QUERY );
            if( xServiceInfo.is() && xServiceInfo->supportsService( "ooo.vba.excel.Worksheet" ) )
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

sal_Int32 ModuleInfoHelper::getModuleType(  const uno::Reference< container::XNameContainer >& rLib, const OUString& rModName )
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

Entry::~Entry()
{ }

DocumentEntry::DocumentEntry (
    ScriptDocument const& rDocument,
    LibraryLocation eLocation,
    EntryType eType
) :
    Entry(eType),
    m_aDocument(rDocument),
    m_eLocation(eLocation)
{
    OSL_ENSURE( m_aDocument.isValid(), "DocumentEntry::DocumentEntry: illegal document!" );
}

DocumentEntry::~DocumentEntry()
{ }

LibEntry::LibEntry (
    ScriptDocument const& rDocument,
    LibraryLocation eLocation,
    OUString const& rLibName,
    EntryType eType
) :
    DocumentEntry(rDocument, eLocation, eType),
    m_aLibName(rLibName)
{ }

LibEntry::~LibEntry()
{ }

EntryDescriptor::EntryDescriptor () :
    m_aDocument(ScriptDocument::getApplicationScriptDocument()),
    m_eLocation(LIBRARY_LOCATION_UNKNOWN),
    m_eType(OBJ_TYPE_UNKNOWN)
{ }

EntryDescriptor::EntryDescriptor (
    ScriptDocument const& rDocument,
    LibraryLocation eLocation,
    OUString const& rLibName,
    OUString const& rLibSubName,
    OUString const& rName,
    EntryType eType
) :
    m_aDocument(rDocument),
    m_eLocation(eLocation),
    m_aLibName(rLibName),
    m_aLibSubName(rLibSubName),
    m_aName(rName),
    m_eType(eType)
{
    OSL_ENSURE( m_aDocument.isValid(), "EntryDescriptor::EntryDescriptor: invalid document!" );
}

EntryDescriptor::EntryDescriptor (
    ScriptDocument const& rDocument,
    LibraryLocation eLocation,
    OUString const& rLibName,
    OUString const& rLibSubName,
    OUString const& rName,
    OUString const& rMethodName,
    EntryType eType
) :
    m_aDocument(rDocument),
    m_eLocation(eLocation),
    m_aLibName(rLibName),
    m_aLibSubName(rLibSubName),
    m_aName(rName),
    m_aMethodName(rMethodName),
    m_eType(eType)
{
    OSL_ENSURE( m_aDocument.isValid(), "EntryDescriptor::EntryDescriptor: invalid document!" );
}

TreeListBox::TreeListBox (vcl::Window* pParent, WinBits nStyle)
    : SvTreeListBox(pParent, nStyle)
    , m_aNotifier( *this )
{
    SetNodeDefaultImages();
    SetSelectionMode( SelectionMode::Single );
    nMode = 0xFF;   // everything
}

VCL_BUILDER_FACTORY_CONSTRUCTOR(TreeListBox, WB_TABSTOP)

TreeListBox::~TreeListBox ()
{
    disposeOnce();
}

void TreeListBox::dispose()
{
    m_aNotifier.dispose();

    // destroy user data
    SvTreeListEntry* pEntry = First();
    while ( pEntry )
    {
        delete static_cast<Entry*>( pEntry->GetUserData() );
        pEntry->SetUserData( nullptr );
        pEntry = Next( pEntry );
    }
    SvTreeListBox::dispose();
}

void TreeListBox::ScanEntry( const ScriptDocument& rDocument, LibraryLocation eLocation )
{
    OSL_ENSURE( rDocument.isAlive(), "TreeListBox::ScanEntry: illegal document!" );
    if ( !rDocument.isAlive() )
        return;

    // can be called multiple times for updating!

    // actually test if basic's in the tree already?!
    SetUpdateMode(false);

    // level 1: BasicManager (application, document, ...)
    SvTreeListEntry* pDocumentRootEntry = FindRootEntry( rDocument, eLocation );
    if ( pDocumentRootEntry && IsExpanded( pDocumentRootEntry ) )
        ImpCreateLibEntries( pDocumentRootEntry, rDocument, eLocation );
    if ( !pDocumentRootEntry )
    {
        OUString aRootName( GetRootEntryName( rDocument, eLocation ) );
        Image aImage;
        GetRootEntryBitmaps( rDocument, aImage );
        AddEntry(
            aRootName,
            aImage,
            nullptr, true, o3tl::make_unique<DocumentEntry>(rDocument, eLocation));
    }

    SetUpdateMode(true);
}

void TreeListBox::ImpCreateLibEntries( SvTreeListEntry* pDocumentRootEntry, const ScriptDocument& rDocument, LibraryLocation eLocation )
{
    // get a sorted list of library names
    Sequence< OUString > aLibNames( rDocument.getLibraryNames() );
    sal_Int32 nLibCount = aLibNames.getLength();
    const OUString* pLibNames = aLibNames.getConstArray();

    for ( sal_Int32 i = 0 ; i < nLibCount ; i++ )
    {
        OUString aLibName = pLibNames[ i ];

        if ( eLocation == rDocument.getLibraryLocation( aLibName ) )
        {
            // check, if the module library is loaded
            bool bModLibLoaded = false;
            Reference< script::XLibraryContainer > xModLibContainer( rDocument.getLibraryContainer( E_SCRIPTS ) );
            if ( xModLibContainer.is() && xModLibContainer->hasByName( aLibName ) && xModLibContainer->isLibraryLoaded( aLibName ) )
                bModLibLoaded = true;

            // check, if the dialog library is loaded
            bool bDlgLibLoaded = false;
            Reference< script::XLibraryContainer > xDlgLibContainer( rDocument.getLibraryContainer( E_DIALOGS ) );
            if ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aLibName ) && xDlgLibContainer->isLibraryLoaded( aLibName ) )
                bDlgLibLoaded = true;

            bool bLoaded = bModLibLoaded || bDlgLibLoaded;

            // if only one of the libraries is loaded, load also the other
            if ( bLoaded )
            {
                if ( xModLibContainer.is() && xModLibContainer->hasByName( aLibName ) && !xModLibContainer->isLibraryLoaded( aLibName ) )
                    xModLibContainer->loadLibrary( aLibName );

                if ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aLibName ) && !xDlgLibContainer->isLibraryLoaded( aLibName ) )
                    xDlgLibContainer->loadLibrary( aLibName );
            }

            // create tree list box entry
            sal_uInt16 nId;
            if ( ( nMode & BROWSEMODE_DIALOGS ) && !( nMode & BROWSEMODE_MODULES ) )
                nId = bLoaded ? RID_BMP_DLGLIB : RID_BMP_DLGLIBNOTLOADED;
            else
                nId = bLoaded ? RID_BMP_MODLIB : RID_BMP_MODLIBNOTLOADED;
            SvTreeListEntry* pLibRootEntry = FindEntry( pDocumentRootEntry, aLibName, OBJ_TYPE_LIBRARY );
            if ( pLibRootEntry )
            {
                SetEntryBitmaps(pLibRootEntry, Image(BitmapEx(IDEResId(nId))));
                if ( IsExpanded(pLibRootEntry))
                    ImpCreateLibSubEntries( pLibRootEntry, rDocument, aLibName );
            }
            else
            {
                AddEntry(
                    aLibName,
                    Image(BitmapEx(IDEResId(nId))),
                    pDocumentRootEntry, true,
                    o3tl::make_unique<Entry>(OBJ_TYPE_LIBRARY));
            }
        }
    }
}

void TreeListBox::ImpCreateLibSubEntries( SvTreeListEntry* pLibRootEntry, const ScriptDocument& rDocument, const OUString& rLibName )
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
                    Sequence< OUString > aModNames = rDocument.getObjectNames( E_SCRIPTS, rLibName );
                    sal_Int32 nModCount = aModNames.getLength();
                    const OUString* pModNames = aModNames.getConstArray();

                    for ( sal_Int32 i = 0 ; i < nModCount ; i++ )
                    {
                        OUString aModName = pModNames[ i ];
                        SvTreeListEntry* pModuleEntry = FindEntry( pLibRootEntry, aModName, OBJ_TYPE_MODULE );
                        if ( !pModuleEntry )
                        {
                            pModuleEntry = AddEntry(
                                aModName,
                                Image(BitmapEx(IDEResId(RID_BMP_MODULE))),
                                pLibRootEntry, false,
                                o3tl::make_unique<Entry>(OBJ_TYPE_MODULE));
                        }

                        // methods
                        if ( nMode & BROWSEMODE_SUBS )
                        {
                            Sequence< OUString > aNames = GetMethodNames( rDocument, rLibName, aModName );
                            sal_Int32 nCount = aNames.getLength();
                            const OUString* pNames = aNames.getConstArray();

                            for ( sal_Int32 j = 0 ; j < nCount ; j++ )
                            {
                                OUString aName = pNames[ j ];
                                SvTreeListEntry* pEntry = FindEntry( pModuleEntry, aName, OBJ_TYPE_METHOD );
                                if ( !pEntry )
                                {
                                    AddEntry(
                                        aName,
                                        Image(BitmapEx(IDEResId(RID_BMP_MACRO))),
                                        pModuleEntry, false,
                                        o3tl::make_unique<Entry>(
                                            OBJ_TYPE_METHOD));
                                }
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
                Sequence< OUString > aDlgNames( rDocument.getObjectNames( E_DIALOGS, rLibName ) );
                sal_Int32 nDlgCount = aDlgNames.getLength();
                const OUString* pDlgNames = aDlgNames.getConstArray();

                for ( sal_Int32 i = 0 ; i < nDlgCount ; i++ )
                {
                    OUString aDlgName = pDlgNames[ i ];
                    SvTreeListEntry* pDialogEntry = FindEntry( pLibRootEntry, aDlgName, OBJ_TYPE_DIALOG );
                    if ( !pDialogEntry )
                    {
                        AddEntry(
                            aDlgName,
                            Image(BitmapEx(IDEResId(RID_BMP_DIALOG))),
                            pLibRootEntry, false,
                            o3tl::make_unique<Entry>(OBJ_TYPE_DIALOG));
                    }
                }
            }
            catch (const container::NoSuchElementException& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
    }
}

void TreeListBox::ImpCreateLibSubEntriesInVBAMode( SvTreeListEntry* pLibRootEntry, const ScriptDocument& rDocument, const OUString& rLibName )
{
    auto const aEntries = {
        std::make_pair( OBJ_TYPE_DOCUMENT_OBJECTS, IDE_RESSTR(RID_STR_DOCUMENT_OBJECTS) ),
        std::make_pair( OBJ_TYPE_USERFORMS, IDE_RESSTR(RID_STR_USERFORMS) ),
        std::make_pair( OBJ_TYPE_NORMAL_MODULES, IDE_RESSTR(RID_STR_NORMAL_MODULES) ),
        std::make_pair( OBJ_TYPE_CLASS_MODULES, IDE_RESSTR(RID_STR_CLASS_MODULES) ) };
    for( auto const & iter: aEntries )
    {
        EntryType eType = iter.first;
        OUString const & aEntryName = iter.second;
        SvTreeListEntry* pLibSubRootEntry = FindEntry( pLibRootEntry, aEntryName, eType );
        if( pLibSubRootEntry )
        {
            SetEntryBitmaps(pLibSubRootEntry, Image(BitmapEx(IDEResId(RID_BMP_MODLIB))));
            if ( IsExpanded( pLibSubRootEntry ) )
                ImpCreateLibSubSubEntriesInVBAMode( pLibSubRootEntry, rDocument, rLibName );
        }
        else
        {
            AddEntry(
                aEntryName,
                Image(BitmapEx(IDEResId(RID_BMP_MODLIB))),
                pLibRootEntry, true, o3tl::make_unique<Entry>(eType));
        }
    }
}

void TreeListBox::ImpCreateLibSubSubEntriesInVBAMode( SvTreeListEntry* pLibSubRootEntry, const ScriptDocument& rDocument, const OUString& rLibName )
{
    uno::Reference< container::XNameContainer > xLib = rDocument.getOrCreateLibrary( E_SCRIPTS, rLibName );
    if( !xLib.is() )
        return;

    try
    {
        // get a sorted list of module names
        Sequence< OUString > aModNames = rDocument.getObjectNames( E_SCRIPTS, rLibName );
        sal_Int32 nModCount = aModNames.getLength();
        const OUString* pModNames = aModNames.getConstArray();

        EntryDescriptor aDesc( GetEntryDescriptor( pLibSubRootEntry ) );
        EntryType eCurrentType( aDesc.GetType() );

        for ( sal_Int32 i = 0 ; i < nModCount ; i++ )
        {
            OUString aModName = pModNames[ i ];
            EntryType eType = OBJ_TYPE_UNKNOWN;
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
            OUString aEntryName = aModName;
            if( eType == OBJ_TYPE_DOCUMENT_OBJECTS )
            {
                OUString sObjName;
                ModuleInfoHelper::getObjectName( xLib, aModName, sObjName );
                if( !sObjName.isEmpty() )
                {
                    aEntryName += " (" + sObjName + ")";
                }
            }
            SvTreeListEntry* pModuleEntry = FindEntry( pLibSubRootEntry, aEntryName, OBJ_TYPE_MODULE );
            if ( !pModuleEntry )
            {
                pModuleEntry = AddEntry(
                    aEntryName,
                    Image(BitmapEx(IDEResId(RID_BMP_MODULE))),
                    pLibSubRootEntry, false,
                    o3tl::make_unique<Entry>(OBJ_TYPE_MODULE));
            }

            // methods
            if ( nMode & BROWSEMODE_SUBS )
            {
                Sequence< OUString > aNames = GetMethodNames( rDocument, rLibName, aModName );
                sal_Int32 nCount = aNames.getLength();
                const OUString* pNames = aNames.getConstArray();

                for ( sal_Int32 j = 0 ; j < nCount ; j++ )
                {
                    OUString aName = pNames[ j ];
                    SvTreeListEntry* pEntry = FindEntry( pModuleEntry, aName, OBJ_TYPE_METHOD );
                    if ( !pEntry )
                    {
                        AddEntry(
                            aName,
                            Image(BitmapEx(IDEResId(RID_BMP_MACRO))),
                            pModuleEntry, false,
                            o3tl::make_unique<Entry>(OBJ_TYPE_METHOD));
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

SvTreeListEntry* TreeListBox::ImpFindEntry( SvTreeListEntry* pParent, const OUString& rText )
{
    sal_uLong nRootPos = 0;
    SvTreeListEntry* pEntry = pParent ? FirstChild( pParent ) : GetEntry( nRootPos );
    while ( pEntry )
    {
        if (  rText.equals(GetEntryText( pEntry )) )
            return pEntry;

        pEntry = pParent ? NextSibling( pEntry ) : GetEntry( ++nRootPos );
    }
    return nullptr;
}

void TreeListBox::onDocumentCreated( const ScriptDocument& /*_rDocument*/ )
{
    UpdateEntries();
}

void TreeListBox::onDocumentOpened( const ScriptDocument& /*_rDocument*/ )
{
    UpdateEntries();
}

void TreeListBox::onDocumentSave( const ScriptDocument& /*_rDocument*/ )
{
    // not interested in
}

void TreeListBox::onDocumentSaveDone( const ScriptDocument& /*_rDocument*/ )
{
    // not interested in
}

void TreeListBox::onDocumentSaveAs( const ScriptDocument& /*_rDocument*/ )
{
    // not interested in
}

void TreeListBox::onDocumentSaveAsDone( const ScriptDocument& /*_rDocument*/ )
{
    UpdateEntries();
}

void TreeListBox::onDocumentClosed( const ScriptDocument& rDocument )
{
    UpdateEntries();
    // The document is not yet actually deleted, so we need to remove its entry
    // manually.
    RemoveEntry(rDocument);
}

void TreeListBox::onDocumentTitleChanged( const ScriptDocument& /*_rDocument*/ )
{
    // not interested in
}

void TreeListBox::onDocumentModeChanged( const ScriptDocument& /*_rDocument*/ )
{
    // not interested in
}

void TreeListBox::UpdateEntries()
{
    EntryDescriptor aCurDesc( GetEntryDescriptor( FirstSelected() ) );

    // removing the invalid entries
    SvTreeListEntry* pLastValid = nullptr;
    SvTreeListEntry* pEntry = First();
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
void TreeListBox::RemoveEntry (SvTreeListEntry* pEntry)
{
    // removing the associated user data
    delete static_cast<Entry*>(pEntry->GetUserData());
    // removing the entry
    GetModel()->Remove( pEntry );
}

// Removes the entry of rDocument.
void TreeListBox::RemoveEntry (ScriptDocument const& rDocument)
{
    // finding the entry of rDocument
    for (SvTreeListEntry* pEntry = First(); pEntry; pEntry = Next(pEntry))
        if (rDocument == GetEntryDescriptor(pEntry).GetDocument())
        {
            RemoveEntry(pEntry);
            break;
        }
}

SvTreeListEntry* TreeListBox::CloneEntry( SvTreeListEntry* pSource )
{
    SvTreeListEntry* pNew = SvTreeListBox::CloneEntry( pSource );
    Entry* pUser = static_cast<Entry*>(pSource->GetUserData());

    assert(pUser && "User-Daten?!");
    DBG_ASSERT( pUser->GetType() != OBJ_TYPE_DOCUMENT, "TreeListBox::CloneEntry: document?!" );

    Entry* pNewUser = new Entry( *pUser );
    pNew->SetUserData( pNewUser );
    return pNew;
}

SvTreeListEntry* TreeListBox::FindEntry( SvTreeListEntry* pParent, const OUString& rText, EntryType eType )
{
    sal_uLong nRootPos = 0;
    SvTreeListEntry* pEntry = pParent ? FirstChild( pParent ) : GetEntry( nRootPos );
    while ( pEntry )
    {
        Entry* pBasicEntry = static_cast<Entry*>(pEntry->GetUserData());
        assert(pBasicEntry && "FindEntry: no Entry ?!");
        if ( ( pBasicEntry->GetType() == eType  ) && ( rText.equals(GetEntryText( pEntry )) ) )
            return pEntry;

        pEntry = pParent ? NextSibling( pEntry ) : GetEntry( ++nRootPos );
    }
    return nullptr;
}

bool TreeListBox::ExpandingHdl()
{
    // expanding or collapsing?
    bool bOK = true;
    if ( GetModel()->GetDepth( GetHdlEntry() ) == 1 )
    {
        SvTreeListEntry* pCurEntry = GetCurEntry();
        EntryDescriptor aDesc( GetEntryDescriptor( pCurEntry ) );
        ScriptDocument aDocument( aDesc.GetDocument() );
        OSL_ENSURE( aDocument.isAlive(), "TreeListBox::ExpandingHdl: no document, or document is dead!" );
        if ( aDocument.isAlive() )
        {
            OUString aLibName( aDesc.GetLibName() );
            OUString aLibSubName( aDesc.GetLibSubName() );
            OUString aName( aDesc.GetName() );
            OUString aMethodName( aDesc.GetMethodName() );

            if ( !aLibName.isEmpty() && aLibSubName.isEmpty() && aName.isEmpty() && aMethodName.isEmpty() )
            {
                // check password, if library is password protected and not verified
                Reference< script::XLibraryContainer > xModLibContainer( aDocument.getLibraryContainer( E_SCRIPTS ) );
                if ( xModLibContainer.is() && xModLibContainer->hasByName( aLibName ) )
                {
                    Reference< script::XLibraryContainerPassword > xPasswd( xModLibContainer, UNO_QUERY );
                    if ( xPasswd.is() && xPasswd->isLibraryPasswordProtected( aLibName ) && !xPasswd->isLibraryPasswordVerified( aLibName ) )
                    {
                        OUString aPassword;
                        bOK = QueryPassword( xModLibContainer, aLibName, aPassword );
                    }
                }
            }
        }
    }
    return bOK;
}

bool TreeListBox::IsEntryProtected( SvTreeListEntry* pEntry )
{
    bool bProtected = false;
    if ( pEntry && ( GetModel()->GetDepth( pEntry ) == 1 ) )
    {
        EntryDescriptor aDesc( GetEntryDescriptor( pEntry ) );
        ScriptDocument aDocument( aDesc.GetDocument() );
        OSL_ENSURE( aDocument.isAlive(), "TreeListBox::IsEntryProtected: no document, or document is dead!" );
        if ( aDocument.isAlive() )
        {
            OUString aOULibName( aDesc.GetLibName() );
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

SvTreeListEntry* TreeListBox::AddEntry(
    OUString const& rText,
    const Image& rImage,
    SvTreeListEntry* pParent,
    bool bChildrenOnDemand,
    std::unique_ptr<Entry> && aUserData
)
{
    SvTreeListEntry* p = InsertEntry(
        rText, rImage, rImage, pParent, bChildrenOnDemand, TREELIST_APPEND,
        aUserData.get()
    );
    aUserData.release();
    return p;
}

void TreeListBox::SetEntryBitmaps( SvTreeListEntry * pEntry, const Image& rImage )
{
    SetExpandedEntryBmp(  pEntry, rImage );
    SetCollapsedEntryBmp( pEntry, rImage );
}

LibraryType TreeListBox::GetLibraryType() const
{
    LibraryType eType = LibraryType::All;
    if ( ( nMode & BROWSEMODE_MODULES ) && !( nMode & BROWSEMODE_DIALOGS ) )
        eType = LibraryType::Module;
    else if ( !( nMode & BROWSEMODE_MODULES ) && ( nMode & BROWSEMODE_DIALOGS ) )
        eType = LibraryType::Dialog;
    return eType;
}

OUString TreeListBox::GetRootEntryName( const ScriptDocument& rDocument, LibraryLocation eLocation ) const
{
    return rDocument.getTitle( eLocation, GetLibraryType() );
}

void TreeListBox::GetRootEntryBitmaps( const ScriptDocument& rDocument, Image& rImage )
{
    OSL_ENSURE( rDocument.isValid(), "TreeListBox::GetRootEntryBitmaps: illegal document!" );
    if ( !rDocument.isValid() )
        return;

    if ( rDocument.isDocument() )
    {
        OUString sFactoryURL;
        Reference<uno::XComponentContext> xContext( ::comphelper::getProcessComponentContext() );
        Reference< frame::XModuleManager2 > xModuleManager( frame::ModuleManager::create(xContext) );
        try
        {
            OUString sModule( xModuleManager->identify( rDocument.getDocument() ) );
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

        if ( !sFactoryURL.isEmpty() )
        {
            rImage = SvFileInformationManager::GetFileImage( INetURLObject( sFactoryURL ) );
        }
        else
        {
            // default icon
            rImage = Image(BitmapEx(IDEResId(RID_BMP_DOCUMENT)));
        }
    }
    else
    {
        rImage = Image(BitmapEx(IDEResId(RID_BMP_INSTALLATION)));
    }
}

void TreeListBox::SetCurrentEntry (EntryDescriptor& rDesc)
{
    SvTreeListEntry* pCurEntry = nullptr;
    EntryDescriptor aDesc = rDesc;
    if ( aDesc.GetType() == OBJ_TYPE_UNKNOWN )
    {
        aDesc = EntryDescriptor(
            ScriptDocument::getApplicationScriptDocument(),
            LIBRARY_LOCATION_USER, "Standard",
            OUString(), ".", OBJ_TYPE_UNKNOWN
        );
    }
    ScriptDocument aDocument = aDesc.GetDocument();
    OSL_ENSURE( aDocument.isValid(), "TreeListBox::SetCurrentEntry: invalid document!" );
    LibraryLocation eLocation = aDesc.GetLocation();
    SvTreeListEntry* pRootEntry = FindRootEntry( aDocument, eLocation );
    if ( pRootEntry )
    {
        pCurEntry = pRootEntry;
        OUString aLibName( aDesc.GetLibName() );
        if ( !aLibName.isEmpty() )
        {
            Expand( pRootEntry );
            SvTreeListEntry* pLibEntry = FindEntry( pRootEntry, aLibName, OBJ_TYPE_LIBRARY );
            if ( pLibEntry )
            {
                pCurEntry = pLibEntry;
                OUString aLibSubName( aDesc.GetLibSubName() );
                if( !aLibSubName.isEmpty() )
                {
                    Expand( pLibEntry );
                    SvTreeListEntry* pLibSubEntry = ImpFindEntry( pLibEntry, aLibSubName );
                    if( pLibSubEntry )
                    {
                        pCurEntry = pLibSubEntry;
                    }
                }
                OUString aName( aDesc.GetName() );
                if ( !aName.isEmpty() )
                {
                    Expand( pCurEntry );
                    EntryType eType = OBJ_TYPE_MODULE;
                    if ( aDesc.GetType() == OBJ_TYPE_DIALOG )
                        eType = OBJ_TYPE_DIALOG;
                    SvTreeListEntry* pEntry = FindEntry( pCurEntry, aName, eType );
                    if ( pEntry )
                    {
                        pCurEntry = pEntry;
                        OUString aMethodName( aDesc.GetMethodName() );
                        if ( !aMethodName.isEmpty() )
                        {
                            Expand( pEntry );
                            SvTreeListEntry* pSubEntry = FindEntry( pEntry, aMethodName, OBJ_TYPE_METHOD );
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

void TreeListBox::MouseButtonDown( const MouseEvent& rMEvt )
{
    SvTreeListBox::MouseButtonDown( rMEvt );
    if ( rMEvt.IsLeft() && ( rMEvt.GetClicks() == 2 ) )
    {
        OpenCurrent();
    }
}

void TreeListBox::KeyInput( const KeyEvent& rEvt )
{
    if ( rEvt.GetKeyCode() == KEY_RETURN && OpenCurrent() )
    {
        return;
    }
    SvTreeListBox::KeyInput( rEvt );
}

bool TreeListBox::OpenCurrent()
{
    EntryDescriptor aDesc = GetEntryDescriptor(GetCurEntry());
    switch (aDesc.GetType())
    {
        case OBJ_TYPE_METHOD:
        case OBJ_TYPE_MODULE:
        case OBJ_TYPE_DIALOG:
            if (SfxDispatcher* pDispatcher = GetDispatcher())
            {
                SbxItem aSbxItem(
                    SID_BASICIDE_ARG_SBX, aDesc.GetDocument(),
                    aDesc.GetLibName(), aDesc.GetName(), aDesc.GetMethodName(),
                    ConvertType(aDesc.GetType())
                );
                pDispatcher->ExecuteList(
                    SID_BASICIDE_SHOWSBX, SfxCallMode::SYNCHRON,
                    { &aSbxItem }
                );
                return true;
            }
            break;

        default:
            break;
    }
    return false;
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
