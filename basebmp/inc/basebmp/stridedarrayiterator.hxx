/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: stridedarrayiterator.hxx,v $
 * $Revision: 1.6 $
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

#ifndef INCLUDED_BASEBMP_STRIDEDARRAYITERATOR_HXX
#define INCLUDED_BASEBMP_STRIDEDARRAYITERATOR_HXX

#include <basebmp/metafunctions.hxx>

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

#endif /* INCLUDED_BASEBMP_STRIDEDARRAYITERATOR_HXX */
