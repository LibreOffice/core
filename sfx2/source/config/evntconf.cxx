/*************************************************************************
 *
 *  $RCSfile: evntconf.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-04 17:35:21 $
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

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SV_RESARY_HXX
#include <vcl/resary.hxx>
#endif
#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _BASMGR_HXX //autogen
#include <basic/basmgr.hxx>
#endif
#ifndef _SB_SBMOD_HXX //autogen
#include <basic/sbmod.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _SBXCLASS_HXX //autogen
#include <svtools/sbx.hxx>
#endif

#include <svtools/securityoptions.hxx>

#pragma hdrstop

#ifndef _RTL_USTRING_
#include <rtl/ustring.h>
#endif

#include <com/sun/star/uno/Any.hxx>
#include <framework/eventsconfiguration.hxx>
#include <comphelper/processfactory.hxx>

#include "evntconf.hxx"

#include "macrconf.hxx"
#include "docfile.hxx"
#include "app.hxx"
#include "objsh.hxx"
#include "dispatch.hxx"
#include "config.hrc"
#include "sfxresid.hxx"
#include "cfgmgr.hxx"
#include "macropg.hxx"
#include "docinf.hxx"
#include "eventsupplier.hxx"

#include "sfxsids.hrc"
#include "sfxlocal.hrc"

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef  _COM_SUN_STAR_CONTAINER_XNAMEREPLACE_HPP_
#include <com/sun/star/container/XNameReplace.hpp>
#endif
#ifndef  _COM_SUN_STAR_DOCUMENT_XEVENTSSUPPLIER_HPP_
#include <com/sun/star/document/XEventsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

// -----------------------------------------------------------------------

#define PROPERTYVALUE           ::com::sun::star::beans::PropertyValue
#define XNAMEREPLACE            ::com::sun::star::container::XNameReplace
#define XEVENTSSUPPLIER         ::com::sun::star::document::XEventsSupplier
#define ANY                     ::com::sun::star::uno::Any
#define REFERENCE               ::com::sun::star::uno::Reference
#define SEQUENCE                ::com::sun::star::uno::Sequence
#define UNO_QUERY               ::com::sun::star::uno::UNO_QUERY

#define OUSTRING                ::rtl::OUString

// -----------------------------------------------------------------------

static const USHORT nCompatVersion = 3;
static const USHORT nOldVersion = 4;
static const USHORT nVersion = 5;

TYPEINIT1(SfxEventHint, SfxHint);

// class SfxMacroQueryDlg_Impl -------------------------------------------

class SfxMacroQueryDlg_Impl : public QueryBox
{
public:
                            SfxMacroQueryDlg_Impl( const String& rMacro, BOOL bDefault );
};

// class SfxMacroQueryDlg_Impl -------------------------------------------

SfxMacroQueryDlg_Impl::SfxMacroQueryDlg_Impl( const String& rMacName, BOOL bDefault ) :
    QueryBox( NULL, SfxResId( DLG_MACROQUERY ) )
{
    SetButtonText( GetButtonId(0), String( SfxResId(BTN_OK) ) );
    SetButtonText( GetButtonId(1), String( SfxResId(BTN_CANCEL) ) );

    String aText = GetMessText();
    aText.SearchAndReplace( String::CreateFromAscii("$(MACRO)"), rMacName );

    if ( bDefault )
    {
        SetFocusButton(GetButtonId(0));
        aText.SearchAndReplace( String::CreateFromAscii("$(TEXT)"), String( SfxResId(FT_OK) ) );
    }
    else
    {
        SetFocusButton(GetButtonId(1));
        aText.SearchAndReplace( String::CreateFromAscii("$(TEXT)"), String( SfxResId(FT_CANCEL) ) );
    }

    SetMessText( aText );
    FreeResource();
}

// class SfxAsyncEvent_Impl ----------------------------------------------

class SfxAsyncEvent_Impl : public SfxListener
{
    String          aArgs;
    SfxObjectShell* pSh;
    const SvxMacro* pMacro;
    Timer *pTimer;

public:

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
    SfxAsyncEvent_Impl( SfxObjectShell *pDoc, const SvxMacro *pMacro, const String& rArgs );
    ~SfxAsyncEvent_Impl();
    DECL_LINK( TimerHdl, Timer*);
};

// -----------------------------------------------------------------------

void SfxAsyncEvent_Impl::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    SfxSimpleHint* pHint = PTR_CAST( SfxSimpleHint, &rHint );
    if( pHint && pHint->GetId() == SFX_HINT_DYING && pTimer->IsActive() )
    {
        pTimer->Stop();
        delete this;
    }
}

// -----------------------------------------------------------------------

SfxAsyncEvent_Impl::SfxAsyncEvent_Impl( SfxObjectShell *pDoc, const SvxMacro *pMac, const String& rArgs )
 : aArgs( rArgs )
 , pSh( pDoc )
 , pMacro( pMac )
{
    if( pSh ) StartListening( *pSh );
    pTimer = new Timer;
    pTimer->SetTimeoutHdl( LINK(this, SfxAsyncEvent_Impl, TimerHdl) );
    pTimer->SetTimeout( 0 );
    pTimer->Start();
}

// -----------------------------------------------------------------------

SfxAsyncEvent_Impl::~SfxAsyncEvent_Impl()
{
    delete pTimer;
}

// -----------------------------------------------------------------------

IMPL_LINK(SfxAsyncEvent_Impl, TimerHdl, Timer*, pTimer)
{
    pTimer->Stop();
    ScriptType eSType = pMacro->GetScriptType();
    BOOL bIsBasic = ( eSType == STARBASIC );
    if ( bIsBasic && StarBASIC::IsRunning() )
        // Neues eventgebundenes Macro erst ausf"uhren, wenn gerade kein anderes Macro mehr l"auft
        pTimer->Start();
    else
    {
        SFX_APP()->GetMacroConfig()->ExecuteMacro( pSh, pMacro, aArgs );
        delete this;
    }

    return 0L;
}

// class SfxEventList_Impl -----------------------------------------------

struct EventNames_Impl
{
    USHORT  mnId;
    String  maEventName;
    String  maUIName;

            EventNames_Impl( USHORT nId,
                             const String& rEventName,
                             const String& rUIName )
                : mnId( nId )
                , maEventName( rEventName )
                , maUIName( rUIName ) {}
};

DECLARE_LIST( SfxEventList_Impl, EventNames_Impl* );

SfxEventList_Impl   *gp_Id_SortList = NULL;
SfxEventList_Impl   *gp_Name_SortList = NULL;

//==========================================================================

SfxEventConfiguration::SfxEventConfiguration()
 : pDocEventConfig( NULL )
 , pAppEventConfig( NULL )
{
    bIgnoreConfigure = sal_False;

    // Array zum Ermitteln der Bindungen
    pEventArr = new SfxEventArr_Impl;

    // Einen default entry eingf"ugen
    const SfxEvent_Impl *pEvent = new SfxEvent_Impl(String(), 0);
    pEventArr->Insert(pEvent, 0);
}

SfxEventConfigItem_Impl* SfxEventConfiguration::GetAppEventConfig_Impl()
{
    if ( !pAppEventConfig )
    {
        pAppEventConfig = new SfxEventConfigItem_Impl( SFX_ITEMTYPE_APPEVENTCONFIG, this );
        pAppEventConfig->Initialize();
    }

    return pAppEventConfig;
}

//==========================================================================

SfxEventConfiguration::~SfxEventConfiguration()
{
    for (USHORT n=0; n<pEventArr->Count(); n++)
        delete (*pEventArr)[n];
    delete pEventArr;
    delete pAppEventConfig;

    if ( gp_Id_SortList )
    {
        EventNames_Impl* pData = gp_Id_SortList->First();
        while ( pData )
        {
            delete pData;
            pData = gp_Id_SortList->Next();
        }
        delete gp_Id_SortList;
        delete gp_Name_SortList;

        gp_Id_SortList = NULL;
        gp_Name_SortList = NULL;
    }
}

//==========================================================================

void SfxEventConfiguration::RegisterEvent(USHORT nId, const String& rName)
{
    USHORT nCount = pEventArr->Count();
    const SfxEvent_Impl *pEvent = new SfxEvent_Impl(rName, nId);
    pEventArr->Insert(pEvent, nCount);
}

//==========================================================================

String SfxEventConfiguration::GetEventName(USHORT nId) const
{
    DBG_ASSERT(pEventArr,"Keine Events angemeldet!");
    USHORT nCount = pEventArr->Count();
    for (USHORT n=1; n<nCount; n++)
    {
        if ((*pEventArr)[n]->nEventId == nId)
        {
            return (*pEventArr)[n]->aEventName;
        }
    }

    DBG_ERROR("Event nicht gefunden!");
    return (*pEventArr)[0]->aEventName;
}

//==========================================================================

USHORT SfxEventConfiguration::GetEventId(const String& rName) const
{
    DBG_ASSERT(pEventArr,"Keine Events angemeldet!");
    USHORT nCount = pEventArr->Count();
    for (USHORT n=1; n<nCount; n++)
    {
        if ((*pEventArr)[n]->aEventName == rName)
        {
            return (*pEventArr)[n]->nEventId;
        }
    }

    DBG_ERROR("Event nicht gefunden!");
    return SFX_NO_EVENT;
}

void SfxEventConfiguration::ConfigureEvent( USHORT nId, const SvxMacro& rMacro, SfxObjectShell *pDoc )
{
    if ( bIgnoreConfigure )
        return;

    SvxMacro *pMacro = NULL;
    if ( rMacro.GetMacName().Len() )
        pMacro = new SvxMacro( rMacro.GetMacName(), rMacro.GetLibName(), rMacro.GetScriptType() );
    if ( pDoc )
    {
        //pDocEventConfig = pDoc->GetEventConfig_Impl( TRUE );
        //pDocEventConfig->ConfigureEvent( nId, pMacro );
        PropagateEvent_Impl( pDoc, nId, pMacro );
    }
    else
    {
        // globale Bindung
        //GetAppEventConfig_Impl()->ConfigureEvent( nId, pMacro );
        PropagateEvent_Impl( NULL, nId, pMacro );
    }
}

//==========================================================================

void SfxEventConfiguration::ExecuteEvent(
    USHORT nId, SfxObjectShell *pDoc, FASTBOOL bSynchron, const String& rArgs )
{
    if ( pDoc && pDoc->IsPreview() )
        return;

    if ( pDoc && pDoc->GetEventConfig_Impl() && pDoc->GetEventConfig_Impl()->aMacroTable.Seek( nId ) )
        return;

    const SvxMacro* pMacro = GetAppEventConfig_Impl()->aMacroTable.Seek( nId );
    if ( pMacro )
    {
        if ( bSynchron )
            SfxMacroConfig::GetOrCreate()->ExecuteMacro( pDoc, pMacro, rArgs );
        else
            new SfxAsyncEvent_Impl( pDoc, pMacro, rArgs );
    }
}

//==========================================================================

const SvxMacro* SfxEventConfiguration::GetMacroForEventId
(
    USHORT          nId,
    SfxObjectShell* pDoc
)
{
    pDocEventConfig = pDoc ? pDoc->GetEventConfig_Impl() : NULL;
    const SvxMacro* pMacro=NULL;
    if ( pDocEventConfig )
        pMacro = pDocEventConfig->aMacroTable.Seek( nId );
    if ( !pMacro )
        pMacro = GetAppEventConfig_Impl()->aMacroTable.Seek( nId );
    return pMacro;
}

const SfxMacroInfo* SfxEventConfiguration::GetMacroInfo
(
    USHORT          nId,
    SfxObjectShell* pDoc
) const
{
    DBG_ASSERT(pEventArr,"Keine Events angemeldet!");

    SfxEventConfigItem_Impl *pDocEventConfig = pDoc ? pDoc->GetEventConfig_Impl() : NULL;
    const SvxMacro* pMacro=NULL;
    if ( pDocEventConfig )
        pMacro = pDocEventConfig->aMacroTable.Seek( nId );
    if ( !pMacro )
        pMacro = const_cast< SfxEventConfiguration* >(this)->GetAppEventConfig_Impl()->aMacroTable.Seek( nId );

    return SFX_APP()->GetMacroConfig()->GetMacroInfo_Impl( pMacro );
}

//==========================================================================

SfxEventConfigItem_Impl::SfxEventConfigItem_Impl( USHORT nConfigId,
    SfxEventConfiguration *pCfg,
    SfxObjectShell *pObjSh)
    : SfxConfigItem( nConfigId, pObjSh ? NULL : SFX_APP()->GetConfigManager_Impl() )
    , aMacroTable( 2, 2 )
    , pEvConfig( pCfg )
    , pObjShell( pObjSh )
    , bInitialized( FALSE )
{
    bInitialized = TRUE;
}

/*
void SfxEventConfigItem_Impl::Init( SfxConfigManager *pMgr )
{
    if ( GetConfigManager() == pMgr )
        Initialize();
    else
        ReInitialize( pMgr );
}
*/

//==========================================================================

int SfxEventConfigItem_Impl::Load(SvStream& rStream)
{
    USHORT nFileVersion;
    rStream >> nFileVersion;
    if ( nFileVersion < nCompatVersion || nFileVersion > nVersion )
        return SfxConfigItem::WARNING_VERSION;

    SvxMacroTableDtor aLocalMacroTable;
    if ( nFileVersion <= nOldVersion )
    {
        if ( nFileVersion > nCompatVersion )
        {
            USHORT nWarn;
            rStream >> nWarn;
            bWarning = ( nWarn & 0x01 ) != 0;
            bAlwaysWarning = ( nWarn & 0x02 ) != 0;
        }
        else
            bWarning = bAlwaysWarning = FALSE;

        USHORT nCount, nEventCount = pEvConfig->GetEventCount();
        rStream >> nCount;

        USHORT i;
        for (i=0; i<nCount; i++)
        {
            USHORT nId;
            SfxMacroInfo aInfo( pObjShell );
            rStream >> nId >> aInfo;

            for (USHORT n=0; n<nEventCount; n++)
            {
                USHORT nEventId = (*pEvConfig->pEventArr)[n+1]->nEventId;
                if ( nEventId == nId )
                {
                    SvxMacro *pMacro = new SvxMacro( aInfo.GetQualifiedName(), aInfo.GetBasicName(), STARBASIC );
                    aLocalMacroTable.Insert( nEventId, pMacro );
                    break;
                }
            }
        }
    }
    else
    {
        USHORT nWarn;
        rStream >> nWarn;
        bWarning = ( nWarn & 0x01 ) != 0;
        bAlwaysWarning = ( nWarn & 0x02 ) != 0;
        aLocalMacroTable.Read( rStream );
    }

    if ( pObjShell && pEvConfig )
        pEvConfig->PropagateEvents_Impl( pObjShell, aLocalMacroTable );

    return SfxConfigItem::ERR_OK;
}

BOOL SfxEventConfigItem_Impl::LoadXML( SvStream& rInStream )
{
    ::framework::EventsConfig aCfg;
    if ( ::framework::EventsConfiguration::LoadEventsConfig( rInStream, aCfg ) )
    {
        long nCount = aCfg.aEventNames.getLength();
        for ( long i=0; i<nCount; i++ )
        {
            SvxMacro* pMacro = SfxEvents_Impl::ConvertToMacro( aCfg.aEventsProperties[i], NULL, TRUE );
            USHORT nID = (USHORT) SfxEventConfiguration::GetEventId_Impl( aCfg.aEventNames[i] );
            if ( nID && pMacro )
                pEvConfig->PropagateEvent_Impl( pObjShell, nID, pMacro );
            else
                DBG_ERROR("Suspicious event binding!");
        }

        return TRUE;
    }

    return FALSE;
}

//==========================================================================

BOOL SfxEventConfigItem_Impl::Store(SvStream& rStream)
{
    USHORT nWarn=0;
    if ( bWarning )
        nWarn |= 0x01;
    if ( bAlwaysWarning )
        nWarn |= 0x02;
    rStream << nVersion << nWarn;
    aMacroTable.Write( rStream );
    return TRUE;
}

BOOL SfxEventConfigItem_Impl::StoreXML( SvStream& rOutStream )
{
    // get the event names
    ResStringArray aEventNames( SfxResId( EVENT_NAMES_ARY ) );
    long nNamesCount = aEventNames.Count();

    // create two sequences for names and properties
    SEQUENCE < OUSTRING > aSequence( nNamesCount );
    SEQUENCE < ANY > aData( nNamesCount );

    // fill in the names
    OUSTRING* pNames = aSequence.getArray();
    long i;
    for ( i=0; i<nNamesCount; i++ )
        pNames[i] = aEventNames.GetString( (USHORT) i);

    REFERENCE< XEVENTSSUPPLIER > xSupplier;
    if ( pObjShell )
    {
        xSupplier = REFERENCE< XEVENTSSUPPLIER >( pObjShell->GetModel(), UNO_QUERY );
    }
    else
    {
        xSupplier = REFERENCE< XEVENTSSUPPLIER >
                ( ::comphelper::getProcessServiceFactory()->createInstance(
                        rtl::OUString::createFromAscii("com.sun.star.frame.GlobalEventBroadcaster" )), UNO_QUERY );
    }

    // fill in the bindings
    REFERENCE< XNAMEREPLACE > xEvents = xSupplier->getEvents();
    SvxMacroTableDtor& rTable = aMacroTable;
    long nCount = (long) rTable.Count();
    for ( i=0; i<nCount; i++ )
    {
        USHORT nID = (USHORT) rTable.GetObjectKey( i );
        OUSTRING aEventName = pEvConfig->GetEventName_Impl( nID );
        if ( aEventName.getLength() )
        {
            // search name
            for ( long n=0; n<(long)nNamesCount; n++ )
            {
                if ( aSequence[n] == aEventName )
                {
                    // create properties from macro
                    //SvxMacro* pMacro = rTable.GetObject( i );
                    //aData[n] = pEvConfig->CreateEventData_Impl( pMacro );
                    aData[n] = xEvents->getByName( aEventName );
                    break;
                }
            }

            DBG_ASSERT( n<nNamesCount, "Unknown event!" );
        }
    }

    ::framework::EventsConfig aCfg;
    aCfg.aEventNames = aSequence;
    aCfg.aEventsProperties = aData;

    return ::framework::EventsConfiguration::StoreEventsConfig( rOutStream, aCfg );
}

//==========================================================================

String SfxEventConfigItem_Impl::GetStreamName() const
{
    return SfxConfigItem::GetStreamName( GetType() );
}

//==========================================================================

void SfxEventConfigItem_Impl::UseDefault()
{
    bWarning = TRUE;
    bAlwaysWarning = FALSE;

    aMacroTable.DelDtor();
    SetDefault( TRUE );
    pEvConfig->PropagateEvents_Impl( pObjShell, aMacroTable );
}

int SfxEventConfigItem_Impl::Load( SotStorage& rStorage )
{
    SotStorageStreamRef xStream = rStorage.OpenSotStream( SfxEventConfigItem_Impl::GetStreamName(), STREAM_STD_READ );
    if ( xStream->GetError() )
        return SfxConfigItem::ERR_READ;
    else
    {
        if ( bInitialized )
        {
            aMacroTable.DelDtor();
            pEvConfig->PropagateEvents_Impl( pObjShell, aMacroTable );
        }

        if ( LoadXML( *xStream ) )
            return SfxConfigItem::ERR_OK;
        else
            return SfxConfigItem::ERR_READ;
    }
}

BOOL SfxEventConfigItem_Impl::Store( SotStorage& rStorage )
{
    if ( pObjShell )
        // DocEventConfig is stored with the document
        return TRUE;

    SotStorageStreamRef xStream = rStorage.OpenSotStream( SfxEventConfigItem_Impl::GetStreamName(), STREAM_STD_READWRITE|STREAM_TRUNC );
    if ( xStream->GetError() )
        return FALSE;
    else
        return StoreXML( *xStream );
}


//==========================================================================

SfxEventConfigItem_Impl::~SfxEventConfigItem_Impl()
{
}

//==========================================================================

void SfxEventConfigItem_Impl::ConfigureEvent( USHORT nId, SvxMacro *pMacro )
{
    if ( aMacroTable.Seek( nId ) )
    {
        if ( pMacro )
            aMacroTable.Replace( nId, pMacro );
        else
            aMacroTable.Remove( nId );
    }
    else if ( pMacro )
        aMacroTable.Insert( nId, pMacro );
    SetDefault(FALSE);
}

void SfxEventConfiguration::AddEvents( SfxMacroTabPage* pPage ) const
{
    DBG_ASSERT(pEventArr,"Keine Events angemeldet!");
    USHORT nCount = pEventArr->Count();
    for (USHORT n=1; n<nCount; n++)
        pPage->AddEvent( (*pEventArr)[n]->aEventName, (*pEventArr)[n]->nEventId );
}

SvxMacroTableDtor* SfxEventConfiguration::GetAppEventTable()
{
    return &GetAppEventConfig_Impl()->aMacroTable;
}

SvxMacroTableDtor* SfxEventConfiguration::GetDocEventTable( SfxObjectShell*pDoc )
{
    pDocEventConfig = pDoc ? pDoc->GetEventConfig_Impl() : NULL;
    if ( pDocEventConfig )
        return &pDocEventConfig->aMacroTable;
    return NULL;
}

void SfxEventConfiguration::SetAppEventTable( const SvxMacroTableDtor& rTable )
{
    //GetAppEventConfig_Impl()->aMacroTable = rTable;
    //pAppEventConfig->SetDefault(FALSE);
    PropagateEvents_Impl( NULL, rTable );
}

void SfxEventConfiguration::SetDocEventTable( SfxObjectShell *pDoc,
    const SvxMacroTableDtor& rTable )
{
    if ( pDoc )
    {
        // if CfgMgr does not exist, create it, otherwise ConfigItem will not have a ConfigManager!
        pDoc->GetConfigManager(TRUE);
        pDocEventConfig = pDoc->GetEventConfig_Impl( TRUE );
        //pDocEventConfig->aMacroTable = rTable;
        //pDocEventConfig->SetDefault(FALSE);
        pDoc->SetModified(TRUE);
        PropagateEvents_Impl( pDoc, rTable );
    }
    else
        DBG_ERROR( "Kein Dokument!" );
}

//--------------------------------------------------------------------------
void SfxEventConfiguration::PropagateEvents_Impl( SfxObjectShell *pDoc,
    const SvxMacroTableDtor& rTable )
{
    REFERENCE< XEVENTSSUPPLIER > xSupplier;
    if ( pDoc )
    {
        xSupplier = REFERENCE< XEVENTSSUPPLIER >( pDoc->GetModel(), UNO_QUERY );
    }
    else
    {
        xSupplier = REFERENCE< XEVENTSSUPPLIER >
                ( ::comphelper::getProcessServiceFactory()->createInstance(
                        rtl::OUString::createFromAscii("com.sun.star.frame.GlobalEventBroadcaster" )), UNO_QUERY );
    }

    if ( xSupplier.is() )
    {
        SvxMacro   *pMacro;
        ULONG       nCount;
        ULONG       nID, i;

        REFERENCE< XNAMEREPLACE > xEvents = xSupplier->getEvents();

        bIgnoreConfigure = sal_True;

        // Erase old values first, because we don't know anything about the
        // changes here

        SEQUENCE < PROPERTYVALUE > aProperties;
        SEQUENCE < OUSTRING > aEventNames = xEvents->getElementNames();
        OUSTRING*   pNames  = aEventNames.getArray();
        ANY aEmpty;

        aEmpty <<= aProperties;
        nCount  = aEventNames.getLength();

        for ( i=0; i<nCount; i++ )
        {
            try
            {
                xEvents->replaceByName( pNames[i], aEmpty );
            }
            catch( ::com::sun::star::lang::IllegalArgumentException )
            { DBG_ERRORFILE( "PropagateEvents_Impl: caught IllegalArgumentException" ) }
            catch( ::com::sun::star::container::NoSuchElementException )
            { DBG_ERRORFILE( "PropagateEvents_Impl: caught NoSuchElementException" ) }
        }

        // now set the new values

        nCount = rTable.Count();

        for ( i=0; i<nCount; i++ )
        {
            pMacro = rTable.GetObject( i );
            nID = rTable.GetObjectKey( i );
            OUSTRING aEventName = GetEventName_Impl( nID );

            if ( aEventName.getLength() )
            {
                ANY aEventData = CreateEventData_Impl( pMacro );
                try
                {
                    xEvents->replaceByName( aEventName, aEventData );
                }
                catch( ::com::sun::star::lang::IllegalArgumentException )
                { DBG_ERRORFILE( "PropagateEvents_Impl: caught IllegalArgumentException" ) }
                catch( ::com::sun::star::container::NoSuchElementException )
                { DBG_ERRORFILE( "PropagateEvents_Impl: caught NoSuchElementException" ) }
            }
            else
                DBG_WARNING( "PropagateEvents_Impl: Got unkown event" );
        }

        bIgnoreConfigure = sal_False;
    }
}

//--------------------------------------------------------------------------
void SfxEventConfiguration::PropagateEvent_Impl( SfxObjectShell *pDoc,
                                                 USHORT nId,
                                                 const SvxMacro* pMacro )
{
    REFERENCE< XEVENTSSUPPLIER > xSupplier;
    if ( pDoc )
    {
        xSupplier = REFERENCE< XEVENTSSUPPLIER >( pDoc->GetModel(), UNO_QUERY );
    }
    else
    {
        xSupplier = REFERENCE< XEVENTSSUPPLIER >
                ( ::comphelper::getProcessServiceFactory()->createInstance(
                        rtl::OUString::createFromAscii("com.sun.star.frame.GlobalEventBroadcaster" )), UNO_QUERY );
    }

    if ( xSupplier.is() )
    {
        REFERENCE< XNAMEREPLACE > xEvents = xSupplier->getEvents();

        bIgnoreConfigure = sal_True;

        OUSTRING aEventName = GetEventName_Impl( nId );

        if ( aEventName.getLength() )
        {
            ANY aEventData = CreateEventData_Impl( pMacro );

            try
            {
                xEvents->replaceByName( aEventName, aEventData );
            }
            catch( ::com::sun::star::lang::IllegalArgumentException )
            { DBG_ERRORFILE( "PropagateEvents_Impl: caught IllegalArgumentException" ) }
            catch( ::com::sun::star::container::NoSuchElementException )
            { DBG_ERRORFILE( "PropagateEvents_Impl: caught NoSuchElementException" ) }
        }
        else
            DBG_WARNING( "PropagateEvents_Impl: Got unkown event" );

        bIgnoreConfigure = sal_False;
    }
}

// -------------------------------------------------------------------------------------------------------
ANY SfxEventConfiguration::CreateEventData_Impl( const SvxMacro *pMacro )
{
/*
    This function converts a SvxMacro into an Any containing three
    properties. These properties are EventType and Script. Possible
    values for EventType ar StarBasic, JavaScript, ...
    The Script property should contain the URL to the macro and looks
    like "macro://./standard.module1.main()"

    If pMacro is NULL, we return an empty property sequence, so PropagateEvent_Impl
    can delete an event binding.
*/
    ANY aEventData;

    if ( pMacro )
    {
        if ( pMacro->GetScriptType() == STARBASIC )
        {
            SEQUENCE < PROPERTYVALUE > aProperties(3);
            PROPERTYVALUE  *pValues = aProperties.getArray();

            OUSTRING    aType   = OUSTRING::createFromAscii( STAR_BASIC );;
            OUSTRING    aLib    = pMacro->GetLibName();
            OUSTRING    aMacro  = pMacro->GetMacName();

            pValues[ 0 ].Name = OUSTRING::createFromAscii( PROP_EVENT_TYPE );
            pValues[ 0 ].Value <<= aType;

            pValues[ 1 ].Name = OUSTRING::createFromAscii( PROP_LIBRARY );
            pValues[ 1 ].Value <<= aLib;

            pValues[ 2 ].Name = OUSTRING::createFromAscii( PROP_MACRO_NAME );
            pValues[ 2 ].Value <<= aMacro;

            aEventData <<= aProperties;
        }
        else if ( pMacro->GetScriptType() == EXTENDED_STYPE )
        {
            SEQUENCE < PROPERTYVALUE > aProperties(2);
            PROPERTYVALUE  *pValues = aProperties.getArray();

            OUSTRING    aLib    = pMacro->GetLibName();
            OUSTRING    aMacro  = pMacro->GetMacName();

            pValues[ 0 ].Name = OUSTRING::createFromAscii( PROP_EVENT_TYPE );
            pValues[ 0 ].Value <<= aLib;

            pValues[ 1 ].Name = OUSTRING::createFromAscii( PROP_SCRIPT );
            pValues[ 1 ].Value <<= aMacro;

            aEventData <<= aProperties;
        }
        else if ( pMacro->GetScriptType() == JAVASCRIPT )
        {
            SEQUENCE < PROPERTYVALUE > aProperties(2);
            PROPERTYVALUE  *pValues = aProperties.getArray();

            OUSTRING    aMacro  = pMacro->GetMacName();

            pValues[ 0 ].Name = OUSTRING::createFromAscii( PROP_EVENT_TYPE );
            pValues[ 0 ].Value <<= ::rtl::OUString::createFromAscii(SVX_MACRO_LANGUAGE_JAVASCRIPT);

            pValues[ 1 ].Name = OUSTRING::createFromAscii( PROP_MACRO_NAME );
            pValues[ 1 ].Value <<= aMacro;

            aEventData <<= aProperties;
        }
        else
        {
            DBG_ERRORFILE( "CreateEventData_Impl(): ScriptType not supported!");
        }
    }
    else
    {
        SEQUENCE < PROPERTYVALUE > aProperties;
        aEventData <<= aProperties;
    }

    return aEventData;
}

// -------------------------------------------------------------------------------------------------------
ULONG SfxEventConfiguration::GetPos_Impl( USHORT nId, sal_Bool &rFound )
{
    rFound = sal_False;

    if ( ! gp_Id_SortList->Count() )
        return 0;

    // use binary search to find the correct position
    // in the list

    int     nCompVal = 1;
    long    nStart = 0;
    long    nEnd = gp_Id_SortList->Count() - 1;
    long    nMid;

    EventNames_Impl* pMid;

    rFound = sal_False;

    while ( nCompVal && ( nStart <= nEnd ) )
    {
        nMid = ( nEnd - nStart ) / 2 + nStart;
        pMid = gp_Id_SortList->GetObject( (USHORT) nMid );

        nCompVal = pMid->mnId - nId;

        if ( nCompVal < 0 )     // pMid < pData
            nStart = nMid + 1;
        else
            nEnd = nMid - 1;
    }

    if ( nCompVal == 0 )
    {
        rFound = sal_True;
    }
    else
    {
        if ( nCompVal < 0 )     // pMid < pData
            nMid++;
    }

    return (USHORT) nMid;
}

// -------------------------------------------------------------------------------------------------------
ULONG SfxEventConfiguration::GetPos_Impl( const String& rName, sal_Bool &rFound )
{
    rFound = sal_False;

    if ( ! gp_Name_SortList->Count() )
        return 0;

    // use binary search to find the correct position
    // in the list

    int     nCompVal = 1;
    long    nStart = 0;
    long    nEnd = gp_Name_SortList->Count() - 1;
    long    nMid;

    EventNames_Impl* pMid;

    rFound = sal_False;

    while ( nCompVal && ( nStart <= nEnd ) )
    {
        nMid = ( nEnd - nStart ) / 2 + nStart;
        pMid = gp_Name_SortList->GetObject( (USHORT) nMid );

        nCompVal = rName.CompareTo( pMid->maEventName );

        if ( nCompVal < 0 )     // pMid < pData
            nStart = nMid + 1;
        else
            nEnd = nMid - 1;
    }

    if ( nCompVal == 0 )
    {
        rFound = sal_True;
    }
    else
    {
        if ( nCompVal < 0 )     // pMid < pData
            nMid++;
    }

    return (USHORT) nMid;
}

//--------------------------------------------------------------------------------------------------------
OUSTRING SfxEventConfiguration::GetEventName_Impl( ULONG nID )
{
    OUSTRING    aRet;

    if ( gp_Id_SortList )
    {
        sal_Bool    bFound;
        ULONG       nPos = GetPos_Impl( (USHORT) nID, bFound );

        if ( bFound )
        {
            EventNames_Impl *pData = gp_Id_SortList->GetObject( nPos );
            aRet = pData->maEventName;
        }
    }

    return aRet;
}

//--------------------------------------------------------------------------------------------------------
ULONG SfxEventConfiguration::GetEventId_Impl( const OUSTRING& rEventName )
{
    ULONG nRet = 0;

    if ( gp_Name_SortList )
    {
        sal_Bool    bFound;
        ULONG       nPos = GetPos_Impl( rEventName, bFound );

        if ( bFound )
        {
            EventNames_Impl *pData = gp_Name_SortList->GetObject( nPos );
            nRet = pData->mnId;
        }
    }

    return nRet;
}

// -------------------------------------------------------------------------------------------------------
void SfxEventConfiguration::RegisterEvent( USHORT nId,
                                           const String& rUIName,
                                           const String& rMacroName )
{
    if ( ! gp_Id_SortList )
    {
        gp_Id_SortList = new SfxEventList_Impl;
        gp_Name_SortList = new SfxEventList_Impl;
    }

    sal_Bool    bFound = sal_False;
    ULONG       nPos = GetPos_Impl( nId, bFound );

    if ( bFound )
    {
        DBG_ERRORFILE( "RegisterEvent: Event already registered?" );
        return;
    }

    EventNames_Impl *pData;

    pData = new EventNames_Impl( nId, rMacroName, rUIName );
    gp_Id_SortList->Insert( pData, nPos );

    nPos = GetPos_Impl( rMacroName, bFound );
    DBG_ASSERT( !bFound, "RegisterEvent: Name in List, but ID not?" );

    gp_Name_SortList->Insert( pData, nPos );

    SFX_APP()->GetEventConfig()->RegisterEvent( nId, rUIName );
}

BOOL SfxEventConfiguration::Import( SvStream& rInStream, SvStream* pOutStream, SfxObjectShell* pDoc )
{
    if ( pDoc )
    {
        // load events, they are automatically propagated to the document
        DBG_ASSERT( !pOutStream, "DocEventConfig must not be converted!" );
        SfxEventConfigItem_Impl* pCfg = pDoc->GetEventConfig_Impl( TRUE );
        if ( pCfg )
            return ( pCfg->Load( rInStream ) == SfxConfigItem::ERR_OK );
        DBG_ERROR("Couldn't create EventConfiguration!");
        return FALSE;
    }
    else if ( pOutStream )
    {
        SfxEventConfiguration aConfig;
        if ( aConfig.GetAppEventConfig_Impl()->Load( rInStream ) == SfxConfigItem::ERR_OK )
            return aConfig.pAppEventConfig->StoreXML( *pOutStream );
        return FALSE;
    }

    DBG_ERROR( "No OutStream!" );
    return FALSE;
}

BOOL SfxEventConfiguration::Export( SvStream* pInStream, SvStream& rOutStream, SfxObjectShell* pDoc )
{
    if ( pDoc )
    {
        DBG_ASSERT( !pInStream, "DocEventConfig can't be converted!" );
        SfxEventConfigItem_Impl* pCfg = pDoc->GetEventConfig_Impl();
        if ( pCfg )
            return pCfg->Store( rOutStream );
        DBG_ERROR("Couldn't create EventConfiguration!");
        return FALSE;
    }
    else if ( pInStream )
    {
        SfxEventConfiguration aConfig;
        if ( aConfig.GetAppEventConfig_Impl()->LoadXML( *pInStream ) )
            return aConfig.pAppEventConfig->Store( rOutStream );
        return FALSE;
    }

    DBG_ERROR( "No InStream!" );
    return FALSE;
}


