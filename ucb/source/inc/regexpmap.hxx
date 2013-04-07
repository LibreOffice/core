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

#ifndef _UCB_REGEXPMAP_HXX_
#define _UCB_REGEXPMAP_HXX_

#include <rtl/ustring.hxx>
#include <sal/types.h>

namespace ucb_impl {

template< typename Val > class RegexpMap;
template< typename Val > class RegexpMapIter;

//============================================================================
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

//============================================================================
template< typename Val > class RegexpMapIterImpl;
    // MSC doesn't like this to be a private RegexpMapConstIter member
    // class...

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

//============================================================================
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

//============================================================================
template< typename Val > struct RegexpMapImpl;
    // MSC doesn't like this to be a RegexpMap member class...

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
             OUString * pReverse = 0);
        // throws com::sun::star::lang::IllegalArgumentException

    iterator find(OUString const & rKey, OUString * pReverse = 0);
        // throws com::sun::star::lang::IllegalArgumentException

    void erase(iterator const & rPos);

    iterator begin();

    const_iterator begin() const;

    iterator end();

    const_iterator end() const;

    bool empty() const;

    size_type size() const;

    Val const * map(OUString const & rString,
                    OUString * pTranslation = 0, bool * pTranslated = 0)
        const;

private:
    RegexpMapImpl< Val > * m_pImpl;
};

}

//============================================================================
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

#endif // _UCB_REGEXPMAP_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
