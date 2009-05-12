/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: pixeliterator.hxx,v $
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

#ifndef INCLUDED_BASEBMP_PIXELITERATOR_HXX
#define INCLUDED_BASEBMP_PIXELITERATOR_HXX

#include <basebmp/metafunctions.hxx>
#include <basebmp/stridedarrayiterator.hxx>

#include <vigra/metaprogramming.hxx>
#include <vigra/diff2d.hxx>

namespace basebmp
{

template< typename Valuetype > class PixelColumnIterator
{
public:
    typedef Valuetype                           value_type;
    typedef Valuetype&                          reference;
    typedef reference                           index_reference;
    typedef Valuetype*                          pointer;
    typedef int                                 difference_type;
    typedef image_traverser_tag                 iterator_category;

    typedef StridedArrayIterator< value_type >  MoveY;

private:
    MoveY           y;

    bool equal( PixelColumnIterator const & rhs ) const
    {
        return rhs.y == y;
    }

    bool less( PixelColumnIterator const & rhs ) const
    {
        return y < rhs.y;
    }

public:
    PixelColumnIterator() :
        y(0)
    {}

    explicit PixelColumnIterator( const MoveY& pos ) :
        y(pos)
    {}

    PixelColumnIterator( const MoveY& pos, int x ) :
        y(pos,x)
    {}

    PixelColumnIterator& operator+=( difference_type d )
    {
        y += d;
        return *this;
    }

    PixelColumnIterator& operator-=( difference_type d )
    {
        y -= d;
        return *this;
    }

    PixelColumnIterator operator+( difference_type d )
    {
        PixelColumnIterator res(*this);
        res += d;
        return res;
    }

    PixelColumnIterator operator-( difference_type d )
    {
        PixelColumnIterator res(*this);
        res -= d;
        return res;
    }

    PixelColumnIterator& operator++()
    {
        ++y;
        return *this;
    }

    PixelColumnIterator& operator--()
    {
        --y;
        return *this;
    }

    PixelColumnIterator operator++(int)
    {
        PixelColumnIterator res(*this);
        ++y;
        return res;
    }

    PixelColumnIterator operator--(int)
    {
        PixelColumnIterator res(*this);
        --y;
        return res;
    }

    bool operator==(PixelColumnIterator const & rhs) const
    {
        return equal( rhs );
    }

    bool operator!=(PixelColumnIterator const & rhs) const
    {
        return !equal( rhs );
    }

    bool operator<(PixelColumnIterator const & rhs) const
    {
        return less(rhs);
    }

    bool operator<=(PixelColumnIterator const & rhs) const
    {
        return !rhs.less(*this);
    }

    bool operator>(PixelColumnIterator const & rhs) const
    {
        return rhs.less(*this);
    }

    bool operator>=(PixelColumnIterator const & rhs) const
    {
        return !less(rhs);
    }

    difference_type operator-(PixelColumnIterator const & rhs) const
    {
        return y - rhs.y;
    }

    value_type get() const
    {
        return *y();
    }

    value_type get(difference_type d) const
    {
        return *y(d);
    }

    void set( value_type v ) const
    {
        *y() = v;
    }

    void set( value_type v, difference_type d ) const
    {
        *y(d) = v;
    }

    reference operator*() const
    {
        return *y();
    }

    pointer operator->() const
    {
        return y();
    }

    reference operator[](difference_type d) const
    {
        return *y(d);
    }

    reference operator()(int dy) const
    {
        return *y(dy);
    }
};

template< typename Valuetype > class PixelIterator
{
public:
    typedef Valuetype                          value_type;
    typedef Valuetype&                         reference;
    typedef reference                          index_reference;
    typedef Valuetype*                         pointer;
    typedef vigra::Diff2D                      difference_type;
    typedef image_traverser_tag                iterator_category;
    typedef pointer                            row_iterator;
    typedef PixelColumnIterator<value_type>    column_iterator;

    typedef int                                MoveX;
    typedef StridedArrayIterator< value_type > MoveY;

    // TODO(F2): direction of iteration (ImageIterator can be made to
    // run backwards)

private:
    bool equal(PixelIterator const & rhs) const
    {
        return (x == rhs.x) && (y == rhs.y);
    }

    pointer current() const
    {
        return y() + x;
    }

    pointer current(int dx, int dy) const
    {
        return y(dy) + x+dx;
    }

public:
    PixelIterator() :
        x(0),
        y(0)
    {}

    PixelIterator(pointer base, int ystride) :
        x(0),
        y(ystride,base)
    {}

    bool operator==(PixelIterator const & rhs) const
    {
        return equal(rhs);
    }

    bool operator!=(PixelIterator const & rhs) const
    {
        return !equal(rhs);
    }

    difference_type operator-(PixelIterator const & rhs) const
    {
        return difference_type(x - rhs.x, y - rhs.y);
    }

    MoveX x;
    MoveY y;

    PixelIterator & operator+=(difference_type const & s)
    {
        x += s.x;
        y += s.y;
        return *this;
    }

    PixelIterator & operator-=(difference_type const & s)
    {
        x -= s.x;
        y -= s.y;
        return *this;
    }

    PixelIterator operator+(difference_type const & s) const
    {
        PixelIterator ret(*this);
        ret += s;
        return ret;
    }

    PixelIterator operator-(difference_type const & s) const
    {
        PixelIterator ret(*this);
        ret -= s;
        return ret;
    }

    row_iterator rowIterator() const
    {
        return row_iterator(y()+x);
    }

    column_iterator columnIterator() const
    {
        return column_iterator(y,x);
    }

    value_type get() const
    {
        return *current();
    }

    value_type get(difference_type const & d) const
    {
        return *current(d.y, d.x);
    }

    void set( value_type v ) const
    {
        *current() = v;
    }

    void set( value_type v, difference_type const & d ) const
    {
        *current(d.y,d.x) = v;
    }

    reference operator*() const
    {
        return *current();
    }

    pointer operator->() const
    {
        return current();
    }

    reference operator[]( const vigra::Diff2D& d ) const
    {
        return *current(d.x,d.y);
    }

    reference operator()(int dx, int dy) const
    {
        return *current(dx,dy);
    }

    pointer operator[](int dy) const
    {
        return y(dy) + x;
    }
};

} // namespace basebmp

#endif /* INCLUDED_BASEBMP_PIXELITERATOR_HXX */
