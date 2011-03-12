/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
    //-------------------------------------------------------------------------
    // const
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
