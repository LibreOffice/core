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
