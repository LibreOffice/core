/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: wizardservices.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 13:00:52 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#ifndef _EXTENSIONS_DBP_WIZARDSERVICES_HXX_
#include "wizardservices.hxx"
#endif
#ifndef _EXTENSIONS_DBP_UNOAUTOPILOT_HXX_
#include "unoautopilot.hxx"
#endif
#ifndef _EXTENSIONS_DBP_GROUPBOXWIZ_HXX_
#include "groupboxwiz.hxx"
#endif
#ifndef _EXTENSIONS_DBP_LISTCOMBOWIZARD_HXX_
#include "listcombowizard.hxx"
#endif
#ifndef _EXTENSIONS_DBP_GRIDWIZARD_HXX_
#include "gridwizard.hxx"
#endif

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

