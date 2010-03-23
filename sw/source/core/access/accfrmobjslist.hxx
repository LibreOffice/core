/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _ACCFRMOBJSLIST_HXX
#define _ACCFRMOBJSLIST_HXX

#include <accfrmobj.hxx>

class SwFrmOrObjSList;

class SwFrmOrObjSList_const_iterator
{
    friend class SwFrmOrObjSList;
    const SwFrmOrObjSList& rList;   // The frame we are iterating over
    SwFrmOrObj aCurr;               // The current object
    sal_uInt16 nNextObj;            // The index of the current sdr object

    inline SwFrmOrObjSList_const_iterator( const SwFrmOrObjSList& rLst );
    SwFrmOrObjSList_const_iterator( const SwFrmOrObjSList& rLst, sal_Bool );

//  SwFrmOrObjSList_const_iterator& begin();
    SwFrmOrObjSList_const_iterator& next();
    SwFrmOrObjSList_const_iterator& next_visible();

public:

    inline SwFrmOrObjSList_const_iterator(
            const SwFrmOrObjSList_const_iterator& rIter );
    inline sal_Bool operator==(
            const SwFrmOrObjSList_const_iterator& r ) const;
    inline sal_Bool operator!=(
            const SwFrmOrObjSList_const_iterator& r ) const;
    inline SwFrmOrObjSList_const_iterator& operator++();
    inline const SwFrmOrObj& operator*() const;
};

// An iterator to iterate over a frame's child in any order
class SwFrmOrObjSList
{
    friend class SwFrmOrObjSList_const_iterator;

    SwRect aVisArea;
    const SwFrm *pFrm;  // The frame we are iterating over
    sal_Bool bVisibleOnly;

public:

    typedef SwFrmOrObjSList_const_iterator const_iterator;

    inline SwFrmOrObjSList( const SwFrm *pF );
    inline SwFrmOrObjSList( const SwRect& rVisArea, const SwFrm *pF );

    inline const_iterator begin() const;
    inline const_iterator end() const;
};

inline SwFrmOrObjSList_const_iterator::SwFrmOrObjSList_const_iterator(
        const SwFrmOrObjSList& rLst ) :
    rList( rLst ), nNextObj( 0 )
{
}

inline SwFrmOrObjSList_const_iterator::SwFrmOrObjSList_const_iterator(
        const SwFrmOrObjSList_const_iterator& rIter ) :
    rList( rIter.rList ),
    aCurr( rIter.aCurr ),
    nNextObj( rIter.nNextObj )
{
}

inline sal_Bool SwFrmOrObjSList_const_iterator::operator==(
    const SwFrmOrObjSList_const_iterator& r ) const
{
    return aCurr == r.aCurr;
}

inline sal_Bool SwFrmOrObjSList_const_iterator::operator!=(
    const SwFrmOrObjSList_const_iterator& r ) const
{
    return !(aCurr == r.aCurr);
}

inline SwFrmOrObjSList_const_iterator& SwFrmOrObjSList_const_iterator::operator++()
{
    return rList.bVisibleOnly ? next_visible() : next();
}

inline const SwFrmOrObj& SwFrmOrObjSList_const_iterator::operator*() const
{
    return aCurr;
}

inline SwFrmOrObjSList::SwFrmOrObjSList( const SwFrm *pF ) :
    pFrm( pF ),
    bVisibleOnly( sal_False )
{
}

inline SwFrmOrObjSList::SwFrmOrObjSList( const SwRect& rVisArea,
                                             const SwFrm *pF ) :
    aVisArea( rVisArea ),
    pFrm( pF )
{
    SwFrmOrObj aFrm( pFrm );
    bVisibleOnly = aFrm.IsVisibleChildrenOnly();
}

inline SwFrmOrObjSList_const_iterator SwFrmOrObjSList::begin() const
{
//  SwFrmOrObjSList_const_iterator aIter2( *this );
//  aIter2.begin();
//  return aIter2;
    return SwFrmOrObjSList_const_iterator( *this, sal_True );
}

inline SwFrmOrObjSList_const_iterator SwFrmOrObjSList::end() const
{
    return SwFrmOrObjSList_const_iterator( *this );
}

#endif
