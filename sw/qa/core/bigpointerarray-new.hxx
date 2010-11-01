/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _BPARR_HXX
#define _BPARR_HXX

#include <tools/solar.h>
#include <tools/debug.hxx>

#include <deque>

class BigPtrArray;

/** Base class for container entries
*/
class BigPtrEntry
{
    friend class BigPtrArray;
    BigPtrArray* pBigPtrArray_;
    ULONG pos_;

protected:
    BigPtrEntry() : pBigPtrArray_(0), pos_(0)
    {}

    virtual ~BigPtrEntry()
    {}

    ULONG GetPos() const
    {
        return pos_;
    }

    BigPtrArray& GetArray() const
    {
        return *pBigPtrArray_;
    }
};

typedef BigPtrEntry* ElementPtr;
typedef BOOL (*FnForEach)(const ElementPtr&, void* pArgs);

/** A container abstraction
*/
class BigPtrArray
{
public:
    typedef std::deque<ElementPtr> Container_t;

public:
    /**
    */
    BigPtrArray();

    /** Return the number of entries inserted
        into the array
    */
    ULONG Count() const;

    /** Insert an Element into the array at a certain
        position

        @param rElem
        [in] the element

        @param pos
        [in] the position where to insert the element.

        @pre (pos >= 0 && pos <= BigPtrArray.Count())
        @post (((oldCount + 1) == BigPtrArray.Count()) && BigPtrArray[pos] == rElem)
    */
    void Insert(const ElementPtr& rElem, ULONG pos);

    /** Remove a specified number of elements starting at a certain position.

        @param pos
        [in] the position where to start removing elements

        @param n
        [in] the number of elements to remove

        @pre (pos < BigPtrArray.Count() && n <= BigPtrArray.Count())
        @post ((oldCount - n) == BigPtrArray.Count())
    */
    void Remove(ULONG pos, ULONG n = 1);

    /** Move an entry from a certain position to another on.

        @param from
        [in]

        @param to
        [in]
    */
    void Move(ULONG fromPos, ULONG toPos);

    /** Replace an entry at a certain position

        @param pos
        [in] the position of the entry

        @param rElem
        [in] the new entry

        @pre pos < BigPtrArray.Count()
        @post (oldCount == BigPtrArray.Count() && BigPtrArray[pos] == rElem)
    */
    void Replace(ULONG pos, const ElementPtr& rElem);

    /** Get the entry at a certain index

        @param pos
        [in] the position of the entry

        @pre pos < BigPtrArray.Count()
    */
    ElementPtr operator[](ULONG pos) const;

    /**
    */
    void ForEach(FnForEach fn, void* pArgs = NULL);

    /**
    */
    void ForEach(ULONG fromPos, ULONG toPos, FnForEach fn, void* pArgs = NULL);

private:

    void FixElementIndizes(Container_t::const_iterator begin, Container_t::const_iterator end) const;

private:
    Container_t container_;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
