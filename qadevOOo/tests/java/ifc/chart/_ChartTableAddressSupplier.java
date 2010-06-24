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

package ifc.chart;

import lib.MultiPropertyTest;

import com.sun.star.chart.ChartSeriesAddress;

/**
* Testing <code>com.sun.star.chart.ChartTableAddressSupplier</code>
* service properties :
* <ul>
*  <li><code> CategoriesRangeAddress</code></li>
*  <li><code> SeriesAddresses</code></li>
*  <li><code> MainTitleAddress</code></li>
*  <li><code> SubTitleAddress</code></li>
* </ul> <p>
* @see com.sun.star.chart.ChartTableAddressSupplier
*/
public class _ChartTableAddressSupplier extends MultiPropertyTest {

    /**
    * Tests property 'SeriesAddresses'.
    */
    public void _SeriesAddresses() {
        testProperty("SeriesAddresses", new PropertyTester() {
            protected Object getNewValue(String propName, Object oldValue) {
                ChartSeriesAddress[] addr = (ChartSeriesAddress[]) oldValue ;
                ChartSeriesAddress[] newAddr = null ;
                if (addr.length > 1) {
                    newAddr = new ChartSeriesAddress[addr.length - 1] ;
                    System.arraycopy(addr, 0, newAddr, 0, addr.length - 1) ;
                } else {
                    newAddr = addr ;
                }

                return newAddr ;
            }
        }) ;
    }
}

