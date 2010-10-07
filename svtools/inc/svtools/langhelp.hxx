/*************************************************************************
 *
 * Copyright 2010 Novell, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * or later, as published by the Free Software Foundation.
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

#ifndef _SVTOOLS_LANGHELP_HXX_
#define _SVTOOLS_LANGHELP_HXX_

#include <svtools/svtdllapi.h>

namespace rtl { class OUString; }

/** Localize a URI to one of the foundation's webservices

    @param io_rURI
    URI to localize - depending on your UI locale, a country local
    part is appended to the URI (like "en", or "fr", or "es")
 */
SVT_DLLPUBLIC void localizeWebserviceURI( ::rtl::OUString& io_rURI );

#endif

