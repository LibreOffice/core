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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/
#include "log_module.hxx"

//........................................................................
namespace logging
{
//........................................................................

    //--------------------------------------------------------------------
    extern void createRegistryInfo_LoggerPool();
    extern void createRegistryInfo_FileHandler();
    extern void createRegistryInfo_ConsoleHandler();
    extern void createRegistryInfo_PlainTextFormatter();
    extern void createRegistryInfo_CsvFormatter();

    static void initializeModule()
    {
        static bool bInitialized( false );
        if ( !bInitialized )
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if ( !bInitialized )
            {
                createRegistryInfo_LoggerPool();
                createRegistryInfo_FileHandler();
                createRegistryInfo_ConsoleHandler();
                createRegistryInfo_PlainTextFormatter();
                createRegistryInfo_CsvFormatter();
            }
        }
    }

//........................................................................
} // namespace logging
//........................................................................

IMPLEMENT_COMPONENT_LIBRARY_API( ::logging::LogModule, ::logging::initializeModule )

