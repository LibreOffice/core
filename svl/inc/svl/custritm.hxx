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



#ifndef _SVTOOLS_CUSTRITM_HXX
#define _SVTOOLS_CUSTRITM_HXX

#include "svl/svldllapi.h"
#include <tools/debug.hxx>
#include <svl/poolitem.hxx>

//============================================================================
DBG_NAMEEX_VISIBILITY(CntUnencodedStringItem, SVL_DLLPUBLIC)

class SVL_DLLPUBLIC CntUnencodedStringItem: public SfxPoolItem
{
    XubString m_aValue;

public:
    CntUnencodedStringItem(sal_uInt16 which = 0): SfxPoolItem(which)
    { DBG_CTOR(CntUnencodedStringItem, 0); }

    CntUnencodedStringItem(sal_uInt16 which, const XubString & rTheValue):
        SfxPoolItem(which), m_aValue(rTheValue)
    { DBG_CTOR(CntUnencodedStringItem, 0); }

    CntUnencodedStringItem(const CntUnencodedStringItem & rItem):
        SfxPoolItem(rItem), m_aValue(rItem.m_aValue)
    { DBG_CTOR(CntUnencodedStringItem, 0); }

    virtual ~CntUnencodedStringItem() { DBG_DTOR(CntUnencodedStringItem, 0); }

    virtual int operator ==(const SfxPoolItem & rItem) const;

    virtual int Compare(const SfxPoolItem & rWith) const;

    virtual int Compare(SfxPoolItem const & rWith,
                        IntlWrapper const & rIntlWrapper) const;

    virtual SfxItemPresentation GetPresentation(SfxItemPresentation,
                                                SfxMapUnit, SfxMapUnit,
                                                XubString & rText,
                                                const IntlWrapper * = 0)
        const;

    virtual sal_Bool QueryValue(com::sun::star::uno::Any& rVal,
                            sal_uInt8 nMemberId = 0) const;

    virtual sal_Bool PutValue(const com::sun::star::uno::Any& rVal,
                          sal_uInt8 nMemberId = 0);

    virtual SfxPoolItem * Clone(SfxItemPool * = 0) const;

    const XubString & GetValue() const { return m_aValue; }

    inline void SetValue(const XubString & rTheValue);
};

inline void CntUnencodedStringItem::SetValue(const XubString & rTheValue)
{
    DBG_ASSERT(GetRefCount() == 0,
               "CntUnencodedStringItem::SetValue(): Pooled item");
    m_aValue = rTheValue;
}

#endif //  _SVTOOLS_CUSTRITM_HXX

