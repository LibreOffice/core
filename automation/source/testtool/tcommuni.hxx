/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tcommuni.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:33:55 $
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

#include "communi.hxx"
#include <tools/link.hxx>
#include <svtools/svarray.hxx>

#include <vos/thread.hxx>

#ifndef _STRING_LIST
DECLARE_LIST( StringList, String * );
#define _STRING_LIST
#endif

#ifndef _PROCESS_HXX
#include <basic/process.hxx>
#endif

class Process;

class CommunicationManagerClientViaSocketTT : public CommunicationManagerClientViaSocket
{
public:
    CommunicationManagerClientViaSocketTT();

    virtual BOOL StartCommunication();
    virtual BOOL StartCommunication( String aApp, String aParams, Environment *pChildEnv );

    BOOL KillApplication();

protected:
    virtual BOOL RetryConnect();
    BOOL bApplicationStarted;
    Time aFirstRetryCall;
    String aAppPath;
    String aAppParams;
    Environment aAppEnv;
    Process *pProcess;
};


String GetHostConfig();
ULONG GetTTPortConfig();
ULONG GetUnoPortConfig();

