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


#ifndef _GETMETRICVAL_HXX
#define _GETMETRICVAL_HXX

#include <tools/solar.h>

#define CM_1  0         // 1 centimeter     or 1/2 inch
#define CM_05 1         // 0.5 centimeter   or 1/4 inch
#define CM_01 2         // 0.1 centimeter   or 1/20 inch

inline sal_uInt16 GetMetricVal( int n )
{
#ifdef USE_MEASUREMENT
    sal_uInt16 nVal = MEASURE_METRIC == SvtSysLocale().GetLocaleData().getMeasurementSystemEnum()
                    ? 567       // 1 cm
                    : 770;      // 1/2 Inch
#else
    sal_uInt16 nVal = 567;      // 1 cm
#endif

    if( CM_01 == n )
        nVal /= 10;
    else if( CM_05 == n )
        nVal /= 2;
    return nVal;
}


#endif
