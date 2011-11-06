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



#ifndef _XPOOL_HXX
#define _XPOOL_HXX

#include <svl/itempool.hxx>
#include <svx/xdef.hxx>
#include "svx/svxdllapi.h"

/*************************************************************************
|*
|* Klassendeklaration
|*
\************************************************************************/

class SVX_DLLPUBLIC XOutdevItemPool : public SfxItemPool
{
protected:
    SfxPoolItem**   mppLocalPoolDefaults;
    SfxItemInfo*    mpLocalItemInfos;

public:
    XOutdevItemPool(
        SfxItemPool* pMaster = 0L,
        sal_uInt16 nAttrStart = XATTR_START,
        sal_uInt16 nAttrEnd = XATTR_END,
        sal_Bool bLoadRefCounts = sal_True);
    XOutdevItemPool(const XOutdevItemPool& rPool);

    virtual SfxItemPool* Clone() const;
protected:
    virtual ~XOutdevItemPool();
};

#endif      // _XPOOL_HXX
