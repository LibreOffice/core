/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Thorsten Behrens <tbehrens@novell.com>
 *
 * Contributor(s): Thorsten Behrens <tbehrens@novell.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#ifndef _SVTOOLS_LANGHELP_HXX_
#define _SVTOOLS_LANGHELP_HXX_

#include <svtools/svtdllapi.h>

#include <rtl/ustring.hxx>

/** Localize a URI to one of the foundation's webservices

    @param io_rURI
    URI to localize - depending on your UI locale, a country local
    part is appended to the URI (like "en", or "fr", or "es")
 */
SVT_DLLPUBLIC void localizeWebserviceURI( OUString& io_rURI );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
