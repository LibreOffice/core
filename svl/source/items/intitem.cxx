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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svl.hxx"

#include <svl/intitem.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <tools/bigint.hxx>
#include <tools/stream.hxx>
#include <svl/metitem.hxx>

//============================================================================
//
//  class SfxByteItem
//
//============================================================================

IMPL_POOLITEM_FACTORY(SfxByteItem)

// virtual
SfxPoolItem * SfxByteItem::Create(SvStream & rStream, sal_uInt16) const
{
    short nValue = 0;
    rStream >> nValue;
    return new SfxByteItem(Which(), sal_uInt8(nValue));
}

//============================================================================
//
//  class SfxInt16Item
//
//============================================================================

IMPL_POOLITEM_FACTORY(SfxInt16Item)
DBG_NAME(SfxInt16Item);

//============================================================================
SfxInt16Item::SfxInt16Item(sal_uInt16 which, SvStream & rStream):
    SfxPoolItem(which)
{
    DBG_CTOR(SfxInt16Item, 0);
    short nTheValue = 0;
    rStream >> nTheValue;
    m_nValue = nTheValue;
}

//============================================================================
// virtual
int SfxInt16Item::operator ==(const SfxPoolItem & rItem) const
{
    DBG_CHKTHIS(SfxInt16Item, 0);
    DBG_ASSERT(SfxPoolItem::operator ==(rItem), "unequal type");
    return m_nValue == SAL_STATIC_CAST(const SfxInt16Item *, &rItem)->
                        m_nValue;
}

//============================================================================
// virtual
int SfxInt16Item::Compare(const SfxPoolItem & rWith) const
{
    DBG_CHKTHIS(SfxInt16Item, 0);
    DBG_ASSERT(SfxPoolItem::operator ==(rWith), "unequal type");
    return SAL_STATIC_CAST(const SfxInt16Item *, &rWith)->m_nValue
             < m_nValue ?
            -1 :
           SAL_STATIC_CAST(const SfxInt16Item *, &rWith)->m_nValue
             == m_nValue ?
            0 : 1;
}

//============================================================================
// virtual
SfxItemPresentation SfxInt16Item::GetPresentation(SfxItemPresentation,
                                                  SfxMapUnit, SfxMapUnit,
                                                  XubString & rText,
                                                  const IntlWrapper *) const
{
    DBG_CHKTHIS(SfxInt16Item, 0);
    rText = UniString::CreateFromInt32(m_nValue);
    return SFX_ITEM_PRESENTATION_NAMELESS;
}


//============================================================================
// virtual
sal_Bool SfxInt16Item::QueryValue(com::sun::star::uno::Any& rVal, sal_uInt8) const
{
    sal_Int16 nValue = m_nValue;
    rVal <<= nValue;
    return sal_True;
}

//============================================================================
// virtual
sal_Bool SfxInt16Item::PutValue(const com::sun::star::uno::Any& rVal, sal_uInt8 )
{
    sal_Int16 nValue = sal_Int16();
    if (rVal >>= nValue)
    {
        m_nValue = nValue;
        return sal_True;
    }

    DBG_ERROR( "SfxInt16Item::PutValue - Wrong type!" );
    return sal_False;
}

//============================================================================
// virtual
SfxPoolItem * SfxInt16Item::Create(SvStream & rStream, sal_uInt16) const
{
    DBG_CHKTHIS(SfxInt16Item, 0);
    return new SfxInt16Item(Which(), rStream);
}

//============================================================================
// virtual
SvStream & SfxInt16Item::Store(SvStream & rStream, sal_uInt16) const
{
    DBG_CHKTHIS(SfxInt16Item, 0);
    rStream << short(m_nValue);
    return rStream;
}

//============================================================================
SfxPoolItem * SfxInt16Item::Clone(SfxItemPool *) const
{
    DBG_CHKTHIS(SfxInt16Item, 0);
    return new SfxInt16Item(*this);
}

//============================================================================
sal_Int16 SfxInt16Item::GetMin() const
{
    DBG_CHKTHIS(SfxInt16Item, 0);
    return -32768;
}

//============================================================================
sal_Int16 SfxInt16Item::GetMax() const
{
    DBG_CHKTHIS(SfxInt16Item, 0);
    return 32767;
}

//============================================================================
SfxFieldUnit SfxInt16Item::GetUnit() const
{
    DBG_CHKTHIS(SfxInt16Item, 0);
    return SFX_FUNIT_NONE;
}

//============================================================================
//
//  class SfxUInt16Item
//
//============================================================================

IMPL_POOLITEM_FACTORY(SfxUInt16Item)

//============================================================================
//
//  class SfxInt32Item
//
//============================================================================

IMPL_POOLITEM_FACTORY(SfxInt32Item)

//============================================================================
//
//  class SfxUInt32Item
//
//============================================================================

IMPL_POOLITEM_FACTORY(SfxUInt32Item)

//============================================================================
//
//  class SfxMetricItem
//
//============================================================================

DBG_NAME(SfxMetricItem);

//============================================================================
SfxMetricItem::SfxMetricItem(sal_uInt16 which, sal_uInt32 nValue):
    SfxInt32Item(which, nValue)
{
    DBG_CTOR(SfxMetricItem, 0);
}

//============================================================================
SfxMetricItem::SfxMetricItem(sal_uInt16 which, SvStream & rStream):
    SfxInt32Item(which, rStream)
{
    DBG_CTOR(SfxMetricItem, 0);
}

//============================================================================
SfxMetricItem::SfxMetricItem(const SfxMetricItem & rItem):
    SfxInt32Item(rItem)
{
    DBG_CTOR(SfxMetricItem, 0);
}

//============================================================================
// virtual
void SfxMetricItem::ScaleMetrics(long nMult, long nDiv)
{
    BigInt aTheValue(GetValue());
    aTheValue *= nMult;
    aTheValue += nDiv / 2;
    aTheValue /= nDiv;
    SetValue(aTheValue);
}

//============================================================================
// virtual
bool SfxMetricItem::HasMetrics() const
{
    return true;
}

