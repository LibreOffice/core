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

#include <basic/basmgr.hxx>
#include <basic/sbmod.hxx>
#include <bastype2.hxx>
#include <bitmaps.hlst>
#include <bastypes.hxx>
#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/script/XLibraryContainerPassword.hpp>
#include <deque>
#include <sfx2/docfac.hxx>
#include <vcl/treelistentry.hxx>
#include <osl/diagnose.h>
#include <tools/debug.hxx>

namespace basctl
{

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;


typedef std::deque< SvTreeListEntry* > EntryArray;


void TreeListBox::RequestingChildren( SvTreeListEntry* pEntry )
{
    EntryDescriptor aDesc = GetEntryDescriptor(pEntry);
    const ScriptDocument& aDocument = aDesc.GetDocument();
    OSL_ENSURE( aDocument.isAlive(), "basctl::TreeListBox::RequestingChildren: invalid document!" );
    if ( !aDocument.isAlive() )
        return;

    LibraryLocation eLocation = aDesc.GetLocation();
    EntryType eType = aDesc.GetType();

    if ( eType == OBJ_TYPE_DOCUMENT )
    {
        ImpCreateLibEntries( pEntry, aDocument, eLocation );
    }
    else if ( eType == OBJ_TYPE_LIBRARY )
    {
        const OUString& aOULibName( aDesc.GetLibName() );

        // check password
        bool bOK = true;
        Reference< script::XLibraryContainer > xModLibContainer( aDocument.getLibraryContainer( E_SCRIPTS ) );
        if ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) )
        {
            Reference< script::XLibraryContainerPassword > xPasswd( xModLibContainer, UNO_QUERY );
            if ( xPasswd.is() && xPasswd->isLibraryPasswordProtected( aOULibName ) && !xPasswd->isLibraryPasswordVerified( aOULibName ) )
            {
                OUString aPassword;
                bOK = QueryPassword( xModLibContainer, aOULibName, aPassword );
            }
        }

        if ( bOK )
        {
            // load module library
            bool bModLibLoaded = false;
            if ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) )
            {
                if ( !xModLibContainer->isLibraryLoaded( aOULibName ) )
                {
                    EnterWait();
                    xModLibContainer->loadLibrary( aOULibName );
                    LeaveWait();
                }
                bModLibLoaded = xModLibContainer->isLibraryLoaded( aOULibName );
            }

            // load dialog library
            bool bDlgLibLoaded = false;
            Reference< script::XLibraryContainer > xDlgLibContainer( aDocument.getLibraryContainer( E_DIALOGS ), UNO_QUERY );
            if ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aOULibName ) )
            {
                if ( !xDlgLibContainer->isLibraryLoaded( aOULibName ) )
                {
                    EnterWait();
                    xDlgLibContainer->loadLibrary( aOULibName );
                    LeaveWait();
                }
                bDlgLibLoaded = xDlgLibContainer->isLibraryLoaded( aOULibName );
            }

            if ( bModLibLoaded || bDlgLibLoaded )
            {
                // create the sub entries
                ImpCreateLibSubEntries( pEntry, aDocument, aOULibName );

                // exchange image
                const bool bDlgMode = (nMode & BrowseMode::Dialogs) && !(nMode & BrowseMode::Modules);
                Image aImage(StockImage::Yes, bDlgMode ? OUStringLiteral(RID_BMP_DLGLIB) : OUStringLiteral(RID_BMP_MODLIB));
                SetEntryBitmaps( pEntry, aImage );
            }
            else
            {
                OSL_FAIL( "basctl::TreeListBox::RequestingChildren: Error loading library!" );
            }
        }
    }
    else if ( eType == OBJ_TYPE_DOCUMENT_OBJECTS
            || eType == OBJ_TYPE_USERFORMS
            || eType == OBJ_TYPE_NORMAL_MODULES
            || eType == OBJ_TYPE_CLASS_MODULES )
    {
        const OUString& aLibName( aDesc.GetLibName() );
        ImpCreateLibSubSubEntriesInVBAMode( pEntry, aDocument, aLibName );
    }
    else {
        OSL_FAIL( "basctl::TreeListBox::RequestingChildren: Unknown Type!" );
    }
}

IMPL_LINK(SbTreeListBox, RequestingChildrenHdl, const weld::TreeIter&, rEntry, bool)
{
    EntryDescriptor aDesc = GetEntryDescriptor(&rEntry);
    const ScriptDocument& aDocument = aDesc.GetDocument();
    OSL_ENSURE( aDocument.isAlive(), "basctl::TreeListBox::RequestingChildren: invalid document!" );
    if (!aDocument.isAlive())
        return false;

    LibraryLocation eLocation = aDesc.GetLocation();
    EntryType eType = aDesc.GetType();

    if ( eType == OBJ_TYPE_DOCUMENT )
    {
        ImpCreateLibEntries( rEntry, aDocument, eLocation );
    }
    else if ( eType == OBJ_TYPE_LIBRARY )
    {
        const OUString& aOULibName( aDesc.GetLibName() );

        // check password
        bool bOK = true;
        Reference< script::XLibraryContainer > xModLibContainer( aDocument.getLibraryContainer( E_SCRIPTS ) );
        if ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) )
        {
            Reference< script::XLibraryContainerPassword > xPasswd( xModLibContainer, UNO_QUERY );
            if ( xPasswd.is() && xPasswd->isLibraryPasswordProtected( aOULibName ) && !xPasswd->isLibraryPasswordVerified( aOULibName ) )
            {
                OUString aPassword;
                bOK = QueryPassword( xModLibContainer, aOULibName, aPassword );
            }
        }

        if ( bOK )
        {
            // load module library
            bool bModLibLoaded = false;
            if ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) )
            {
                if ( !xModLibContainer->isLibraryLoaded( aOULibName ) )
                {
                    weld::WaitObject aWait(m_pTopLevel);
                    xModLibContainer->loadLibrary( aOULibName );
                }
                bModLibLoaded = xModLibContainer->isLibraryLoaded( aOULibName );
            }

            // load dialog library
            bool bDlgLibLoaded = false;
            Reference< script::XLibraryContainer > xDlgLibContainer( aDocument.getLibraryContainer( E_DIALOGS ), UNO_QUERY );
            if ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aOULibName ) )
            {
                if ( !xDlgLibContainer->isLibraryLoaded( aOULibName ) )
                {
                    weld::WaitObject aWait(m_pTopLevel);
                    xDlgLibContainer->loadLibrary( aOULibName );
                }
                bDlgLibLoaded = xDlgLibContainer->isLibraryLoaded( aOULibName );
            }

            if ( bModLibLoaded || bDlgLibLoaded )
            {
                // create the sub entries
                ImpCreateLibSubEntries( rEntry, aDocument, aOULibName );

                // exchange image
                const bool bDlgMode = (nMode & BrowseMode::Dialogs) && !(nMode & BrowseMode::Modules);
                OUString aImage(bDlgMode ? OUStringLiteral(RID_BMP_DLGLIB) : OUStringLiteral(RID_BMP_MODLIB));
                SetEntryBitmaps(rEntry, aImage);
            }
            else
            {
                OSL_FAIL( "basctl::TreeListBox::RequestingChildren: Error loading library!" );
            }
        }
    }
    else if ( eType == OBJ_TYPE_DOCUMENT_OBJECTS
            || eType == OBJ_TYPE_USERFORMS
            || eType == OBJ_TYPE_NORMAL_MODULES
            || eType == OBJ_TYPE_CLASS_MODULES )
    {
        const OUString& aLibName( aDesc.GetLibName() );
        ImpCreateLibSubSubEntriesInVBAMode( rEntry, aDocument, aLibName );
    }

    return true;
}

void TreeListBox::ExpandedHdl()
{
    SvTreeListEntry* pEntry = GetHdlEntry();
    assert(pEntry && "What was collapsed?");
    if ( !IsExpanded( pEntry ) && pEntry->HasChildrenOnDemand() )
    {
        SvTreeListEntry* pChild = FirstChild( pEntry );
        while ( pChild )
        {
            GetModel()->Remove( pChild );   // does also call the DTOR
            pChild = FirstChild( pEntry );
        }
    }
}

void TreeListBox::ScanAllEntries()
{
    ScanEntry( ScriptDocument::getApplicationScriptDocument(), LIBRARY_LOCATION_USER );
    ScanEntry( ScriptDocument::getApplicationScriptDocument(), LIBRARY_LOCATION_SHARE );

    ScriptDocuments aDocuments( ScriptDocument::getAllScriptDocuments( ScriptDocument::DocumentsSorted ) );
    for (auto const& doc : aDocuments)
    {
        if ( doc.isAlive() )
            ScanEntry(doc, LIBRARY_LOCATION_DOCUMENT);
    }
}

void SbTreeListBox::ScanAllEntries()
{
    ScanEntry( ScriptDocument::getApplicationScriptDocument(), LIBRARY_LOCATION_USER );
    ScanEntry( ScriptDocument::getApplicationScriptDocument(), LIBRARY_LOCATION_SHARE );

    ScriptDocuments aDocuments( ScriptDocument::getAllScriptDocuments( ScriptDocument::DocumentsSorted ) );
    for (auto const& doc : aDocuments)
    {
        if ( doc.isAlive() )
            ScanEntry(doc, LIBRARY_LOCATION_DOCUMENT);
    }
}

SbxVariable* SbTreeListBox::FindVariable(const weld::TreeIter* pEntry)
{
    if ( !pEntry )
        return nullptr;

    ScriptDocument aDocument( ScriptDocument::getApplicationScriptDocument() );
    std::unique_ptr<weld::TreeIter> xIter(m_xControl->make_iterator(pEntry));
    std::vector<std::pair<Entry*, OUString>> aEntries;
    bool bValidIter = true;
    do
    {
        sal_uInt16 nDepth = m_xControl->get_iter_depth(*xIter);
        Entry* pBE = reinterpret_cast<Entry*>(m_xControl->get_id(*xIter).toInt64());
        switch (nDepth)
        {
            case 4:
            case 3:
            case 2:
            case 1:
            {
                aEntries.emplace_back(pBE, m_xControl->get_text(*xIter));
            }
            break;
            case 0:
            {
                aDocument = static_cast<DocumentEntry*>(pBE)->GetDocument();
            }
            break;
        }
        bValidIter = m_xControl->iter_parent(*xIter);
    } while (bValidIter);

    SbxVariable* pVar = nullptr;
    if (!aEntries.empty())
    {
        std::reverse(aEntries.begin(), aEntries.end());
        bool bDocumentObjects = false;
        for (auto& pair : aEntries)
        {
            Entry* pBE = pair.first;
            assert(pBE && "No data found in entry!");
            OUString aName(pair.second);

            switch ( pBE->GetType() )
            {
            case OBJ_TYPE_LIBRARY:
                if (BasicManager* pBasMgr = aDocument.getBasicManager())
                    pVar = pBasMgr->GetLib( aName );
                break;
            case OBJ_TYPE_MODULE:
                DBG_ASSERT(dynamic_cast<StarBASIC*>(pVar), "FindVariable: invalid Basic");
                if(!pVar)
                {
                    break;
                }
                // extract the module name from the string like "Sheet1 (Example1)"
                if( bDocumentObjects )
                {
                    aName = aName.getToken( 0, ' ' );
                }
                pVar = static_cast<StarBASIC*>(pVar)->FindModule( aName );
                break;
            case OBJ_TYPE_METHOD:
                DBG_ASSERT(dynamic_cast<SbxObject*>(pVar), "FindVariable: invalid module/object");
                if(!pVar)
                {
                    break;
                }
                pVar = static_cast<SbxObject*>(pVar)->GetMethods()->Find(aName, SbxClassType::Method);
                break;
            case OBJ_TYPE_DIALOG:
                // sbx dialogs removed
                break;
            case OBJ_TYPE_DOCUMENT_OBJECTS:
                bDocumentObjects = true;
                [[fallthrough]];
            case OBJ_TYPE_USERFORMS:
            case OBJ_TYPE_NORMAL_MODULES:
            case OBJ_TYPE_CLASS_MODULES:
                // skip, to find the child entry.
                continue;
            default:
                OSL_FAIL( "FindVariable: unknown type" );
                pVar = nullptr;
                break;
            }
            if ( !pVar )
                break;
        }
    }
    return pVar;
}

EntryDescriptor TreeListBox::GetEntryDescriptor( SvTreeListEntry* pEntry )
{
    ScriptDocument aDocument( ScriptDocument::getApplicationScriptDocument() );
    LibraryLocation eLocation = LIBRARY_LOCATION_UNKNOWN;
    OUString aLibName;
    OUString aLibSubName;
    OUString aName;
    OUString aMethodName;
    EntryType eType = OBJ_TYPE_UNKNOWN;

    if ( !pEntry )
        return EntryDescriptor( aDocument, eLocation, aLibName, aLibSubName, aName, aMethodName, eType );

    EntryArray aEntries;

    while ( pEntry )
    {
        sal_uInt16 nDepth = GetModel()->GetDepth( pEntry );
        switch ( nDepth )
        {
            case 4:
            case 3:
            case 2:
            case 1:
            {
                aEntries.push_front( pEntry );
            }
            break;
            case 0:
            {
                if (DocumentEntry* pDocumentEntry = static_cast<DocumentEntry*>(pEntry->GetUserData()))
                {
                    aDocument = pDocumentEntry->GetDocument();
                    eLocation = pDocumentEntry->GetLocation();
                    eType = OBJ_TYPE_DOCUMENT;
                }
            }
            break;
        }
        pEntry = GetParent( pEntry );
    }

    for (SvTreeListEntry* pLE : aEntries)
    {
        assert(pLE && "Entry not found in array");
        Entry* pBE = static_cast<Entry*>(pLE->GetUserData());
        assert(pBE && "No data found in entry!");

        switch ( pBE->GetType() )
        {
            case OBJ_TYPE_LIBRARY:
            {
                aLibName = GetEntryText( pLE );
                eType = pBE->GetType();
            }
            break;
            case OBJ_TYPE_MODULE:
            {
                aName = GetEntryText( pLE );
                eType = pBE->GetType();
            }
            break;
            case OBJ_TYPE_METHOD:
            {
                aMethodName = GetEntryText( pLE );
                eType = pBE->GetType();
            }
            break;
            case OBJ_TYPE_DIALOG:
            {
                aName = GetEntryText( pLE );
                eType = pBE->GetType();
            }
            break;
            case OBJ_TYPE_DOCUMENT_OBJECTS:
            case OBJ_TYPE_USERFORMS:
            case OBJ_TYPE_NORMAL_MODULES:
            case OBJ_TYPE_CLASS_MODULES:
            {
                aLibSubName = GetEntryText( pLE );
                eType = pBE->GetType();
            }
            break;
            default:
            {
                OSL_FAIL( "GetEntryDescriptor: unknown type" );
                eType = OBJ_TYPE_UNKNOWN;
            }
            break;
        }

        if ( eType == OBJ_TYPE_UNKNOWN )
            break;
    }

    return EntryDescriptor( aDocument, eLocation, aLibName, aLibSubName, aName, aMethodName, eType );
}

EntryDescriptor SbTreeListBox::GetEntryDescriptor(const weld::TreeIter* pEntry)
{
    ScriptDocument aDocument( ScriptDocument::getApplicationScriptDocument() );
    LibraryLocation eLocation = LIBRARY_LOCATION_UNKNOWN;
    OUString aLibName;
    OUString aLibSubName;
    OUString aName;
    OUString aMethodName;
    EntryType eType = OBJ_TYPE_UNKNOWN;

    if ( !pEntry )
        return EntryDescriptor( aDocument, eLocation, aLibName, aLibSubName, aName, aMethodName, eType );

    std::vector<std::pair<Entry*, OUString>> aEntries;

    std::unique_ptr<weld::TreeIter> xIter(m_xControl->make_iterator(pEntry));
    bool bValidIter = true;
    do
    {
        sal_uInt16 nDepth = m_xControl->get_iter_depth(*xIter);
        Entry* pBE = reinterpret_cast<Entry*>(m_xControl->get_id(*xIter).toInt64());
        switch (nDepth)
        {
            case 4:
            case 3:
            case 2:
            case 1:
            {
                aEntries.emplace_back(pBE, m_xControl->get_text(*xIter));
            }
            break;
            case 0:
            {
                if (DocumentEntry* pDocumentEntry = static_cast<DocumentEntry*>(pBE))
                {
                    aDocument = pDocumentEntry->GetDocument();
                    eLocation = pDocumentEntry->GetLocation();
                    eType = OBJ_TYPE_DOCUMENT;
                }
            }
            break;
        }
        bValidIter = m_xControl->iter_parent(*xIter);
    } while (bValidIter);

    if ( !aEntries.empty() )
    {
        std::reverse(aEntries.begin(), aEntries.end());
        for (auto& pair : aEntries)
        {
            Entry* pBE = pair.first;
            assert(pBE && "No data found in entry!");

            switch ( pBE->GetType() )
            {
                case OBJ_TYPE_LIBRARY:
                {
                    aLibName = pair.second;
                    eType = pBE->GetType();
                }
                break;
                case OBJ_TYPE_MODULE:
                {
                    aName = pair.second;
                    eType = pBE->GetType();
                }
                break;
                case OBJ_TYPE_METHOD:
                {
                    aMethodName = pair.second;
                    eType = pBE->GetType();
                }
                break;
                case OBJ_TYPE_DIALOG:
                {
                    aName = pair.second;
                    eType = pBE->GetType();
                }
                break;
                case OBJ_TYPE_DOCUMENT_OBJECTS:
                case OBJ_TYPE_USERFORMS:
                case OBJ_TYPE_NORMAL_MODULES:
                case OBJ_TYPE_CLASS_MODULES:
                {
                    aLibSubName = pair.second;
                    eType = pBE->GetType();
                }
                break;
                default:
                {
                    OSL_FAIL( "GetEntryDescriptor: unknown type" );
                    eType = OBJ_TYPE_UNKNOWN;
                }
                break;
            }

            if ( eType == OBJ_TYPE_UNKNOWN )
                break;
        }
    }

    return EntryDescriptor( aDocument, eLocation, aLibName, aLibSubName, aName, aMethodName, eType );
}

ItemType TreeListBox::ConvertType (EntryType eType)
{
    switch (eType)
    {
        case OBJ_TYPE_DOCUMENT:  return TYPE_SHELL;
        case OBJ_TYPE_LIBRARY:   return TYPE_LIBRARY;
        case OBJ_TYPE_MODULE:    return TYPE_MODULE;
        case OBJ_TYPE_DIALOG:    return TYPE_DIALOG;
        case OBJ_TYPE_METHOD:    return TYPE_METHOD;
        default:
            return static_cast<ItemType>(OBJ_TYPE_UNKNOWN);
    }
}

ItemType SbTreeListBox::ConvertType (EntryType eType)
{
    switch (eType)
    {
        case OBJ_TYPE_DOCUMENT:  return TYPE_SHELL;
        case OBJ_TYPE_LIBRARY:   return TYPE_LIBRARY;
        case OBJ_TYPE_MODULE:    return TYPE_MODULE;
        case OBJ_TYPE_DIALOG:    return TYPE_DIALOG;
        case OBJ_TYPE_METHOD:    return TYPE_METHOD;
        default:
            return static_cast<ItemType>(OBJ_TYPE_UNKNOWN);
    }
}

bool TreeListBox::IsValidEntry( SvTreeListEntry* pEntry )
{
    bool bIsValid = false;

    EntryDescriptor aDesc( GetEntryDescriptor( pEntry ) );
    const ScriptDocument& aDocument( aDesc.GetDocument() );
    LibraryLocation eLocation( aDesc.GetLocation() );
    const OUString& aLibName( aDesc.GetLibName() );
    const OUString& aName( aDesc.GetName() );
    const OUString& aMethodName( aDesc.GetMethodName() );
    EntryType eType( aDesc.GetType() );

    switch ( eType )
    {
        case OBJ_TYPE_DOCUMENT:
        {
            bIsValid = aDocument.isAlive()
                && (aDocument.isApplication()
                    || GetRootEntryName(aDocument, eLocation) == GetEntryText(pEntry));
        }
        break;
        case OBJ_TYPE_LIBRARY:
        {
            bIsValid = aDocument.hasLibrary( E_SCRIPTS, aLibName ) || aDocument.hasLibrary( E_DIALOGS, aLibName );
        }
        break;
        case OBJ_TYPE_MODULE:
        {
            bIsValid = aDocument.hasModule( aLibName, aName );
        }
        break;
        case OBJ_TYPE_DIALOG:
        {
            bIsValid = aDocument.hasDialog( aLibName, aName );
        }
        break;
        case OBJ_TYPE_METHOD:
        {
            bIsValid = HasMethod( aDocument, aLibName, aName, aMethodName );
        }
        break;
        case OBJ_TYPE_DOCUMENT_OBJECTS:
        case OBJ_TYPE_USERFORMS:
        case OBJ_TYPE_NORMAL_MODULES:
        case OBJ_TYPE_CLASS_MODULES:
        {
            bIsValid = true;
        }
        break;
        default: ;
    }

    return bIsValid;
}

bool SbTreeListBox::IsValidEntry(weld::TreeIter& rEntry)
{
    bool bIsValid = false;

    EntryDescriptor aDesc(GetEntryDescriptor(&rEntry));
    const ScriptDocument& aDocument( aDesc.GetDocument() );
    LibraryLocation eLocation( aDesc.GetLocation() );
    const OUString& aLibName( aDesc.GetLibName() );
    const OUString& aName( aDesc.GetName() );
    const OUString& aMethodName( aDesc.GetMethodName() );
    EntryType eType( aDesc.GetType() );

    switch ( eType )
    {
        case OBJ_TYPE_DOCUMENT:
        {
            bIsValid = aDocument.isAlive()
                && (aDocument.isApplication()
                    || GetRootEntryName(aDocument, eLocation) == m_xControl->get_text(rEntry));
        }
        break;
        case OBJ_TYPE_LIBRARY:
        {
            bIsValid = aDocument.hasLibrary( E_SCRIPTS, aLibName ) || aDocument.hasLibrary( E_DIALOGS, aLibName );
        }
        break;
        case OBJ_TYPE_MODULE:
        {
            bIsValid = aDocument.hasModule( aLibName, aName );
        }
        break;
        case OBJ_TYPE_DIALOG:
        {
            bIsValid = aDocument.hasDialog( aLibName, aName );
        }
        break;
        case OBJ_TYPE_METHOD:
        {
            bIsValid = HasMethod( aDocument, aLibName, aName, aMethodName );
        }
        break;
        case OBJ_TYPE_DOCUMENT_OBJECTS:
        case OBJ_TYPE_USERFORMS:
        case OBJ_TYPE_NORMAL_MODULES:
        case OBJ_TYPE_CLASS_MODULES:
        {
            bIsValid = true;
        }
        break;
        default: ;
    }

    return bIsValid;
}

SbModule* SbTreeListBox::FindModule(const weld::TreeIter* pEntry)
{
    return dynamic_cast<SbModule*>(FindVariable(pEntry));
}

SvTreeListEntry* TreeListBox::FindRootEntry( const ScriptDocument& rDocument, LibraryLocation eLocation )
{
    OSL_ENSURE( rDocument.isValid(), "basctl::TreeListBox::FindRootEntry: invalid document!" );
    sal_uLong nRootPos = 0;
    SvTreeListEntry* pRootEntry = GetEntry( nRootPos );
    while ( pRootEntry )
    {
        DBG_ASSERT( static_cast<Entry*>(pRootEntry->GetUserData())->GetType() == OBJ_TYPE_DOCUMENT, "No shell entry?" );
        DocumentEntry* pBDEntry = static_cast<DocumentEntry*>(pRootEntry->GetUserData());
        if (pBDEntry && pBDEntry->GetDocument() == rDocument && pBDEntry->GetLocation() == eLocation)
            return pRootEntry;
        pRootEntry = GetEntry( ++nRootPos );
    }
    return nullptr;
}

bool SbTreeListBox::FindRootEntry( const ScriptDocument& rDocument, LibraryLocation eLocation, weld::TreeIter& rIter)
{
    OSL_ENSURE( rDocument.isValid(), "basctl::TreeListBox::FindRootEntry: invalid document!" );
    bool bValidIter = m_xControl->get_iter_first(rIter);
    while (bValidIter)
    {
        DocumentEntry* pBDEntry = reinterpret_cast<DocumentEntry*>(m_xControl->get_id(rIter).toInt64());
        if (pBDEntry && pBDEntry->GetDocument() == rDocument && pBDEntry->GetLocation() == eLocation)
            return true;
        bValidIter = m_xControl->iter_next_sibling(rIter);
    }
    return false;
}

OUString CreateMgrAndLibStr( const OUString& rMgrName, const OUString& rLibName )
{
    return "[" + rMgrName + "]." + rLibName;
}


} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
