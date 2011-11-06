/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
