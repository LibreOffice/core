/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _DBHELPER_DBCONVERSION_HXX_
#define _DBHELPER_DBCONVERSION_HXX_

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include "connectivity/dbtoolsdllapi.hxx"

// forward declarations
namespace rtl
{
    class OUString;
}
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

//.........................................................................
namespace dbtools
{
//.........................................................................

    class OOO_DLLPUBLIC_DBTOOLS DBTypeConversion
    {
    public:
        static ::com::sun::star::util::Date getStandardDate();
        static void setValue(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumnUpdate>& xVariant,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter>& xFormatter,
                                    const ::com::sun::star::util::Date& rNullDate,
                                    const ::rtl::OUString& rString,
                                    sal_Int32 nKey,
                                    sal_Int16 nFieldType,
                                    sal_Int16 nKeyType) throw(::com::sun::star::lang::IllegalArgumentException);

        static void setValue(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumnUpdate>& xVariant,
                             const ::com::sun::star::util::Date& rNullDate,
                             const double& rValue,
                             sal_Int16 nKeyType) throw(::com::sun::star::lang::IllegalArgumentException);

        static double getValue( const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn>& xVariant, const ::com::sun::star::util::Date& rNullDate );

        // get the columnvalue as string with a default format given by the column or a default format
        // for the type
        static ::rtl::OUString getFormattedValue(
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xColumn,
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter>& xFormatter,
                                        const ::com::sun::star::lang::Locale& _rLocale,
                                        const ::com::sun::star::util::Date& rNullDate);

        static ::rtl::OUString getFormattedValue(
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn>& _xColumn,
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter>& xFormatter,
                                        const ::com::sun::star::util::Date& rNullDate,
                                        sal_Int32 nKey,
                                        sal_Int16 nKeyType);

        static ::com::sun::star::util::Date     toDate(double dVal, const ::com::sun::star::util::Date& _rNullDate = getStandardDate());
        static ::com::sun::star::util::Date     toDate(const ::rtl::OUString& _sSQLDate);
        static ::com::sun::star::util::Time     toTime(double dVal);
        static ::com::sun::star::util::Time     toTime(const ::rtl::OUString& _sSQLDate);
        static ::com::sun::star::util::DateTime toDateTime(double dVal, const ::com::sun::star::util::Date& _rNullDate = getStandardDate());
        static ::com::sun::star::util::DateTime toDateTime(const ::rtl::OUString& _sSQLDate);


        /** return the given DateTime as JDBC compliant 64 bit value
        */
        static sal_Int64 toINT64(const ::com::sun::star::util::DateTime& rVal);
        static sal_Int32 getMsFromTime(const ::com::sun::star::util::Time& rVal);

        static sal_Int32 toDays(const ::com::sun::star::util::Date& _rVal, const ::com::sun::star::util::Date& _rNullDate = getStandardDate());

        static double   toDouble(const ::com::sun::star::util::Date& rVal, const ::com::sun::star::util::Date& _rNullDate = getStandardDate());
        static double   toDouble(const ::com::sun::star::util::Time& rVal);
        static double   toDouble(const ::com::sun::star::util::DateTime& rVal, const ::com::sun::star::util::Date& _rNullDate = getStandardDate());

        static sal_Int32    toINT32(const ::com::sun::star::util::Date& rVal);
        static sal_Int32    toINT32(const ::com::sun::star::util::Time& rVal);

        static ::com::sun::star::util::Date toDate(sal_Int32 _nVal);
        static ::com::sun::star::util::Time toTime(sal_Int32 _nVal);

        /** convert a double which is a date value relative to a given fixed date into a date value relative
            to the standard db null date.
        */
        static double toStandardDbDate(const ::com::sun::star::util::Date& _rNullDate, double _rVal) { return _rVal + toDays(_rNullDate); }
        /** convert a double which is a date value relative to the standard db null date into a date value relative
            to a given fixed date.
        */
        static double toNullDate(const ::com::sun::star::util::Date& _rNullDate, double _rVal) { return _rVal - toDays(_rNullDate); }

        // return the date from the numberformatsupplier or the STANDARD_DATE (1900,1,1)
        static ::com::sun::star::util::Date getNULLDate(const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier > &xSupplier);

        // return the date in the format %04d-%02d-%02d
        static ::rtl::OUString toDateString(const ::com::sun::star::util::Date& rDate);
        // return the time in the format %02d:%02d:%02d
        static ::rtl::OUString toTimeString(const ::com::sun::star::util::Time& rTime);
        // return the DateTime in the format %04d-%02d-%02d %02d:%02d:%02d
        static ::rtl::OUString toDateTimeString(const ::com::sun::star::util::DateTime& _rDateTime);
        // return the any in an sql standard format
        static ::rtl::OUString toSQLString(sal_Int32 eType, const ::com::sun::star::uno::Any& _rVal, sal_Bool bQuote,
            const ::com::sun::star::uno::Reference< ::com::sun::star::script::XTypeConverter >&  _rxTypeConverter);

        /** converts a Unicode string into a 8-bit string, using the given encoding

            @param _rSource
                the source string to convert
            @param _rDest
                the destination string
            @param _eEncoding
                the encoding to use for the conversion

            @throws com::sun::star::sdbc::SQLException
                if the given string contains characters which are not convertible using the given encoding
                The SQLState of the exception will be set to 22018 ("Invalid character value for cast specification")

            @return
                the length of the converted string
        */
        static sal_Int32 convertUnicodeString(
            const ::rtl::OUString& _rSource,
            ::rtl::OString&  _rDest,
            rtl_TextEncoding _eEncoding
        )
            SAL_THROW((::com::sun::star::sdbc::SQLException));

        /** converts a Unicode string into a 8-bit string, using the given encoding

            @param _rSource
                the source string to convert

            @param _rDest
                the destination string

            @param _nMaxLen
                the maximum length of the destination string

            @param _eEncoding
                the encoding to use for the conversion

            @throws com::sun::star::sdbc::SQLException
                if convertUnicodeString, which is called internally, throws such an exception

            @throws com::sun::star::sdbc::SQLException
                if the conversion results in a string which is longer than _nMaxLen

            @return
                the length of the converted string
        */
        static sal_Int32 convertUnicodeStringToLength(
            const ::rtl::OUString& _rSource,
            ::rtl::OString&  _rDest,
            sal_Int32 _nMaxLen,
            rtl_TextEncoding _eEncoding
       )
            SAL_THROW((::com::sun::star::sdbc::SQLException));
    };

//.........................................................................
}   // namespace dbtools
//.........................................................................

#endif // _DBHELPER_DBCONVERSION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
