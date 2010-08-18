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

#include <precomp.h>
#include <ary/namesort.hxx>


// NOT FULLY DEFINED SERVICES



namespace
{


int C_cAutodocNameOrdering1[256] =
    {   0,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255, //  0 ..
      255,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255,
      255,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255, // 32 ..
       70, 71, 72, 73,  74, 75, 76, 77,      78, 79,255,255, 255,255,255,255,

      255, 11, 13, 15,  17, 19, 21, 23,      25, 27, 29, 31,  33, 35, 37, 39, // 64 ..
       41, 43, 45, 47,  49, 51, 53, 55,      57, 59, 61,255, 255,255,255, 63,
      255, 11, 13, 15,  17, 19, 21, 23,      25, 27, 29, 31,  33, 35, 37, 39, // 96 ..
       41, 43, 45, 47,  49, 51, 53, 55,      57, 59, 61,255, 255,255,255,255,

      255,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255, //128 ..
      255,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255,
      255,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255, //160 ..
      255,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255,

      255,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255,
      255,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255,
      255,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255,
      255,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255
    };

int C_cAutodocNameOrdering2[256] =
    {   0,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255, //  0 ..
      255,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255,
      255,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255, // 32 ..
       70, 71, 72, 73,  74, 75, 76, 77,      78, 79,255,255, 255,255,255,255,

      255, 11, 13, 15,  17, 19, 21, 23,      25, 27, 29, 31,  33, 35, 37, 39, // 64 ..
       41, 43, 45, 47,  49, 51, 53, 55,      57, 59, 61,255, 255,255,255, 63,
      255, 12, 14, 16,  18, 20, 22, 24,      26, 28, 30, 32,  34, 36, 38, 40, // 96 ..
       42, 44, 46, 48,  50, 52, 54, 56,      58, 60, 62,255, 255,255,255,255,

      255,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255, //128 ..
      255,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255,
      255,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255, //160 ..
      255,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255,

      255,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255,
      255,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255,
      255,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255,
      255,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255
    };


}   // namespace anonymous


namespace ary
{


const csv::CharOrder_Table
LesserName::aOrdering1_(C_cAutodocNameOrdering1);

const csv::CharOrder_Table
LesserName::aOrdering2_(C_cAutodocNameOrdering2);



}   // namespace ary
