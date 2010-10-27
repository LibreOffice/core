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
            OSL_ENSURE(0,"Please use DBConversion instead!");
            return 0;
        }
        static sal_Int32 toINT32(const ::com::sun::star::util::Time&)
        {
            OSL_ENSURE(0,"Please use DBConversion instead!");
            return 0;
        }
        static sal_Int64 toINT64(const ::com::sun::star::util::DateTime&)
        {
            OSL_ENSURE(0,"Please use DBConversion instead!");
            return 0;
        }
        static sal_Int32 getMsFromTime(const ::com::sun::star::util::Time&)
        {
            OSL_ENSURE(0,"Please use DBConversion instead!");
            return 0;
        }
        static double    toDouble(const ::com::sun::star::util::Date&)
        {
            OSL_ENSURE(0,"Please use DBConversion instead!");
            return 0;
        }
        static double    toDouble(const ::com::sun::star::util::Time&)
        {
            OSL_ENSURE(0,"Please use DBConversion instead!");
            return 0;
        }
        static double    toDouble(const ::com::sun::star::util::DateTime&)
        {
            OSL_ENSURE(0,"Please use DBConversion instead!");
            return 0;
        }
        static ::com::sun::star::util::Date          toDate(double,const ::com::sun::star::util::Date& =::com::sun::star::util::Date(01,01,1900))
        {
            OSL_ENSURE(0,"Please use DBConversion instead!");
            return ::com::sun::star::util::Date();
        }
        static ::com::sun::star::util::Time          toTime(double)
        {
            OSL_ENSURE(0,"Please use DBConversion instead!");
            return ::com::sun::star::util::Time();
        }
        static ::com::sun::star::util::DateTime      toDateTime(double,const ::com::sun::star::util::Date& =::com::sun::star::util::Date(01,01,1900))
        {
            OSL_ENSURE(0,"Please use DBConversion instead!");
            return ::com::sun::star::util::DateTime();
        }
    };
}
#endif // _CONNECTIVITY_DATECONVERSION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
