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

#ifndef _EXTENSIONS_DBP_WIZARDSERVICES_HXX_
#include "wizardservices.hxx"
#endif
#include "unoautopilot.hxx"
#include "groupboxwiz.hxx"
#include "listcombowizard.hxx"
#include "gridwizard.hxx"

// the registration methods
extern "C" void SAL_CALL createRegistryInfo_OGroupBoxWizard()
{
    static ::dbp::OMultiInstanceAutoRegistration<
        ::dbp::OUnoAutoPilot< ::dbp::OGroupBoxWizard, ::dbp::OGroupBoxSI >
    > aAutoRegistration;
}

extern "C" void SAL_CALL createRegistryInfo_OListComboWizard()
{
    static ::dbp::OMultiInstanceAutoRegistration<
        ::dbp::OUnoAutoPilot< ::dbp::OListComboWizard, ::dbp::OListComboSI >
    > aAutoRegistration;
}

extern "C" void SAL_CALL createRegistryInfo_OGridWizard()
{
    static ::dbp::OMultiInstanceAutoRegistration<
        ::dbp::OUnoAutoPilot< ::dbp::OGridWizard, ::dbp::OGridSI >
    > aAutoRegistration;
}

//.........................................................................
namespace dbp
{
//.........................................................................

    using namespace ::com::sun::star::uno;

    //=====================================================================
    //= OGroupBoxSI
    //=====================================================================
    //---------------------------------------------------------------------
    ::rtl::OUString OGroupBoxSI::getImplementationName() const
    {
        return ::rtl::OUString::createFromAscii("org.openoffice.comp.dbp.OGroupBoxWizard");
    }

    //---------------------------------------------------------------------
    Sequence< ::rtl::OUString > OGroupBoxSI::getServiceNames() const
    {
        Sequence< ::rtl::OUString > aReturn(1);
        aReturn[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdb.GroupBoxAutoPilot");
        return aReturn;
    }

    //=====================================================================
    //= OListComboSI
    //=====================================================================
    //---------------------------------------------------------------------
    ::rtl::OUString OListComboSI::getImplementationName() const
    {
        return ::rtl::OUString::createFromAscii("org.openoffice.comp.dbp.OListComboWizard");
    }

    //---------------------------------------------------------------------
    Sequence< ::rtl::OUString > OListComboSI::getServiceNames() const
    {
        Sequence< ::rtl::OUString > aReturn(1);
        aReturn[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdb.ListComboBoxAutoPilot");
        return aReturn;
    }

    //=====================================================================
    //= OGridSI
    //=====================================================================
    //---------------------------------------------------------------------
    ::rtl::OUString OGridSI::getImplementationName() const
    {
        return ::rtl::OUString::createFromAscii("org.openoffice.comp.dbp.OGridWizard");
    }

    //---------------------------------------------------------------------
    Sequence< ::rtl::OUString > OGridSI::getServiceNames() const
    {
        Sequence< ::rtl::OUString > aReturn(1);
        aReturn[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdb.GridControlAutoPilot");
        return aReturn;
    }

//.........................................................................
}   // namespace dbp
//.........................................................................

