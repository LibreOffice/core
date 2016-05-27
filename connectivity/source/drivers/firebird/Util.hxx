/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_FIREBIRD_UTIL_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_FIREBIRD_UTIL_HXX

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
         * 31), whereby the name is simply padded with trailing whitespace.
         * This removes all trailing whitespace (i.e. if necessary so that
         * the length is below 31 characters). Firebird automatically compensates
         * for such shorter strings, however any trailing padding makes the gui
         * editing of such names harder, hence we remove all trailing whitespace.
         */
        OUString sanitizeIdentifier(const OUString& rIdentifier);

        inline bool IndicatesError(const ISC_STATUS_ARRAY& rStatusVector)
        {
            return rStatusVector[0]==1 && rStatusVector[1]; // indicates error;
        }

        OUString StatusVectorToString(const ISC_STATUS_ARRAY& rStatusVector,
                                    const OUString& rCause);

        /**
         * Evaluate a firebird status vector and throw exceptions as necessary.
         * The content of the status vector is included in the thrown exception.
         */
        void evaluateStatusVector(const ISC_STATUS_ARRAY& rStatusVector,
                                  const ::rtl::OUString& aCause,
                                  const css::uno::Reference< css::uno::XInterface >& _rxContext)
                throw (css::sdbc::SQLException);

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

        void freeSQLVAR(XSQLDA* pSqlda);
    }
}
#endif // INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_FIREBIRD_UTIL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
