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


#include "wizardservices.hxx"
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
        return ::rtl::OUString("org.openoffice.comp.dbp.OGroupBoxWizard");
    }

    //---------------------------------------------------------------------
    Sequence< ::rtl::OUString > OGroupBoxSI::getServiceNames() const
    {
        Sequence< ::rtl::OUString > aReturn(1);
        aReturn[0] = ::rtl::OUString("com.sun.star.sdb.GroupBoxAutoPilot");
        return aReturn;
    }

    //=====================================================================
    //= OListComboSI
    //=====================================================================
    //---------------------------------------------------------------------
    ::rtl::OUString OListComboSI::getImplementationName() const
    {
        return ::rtl::OUString("org.openoffice.comp.dbp.OListComboWizard");
    }

    //---------------------------------------------------------------------
    Sequence< ::rtl::OUString > OListComboSI::getServiceNames() const
    {
        Sequence< ::rtl::OUString > aReturn(1);
        aReturn[0] = ::rtl::OUString("com.sun.star.sdb.ListComboBoxAutoPilot");
        return aReturn;
    }

    //=====================================================================
    //= OGridSI
    //=====================================================================
    //---------------------------------------------------------------------
    ::rtl::OUString OGridSI::getImplementationName() const
    {
        return ::rtl::OUString("org.openoffice.comp.dbp.OGridWizard");
    }

    //---------------------------------------------------------------------
    Sequence< ::rtl::OUString > OGridSI::getServiceNames() const
    {
        Sequence< ::rtl::OUString > aReturn(1);
        aReturn[0] = ::rtl::OUString("com.sun.star.sdb.GridControlAutoPilot");
        return aReturn;
    }

//.........................................................................
}   // namespace dbp
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
