/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Helper string methods

#ifndef INCLUDED_L10NTOOLS_INC_HELPER_HXX
#define INCLUDED_L10NTOOLS_INC_HELPER_HXX

#include <sal/config.h>

#include <string_view>

#include <sal/types.h>

#include <libxml/parser.h>

#include <rtl/string.hxx>
#include <rtl/strbuf.hxx>


namespace helper {

/// Escape all given character in the text
OString escapeAll(
    const OString& rText, const OString& rUnEscaped, const OString& rEscaped );
/// Unescape all given character in the text
OString unEscapeAll(
    const OString& rText, const OString& rEscaped, std::string_view rUnEscaped  );

/// Convert special characters to XML entity references
OString QuotHTML( const OString &rString );
/// Convert XML entity references to single characters
OString UnQuotHTML( const OString& rString );

/// Check whether text is a valid XML expression
bool isWellFormedXML( std::string_view text );

/// Convert xmlChar* to OString
OString xmlStrToOString( const xmlChar* pString );

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
