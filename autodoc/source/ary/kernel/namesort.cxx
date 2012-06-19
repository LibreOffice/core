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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
