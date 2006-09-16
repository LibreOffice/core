/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: preloadservices.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 13:12:27 $
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

#ifndef _EXTENSIONS_PRELOAD_PRELOADSERVICES_HXX_
#include "preloadservices.hxx"
#endif
#ifndef _EXTENSIONS_COMPONENT_MODULE_HXX_
#include "componentmodule.hxx"
#endif
#ifndef _EXTENSIONS_PRELOAD_UNOAUTOPILOT_HXX_
#include "unoautopilot.hxx"
#endif
#ifndef _EXTENSIONS_PRELOAD_OEMWIZ_HXX_
#include "oemwiz.hxx"
#endif

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

