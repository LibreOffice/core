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

#ifndef INCLUDED_SYSTOOLS_WIN32_QSWIN32_H
#define INCLUDED_SYSTOOLS_WIN32_QSWIN32_H

#define QUICKSTART_CLASSNAMEA           "LO Listener Class"
#define QUICKSTART_WINDOWNAMEA          "LO Listener Window"
#define SHUTDOWN_QUICKSTART_MESSAGEA    "LO KillTray"

#define QUICKSTART_CLASSNAMEW           L##QUICKSTART_CLASSNAMEA
#define QUICKSTART_WINDOWNAMEW          L##QUICKSTART_WINDOWNAMEA
#define SHUTDOWN_QUICKSTART_MESSAGEW    L##SHUTDOWN_QUICKSTART_MESSAGEA

#ifdef UNICODE
#   define QUICKSTART_CLASSNAME             QUICKSTART_CLASSNAMEW
#   define QUICKSTART_WINDOWNAME            QUICKSTART_WINDOWNAMEW
#   define SHUTDOWN_QUICKSTART_MESSAGE      SHUTDOWN_QUICKSTART_MESSAGEW
#   ifndef FindWindow
#       define FindWindow   FindWindowW
#   endif
#else
#   define QUICKSTART_CLASSNAME             QUICKSTART_CLASSNAMEA
#   define QUICKSTART_WINDOWNAME            QUICKSTART_WINDOWNAMEA
#   define SHUTDOWN_QUICKSTART_MESSAGE      SHUTDOWN_QUICKSTART_MESSAGEA
#   ifndef FindWindow
#       define FindWindow   FindWindowA
#   endif
#endif

#endif // INCLUDED_SYSTOOLS_WIN32_QSWIN32_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
