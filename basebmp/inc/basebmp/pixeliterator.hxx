/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pixeliterator.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: thb $ $Date: 2006-06-02 08:36:14 $
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

#ifndef INCLUDED_BASEBMP_PIXELITERATOR_HXX
#define INCLUDED_BASEBMP_PIXELITERATOR_HXX

#include <basebmp/metafunctions.hxx>
#include <basebmp/stridedarrayiterator.hxx>

#include <boost/static_assert.hpp>
#include <vigra/metaprogramming.hxx>
#include <vigra/diff2d.hxx>

namespace basebmp
{

template< typename Valuetype > class PixelColumnIterator
{
public:
    typedef Valuetype                           value_type;
    typedef Valuetype&                          reference;
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
        return !less(rhs);
    }

    bool operator>(PixelColumnIterator const & rhs) const
    {
        return rhs.less(*this);
    }

    bool operator>=(PixelColumnIterator const & rhs) const
    {
        return !rhs.less(*this);
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
        return y(dy) + x+dy;
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
