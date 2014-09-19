/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_ODBC_APPENDSQLWCHARS_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_ODBC_APPENDSQLWCHARS_HXX

#include <sal/config.h>

#include <rtl/ustrbuf.hxx>
#include <sal/types.h>

namespace connectivity { namespace odbc {

void appendSQLWCHARs(OUStringBuffer & s, const sal_Unicode* d, sal_Int32 n);

#if defined SAL_UNICODE_NOTEQUAL_WCHAR_T
void appendSQLWCHARs(OUStringBuffer & s, const wchar_t* d, sal_Int32 n);
#endif

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
