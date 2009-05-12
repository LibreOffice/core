/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: tcommuni.hxx,v $
 * $Revision: 1.10 $
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

#include <automation/communi.hxx>
#include <tools/link.hxx>
#include <svtools/svarray.hxx>

#include <vos/thread.hxx>

#ifndef _STRING_LIST
DECLARE_LIST( StringList, String * )
#define _STRING_LIST
#endif
#include <basic/process.hxx>

class Process;

class CommunicationManagerClientViaSocketTT : public CommunicationManagerClientViaSocket
{
public:
    CommunicationManagerClientViaSocketTT();

    using CommunicationManagerClientViaSocket::StartCommunication;
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

