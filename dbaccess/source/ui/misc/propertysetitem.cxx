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
#include "precompiled_dbui.hxx"

#ifndef _DBAUI_PROPERTYSETITEM_HXX_
#include "propertysetitem.hxx"
#endif

//.........................................................................
namespace dbaui
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;

    //=========================================================================
    //= OPropertySetItem
    //=========================================================================
    TYPEINIT1(OPropertySetItem, SfxPoolItem);
    //-------------------------------------------------------------------------
    OPropertySetItem::OPropertySetItem(sal_Int16 _nWhich)
        :SfxPoolItem(_nWhich)
    {
    }

    //-------------------------------------------------------------------------
    OPropertySetItem::OPropertySetItem(sal_Int16 _nWhich, const Reference< XPropertySet >& _rxSet)
        :SfxPoolItem(_nWhich)
        ,m_xSet(_rxSet)
    {
    }

    //-------------------------------------------------------------------------
    OPropertySetItem::OPropertySetItem(const OPropertySetItem& _rSource)
        :SfxPoolItem(_rSource)
        ,m_xSet(_rSource.m_xSet)
    {
    }

    //-------------------------------------------------------------------------
    int OPropertySetItem::operator==(const SfxPoolItem& _rItem) const
    {
        const OPropertySetItem* pCompare = PTR_CAST(OPropertySetItem, &_rItem);
        if ((!pCompare) || (pCompare->m_xSet.get() != m_xSet.get()))
            return 0;

        return 1;
    }

    //-------------------------------------------------------------------------
    SfxPoolItem* OPropertySetItem::Clone(SfxItemPool* /* _pPool */) const
    {
        return new OPropertySetItem(*this);
    }

//.........................................................................
}   // namespace dbaui
//.........................................................................

