/*************************************************************************
 *
 *  $RCSfile: dbconversion.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: oj $ $Date: 2000-11-09 08:41:49 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DBHELPER_DBCONVERSION_HXX_
#define _DBHELPER_DBCONVERSION_HXX_

//#ifndef _COM_SUN_STAR_SDB_XCOLUMNUPDATE_HPP_
//#include <com/sun/star/sdb/XColumnUpdate.hpp>
//#endif
//#ifndef _COM_SUN_STAR_SDB_XCOLUMN_HPP_
//#include <com/sun/star/sdb/XColumn.hpp>
//#endif
//#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTER_HPP_
//#include <com/sun/star/util/XNumberFormatter.hpp>
//#endif
//#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
//#include <com/sun/star/beans/XPropertySet.hpp>
//#endif
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
                class IllegalArgumentException;
                struct Locale;
            }
            namespace sdb
            {
                class XColumn;
                class XColumnUpdate;
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
        static ::com::sun::star::util::Date STANDARD_DB_DATE;

    public:
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

        static ::com::sun::star::util::Date     toDate(double dVal, const ::com::sun::star::util::Date& _rNullDate = STANDARD_DB_DATE);
        static ::com::sun::star::util::Time     toTime(double dVal);
        static ::com::sun::star::util::DateTime toDateTime(double dVal, const ::com::sun::star::util::Date& _rNullDate = STANDARD_DB_DATE);

        /** return the given DateTime as JDBC compliant 64 bit value
        */
        static sal_Int64 toINT64(const ::com::sun::star::util::DateTime& rVal);
        static sal_Int32 getMsFromTime(const ::com::sun::star::util::Time& rVal);

        static sal_Int32 toDays(const ::com::sun::star::util::Date& _rVal, const ::com::sun::star::util::Date& _rNullDate = STANDARD_DB_DATE);

        static double   toDouble(const ::com::sun::star::util::Date& rVal, const ::com::sun::star::util::Date& _rNullDate = STANDARD_DB_DATE);
        static double   toDouble(const ::com::sun::star::util::Time& rVal);
        static double   toDouble(const ::com::sun::star::util::DateTime& rVal, const ::com::sun::star::util::Date& _rNullDate = STANDARD_DB_DATE);

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
        ::rtl::OUString toSQLString(sal_Int32 eType, const ::com::sun::star::uno::Any& _rVal, sal_Bool bQuote,
            const ::com::sun::star::uno::Reference< ::com::sun::star::script::XTypeConverter >&  _rxTypeConverter);
    };

//.........................................................................
}   // namespace dbtools
//.........................................................................

#endif // _DBHELPER_DBCONVERSION_HXX_

