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


#ifndef _SFXPOOLCACH_HXX
#define _SFXPOOLCACH_HXX

#include "svl/svldllapi.h"
#include <tools/solar.h>
#include <vector>

//------------------------------------------------------------------------

class SfxItemPool;
class SfxItemSet;
class SfxPoolItem;
class SfxSetItem;

struct SfxItemModifyImpl
{
    const SfxSetItem  *pOrigItem;
    SfxSetItem        *pPoolItem;
};

typedef std::vector<SfxItemModifyImpl> SfxItemModifyArr_Impl;


class SVL_DLLPUBLIC SfxItemPoolCache
{
    SfxItemPool             *pPool;
    SfxItemModifyArr_Impl   *pCache;
    const SfxItemSet        *pSetToPut;
    const SfxPoolItem       *pItemToPut;

public:
                            SfxItemPoolCache( SfxItemPool *pPool,
                                              const SfxPoolItem *pPutItem );
                            SfxItemPoolCache( SfxItemPool *pPool,
                                              const SfxItemSet *pPutSet );
                            ~SfxItemPoolCache();

    const SfxSetItem&       ApplyTo( const SfxSetItem& rSetItem, sal_Bool bNew = sal_False );
};


#endif

