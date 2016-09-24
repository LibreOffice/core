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

#include <macros/registration.hxx>

/*=================================================================================================================
    Add new include and new register info to for new services.

    Example:

        #include <service1.hxx>
        #include <service2.hxx>

        COMPONENTGETFACTORY (   IFFACTORIE( Service1 )
                                 else
                                IFFACTORIE( Service2 )
                             )
=================================================================================================================*/
#include <jobs/helponstartup.hxx>
#include <tabwin/tabwinfactory.hxx>
#include <dispatch/systemexec.hxx>
#include <jobs/shelljob.hxx>

COMPONENTGETFACTORY ( fwm,
                        IFFACTORY( ::framework::HelpOnStartup       ) else
                        IFFACTORY( ::framework::TabWinFactory       ) else
                        IFFACTORY( ::framework::SystemExec          ) else
                        IFFACTORY( ::framework::ShellJob            )
                    )

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
