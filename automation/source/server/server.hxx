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

#define TT_PORT_NOT_INITIALIZED     sal_uLong(0xFFFFFFFF)   // Eigentlich ja noch mehr, aber soll mal reichen
#define TT_NO_PORT_DEFINED          0

class RemoteControlCommunicationManager : public CommunicationManagerServerViaSocket
{
#if OSL_DEBUG_LEVEL > 1
    EditWindow     *m_pDbgWin;
#endif
    String aOriginalWinCaption;
    String aAdditionalWinCaption;
    sal_Bool bIsPortValid;
    DECL_LINK( SetWinCaption, Timer* = NULL);
    Timer* pTimer;
    virtual void InfoMsg( InfoString aMsg );
    static sal_uLong nPortIs;
    static sal_Bool bQuiet;

public:
#if OSL_DEBUG_LEVEL > 1
    RemoteControlCommunicationManager( EditWindow * pDbgWin );
#else
    RemoteControlCommunicationManager();
#endif
    ~RemoteControlCommunicationManager();

    virtual void ConnectionOpened( CommunicationLink* pCL );
    virtual void ConnectionClosed( CommunicationLink* pCL );

    static sal_uLong GetPort();
    static sal_uInt16 nComm;

#if OSL_DEBUG_LEVEL > 1
//  virtual void DataReceived( CommunicationLink* pCL );
#endif

};

#endif
