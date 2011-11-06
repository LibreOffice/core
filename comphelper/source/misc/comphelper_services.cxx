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
#include "precompiled_comphelper.hxx"

#include "comphelper_module.hxx"

//--------------------------------------------------------------------
extern void createRegistryInfo_OPropertyBag();
extern void createRegistryInfo_SequenceOutputStream();
extern void createRegistryInfo_SequenceInputStream();
extern void createRegistryInfo_UNOMemoryStream();
extern void createRegistryInfo_IndexedPropertyValuesContainer();
extern void createRegistryInfo_NamedPropertyValuesContainer();
extern void createRegistryInfo_AnyCompareFactory();
extern void createRegistryInfo_OfficeInstallationDirectories();
extern void createRegistryInfo_OInstanceLocker();
extern void createRegistryInfo_Map();
extern void createRegistryInfo_OSimpleLogRing();
extern void createRegistryInfo_OOfficeRestartManager();

//........................................................................
namespace comphelper { namespace module
{
//........................................................................

    static void initializeModule()
    {
        static bool bInitialized( false );
        if ( !bInitialized )
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if ( !bInitialized )
            {
                createRegistryInfo_OPropertyBag();
                createRegistryInfo_SequenceOutputStream();
                createRegistryInfo_SequenceInputStream();
                createRegistryInfo_UNOMemoryStream();
                createRegistryInfo_IndexedPropertyValuesContainer();
                createRegistryInfo_NamedPropertyValuesContainer();
                createRegistryInfo_AnyCompareFactory();
                createRegistryInfo_OfficeInstallationDirectories();
                createRegistryInfo_OInstanceLocker();
                createRegistryInfo_Map();
                createRegistryInfo_OSimpleLogRing();
                createRegistryInfo_OOfficeRestartManager();
            }
        }
    }

//........................................................................
} } // namespace comphelper::module
//........................................................................

IMPLEMENT_COMPONENT_LIBRARY_API( ::comphelper::module::ComphelperModule, ::comphelper::module::initializeModule )
