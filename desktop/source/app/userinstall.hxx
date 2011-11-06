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




#include <sal/types.h>
#include <rtl/ustring.hxx>

namespace desktop
{

class UserInstall
{
public:
    enum UserInstallError {
        E_None,                 // no error
        E_Creation,             // error while creating user install
        E_InvalidBaseinstall,   // corrupt base installation
        E_SetupFailed,          // external setup did not run correctly
        E_Configuration,        // error while accessing configuration
        E_License,              // License not accepted
        E_NoDiskSpace,          // not enough disk space
        E_NoWriteAccess,        // no write access
        E_Unknown               // unknown error
    };

    static UserInstallError finalize();
};
}
