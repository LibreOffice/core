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

#include <config_features.h>

#include <comphelper/lok.hxx>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <unotools/historyoptions.hxx>
#include <unotools/useroptions.hxx>
#include <tools/urlobj.hxx>
#include <framework/menuconfiguration.hxx>
#include <sax/tools/converter.hxx>
#include <svl/inethist.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/pngwrite.hxx>
#include <osl/file.hxx>
#include <unotools/localfilehelper.hxx>
#include <cppuhelper/implbase.hxx>



#include <sfx2/app.hxx>
#include "sfxpicklist.hxx"
#include <sfx2/sfxuno.hxx>
#include "sfxtypes.hxx"
#include <sfx2/request.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/sfx.hrc>
#include <sfx2/event.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/docfile.hxx>
#include "objshimp.hxx"
#include <stringhint.hxx>
#include <sfx2/docfilt.hxx>

#include <rtl/instance.hxx>

#include <algorithm>



using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;



class StringLength : public ::cppu::WeakImplHelper< XStringWidth >
{
    public:
        StringLength() {}
        virtual ~StringLength() {}

        // XStringWidth
        sal_Int32 SAL_CALL queryStringWidth( const OUString& aString )
            throw (css::uno::RuntimeException, std::exception) override
        {
            return aString.getLength();
        }
};

void SfxPickList::CreatePicklistMenuTitle( Menu* pMenu, sal_uInt16 nItemId, const OUString& aURLString, sal_uInt32 nNo )
{
    OUStringBuffer aPickEntry;

    if ( nNo < 9 )
    {
        aPickEntry.append('~');
        aPickEntry.append(OUString::number(nNo + 1));
    }
    else if ( nNo == 9 )
        aPickEntry.append("1~0");
    else
        aPickEntry.append(OUString::number(nNo + 1));
    aPickEntry.append(": ");

    INetURLObject   aURL( aURLString );
    OUString   aTipHelpText;
    OUString   aAccessibleName = aPickEntry.toString();

    if ( aURL.GetProtocol() == INetProtocol::File )
    {
        // Do handle file URL differently => convert it to a system
        // path and abbreviate it with a special function:
        OUString aFileSystemPath( aURL.getFSysPath( INetURLObject::FSYS_DETECT ) );

        OUString aSystemPath( aFileSystemPath );
        OUString aCompactedSystemPath;

        aTipHelpText = aSystemPath;
        aAccessibleName += aSystemPath;
        oslFileError nError = osl_abbreviateSystemPath( aSystemPath.pData, &aCompactedSystemPath.pData, 46, nullptr );
        if ( !nError )
            aPickEntry.append( aCompactedSystemPath );
        else
            aPickEntry.append( aFileSystemPath );

        if ( aPickEntry.getLength() > 50 )
        {
            aPickEntry.setLength( 47 );
            aPickEntry.append("...");
        }
    }
    else
    {
        // Use INetURLObject to abbreviate all other URLs
        OUString aShortURL;
        aShortURL = aURL.getAbbreviated( m_xStringLength, 46, INetURLObject::DECODE_UNAMBIGUOUS );
        aPickEntry.append(aShortURL);
        aTipHelpText = aURLString;
        aAccessibleName += aURLString;
    }

    // Set menu item text, tip help and accessible name
    pMenu->SetItemText( nItemId, aPickEntry.toString() );
    pMenu->SetTipHelpText( nItemId, aTipHelpText );
    pMenu->SetAccessibleName( nItemId, aAccessibleName );
}

namespace
{
    class thePickListMutex
        : public rtl::Static<osl::Mutex, thePickListMutex> {};
}

void SfxPickList::RemovePickListEntries()
{
    ::osl::MutexGuard aGuard( thePickListMutex::get() );
    for ( size_t i = 0; i < m_aPicklistVector.size(); i++ )
        delete m_aPicklistVector[i];
    m_aPicklistVector.clear();
}

SfxPickList::PickListEntry* SfxPickList::GetPickListEntry( sal_uInt32 nIndex )
{
    OSL_ASSERT( m_aPicklistVector.size() > nIndex );

    if ( nIndex < m_aPicklistVector.size() )
        return m_aPicklistVector[ nIndex ];
    else
        return nullptr;
}

void SfxPickList::AddDocumentToPickList( SfxObjectShell* pDocSh )
{
    if (pDocSh->IsAvoidRecentDocs() || comphelper::LibreOfficeKit::isActive())
        return;

    SfxMedium *pMed = pDocSh->GetMedium();
    if( !pMed )
        return;

    // Unnamed Documents and embedded-Documents not in Picklist
    if ( !pDocSh->HasName() ||
            SfxObjectCreateMode::STANDARD != pDocSh->GetCreateMode() )
        return;

    // Help not in History
    INetURLObject aURL( pDocSh->IsDocShared() ? pDocSh->GetSharedFileURL() : OUString( pMed->GetOrigURL() ) );
    if ( aURL.GetProtocol() == INetProtocol::VndSunStarHelp )
        return;

    if ( !pMed->IsUpdatePickList() )
        return;

    // add no document that forbids this (for example Message-Body)
    const SfxBoolItem* pPicklistItem = SfxItemSet::GetItem<SfxBoolItem>(pMed->GetItemSet(), SID_PICKLIST, false);
    if ( pPicklistItem && !pPicklistItem->GetValue() )
        return;

    // ignore hidden documents
    if ( !SfxViewFrame::GetFirst( pDocSh ) )
        return;

    OUString  aTitle = pDocSh->GetTitle(SFX_TITLE_PICKLIST);
    OUString  aFilter;
    const SfxFilter* pFilter = pMed->GetOrigFilter();
    if ( pFilter )
        aFilter = pFilter->GetFilterName();

    boost::optional<OUString> aThumbnail;

    // generate the thumbnail
    if (!pDocSh->IsModified() && !Application::IsHeadlessModeEnabled())
    {
        // not modified => the document matches what is in the shell
        const SfxUnoAnyItem* pEncryptionDataItem = SfxItemSet::GetItem<SfxUnoAnyItem>(pMed->GetItemSet(), SID_ENCRYPTIONDATA, false);
        if ( pEncryptionDataItem )
        {
            // encrypted document, will show a generic document icon instead
            aThumbnail = OUString();
        }
        else
        {
            std::shared_ptr<GDIMetaFile> xMetaFile = pDocSh->GetPreviewMetaFile();
            BitmapEx aResultBitmap;
            if (xMetaFile->CreateThumbnail(aResultBitmap))
            {
                SvMemoryStream aStream(65535, 65535);
                vcl::PNGWriter aWriter(aResultBitmap);
                if (aWriter.Write(aStream))
                {
                    Sequence<sal_Int8> aSequence(static_cast<const sal_Int8*>(aStream.GetData()), aStream.Tell());
                    OUStringBuffer aBuffer;
                    ::sax::Converter::encodeBase64(aBuffer, aSequence);
                    aThumbnail = aBuffer.makeStringAndClear();
                }
            }
        }
    }

    // add to svtool history options
    SvtHistoryOptions().AppendItem( ePICKLIST,
            aURL.GetURLNoPass( INetURLObject::NO_DECODE ),
            aFilter,
            aTitle,
            OUString(),
            aThumbnail);

    if ( aURL.GetProtocol() == INetProtocol::File )
        Application::AddToRecentDocumentList( aURL.GetURLNoPass( INetURLObject::NO_DECODE ),
                                                                 (pFilter) ? pFilter->GetMimeType() : OUString(),
                                                                 (pFilter) ? pFilter->GetServiceName() : OUString() );
}

SfxPickList& SfxPickList::Get()
{
    static SfxPickList aUniqueInstance(SvtHistoryOptions().GetSize(ePICKLIST));
    return aUniqueInstance;
}

SfxPickList::SfxPickList( sal_uInt32 nAllowedMenuSize ) :
    m_nAllowedMenuSize( nAllowedMenuSize )
{
    m_xStringLength = new StringLength;
    m_nAllowedMenuSize = ::std::min( m_nAllowedMenuSize, (sal_uInt32)PICKLIST_MAXSIZE );
    StartListening( *SfxGetpApp() );
}

SfxPickList::~SfxPickList()
{
    RemovePickListEntries();
}

void SfxPickList::CreatePickListEntries()
{
    RemovePickListEntries();

    // Reading the pick list
    Sequence< Sequence< PropertyValue > > seqPicklist = SvtHistoryOptions().GetList( ePICKLIST );

    sal_uInt32 nCount   = seqPicklist.getLength();
    sal_uInt32 nEntries = ::std::min( m_nAllowedMenuSize, nCount );

    for( sal_uInt32 nItem=0; nItem < nEntries; ++nItem )
    {
        Sequence< PropertyValue > seqPropertySet = seqPicklist[ nItem ];

        INetURLObject   aURL;
        OUString sURL;
        OUString sFilter;
        OUString sTitle;

        sal_uInt32 nPropertyCount = seqPropertySet.getLength();
        for( sal_uInt32 nProperty=0; nProperty<nPropertyCount; ++nProperty )
        {
            if( seqPropertySet[nProperty].Name == HISTORY_PROPERTYNAME_URL )
            {
                seqPropertySet[nProperty].Value >>= sURL;
            }
            else if( seqPropertySet[nProperty].Name == HISTORY_PROPERTYNAME_FILTER )
            {
                seqPropertySet[nProperty].Value >>= sFilter;
            }
            else if( seqPropertySet[nProperty].Name == HISTORY_PROPERTYNAME_TITLE )
            {
                seqPropertySet[nProperty].Value >>= sTitle;
            }
        }

        aURL.SetSmartURL( sURL );
        aURL.SetPass( OUString() );

        PickListEntry *pPick = new PickListEntry( aURL.GetMainURL( INetURLObject::NO_DECODE ), sFilter, sTitle );
        m_aPicklistVector.push_back( pPick );
    }
}

void SfxPickList::CreateMenuEntries( Menu* pMenu )
{
    ::osl::MutexGuard aGuard( thePickListMutex::get() );

    static bool bPickListMenuInitializing = false;

    if ( bPickListMenuInitializing ) // method is not reentrant!
        return;

    bPickListMenuInitializing = true;
    CreatePickListEntries();

    for ( sal_uInt16 nId = START_ITEMID_PICKLIST; nId <= END_ITEMID_PICKLIST; ++nId )
        pMenu->RemoveItem( pMenu->GetItemPos( nId ) );

    if ( pMenu->GetItemType( pMenu->GetItemCount()-1 ) == MenuItemType::SEPARATOR )
        pMenu->RemoveItem( pMenu->GetItemCount()-1 );

    if ( m_aPicklistVector.size() > 0 &&
         pMenu->GetItemType( pMenu->GetItemCount()-1 )
            != MenuItemType::SEPARATOR && m_nAllowedMenuSize )
        pMenu->InsertSeparator();

    OUString aEmptyString;
    for ( size_t i = 0; i < m_aPicklistVector.size(); i++ )
    {
        PickListEntry* pEntry = GetPickListEntry( i );

        pMenu->InsertItem( (sal_uInt16)(START_ITEMID_PICKLIST + i), aEmptyString );
        CreatePicklistMenuTitle( pMenu, (sal_uInt16)(START_ITEMID_PICKLIST + i), pEntry->aName, i );
    }

    bPickListMenuInitializing = false;
}

void SfxPickList::ExecuteEntry( sal_uInt32 nIndex )
{
    ::osl::ClearableMutexGuard aGuard( thePickListMutex::get() );

    PickListEntry *pPick = SfxPickList::Get().GetPickListEntry( nIndex );

    if ( pPick )
    {
        SfxRequest aReq( SID_OPENDOC, SfxCallMode::ASYNCHRON, SfxGetpApp()->GetPool() );
        aReq.AppendItem( SfxStringItem( SID_FILE_NAME, pPick->aName ));
        aReq.AppendItem( SfxStringItem( SID_REFERER, "private:user" ) );
        aReq.AppendItem( SfxStringItem( SID_TARGETNAME, "_default" ) );
        OUString aFilter( pPick->aFilter );
        aGuard.clear();

        sal_Int32 nPos = aFilter.indexOf('|');
        if( nPos != -1 )
        {
            OUString aOptions(aFilter.copy(nPos+1));
            aFilter = aFilter.copy( 0, nPos );
            aReq.AppendItem( SfxStringItem(SID_FILE_FILTEROPTIONS, aOptions));
        }

        aReq.AppendItem(SfxStringItem( SID_FILTER_NAME, aFilter ));
        aReq.AppendItem( SfxBoolItem( SID_TEMPLATE, false ) );
        SfxGetpApp()->ExecuteSlot( aReq );
    }
}

void SfxPickList::ExecuteMenuEntry( sal_uInt16 nId )
{
    ExecuteEntry( (sal_uInt32)( nId - START_ITEMID_PICKLIST ) );
}

void SfxPickList::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    const SfxStringHint* pStringHint = dynamic_cast<const SfxStringHint*>(&rHint);
    if ( pStringHint )
    {
        if ( pStringHint->GetId() == SID_OPENURL )
            INetURLHistory::GetOrCreate()->PutUrl( INetURLObject( pStringHint->GetObject() ));
    }

    const SfxEventHint* pEventHint = dynamic_cast<const SfxEventHint*>(&rHint);
    if ( pEventHint )
    {
        // only ObjectShell-related events with media interest
        SfxObjectShell* pDocSh = pEventHint->GetObjShell();
        if( !pDocSh )
            return;

        switch ( pEventHint->GetEventId() )
        {
            case SFX_EVENT_CREATEDOC:
            {
                bool bAllowModif = pDocSh->IsEnableSetModified();
                if ( bAllowModif )
                    pDocSh->EnableSetModified( false );

                using namespace ::com::sun::star;
                uno::Reference<document::XDocumentProperties> xDocProps(
                    pDocSh->getDocProperties());
                if (xDocProps.is()) {
                    xDocProps->setAuthor( SvtUserOptions().GetFullName() );
                    ::DateTime now( ::DateTime::SYSTEM );
                    xDocProps->setCreationDate( now.GetUNODateTime() );
                }

                if ( bAllowModif )
                    pDocSh->EnableSetModified( bAllowModif );
            }
            break;

            case SFX_EVENT_OPENDOC:
            case SFX_EVENT_SAVEDOCDONE:
            case SFX_EVENT_SAVEASDOCDONE:
            case SFX_EVENT_SAVETODOCDONE:
            case SFX_EVENT_CLOSEDOC:
            {
                AddDocumentToPickList(pDocSh);
            }
            break;

            case SFX_EVENT_SAVEASDOC:
            {
                SfxMedium *pMedium = pDocSh->GetMedium();
                if (!pMedium)
                    return;

                // We're starting a "Save As". Add the current document (if it's
                // not a "new" document) to the "Recent Documents" list before we
                // switch to the new path.
                // If the current document is new, path will be empty.
                OUString path = pMedium->GetOrigURL();
                if (!path.isEmpty())
                {
                    AddDocumentToPickList(pDocSh);
                }
            }
            break;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
