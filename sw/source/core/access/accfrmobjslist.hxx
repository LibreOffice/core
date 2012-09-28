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

    inline bool operator==( const SwAccessibleChildSList_const_iterator& r ) const
    {
        return aCurr == r.aCurr;
    }

    inline bool operator!=(
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
    const bool mbVisibleChildrenOnly;
    SwAccessibleMap& mrAccMap;

public:

    typedef SwAccessibleChildSList_const_iterator const_iterator;

    inline SwAccessibleChildSList( const SwFrm& rFrm,
                                   SwAccessibleMap& rAccMap )
        : maVisArea()
        , mrFrm( rFrm )
        , mbVisibleChildrenOnly( false )
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

    inline bool IsVisibleChildrenOnly() const
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
