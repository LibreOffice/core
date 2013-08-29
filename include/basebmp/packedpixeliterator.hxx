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

#ifndef INCLUDED_BASEBMP_PACKEDPIXELITERATOR_HXX
#define INCLUDED_BASEBMP_PACKEDPIXELITERATOR_HXX

#include <basebmp/metafunctions.hxx>
#include <basebmp/stridedarrayiterator.hxx>
#include <basebmp/nonstandarditerator.hxx>
#include <basebmp/accessortraits.hxx>

#include <boost/static_assert.hpp>
#include <vigra/metaprogramming.hxx>
#include <vigra/diff2d.hxx>

namespace basebmp
{

/// Get bitmask for data at given intra-word position, for given bit depth
template< typename value_type,
          int      bits_per_pixel,
          bool     MsbFirst,
          typename difference_type >
inline value_type get_mask( difference_type d )
{
    BOOST_STATIC_ASSERT(bits_per_pixel > 0);
    BOOST_STATIC_ASSERT(sizeof(value_type)*8 % bits_per_pixel == 0);
    BOOST_STATIC_ASSERT(sizeof(value_type)*8 / bits_per_pixel > 1);
    BOOST_STATIC_ASSERT(vigra::TypeTraits<value_type>::isPOD::asBool);

    const unsigned int nIntraWordPositions( sizeof(value_type)*8 / bits_per_pixel );

    //      create bits_per_pixel 1s      shift to intra-word position
    return ((~(~0U << bits_per_pixel)) << bits_per_pixel*(MsbFirst ?
                                                         (nIntraWordPositions-1 - (d % nIntraWordPositions)) :
                                                         (d % nIntraWordPositions)));
}

template< int num_intraword_positions, int bits_per_pixel, bool MsbFirst, typename difference_type > inline difference_type get_shift( difference_type remainder )
{
    return bits_per_pixel*(MsbFirst ?
                           (num_intraword_positions - 1 - remainder) :
                           remainder);
}

template< typename Valuetype,
          int      bits_per_pixel,
          bool     MsbFirst > class PackedPixelColumnIterator : public NonStandardIterator
{
public:
    // no reference, no index_reference type here
    typedef Valuetype                                   value_type;
    typedef int                                         difference_type;
    typedef image_traverser_tag                         iterator_category;

    typedef typename remove_const<value_type>::type     mask_type;
    typedef value_type*                                 pointer;
    typedef StridedArrayIterator< value_type >          MoveY;

    enum {
        /** The number of pixel within a single value_type value
         */
        num_intraword_positions=sizeof(value_type)*8/bits_per_pixel,
        /** Bit mask for one pixel (least significant bits)
         */
        bit_mask=~(~0 << bits_per_pixel)
    };

private:
    MoveY           y;
    mask_type       mask_;
    difference_type shift_;

    void inc()
    {
        ++y;
    }

    void dec()
    {
        --y;
    }

    bool equal( PackedPixelColumnIterator const & rhs ) const
    {
        return rhs.y == y;
    }

    bool less( PackedPixelColumnIterator const & rhs ) const
    {
        return y < rhs.y;
    }

public:
    PackedPixelColumnIterator() :
        y(0),
        mask_( get_mask<value_type, bits_per_pixel, MsbFirst, difference_type>(0) ),
        shift_( get_shift<num_intraword_positions, bits_per_pixel, MsbFirst, difference_type>(0) )
    {}

    PackedPixelColumnIterator( const MoveY& base, difference_type remainder ) :
        y(base),
        mask_( get_mask<value_type, bits_per_pixel, MsbFirst>(remainder) ),
        shift_( get_shift<num_intraword_positions, bits_per_pixel, MsbFirst>(remainder) )
    {}

    PackedPixelColumnIterator& operator+=( difference_type d )
    {
        y += d;
        return *this;
    }

    PackedPixelColumnIterator& operator-=( difference_type d )
    {
        y -= d;
        return *this;
    }

    PackedPixelColumnIterator operator+( difference_type d )
    {
        PackedPixelColumnIterator res(*this);
        res += d;
        return res;
    }

    PackedPixelColumnIterator operator-( difference_type d )
    {
        PackedPixelColumnIterator res(*this);
        res -= d;
        return res;
    }

    PackedPixelColumnIterator& operator++()
    {
        inc();
        return *this;
    }

    PackedPixelColumnIterator& operator--()
    {
        dec();
        return *this;
    }

    PackedPixelColumnIterator operator++(int)
    {
        PackedPixelColumnIterator res(*this);
        inc();
        return res;
    }

    PackedPixelColumnIterator operator--(int)
    {
        PackedPixelColumnIterator res(*this);
        dec();
        return res;
    }

    bool operator==(PackedPixelColumnIterator const & rhs) const
    {
        return equal( rhs );
    }

    bool operator!=(PackedPixelColumnIterator const & rhs) const
    {
        return !equal( rhs );
    }

    bool operator<(PackedPixelColumnIterator const & rhs) const
    {
        return less(rhs);
    }

    bool operator<=(PackedPixelColumnIterator const & rhs) const
    {
        return !rhs.less(*this);
    }

    bool operator>(PackedPixelColumnIterator const & rhs) const
    {
        return rhs.less(*this);
    }

    bool operator>=(PackedPixelColumnIterator const & rhs) const
    {
        return !less(rhs);
    }

    difference_type operator-(PackedPixelColumnIterator const & rhs) const
    {
        return y - rhs.y;
    }

    value_type get() const
    {
        return unsigned_cast<value_type>(*y() & mask_) >> shift_;
    }

    value_type get(difference_type d) const
    {
        return unsigned_cast<value_type>(*y(d) & mask_) >> shift_;
    }

    void set( value_type v ) const
    {
        const value_type pixel_value( (v << shift_) & mask_ );
        *y() = (*y() & ~mask_) | pixel_value;
    }

    void set( value_type v, difference_type d ) const
    {
        const value_type pixel_value( (v << shift_) & mask_ );
        *y(d) = (*y(d) & ~mask_) | pixel_value;
    }
};

template< typename Valuetype,
          int      bits_per_pixel,
          bool     MsbFirst > class PackedPixelRowIterator : public NonStandardIterator
{
public:
    // no reference, no index_reference type here
    typedef Valuetype                                   value_type;
    typedef int                                         difference_type;
    typedef image_traverser_tag                         iterator_category;

    typedef typename remove_const<value_type>::type     mask_type;
    typedef value_type*                                 pointer;

    enum {
        /** The number of pixel within a single value_type value
         */
        num_intraword_positions=sizeof(value_type)*8/bits_per_pixel,
        /** Bit mask for one pixel (least significant bits)
         */
        bit_mask=~(~0 << bits_per_pixel)
    };

private:
    pointer         data_;
    mask_type       mask_;
    difference_type remainder_;

    void update_mask()
    {
        mask_ = get_mask<value_type, bits_per_pixel, MsbFirst>(remainder_);
    }

    void inc()
    {
        const difference_type newValue( remainder_ + 1 );
        const difference_type data_offset( newValue / num_intraword_positions );

        data_ += data_offset;
        remainder_ = newValue % num_intraword_positions;

        const mask_type shifted_mask(
            MsbFirst ?
            unsigned_cast<mask_type>(mask_) >> bits_per_pixel :
            mask_ << bits_per_pixel );

        // data_offset is 0 for shifted mask, and 1 for wrapped-around mask
        mask_ = (1-data_offset)*shifted_mask + data_offset*(MsbFirst ?
                                                            bit_mask << bits_per_pixel*(num_intraword_positions-1) :
                                                            bit_mask);
    }

    void dec()
    {
        const difference_type newValue( remainder_ - 1 );
        const bool            isNegative( is_negative(newValue) );
        const difference_type newRemainder( newValue % num_intraword_positions );

        // calc  data_ += newValue / num_intraword_positions;
        //       remainder_ = newRemainder;
        // for newValue >= 0, and
        //       data_ += newValue / num_intraword_positions - 1;
        //       remainder_ = num_intraword_positions - newRemainder;
        // (to force remainder_ to be positive).
        // This is branch-free, if is_negative() is branch-free
        const difference_type data_offset( newValue / num_intraword_positions - isNegative );
        data_     += data_offset;
        remainder_ = newRemainder + isNegative*num_intraword_positions;

        const mask_type shifted_mask(
            MsbFirst ?
            mask_ << bits_per_pixel :
            unsigned_cast<mask_type>(mask_) >> bits_per_pixel );

        // data_offset is 0 for shifted mask, and 1 for wrapped-around mask
        mask_ = (1-data_offset)*shifted_mask + data_offset*(MsbFirst ?
                                                            bit_mask :
                                                            bit_mask << bits_per_pixel*(num_intraword_positions-1));
    }

    bool equal( PackedPixelRowIterator const & rhs ) const
    {
        return rhs.data_ == data_ && rhs.remainder_ == remainder_;
    }

    bool less( PackedPixelRowIterator const & rhs ) const
    {
        return data_ == rhs.data_ ?
            (remainder_ < rhs.remainder_) :
            (data_ < rhs.data_);
    }

public:
    PackedPixelRowIterator() :
        data_(0),
        mask_( get_mask<value_type, bits_per_pixel, MsbFirst, difference_type>(0) ),
        remainder_(0)
    {}

    explicit PackedPixelRowIterator( pointer base, int x ) :
        data_(base),
        mask_(0),
        remainder_(x % num_intraword_positions)
    {
        update_mask();
    }

    PackedPixelRowIterator& operator+=( difference_type d )
    {
        const difference_type newValue( remainder_ + d );
        const bool            isNegative( is_negative(newValue) );
        const difference_type newRemainder( newValue % num_intraword_positions );

        // calc  data_ += newValue / num_intraword_positions;
        //       remainder_ = newRemainder;
        // for newValue >= 0, and
        //       data_ += newValue / num_intraword_positions - 1;
        //       remainder_ = newRemainder + num_intraword_positions;
        // (to force remainder_ to be positive).
        // This is branch-free, if is_negative() is branch-free
        data_     += newValue / num_intraword_positions - isNegative;
        remainder_ = newRemainder + isNegative*num_intraword_positions;
        update_mask();

        return *this;
    }

    PackedPixelRowIterator& operator-=( difference_type d )
    {
        // forward to operator+= - which has to cope with negative
        // values, anyway.
        return *this += -d;
    }

    PackedPixelRowIterator operator+( difference_type d )
    {
        PackedPixelRowIterator res(*this);
        res += d;
        return res;
    }

    PackedPixelRowIterator operator-( difference_type d )
    {
        PackedPixelRowIterator res(*this);
        res -= d;
        return res;
    }

    PackedPixelRowIterator& operator++()
    {
        inc();
        return *this;
    }

    PackedPixelRowIterator& operator--()
    {
        dec();
        return *this;
    }

    PackedPixelRowIterator operator++(int)
    {
        PackedPixelRowIterator res(*this);
        inc();
        return res;
    }

    PackedPixelRowIterator operator--(int)
    {
        PackedPixelRowIterator res(*this);
        dec();
        return res;
    }

    bool operator==(PackedPixelRowIterator const & rhs) const
    {
        return equal( rhs );
    }

    bool operator!=(PackedPixelRowIterator const & rhs) const
    {
        return !equal( rhs );
    }

    bool operator<(PackedPixelRowIterator const & rhs) const
    {
        return less(rhs);
    }

    bool operator<=(PackedPixelRowIterator const & rhs) const
    {
        return !rhs.less(*this);
    }

    bool operator>(PackedPixelRowIterator const & rhs) const
    {
        return rhs.less(*this);
    }

    bool operator>=(PackedPixelRowIterator const & rhs) const
    {
        return !less(rhs);
    }

    difference_type operator-(PackedPixelRowIterator const & rhs) const
    {
        return (data_ - rhs.data_)*num_intraword_positions + (remainder_ - rhs.remainder_);
    }

    value_type get() const
    {
        return unsigned_cast<value_type>(*data_ & mask_) >>
            get_shift<num_intraword_positions,
                      bits_per_pixel,
                      MsbFirst>(remainder_);
    }

    value_type get(difference_type d) const
    {
        PackedPixelRowIterator tmp(*this);
        tmp += d;
        return tmp.get();
    }

    void set( value_type v ) const
    {
        const value_type pixel_value(
            (v <<
             get_shift<num_intraword_positions, bits_per_pixel, MsbFirst>(remainder_))
            & mask_ );
        *data_ = (*data_ & ~mask_) | pixel_value;
    }

    void set( value_type v, difference_type d ) const
    {
        PackedPixelRowIterator tmp(*this);
        tmp += d;
        tmp.set(v);
    }
};

/** 2D image iterator for packed pixel formats

    This iterator can be used for image formats that pack more than
    one pixel into an machine data type (like one bit per pixel, eight
    of which packed into one char)
 */
template< typename Valuetype,
          int      bits_per_pixel,
          bool     MsbFirst > class PackedPixelIterator : public NonStandardIterator
{
public:
    // no reference, no index_reference type here
    typedef Valuetype                                   value_type;
    typedef vigra::Diff2D                               difference_type;
    typedef image_traverser_tag                         iterator_category;
    typedef PackedPixelRowIterator<value_type,
                                   bits_per_pixel,
                                   MsbFirst>            row_iterator;
    typedef PackedPixelColumnIterator<value_type,
                                      bits_per_pixel,
                                      MsbFirst>         column_iterator;

    typedef value_type*                                 pointer;
    typedef int                                         MoveX;
    typedef StridedArrayIterator< value_type >          MoveY;

    enum {
        /** The number of pixel within a single value_type value
         */
        num_intraword_positions=sizeof(value_type)*8/bits_per_pixel,
        /** Bit mask for one pixel (least significant bits)
         */
        bit_mask=~(~0 << bits_per_pixel)
    };

    // TODO(F2): direction of iteration (ImageIterator can be made to
    // run backwards)

private:
    pointer current() const
    {
        return y() + (x / num_intraword_positions);
    }

    pointer current(int dx, int dy) const
    {
        return y(dy) + ((x+dx)/num_intraword_positions);
    }

    bool equal(PackedPixelIterator const & rhs) const
    {
        return (x == rhs.x) && (y == rhs.y);
    }

public:
    PackedPixelIterator() :
        x(0),
        y(0)
    {}

    PackedPixelIterator(pointer base, int ystride) :
        x(0),
        y(ystride,base)
    {}

    bool operator==(PackedPixelIterator const & rhs) const
    {
        return equal(rhs);
    }

    bool operator!=(PackedPixelIterator const & rhs) const
    {
        return !equal(rhs);
    }

    difference_type operator-(PackedPixelIterator const & rhs) const
    {
        return difference_type(x - rhs.x, y - rhs.y);
    }

    MoveX x;
    MoveY y;

    PackedPixelIterator & operator+=(difference_type const & s)
    {
        x += s.x;
        y += s.y;
        return *this;
    }

    PackedPixelIterator & operator-=(difference_type const & s)
    {
        x -= s.x;
        y -= s.y;
        return *this;
    }

    PackedPixelIterator operator+(difference_type const & s) const
    {
        PackedPixelIterator ret(*this);
        ret += s;
        return ret;
    }

    PackedPixelIterator operator-(difference_type const & s) const
    {
        PackedPixelIterator ret(*this);
        ret -= s;
        return ret;
    }

    row_iterator rowIterator() const
    {
        return row_iterator(current(),x);
    }

    column_iterator columnIterator() const
    {
        return column_iterator(MoveY(y,
                                     x / num_intraword_positions),
                               x % num_intraword_positions);
    }

    value_type get() const
    {
        const int remainder( x % num_intraword_positions );

        return (unsigned_cast<value_type>(*current() &
                                          get_mask<value_type, bits_per_pixel, MsbFirst>(remainder))
                >> get_shift<num_intraword_positions, bits_per_pixel, MsbFirst>(remainder));
    }

    value_type get(difference_type const & d) const
    {
        const int remainder( x(d.x) % num_intraword_positions );

        return (unsigned_cast<value_type>(*current(d.x,d.y) &
                                          get_mask<value_type, bits_per_pixel, MsbFirst>(remainder))
                >> get_shift<num_intraword_positions, bits_per_pixel, MsbFirst>(remainder));
    }

    void set( value_type v ) const
    {
        const int remainder( x % num_intraword_positions );
        const int mask( get_mask<value_type, bits_per_pixel, MsbFirst>(remainder) );
        const value_type pixel_value(
            (v <<
             get_shift<num_intraword_positions, bits_per_pixel, MsbFirst>(remainder))
            & mask );
        pointer p = current();
        *p = (*p & ~mask) | pixel_value;
    }

    void set( value_type v, difference_type const & d ) const
    {
        const int remainder( (x + d.x) % num_intraword_positions );
        const int mask( get_mask<value_type, bits_per_pixel, MsbFirst>(remainder) );
        const value_type pixel_value(
            (v <<
             get_shift<num_intraword_positions, bits_per_pixel, MsbFirst>(remainder))
             & mask );
        pointer p = current(d.x,d.y);
        *p = (*p & ~mask) | pixel_value;
    }
};

//-----------------------------------------------------------------------------

// partial specialization for the accessor traits masked_accessor
// selector metafunction - can employ fast mask functor for the 1bpp
// case.
template< class Accessor,
          class MaskAccessor,
          class Iterator,
          bool  polarity,
          bool  MsbFirst > struct maskedAccessorSelector< Accessor,
                                                          MaskAccessor,
                                                          Iterator,
                                                          PackedPixelIterator< typename MaskAccessor::value_type,
                                                                               1,
                                                                               MsbFirst >,
                                                          polarity >
{
    typedef TernarySetterFunctionAccessorAdapter<
        Accessor,
        MaskAccessor,
        typename outputMaskFunctorSelector<
            typename Accessor::value_type,
            typename MaskAccessor::value_type,
            polarity,
            FastMask>::type >
        type;
};

} // namespace basebmp

#endif /* INCLUDED_BASEBMP_PACKEDPIXELITERATOR_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
