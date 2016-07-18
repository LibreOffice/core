/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_CONNECTIVITY_DBCONVERSION_HXX
#define INCLUDED_CONNECTIVITY_DBCONVERSION_HXX

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <connectivity/dbtoolsdllapi.hxx>

namespace com
{
    namespace sun
    {
        namespace star
        {
            namespace lang
            {
                struct Locale;
            }
            namespace sdb
            {
                class XColumn;
                class XColumnUpdate;
            }
            namespace sdbc
            {
                class SQLException;
            }
            namespace beans
            {
                class XPropertySet;
            }
            namespace util
            {
                class XNumberFormatter;
                class XNumberFormatsSupplier;
                struct Time;
                struct DateTime;
            }
            namespace script
            {
                class XTypeConverter;
            }
        }
    }
}

namespace dbtools
{
    namespace DBTypeConversion
    {
        OOO_DLLPUBLIC_DBTOOLS css::util::Date const & getStandardDate();
        OOO_DLLPUBLIC_DBTOOLS void setValue(const css::uno::Reference< css::sdb::XColumnUpdate>& xVariant,
                                    const css::uno::Reference< css::util::XNumberFormatter>& xFormatter,
                                    const css::util::Date& rNullDate,
                                    const OUString& rString,
                                    sal_Int32 nKey,
                                    sal_Int16 nFieldType,
                                    sal_Int16 nKeyType) throw(css::lang::IllegalArgumentException);

        OOO_DLLPUBLIC_DBTOOLS void setValue(const css::uno::Reference< css::sdb::XColumnUpdate>& xVariant,
                             const css::util::Date& rNullDate,
                             const double& rValue,
                             sal_Int16 nKeyType) throw(css::lang::IllegalArgumentException);

        OOO_DLLPUBLIC_DBTOOLS double getValue( const css::uno::Reference< css::sdb::XColumn>& xVariant, const css::util::Date& rNullDate );

        // get the columnvalue as string with a default format given by the column or a default format
        // for the type
        OOO_DLLPUBLIC_DBTOOLS OUString getFormattedValue(
                                        const css::uno::Reference< css::beans::XPropertySet>& _xColumn,
                                        const css::uno::Reference< css::util::XNumberFormatter>& xFormatter,
                                        const css::lang::Locale& _rLocale,
                                        const css::util::Date& rNullDate);

        OOO_DLLPUBLIC_DBTOOLS OUString getFormattedValue(
                                        const css::uno::Reference< css::sdb::XColumn>& _xColumn,
                                        const css::uno::Reference< css::util::XNumberFormatter>& xFormatter,
                                        const css::util::Date& rNullDate,
                                        sal_Int32 nKey,
                                        sal_Int16 nKeyType);

        OOO_DLLPUBLIC_DBTOOLS css::util::Date     toDate(double dVal, const css::util::Date& _rNullDate = getStandardDate());
        OOO_DLLPUBLIC_DBTOOLS css::util::Date     toDate(const OUString& _sSQLDate);
        OOO_DLLPUBLIC_DBTOOLS css::util::Time     toTime(double dVal, short nDigits = 9);
        OOO_DLLPUBLIC_DBTOOLS css::util::Time     toTime(const OUString& _sSQLDate);
        OOO_DLLPUBLIC_DBTOOLS css::util::DateTime toDateTime(double dVal, const css::util::Date& _rNullDate = getStandardDate());
        OOO_DLLPUBLIC_DBTOOLS css::util::DateTime toDateTime(const OUString& _sSQLDate);

        OOO_DLLPUBLIC_DBTOOLS sal_Int64 getNsFromTime(const css::util::Time& rVal);

        OOO_DLLPUBLIC_DBTOOLS sal_Int32 toDays(const css::util::Date& _rVal, const css::util::Date& _rNullDate = getStandardDate());

        OOO_DLLPUBLIC_DBTOOLS double   toDouble(const css::util::Date& rVal, const css::util::Date& _rNullDate = getStandardDate());
        OOO_DLLPUBLIC_DBTOOLS double   toDouble(const css::util::Time& rVal);
        OOO_DLLPUBLIC_DBTOOLS double   toDouble(const css::util::DateTime& rVal, const css::util::Date& _rNullDate = getStandardDate());

        OOO_DLLPUBLIC_DBTOOLS css::util::Date toDate(sal_Int32 _nVal);
        OOO_DLLPUBLIC_DBTOOLS css::util::Time toTime(sal_Int64 _nVal);

        /** convert a double which is a date value relative to the standard db null date into a date value relative
            to a given fixed date.
        */
        inline double toNullDate(const css::util::Date& _rNullDate, double _rVal) { return _rVal - toDays(_rNullDate); }

        // return the date from the numberformatsupplier or the STANDARD_DATE (1900,1,1)
        OOO_DLLPUBLIC_DBTOOLS css::util::Date getNULLDate(const css::uno::Reference< css::util::XNumberFormatsSupplier > &xSupplier);

        // return the date in the format %04d-%02d-%02d
        OOO_DLLPUBLIC_DBTOOLS OUString toDateString(const css::util::Date& rDate);
        // return the time in the format %02d:%02d:%02d
        OOO_DLLPUBLIC_DBTOOLS OUString toTimeStringS(const css::util::Time& rTime);
        // return the time in the format %02d:%02d:%02d.%09d
        OOO_DLLPUBLIC_DBTOOLS OUString toTimeString(const css::util::Time& rTime);
        // return the DateTime in the format %04d-%02d-%02d %02d:%02d:%02d.%09d
        OOO_DLLPUBLIC_DBTOOLS OUString toDateTimeString(const css::util::DateTime& _rDateTime);
        // return the any in an sql standard format
        OOO_DLLPUBLIC_DBTOOLS OUString toSQLString(sal_Int32 eType, const css::uno::Any& _rVal, bool bQuote,
            const css::uno::Reference< css::script::XTypeConverter >&  _rxTypeConverter);

        /** converts a Unicode string into a 8-bit string, using the given encoding

            @param _rSource
                the source string to convert
            @param _rDest
                the destination string
            @param _eEncoding
                the encoding to use for the conversion

            @throws css::sdbc::SQLException
                if the given string contains characters which are not convertible using the given encoding
                The SQLState of the exception will be set to 22018 ("Invalid character value for cast specification")

            @return
                the length of the converted string
        */
        OOO_DLLPUBLIC_DBTOOLS sal_Int32 convertUnicodeString(
            const OUString& _rSource,
            OString&  _rDest,
            rtl_TextEncoding _eEncoding
        );

        /** converts a Unicode string into a 8-bit string, using the given encoding

            @param _rSource
                the source string to convert

            @param _rDest
                the destination string

            @param _nMaxLen
                the maximum length of the destination string

            @param _eEncoding
                the encoding to use for the conversion

            @throws css::sdbc::SQLException
                if convertUnicodeString, which is called internally, throws such an exception

            @throws css::sdbc::SQLException
                if the conversion results in a string which is longer than _nMaxLen

            @return
                the length of the converted string
        */
        OOO_DLLPUBLIC_DBTOOLS sal_Int32 convertUnicodeStringToLength(
            const OUString& _rSource,
            OString&  _rDest,
            sal_Int32 _nMaxLen,
            rtl_TextEncoding _eEncoding
       );
    }

} // namespace dbtools

#endif // INCLUDED_CONNECTIVITY_DBCONVERSION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
