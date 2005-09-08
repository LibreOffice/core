/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DateConversion.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 04:57:12 $
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

#ifndef _CONNECTIVITY_DATECONVERSION_HXX_
#define _CONNECTIVITY_DATECONVERSION_HXX_

#ifndef _COM_SUN_STAR_UTIL_DATE_HPP_
#include <com/sun/star/util/Date.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_TIME_HPP_
#include <com/sun/star/util/Time.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif

namespace connectivity
{
    class DateConversion
    {
    public:
        static sal_Int32 toINT32(const ::com::sun::star::util::Date& rVal)
        {
            OSL_ENSURE(0,"Please use DBConversion instead!");
            return 0;
        }
        static sal_Int32 toINT32(const ::com::sun::star::util::Time& rVal)
        {
            OSL_ENSURE(0,"Please use DBConversion instead!");
            return 0;
        }
        static sal_Int64 toINT64(const ::com::sun::star::util::DateTime& rVal)
        {
            OSL_ENSURE(0,"Please use DBConversion instead!");
            return 0;
        }
        static sal_Int32 getMsFromTime(const ::com::sun::star::util::Time& rVal)
        {
            OSL_ENSURE(0,"Please use DBConversion instead!");
            return 0;
        }
        static double    toDouble(const ::com::sun::star::util::Date& rVal)
        {
            OSL_ENSURE(0,"Please use DBConversion instead!");
            return 0;
        }
        static double    toDouble(const ::com::sun::star::util::Time& rVal)
        {
            OSL_ENSURE(0,"Please use DBConversion instead!");
            return 0;
        }
        static double    toDouble(const ::com::sun::star::util::DateTime& rVal)
        {
            OSL_ENSURE(0,"Please use DBConversion instead!");
            return 0;
        }
        static ::com::sun::star::util::Date          toDate(double dVal,const ::com::sun::star::util::Date& _rSTANDARD_DB_DATE=::com::sun::star::util::Date(01,01,1900))
        {
            OSL_ENSURE(0,"Please use DBConversion instead!");
            return ::com::sun::star::util::Date();
        }
        static ::com::sun::star::util::Time          toTime(double dVal)
        {
            OSL_ENSURE(0,"Please use DBConversion instead!");
            return ::com::sun::star::util::Time();
        }
        static ::com::sun::star::util::DateTime      toDateTime(double dVal,const ::com::sun::star::util::Date& _rSTANDARD_DB_DATE=::com::sun::star::util::Date(01,01,1900))
        {
            OSL_ENSURE(0,"Please use DBConversion instead!");
            return ::com::sun::star::util::DateTime();
        }
    };
}
#endif // _CONNECTIVITY_DATECONVERSION_HXX_

