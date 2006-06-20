/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DateTimeHelper.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:35:23 $
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

#ifndef _WEBDAV_DATETIME_HELPER_HXX
#define _WEBDAV_DATETIME_HELPER_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

namespace com { namespace sun { namespace star { namespace util {
    struct DateTime;
} } } }

namespace rtl {
    class OUString;
}

namespace webdav_ucp
{

class DateTimeHelper
{
private:
    static sal_Int32 convertMonthToInt (const ::rtl::OUString& );

    static bool ISO8601_To_DateTime (const ::rtl::OUString&,
        ::com::sun::star::util::DateTime& );

    static bool RFC2068_To_DateTime (const ::rtl::OUString&,
        ::com::sun::star::util::DateTime& );

public:
    static bool convert (const ::rtl::OUString&,
        ::com::sun::star::util::DateTime& );
};

} // namespace webdav_ucp

#endif

