/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CONNECTIVITY_FIREBIRD_WRAPPER_SQLDA_HXX
#define CONNECTIVITY_FIREBIRD_WRAPPER_SQLDA_HXX

#include <ibase.h>

namespace connectivity
{
    namespace firebird
    {
        namespace wrapper
        {
           /*
            * Default sqlvar length that we allocate.
            */
            static const unsigned int DEFAULT_SQLDA_SIZE = 10;

            class Sqlda
            {
            private:
                XSQLDA* mpSqlda;

            public:
                Sqlda();
                ~Sqlda();
                XSQLDA* operator&() { return mpSqlda; };

                /**
                 * Set up the Sqlda for a given statement, is equivalent to
                 * using isc_dsql_describe, but with all the details handled
                 * within.
                 *
                 * Use bDescribeBind if we are binding an input sqlda, i.e.
                 * if we want isc_dsql_describe_bind instead of isc_dsql_describe.
                 */
                void describeStatement(
                    isc_stmt_handle& aStatementHandle,
                    bool bDescribeBind = false);
            };
        }
    }
}

#endif // CONNECTIVITY_FIREBIRD_WRAPPER_SQLDA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */