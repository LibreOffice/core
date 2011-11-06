/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



//= conversions UNO3.TimeClass <-> Tools.TimeClass (Date/Time/DateTime)
#include "unotools/unotoolsdllapi.h"

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

