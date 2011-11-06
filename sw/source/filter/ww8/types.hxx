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



/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */

#ifndef WW_TYPES
#define WW_TYPES

#include <vector>

//if part of OOo
#include "typessw.hxx"
//else
//include standalonetypes.hxx
//endif

namespace ww
{
    typedef std::vector<sal_uInt8> bytes;

    enum WordVersion {eWW2 = 2, eWW6 = 6, eWW7 = 7, eWW8 = 8};
    inline bool IsSevenMinus(WordVersion eVer) { return eVer <= eWW7; }
    inline bool IsEightPlus(WordVersion eVer) { return eVer >= eWW8; }

    /** For custom wrapping

        When you edit the wrap points of a contour in word, word uses a relative
        scale of 0 to 21600 where 21600 is apparently 100% of the graphic width
    */
    const int nWrap100Percent = 21600;
}

#endif
/* vi:set tabstop=4 shiftwidth=4 expandtab: */
