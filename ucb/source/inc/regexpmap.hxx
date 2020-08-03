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

#pragma once

#include <sal/config.h>

#include <vector>
#include <memory>

#include <rtl/ustring.hxx>
#include <sal/types.h>

#include "regexp.hxx"

namespace ucb_impl {

template< typename Val > class RegexpMap;
template< typename Val > class RegexpMapIter;


template< typename Val >
class RegexpMapEntry
{
public:
    RegexpMapEntry(OUString const & rTheRegexp,
                          Val * pTheValue):
        m_aRegexp(rTheRegexp), m_pValue(pTheValue) {}

    const OUString& getRegexp() const { return m_aRegexp; }

    Val const & getValue() const { return *m_pValue; }

    Val & getValue() { return *m_pValue; }

private:
    OUString m_aRegexp;
    Val * m_pValue;
};


template< typename Val >
struct Entry
{
    Regexp m_aRegexp;
    Val m_aValue;

    Entry(Regexp const & rTheRegexp, Val const & rTheValue):
        m_aRegexp(rTheRegexp), m_aValue(rTheValue) {}
};


template< typename Val >
class RegexpMapConstIter
{
    friend class RegexpMap< Val >; // to access m_pImpl, ctor
    friend class RegexpMapIter< Val >; // to access m_pImpl, ctor

public:
    typedef typename std::vector< Entry< Val > >::iterator ListIterator;

    RegexpMapConstIter();

    RegexpMapConstIter(RegexpMap< Val > * pTheMap, bool bBegin);

    RegexpMapConstIter(RegexpMap< Val > * pTheMap, int nTheList,
                             ListIterator aTheIndex);

    RegexpMapConstIter(RegexpMapConstIter const & rOther);

    RegexpMapConstIter & operator =(RegexpMapConstIter const & rOther);

    RegexpMapConstIter & operator ++();

    RegexpMapEntry< Val > const * operator ->() const;

    bool equals(RegexpMapConstIter const & rOther) const;
        // for free operator ==(), operator !=()

protected:
    RegexpMapEntry< Val > & get() const;

private:
    mutable RegexpMapEntry< Val > m_aEntry;
    typename std::vector< Entry< Val > >::iterator m_aIndex;
    RegexpMap< Val > * m_pMap;
    int m_nList;
    mutable bool m_bEntrySet;
};

template< typename Val >
RegexpMapConstIter< Val >::RegexpMapConstIter():
    m_aEntry(OUString(), 0),
    m_pMap(nullptr),
    m_nList(-1),
    m_bEntrySet(false)
{}

template< typename Val >
RegexpMapConstIter< Val >::RegexpMapConstIter(RegexpMap< Val > * pTheMap,
                                            bool bBegin):
    m_aEntry(OUString(), 0),
    m_pMap(pTheMap),
    m_bEntrySet(false)
{
    if (bBegin)
    {
        m_nList = -1;
        if (!m_pMap->m_pDefault)
            operator++();
    }
    else
    {
        m_nList = Regexp::KIND_DOMAIN;
        m_aIndex = m_pMap->m_aList[Regexp::KIND_DOMAIN].end();
    }
}

template< typename Val >
inline RegexpMapConstIter< Val >::RegexpMapConstIter(RegexpMap< Val > * pTheMap,
                                                   int nTheList,
                                                   ListIterator aTheIndex):
    m_aEntry(OUString(), 0),
    m_aIndex(aTheIndex),
    m_pMap(pTheMap),
    m_nList(nTheList),
    m_bEntrySet(false)
{}

template< typename Val >
RegexpMapConstIter< Val >::RegexpMapConstIter(RegexpMapConstIter const &
                                                  rOther):
    m_aEntry(rOther.m_aEntry), m_pMap(rOther.m_pMap), m_nList(rOther.m_nList),
    m_bEntrySet(rOther.m_bEntrySet)
{
    if (m_nList != -1)
        m_aIndex = rOther.m_aIndex;
}

template< typename Val >
RegexpMapConstIter< Val > &
RegexpMapConstIter< Val >::operator =(RegexpMapConstIter const & rOther)
{
    if (this != &rOther)
    {
        m_aEntry = rOther.m_aEntry;
        m_pMap = rOther.m_pMap;
        m_nList = rOther.m_nList;
        m_bEntrySet = rOther.m_bEntrySet;
        if (m_nList == -1)
            m_aIndex = typename std::vector< Entry<Val> >::iterator();
        else
            m_aIndex = rOther.m_aIndex;
    }
    return *this;
}

template< typename Val >
RegexpMapConstIter< Val > & RegexpMapConstIter< Val >::operator ++()
{
    switch (m_nList)
    {
        case Regexp::KIND_DOMAIN:
            if (m_aIndex == m_pMap->m_aList[m_nList].end())
                return *this;
            [[fallthrough]];
        default:
            ++m_aIndex;
            if (m_nList == Regexp::KIND_DOMAIN
                || m_aIndex != m_pMap->m_aList[m_nList].end())
                break;
            [[fallthrough]];
        case -1:
            do
            {
                ++m_nList;
                m_aIndex = m_pMap->m_aList[m_nList].begin();
            }
            while (m_nList < Regexp::KIND_DOMAIN
                   && m_aIndex == m_pMap->m_aList[m_nList].end());
            break;
    }
    m_bEntrySet = false;
    return *this;
}

template< typename Val >
RegexpMapEntry< Val > & RegexpMapConstIter< Val >::get() const
{
    if (!m_bEntrySet)
    {
        Entry< Val > const & rTheEntry
            = m_nList == -1 ? *m_pMap->m_pDefault : *m_aIndex;
        m_aEntry
            = RegexpMapEntry< Val >(rTheEntry.m_aRegexp.getRegexp(),
                                    const_cast< Val * >(&rTheEntry.m_aValue));
        m_bEntrySet = true;
    }
    return m_aEntry;
}

template< typename Val >
RegexpMapEntry< Val > const * RegexpMapConstIter< Val >::operator ->() const
{
    return &get();
}

template< typename Val >
bool RegexpMapConstIter< Val >::equals(RegexpMapConstIter const & rOther)
    const
{
    return m_pMap == rOther.m_pMap
           && m_nList == rOther.m_nList
           && (m_nList == -1 || m_aIndex == rOther.m_aIndex);
}


template< typename Val >
class RegexpMapIter: public RegexpMapConstIter< Val >
{
    friend class RegexpMap< Val >; // to access ctor

public:
    RegexpMapIter() {}

    RegexpMapIter(RegexpMap< Val > * pTheMap, bool bBegin):
        RegexpMapConstIter<Val>(pTheMap, bBegin)
    {}

    RegexpMapIter(RegexpMap< Val > * pTheMap, int nTheList, typename RegexpMapConstIter< Val >::ListIterator aTheIndex):
        RegexpMapConstIter<Val>(pTheMap, nTheList, aTheIndex)
    {}

    RegexpMapEntry< Val > * operator ->();

    RegexpMapEntry< Val > const * operator ->() const;
};

template< typename Val >
RegexpMapEntry< Val > * RegexpMapIter< Val >::operator ->()
{
    return &RegexpMapConstIter<Val>::get();
}

template< typename Val >
RegexpMapEntry< Val > const * RegexpMapIter< Val >::operator ->() const
{
    return &RegexpMapConstIter<Val>::get();
}


template< typename Val >
class RegexpMap
{
friend class RegexpMapConstIter<Val>;
public:
    typedef sal_uInt32 size_type;
    typedef RegexpMapIter< Val > iterator;
    typedef RegexpMapConstIter< Val > const_iterator;

    void add(OUString const & rKey, Val const & rValue);

    iterator find(OUString const & rKey);

    void erase(iterator const & rPos);

    iterator begin();

    const_iterator begin() const;

    iterator end();

    const_iterator end() const;

    size_type size() const;

    Val const * map(OUString const & rString) const;

private:
    std::vector< Entry<Val> > m_aList[Regexp::KIND_DOMAIN + 1];
    std::unique_ptr<Entry< Val >> m_pDefault;
};

template< typename Val >
void RegexpMap< Val >::add(OUString const & rKey, Val const & rValue)
{
    Regexp aRegexp(Regexp::parse(rKey));

    if (aRegexp.isDefault())
    {
        if (m_pDefault)
        {
            return;
        }
        m_pDefault.reset( new Entry< Val >(aRegexp, rValue) );
    }
    else
    {
        std::vector< Entry<Val> > & rTheList = m_aList[aRegexp.getKind()];

        for (auto const& elem : rTheList)
        {
            if (elem.m_aRegexp == aRegexp)
            {
               return;
            }
        }

        rTheList.push_back(Entry< Val >(aRegexp, rValue));
    }
}

template< typename Val >
typename RegexpMap< Val >::iterator RegexpMap< Val >::find(OUString const & rKey)
{
    Regexp aRegexp(Regexp::parse(rKey));

    if (aRegexp.isDefault())
    {
        if (m_pDefault)
            return RegexpMapIter< Val >(this, true);
    }
    else
    {
        std::vector< Entry<Val> > & rTheList = m_aList[aRegexp.getKind()];

        typename std::vector< Entry<Val> >::iterator aEnd(rTheList.end());
        for (typename std::vector< Entry<Val> >::iterator aIt(rTheList.begin()); aIt != aEnd; ++aIt)
            if (aIt->m_aRegexp == aRegexp)
                return RegexpMapIter< Val >(this, aRegexp.getKind(), aIt);
    }

    return RegexpMapIter< Val >(this, false);
}

template< typename Val >
void RegexpMap< Val >::erase(iterator const & rPos)
{
    assert(rPos.m_pMap == this);
    if (rPos.m_pMap == this)
    {
        if (rPos.m_nList == -1)
        {
            m_pDefault.reset();
        }
        else
            m_aList[rPos.m_nList].erase(rPos.m_aIndex);
    }
}

template< typename Val >
typename RegexpMap< Val >::iterator RegexpMap< Val >::begin()
{
    return RegexpMapIter< Val >(this, true);
}

template< typename Val >
typename RegexpMap< Val >::const_iterator RegexpMap< Val >::begin() const
{
    return RegexpMapConstIter< Val >(this, true);
}

template< typename Val >
typename RegexpMap< Val >::iterator RegexpMap< Val >::end()
{
    return RegexpMapIter< Val >(this, false);
}

template< typename Val >
typename RegexpMap< Val >::const_iterator RegexpMap< Val >::end() const
{
    return RegexpMapConstIter< Val >(this, false);
}

template< typename Val >
typename RegexpMap< Val >::size_type RegexpMap< Val >::size() const
{
    return (m_pDefault ? 1 : 0)
               + m_aList[Regexp::KIND_PREFIX].size()
               + m_aList[Regexp::KIND_AUTHORITY].size()
               + m_aList[Regexp::KIND_DOMAIN].size();
}

template< typename Val >
Val const * RegexpMap< Val >::map(OUString const & rString) const
{
    for (int n = Regexp::KIND_DOMAIN; n >= Regexp::KIND_PREFIX; --n)
    {
        std::vector< Entry<Val> > const & rTheList = m_aList[n];

        for (auto const & rItem : rTheList)
            if (rItem.m_aRegexp.matches(rString))
                return &rItem.m_aValue;
    }
    if (m_pDefault
        && m_pDefault->m_aRegexp.matches(rString))
        return &m_pDefault->m_aValue;
    return 0;
}

}


template< typename Val >
inline bool operator ==(ucb_impl::RegexpMapConstIter< Val > const & rIter1,
                        ucb_impl::RegexpMapConstIter< Val > const & rIter2)
{
    return rIter1.equals(rIter2);
}

template< typename Val >
inline bool operator !=(ucb_impl::RegexpMapConstIter< Val > const & rIter1,
                        ucb_impl::RegexpMapConstIter< Val > const & rIter2)
{
    return !rIter1.equals(rIter2);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
