/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: evntconf.cxx,v $
 * $Revision: 1.32 $
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
#include <vcl/msgbox.hxx>
#ifndef _SV_RESARY_HXX
#include <tools/resary.hxx>
#endif
#include <svtools/lstner.hxx>
#include <basic/basmgr.hxx>
#include <basic/sbmod.hxx>
#include <tools/urlobj.hxx>
#include <basic/sbx.hxx>

#include <sot/storage.hxx>
#include <svtools/securityoptions.hxx>

#ifndef _RTL_USTRING_
#include <rtl/ustring.h>
#endif

#include <com/sun/star/uno/Any.hxx>
#include <framework/eventsconfiguration.hxx>
#include <comphelper/processfactory.hxx>

#include <sfx2/evntconf.hxx>

#include <sfx2/macrconf.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/dispatch.hxx>
#include "config.hrc"
#include "sfxresid.hxx"
#include "eventsupplier.hxx"

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>

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
TYPEINIT1(SfxEventNamesItem, SfxPoolItem);

using namespace com::sun::star;

SfxEventNamesList& SfxEventNamesList::operator=( const SfxEventNamesList& rTbl )
{
    DelDtor();
    for (USHORT n=0; n<rTbl.Count(); n++ )
    {
        SfxEventName* pTmp = ((SfxEventNamesList&)rTbl).GetObject(n);
        SfxEventName *pNew = new SfxEventName( *pTmp );
        Insert( pNew, n );
    }
    return *this;
}

void SfxEventNamesList::DelDtor()
{
    SfxEventName* pTmp = First();
    while( pTmp )
    {
        delete pTmp;
        pTmp = Next();
    }
    Clear();
}

int SfxEventNamesItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    const SfxEventNamesList& rOwn = aEventsList;
    const SfxEventNamesList& rOther = ( (SfxEventNamesItem&) rAttr ).aEventsList;

    if ( rOwn.Count() != rOther.Count() )
        return FALSE;

    for ( USHORT nNo = 0; nNo < rOwn.Count(); ++nNo )
    {
        const SfxEventName *pOwn = rOwn.GetObject(nNo);
        const SfxEventName *pOther = rOther.GetObject(nNo);
        if (    pOwn->mnId != pOther->mnId ||
                pOwn->maEventName != pOther->maEventName ||
                pOwn->maUIName != pOther->maUIName )
            return FALSE;
    }

    return TRUE;

}

SfxItemPresentation SfxEventNamesItem::GetPresentation( SfxItemPresentation,
                                    SfxMapUnit,
                                    SfxMapUnit,
                                    XubString &rText,
                                    const IntlWrapper* ) const
{
    rText.Erase();
    return SFX_ITEM_PRESENTATION_NONE;
}

SfxPoolItem* SfxEventNamesItem::Clone( SfxItemPool *) const
{
    return new SfxEventNamesItem(*this);
}

SfxPoolItem* SfxEventNamesItem::Create(SvStream &, USHORT) const
{
    DBG_ERROR("not streamable!");
    return new SfxEventNamesItem(Which());
}

SvStream& SfxEventNamesItem::Store(SvStream &rStream, USHORT ) const
{
    DBG_ERROR("not streamable!");
    return rStream;
}

USHORT SfxEventNamesItem::GetVersion( USHORT ) const
{
    DBG_ERROR("not streamable!");
    return 0;
}

void SfxEventNamesItem::AddEvent( const String& rName, const String& rUIName, USHORT nID )
{
    aEventsList.Insert( new SfxEventName( nID, rName, rUIName.Len() ? rUIName : rName ) );
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

void SfxAsyncEvent_Impl::Notify( SfxBroadcaster&, const SfxHint& rHint )
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

IMPL_LINK(SfxAsyncEvent_Impl, TimerHdl, Timer*, pAsyncTimer)
{
    pAsyncTimer->Stop();
    ScriptType eSType = pMacro->GetScriptType();
    BOOL bIsBasic = ( eSType == STARBASIC );
    if ( bIsBasic && StarBASIC::IsRunning() )
        // Neues eventgebundenes Macro erst ausf"uhren, wenn gerade kein anderes Macro mehr l"auft
        pAsyncTimer->Start();
    else
    {
        SFX_APP()->GetMacroConfig()->ExecuteMacro( pSh, pMacro, aArgs );
        delete this;
    }

    return 0L;
}

SfxEventNamesList   *gp_Id_SortList = NULL;
SfxEventNamesList   *gp_Name_SortList = NULL;

//==========================================================================

SfxEventConfiguration::SfxEventConfiguration()
 : pAppTable( NULL )
 , pDocTable( NULL )
{
    bIgnoreConfigure = sal_False;
}

//==========================================================================

SfxEventConfiguration::~SfxEventConfiguration()
{
    delete pDocTable;

    if ( gp_Id_SortList )
    {
        delete gp_Id_SortList;
        delete gp_Name_SortList;
        gp_Id_SortList = NULL;
        gp_Name_SortList = NULL;
    }
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
        PropagateEvent_Impl( pDoc, nId, pMacro );
    }
    else
    {
        PropagateEvent_Impl( NULL, nId, pMacro );
    }
}

//==========================================================================

SvxMacroTableDtor* SfxEventConfiguration::GetDocEventTable( SfxObjectShell*pDoc )
{
    if ( pDocTable )
        delete pDocTable;

    pDocTable = new SvxMacroTableDtor;

    if ( pDoc )
    {
        REFERENCE< XEVENTSSUPPLIER > xSup( pDoc->GetModel(), UNO_QUERY );
        uno::Reference < container::XNameReplace > xEvents = xSup->getEvents();

        uno::Sequence < ::rtl::OUString > aNames = xEvents->getElementNames();
        for ( sal_Int32 i=0; i<aNames.getLength(); i++ )
        {
            SvxMacro* pMacro = SfxEvents_Impl::ConvertToMacro( xEvents->getByName( aNames[i] ), pDoc, TRUE );
            USHORT nID = (USHORT) GetEventId_Impl( aNames[i] );
            if ( nID && pMacro )
                pDocTable->Insert( nID, pMacro );
        }
    }

    return pDocTable;
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
            { DBG_ERRORFILE( "PropagateEvents_Impl: caught IllegalArgumentException" ); }
            catch( ::com::sun::star::container::NoSuchElementException )
            { DBG_ERRORFILE( "PropagateEvents_Impl: caught NoSuchElementException" ); }
        }
        else {
            DBG_WARNING( "PropagateEvents_Impl: Got unkown event" );
        }

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
    long    nMid = 0;

    SfxEventName* pMid;

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
    long    nMid = 0;

    SfxEventName* pMid;

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
            SfxEventName *pData = gp_Id_SortList->GetObject( nPos );
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
            SfxEventName *pData = gp_Name_SortList->GetObject( nPos );
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
        gp_Id_SortList = new SfxEventNamesList;
        gp_Name_SortList = new SfxEventNamesList;
    }

    sal_Bool    bFound = sal_False;
    ULONG       nPos = GetPos_Impl( nId, bFound );

    if ( bFound )
    {
        DBG_ERRORFILE( "RegisterEvent: Event already registered?" );
        return;
    }

    gp_Id_SortList->Insert( new SfxEventName( nId, rMacroName, rUIName ), nPos );
    nPos = GetPos_Impl( rMacroName, bFound );

    DBG_ASSERT( !bFound, "RegisterEvent: Name in List, but ID not?" );

    gp_Name_SortList->Insert( new SfxEventName( nId, rMacroName, rUIName ), nPos );

    SFX_APP()->GetEventConfig();
}
