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



#ifndef _SVX_XFTSHXY_HXX
#define _SVX_XFTSHXY_HXX

#include <svl/metitem.hxx>
#include "svx/svxdllapi.h"

/*************************************************************************
|*
|* FormText-ShadowXValItem
|*
\************************************************************************/

class SVX_DLLPUBLIC XFormTextShadowXValItem : public SfxMetricItem
{
public:
    POOLITEM_FACTORY()
                            XFormTextShadowXValItem(long nVal = 0);
                            XFormTextShadowXValItem(SvStream& rIn);
    virtual SfxPoolItem*    Clone(SfxItemPool* pPool = 0) const;
    virtual SfxPoolItem*    Create(SvStream& rIn, sal_uInt16 nVer) const;
};

/*************************************************************************
|*
|* FormText-ShadowYValItem
|*
\************************************************************************/

class SVX_DLLPUBLIC XFormTextShadowYValItem : public SfxMetricItem
{
public:
    POOLITEM_FACTORY()
                            XFormTextShadowYValItem(long nVal = 0);
                            XFormTextShadowYValItem(SvStream& rIn);
    virtual SfxPoolItem*    Clone(SfxItemPool* pPool = 0) const;
    virtual SfxPoolItem*    Create(SvStream& rIn, sal_uInt16 nVer) const;
};

#endif

