/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: preloadservices.cxx,v $
 * $Revision: 1.4 $
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

