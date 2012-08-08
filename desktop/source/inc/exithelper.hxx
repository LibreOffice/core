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

#ifndef _DESKTOP_EXITHELPER_HXX_
#define _DESKTOP_EXITHELPER_HXX_

namespace desktop
{

//=============================================================================
/** @short  provide helper functions to handle a abnormal exit
            and contain a list of all "well known" exit codes.
 */
class ExitHelper
{
    public:

        //---------------------------------------------------------------------
        /** @short  list of all well known exit codes.

            @descr  Its not allowed to use exit codes hard coded
                    inside office. All places must use these list to
                    be synchron.
         */
        enum EExitCodes
        {
            /// e.g. used to force showing of the command line help
            E_NO_ERROR = 0,
            /// pipe was detected - second office must terminate itself
            E_SECOND_OFFICE = 1,
            /// an uno exception was catched during startup
            E_FATAL_ERROR = 333,    // Only the low 8 bits are significant 333 % 256 = 77
            /// user force automatic restart after crash
            E_CRASH_WITH_RESTART = 79,
            /// the office restarts itself
            E_NORMAL_RESTART = 81
        };
};

} // namespace desktop

#endif // #ifndef _DESKTOP_EXITHELPER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
