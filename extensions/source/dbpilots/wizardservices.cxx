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

#include "sal/config.h"

#include "dbpservices.hxx"
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


namespace dbp
{


    using namespace ::com::sun::star::uno;

    OUString OGroupBoxSI::getImplementationName()
    {
        return OUString("org.openoffice.comp.dbp.OGroupBoxWizard");
    }


    Sequence< OUString > OGroupBoxSI::getServiceNames()
    {
        Sequence< OUString > aReturn { "com.sun.star.sdb.GroupBoxAutoPilot" };
        return aReturn;
    }

    OUString OListComboSI::getImplementationName()
    {
        return OUString("org.openoffice.comp.dbp.OListComboWizard");
    }


    Sequence< OUString > OListComboSI::getServiceNames()
    {
        Sequence< OUString > aReturn { "com.sun.star.sdb.ListComboBoxAutoPilot" };
        return aReturn;
    }

    OUString OGridSI::getImplementationName()
    {
        return OUString("org.openoffice.comp.dbp.OGridWizard");
    }


    Sequence< OUString > OGridSI::getServiceNames()
    {
        Sequence< OUString > aReturn { "com.sun.star.sdb.GridControlAutoPilot" };
        return aReturn;
    }


}   // namespace dbp


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
