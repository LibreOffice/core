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

#ifndef INCLUDED_UCB_SOURCE_INC_REGEXPMAP_HXX
#define INCLUDED_UCB_SOURCE_INC_REGEXPMAP_HXX

#include "sal/config.h"

#include <list>

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
    inline RegexpMapEntry(OUString const & rTheRegexp,
                          Val * pTheValue):
        m_aRegexp(rTheRegexp), m_pValue(pTheValue) {}

    OUString getRegexp() const { return m_aRegexp; }

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

    inline Entry(Regexp const & rTheRegexp, Val const & rTheValue):
        m_aRegexp(rTheRegexp), m_aValue(rTheValue) {}
};


template< typename Val > class List: public std::list< Entry< Val > > {};


template< typename Val >
struct RegexpMapImpl
{
    List< Val > m_aList[Regexp::KIND_DOMAIN + 1];
    Entry< Val > * m_pDefault;

    RegexpMapImpl(): m_pDefault(0) {}

    ~RegexpMapImpl() { delete m_pDefault; }
};


template< typename Val >
class RegexpMapIterImpl
{
public:
    typedef RegexpMapImpl< Val > MapImpl;
    typedef typename List< Val >::iterator ListIterator;

    // Solaris needs these for the ctor...

    inline RegexpMapIterImpl();

    inline RegexpMapIterImpl(MapImpl * pTheMap, int nTheList,
                             ListIterator aTheIndex);

    RegexpMapIterImpl(RegexpMapImpl< Val > * pTheMap, bool bBegin);

    RegexpMapIterImpl(RegexpMapIterImpl const & rOther);

    RegexpMapIterImpl & operator =(RegexpMapIterImpl const & rOther);

    bool operator ==(RegexpMapIterImpl const & rOther) const;

    RegexpMapImpl< Val > const * getMap() const { return m_pMap; }

    int getList() const { return m_nList; }

    typename List< Val >::iterator const & getIndex() const { return m_aIndex; }

    void next();

    RegexpMapEntry< Val > & get();

private:
    mutable RegexpMapEntry< Val > m_aEntry;
    typename List< Val >::iterator m_aIndex;
    RegexpMapImpl< Val > * m_pMap;
    int m_nList;
    mutable bool m_bEntrySet;

    void setEntry() const;
};

template< typename Val >
inline RegexpMapIterImpl< Val >::RegexpMapIterImpl():
    m_aEntry(rtl::OUString(), 0),
    m_pMap(0),
    m_nList(-1),
    m_bEntrySet(false)
{}

template< typename Val >
inline RegexpMapIterImpl< Val >::RegexpMapIterImpl(MapImpl * pTheMap,
                                                   int nTheList,
                                                   ListIterator aTheIndex):
    m_aEntry(rtl::OUString(), 0),
    m_aIndex(aTheIndex),
    m_pMap(pTheMap),
    m_nList(nTheList),
    m_bEntrySet(false)
{}

template< typename Val >
void RegexpMapIterImpl< Val >::setEntry() const
{
    if (!m_bEntrySet)
    {
        Entry< Val > const & rTheEntry
            = m_nList == -1 ? *m_pMap->m_pDefault : *m_aIndex;
        m_aEntry
            = RegexpMapEntry< Val >(rTheEntry.m_aRegexp.getRegexp(false),
                                    const_cast< Val * >(&rTheEntry.m_aValue));
        m_bEntrySet = true;
    }
}

template< typename Val >
RegexpMapIterImpl< Val >::RegexpMapIterImpl(RegexpMapImpl< Val > * pTheMap,
                                            bool bBegin):
    m_aEntry(rtl::OUString(), 0),
    m_pMap(pTheMap),
    m_bEntrySet(false)
{
    if (bBegin)
    {
        m_nList = -1;
        if (!m_pMap->m_pDefault)
            next();
    }
    else
    {
        m_nList = Regexp::KIND_DOMAIN;
        m_aIndex = m_pMap->m_aList[Regexp::KIND_DOMAIN].end();
    }
}

template< typename Val >
RegexpMapIterImpl< Val >::RegexpMapIterImpl(RegexpMapIterImpl const & rOther):
    m_aEntry(rOther.m_aEntry), m_pMap(rOther.m_pMap), m_nList(rOther.m_nList),
    m_bEntrySet(rOther.m_bEntrySet)
{
    if (m_nList != -1)
        m_aIndex = rOther.m_aIndex;
}

template< typename Val >
RegexpMapIterImpl< Val > & RegexpMapIterImpl< Val >::operator =(
    RegexpMapIterImpl const & rOther)
{
    if (this != &rOther)
    {
        m_aEntry = rOther.m_aEntry;
        m_pMap = rOther.m_pMap;
        m_nList = rOther.m_nList;
        m_bEntrySet = rOther.m_bEntrySet;
        if (m_nList == -1)
            m_aIndex = typename List< Val >::iterator();
        else
            m_aIndex = rOther.m_aIndex;
    }
    return *this;
}

template< typename Val >
bool RegexpMapIterImpl< Val >::operator ==(RegexpMapIterImpl const & rOther)
    const
{
    return m_pMap == rOther.m_pMap
           && m_nList == rOther.m_nList
           && (m_nList == -1 || m_aIndex == rOther.m_aIndex);
}

template< typename Val >
void RegexpMapIterImpl< Val >::next()
{
    switch (m_nList)
    {
        case Regexp::KIND_DOMAIN:
            if (m_aIndex == m_pMap->m_aList[m_nList].end())
                return;
            SAL_FALLTHROUGH;
        default:
            ++m_aIndex;
            if (m_nList == Regexp::KIND_DOMAIN
                || m_aIndex != m_pMap->m_aList[m_nList].end())
                break;
            SAL_FALLTHROUGH;
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
}

template< typename Val >
RegexpMapEntry< Val > & RegexpMapIterImpl< Val >::get()
{
    setEntry();
    return m_aEntry;
}


template< typename Val >
class RegexpMapConstIter
{
    friend class RegexpMap< Val >; // to access m_pImpl, ctor
    friend class RegexpMapIter< Val >; // to access m_pImpl, ctor

public:
    RegexpMapConstIter();

    RegexpMapConstIter(RegexpMapConstIter const & rOther);

    ~RegexpMapConstIter();

    RegexpMapConstIter & operator =(RegexpMapConstIter const & rOther);

    RegexpMapConstIter & operator ++();

    RegexpMapConstIter operator ++(int);

    RegexpMapEntry< Val > const & operator *() const;

    RegexpMapEntry< Val > const * operator ->() const;

    bool equals(RegexpMapConstIter const & rOther) const;
        // for free operator ==(), operator !=()

private:
    RegexpMapIterImpl< Val > * m_pImpl;

    RegexpMapConstIter(RegexpMapIterImpl< Val > * pTheImpl);
};

template< typename Val >
RegexpMapConstIter< Val >::RegexpMapConstIter(RegexpMapIterImpl< Val > *
                                                  pTheImpl):
    m_pImpl(pTheImpl)
{}

template< typename Val >
RegexpMapConstIter< Val >::RegexpMapConstIter():
    m_pImpl(new RegexpMapIterImpl< Val >)
{}

template< typename Val >
RegexpMapConstIter< Val >::RegexpMapConstIter(RegexpMapConstIter const &
                                                  rOther):
    m_pImpl(new RegexpMapIterImpl< Val >(*rOther.m_pImpl))
{}

template< typename Val >
RegexpMapConstIter< Val >::~RegexpMapConstIter()
{
    delete m_pImpl;
}

template< typename Val >
RegexpMapConstIter< Val > &
RegexpMapConstIter< Val >::operator =(RegexpMapConstIter const & rOther)
{
    *m_pImpl = *rOther.m_pImpl;
    return *this;
}

template< typename Val >
RegexpMapConstIter< Val > & RegexpMapConstIter< Val >::operator ++()
{
    m_pImpl->next();
    return *this;
}

template< typename Val >
RegexpMapConstIter< Val > RegexpMapConstIter< Val >::operator ++(int)
{
    RegexpMapConstIter aTemp(*this);
    m_pImpl->next();
    return aTemp;
}

template< typename Val >
RegexpMapEntry< Val > const & RegexpMapConstIter< Val >::operator *() const
{
    return m_pImpl->get();
}

template< typename Val >
RegexpMapEntry< Val > const * RegexpMapConstIter< Val >::operator ->() const
{
    return &m_pImpl->get();
}

template< typename Val >
bool RegexpMapConstIter< Val >::equals(RegexpMapConstIter const & rOther)
    const
{
    return *m_pImpl == *rOther.m_pImpl;
}


template< typename Val >
class RegexpMapIter: public RegexpMapConstIter< Val >
{
    friend class RegexpMap< Val >; // to access ctor

public:
    RegexpMapIter() {}

    RegexpMapIter & operator ++();

    RegexpMapIter operator ++(int);

    RegexpMapEntry< Val > & operator *();

    RegexpMapEntry< Val > const & operator *() const;

    RegexpMapEntry< Val > * operator ->();

    RegexpMapEntry< Val > const * operator ->() const;

private:
    RegexpMapIter(RegexpMapIterImpl< Val > * pTheImpl);
};

template< typename Val >
RegexpMapIter< Val >::RegexpMapIter(RegexpMapIterImpl< Val > * pTheImpl):
    RegexpMapConstIter< Val >(pTheImpl)
{}

template< typename Val >
RegexpMapIter< Val > & RegexpMapIter< Val >::operator ++()
{
    this->m_pImpl->next();
    return *this;
}

template< typename Val >
RegexpMapIter< Val > RegexpMapIter< Val >::operator ++(int)
{
    RegexpMapIter aTemp(*this);
    this->m_pImpl->next();
    return aTemp;
}

template< typename Val >
RegexpMapEntry< Val > & RegexpMapIter< Val >::operator *()
{
    return this->m_pImpl->get();
}

template< typename Val >
RegexpMapEntry< Val > const & RegexpMapIter< Val >::operator *() const
{
    return this->m_pImpl->get();
}

template< typename Val >
RegexpMapEntry< Val > * RegexpMapIter< Val >::operator ->()
{
    return &this->m_pImpl->get();
}

template< typename Val >
RegexpMapEntry< Val > const * RegexpMapIter< Val >::operator ->() const
{
    return &this->m_pImpl->get();
}


template< typename Val >
class RegexpMap
{
public:
    typedef sal_uInt32 size_type;
    typedef RegexpMapIter< Val > iterator;
    typedef RegexpMapConstIter< Val > const_iterator;

    RegexpMap();

    RegexpMap(RegexpMap const & rOther);

    ~RegexpMap();

    RegexpMap & operator =(RegexpMap const & rOther);

    bool add(OUString const & rKey, Val const & rValue, bool bOverwrite,
             OUString * pReverse = nullptr);

    iterator find(OUString const & rKey, OUString * pReverse = nullptr);

    void erase(iterator const & rPos);

    iterator begin();

    const_iterator begin() const;

    iterator end();

    const_iterator end() const;

    size_type size() const;

    Val const * map(OUString const & rString,
                    OUString * pTranslation = nullptr, bool * pTranslated = nullptr)
        const;

private:
    RegexpMapImpl< Val > * m_pImpl;
};

template< typename Val >
RegexpMap< Val >::RegexpMap():
    m_pImpl(new RegexpMapImpl< Val >)
{}

template< typename Val >
RegexpMap< Val >::RegexpMap(RegexpMap const & rOther):
    m_pImpl(new RegexpMapImpl< Val >(*rOther.m_pImpl))
{}

template< typename Val >
RegexpMap< Val >::~RegexpMap()
{
    delete m_pImpl;
}

template< typename Val >
RegexpMap< Val > & RegexpMap< Val >::operator =(RegexpMap const & rOther)
{
    *m_pImpl = *rOther.m_pImpl;
    return *this;
}

template< typename Val >
bool RegexpMap< Val >::add(rtl::OUString const & rKey, Val const & rValue,
                           bool bOverwrite, rtl::OUString * pReverse)
{
    Regexp aRegexp(Regexp::parse(rKey));

    if (aRegexp.isDefault())
    {
        if (m_pImpl->m_pDefault)
        {
            if (!bOverwrite)
                return false;
            delete m_pImpl->m_pDefault;
        }
        m_pImpl->m_pDefault = new Entry< Val >(aRegexp, rValue);
    }
    else
    {
        List< Val > & rTheList = m_pImpl->m_aList[aRegexp.getKind()];

        typename List< Val >::iterator aEnd(rTheList.end());
        for (typename List< Val >::iterator aIt(rTheList.begin()); aIt != aEnd; ++aIt)
        {
            if (aIt->m_aRegexp == aRegexp)
            {
                if (bOverwrite)
                {
                    rTheList.erase(aIt);
                    break;
                }
                else
                    return false;
            }
        }

        rTheList.push_back(Entry< Val >(aRegexp, rValue));
    }

    if (pReverse)
        *pReverse = aRegexp.getRegexp(true);

    return true;
}

template< typename Val >
typename RegexpMap< Val >::iterator RegexpMap< Val >::find(rtl::OUString const & rKey,
                                                  rtl::OUString * pReverse)
{
    Regexp aRegexp(Regexp::parse(rKey));

    if (pReverse)
        *pReverse = aRegexp.getRegexp(true);

    if (aRegexp.isDefault())
    {
        if (m_pImpl->m_pDefault)
            return RegexpMapIter< Val >(new RegexpMapIterImpl< Val >(m_pImpl,
                                                                     true));
    }
    else
    {
        List< Val > & rTheList = m_pImpl->m_aList[aRegexp.getKind()];

        typename List< Val > ::iterator aEnd(rTheList.end());
        for (typename List< Val >::iterator aIt(rTheList.begin()); aIt != aEnd; ++aIt)
            if (aIt->m_aRegexp == aRegexp)
                return RegexpMapIter< Val >(new RegexpMapIterImpl< Val >(
                                                    m_pImpl,
                                                    aRegexp.getKind(), aIt));
    }

    return RegexpMapIter< Val >(new RegexpMapIterImpl< Val >(m_pImpl, false));
}

template< typename Val >
void RegexpMap< Val >::erase(iterator const & rPos)
{
    if (rPos.m_pImpl->getMap() == m_pImpl)
    {
        if (rPos.m_pImpl->getList() == -1)
        {
            if (m_pImpl->m_pDefault)
            {
                delete m_pImpl->m_pDefault;
                m_pImpl->m_pDefault = 0;
            }
        }
        else
            m_pImpl->m_aList[rPos.m_pImpl->getList()].
                         erase(rPos.m_pImpl->getIndex());
    }
}

template< typename Val >
typename RegexpMap< Val >::iterator RegexpMap< Val >::begin()
{
    return RegexpMapIter< Val >(new RegexpMapIterImpl< Val >(m_pImpl, true));
}

template< typename Val >
typename RegexpMap< Val >::const_iterator RegexpMap< Val >::begin() const
{
    return RegexpMapConstIter< Val >(new RegexpMapIterImpl< Val >(m_pImpl,
                                                                  true));
}

template< typename Val >
typename RegexpMap< Val >::iterator RegexpMap< Val >::end()
{
    return RegexpMapIter< Val >(new RegexpMapIterImpl< Val >(m_pImpl, false));
}

template< typename Val >
typename RegexpMap< Val >::const_iterator RegexpMap< Val >::end() const
{
    return RegexpMapConstIter< Val >(new RegexpMapIterImpl< Val >(m_pImpl,
                                                                  false));
}

template< typename Val >
typename RegexpMap< Val >::size_type RegexpMap< Val >::size() const
{
    return (m_pImpl->m_pDefault ? 1 : 0)
               + m_pImpl->m_aList[Regexp::KIND_PREFIX].size()
               + m_pImpl->m_aList[Regexp::KIND_AUTHORITY].size()
               + m_pImpl->m_aList[Regexp::KIND_DOMAIN].size();
}

template< typename Val >
Val const * RegexpMap< Val >::map(rtl::OUString const & rString,
                                  rtl::OUString * pTranslation,
                                  bool * pTranslated) const
{
    for (int n = Regexp::KIND_DOMAIN; n >= Regexp::KIND_PREFIX; --n)
    {
        List< Val > const & rTheList = m_pImpl->m_aList[n];

        typename List< Val >::const_iterator aEnd(rTheList.end());
        for (typename List< Val >::const_iterator aIt(rTheList.begin()); aIt != aEnd;
             ++aIt)
            if (aIt->m_aRegexp.matches(rString, pTranslation, pTranslated))
                return &aIt->m_aValue;
    }
    if (m_pImpl->m_pDefault
        && m_pImpl->m_pDefault->m_aRegexp.matches(rString, pTranslation,
                                                  pTranslated))
        return &m_pImpl->m_pDefault->m_aValue;
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

#endif // INCLUDED_UCB_SOURCE_INC_REGEXPMAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
