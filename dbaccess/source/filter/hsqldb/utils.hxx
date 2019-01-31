/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_DBACCESS_SOURCE_FILTER_HSQLDB_UTILS_HXX
#define INCLUDED_DBACCESS_SOURCE_FILTER_HSQLDB_UTILS_HXX

#include <rtl/ustring.hxx>

namespace dbahsql
{
namespace utils
{
OUString convertToUTF8(const OString& original);

OUString getTableNameFromStmt(const OUString& sSql);

void ensureFirebirdTableLength(const OUString& sName);
}
}

#endif // INCLUDED_DBACCESS_SOURCE_FILTER_HSQLDB_UTILS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
