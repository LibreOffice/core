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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include "bparr.hxx"
#include <algorithm>

BigPtrArray::BigPtrArray()
{
    //container_.reserve(1000);
}

sal_uLong BigPtrArray::Count() const
{
    return container_.size();
}

void BigPtrArray::Move(sal_uLong fromPos, sal_uLong toPos)
{
    DBG_ASSERT(fromPos < container_.size() && toPos < container_.size(), "BigPtrArray.Move precondition violation");
    Insert(container_[fromPos], toPos);
    Remove(toPos < fromPos ? fromPos + 1 : fromPos, 1);
}

void BigPtrArray::ForEach(sal_uLong fromPos, sal_uLong toPos, FnForEach fn, void* pArgs)
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

ElementPtr BigPtrArray::operator[](sal_uLong pos) const
{
    DBG_ASSERT(pos < container_.size(), "BigPtrArray::operator[] precondition violation");
    return container_[pos];
}

void BigPtrArray::Insert(const ElementPtr& rElem, sal_uLong pos)
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

void BigPtrArray::Remove( sal_uLong pos, sal_uLong n )
{
    DBG_ASSERT((pos < container_.size()) && ((container_.begin() + pos + n) < container_.end()), "BigPtrArray.Remove precondition violation")
    container_.erase(container_.begin() + pos, container_.begin() + pos + n);
    FixElementIndizes(container_.begin(), container_.end());
}

void BigPtrArray::Replace(sal_uLong pos, const ElementPtr& rElem)
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
