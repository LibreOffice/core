/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: loadmodule.hxx,v $
 * $Revision: 1.3 $
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

#ifndef INCLUDED_CPPU_SOURCE_UNO_CREATEMODULENAME_HXX
#define INCLUDED_CPPU_SOURCE_UNO_CREATEMODULENAME_HXX

#include "sal/config.h"
#include "osl/module.h"

namespace rtl { class OUString; }

namespace cppu { namespace detail {

/** Load a module.

    @param name
    the nucleus of a module name (without any "lib...so", ".dll", etc.
    decoration, and without a path).

    @return
    the handle returned by osl_loadModule.
*/
::oslModule loadModule(::rtl::OUString const & name);

} }

#endif
