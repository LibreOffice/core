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
#include "preloadservices.hxx"
#include "componentmodule.hxx"
#include "unoautopilot.hxx"
#include "oemwiz.hxx"

// the registration methods
extern "C" void SAL_CALL createRegistryInfo_OEMPreloadDialog()
{
    static ::preload::OMultiInstanceAutoRegistration<
        ::preload::OUnoAutoPilot< ::preload::OEMPreloadDialog, ::preload::OEMPreloadSI >
    > aAutoRegistration;
}
static const char cServiceName[] = "org.openoffice.comp.preload.OEMPreloadWizard";
//.........................................................................
namespace preload
{
//.........................................................................

    using namespace ::com::sun::star::uno;

    //=====================================================================
    //= OEMPreloadSI
    //=====================================================================
    //---------------------------------------------------------------------
    ::rtl::OUString OEMPreloadSI::getImplementationName() const
    {
        return ::rtl::OUString::createFromAscii(cServiceName);
    }

    //---------------------------------------------------------------------
    Sequence< ::rtl::OUString > OEMPreloadSI::getServiceNames() const
    {
        Sequence< ::rtl::OUString > aReturn(1);
        aReturn[0] = ::rtl::OUString::createFromAscii(cServiceName);
        return aReturn;
    }


//.........................................................................
}   // namespace preload
//.........................................................................

