/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_VCL_INC_UNX_SM_HXX
#define INCLUDED_VCL_INC_UNX_SM_HXX

#include "sal/config.h"

#include <X11/SM/SMlib.h>

#include "tools/link.hxx"
#include <rtl/ustring.hxx>
#include <memory>

#include "vclpluginapi.h"

class ICEConnectionObserver;
class SalSession;

class SessionManagerClient
{
    static SalSession * m_pSession;
    static std::unique_ptr< ICEConnectionObserver > m_xICEConnectionObserver;
    static SmcConn m_pSmcConnection;
    static OString m_aClientID;
    static OString m_aTimeID;
    static bool m_bDocSaveDone;

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

    static OString getPreviousSessionID();

    DECL_STATIC_LINK( SessionManagerClient, ShutDownHdl, void*, void );
    DECL_STATIC_LINK( SessionManagerClient, ShutDownCancelHdl, void*, void );
    DECL_STATIC_LINK( SessionManagerClient, SaveYourselfHdl, void*, void );
    DECL_STATIC_LINK( SessionManagerClient, InteractionHdl, void*, void );
public:
    static void open(SalSession * pSession);
    static void close();

    static bool checkDocumentsSaved();
    static bool queryInteraction();
    static void saveDone();
    static void interactionDone( bool bCancelShutdown );

    static OUString getExecName();
    static VCLPLUG_GEN_PUBLIC const OString& getSessionID();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
