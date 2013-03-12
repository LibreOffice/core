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

#ifndef _DESKTOP_EXITHELPER_H_
#define _DESKTOP_EXITHELPER_H_

enum EExitCodes {
    /* e.g. used to force showing of the command line help */
    EXITHELPER_NO_ERROR = 0,
    /* pipe was detected - second office must terminate itself */
    EXITHELPER_SECOND_OFFICE = 1,
    /* an uno exception was catched during startup */
    EXITHELPER_FATAL_ERROR = 333,    /* Only the low 8 bits are significant 333 % 256 = 77 */
    /* user force automatic restart after crash */
    EXITHELPER_CRASH_WITH_RESTART = 79,
    /* the office restarts itself */
    EXITHELPER_NORMAL_RESTART = 81
};

#endif /* #ifndef _DESKTOP_EXITHELPER_H_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
