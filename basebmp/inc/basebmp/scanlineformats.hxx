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



#ifndef INCLUDED_BASEBMP_SCANLINEFORMATS_HXX
#define INCLUDED_BASEBMP_SCANLINEFORMATS_HXX

#include <sal/types.h>

/* Definition of Scanline formats */

namespace basebmp { namespace Format
{
    static const sal_Int32 NONE                         = 0;
    static const sal_Int32 ONE_BIT_MSB_GREY             = (sal_Int32)0x01;
    static const sal_Int32 ONE_BIT_LSB_GREY             = (sal_Int32)0x02;
    static const sal_Int32 ONE_BIT_MSB_PAL              = (sal_Int32)0x03;
    static const sal_Int32 ONE_BIT_LSB_PAL              = (sal_Int32)0x04;
    static const sal_Int32 FOUR_BIT_MSB_GREY            = (sal_Int32)0x05;
    static const sal_Int32 FOUR_BIT_LSB_GREY            = (sal_Int32)0x06;
    static const sal_Int32 FOUR_BIT_MSB_PAL             = (sal_Int32)0x07;
    static const sal_Int32 FOUR_BIT_LSB_PAL             = (sal_Int32)0x08;
    static const sal_Int32 EIGHT_BIT_PAL                = (sal_Int32)0x09;
    static const sal_Int32 EIGHT_BIT_GREY               = (sal_Int32)0x0A;
    static const sal_Int32 SIXTEEN_BIT_LSB_TC_MASK      = (sal_Int32)0x0B;
    static const sal_Int32 SIXTEEN_BIT_MSB_TC_MASK      = (sal_Int32)0x0C;
    static const sal_Int32 TWENTYFOUR_BIT_TC_MASK       = (sal_Int32)0x0D;
    static const sal_Int32 THIRTYTWO_BIT_TC_MASK        = (sal_Int32)0x0E;
    static const sal_Int32 THIRTYTWO_BIT_TC_MASK_ARGB   = (sal_Int32)0x0F;
    static const sal_Int32 MAX                          = (sal_Int32)0x0F;
} }

#endif /* INCLUDED_BASEBMP_SCANLINEFORMATS_HXX */
