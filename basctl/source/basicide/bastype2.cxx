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
#include <strings.hrc>
#include <bitmaps.hlst>
#include <iderid.hxx>
#include <tools/urlobj.hxx>
#include <tools/diagnose_ex.h>
#include <tools/debug.hxx>
#include <svtools/imagemgr.hxx>
#include <vcl/treelistentry.hxx>
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
    OUString const& rLibName
) :
    DocumentEntry(rDocument, eLocation, OBJ_TYPE_LIBRARY),
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
    nMode = BrowseMode::All;   // everything
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
            nullptr, true, std::make_unique<DocumentEntry>(rDocument, eLocation));
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
            OUString sId;
            if ( ( nMode & BrowseMode::Dialogs ) && !( nMode & BrowseMode::Modules ) )
                sId = bLoaded ? OUStringLiteral(RID_BMP_DLGLIB) : OUStringLiteral(RID_BMP_DLGLIBNOTLOADED);
            else
                sId = bLoaded ? OUStringLiteral(RID_BMP_MODLIB) : OUStringLiteral(RID_BMP_MODLIBNOTLOADED);
            SvTreeListEntry* pLibRootEntry = FindEntry( pDocumentRootEntry, aLibName, OBJ_TYPE_LIBRARY );
            if ( pLibRootEntry )
            {
                SetEntryBitmaps(pLibRootEntry, Image(StockImage::Yes, sId));
                if ( IsExpanded(pLibRootEntry))
                    ImpCreateLibSubEntries( pLibRootEntry, rDocument, aLibName );
            }
            else
            {
                AddEntry(
                    aLibName,
                    Image(StockImage::Yes, sId),
                    pDocumentRootEntry, true,
                    std::make_unique<Entry>(OBJ_TYPE_LIBRARY));
            }
        }
    }
}

void TreeListBox::ImpCreateLibSubEntries( SvTreeListEntry* pLibRootEntry, const ScriptDocument& rDocument, const OUString& rLibName )
{
    // modules
    if ( nMode & BrowseMode::Modules )
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
                                Image(StockImage::Yes, RID_BMP_MODULE),
                                pLibRootEntry, false,
                                std::make_unique<Entry>(OBJ_TYPE_MODULE));
                        }

                        // methods
                        if ( nMode & BrowseMode::Subs )
                        {
                            Sequence< OUString > aNames = GetMethodNames( rDocument, rLibName, aModName );
                            FillTreeListBox( pModuleEntry, aNames, OBJ_TYPE_METHOD, RID_BMP_MACRO );
                        }
                    }
                }
            }
            catch ( const container::NoSuchElementException& )
            {
                DBG_UNHANDLED_EXCEPTION("basctl.basicide");
            }
        }
    }

    // dialogs
    if ( nMode & BrowseMode::Dialogs )
    {
         Reference< script::XLibraryContainer > xDlgLibContainer( rDocument.getLibraryContainer( E_DIALOGS ) );

         if ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( rLibName ) && xDlgLibContainer->isLibraryLoaded( rLibName ) )
         {
            try
            {
                // get a sorted list of dialog names
                Sequence< OUString > aDlgNames( rDocument.getObjectNames( E_DIALOGS, rLibName ) );
                FillTreeListBox( pLibRootEntry, aDlgNames, OBJ_TYPE_DIALOG, RID_BMP_DIALOG );
            }
            catch (const container::NoSuchElementException& )
            {
                DBG_UNHANDLED_EXCEPTION("basctl.basicide");
            }
        }
    }
}

void TreeListBox::ImpCreateLibSubEntriesInVBAMode( SvTreeListEntry* pLibRootEntry, const ScriptDocument& rDocument, const OUString& rLibName )
{
    auto const aEntries = {
        std::make_pair( OBJ_TYPE_DOCUMENT_OBJECTS, IDEResId(RID_STR_DOCUMENT_OBJECTS) ),
        std::make_pair( OBJ_TYPE_USERFORMS, IDEResId(RID_STR_USERFORMS) ),
        std::make_pair( OBJ_TYPE_NORMAL_MODULES, IDEResId(RID_STR_NORMAL_MODULES) ),
        std::make_pair( OBJ_TYPE_CLASS_MODULES, IDEResId(RID_STR_CLASS_MODULES) ) };
    for( auto const & iter: aEntries )
    {
        EntryType eType = iter.first;
        OUString const & aEntryName = iter.second;
        SvTreeListEntry* pLibSubRootEntry = FindEntry( pLibRootEntry, aEntryName, eType );
        if( pLibSubRootEntry )
        {
            SetEntryBitmaps(pLibSubRootEntry, Image(StockImage::Yes, RID_BMP_MODLIB));
            if ( IsExpanded( pLibSubRootEntry ) )
                ImpCreateLibSubSubEntriesInVBAMode( pLibSubRootEntry, rDocument, rLibName );
        }
        else
        {
            AddEntry(
                aEntryName,
                Image(StockImage::Yes, RID_BMP_MODLIB),
                pLibRootEntry, true, std::make_unique<Entry>(eType));
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
                    Image(StockImage::Yes, RID_BMP_MODULE),
                    pLibSubRootEntry, false,
                    std::make_unique<Entry>(OBJ_TYPE_MODULE));
            }

            // methods
            if ( nMode & BrowseMode::Subs )
            {
                Sequence< OUString > aNames = GetMethodNames( rDocument, rLibName, aModName );
                FillTreeListBox( pModuleEntry, aNames, OBJ_TYPE_METHOD, RID_BMP_MACRO );
            }
        }
    }
    catch ( const container::NoSuchElementException& )
    {
        DBG_UNHANDLED_EXCEPTION("basctl.basicide");
    }
}

SvTreeListEntry* TreeListBox::ImpFindEntry( SvTreeListEntry* pParent, const OUString& rText )
{
    sal_uLong nRootPos = 0;
    SvTreeListEntry* pEntry = pParent ? FirstChild( pParent ) : GetEntry( nRootPos );
    while ( pEntry )
    {
        if (  rText == GetEntryText( pEntry ) )
            return pEntry;

        pEntry = pParent ? pEntry->NextSibling() : GetEntry( ++nRootPos );
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
void TreeListBox::RemoveEntry (SvTreeListEntry const * pEntry)
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

    assert(pUser && "User data?!");
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
        if ( ( pBasicEntry->GetType() == eType  ) && ( rText == GetEntryText( pEntry ) ) )
            return pEntry;

        pEntry = pParent ? pEntry->NextSibling() : GetEntry( ++nRootPos );
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
        const ScriptDocument& aDocument( aDesc.GetDocument() );
        OSL_ENSURE( aDocument.isAlive(), "TreeListBox::ExpandingHdl: no document, or document is dead!" );
        if ( aDocument.isAlive() )
        {
            const OUString& aLibName( aDesc.GetLibName() );
            const OUString& aLibSubName( aDesc.GetLibSubName() );
            const OUString& aName( aDesc.GetName() );
            const OUString& aMethodName( aDesc.GetMethodName() );

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

//Fills up treelist for macros and dialogs
void TreeListBox::FillTreeListBox( SvTreeListEntry* pRootEntry, const Sequence< OUString >& rNames,
                            const EntryType& eType, const OUString& aBmpMacro )
{
    sal_Int32 nCount = rNames.getLength();
    const OUString* pNames = rNames.getConstArray();

    for ( sal_Int32 j = 0 ; j < nCount ; j++ )
    {
        OUString aName = pNames[ j ];
        SvTreeListEntry* pEntry = FindEntry( pRootEntry, aName, eType );

        if ( !pEntry )
        {
            AddEntry(
                aName,
                Image(StockImage::Yes, aBmpMacro),
                pRootEntry, false,
                std::make_unique<Entry>( eType ));
        }
    }
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
    if ( ( nMode & BrowseMode::Modules ) && !( nMode & BrowseMode::Dialogs ) )
        eType = LibraryType::Module;
    else if ( !( nMode & BrowseMode::Modules ) && ( nMode & BrowseMode::Dialogs ) )
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
            DBG_UNHANDLED_EXCEPTION("basctl.basicide");
        }

        if ( !sFactoryURL.isEmpty() )
        {
            rImage = SvFileInformationManager::GetFileImage( INetURLObject( sFactoryURL ) );
        }
        else
        {
            // default icon
            rImage = Image(StockImage::Yes, RID_BMP_DOCUMENT);
        }
    }
    else
    {
        rImage = Image(StockImage::Yes, RID_BMP_INSTALLATION);
    }
}

void TreeListBox::SetCurrentEntry (EntryDescriptor const & rDesc)
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
        const OUString& aLibName( aDesc.GetLibName() );
        if ( !aLibName.isEmpty() )
        {
            Expand( pRootEntry );
            SvTreeListEntry* pLibEntry = FindEntry( pRootEntry, aLibName, OBJ_TYPE_LIBRARY );
            if ( pLibEntry )
            {
                pCurEntry = pLibEntry;
                const OUString& aLibSubName( aDesc.GetLibSubName() );
                if( !aLibSubName.isEmpty() )
                {
                    Expand( pLibEntry );
                    SvTreeListEntry* pLibSubEntry = ImpFindEntry( pLibEntry, aLibSubName );
                    if( pLibSubEntry )
                    {
                        pCurEntry = pLibSubEntry;
                    }
                }
                const OUString& aName( aDesc.GetName() );
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
                        const OUString& aMethodName( aDesc.GetMethodName() );
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

SbTreeListBox::SbTreeListBox(std::unique_ptr<weld::TreeView> xControl, weld::Window* pTopLevel)
    : m_xControl(std::move(xControl))
    , m_xIter(m_xControl->make_iterator())
    , m_pTopLevel(pTopLevel)
    , m_aNotifier(*this)
{
    m_xControl->connect_row_activated(LINK(this, SbTreeListBox, OpenCurrentHdl));
    m_xControl->connect_expanding(LINK(this, SbTreeListBox, RequestingChildrenHdl));
    nMode = BrowseMode::All;   // everything
}

SbTreeListBox::~SbTreeListBox()
{
    m_aNotifier.dispose();

    bool bValidIter = m_xControl->get_iter_first(*m_xIter);
    while (bValidIter)
    {
        Entry* pBasicEntry = reinterpret_cast<Entry*>(m_xControl->get_id(*m_xIter).toInt64());
        delete pBasicEntry;
        bValidIter = m_xControl->iter_next(*m_xIter);
    }
}

void SbTreeListBox::ScanEntry( const ScriptDocument& rDocument, LibraryLocation eLocation )
{
    OSL_ENSURE( rDocument.isAlive(), "TreeListBox::ScanEntry: illegal document!" );
    if ( !rDocument.isAlive() )
        return;

    // can be called multiple times for updating!

    // actually test if basic's in the tree already?!
    m_xControl->freeze();
    // level 1: BasicManager (application, document, ...)
    bool bDocumentRootEntry = FindRootEntry(rDocument, eLocation, *m_xIter);
    if (bDocumentRootEntry && m_xControl->get_row_expanded(*m_xIter))
        ImpCreateLibEntries(*m_xIter, rDocument, eLocation);
    if (!bDocumentRootEntry)
    {
        OUString aRootName(GetRootEntryName(rDocument, eLocation));
        OUString aImage(GetRootEntryBitmaps(rDocument));
        AddEntry(aRootName, aImage, nullptr, true, std::make_unique<DocumentEntry>(rDocument, eLocation));
    }
    m_xControl->thaw();
}

void SbTreeListBox::ImpCreateLibEntries(const weld::TreeIter& rIter, const ScriptDocument& rDocument, LibraryLocation eLocation)
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
            OUString sId;
            if ( ( nMode & BrowseMode::Dialogs ) && !( nMode & BrowseMode::Modules ) )
                sId = bLoaded ? OUStringLiteral(RID_BMP_DLGLIB) : OUStringLiteral(RID_BMP_DLGLIBNOTLOADED);
            else
                sId = bLoaded ? OUStringLiteral(RID_BMP_MODLIB) : OUStringLiteral(RID_BMP_MODLIBNOTLOADED);
            std::unique_ptr<weld::TreeIter> xLibRootEntry(m_xControl->make_iterator(&rIter));
            bool bLibRootEntry = FindEntry(aLibName, OBJ_TYPE_LIBRARY, *xLibRootEntry);
            if (bLibRootEntry)
            {
                SetEntryBitmaps(*xLibRootEntry, sId);
                if (m_xControl->get_row_expanded(*xLibRootEntry))
                    ImpCreateLibSubEntries(*xLibRootEntry, rDocument, aLibName);
            }
            else
            {
                AddEntry(aLibName, sId, &rIter, true, std::make_unique<Entry>(OBJ_TYPE_LIBRARY));
            }
        }
    }
}

void SbTreeListBox::ImpCreateLibSubEntries(const weld::TreeIter& rLibRootEntry, const ScriptDocument& rDocument, const OUString& rLibName)
{
    // modules
    if ( nMode & BrowseMode::Modules )
    {
        Reference< script::XLibraryContainer > xModLibContainer( rDocument.getLibraryContainer( E_SCRIPTS ) );

        if ( xModLibContainer.is() && xModLibContainer->hasByName( rLibName ) && xModLibContainer->isLibraryLoaded( rLibName ) )
        {
            try
            {
                if( rDocument.isInVBAMode() )
                {
                    ImpCreateLibSubEntriesInVBAMode(rLibRootEntry, rDocument, rLibName);
                }
                else
                {
                    // get a sorted list of module names
                    Sequence< OUString > aModNames = rDocument.getObjectNames( E_SCRIPTS, rLibName );
                    sal_Int32 nModCount = aModNames.getLength();
                    const OUString* pModNames = aModNames.getConstArray();

                    auto xTreeIter = m_xControl->make_iterator();

                    for ( sal_Int32 i = 0 ; i < nModCount ; i++ )
                    {
                        OUString aModName = pModNames[ i ];
                        m_xControl->copy_iterator(rLibRootEntry, *xTreeIter);
                        bool bModuleEntry = FindEntry(aModName, OBJ_TYPE_MODULE, *xTreeIter);
                        if (!bModuleEntry)
                        {
                            AddEntry(aModName, RID_BMP_MODULE, &rLibRootEntry, false, std::make_unique<Entry>(OBJ_TYPE_MODULE));
                        }

                        // methods
                        if ( nMode & BrowseMode::Subs )
                        {
                            Sequence< OUString > aNames = GetMethodNames( rDocument, rLibName, aModName );
                            sal_Int32 nCount = aNames.getLength();
                            const OUString* pNames = aNames.getConstArray();

                            auto xSubTreeIter = m_xControl->make_iterator();

                            for ( sal_Int32 j = 0 ; j < nCount ; j++ )
                            {
                                OUString aName = pNames[ j ];
                                m_xControl->copy_iterator(*xTreeIter, *xSubTreeIter);
                                bool bEntry = FindEntry(aName, OBJ_TYPE_METHOD, *xSubTreeIter);
                                if (!bEntry)
                                {
                                    AddEntry(aName, RID_BMP_MACRO, xTreeIter.get(), false, std::make_unique<Entry>(OBJ_TYPE_METHOD));
                                }
                            }
                        }
                    }
                }
            }
            catch ( const container::NoSuchElementException& )
            {
                DBG_UNHANDLED_EXCEPTION("basctl.basicide");
            }
        }
    }

    // dialogs
    if ( nMode & BrowseMode::Dialogs )
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

                auto xTreeIter = m_xControl->make_iterator();

                for ( sal_Int32 i = 0 ; i < nDlgCount ; i++ )
                {
                    OUString aDlgName = pDlgNames[ i ];
                    m_xControl->copy_iterator(rLibRootEntry, *xTreeIter);
                    bool bDialogEntry = FindEntry(aDlgName, OBJ_TYPE_DIALOG, *xTreeIter);
                    if (!bDialogEntry)
                    {
                        AddEntry(aDlgName, RID_BMP_DIALOG, &rLibRootEntry, false, std::make_unique<Entry>(OBJ_TYPE_DIALOG));
                    }
                }
            }
            catch (const container::NoSuchElementException& )
            {
                DBG_UNHANDLED_EXCEPTION("basctl.basicide");
            }
        }
    }
}

void SbTreeListBox::ImpCreateLibSubEntriesInVBAMode(const weld::TreeIter& rLibRootEntry, const ScriptDocument& rDocument, const OUString& rLibName )
{
    auto const aEntries = {
        std::make_pair( OBJ_TYPE_DOCUMENT_OBJECTS, IDEResId(RID_STR_DOCUMENT_OBJECTS) ),
        std::make_pair( OBJ_TYPE_USERFORMS, IDEResId(RID_STR_USERFORMS) ),
        std::make_pair( OBJ_TYPE_NORMAL_MODULES, IDEResId(RID_STR_NORMAL_MODULES) ),
        std::make_pair( OBJ_TYPE_CLASS_MODULES, IDEResId(RID_STR_CLASS_MODULES) ) };
    for( auto const & iter: aEntries )
    {
        EntryType eType = iter.first;
        OUString const & aEntryName = iter.second;
        std::unique_ptr<weld::TreeIter> xLibSubRootEntry(m_xControl->make_iterator(&rLibRootEntry));
        bool bLibSubRootEntry = FindEntry(aEntryName, eType, *xLibSubRootEntry);
        if (bLibSubRootEntry)
        {
            SetEntryBitmaps(*xLibSubRootEntry, RID_BMP_MODLIB);
            if (m_xControl->get_row_expanded(*xLibSubRootEntry))
                ImpCreateLibSubSubEntriesInVBAMode(*xLibSubRootEntry, rDocument, rLibName);
        }
        else
        {
            m_xControl->copy_iterator(rLibRootEntry, *xLibSubRootEntry);
            AddEntry(aEntryName, RID_BMP_MODLIB, xLibSubRootEntry.get(), true, std::make_unique<Entry>(eType));
        }
    }
}

void SbTreeListBox::ImpCreateLibSubSubEntriesInVBAMode(const weld::TreeIter& rLibSubRootEntry, const ScriptDocument& rDocument, const OUString& rLibName)
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

        EntryDescriptor aDesc(GetEntryDescriptor(&rLibSubRootEntry));
        EntryType eCurrentType(aDesc.GetType());

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
            std::unique_ptr<weld::TreeIter> xModuleEntry(m_xControl->make_iterator(&rLibSubRootEntry));
            bool bModuleEntry = FindEntry(aEntryName, OBJ_TYPE_MODULE, *xModuleEntry);
            if (!bModuleEntry)
            {
                m_xControl->copy_iterator(rLibSubRootEntry, *xModuleEntry);
                AddEntry(aEntryName, RID_BMP_MODULE, xModuleEntry.get(), false,
                         std::make_unique<Entry>(OBJ_TYPE_MODULE));
            }

            // methods
            if ( nMode & BrowseMode::Subs )
            {
                Sequence< OUString > aNames = GetMethodNames( rDocument, rLibName, aModName );
                sal_Int32 nCount = aNames.getLength();
                const OUString* pNames = aNames.getConstArray();

                for ( sal_Int32 j = 0 ; j < nCount ; j++ )
                {
                    OUString aName = pNames[ j ];
                    std::unique_ptr<weld::TreeIter> xEntry(m_xControl->make_iterator(xModuleEntry.get()));
                    bool bEntry = FindEntry(aName, OBJ_TYPE_METHOD, *xEntry);
                    if (!bEntry)
                    {
                        AddEntry(aName, RID_BMP_MACRO, xModuleEntry.get(), false, std::make_unique<Entry>(OBJ_TYPE_METHOD));
                    }
                }
            }
        }
    }
    catch ( const container::NoSuchElementException& )
    {
        DBG_UNHANDLED_EXCEPTION("basctl.basicide");
    }
}

bool SbTreeListBox::ImpFindEntry(weld::TreeIter& rIter, const OUString& rText)
{
    bool bValidIter = m_xControl->iter_children(rIter);
    while (bValidIter)
    {
        if (rText == m_xControl->get_text(rIter))
            return true;
        bValidIter = m_xControl->iter_next_sibling(rIter);
    }
    return false;
}

void SbTreeListBox::onDocumentCreated( const ScriptDocument& /*_rDocument*/ )
{
    UpdateEntries();
}

void SbTreeListBox::onDocumentOpened( const ScriptDocument& /*_rDocument*/ )
{
    UpdateEntries();
}

void SbTreeListBox::onDocumentSave( const ScriptDocument& /*_rDocument*/ )
{
    // not interested in
}

void SbTreeListBox::onDocumentSaveDone( const ScriptDocument& /*_rDocument*/ )
{
    // not interested in
}

void SbTreeListBox::onDocumentSaveAs( const ScriptDocument& /*_rDocument*/ )
{
    // not interested in
}

void SbTreeListBox::onDocumentSaveAsDone( const ScriptDocument& /*_rDocument*/ )
{
    UpdateEntries();
}

void SbTreeListBox::onDocumentClosed( const ScriptDocument& rDocument )
{
    UpdateEntries();
    // The document is not yet actually deleted, so we need to remove its entry
    // manually.
    RemoveEntry(rDocument);
}

void SbTreeListBox::onDocumentTitleChanged( const ScriptDocument& /*_rDocument*/ )
{
    // not interested in
}

void SbTreeListBox::onDocumentModeChanged( const ScriptDocument& /*_rDocument*/ )
{
    // not interested in
}

void SbTreeListBox::UpdateEntries()
{
    bool bValidIter = m_xControl->get_selected(m_xIter.get());
    EntryDescriptor aCurDesc(GetEntryDescriptor(bValidIter ? m_xIter.get() : nullptr));

    // removing the invalid entries
    std::unique_ptr<weld::TreeIter> xLastValid(m_xControl->make_iterator(nullptr));
    bool bLastValid = false;
    bValidIter = m_xControl->get_iter_first(*m_xIter);
    while (bValidIter)
    {
        if (IsValidEntry(*m_xIter))
        {
            m_xControl->copy_iterator(*m_xIter, *xLastValid);
            bLastValid = true;
        }
        else
            RemoveEntry(*m_xIter);
        if (bLastValid)
        {
            m_xControl->copy_iterator(*xLastValid, *m_xIter);
            bValidIter = m_xControl->iter_next(*m_xIter);
        }
        else
            bValidIter = m_xControl->get_iter_first(*m_xIter);
    }

    ScanAllEntries();

    SetCurrentEntry( aCurDesc );
}

// Removes the entry from the tree.
void SbTreeListBox::RemoveEntry(const weld::TreeIter& rIter)
{
    // removing the associated user data
    Entry* pBasicEntry = reinterpret_cast<Entry*>(m_xControl->get_id(rIter).toInt64());
    delete pBasicEntry;
    // removing the entry
    m_xControl->remove(rIter);
}

// Removes the entry of rDocument.
void SbTreeListBox::RemoveEntry (ScriptDocument const& rDocument)
{
    // finding the entry of rDocument
    bool bValidIter = m_xControl->get_iter_first(*m_xIter);
    while (bValidIter)
    {
        if (rDocument == GetEntryDescriptor(m_xIter.get()).GetDocument())
        {
            RemoveEntry(*m_xIter);
            break;
        }
        bValidIter = m_xControl->iter_next(*m_xIter);
    }
}

bool SbTreeListBox::FindEntry(const OUString& rText, EntryType eType, weld::TreeIter& rIter)
{
    bool bValidIter = m_xControl->iter_children(rIter);
    while (bValidIter)
    {
        Entry* pBasicEntry = reinterpret_cast<Entry*>(m_xControl->get_id(rIter).toInt64());
        assert(pBasicEntry && "FindEntry: no Entry ?!");
        if (pBasicEntry->GetType() == eType && rText == m_xControl->get_text(rIter))
            return true;
        bValidIter = m_xControl->iter_next_sibling(rIter);
    }
    return false;
}

bool SbTreeListBox::IsEntryProtected(const weld::TreeIter* pEntry)
{
    bool bProtected = false;
    if (pEntry && m_xControl->get_iter_depth(*pEntry) == 1)
    {
        EntryDescriptor aDesc(GetEntryDescriptor(pEntry));
        const ScriptDocument& rDocument( aDesc.GetDocument() );
        OSL_ENSURE( rDocument.isAlive(), "TreeListBox::IsEntryProtected: no document, or document is dead!" );
        if ( rDocument.isAlive() )
        {
            const OUString& aOULibName( aDesc.GetLibName() );
            Reference< script::XLibraryContainer > xModLibContainer( rDocument.getLibraryContainer( E_SCRIPTS ) );
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

void SbTreeListBox::AddEntry(
    const OUString& rText,
    const OUString& rImage,
    const weld::TreeIter* pParent,
    bool bChildrenOnDemand,
    std::unique_ptr<Entry>&& rUserData)
{
    OUString sId(OUString::number(reinterpret_cast<sal_uInt64>(rUserData.release())));
    m_xControl->insert(pParent, -1, &rText, &sId, nullptr, nullptr, &rImage, bChildrenOnDemand, nullptr);
}

void SbTreeListBox::SetEntryBitmaps(const weld::TreeIter& rIter, const OUString& rImage)
{
    m_xControl->set_image(rIter, rImage, -1);
}

LibraryType SbTreeListBox::GetLibraryType() const
{
    LibraryType eType = LibraryType::All;
    if ( ( nMode & BrowseMode::Modules ) && !( nMode & BrowseMode::Dialogs ) )
        eType = LibraryType::Module;
    else if ( !( nMode & BrowseMode::Modules ) && ( nMode & BrowseMode::Dialogs ) )
        eType = LibraryType::Dialog;
    return eType;
}

OUString SbTreeListBox::GetRootEntryName( const ScriptDocument& rDocument, LibraryLocation eLocation ) const
{
    return rDocument.getTitle( eLocation, GetLibraryType() );
}

OUString SbTreeListBox::GetRootEntryBitmaps(const ScriptDocument& rDocument)
{
    OSL_ENSURE( rDocument.isValid(), "TreeListBox::GetRootEntryBitmaps: illegal document!" );
    if (!rDocument.isValid())
        return OUString();

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
            DBG_UNHANDLED_EXCEPTION("basctl.basicide");
        }

        if ( !sFactoryURL.isEmpty() )
        {
            return SvFileInformationManager::GetFileImageId(INetURLObject(sFactoryURL));
        }
        else
        {
            // default icon
            return OUString(RID_BMP_DOCUMENT);
        }
    }
    return OUString(RID_BMP_INSTALLATION);
}

void SbTreeListBox::SetCurrentEntry (EntryDescriptor const & rDesc)
{
    bool bCurEntry = false;
    auto xCurIter = m_xControl->make_iterator();
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
    bool bRootEntry = FindRootEntry(aDocument, eLocation, *m_xIter);
    if (bRootEntry)
    {
        m_xControl->copy_iterator(*m_xIter, *xCurIter);
        bCurEntry = true;
        const OUString& aLibName( aDesc.GetLibName() );
        if ( !aLibName.isEmpty() )
        {
            m_xControl->expand_row(*m_xIter);
            auto xLibIter = m_xControl->make_iterator(m_xIter.get());
            bool bLibEntry = FindEntry(aLibName, OBJ_TYPE_LIBRARY, *xLibIter);
            if (bLibEntry)
            {
                m_xControl->copy_iterator(*xLibIter, *xCurIter);
                const OUString& aLibSubName( aDesc.GetLibSubName() );
                if( !aLibSubName.isEmpty() )
                {
                    m_xControl->expand_row(*xLibIter);
                    auto xSubLibIter = m_xControl->make_iterator(xLibIter.get());
                    bool bSubLibEntry = ImpFindEntry(*xSubLibIter, aLibSubName);
                    if (bSubLibEntry)
                    {
                        m_xControl->copy_iterator(*xSubLibIter, *xCurIter);
                    }
                }
                const OUString& aName( aDesc.GetName() );
                if ( !aName.isEmpty() )
                {
                    m_xControl->expand_row(*xCurIter);
                    EntryType eType = OBJ_TYPE_MODULE;
                    if ( aDesc.GetType() == OBJ_TYPE_DIALOG )
                        eType = OBJ_TYPE_DIALOG;
                    auto xEntryIter = m_xControl->make_iterator(xCurIter.get());
                    bool bEntry = FindEntry(aName, eType, *xEntryIter);
                    if (bEntry)
                    {
                        m_xControl->copy_iterator(*xEntryIter, *xCurIter);
                        const OUString& aMethodName( aDesc.GetMethodName() );
                        if (!aMethodName.isEmpty())
                        {
                            m_xControl->expand_row(*xCurIter);
                            auto xSubEntryIter = m_xControl->make_iterator(xCurIter.get());
                            bool bSubEntry = FindEntry(aMethodName, OBJ_TYPE_METHOD, *xSubEntryIter);
                            if (bSubEntry)
                            {
                                m_xControl->copy_iterator(*xSubEntryIter, *xCurIter);
                            }
                            else
                            {
                                m_xControl->copy_iterator(*xCurIter, *xSubEntryIter);
                                if (m_xControl->iter_children(*xSubEntryIter))
                                    m_xControl->copy_iterator(*xSubEntryIter, *xCurIter);
                            }
                        }
                    }
                    else
                    {
                        auto xSubEntryIter = m_xControl->make_iterator(xCurIter.get());
                        if (m_xControl->iter_children(*xSubEntryIter))
                            m_xControl->copy_iterator(*xSubEntryIter, *xCurIter);
                    }
                }
            }
            else
            {
                auto xSubLibIter = m_xControl->make_iterator(m_xIter.get());
                if (m_xControl->iter_children(*xSubLibIter))
                    m_xControl->copy_iterator(*xLibIter, *xCurIter);
            }
        }
    }
    else
    {
        bCurEntry = m_xControl->get_iter_first(*xCurIter);
    }

    if (!bCurEntry)
        return;

    m_xControl->set_cursor(*xCurIter);
}

IMPL_LINK_NOARG(SbTreeListBox, OpenCurrentHdl, weld::TreeView&, void)
{
    bool bValidIter = m_xControl->get_cursor(m_xIter.get());
    if (!bValidIter)
        return;
    EntryDescriptor aDesc = GetEntryDescriptor(m_xIter.get());
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
                return;
            }
            break;

        default:
            if (!m_xControl->get_row_expanded(*m_xIter))
                m_xControl->expand_row(*m_xIter);
            else
                m_xControl->collapse_row(*m_xIter);
            break;
    }
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
