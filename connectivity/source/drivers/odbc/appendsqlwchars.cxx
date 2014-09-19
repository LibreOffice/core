/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <boost/static_assert.hpp>
#include <rtl/ustrbuf.hxx>
#include <sal/types.h>

#include <appendsqlwchars.hxx>

namespace connectivity { namespace odbc {

void appendSQLWCHARs(OUStringBuffer & s, const sal_Unicode* d, sal_Int32 n)
{
    s.append(d, n);
}

#if defined SAL_UNICODE_NOTEQUAL_WCHAR_T
BOOST_STATIC_ASSERT(sizeof (wchar_t) == 4);
void appendSQLWCHARs(OUStringBuffer & s, const wchar_t* d, sal_Int32 n)
{
    const wchar_t * const end = d + n;
    for (; d < end; ++d)
    {
        s.appendUtf32(*d);
    }
}
#endif

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
