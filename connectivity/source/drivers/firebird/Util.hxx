/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CONNECTIVITY_FIREBIRD_UTIL_HXX
#define CONNECTIVITY_FIREBIRD_UTIL_HXX

#include <ibase.h>

#include <rtl/ustring.hxx>

#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>

namespace connectivity
{
    namespace firebird
    {

        /**
         * Make sure an identifier is safe to use within the databse. Currently
         * firebird seems to return identifiers with 93 character (instead of
         * 31). Use this to avoid issues when using the identifier in other
         * sql queries.
         */
        void sanitizeIdentifier(::rtl::OUString& rIdentifier);

        /**
         * Evaluate a firebird status vector and throw exceptions as necessary.
         * The content of the status vector is included in the thrown exception.
         */
        void evaluateStatusVector(ISC_STATUS_ARRAY& aStatusVector,
                                  const ::rtl::OUString& aCause,
                                  const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxContext)
                throw (::com::sun::star::sdbc::SQLException);

        sal_Int32 getColumnTypeFromFBType(short aType);
        ::rtl::OUString getColumnTypeNameFromFBType(short aType);

        /**
         * Internally (i.e. in RDB$FIELD_TYPE) firebird stores the data type
         * for a column as defined in blr_*, however in the firebird
         * api the SQL_* types are used, hence we need to be able to convert
         * between the two when retrieving column metadata.
         */
        short getFBTypeFromBlrType(short blrType);

        void mallocSQLVAR(XSQLDA* pSqlda);

//         void freeSQLVAR(XSQLDA* pSqlda);
    }
}
#endif //CONNECTIVITY_FIREBIRD_UTIL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */