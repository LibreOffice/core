/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svl/grabbagitem.hxx>
#include <svl/poolitem.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <comphelper/sequence.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>

DBG_NAME(SfxGrabBagItem)

TYPEINIT1_AUTOFACTORY(SfxGrabBagItem, SfxPoolItem);

using namespace com::sun::star;

SfxGrabBagItem::SfxGrabBagItem()
{
}

SfxGrabBagItem::SfxGrabBagItem(sal_uInt16 nWhich, const std::map<OUString, uno::Any> *pMap) :
    SfxPoolItem( nWhich )
{
    if (pMap)
        m_aMap = *pMap;
}

SfxGrabBagItem::SfxGrabBagItem(const SfxGrabBagItem& rItem) :
    SfxPoolItem(rItem),
    m_aMap(rItem.m_aMap)
{
}

SfxGrabBagItem::~SfxGrabBagItem()
{
}

void SfxGrabBagItem::SetGrabBag(const std::map<OUString, uno::Any>& rMap)
{
    m_aMap = rMap;
}

const std::map<OUString, uno::Any>& SfxGrabBagItem::GetGrabBag() const
{
    return m_aMap;
}

int SfxGrabBagItem::operator==(const SfxPoolItem& rItem) const
{
    SfxGrabBagItem* pItem = (SfxGrabBagItem*)&rItem;

    return m_aMap == pItem->m_aMap;
}

SfxPoolItem* SfxGrabBagItem::Clone(SfxItemPool * /*pPool*/) const
{
    return new SfxGrabBagItem(*this);
}

bool SfxGrabBagItem::PutValue(const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    uno::Sequence<beans::PropertyValue> aValue;
    if ( rVal >>= aValue )
    {
        m_aMap.clear();
        comphelper::OSequenceIterator<beans::PropertyValue> i(aValue);
        while (i.hasMoreElements())
        {
            beans::PropertyValue aPropertyValue = i.nextElement().get<beans::PropertyValue>();
            m_aMap[aPropertyValue.Name] = aPropertyValue.Value;
        }
        return true;
    }

    SAL_WARN("svl", "SfxGrabBagItem::PutValue: wrong type");
    return false;
}

bool SfxGrabBagItem::QueryValue(uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    uno::Sequence<beans::PropertyValue> aValue(m_aMap.size());
    beans::PropertyValue* pValue = aValue.getArray();
    for (std::map<OUString, com::sun::star::uno::Any>::const_iterator i = m_aMap.begin(); i != m_aMap.end(); ++i)
    {
        pValue[0].Name = i->first;
        pValue[0].Value = i->second;
        ++pValue;
    }
    rVal = uno::makeAny(aValue);
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
