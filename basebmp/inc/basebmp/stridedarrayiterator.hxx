/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stridedarrayiterator.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 13:47:26 $
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

#ifndef INCLUDED_BASEBMP_STRIDEDARRAYITERATOR_HXX
#define INCLUDED_BASEBMP_STRIDEDARRAYITERATOR_HXX

#include <basebmp/metafunctions.hxx>

namespace basebmp
{

// changed semantics re. DirectionSelector<StridedArrayTag>: stride
// now counts in <em>raw</em> bytes!
template< typename T > class StridedArrayIterator
{
public:
    typedef typename clone_const<T, unsigned char>::type  internal_type;

    StridedArrayIterator(int stride, T* ptr = 0) :
        stride_(stride),
        current_(reinterpret_cast<internal_type*>(ptr))
    {}

    /// Copy from other StridedArrayIterator, plus given offset
    StridedArrayIterator( StridedArrayIterator const& rSrc,
                          int                         offset ) :
        stride_(rSrc.stride_),
        current_(reinterpret_cast<internal_type*>(
                     reinterpret_cast<T*>(rSrc.current_)+offset))
    {}

    void operator++()       { current_ += stride_; }
    void operator++(int)    {current_ += stride_; }
    void operator--()       {current_ -= stride_; }
    void operator--(int)    {current_ -= stride_; }
    void operator+=(int dy) {current_ += dy*stride_; }
    void operator-=(int dy) {current_ -= dy*stride_; }

    bool operator==(StridedArrayIterator const & rhs) const
    { return current_ == rhs.current_; }

    bool operator!=(StridedArrayIterator const & rhs) const
    { return current_ != rhs.current_; }

    bool operator<(StridedArrayIterator const & rhs) const
    { return current_ < rhs.current_; }

    bool operator<=(StridedArrayIterator const & rhs) const
    { return current_ <= rhs.current_; }

    bool operator>(StridedArrayIterator const & rhs) const
    { return current_ > rhs.current_; }

    bool operator>=(StridedArrayIterator const & rhs) const
    { return current_ >= rhs.current_; }

    int operator-(StridedArrayIterator const & rhs) const
    { return (current_ - rhs.current_) / stride_; }

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
