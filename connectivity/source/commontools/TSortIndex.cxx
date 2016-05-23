/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "TSortIndex.hxx"
#include <algorithm>
#include <iterator>
#include <o3tl/functional.hxx>

using namespace connectivity;

/// binary_function Functor object for class OSortIndex::TIntValuePairVector::value_type returntype is bool
struct TKeyValueFunc : ::std::binary_function<OSortIndex::TIntValuePairVector::value_type,OSortIndex::TIntValuePairVector::value_type,bool>
{
    OSortIndex* pIndex;

    explicit TKeyValueFunc(OSortIndex* _pIndex) : pIndex(_pIndex)
    {
    }
    // return false if compared values are equal otherwise true
    inline bool operator()(const OSortIndex::TIntValuePairVector::value_type& lhs,const OSortIndex::TIntValuePairVector::value_type& rhs)   const
    {
        const ::std::vector<OKeyType>& aKeyType = pIndex->getKeyType();
        ::std::vector<OKeyType>::const_iterator aIter = aKeyType.begin();
        for (::std::vector<sal_Int16>::size_type i=0;aIter != aKeyType.end(); ++aIter,++i)
        {
            const bool bGreater = pIndex->getAscending(i) != TAscendingOrder::ASC;
            const bool bLess = !bGreater;

            // compare depending for type
            switch (*aIter)
            {
                case OKeyType::String:
                {
                    sal_Int32 nRes = lhs.second->getKeyString(i).compareTo(rhs.second->getKeyString(i));
                    if (nRes < 0)
                        return bLess;
                    else if (nRes > 0)
                        return bGreater;
                }
                break;
                case OKeyType::Double:
                {
                    double d1 = lhs.second->getKeyDouble(i);
                    double d2 = rhs.second->getKeyDouble(i);

                    if (d1 < d2)
                        return bLess;
                    else if (d1 > d2)
                        return bGreater;
                }
                break;
                case OKeyType::NONE:
                    break;
            }
        }

        // know we know that the values are equal
        return false;
    }
};


::rtl::Reference<OKeySet> OSortIndex::CreateKeySet()
{
    Freeze();

    ::rtl::Reference<OKeySet> pKeySet = new OKeySet();
    pKeySet->get().reserve(m_aKeyValues.size());
    ::std::transform(m_aKeyValues.begin()
                    ,m_aKeyValues.end()
                    ,::std::back_inserter(pKeySet->get())
                    ,::o3tl::select1st<TIntValuePairVector::value_type>());
    pKeySet->setFrozen();
    return pKeySet;
}

OSortIndex::OSortIndex( const ::std::vector<OKeyType>& _aKeyType,
                        const ::std::vector<TAscendingOrder>& _aAscending)
    :m_aKeyType(_aKeyType)
    ,m_aAscending(_aAscending)
    ,m_bFrozen(false)
{
}

OSortIndex::~OSortIndex()
{
}

void OSortIndex::AddKeyValue(OKeyValue * pKeyValue)
{
    assert(pKeyValue && "Can not be null here!");
    if(m_bFrozen)
    {
        m_aKeyValues.push_back(TIntValuePairVector::value_type(pKeyValue->getValue(),nullptr));
        delete pKeyValue;
    }
    else
        m_aKeyValues.push_back(TIntValuePairVector::value_type(pKeyValue->getValue(),pKeyValue));
}

void OSortIndex::Freeze()
{
    OSL_ENSURE(! m_bFrozen,"OSortIndex::Freeze: already frozen!");
    // Sortierung:
    if (m_aKeyType[0] != OKeyType::NONE)
        // we will sort ourself when the first keyType say so
        ::std::sort(m_aKeyValues.begin(),m_aKeyValues.end(),TKeyValueFunc(this));

    TIntValuePairVector::iterator aIter = m_aKeyValues.begin();
    for(;aIter != m_aKeyValues.end();++aIter)
    {
        delete aIter->second;
        aIter->second = nullptr;
    }

    m_bFrozen = true;
}


OKeyValue::OKeyValue(sal_Int32 nVal)
: m_nValue(nVal)
{
}

OKeyValue::~OKeyValue()
{
}

OKeyValue* OKeyValue::createKeyValue(sal_Int32 _nVal)
{
    return new OKeyValue(_nVal);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
