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
#include <swrect.hxx>

class SwAccessibleMap;

class SwAccessibleChildSList;

class SwAccessibleChildSList_const_iterator
{
private:
    friend class SwAccessibleChildSList;

    const SwAccessibleChildSList& rList;    // The frame we are iterating over
    sw::access::SwAccessibleChild aCurr;    // The current object
    sal_uInt16 nNextObj;                    // The index of the current sdr object

    inline SwAccessibleChildSList_const_iterator( const SwAccessibleChildSList& rLst )
        : rList( rLst )
        , nNextObj( 0 )
    {}

    SwAccessibleChildSList_const_iterator( const SwAccessibleChildSList& rLst,
                                           SwAccessibleMap& rAccMap );

    SwAccessibleChildSList_const_iterator& next();
    SwAccessibleChildSList_const_iterator& next_visible();

public:

    inline SwAccessibleChildSList_const_iterator( const SwAccessibleChildSList_const_iterator& rIter )
        : rList( rIter.rList )
        , aCurr( rIter.aCurr )
        , nNextObj( rIter.nNextObj )
    {}

    inline sal_Bool operator==( const SwAccessibleChildSList_const_iterator& r ) const
    {
        return aCurr == r.aCurr;
    }

    inline sal_Bool operator!=(
            const SwAccessibleChildSList_const_iterator& r ) const
    {
        return !(*this == r);
    }

    SwAccessibleChildSList_const_iterator& operator++();

    inline const sw::access::SwAccessibleChild& operator*() const
    {
        return aCurr;
    }
};

// An iterator to iterate over a frame's child in any order
class SwAccessibleChildSList
{
    const SwRect maVisArea;
    const SwFrm& mrFrm;
    const sal_Bool mbVisibleChildrenOnly;
    SwAccessibleMap& mrAccMap;

public:

    typedef SwAccessibleChildSList_const_iterator const_iterator;

    inline SwAccessibleChildSList( const SwFrm& rFrm,
                                   SwAccessibleMap& rAccMap )
        : maVisArea()
        , mrFrm( rFrm )
        , mbVisibleChildrenOnly( sal_False )
        , mrAccMap( rAccMap )
    {}

    inline SwAccessibleChildSList( const SwRect& rVisArea,
                                   const SwFrm& rFrm,
                                   SwAccessibleMap& rAccMap )
        : maVisArea( rVisArea )
        , mrFrm( rFrm )
        , mbVisibleChildrenOnly( sw::access::SwAccessibleChild( &rFrm ).IsVisibleChildrenOnly() )
        , mrAccMap( rAccMap )
    {
    }

    inline const_iterator begin() const
    {
        return SwAccessibleChildSList_const_iterator( *this, mrAccMap );
    }

    inline const_iterator end() const
    {
        return SwAccessibleChildSList_const_iterator( *this );
    }

    inline const SwFrm& GetFrm() const
    {
        return mrFrm;
    }

    inline sal_Bool IsVisibleChildrenOnly() const
    {
        return mbVisibleChildrenOnly;
    }

    inline const SwRect& GetVisArea() const
    {
        return maVisArea;
    }

    inline SwAccessibleMap& GetAccMap() const
    {
        return mrAccMap;
    }
};

#endif
