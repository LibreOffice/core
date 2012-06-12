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

#ifndef _CONNECTIVITY_DATECONVERSION_HXX_
#define _CONNECTIVITY_DATECONVERSION_HXX_

#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include "connectivity/dbtoolsdllapi.hxx"

namespace connectivity
{
    class OOO_DLLPUBLIC_DBTOOLS DateConversion
    {
    public:
        static sal_Int32 toINT32(const ::com::sun::star::util::Date&)
        {
            OSL_FAIL("Please use DBConversion instead!");
            return 0;
        }
        static sal_Int32 toINT32(const ::com::sun::star::util::Time&)
        {
            OSL_FAIL("Please use DBConversion instead!");
            return 0;
        }
        static sal_Int64 toINT64(const ::com::sun::star::util::DateTime&)
        {
            OSL_FAIL("Please use DBConversion instead!");
            return 0;
        }
        static sal_Int32 getMsFromTime(const ::com::sun::star::util::Time&)
        {
            OSL_FAIL("Please use DBConversion instead!");
            return 0;
        }
        static double    toDouble(const ::com::sun::star::util::Date&)
        {
            OSL_FAIL("Please use DBConversion instead!");
            return 0;
        }
        static double    toDouble(const ::com::sun::star::util::Time&)
        {
            OSL_FAIL("Please use DBConversion instead!");
            return 0;
        }
        static double    toDouble(const ::com::sun::star::util::DateTime&)
        {
            OSL_FAIL("Please use DBConversion instead!");
            return 0;
        }
        static ::com::sun::star::util::Date          toDate(double,const ::com::sun::star::util::Date& =::com::sun::star::util::Date(01,01,1900))
        {
            OSL_FAIL("Please use DBConversion instead!");
            return ::com::sun::star::util::Date();
        }
        static ::com::sun::star::util::Time          toTime(double)
        {
            OSL_FAIL("Please use DBConversion instead!");
            return ::com::sun::star::util::Time();
        }
        static ::com::sun::star::util::DateTime      toDateTime(double,const ::com::sun::star::util::Date& =::com::sun::star::util::Date(01,01,1900))
        {
            OSL_FAIL("Please use DBConversion instead!");
            return ::com::sun::star::util::DateTime();
        }
    };
}
#endif // _CONNECTIVITY_DATECONVERSION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
