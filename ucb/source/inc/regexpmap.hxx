/*************************************************************************
 *
 *  $RCSfile: regexpmap.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: kso $ $Date: 2000-10-16 14:53:07 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _UCB_REGEXPMAP_HXX_
#define _UCB_REGEXPMAP_HXX_

#ifndef _RTL_OUSTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

namespace ucb {

template< typename Val > class RegexpMap;
template< typename Val > class RegexpMapIter;

//============================================================================
template< typename Val >
class RegexpMapEntry
{
public:
    inline RegexpMapEntry(rtl::OUString const & rTheRegexp,
                          Val * pTheValue):
        m_aRegexp(rTheRegexp), m_pValue(pTheValue) {}

    rtl::OUString getRegexp() const { return m_aRegexp; }

    Val const & getValue() const { return *m_pValue; }

    Val & getValue() { return *m_pValue; }

private:
    rtl::OUString m_aRegexp;
    Val * m_pValue;
};

//============================================================================
template< typename Val > class RegexpMapIterImpl;
    // MSC doesn't like this to be a private RegexpMapConstIter member
    // class...

template< typename Val >
class RegexpMapConstIter
{
    friend RegexpMap< Val >; // to access m_pImpl, ctor
    friend RegexpMapIter< Val >; // to access m_pImpl, ctor

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
    friend RegexpMap< Val >; // to access ctor

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

    bool add(rtl::OUString const & rKey, Val const & rValue, bool bOverwrite,
             rtl::OUString * pReverse = 0);
        // throws com::sun::star::lang::IllegalArgumentException

    iterator find(rtl::OUString const & rKey, rtl::OUString * pReverse = 0);
        // throws com::sun::star::lang::IllegalArgumentException

    void erase(iterator const & rPos);

    iterator begin();

    const_iterator begin() const;

    iterator end();

    const_iterator end() const;

    bool empty() const;

    size_type size() const;

    Val const * map(rtl::OUString const & rString,
                    rtl::OUString * pTranslation = 0, bool * pTranslated = 0)
        const;

private:
    RegexpMapImpl< Val > * m_pImpl;
};

}

//============================================================================
template< typename Val >
inline bool operator ==(ucb::RegexpMapConstIter< Val > const & rIter1,
                        ucb::RegexpMapConstIter< Val > const & rIter2)
{
    return rIter1.equals(rIter2);
}

template< typename Val >
inline bool operator !=(ucb::RegexpMapConstIter< Val > const & rIter1,
                        ucb::RegexpMapConstIter< Val > const & rIter2)
{
    return !rIter1.equals(rIter2);
}

#endif // _UCB_REGEXPMAP_HXX_

