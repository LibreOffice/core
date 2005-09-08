/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dbconversion.hxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 05:00:55 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _DBHELPER_DBCONVERSION_HXX_
#define _DBHELPER_DBCONVERSION_HXX_

#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATE_HPP_
#include <com/sun/star/util/Date.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif


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

    class DBTypeConversion
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

        static double getValue(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn>& xVariant, const ::com::sun::star::util::Date& rNullDate,
                                    sal_Int16 nKeyType);

        // get the columnvalue as string with a default format given by the column or a default format
        // for the type
        static ::rtl::OUString getValue(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xColumn,
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter>& xFormatter,
                                        const ::com::sun::star::lang::Locale& _rLocale,
                                        const ::com::sun::star::util::Date& rNullDate);

        static ::rtl::OUString getValue(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn>& _xColumn,
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
    };

//.........................................................................
}   // namespace dbtools
//.........................................................................

#endif // _DBHELPER_DBCONVERSION_HXX_

