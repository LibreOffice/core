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
    else if (nYear<100)
        sBuffer.appendAscii("00");
    else if (nYear<1000)
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
