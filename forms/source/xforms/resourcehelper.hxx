/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: resourcehelper.hxx,v $
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
#ifndef _RESOURCEHELPER_HXX
#define _RESOURCEHELPER_HXX

// include resource IDs
#include <frm_resource.hrc>

#include <sal/types.h>

namespace rtl { class OUString; }

namespace xforms
{
    /// get a resource string for the current language
    rtl::OUString getResource( sal_uInt16 );

    // overloaded: get a resource string, and substitute parameters
    rtl::OUString getResource( sal_uInt16, const rtl::OUString& );
    rtl::OUString getResource( sal_uInt16, const rtl::OUString&,
                                           const rtl::OUString& );
    rtl::OUString getResource( sal_uInt16, const rtl::OUString&,
                                           const rtl::OUString&,
                                           const rtl::OUString& );

} // namespace

#endif
