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


#ifndef INCLUDED_UNOTOOLS_DATETIME_HXX
#define INCLUDED_UNOTOOLS_DATETIME_HXX

//= conversions UNO3.TimeClass <-> Tools.TimeClass (Date/Time/DateTime)
#include <unotools/unotoolsdllapi.h>
#include <rtl/ustring.hxx>

namespace com { namespace sun { namespace star { namespace util { struct Date; } } } }
namespace com { namespace sun { namespace star { namespace util { struct DateTime; } } } }
namespace com { namespace sun { namespace star { namespace util { struct Time; } } } }

class Date;
class DateTime;
class LocaleDataWrapper;

namespace utl
{
    UNOTOOLS_DLLPUBLIC const LocaleDataWrapper& GetLocaleData();
    UNOTOOLS_DLLPUBLIC DateTime GetDateTime(const css::util::DateTime& _rDT);
    UNOTOOLS_DLLPUBLIC OUString GetDateTimeString(const css::util::DateTime& _rDT);
    UNOTOOLS_DLLPUBLIC OUString GetDateTimeString(sal_Int32 _nDate, sal_Int32 _nTime);
    UNOTOOLS_DLLPUBLIC OUString GetDateString(const css::util::DateTime& _rDT);

    UNOTOOLS_DLLPUBLIC void typeConvert(const Date& _rDate, css::util::Date& _rOut);
    UNOTOOLS_DLLPUBLIC void typeConvert(const css::util::Date& _rDate, Date& _rOut);

    UNOTOOLS_DLLPUBLIC void typeConvert(const DateTime& _rDateTime, css::util::DateTime& _rOut);
    UNOTOOLS_DLLPUBLIC void typeConvert(const css::util::DateTime& _rDateTime, DateTime& _rOut);

    UNOTOOLS_DLLPUBLIC OUString toISO8601(const css::util::DateTime& _rDateTime);
    UNOTOOLS_DLLPUBLIC bool            ISO8601parseDateTime(const OUString &i_rIn, css::util::DateTime& o_rDateTime);
    UNOTOOLS_DLLPUBLIC bool            ISO8601parseDate(const OUString &i_rIn, css::util::Date& o_rDate);
    UNOTOOLS_DLLPUBLIC bool            ISO8601parseTime(const OUString &i_rIn, css::util::Time& o_Time);

}   // namespace utl

#endif // INCLUDED_UNOTOOLS_DATETIME_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
