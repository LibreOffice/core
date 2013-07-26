/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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
