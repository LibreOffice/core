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


#ifndef _VCL_SM_HXX
#define _VCL_SM_HXX

#include "tools/link.hxx"
#include "unx/salunx.h"
#include <X11/SM/SMlib.h>
#include "salsession.hxx"
#include "vclpluginapi.h"

class SessionManagerClient
{
    static SmcConn          aSmcConnection;
    static ByteString       aClientID;
    static bool         bDocSaveDone;

    static void SaveYourselfProc(       SmcConn connection,
                                        SmPointer client_data,
                                        int save_type,
                                        Bool shutdown,
                                        int interact_style,
                                        Bool fast );
    static void DieProc(                SmcConn connection,
                                        SmPointer client_data );
    static void SaveCompleteProc(       SmcConn connection,
                                        SmPointer client_data );
    static void ShutdownCanceledProc(   SmcConn connection,
                                        SmPointer client_data );
    static void InteractProc(           SmcConn connection,
                                        SmPointer clientData );

    static const ByteString& getPreviousSessionID();

    DECL_STATIC_LINK( SessionManagerClient, ShutDownHdl, void* );
    DECL_STATIC_LINK( SessionManagerClient, ShutDownCancelHdl, void* );
    DECL_STATIC_LINK( SessionManagerClient, SaveYourselfHdl, void* );
    DECL_STATIC_LINK( SessionManagerClient, InteractionHdl, void* );
public:
    static VCLPLUG_GEN_PUBLIC void open(); // needed by other plugins, so export
    static void close();

    static bool checkDocumentsSaved();
    static bool queryInteraction();
    static void saveDone();
    static void interactionDone( bool bCancelShutdown );

    static String getExecName();
    static VCLPLUG_GEN_PUBLIC const ByteString&  getSessionID();
};

class SalFrame;

class IceSalSession : public SalSession
{
public:
    IceSalSession();
    virtual ~IceSalSession();

    virtual void queryInteraction();
    virtual void interactionDone();
    virtual void saveDone();
    virtual bool cancelShutdown();

    static void handleOldX11SaveYourself( SalFrame* pFrame );
};

#endif
