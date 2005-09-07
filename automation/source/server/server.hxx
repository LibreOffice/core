/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: server.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:26:41 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SERVER_HXX
#define _SERVER_HXX

/*#include <vos/thread.hxx>
#ifndef _STD_NO_NAMESPACE
namespace vos
{
#endif
    class OAcceptorSocket;
#ifndef _STD_NO_NAMESPACE
}
#endif
*/

#ifndef _EDITWIN_HXX
#include "editwin.hxx"
#endif

#ifndef _COMMUNI_HXX
#include "communi.hxx"
#endif

#define TT_PORT_NOT_INITIALIZED     ULONG(0xFFFFFFFF)   // Eigentlich ja noch mehr, aber soll mal reichen
#define TT_NO_PORT_DEFINED          0

class RemoteControlCommunicationManager : public CommunicationManagerServerViaSocket
{
#if OSL_DEBUG_LEVEL > 1
    EditWindow     *m_pDbgWin;
#endif
    String aOriginalWinCaption;
    String aAdditionalWinCaption;
    BOOL bIsPortValid;
    DECL_LINK( SetWinCaption, Timer* = NULL);
    Timer* pTimer;
    virtual void InfoMsg( InfoString aMsg );
    static ULONG nPortIs;
    static BOOL bQuiet;

public:
#if OSL_DEBUG_LEVEL > 1
    RemoteControlCommunicationManager( EditWindow * pDbgWin );
#else
    RemoteControlCommunicationManager();
#endif
    ~RemoteControlCommunicationManager();

    virtual void ConnectionOpened( CommunicationLink* pCL );
    virtual void ConnectionClosed( CommunicationLink* pCL );

    static ULONG GetPort();
    static USHORT nComm;

#if OSL_DEBUG_LEVEL > 1
//  virtual void DataReceived( CommunicationLink* pCL );
#endif

};

#endif
