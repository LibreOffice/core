/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_UNOTOOLS_DEFAULTENCODING_HXX
#define INCLUDED_UNOTOOLS_DEFAULTENCODING_HXX

#include <sal/config.h>

#include <unotools/unotoolsdllapi.h>
#include <rtl/ustring.hxx>

/** Determines which locale should be used by default in filters when importing non-unicode strings.
This value should be used when the file does not explicitly define encoding of 8-bit strings. It is
defined by officecfg::Office::Linguistic::General::DefaultLocale, or when it's not set,
officecfg::Setup::L10N::ooSetupSystemLocale, and is typically mapped to a corresponding Windows
codepage by filters.

@return
A BCP-47 language tag string.
*/
UNOTOOLS_DLLPUBLIC OUString utl_getLocaleForGlobalDefaultEncoding();

#endif // INCLUDED_UNOTOOLS_DEFAULTENCODING_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
