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



#ifndef _SVDPOOL_HXX
#define _SVDPOOL_HXX

#include <svx/xpool.hxx>
#include <svx/svddef.hxx>
#include "svx/svxdllapi.h"

class XLineAttrSetItem;
class XFillAttrSetItem;

////////////////////////////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrItemPool : public XOutdevItemPool
{
public:
    SdrItemPool(SfxItemPool* pMaster = 0L, sal_Bool bLoadRefCounts = sal_True);
    SdrItemPool(const SdrItemPool& rPool);
protected:
    virtual ~SdrItemPool();
public:

    virtual SfxItemPool* Clone() const;
    virtual SfxItemPresentation GetPresentation(const SfxPoolItem& rItem,
        SfxItemPresentation ePresentation,
        SfxMapUnit          ePresentationMetric,
        String&             rText,
        const IntlWrapper * pIntlWrapper
        = 0) const;

    static void TakeItemName(sal_uInt16 nWhich, String& rItemName);
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // _SVDPOOL_HXX
// eof
