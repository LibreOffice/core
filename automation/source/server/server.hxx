/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
