/*************************************************************************
 *
 *  $RCSfile: dbconversion.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-05 08:56:19 $
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

#ifndef _COM_SUN_STAR_SDB_XCOLUMNUPDATE_HPP_
#include <com/sun/star/sdb/XColumnUpdate.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XCOLUMN_HPP_
#include <com/sun/star/sdb/XColumn.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTER_HPP_
#include <com/sun/star/util/XNumberFormatter.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

//.........................................................................
namespace dbtools
{
//.........................................................................

    namespace staruno   = ::com::sun::star::uno;
    namespace starsdbc  = ::com::sun::star::sdbc;
    namespace starsdb   = ::com::sun::star::sdb;
    namespace starutil  = ::com::sun::star::util;
    namespace starlang  = ::com::sun::star::lang;
    namespace starbeans = ::com::sun::star::beans;

class DBTypeConversion
{
public:
    static starutil::Date STANDARD_DB_DATE;

public:
    static void setValue(const staruno::Reference<starsdb::XColumnUpdate>& xVariant,
                                const staruno::Reference<starutil::XNumberFormatter>& xFormatter,
                                const starutil::Date& rNullDate,
                                const ::rtl::OUString& rString,
                                sal_Int32 nKey,
                                sal_Int16 nFieldType,
                                sal_Int16 nKeyType) throw(starlang::IllegalArgumentException);

    static void setValue(const staruno::Reference<starsdb::XColumnUpdate>& xVariant,
                         const starutil::Date& rNullDate,
                         const double& rValue,
                         sal_Int16 nKeyType) throw(starlang::IllegalArgumentException);

    static double getValue(const staruno::Reference<starsdb::XColumn>& xVariant, const starutil::Date& rNullDate,
                                sal_Int16 nKeyType);

    // get the columnvalue as string with a default format given by the column or a default format
    // for the type
    static ::rtl::OUString getValue(const staruno::Reference<starbeans::XPropertySet>& _xColumn,
                                    const staruno::Reference<starutil::XNumberFormatter>& xFormatter,
                                    const starlang::Locale& _rLocale,
                                    const starutil::Date& rNullDate);

    static ::rtl::OUString getValue(const staruno::Reference<starsdb::XColumn>& _xColumn,
                                    const staruno::Reference<starutil::XNumberFormatter>& xFormatter,
                                    const starutil::Date& rNullDate,
                                    sal_Int32 nKey,
                                    sal_Int16 nKeyType);

    static starutil::Date       toDate(double dVal, const starutil::Date& _rNullDate = STANDARD_DB_DATE);
    static starutil::Time       toTime(double dVal);
    static starutil::DateTime   toDateTime(double dVal, const starutil::Date& _rNullDate = STANDARD_DB_DATE);

    /** return the given DateTime as JDBC compliant 64 bit value
    */
    static sal_Int64 toINT64(const starutil::DateTime& rVal);
    static sal_Int32 getMsFromTime(const starutil::Time& rVal);

    static sal_Int32 toDays(const starutil::Date& _rVal, const starutil::Date& _rNullDate = STANDARD_DB_DATE);

    static double   toDouble(const starutil::Date& rVal, const starutil::Date& _rNullDate = STANDARD_DB_DATE);
    static double   toDouble(const starutil::Time& rVal);
    static double   toDouble(const starutil::DateTime& rVal, const starutil::Date& _rNullDate = STANDARD_DB_DATE);

    static sal_Int32    toINT32(const starutil::Date& rVal);
    static sal_Int32    toINT32(const starutil::Time& rVal);

    /** convert a double which is a date value relative to a given fixed date into a date value relative
        to the standard db null date.
    */
    static double toStandardDbDate(const starutil::Date& _rNullDate, double _rVal) { return _rVal + toDays(_rNullDate); }
    /** convert a double which is a date value relative to the standard db null date into a date value relative
        to a given fixed date.
    */
    static double toNullDate(const starutil::Date& _rNullDate, double _rVal) { return _rVal - toDays(_rNullDate); }
};

//.........................................................................
}   // namespace dbtools
//.........................................................................

#endif // _DBHELPER_DBCONVERSION_HXX_

