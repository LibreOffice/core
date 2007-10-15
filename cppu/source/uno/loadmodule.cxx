/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: loadmodule.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-10-15 12:19:51 $
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
#include "precompiled_cppu.hxx"

#include "sal/config.h"

#include "osl/module.h"
#include "rtl/string.h"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.hxx"

#include "loadmodule.hxx"

namespace cppu { namespace detail {

::oslModule loadModule(rtl::OUString const & name) {
    rtl::OUStringBuffer b;
#if defined SAL_DLLPREFIX
    b.appendAscii(RTL_CONSTASCII_STRINGPARAM(SAL_DLLPREFIX));
#endif
    b.append(name);
    b.appendAscii(RTL_CONSTASCII_STRINGPARAM(SAL_DLLEXTENSION));
    return ::osl_loadModuleRelative(
        reinterpret_cast< oslGenericFunction >(&loadModule),
        b.makeStringAndClear().pData,
        SAL_LOADMODULE_GLOBAL | SAL_LOADMODULE_LAZY);
}

} }
