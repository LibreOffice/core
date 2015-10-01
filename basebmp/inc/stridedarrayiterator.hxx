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

#ifndef INCLUDED_BASEBMP_INC_STRIDEDARRAYITERATOR_HXX
#define INCLUDED_BASEBMP_INC_STRIDEDARRAYITERATOR_HXX

#include <metafunctions.hxx>

namespace basebmp
{

/** Like vigra::StridedArrayIterator

    Changed semantics re. DirectionSelector<StridedArrayTag>: stride
    now counts in <em>raw</em> bytes

    Adapts given ptr, in a way that iterator increments move forward
    in strided steps. Stride can, by the way, also be negative
 */
template< typename T > class StridedArrayIterator
{
public:
    typedef typename clone_const<T, unsigned char>::type  internal_type;

    /** Create iterator

        @param stride

        Stride in bytes. Given value should better match memory layout
        of T, as memory gets reinterpret-casted.
     */
    explicit StridedArrayIterator(int stride, T* ptr = 0) :
        stride_(stride),
        current_(reinterpret_cast<internal_type*>(ptr))
    {}

    /** Copy from other StridedArrayIterator, plus given offset

        @param offset
        Offset in bytes
     */
    StridedArrayIterator( StridedArrayIterator const& rSrc,
                          int                         offset ) :
        stride_(rSrc.stride_),
        current_(reinterpret_cast<internal_type*>(
                     reinterpret_cast<T*>(rSrc.current_)+offset))
    {}

    void operator++()       {current_ += stride_; }
    void operator++(int)    {current_ += stride_; }
    void operator--()       {current_ -= stride_; }
    void operator--(int)    {current_ -= stride_; }
    void operator+=(int dy) {current_ += dy*stride_; }
    void operator-=(int dy) {current_ -= dy*stride_; }

    int operator-(StridedArrayIterator const & rhs) const
    { return (current_ - rhs.current_) / stride_; }

    bool operator==(StridedArrayIterator const & rhs) const
    { return current_ == rhs.current_; }

    bool operator!=(StridedArrayIterator const & rhs) const
    { return current_ != rhs.current_; }

    bool operator<(StridedArrayIterator const & rhs) const
    { return *this - rhs < 0; }

    bool operator<=(StridedArrayIterator const & rhs) const
    { return *this - rhs <= 0; }

    bool operator>(StridedArrayIterator const & rhs) const
    { return *this - rhs > 0; }

    bool operator>=(StridedArrayIterator const & rhs) const
    { return *this - rhs >= 0; }

    T* operator()() const
    { return reinterpret_cast<T*>(current_); }

    T* operator()(int d) const
    { return reinterpret_cast<T*>(current_ + d*stride_); }

private:
    int            stride_;
    internal_type* current_;
};

} // namespace basebmp

#endif /* INCLUDED_BASEBMP_INC_STRIDEDARRAYITERATOR_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
