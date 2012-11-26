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
#include <com/sun/star/uno/Any.hxx>

#include <unotools/intlwrapper.hxx>
#include <tools/stream.hxx>
#include <svl/custritm.hxx>

//============================================================================
//
//  class CntUnencodedStringItem
//
//============================================================================

DBG_NAME(CntUnencodedStringItem)

//============================================================================
// virtual
int CntUnencodedStringItem::operator ==(const SfxPoolItem & rItem) const
{
    DBG_CHKTHIS(CntUnencodedStringItem, 0);
    DBG_ASSERT(dynamic_cast< const CntUnencodedStringItem* >(&rItem), "CntUnencodedStringItem::operator ==(): Bad type");
    return m_aValue
            == SAL_STATIC_CAST(const CntUnencodedStringItem *, &rItem)->
                m_aValue;
}

//============================================================================
// virtual
int CntUnencodedStringItem::Compare(SfxPoolItem const & rWith) const
{
    DBG_ERROR("CntUnencodedStringItem::Compare(): No international");
    DBG_CHKTHIS(CntUnencodedStringItem, 0);
    DBG_ASSERT(dynamic_cast< const CntUnencodedStringItem* >(&rWith), "CntUnencodedStringItem::Compare(): Bad type");
    switch (m_aValue.CompareTo(static_cast< CntUnencodedStringItem const * >(
                                       &rWith)->
                                   m_aValue))
    {
        case COMPARE_LESS:
            return -1;

        case COMPARE_EQUAL:
            return 0;

        default: // COMPARE_GREATER
            return 1;
    }
}

//============================================================================
// virtual
int CntUnencodedStringItem::Compare(SfxPoolItem const & rWith,
                                    IntlWrapper const & rIntlWrapper)
    const
{
    DBG_CHKTHIS(CntUnencodedStringItem, 0);
    DBG_ASSERT(dynamic_cast< const CntUnencodedStringItem* >(&rWith), "CntUnencodedStringItem::Compare(): Bad type");
    return rIntlWrapper.getCollator()->compareString( m_aValue,
        static_cast< CntUnencodedStringItem const * >(&rWith)->m_aValue );
}

//============================================================================
// virtual
SfxItemPresentation
CntUnencodedStringItem::GetPresentation(SfxItemPresentation, SfxMapUnit,
                                        SfxMapUnit, XubString & rText,
                                        const IntlWrapper *) const
{
    DBG_CHKTHIS(CntUnencodedStringItem, 0);
    rText = m_aValue;
    return SFX_ITEM_PRESENTATION_NAMELESS;
}

//============================================================================
// virtual
sal_Bool CntUnencodedStringItem::QueryValue(com::sun::star::uno::Any& rVal, sal_uInt8)
    const
{
    rVal <<= rtl::OUString(m_aValue);
    return true;
}

//============================================================================
// virtual
sal_Bool CntUnencodedStringItem::PutValue(const com::sun::star::uno::Any& rVal,
                                         sal_uInt8)
{
    rtl::OUString aTheValue;
    if (rVal >>= aTheValue)
    {
        m_aValue = UniString(aTheValue);
        return true;
    }
    DBG_ERROR("CntUnencodedStringItem::PutValue(): Wrong type");
    return false;
}

//============================================================================
// virtual
SfxPoolItem * CntUnencodedStringItem::Clone(SfxItemPool *) const
{
    DBG_CHKTHIS(CntUnencodedStringItem, 0);
    return new CntUnencodedStringItem(*this);
}

