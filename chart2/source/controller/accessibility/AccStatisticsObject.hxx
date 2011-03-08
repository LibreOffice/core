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
#ifndef _CHART2_ACCSTATISTICSOBJECT_HXX
#define _CHART2_ACCSTATISTICSOBJECT_HXX

#include "AccessibleChartElement.hxx"

#include "ObjectIdentifier.hxx"

namespace chart
{

class AccStatisticsObject : public AccessibleChartElement
{
public:
    /*
    enum StatisticsObjectType
    {
        MEAN_VAL_LINE = OBJECTTYPE_AVERAGEVALUE,
        ERROR_BARS    = OBJECTTYPE_ERROR,
        REGRESSION    = OBJECTTYPE_REGRESSION
    };
    */

    AccStatisticsObject( const AccessibleElementInfo& rAccInfo );

    virtual ~AccStatisticsObject();

    // ________ XAccessibleContext ________
    virtual ::rtl::OUString SAL_CALL getAccessibleName() throw (::com::sun::star::uno::RuntimeException);

    // ________ XServiceInfo ________
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException);

private:
    //const StatisticsObjectType    m_eType;
    //const sal_uInt16              m_nSeriesIndex;
};

}  // accessibility

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
