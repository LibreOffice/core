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
#include <rtl/ustrbuf.hxx>

#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>

#include <vector>

namespace connectivity
{
    namespace firebird
    {
        typedef std::vector< OString > OStringVector;
        // Type Blob has 2 subtypes values
        // 0 for BLOB, 1 for CLOB
        // see http://www.firebirdfaq.org/faq48/
        enum class BlobSubtype {
            Blob = 0,
            Clob = 1
        };

        /**
         * Make sure an identifier is safe to use within the database. Currently
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
         *
         * @throws css::sdbc::SQLException
         */
        void evaluateStatusVector(const ISC_STATUS_ARRAY& rStatusVector,
                                  const ::rtl::OUString& aCause,
                                  const css::uno::Reference< css::uno::XInterface >& _rxContext);

        sal_Int32 getColumnTypeFromFBType(short aType, short aSubType, short aScale);
        ::rtl::OUString getColumnTypeNameFromFBType(short aType, short aSubType, short aScale);

        /**
         * Internally (i.e. in RDB$FIELD_TYPE) firebird stores the data type
         * for a column as defined in blr_*, however in the firebird
         * api the SQL_* types are used, hence we need to be able to convert
         * between the two when retrieving column metadata.
         */
        short getFBTypeFromBlrType(short blrType);

        void mallocSQLVAR(XSQLDA* pSqlda);

        void freeSQLVAR(XSQLDA* pSqlda);

        OUString escapeWith( const OUString& sText, const char aKey, const char aEscapeChar);
        sal_Int64 pow10Integer( int nDecimalCount );
    }
}
#endif // INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_FIREBIRD_UTIL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
