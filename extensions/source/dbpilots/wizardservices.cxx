/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: wizardservices.cxx,v $
 * $Revision: 1.7 $
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

