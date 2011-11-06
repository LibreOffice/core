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


#ifndef _SFX_WHITER_HXX
#define _SFX_WHITER_HXX

#include "svl/svldllapi.h"

#ifndef INCLUDED_LIMITS_H
#include <limits.h>
#define INCLUDED_LIMITS_H
#endif
#include <tools/solar.h>

class SfxItemSet;


// INCLUDE ---------------------------------------------------------------

class SVL_DLLPUBLIC SfxWhichIter
{
    const sal_uInt16 *pRanges, *pStart;
    sal_uInt16 nOfst, nFrom, nTo;

public:
    SfxWhichIter( const SfxItemSet& rSet, sal_uInt16 nFrom = 0, sal_uInt16 nTo = USHRT_MAX );
    ~SfxWhichIter();

    sal_uInt16  GetCurWhich() const { return *pRanges + nOfst; }
    sal_uInt16  NextWhich();
    sal_uInt16  PrevWhich();

    sal_uInt16  FirstWhich();
    sal_uInt16  LastWhich();
};

#endif
