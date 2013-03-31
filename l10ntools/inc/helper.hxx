/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/// Helper methods to work with xml files

#ifndef INCLUDED_L10NTOOLS_SOURCE_HELPER_HXX
#define INCLUDED_L10NTOOLS_SOURCE_HELPER_HXX

#include "sal/config.h"
#include "sal/types.h"

#include <cassert>

#include <libxml/parser.h>

#include <rtl/string.hxx>
#include <rtl/strbuf.hxx>


namespace helper {

OString QuotHTML( const OString &rString );
OString UnQuotHTML( const OString& rString );

bool isWellFormedXML( OString const & text );

//Convert xmlChar* to OString
OString xmlStrToOString( const xmlChar* pString );

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
