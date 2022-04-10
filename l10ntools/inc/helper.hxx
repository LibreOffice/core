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

#include <libxml/xmlstring.h>

#include <rtl/string.hxx>


namespace helper {

/// Escape all given character in the text
OString escapeAll(
    std::string_view rText, std::string_view rUnEscaped, std::string_view rEscaped );
/// Unescape all given character in the text
OString unEscapeAll(
    std::string_view rText, std::string_view rEscaped, std::string_view rUnEscaped  );

/// Convert special characters to XML entity references
OString QuotHTML( std::string_view rString );
/// Convert XML entity references to single characters
OString UnQuotHTML( std::string_view rString );

/// Check whether text is a valid XML expression
bool isWellFormedXML( std::string_view text );

/// Convert xmlChar* to OString
OString xmlStrToOString( const xmlChar* pString );

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
