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
#include "precompiled_sfx2.hxx"

#include <com/sun/star/document/XDocumentProperties.hpp>
#include <unotools/historyoptions.hxx>
#include <unotools/useroptions.hxx>
#include <tools/urlobj.hxx>
#include <framework/menuconfiguration.hxx>
#include <svl/inethist.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <osl/file.hxx>
#include <unotools/localfilehelper.hxx>
#include <cppuhelper/implbase1.hxx>

// ----------------------------------------------------------------------------

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
#include "referers.hxx"
#include <sfx2/docfile.hxx>
#include "objshimp.hxx"
#include <sfx2/docfilt.hxx>

#include <algorithm>

// ----------------------------------------------------------------------------

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;

// ----------------------------------------------------------------------------

osl::Mutex*     SfxPickList::pMutex = 0;
SfxPickList*    SfxPickList::pUniqueInstance = 0;

// ----------------------------------------------------------------------------

class StringLength : public ::cppu::WeakImplHelper1< XStringWidth >
{
    public:
        StringLength() {}
        virtual ~StringLength() {}

        // XStringWidth
        sal_Int32 SAL_CALL queryStringWidth( const ::rtl::OUString& aString )
            throw (::com::sun::star::uno::RuntimeException)
        {
            return aString.getLength();
        }
};

// ----------------------------------------------------------------------------

osl::Mutex* SfxPickList::GetOrCreateMutex()
{
    if ( !pMutex )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if ( !pMutex )
            pMutex = new osl::Mutex;
    }

    return pMutex;
}

void SfxPickList::CreatePicklistMenuTitle( Menu* pMenu, sal_uInt16 nItemId, const String& aURLString, sal_uInt32 nNo )
{
    String aPickEntry;

    if ( nNo < 9 )
    {
        aPickEntry += '~';
        aPickEntry += String::CreateFromInt32( nNo + 1 );
    }
    else if ( nNo == 9 )
        aPickEntry += DEFINE_CONST_UNICODE("1~0");
    else
        aPickEntry += String::CreateFromInt32( nNo + 1 );
    aPickEntry += DEFINE_CONST_UNICODE(": ");

    INetURLObject   aURL( aURLString );
    rtl::OUString   aTipHelpText;
    rtl::OUString   aAccessibleName( aPickEntry );

    if ( aURL.GetProtocol() == INET_PROT_FILE )
    {
        // Do handle file URL differently => convert it to a system
        // path and abbreviate it with a special function:
        String aFileSystemPath( aURL.getFSysPath( INetURLObject::FSYS_DETECT ) );

//      ::utl::LocalFileHelper::ConvertURLToPhysicalName( aURLString, aPhysicalName );

        ::rtl::OUString aSystemPath( aFileSystemPath );
        ::rtl::OUString aCompactedSystemPath;

        aTipHelpText = aSystemPath;
        aAccessibleName += aSystemPath;
        oslFileError nError = osl_abbreviateSystemPath( aSystemPath.pData, &aCompactedSystemPath.pData, 46, NULL );
        if ( !nError )
            aPickEntry += String( aCompactedSystemPath );
        else
            aPickEntry += aFileSystemPath;

        if ( aPickEntry.Len() > 50 )
        {
            aPickEntry.Erase( 47 );
            aPickEntry += DEFINE_CONST_UNICODE("...");
        }
    }
    else
    {
        // Use INetURLObject to abbreviate all other URLs
        String  aShortURL;
        aShortURL = aURL.getAbbreviated( m_xStringLength, 46, INetURLObject::DECODE_UNAMBIGUOUS );
        aPickEntry += aShortURL;
        aTipHelpText = aURLString;
        aAccessibleName += aURLString;
    }

    // Set menu item text, tip help and accessible name
    pMenu->SetItemText( nItemId, aPickEntry );
    pMenu->SetTipHelpText( nItemId, aTipHelpText );
    pMenu->SetAccessibleName( nItemId, aAccessibleName );
}

void SfxPickList::RemovePickListEntries()
{
    ::osl::MutexGuard aGuard( GetOrCreateMutex() );
    for ( sal_uInt32 i = 0; i < m_aPicklistVector.size(); i++ )
        delete m_aPicklistVector[i];
    m_aPicklistVector.clear();
}

SfxPickList::PickListEntry* SfxPickList::GetPickListEntry( sal_uInt32 nIndex )
{
    OSL_ASSERT( m_aPicklistVector.size() > nIndex );

    if ( nIndex < m_aPicklistVector.size() )
        return m_aPicklistVector[ nIndex ];
    else
        return 0;
}

SfxPickList*    SfxPickList::GetOrCreate( const sal_uInt32 nMenuSize )
{
    if ( !pUniqueInstance )
    {
        ::osl::MutexGuard aGuard( GetOrCreateMutex() );
        if ( !pUniqueInstance )
            pUniqueInstance = new SfxPickList( nMenuSize );
    }

    return pUniqueInstance;
}

SfxPickList* SfxPickList::Get()
{
    ::osl::MutexGuard aGuard( GetOrCreateMutex() );
    return pUniqueInstance;
}

void SfxPickList::Delete()
{
    ::osl::MutexGuard aGuard( GetOrCreateMutex() );
    DELETEZ( pUniqueInstance );
}

SfxPickList::SfxPickList( sal_uInt32 nAllowedMenuSize ) :
    m_nAllowedMenuSize( nAllowedMenuSize )
{
    m_xStringLength = new StringLength;
    m_nAllowedMenuSize = ::std::min( m_nAllowedMenuSize, (sal_uInt32)PICKLIST_MAXSIZE );
    StartListening( *SFX_APP() );
}

SfxPickList::~SfxPickList()
{
    RemovePickListEntries();
}

void SfxPickList::CreatePickListEntries()
{
    RemovePickListEntries();

    // Einlesen der Pickliste
    Sequence< Sequence< PropertyValue > > seqPicklist = SvtHistoryOptions().GetList( ePICKLIST );

    sal_uInt32 nCount   = seqPicklist.getLength();
    sal_uInt32 nEntries = ::std::min( m_nAllowedMenuSize, nCount );

    for( sal_uInt32 nItem=0; nItem < nEntries; ++nItem )
    {
        Sequence< PropertyValue > seqPropertySet = seqPicklist[ nItem ];

        INetURLObject   aURL;
        ::rtl::OUString sURL;
        ::rtl::OUString sFilter;
        ::rtl::OUString sTitle;
        ::rtl::OUString sPassword;

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
            else if( seqPropertySet[nProperty].Name == HISTORY_PROPERTYNAME_PASSWORD )
            {
                seqPropertySet[nProperty].Value >>= sPassword;
            }
        }

        aURL.SetSmartURL( sURL );
        aURL.SetPass( SfxStringDecode( sPassword ) );

        PickListEntry *pPick = new PickListEntry( aURL.GetMainURL( INetURLObject::NO_DECODE ), sFilter, sTitle );
        m_aPicklistVector.push_back( pPick );
    }
}

void SfxPickList::CreateMenuEntries( Menu* pMenu )
{
    static sal_Bool bPickListMenuInitializing = sal_False;

    ::osl::MutexGuard aGuard( GetOrCreateMutex() );

    if ( bPickListMenuInitializing ) // method is not reentrant!
        return;

    bPickListMenuInitializing = sal_True;
    CreatePickListEntries();

    for ( sal_uInt16 nId = START_ITEMID_PICKLIST; nId <= END_ITEMID_PICKLIST; ++nId )
        pMenu->RemoveItem( pMenu->GetItemPos( nId ) );

    if ( pMenu->GetItemType( pMenu->GetItemCount()-1 ) == MENUITEM_SEPARATOR )
        pMenu->RemoveItem( pMenu->GetItemCount()-1 );

    if ( m_aPicklistVector.size() > 0 &&
         pMenu->GetItemType( pMenu->GetItemCount()-1 )
            != MENUITEM_SEPARATOR && m_nAllowedMenuSize )
        pMenu->InsertSeparator();

    rtl::OUString aEmptyString;
    for ( sal_uInt32 i = 0; i < m_aPicklistVector.size(); i++ )
    {
        PickListEntry* pEntry = GetPickListEntry( i );

        pMenu->InsertItem( (sal_uInt16)(START_ITEMID_PICKLIST + i), aEmptyString );
        CreatePicklistMenuTitle( pMenu, (sal_uInt16)(START_ITEMID_PICKLIST + i), pEntry->aName, i );
    }

    bPickListMenuInitializing = sal_False;
}

void SfxPickList::ExecuteEntry( sal_uInt32 nIndex )
{
    ::osl::ClearableMutexGuard aGuard( GetOrCreateMutex() );

    PickListEntry *pPick = SfxPickList::Get()->GetPickListEntry( nIndex );

    if ( pPick )
    {
        SfxRequest aReq( SID_OPENDOC, SFX_CALLMODE_ASYNCHRON, SFX_APP()->GetPool() );
        aReq.AppendItem( SfxStringItem( SID_FILE_NAME, pPick->aName ));
        aReq.AppendItem( SfxStringItem( SID_REFERER, DEFINE_CONST_UNICODE( SFX_REFERER_USER ) ) );
        aReq.AppendItem( SfxStringItem( SID_TARGETNAME, DEFINE_CONST_UNICODE("_default") ) );
        String aFilter( pPick->aFilter );
        aGuard.clear();

        sal_uInt16 nPos=aFilter.Search('|');
        if( nPos != STRING_NOTFOUND )
        {
            String aOptions(aFilter.Copy( nPos ).GetBuffer()+1);
            aFilter.Erase( nPos );
            aReq.AppendItem( SfxStringItem(SID_FILE_FILTEROPTIONS, aOptions));
        }

        aReq.AppendItem(SfxStringItem( SID_FILTER_NAME, aFilter ));
        aReq.AppendItem( SfxBoolItem( SID_TEMPLATE, sal_False ) );
        SFX_APP()->ExecuteSlot( aReq );
    }
}

void SfxPickList::ExecuteMenuEntry( sal_uInt16 nId )
{
    ExecuteEntry( (sal_uInt32)( nId - START_ITEMID_PICKLIST ) );
}

String SfxPickList::GetMenuEntryTitle( sal_uInt32 nIndex )
{
    PickListEntry *pPick = SfxPickList::Get()->GetPickListEntry( nIndex );

    if ( pPick )
        return pPick->aTitle;
    else
        return String();
}

void SfxPickList::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( rHint.IsA( TYPE( SfxStringHint )))
    {
        SfxStringHint* pStringHint = (SfxStringHint*) &rHint;

        if ( pStringHint->GetId() == SID_OPENURL )
            INetURLHistory::GetOrCreate()->PutUrl( INetURLObject( pStringHint->GetObject() ));
    }

    if ( rHint.IsA( TYPE( SfxEventHint )))
    {
        SfxEventHint* pEventHint = PTR_CAST(SfxEventHint,&rHint);
        // nur ObjectShell-bezogene Events mit Medium interessieren
        SfxObjectShell* pDocSh = pEventHint->GetObjShell();
        if( !pDocSh )
            return;

        switch ( pEventHint->GetEventId() )
        {
            case SFX_EVENT_CREATEDOC:
            {
                sal_Bool bAllowModif = pDocSh->IsEnableSetModified();
                if ( bAllowModif )
                    pDocSh->EnableSetModified( sal_False );

                using namespace ::com::sun::star;
                uno::Reference<document::XDocumentProperties> xDocProps(
                    pDocSh->getDocProperties());
                if (xDocProps.is()) {
                    xDocProps->setAuthor( SvtUserOptions().GetFullName() );
                    ::DateTime now;
                    xDocProps->setCreationDate( util::DateTime(
                        now.Get100Sec(), now.GetSec(), now.GetMin(),
                        now.GetHour(), now.GetDay(), now.GetMonth(),
                        now.GetYear() ) );
                }

                if ( bAllowModif )
                    pDocSh->EnableSetModified( bAllowModif );
            }
            break;

            case SFX_EVENT_OPENDOC:
            {
                SfxMedium *pMed = pDocSh->GetMedium();
                if( !pMed )
                    return;

                // unbenannt-Docs und embedded-Docs nicht in History
                if ( !pDocSh->HasName() ||
                     SFX_CREATE_MODE_STANDARD != pDocSh->GetCreateMode() )
                    return;

                // Hilfe nicht in History
                INetURLObject aURL( pDocSh->IsDocShared() ? pDocSh->GetSharedFileURL() : ::rtl::OUString( pMed->GetOrigURL() ) );
                if ( aURL.GetProtocol() == INET_PROT_VND_SUN_STAR_HELP )
                    return;

                ::rtl::OUString  aTitle = pDocSh->GetTitle(SFX_TITLE_PICKLIST);
                ::rtl::OUString  aFilter;
                const SfxFilter* pFilter = pMed->GetOrigFilter();
                if ( pFilter )
                    aFilter = pFilter->GetFilterName();

                // add to svtool history options
                SvtHistoryOptions().AppendItem( eHISTORY,
                        aURL.GetURLNoPass( INetURLObject::NO_DECODE ),
                        aFilter,
                        aTitle,
                        SfxStringEncode( aURL.GetPass() ) );
            }
            break;

            case SFX_EVENT_CLOSEDOC:
            {
                SfxMedium *pMed = pDocSh->GetMedium();
                if( !pMed )
                    return;

                // unbenannt-Docs und embedded-Docs nicht in Pickliste
                if ( !pDocSh->HasName() ||
                     SFX_CREATE_MODE_STANDARD != pDocSh->GetCreateMode() )
                    return;

                // Hilfe nicht in History
                INetURLObject aURL( pDocSh->IsDocShared() ? pDocSh->GetSharedFileURL() : ::rtl::OUString( pMed->GetOrigURL() ) );
                if ( aURL.GetProtocol() == INET_PROT_VND_SUN_STAR_HELP )
                    return;

                // only add r/w document into picklist
                if ( pDocSh->IsReadOnly() || !pMed->IsUpdatePickList() )
                    return;

                // add no document that forbids this (for example Message-Body)
                SFX_ITEMSET_ARG( pMed->GetItemSet(), pPicklistItem, SfxBoolItem, SID_PICKLIST, sal_False );
                if (
                    (pPicklistItem && !pPicklistItem->GetValue()) ||
                    (!(pDocSh->Get_Impl()->bWaitingForPicklist) )
                   )
                    return;

                // ignore hidden documents
                if ( !SfxViewFrame::GetFirst( pDocSh, sal_True ) )
                    return;

                ::rtl::OUString  aTitle = pDocSh->GetTitle(SFX_TITLE_PICKLIST);
                ::rtl::OUString  aFilter;
                const SfxFilter* pFilter = pMed->GetOrigFilter();
                if ( pFilter )
                    aFilter = pFilter->GetFilterName();

                // add to svtool history options
                SvtHistoryOptions().AppendItem( ePICKLIST,
                        aURL.GetURLNoPass( INetURLObject::NO_DECODE ),
                        aFilter,
                        aTitle,
                        SfxStringEncode( aURL.GetPass() ) );

                pDocSh->Get_Impl()->bWaitingForPicklist = sal_False;

                if ( aURL.GetProtocol() == INET_PROT_FILE )
                    Application::AddToRecentDocumentList( aURL.GetURLNoPass( INetURLObject::NO_DECODE ), (pFilter) ? pFilter->GetMimeType() : String() );
            }
            break;
        }
    }
}
