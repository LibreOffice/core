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

#include <stdio.h>


#include "bf_so3/ipclient.hxx"
#include <tools/debug.hxx>
#include <vcl/wrkwin.hxx>

#include <bf_so3/so2dll.hxx>
#include <bf_so3/ipobj.hxx>
#include <bf_so3/ipenv.hxx>
#include "bf_so3/soerr.hxx"

namespace binfilter {

//=========================================================================
//=========================================================================
//=========================================================================
SV_IMPL_FACTORY(SvInPlaceClientFactory)
    {
    }
};
TYPEINIT1(SvInPlaceClientFactory,SvFactory);

SO2_IMPL_STANDARD_CLASS1_DLL(SvInPlaceClient,SvInPlaceClientFactory,SvEmbeddedClient,
                             0x35356980L, 0x795D, 0x101B,
                             0x80,0x4C,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD )

//=========================================================================
/*	[Beschreibung]

    Makro, um fuer die verschiedenen Konstruktoren die gleich
    Initialisierung sicherzustellen.
*/
#define INIT_CLIENT_CTOR		\
      pObjI		( NULL )

//=========================================================================
SvInPlaceClient::SvInPlaceClient()
    : INIT_CLIENT_CTOR
/*	[Beschreibung]

    Dieser Konstruktor wird verwendet, wenn man sich vom Client abgeleitet
    hat oder wenn man einen Client zu einem Objekt erstellen will.
*/
{
}

//=========================================================================
SvInPlaceClient::~SvInPlaceClient()
/*	[Beschreibung]

    Destruktor der Klasse SvInPlaceClient. Wurde das Environment noch
    nicht freigegeben, dann passiert dies hier.

    [Querverweise]
*/
{
    DBG_ASSERT( LIST_ENTRY_NOTFOUND == GetIPActiveClientList().GetPos( this ),
                "ip-client in ip-client-list" );

}

//=========================================================================
::IUnknown * SvInPlaceClient::GetMemberInterface( const SvGlobalName & rName )
{
    (void)rName;
    return NULL;
}

//=========================================================================
void SvInPlaceClient::TestMemberObjRef( BOOL bFree )
{
    (void)bFree;
#ifdef DBG_UTIL
    if( GetIPObj() )
    {
        ByteString aTest = "\t\tGetIPObj() == ";
        aTest.Append( ByteString::CreateFromInt32( (ULONG)(SvObject *)GetIPObj() ) );
        DBG_TRACE( aTest.GetBuffer() );
    }
#endif
}

//=========================================================================
#ifdef TEST_INVARIANT
void SvInPlaceClient::TestMemberInvariant( BOOL bPrint )
{
    (void)bPrint;
}
#endif

//=========================================================================
BOOL SvInPlaceClient::CanInPlaceActivate() const
/*	[Beschreibung]

    Gibt an, ob der Container zum jetzigen Zeitpunkt IP-Aktivierung
    unterst"utzt.

    [R"uckgabewert]

    BOOL		TRUE, der Container unterst"utzt zur Zeit IP-Aktivierung.
                FALSE, der Container unterst"utzt zur Zeit keine
                IP-Aktivierung.
*/
{
    BOOL bRet = FALSE;
    if( Owner() )
    {
        bRet = GetAspect() == ASPECT_CONTENT;
        if( bRet )
            // Nur IP gestatten, wenn das Environment verf"ugbar ist
            bRet = NULL != ((SvInPlaceClient *)this)->GetEnv();
    }
    return bRet;
}

//=========================================================================
SvInPlaceClientList & SvInPlaceClient::GetIPActiveClientList()
/*	[Beschreibung]

    Gibt die Liste aller zu diesem Zeitpunkt IP-Aktiven Clients
    zur"uck.

    [R"uckgabewert]

    SvInPlaceClientList & 	Die Liste der Clients.
*/
{
    SoDll * pSoApp = SOAPP;
    if( !pSoApp->pIPActiveClientList )
        pSoApp->pIPActiveClientList = new SvInPlaceClientList();
    return *pSoApp->pIPActiveClientList;
}

//=========================================================================
void SvInPlaceClient::MakeViewData()
/*	[Beschreibung]

    Gibt das ContainerEnvironment zur"uck. Wurde es nicht im
    Konstruktor "ubergeben, dann muss diese Methode "uberladen
    werden.

    [R"uckgabewert]

    SvClientData * 	Das Environment zu diesem Client wird zur"uckgegeben.

    [Querverweise]

    <SvEmbeddedClient::MakeViewData>, <SvEmbeddedClient::FreeViewData>
*/
{

    if( !Owner() && !HasViewData() && GetProtocol().IsConnect() )
    {
        pData = new SvContainerEnvironment( this );
        bDeleteData = TRUE;
    }
    else
        SvEmbeddedClient::MakeViewData();
}

//=========================================================================
SvContainerEnvironment * SvInPlaceClient::GetEnv()
/*	[Beschreibung]

    Gibt das ContainerEnvironment zur"uck. Ein neues Environment
    wird nur angelegt, wenn der Client "Connected" ist.

    [R"uckgabewert]

    SvContainerEnvironment * 	Das Environment zu diesem Client wird
                                zur"uckgegeben.

    [Querverweise]

    <SvProtocol::IsConnect>, <SvInPlaceClient::MakeViewData>
*/
{
    if( !pData && GetProtocol().IsConnect() )
        MakeViewData();
    return PTR_CAST( SvContainerEnvironment, pData );
}

//=========================================================================
void SvInPlaceClient::MakeVisible()
/*	[Beschreibung]

    Das Objekt, mit dem dieses Environment verbunden ist, soll in den
    sichtbaren Bereich des Containers geschoben werden. Als Default wird
    das Top- und das DocWindow angezeigt.
*/
{
    SvEmbeddedClient::MakeVisible();
    if( Owner() )
    {
        SvContainerEnvironment * pEnv = GetEnv();
        if( pEnv )
        {
            WorkWindow * pDoc = pEnv->GetDocWin();
            WorkWindow * pTop = pEnv->GetTopWin();
            if( pDoc )
            {
                if( pDoc->IsMinimized() )
                    pDoc->Restore();
                //pDoc->ToTop();
                pDoc->Show();
            }
            if( pTop )
            {
                if( pTop->IsMinimized() )
                    pTop->Restore();
                //pTop->ToTop();
                pTop->Show();
            }
        }
    }
}

//=========================================================================
void SvInPlaceClient::Opened
(
    BOOL bOpen	/* TRUE, in den Open-Status.
                   FALSE, in den Connect-Status. */
)
/*	[Beschreibung]

    Dieser Handler wird gerufen, wenn der Open-Status aktiviert oder
    deaktiviert wird.
    Ist bOpen == FALSE, wird die Verbindung aller Environment-Kinder
    abgebrochen.
*/
{
    SvEmbeddedClient::Opened( bOpen );
}

#ifdef WAR_NUR_EIN_TEST
//=========================================================================
void SvInPlaceClient::Embedded
(
    BOOL bEmbed	/* TRUE, in den Embed-Status. FALSE, in den Open-Status. */
)
/*	[Beschreibung]

    Dieser Handler wird gerufen, wenn der Embed-Status aktiviert oder
    deaktiviert wird.
    Es wird zuerst die <SvEmbeddedClient::Embedded()> gerufen. Ist
    bEmbed == FALSE, wird die Verbindung aller Environment-Kinder
    abgebrochen.
*/
{
    SvEmbeddedClient::Embedded( bEmbed );
    if( !bEmbed )
    {
        SvContainerEnvironment * pEnv = GetEnv();
        if( pEnv )
            pEnv->ResetChilds();
    }
}
#endif

//=========================================================================
void SvInPlaceClient::InPlaceActivate
(
    BOOL bActivate	/* TRUE, in den InPlace-Status.
                       FALSE, in den Open-Status. */
)
/*	[Beschreibung]

    Dieser Handler wird gerufen, wenn der InPlace-Status aktiviert oder
    deaktiviert wird.
    Ist bActivate == FALSE, wird die Verbindung aller Environment-Kinder
    abgebrochen.
*/
{
    if( !bActivate )
    {
        SvContainerEnvironment * pEnv = GetEnv();
        pEnv->ResetChilds();
    }

    if( !bActivate && HasViewData() )
        FreeViewData( pData );
}

//=========================================================================
void SvInPlaceClient::UIActivate
(
    BOOL bUIActivate	/* TRUE, in den UI-Status.
                           FALSE, in den IP-Status */
)
/*	[Beschreibung]

    Dieser Handler wird gerufen, wenn der UI-Status aktiviert oder
    deaktiviert wird.
*/
{
    if( Owner() )
    {
        if( !bUIActivate )
        {
            SvInPlaceEnvironment * pActEnv = SOAPP->pUIShowIPEnv;
            SvContainerEnvironment * pEnv = GetEnv();
            if( pActEnv )
            {
                if( !pEnv->IsChild( pActEnv->GetContainerEnv() ) )
                { // es wurde kein Child aktiviert
                    pEnv->GetIPEnv()->DoShowIPObj( FALSE );
                }
            }
            else
            {
                pEnv->GetIPEnv()->DoShowIPObj( FALSE );
                if( pEnv->GetParent() )
                    // ein Deactivate ohne ein Activate
                    pEnv->GetParent()->ShowUIByChildDeactivate();
            }

        }
    }
}

//=========================================================================
void SvInPlaceClient::DeactivateAndUndo()
{
}

//=========================================================================
void SvInPlaceClient::DiscardUndoState()
{
}


}
