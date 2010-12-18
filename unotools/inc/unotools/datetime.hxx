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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
