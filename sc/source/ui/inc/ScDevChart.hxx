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
#ifndef INCLUDED_DEVCHARTCONFIG_HXX
#define INCLUDED_DEVCHARTCONFIG_HXX

#include <unotools/configitem.hxx>

namespace ScDevChart
{

class DevChartConfigItem : public ::utl::ConfigItem
{
public:
    DevChartConfigItem() :
            ConfigItem( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Office.Common/Internal" )),
                        CONFIG_MODE_IMMEDIATE_UPDATE )
    {}

    bool UseDevelopmentChart();
};

bool DevChartConfigItem::UseDevelopmentChart()
{
    bool bResult = false;

    ::com::sun::star::uno::Sequence< ::rtl::OUString > aNames( 1 );
    aNames[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DevelopmentChart" ));

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > aResult( GetProperties( aNames ));

    OSL_ASSERT( aResult.getLength());
    aResult[0] >>= bResult;

    return bResult;
}

/** States whether the new chart implementation or the old one should be used.
    If <TRUE/> is returned the newly developed chart (chart2) should be used.
    If <FALSE/> is returned, the old chart (sch) should be used.

    Config-Item: Office.Common/Internal:DevelopmentChart

    This function (the complete header) is only for a transitional period.  It
    will be deprecated after the new chart is definitely integrated into the
    product.
 */
bool UseDevChart()
{
//     static DevChartConfigItem aCfgItem;
//     return aCfgItem.UseDevelopmentChart();

    // ignore configuration
    //@todo: get rid of this class
    return true;
}

} // namespace ScDevChart

// INCLUDED_DEVCHARTCONFIG_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
