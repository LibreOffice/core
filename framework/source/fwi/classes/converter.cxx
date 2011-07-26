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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"
#include <classes/converter.hxx>
#include <rtl/ustrbuf.hxx>

namespace framework{

//-----------------------------------------------------------------------------
/**
 * converts a sequence of PropertyValue to a sequence of NamedValue.
 */
css::uno::Sequence< css::beans::NamedValue > Converter::convert_seqPropVal2seqNamedVal( const css::uno::Sequence< css::beans::PropertyValue >& lSource )
{
    sal_Int32 nCount = lSource.getLength();
    css::uno::Sequence< css::beans::NamedValue > lDestination(nCount);
    for (sal_Int32 nItem=0; nItem<nCount; ++nItem)
    {
        lDestination[nItem].Name  = lSource[nItem].Name ;
        lDestination[nItem].Value = lSource[nItem].Value;
    }
    return lDestination;
}

//-----------------------------------------------------------------------------
/**
 * converts a sequence of unicode strings into a vector of such items
 */
OUStringList Converter::convert_seqOUString2OUStringList( const css::uno::Sequence< ::rtl::OUString >& lSource )
{
    OUStringList lDestination;
    sal_Int32 nCount = lSource.getLength();

    for (sal_Int32 nItem=0; nItem<nCount; ++nItem )
    {
        lDestination.push_back(lSource[nItem]);
    }

    return lDestination;
}

::rtl::OUString Converter::convert_DateTime2ISO8601( const DateTime& aSource )
{
    ::rtl::OUStringBuffer sBuffer(25);

    sal_Int32 nYear  = aSource.GetYear();
    sal_Int32 nMonth = aSource.GetMonth();
    sal_Int32 nDay   = aSource.GetDay();

    sal_Int32 nHour  = aSource.GetHour();
    sal_Int32 nMin   = aSource.GetMin();
    sal_Int32 nSec   = aSource.GetSec();

    // write year formated as "YYYY"
    if (nYear<10)
        sBuffer.appendAscii("000");
    else
    if (nYear<100)
        sBuffer.appendAscii("00");
    else
    if (nYear<1000)
        sBuffer.appendAscii("0");
    sBuffer.append( (sal_Int32)nYear );

    sBuffer.appendAscii("-");
    // write month formated as "MM"
    if (nMonth<10)
        sBuffer.appendAscii("0");
    sBuffer.append( (sal_Int32)nMonth );

    sBuffer.appendAscii("-");
    // write day formated as "DD"
    if (nDay<10)
        sBuffer.appendAscii("0");
    sBuffer.append( (sal_Int32)nDay );

    sBuffer.appendAscii("T");
    // write hours formated as "hh"
    if (nHour<10)
        sBuffer.appendAscii("0");
    sBuffer.append( (sal_Int32)nHour );

    sBuffer.appendAscii(":");
    // write min formated as "mm"
    if (nMin<10)
        sBuffer.appendAscii("0");
    sBuffer.append( (sal_Int32)nMin );

    sBuffer.appendAscii(":");
    // write sec formated as "ss"
    if (nSec<10)
        sBuffer.appendAscii("0");
    sBuffer.append( (sal_Int32)nSec );

    sBuffer.appendAscii("Z");

    return sBuffer.makeStringAndClear();
}

}       //  namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
