/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bigpointerarray-new.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2006-08-14 15:39:36 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _BPARR_HXX
#define _BPARR_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif

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
