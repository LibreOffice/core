/*************************************************************************
 *
 *  $RCSfile: sfxpicklist.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: hr $ $Date: 2003-09-29 14:52:08 $
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

#ifndef INCLUDED_SVTOOLS_HISTORYOPTIONS_HXX
#include <svtools/historyoptions.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_USEROPTIONS_HXX
#include <svtools/useroptions.hxx>
#endif

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_MENUCONFIGURATION_HXX_
#include <framework/menuconfiguration.hxx>
#endif

#ifndef _INETHIST_HXX
#include <svtools/inethist.hxx>
#endif

#ifndef _SFXSTRITEM_HXX
#include <svtools/stritem.hxx>
#endif

#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#include <unotools/localfilehelper.hxx>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#include <shell/systemshell.hxx>

// ----------------------------------------------------------------------------

#include "app.hxx"
#include "sfxpicklist.hxx"
#include "sfxuno.hxx"
#include "sfxtypes.hxx"
#include "request.hxx"
#include "sfxsids.hrc"
#include "sfx.hrc"
#include "event.hxx"
#include "objsh.hxx"
#include "docinf.hxx"
#include "bindings.hxx"
#include "referers.hxx"
#include "docfile.hxx"
#include "objshimp.hxx"
#include "docfilt.hxx"

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

void SfxPickList::CreatePicklistMenuTitle( Menu* pMenu, USHORT nItemId, const String& aURLString, sal_uInt32 nNo )
{
#ifdef MAC
    String aPickEntry;
#else
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
#endif

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

SfxPickList*    SfxPickList::GetOrCreate( const ULONG nMenuSize )
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

        pMenu->InsertItem( (USHORT)(START_ITEMID_PICKLIST + i), aEmptyString );
        CreatePicklistMenuTitle( pMenu, (USHORT)(START_ITEMID_PICKLIST + i), pEntry->aName, i );
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

        USHORT nPos=aFilter.Search('|');
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

void SfxPickList::ExecuteMenuEntry( USHORT nId )
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

void SfxPickList::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
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
                pDocSh->GetDocInfo().SetCreated( SvtUserOptions().GetFullName() );
                break;
            }

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
                INetURLObject aURL( pMed->GetOrigURL() );
                if ( aURL.GetProtocol() != INET_PROT_VND_SUN_STAR_HELP )
                {
                    ::rtl::OUString     aTitle = pDocSh->GetTitle(SFX_TITLE_PICKLIST);
                    ::rtl::OUString     aFilter;

                    const SfxFilter*    pFilter = pMed->GetOrigFilter();
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
            }

            case SFX_EVENT_CLOSEDOC:
            {
                SfxMedium *pMed = pDocSh->GetMedium();
                if( !pMed )
                    return;

                // unbenannt-Docs und embedded-Docs nicht in Pickliste
                if ( !pDocSh->HasName() ||
                     SFX_CREATE_MODE_STANDARD != pDocSh->GetCreateMode() )
                    return;

                if( pDocSh->Get_Impl()->bWaitingForPicklist &&
                    !pDocSh->Get_Impl()->bIsHelpObjSh )
                {
                    // only add r/w document into picklist
                    SfxMedium *pMed = pDocSh->GetMedium();
                    if( !pMed || pDocSh->IsReadOnly() || !pMed->IsUpdatePickList() )
                        return;

                    // add no document that forbids this (for example Message-Body)
                    SFX_ITEMSET_ARG( pMed->GetItemSet(), pPicklistItem, SfxBoolItem, SID_PICKLIST, sal_False );
                    if ( pPicklistItem && !pPicklistItem->GetValue() )
                        return;

                    // ignore hidden documents
                    if ( pDocSh->Get_Impl()->bHidden )
                        return;

                    ::rtl::OUString     aTitle = pDocSh->GetTitle(SFX_TITLE_PICKLIST);
                    ::rtl::OUString     aFilter;

                    INetURLObject       aURL( pMed->GetOrigURL() );
                    const SfxFilter*    pFilter = pMed->GetOrigFilter();

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
                        SystemShell::AddToRecentDocumentList( aURL.GetURLNoPass( INetURLObject::NO_DECODE ), (pFilter) ? pFilter->GetMimeType() : String() );
                }
                break;
            }
        }
    }
}
