/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: server.hxx,v $
 * $Revision: 1.6 $
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

#include "editwin.hxx"
#include <automation/communi.hxx>

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
