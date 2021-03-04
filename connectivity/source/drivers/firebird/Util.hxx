/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <ibase.h>

#include <rtl/ustring.hxx>

#include <com/sun/star/uno/XInterface.hpp>

namespace connectivity::firebird
    {
        // Type Blob has 2 subtypes values
        // 0 for BLOB, 1 for CLOB
        // see http://www.firebirdfaq.org/faq48/
        // User-defined subtypes are negative.
        // Use a number for image which is very unlikely to be defined by a
        // user.
        enum class BlobSubtype {
            Blob = 0,
            Clob = 1,
            Image = -9546
        };

        /**
         * Numeric and decimal types can be identified by their subtype in
         * Firebird API. 1 for NUMERIC, 2 for DECIMAL.
         */
        enum class NumberSubType {
            Other = 0,
            Numeric = 1,
            Decimal = 2
        };

        class ColumnTypeInfo {
private:
            short m_aType;
            short m_aSubType;
            short m_nScale;
            OUString m_sCharsetName;
public:
            /**
             * @param tType SQL type of column defined by Firebird (e.g.
             * SQL_DOUBLE)
             * @param aSubType SQL sub type as in firebird API. See
             * NumberSubType.
             * @param scale: Scale of the number. It is ignored in case it's not
             * a number. Scale obtained from the Firebird API is negative, so
             * that should be negated before passing to this constructor.
             *
             */
            explicit ColumnTypeInfo( short aType, short aSubType = 0,
                    short nScale = 0, const OUString& sCharset = OUString() )
                : m_aType(aType)
                , m_aSubType(aSubType)
                , m_nScale(nScale)
                , m_sCharsetName(sCharset) {}
            explicit ColumnTypeInfo( short aType, const OUString& sCharset )
                : m_aType(aType)
                , m_aSubType(0)
                , m_nScale(0)
                , m_sCharsetName(sCharset) {}
            short getType() const { return m_aType; }
            short getSubType() const { return m_aSubType; }
            short getScale() const { return m_nScale; }
            OUString const & getCharacterSet() const { return m_sCharsetName; }

            sal_Int32 getSdbcType() const;
            OUString getColumnTypeName() const;

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
                                    std::u16string_view rCause);

        /**
         * Evaluate a firebird status vector and throw exceptions as necessary.
         * The content of the status vector is included in the thrown exception.
         *
         * @throws css::sdbc::SQLException
         */
        void evaluateStatusVector(const ISC_STATUS_ARRAY& rStatusVector,
                                  std::u16string_view aCause,
                                  const css::uno::Reference< css::uno::XInterface >& _rxContext);

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
