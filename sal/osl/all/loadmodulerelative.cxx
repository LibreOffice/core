/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: loadmodulerelative.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-10-15 12:47:55 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
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
#include "precompiled_sal.hxx"

#include "sal/config.h"

#include <cstddef>

#include "osl/diagnose.h"
#include "osl/module.h"
#include "osl/module.hxx"
#include "osl/thread.h"
#include "rtl/malformeduriexception.hxx"
#include "rtl/uri.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

extern "C" {

oslModule SAL_CALL osl_loadModuleRelative(
    oslGenericFunction const baseModule, rtl_uString * const relativePath,
    sal_Int32 const mode)
{
    ::rtl::OUString base;
    if (!::osl::Module::getUrlFromAddress(baseModule, base)) {
        OSL_TRACE("osl::Module::getUrlFromAddress failed");
        return NULL;
    }
    ::rtl::OUString abs;
    try {
        abs = ::rtl::Uri::convertRelToAbs(base, relativePath);
    } catch (::rtl::MalformedUriException & e) {
        (void) e; // avoid warnings
        OSL_TRACE(
            "rtl::MalformedUriException <%s>",
            rtl::OUStringToOString(e.getMessage(), osl_getThreadTextEncoding()).
                getStr());
            //TODO: let some OSL_TRACE variant take care of text conversion?
        return NULL;
    }
    return ::osl_loadModule(abs.pData, mode);
}

}
