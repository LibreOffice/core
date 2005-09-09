/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: datetime.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:31:07 $
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

//= conversions UNO3.TimeClass <-> Tools.TimeClass (Date/Time/DateTime)
#ifndef INCLUDED_UNOTOOLSDLLAPI_H
#include "unotools/unotoolsdllapi.h"
#endif

#ifndef _UNOTOOLS_DATETIME_HXX_
#define _UNOTOOLS_DATETIME_HXX_

#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/DateTime.hpp>

class Date;
class Time;
class DateTime;

//.........................................................................
namespace utl
{
//.........................................................................

    namespace starutil = ::com::sun::star::util;

    UNOTOOLS_DLLPUBLIC void typeConvert(const Time& _rTime, starutil::Time& _rOut);
    UNOTOOLS_DLLPUBLIC void typeConvert(const starutil::Time& _rTime, Time& _rOut);

    UNOTOOLS_DLLPUBLIC void typeConvert(const Date& _rDate, starutil::Date& _rOut);
    UNOTOOLS_DLLPUBLIC void typeConvert(const starutil::Date& _rDate, Date& _rOut);

    UNOTOOLS_DLLPUBLIC void typeConvert(const DateTime& _rDateTime, starutil::DateTime& _rOut);
    UNOTOOLS_DLLPUBLIC void typeConvert(const starutil::DateTime& _rDateTime, DateTime& _rOut);

//.........................................................................
}   // namespace utl
//.........................................................................

#endif // _UNOTOOLS_DATETIME_HXX_

