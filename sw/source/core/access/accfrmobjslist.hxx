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

#ifndef INCLUDED_SW_SOURCE_CORE_ACCESS_ACCFRMOBJSLIST_HXX
#define INCLUDED_SW_SOURCE_CORE_ACCESS_ACCFRMOBJSLIST_HXX

#include "accfrmobj.hxx"
#include <swrect.hxx>

class SwAccessibleMap;
class SwAccessibleChildSList;

class SwAccessibleChildSList_const_iterator
{
private:
    friend class SwAccessibleChildSList;

    const SwAccessibleChildSList& m_rList;    // The frame we are iterating over
    sw::access::SwAccessibleChild m_aCurr;    // The current object
    size_t m_nNextObj;                        // The index of the current sdr object

    SwAccessibleChildSList_const_iterator( const SwAccessibleChildSList& rLst )
        : m_rList( rLst )
        , m_nNextObj( 0 )
    {}

    SwAccessibleChildSList_const_iterator( const SwAccessibleChildSList& rLst,
                                           SwAccessibleMap& rAccMap );

    SwAccessibleChildSList_const_iterator& next();
    SwAccessibleChildSList_const_iterator& next_visible();

public:
    bool operator==( const SwAccessibleChildSList_const_iterator& r ) const
    {
        return m_aCurr == r.m_aCurr;
    }

    SwAccessibleChildSList_const_iterator& operator++();

    const sw::access::SwAccessibleChild& operator*() const
    {
        return m_aCurr;
    }
};

// An iterator to iterate over a frame's child in any order
class SwAccessibleChildSList
{
    const SwRect maVisArea;
    const SwFrame& mrFrame;
    const bool mbVisibleChildrenOnly;
    SwAccessibleMap& mrAccMap;

public:
    typedef SwAccessibleChildSList_const_iterator const_iterator;

    SwAccessibleChildSList( const SwFrame& rFrame,
                                   SwAccessibleMap& rAccMap )
        : maVisArea()
        , mrFrame( rFrame )
        , mbVisibleChildrenOnly( false )
        , mrAccMap( rAccMap )
    {}

    SwAccessibleChildSList( const SwRect& rVisArea,
                                   const SwFrame& rFrame,
                                   SwAccessibleMap& rAccMap )
        : maVisArea( rVisArea )
        , mrFrame( rFrame )
        , mbVisibleChildrenOnly( sw::access::SwAccessibleChild( &rFrame ).IsVisibleChildrenOnly() )
        , mrAccMap( rAccMap )
    {
    }

    const_iterator begin() const
    {
        return SwAccessibleChildSList_const_iterator( *this, mrAccMap );
    }

    const_iterator end() const
    {
        return SwAccessibleChildSList_const_iterator( *this );
    }

    const SwFrame& GetFrame() const
    {
        return mrFrame;
    }

    bool IsVisibleChildrenOnly() const
    {
        return mbVisibleChildrenOnly;
    }

    const SwRect& GetVisArea() const
    {
        return maVisArea;
    }

    SwAccessibleMap& GetAccMap() const
    {
        return mrAccMap;
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
