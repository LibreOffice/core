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

#if defined(WIN) || defined(WNT)
#include <tools/svwin.h>
#endif

#include "impldde.hxx"

#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>
#include <sot/exchange.hxx>
#include <rtl/ustring.hxx>

#include <bf_so3/iface.hxx>
#include "svuidlg.hrc"
#include "bf_so3/lnkbase.hxx"
#include "bf_so3/linkmgr.hxx"
#include "bf_so3/soerr.hxx"

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <bf_svtools/svdde.hxx>
#include <sot/formats.hxx>

#define DDELINK_COLD		0
#define DDELINK_HOT 		1

#define DDELINK_ERROR_APP	1
#define DDELINK_ERROR_DATA	2
#define DDELINK_ERROR_LINK	3

using namespace ::com::sun::star::uno;

namespace binfilter
{

SvDDEObject::SvDDEObject()
    : pConnection( 0 ), pLink( 0 ), pRequest( 0 ), pGetData( 0 ), nError( 0 )
{
    SetUpdateTimeout( 100 );
    bWaitForData = FALSE;
}

SvDDEObject::~SvDDEObject()
{
    delete pLink;
    delete pRequest;
    delete pConnection;
}

BOOL SvDDEObject::GetData( ::com::sun::star::uno::Any & rData /*out param*/,
                            const String & rMimeType,
                            BOOL bSynchron )
{
    if( !pConnection )
        return FALSE;

    if( pConnection->GetError() )		// dann versuchen wir es nochmal
    {
        String sServer( pConnection->GetServiceName() );
        String sTopic( pConnection->GetTopicName() );

        delete pConnection;
        pConnection = new DdeConnection( sServer, sTopic );
        if( pConnection->GetError() )
            nError = DDELINK_ERROR_APP;
    }

    if( bWaitForData )		// wir sind rekursiv drin, wieder raus
        return FALSE;

    // Verriegeln gegen Reentrance
    bWaitForData = TRUE;

    // falls gedruckt werden soll, warten wir bis die Daten vorhanden sind
    if( bSynchron )
    {
        DdeRequest aReq( *pConnection, sItem, 5000 );
        aReq.SetDataHdl( LINK( this, SvDDEObject, ImplGetDDEData ) );
        aReq.SetFormat( SotExchange::GetFormatIdFromMimeType( rMimeType ));

        pGetData = &rData;

        do {
            aReq.Execute();
        } while( aReq.GetError() && ImplHasOtherFormat( aReq ) );

        if( pConnection->GetError() )
            nError = DDELINK_ERROR_DATA;

        bWaitForData = FALSE;
    }
    else
    {
        // ansonsten wird es asynchron ausgefuehrt
//		if( !pLink || !pLink->IsBusy() )
        {
            if( pRequest )
                delete pRequest;

            pRequest = new DdeRequest( *pConnection, sItem );
            pRequest->SetDataHdl( LINK( this, SvDDEObject, ImplGetDDEData ) );
            pRequest->SetDoneHdl( LINK( this, SvDDEObject, ImplDoneDDEData ) );
            pRequest->SetFormat( SotExchange::GetFormatIdFromMimeType(
                                    rMimeType ) );
            pRequest->Execute();
        }

        ::rtl::OUString aEmptyStr;
        rData <<= aEmptyStr;
    }
    return 0 == pConnection->GetError();
}


BOOL SvDDEObject::Connect( SvBaseLink * pSvLink )
{
#if defined(WIN) || defined(WNT)
    static BOOL bInWinExec = FALSE;
#endif

    USHORT nLinkType = pSvLink->GetUpdateMode();
    if( pConnection )		// Verbindung steht ja schon
    {
        // tja, dann nur noch als Abhaengig eintragen
        AddDataAdvise( pSvLink,
                SotExchange::GetFormatMimeType( pSvLink->GetContentType()),
                LINKUPDATE_ONCALL == nLinkType
                        ? ADVISEMODE_ONLYONCE
                        : 0 );
        AddConnectAdvise( pSvLink );

        return TRUE;
    }

    if( !pSvLink->GetLinkManager() )
        return FALSE;

    String sServer, sTopic;
    pSvLink->GetLinkManager()->GetDisplayNames( pSvLink, &sServer, &sTopic, &sItem );

    if( !sServer.Len() || !sTopic.Len() || !sItem.Len() )
        return FALSE;

    pConnection = new DdeConnection( sServer, sTopic );
    if( pConnection->GetError() )
    {
        // kann man denn das System-Topic ansprechen ?
        // dann ist der Server oben, kennt nur nicht das Topic!
        if( sTopic.EqualsIgnoreCaseAscii( "SYSTEM" ) )
        {
            BOOL bSysTopic;
            {
                DdeConnection aTmp( sServer, String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "SYSTEM" ) ) );
                bSysTopic = !aTmp.GetError();
            }

            if( bSysTopic )
            {
                nError = DDELINK_ERROR_DATA;
                return FALSE;
            }
            // ansonsten unter Win/WinNT die Applikation direkt starten
        }

#if defined(WIN) || defined(WNT)

        // Server nicht da, starten und nochmal versuchen
        if( !bInWinExec )
        {
            ByteString aCmdLine( sServer, RTL_TEXTENCODING_ASCII_US );
            aCmdLine.Append( ".exe " );
            aCmdLine.Append( ByteString( sTopic, RTL_TEXTENCODING_ASCII_US ) );

            if( WinExec( aCmdLine.GetBuffer(), SW_SHOWMINIMIZED ) < 32 )
                nError = DDELINK_ERROR_APP;
            else
            {
                USHORT i;
                for( i=0; i<5; i++ )
                {
                    bInWinExec = TRUE;
                    Application::Reschedule();
                    bInWinExec = FALSE;

                    delete pConnection;
                    pConnection = new DdeConnection( sServer, sTopic );
                    if( !pConnection->GetError() )
                        break;
                }

                if( i == 5 )
                {
                    nError = DDELINK_ERROR_APP;
                }
            }
        }
        else
#endif	// WIN / WNT
        {
            nError = DDELINK_ERROR_APP;
        }
    }

    if( LINKUPDATE_ALWAYS == nLinkType && !pLink && !pConnection->GetError() )
    {
        // Hot Link einrichten, Daten kommen irgendwann spaeter
        pLink = new DdeHotLink( *pConnection, sItem );
        pLink->SetDataHdl( LINK( this, SvDDEObject, ImplGetDDEData ) );
        pLink->SetDoneHdl( LINK( this, SvDDEObject, ImplDoneDDEData ) );
        pLink->SetFormat( pSvLink->GetContentType() );
        pLink->Execute();
    }

    if( pConnection->GetError() )
        return FALSE;

    AddDataAdvise( pSvLink,
                SotExchange::GetFormatMimeType( pSvLink->GetContentType()),
                LINKUPDATE_ONCALL == nLinkType
                        ? ADVISEMODE_ONLYONCE
                        : 0 );
    AddConnectAdvise( pSvLink );
    SetUpdateTimeout( 0 );
    return TRUE;
}

String SvDDEObject::Edit( Window* /*pParent*/, SvBaseLink * /*pLink_*/ )
{
    DBG_ERROR( "SvDDEObject::Edit: not implemented!" );
    // TODO: dead corpses
    return String();
}

BOOL SvDDEObject::ImplHasOtherFormat( DdeTransaction& rReq )
{
    USHORT nFmt = 0;
    switch( rReq.GetFormat() )
    {
    case FORMAT_RTF:
        nFmt = FORMAT_STRING;
        break;

    case SOT_FORMATSTR_ID_HTML_SIMPLE:
    case SOT_FORMATSTR_ID_HTML:
        nFmt = FORMAT_RTF;
        break;

    case FORMAT_GDIMETAFILE:
        nFmt = FORMAT_BITMAP;
        break;

    case SOT_FORMATSTR_ID_SVXB:
        nFmt = FORMAT_GDIMETAFILE;
        break;

    // sonst noch irgendwas ??
    }
    if( nFmt )
        rReq.SetFormat( nFmt );		// damit nochmal versuchen
    return 0 != nFmt;
}

BOOL SvDDEObject::IsPending() const
/*	[Beschreibung]

    Die Methode stellt fest, ob aus einem DDE-Object die Daten gelesen
    werden kann.
    Zurueckgegeben wird:
        ERRCODE_NONE 			wenn sie komplett gelesen wurde
        ERRCODE_SO_PENDING		wenn sie noch nicht komplett gelesen wurde
        ERRCODE_SO_FALSE		sonst
*/
{
    return bWaitForData;
}

BOOL SvDDEObject::IsDataComplete() const
{
    return bWaitForData;
}

IMPL_LINK( SvDDEObject, ImplGetDDEData, DdeData*, pData )
{
    ULONG nFmt = pData->GetFormat();
    switch( nFmt )
    {
    case FORMAT_GDIMETAFILE:
        break;

    case FORMAT_BITMAP:
        break;

    default:
        {
            const sal_Char* p = (sal_Char*)( pData->operator const void*() );
            long nLen = FORMAT_STRING == nFmt ? (p ? strlen( p ) : 0) : (long)*pData;

            Sequence< sal_Int8 > aSeq( (const sal_Int8*)p, nLen );
            if( pGetData )
            {
                *pGetData <<= aSeq; 	// Daten kopieren
                pGetData = 0;			// und den Pointer bei mir zuruecksetzen
            }
            else
            {
                Any aVal;
                aVal <<= aSeq;
                DataChanged( SotExchange::GetFormatMimeType(
                                                pData->GetFormat() ), aVal );
                bWaitForData = FALSE;
            }
        }
    }

    return 0;
}

IMPL_LINK( SvDDEObject, ImplDoneDDEData, void*, pData )
{
    BOOL bValid = (BOOL)(ULONG)pData;
    if( !bValid && ( pRequest || pLink ))
    {
        DdeTransaction* pReq = 0;
        if( !pLink || ( pLink && pLink->IsBusy() ))
            pReq = pRequest;		// dann kann nur der fertig sein
        else if( pRequest && pRequest->IsBusy() )
            pReq = pLink;			// dann kann nur der fertig sein

        if( pReq )
        {
            if( ImplHasOtherFormat( *pReq ) )
            {
                pReq->Execute();
            }
            else if( pReq == pRequest )
            {
                // das wars dann
                bWaitForData = FALSE;
            }
        }
    }
    else
        // das warten ist beendet
        bWaitForData = FALSE;

    return 0;
}

}
