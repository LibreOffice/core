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
#include <svl/visitem.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <tools/stream.hxx>

//============================================================================
//
//  class SfxVisibilityItem
//
//============================================================================

DBG_NAME(SfxVisibilityItem)

//============================================================================
SfxVisibilityItem::SfxVisibilityItem(sal_uInt16 which, SvStream & rStream):
    SfxPoolItem(which)
{
    DBG_CTOR(SfxVisibilityItem, 0);
    sal_Bool bValue = 0;
    rStream >> bValue;
    m_nValue.bVisible = bValue;
}

//============================================================================
// virtual
int SfxVisibilityItem::operator ==(const SfxPoolItem & rItem) const
{
    DBG_CHKTHIS(SfxVisibilityItem, 0);
    DBG_ASSERT(SfxPoolItem::operator ==(rItem), "unequal type");
    return m_nValue.bVisible == SAL_STATIC_CAST(const SfxVisibilityItem *, &rItem)->
                        m_nValue.bVisible;
}

//============================================================================
// virtual
int SfxVisibilityItem::Compare(const SfxPoolItem & rWith) const
{
    DBG_ASSERT(dynamic_cast< const SfxVisibilityItem* >(&rWith), "SfxVisibilityItem::Compare(): Bad type");
    return m_nValue.bVisible == static_cast< SfxVisibilityItem const * >(&rWith)->m_nValue.bVisible ?
               0 : m_nValue.bVisible ? -1 : 1;
}

//============================================================================
// virtual
SfxItemPresentation SfxVisibilityItem::GetPresentation(SfxItemPresentation,
                                                  SfxMapUnit, SfxMapUnit,
                                                  XubString & rText,
                                                  const IntlWrapper *) const
{
    rText = GetValueTextByVal(m_nValue.bVisible);
    return SFX_ITEM_PRESENTATION_NAMELESS;
}


//============================================================================
// virtual
sal_Bool SfxVisibilityItem::QueryValue(com::sun::star::uno::Any& rVal,sal_uInt8) const
{
    rVal <<= m_nValue;
    return sal_True;
}

//============================================================================
// virtual
sal_Bool SfxVisibilityItem::PutValue(const com::sun::star::uno::Any& rVal,sal_uInt8)
{
    if (rVal >>= m_nValue)
        return sal_True;

    DBG_ERROR( "SfxInt16Item::PutValue - Wrong type!" );
    return sal_False;
}

//============================================================================
// virtual
SfxPoolItem * SfxVisibilityItem::Create(SvStream & rStream, sal_uInt16) const
{
    DBG_CHKTHIS(SfxVisibilityItem, 0);
    return new SfxVisibilityItem(Which(), rStream);
}

//============================================================================
// virtual
SvStream & SfxVisibilityItem::Store(SvStream & rStream, sal_uInt16) const
{
    DBG_CHKTHIS(SfxVisibilityItem, 0);
    rStream << m_nValue.bVisible;
    return rStream;
}

//============================================================================
// virtual
SfxPoolItem * SfxVisibilityItem::Clone(SfxItemPool *) const
{
    DBG_CHKTHIS(SfxVisibilityItem, 0);
    return new SfxVisibilityItem(*this);
}

//============================================================================
// virtual
sal_uInt16 SfxVisibilityItem::GetValueCount() const
{
    return 2;
}

//============================================================================
// virtual
UniString SfxVisibilityItem::GetValueTextByVal(sal_Bool bTheValue) const
{
    return
        bTheValue ?
            UniString::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("sal_True")) :
            UniString::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("sal_False"));
}
