/*************************************************************************
 *
 *  $RCSfile: commtest.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:59:03 $
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

#ifndef _SV_SVAPP_HXX //autogen wg. Application
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_WRKWIN_HXX //autogen wg. WorkWindow
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SV_TOOLBOX_HXX //autogen wg. ToolBox
#include <vcl/toolbox.hxx>
#endif


#ifndef _SIMPLECM_HXX //autogen wg. CommunicationManager
#include <tools/simplecm.hxx>
#endif
#ifndef _COMMUNI_HXX //autogen
#include "communi.hxx"
#endif
#include "brooker.hxx"
//#include <tools/bcst.hxx>

#include "commtest.hrc"



#define TCP_PORT 17612

#define CUniString( constAsciiStr ) UniString( RTL_CONSTASCII_USTRINGPARAM ( constAsciiStr ) )


class PacketSender : public Timer
{
    SvStream* mpData;
    CommunicationLinkRef mxCL;

public:
    PacketSender( ULONG nDelay, SvStream* pData, CommunicationLink* pCL );
    virtual void    Timeout();
};

PacketSender::PacketSender( ULONG nDelay, SvStream* pData, CommunicationLink* pCL )
: mpData( pData )
, mxCL( pCL )
{
    SetTimeout( nDelay );
    Start();
}

void PacketSender::Timeout()
{
    mxCL->TransferDataStream( mpData );
    delete mpData;
    delete this;
}



class DelayedDeleter : public Timer
{
    CommunicationManager *mpManager;

public:
    DelayedDeleter( ULONG nDelay, CommunicationManager *pManager );
    virtual void    Timeout();
};

DelayedDeleter::DelayedDeleter( ULONG nDelay, CommunicationManager *pManager )
: mpManager( pManager )
{
    SetTimeout( nDelay );
    Start();
}

void DelayedDeleter::Timeout()
{
    delete mpManager;
    delete this;
}




class CommunicationTester : public Application
{
    DECL_LINK( TBClick, ToolBox* );
    DECL_LINK( DataReceived, CommunicationLink* );
    DECL_LINK( ConnectionOpened, CommunicationLink* );
    DECL_LINK( ConnectionClosed, CommunicationLink* );


    CommunicationManager *pClientTcp, *pServerTcp;
    InformationBroadcaster *pBCSTSend;
    InformationBroadcaster *pBCSTListen;
    InformationBrooker *pBCSTBrooker;
public:
    CommunicationTester();

    virtual void Main();
};

CommunicationTester IchSelber;

CommunicationTester::CommunicationTester()
: pBCSTSend( NULL )
, pBCSTListen( NULL )
, pClientTcp( NULL )
, pBCSTBrooker( NULL )
, pServerTcp( NULL )
{}

void CommunicationTester::Main()
{
    ResMgr *pRes = new ResMgr( CUniString("commtest.res") );
    Resource::SetResManager( pRes );
    WorkWindow aWW( NULL, WB_APP | WB_STDWORK );
    aWW.Show();
    ToolBox aTB( &aWW, ResId( TBMenu ) );
    aTB.Show();
    aTB.RecalcItems();
    aTB.SetFloatingMode( TRUE );
    aTB.SetFloatingMode( FALSE );
    aTB.SetClickHdl( LINK( this, CommunicationTester, TBClick ) );

    Execute();
}

#define SWITCH( pManager, ManagerClass )                    \
{                                                           \
    if ( pManager )                                         \
    {                                                       \
        pManager->StopCommunication();                      \
        new DelayedDeleter( 1000, pManager );               \
        pTB->SetItemState( pTB->GetCurItemId(), STATE_NOCHECK );    \
        pManager = NULL;                                    \
    }                                                       \
    else                                                    \
    {                                                       \
        pManager = new ManagerClass;                        \
        pManager->SetConnectionOpenedHdl( LINK( this, CommunicationTester, ConnectionOpened ) );\
        pManager->SetConnectionClosedHdl( LINK( this, CommunicationTester, ConnectionClosed ) );\
        pManager->SetDataReceivedHdl( LINK( this, CommunicationTester, DataReceived ) );\
        pTB->SetItemState( pTB->GetCurItemId(), STATE_CHECK );  \
    }                                                       \
}


IMPL_LINK( CommunicationTester, TBClick, ToolBox*, pTB )
{
    switch ( pTB->GetCurItemId() )
    {
        case SERVER_TCP:
            {
                SWITCH( pServerTcp, CommunicationManagerServerViaSocket( TCP_PORT, (USHORT) 32000 ) );
                if ( pServerTcp )
                    pServerTcp->StartCommunication();   // Am Port horchen
            }
            break;
        case CLIENT_TCP:
            {
                SWITCH( pClientTcp, CommunicationManagerClientViaSocket( "localhost", TCP_PORT ) );
                if ( pClientTcp )
                    pClientTcp->StartCommunication();   // Eine Verbindung aufbauen
            }
            break;
        case BCST_BROOKER:
            {
                if ( pBCSTBrooker )
                {
                    delete pBCSTBrooker;
                    pBCSTBrooker = NULL;
                }
                else
                {
                    pBCSTBrooker = new InformationBrooker();
                }
            }
            break;
        case BCST_LISTEN:
            {
                if ( pBCSTListen )
                {
                    delete pBCSTListen;
                    pBCSTListen = NULL;
                }
                else
                {
                    pBCSTListen = new InformationBroadcaster();
                }
            }
        case BCST_SEND:
            {
                if ( pBCSTSend )
                {
                    pBCSTSend->Broadcast( BCST_CAT_PL2X,      "Message: BCST_CAT_PL2X" );
                    pBCSTSend->Broadcast( BCST_CAT_MINORCOPY, "Message: BCST_CAT_MINORCOPY" );
                    pBCSTSend->Broadcast( BCST_CAT_DELIVER,   "Message: BCST_CAT_DELIVER" );
                    pBCSTSend->Broadcast( BCST_CAT_ALL,       "Message: BCST_CAT_ALL" );
                }
                else
                {
                    pBCSTSend = new InformationBroadcaster();
                }
            }
            break;
    }
    return 0;
}

IMPL_LINK( CommunicationTester, ConnectionOpened, CommunicationLink*, pCL )
{
    SvStream *pData = pCL->GetBestCommunicationStream();
    while ( pData->Tell() < 70 ) *pData << 123;

    pCL->TransferDataStream( pData );
    return 0;
}

IMPL_LINK( CommunicationTester, ConnectionClosed, CommunicationLink*, pCL )
{
    return 0;
}

IMPL_LINK( CommunicationTester, DataReceived, CommunicationLink*, pCL )
{
    // Find Manager
    CommunicationManager* pManager;
    if ( pClientTcp && pClientTcp->IsLinkValid( pCL ) )
    {
        pManager = pClientTcp;
    }
    if ( pServerTcp && pServerTcp->IsLinkValid( pCL ) )
    {
        DBG_ASSERT( !pManager, "CommunicationLink bei mehreren Managern eingetragen");
        pManager = pServerTcp;
    }
    DBG_ASSERT( pCL->GetCommunicationManager() == pManager, "Manager des Link != Manager bei dem der Link Valid ist");

    // Send Data Back (Echo)
    new PacketSender( 1000, pCL->GetServiceData(), pCL );

    return 0;
}

