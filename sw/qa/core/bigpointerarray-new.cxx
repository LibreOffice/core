/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bigpointerarray-new.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2006-08-14 15:39:23 $
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

#include "bparr.hxx"
#include <algorithm>

BigPtrArray::BigPtrArray()
{
    //container_.reserve(1000);
}

ULONG BigPtrArray::Count() const
{
    return container_.size();
}

void BigPtrArray::Move(ULONG fromPos, ULONG toPos)
{
    DBG_ASSERT(fromPos < container_.size() && toPos < container_.size(), "BigPtrArray.Move precondition violation");
    Insert(container_[fromPos], toPos);
    Remove(toPos < fromPos ? fromPos + 1 : fromPos, 1);
}

void BigPtrArray::ForEach(ULONG fromPos, ULONG toPos, FnForEach fn, void* pArgs)
{
    DBG_ASSERT(fromPos < toPos && fromPos < container_.size() && toPos < container_.size(), "BigPtrArray::ForEach precondition violation");
    Container_t::const_iterator iter = container_.begin() + fromPos;
    Container_t::const_iterator iter_end = container_.begin() + toPos;
    for (/*no init*/; iter != iter_end; ++iter)
        fn(*iter, pArgs);
}

void BigPtrArray::ForEach(FnForEach fn, void* pArgs)
{
    Container_t::const_iterator iter = container_.begin();
    Container_t::const_iterator iter_end = container_.end();
    for ( /*no init*/; iter != iter_end; ++iter)
        fn(*iter, pArgs);
}

ElementPtr BigPtrArray::operator[](ULONG pos) const
{
    DBG_ASSERT(pos < container_.size(), "BigPtrArray::operator[] precondition violation");
    return container_[pos];
}

void BigPtrArray::Insert(const ElementPtr& rElem, ULONG pos)
{
    DBG_ASSERT(pos <= container_.size(), "BigPtrArray::Insert precondition violation");

    rElem->pBigPtrArray_ = this;
    rElem->pos_ = pos;

    if (pos == container_.size())
        container_.push_back(rElem);
    else
    {
        container_.insert(container_.begin() + pos, rElem);
        FixElementIndizes(container_.begin(), container_.end());
    }
}

void BigPtrArray::Remove( ULONG pos, ULONG n )
{
    DBG_ASSERT((pos < container_.size()) && ((container_.begin() + pos + n) < container_.end()), "BigPtrArray.Remove precondition violation")
    container_.erase(container_.begin() + pos, container_.begin() + pos + n);
    FixElementIndizes(container_.begin(), container_.end());
}

void BigPtrArray::Replace(ULONG pos, const ElementPtr& rElem)
{
    DBG_ASSERT(pos < container_.size(), "BigPtrArray::Replace precondition violation");
    rElem->pBigPtrArray_ = this;
    rElem->pos_ = pos;
    container_[pos] = rElem;
}

void BigPtrArray::FixElementIndizes(Container_t::const_iterator begin, Container_t::const_iterator end) const
{
    Container_t::const_iterator iter = begin;
    for (int i = 0; iter != end; ++iter, i++)
        (*iter)->pos_ = i;
}
