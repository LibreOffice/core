/*************************************************************************
 *
 *  $RCSfile: server.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-15 15:53:49 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2002
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2002 by Sun Microsystems, Inc.
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
#include <svtools/communi.hxx>
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
