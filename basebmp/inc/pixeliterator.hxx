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

#ifndef INCLUDED_BASEBMP_INC_PIXELITERATOR_HXX
#define INCLUDED_BASEBMP_INC_PIXELITERATOR_HXX

#include <metafunctions.hxx>
#include <stridedarrayiterator.hxx>

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

#endif /* INCLUDED_BASEBMP_INC_PIXELITERATOR_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
