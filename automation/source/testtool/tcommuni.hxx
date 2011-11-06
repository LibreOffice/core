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



#include <automation/communi.hxx>
#include <tools/link.hxx>
#include <svl/svarray.hxx>

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
    virtual sal_Bool StartCommunication();
    virtual sal_Bool StartCommunication( String aApp, String aParams, Environment *pChildEnv );

    sal_Bool KillApplication();

protected:
    virtual sal_Bool RetryConnect();
    sal_Bool bApplicationStarted;
    Time aFirstRetryCall;
    String aAppPath;
    String aAppParams;
    Environment aAppEnv;
    Process *pProcess;
};


String GetHostConfig();
sal_uLong GetTTPortConfig();
sal_uLong GetUnoPortConfig();

