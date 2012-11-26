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



#ifndef _SFXENUMITEM_HXX
#define _SFXENUMITEM_HXX

#include "svl/svldllapi.h"
#include <svl/cenumitm.hxx>

//============================================================================
class SVL_DLLPUBLIC SfxEnumItem: public CntEnumItem
{
protected:
    SfxEnumItem(sal_uInt16 which = 0, sal_uInt16 nValue = 0):
        CntEnumItem(which, nValue) {}

    SfxEnumItem(sal_uInt16 which, SvStream & rStream):
        CntEnumItem(which, rStream) {}

public:
};

//============================================================================
class SVL_DLLPUBLIC SfxBoolItem: public CntBoolItem
{
public:
    POOLITEM_FACTORY()
    SfxBoolItem(sal_uInt16 which = 0, sal_Bool bValue = sal_False):
        CntBoolItem(which, bValue) {}

    SfxBoolItem(sal_uInt16 which, SvStream & rStream):
        CntBoolItem(which, rStream) {}

    virtual SfxPoolItem * Create(SvStream & rStream, sal_uInt16) const
    { return new SfxBoolItem(Which(), rStream); }

    virtual SfxPoolItem * Clone(SfxItemPool * = 0) const
    { return new SfxBoolItem(*this); }
};

#endif //  _SFXENUMITEM_HXX

