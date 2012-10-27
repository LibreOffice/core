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

#ifndef  _USE_MATH_DEFINES
#define  _USE_MATH_DEFINES  // needed by Visual C++ for math constants
#endif
#include <math.h>

#include <comphelper/processfactory.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/ucb/UniversalContentBroker.hpp>

#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/unowrap.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/bmpacc.hxx>

#include <basegfx/polygon/b2dlinegeometry.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/range/b2ibox.hxx>
#include <basegfx/vector/b2isize.hxx>
#include <basegfx/vector/b2enums.hxx>
#include <basegfx/point/b2ipoint.hxx>

#include <basebmp/color.hxx>
#include <basebmp/scanlineformats.hxx>
#include <basebmp/bitmapdevice.hxx>

#include <rtl/bootstrap.hxx>

#include <vigra/metaprogramming.hxx>
#include <vigra/static_assert.hxx>
#include <vigra/basicimageview.hxx>

#include <boost/static_assert.hpp>
#include <algorithm>

using namespace ::com::sun::star;


namespace
{

/// template meta function: add const qualifier, if given 2nd type has it
template<typename A, typename B> struct clone_const
{
    typedef B type;
};
template<typename A, typename B> struct clone_const<const A,B>
{
    typedef const B type;
};

template< class DestIterator, class DestAccessor > class Renderer :
        public basegfx::B2DPolyPolygonRasterConverter
{
private:
    typename DestIterator::value_type fillColor_;
    typename DestIterator::value_type clearColor_;
    DestIterator                      begin_;
    DestAccessor                      accessor_;

public:
    Renderer(const basegfx::B2DPolyPolygon&     rPolyPolyRaster,
             typename DestIterator::value_type  fillColor,
             typename DestIterator::value_type  clearColor,
             DestIterator                       begin,
             DestIterator                       end,
             DestAccessor                       accessor ) :
        B2DPolyPolygonRasterConverter(rPolyPolyRaster,
                                      basegfx::B2DRange(0,0,
                                                        end.x - end.x,
                                                        begin.y - begin.y )),
        fillColor_( fillColor ),
        clearColor_( clearColor ),
        begin_( begin ),
        accessor_( accessor )
    {
    }

    virtual void span(const double& rfXLeft,
                      const double& rfXRight,
                      sal_Int32     nY,
                      bool          bOn )
    {
        DestIterator currIter( begin_ + vigra::Diff2D(0,nY) );
        typename DestIterator::row_iterator rowIter( currIter.rowIterator() +
                                                     basegfx::fround(rfXLeft) );
        typename DestIterator::row_iterator rowEnd( currIter.rowIterator() +
                                                    basegfx::fround(rfXRight) );
        if( bOn )
            while( rowIter != rowEnd )
            {
                accessor_.set(fillColor_, rowIter);
                ++rowIter;
            }
        else
            while( rowIter != rowEnd )
            {
                accessor_.set(accessor_(rowIter)*clearColor_, rowIter);
                ++rowIter;
            }
    }
};

template< class DestIterator, class DestAccessor >
    std::auto_ptr< Renderer< DestIterator, DestAccessor > > makeRenderer(
        const basegfx::B2DPolyPolygon&                          rPolyPolyRaster,
        typename DestIterator::value_type                       fillColor,
        typename DestIterator::value_type                       clearColor,
        vigra::triple<DestIterator, DestIterator, DestAccessor> dest )
{
    return std::auto_ptr< Renderer< DestIterator, DestAccessor > >(
        new Renderer< DestIterator, DestAccessor >(rPolyPolyRaster,
                                                   fillColor,
                                                   clearColor,
                                                   dest.first,
                                                   dest.second,
                                                   dest.third));
}


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

    void operator++() {current_ += stride_; }
    void operator++(int) {current_ += stride_; }
    void operator--() {current_ -= stride_; }
    void operator--(int) {current_ -= stride_; }
    void operator+=(int dy) {current_ += dy*stride_; }
    void operator-=(int dy) {current_ -= dy*stride_; }

    bool operator==(StridedArrayIterator const & rhs) const
    { return (current_ == rhs.current_); }

    bool operator!=(StridedArrayIterator const & rhs) const
    { return (current_ != rhs.current_); }

    bool operator<(StridedArrayIterator const & rhs) const
    { return (current_ < rhs.current_); }

    bool operator<=(StridedArrayIterator const & rhs) const
    { return (current_ <= rhs.current_); }

    bool operator>(StridedArrayIterator const & rhs) const
    { return (current_ > rhs.current_); }

    bool operator>=(StridedArrayIterator const & rhs) const
    { return (current_ >= rhs.current_); }

    int operator-(StridedArrayIterator const & rhs) const
    { return (current_ - rhs.current_) / stride_; }

    T* operator()() const
    { return reinterpret_cast<T*>(current_); }

    T* operator()(int d) const
    { return reinterpret_cast<T*>(current_ + d*stride_); }

    int            stride_;
    internal_type* current_;
};

/// template meta function: remove const qualifier from plain type
template <typename T> struct remove_const
{
    typedef T type;
};
template <typename T> struct remove_const<const T>
{
    typedef T type;
};

/// returns true, if given number is strictly less than 0
template< typename T > inline bool is_negative( T x )
{
    return x < 0;
}

/// Overload for ints (branch-free)
inline bool is_negative( int x )
{
    // force logic shift (result for signed shift right is undefined)
    return static_cast<unsigned int>(x) >> (sizeof(int)*8-1);
}

/// Get bitmask for data at given intra-word position, for given bit depth
template< typename data_type, int bits_per_pixel, bool MsbFirst, typename difference_type > inline data_type get_mask( difference_type d )
{
    BOOST_STATIC_ASSERT(bits_per_pixel > 0);
    BOOST_STATIC_ASSERT(sizeof(data_type)*8 % bits_per_pixel == 0);
    BOOST_STATIC_ASSERT(sizeof(data_type)*8 / bits_per_pixel > 1);
    BOOST_STATIC_ASSERT(vigra::TypeTraits<data_type>::isPOD::asBool);

    const unsigned int nIntraWordPositions( sizeof(data_type)*8 / bits_per_pixel );

    //      create bits_per_pixel 1s      shift to intra-word position
    return ((~(~0 << bits_per_pixel)) << bits_per_pixel*(MsbFirst ?
                                                         (nIntraWordPositions-1 - (d % nIntraWordPositions)) :
                                                         (d % nIntraWordPositions)));
}

template< int num_intraword_positions, int bits_per_pixel, bool MsbFirst, typename difference_type > inline difference_type get_shift( difference_type remainder )
{
    return bits_per_pixel*(MsbFirst ?
                           (num_intraword_positions - 1 - remainder) :
                           remainder);
}

template< typename Datatype,
          typename Valuetype,
          int      bits_per_pixel,
          bool     MsbFirst > class PackedPixelColumnIterator
{
public:
    // no reference, no index_reference type here
    typedef Datatype                                    data_type;
    typedef Valuetype                                   value_type;
    typedef int                                         difference_type;
    typedef image_traverser_tag                         iterator_category;

    typedef typename remove_const<data_type>::type      mask_type;
    typedef data_type*                                  pointer;
    typedef StridedArrayIterator< data_type >           MoveY;

    enum {
        /** The number of pixel within a single data_type value
         */
        num_intraword_positions=sizeof(data_type)*8/bits_per_pixel,
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
        mask_( get_mask<data_type, bits_per_pixel, MsbFirst, difference_type>(0) ),
        shift_( get_shift<num_intraword_positions, bits_per_pixel, MsbFirst, difference_type>(0) )
    {}

    PackedPixelColumnIterator( const MoveY& base, difference_type remainder ) :
        y(base),
        mask_( get_mask<data_type, bits_per_pixel, MsbFirst>(remainder) ),
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
        res.inc();
        return res;
    }

    PackedPixelColumnIterator operator--(int)
    {
        PackedPixelColumnIterator res(*this);
        res.dec();
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
        return !less(rhs);
    }

    bool operator>(PackedPixelColumnIterator const & rhs) const
    {
        return rhs.less(*this);
    }

    bool operator>=(PackedPixelColumnIterator const & rhs) const
    {
        return !rhs.less(*this);
    }

    difference_type operator-(PackedPixelColumnIterator const & rhs) const
    {
        return y - rhs.y;
    }

    value_type get() const
    {
        // TODO(Q3): use traits to get unsigned type for data_type (if
        // not already)
        return static_cast<unsigned int>(*y() & mask_) >> shift_;
    }

    value_type get(difference_type d) const
    {
        // TODO(Q3): use traits to get unsigned type for data_type (if
        // not already)
        return static_cast<unsigned int>(*y(d) & mask_) >> shift_;
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

template< typename Datatype,
          typename Valuetype,
          int      bits_per_pixel,
          bool     MsbFirst > class PackedPixelRowIterator
{
public:
    // no reference, no index_reference type here
    typedef Datatype                                    data_type;
    typedef Valuetype                                   value_type;
    typedef int                                         difference_type;
    typedef image_traverser_tag                         iterator_category;

    typedef typename remove_const<data_type>::type      mask_type;
    typedef data_type*                                  pointer;

    enum {
        /** The number of pixel within a single data_type value
         */
        num_intraword_positions=sizeof(data_type)*8/bits_per_pixel,
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
        mask_ = get_mask<data_type, bits_per_pixel, MsbFirst>(remainder_);
    }

    void inc()
    {
        const difference_type newValue( remainder_ + 1 );
        const difference_type data_offset( newValue / num_intraword_positions );

        data_ += data_offset;
        remainder_ = newValue % num_intraword_positions;

        const mask_type shifted_mask(
            MsbFirst ?
            // TODO(Q3): use traits to get unsigned type for data_type
            // (if not already)
            static_cast<unsigned int>(mask_) >> bits_per_pixel :
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
            // TODO(Q3): use traits to get unsigned type for data_type
            // (if not already)
            static_cast<unsigned int>(mask_) >> bits_per_pixel );

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
        mask_( get_mask<data_type, bits_per_pixel, MsbFirst, difference_type>(0) ),
        remainder_(0)
    {}

    explicit PackedPixelRowIterator( pointer base ) :
        data_(base),
        mask_( get_mask<data_type, bits_per_pixel, MsbFirst, difference_type>(0) ),
        remainder_(0)
    {}

    PackedPixelRowIterator& operator+=( difference_type d )
    {
        const difference_type newValue( remainder_ + d );

        data_ += newValue / num_intraword_positions;
        remainder_ = newValue % num_intraword_positions;
        update_mask();

        return *this;
    }

    PackedPixelRowIterator& operator-=( difference_type d )
    {
        const difference_type newValue( remainder_ - d );
        const bool            isNegative( is_negative(newValue) );
        const difference_type newRemainder( newValue % num_intraword_positions );

        // calc  data_ += newValue / num_intraword_positions;
        //       remainder_ = newRemainder;
        // for newValue >= 0, and
        //       data_ += newValue / num_intraword_positions - 1;
        //       remainder_ = num_intraword_positions - newRemainder;
        // (to force remainder_ to be positive).
        // This is branch-free, if is_negative() is branch-free
        data_     += newValue / num_intraword_positions - isNegative;
        remainder_ = newRemainder + isNegative*(num_intraword_positions - 2*newRemainder);
        update_mask();

        return *this;
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
        res.inc();
        return res;
    }

    PackedPixelRowIterator operator--(int)
    {
        PackedPixelRowIterator res(*this);
        res.dec();
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
        return !less(rhs);
    }

    bool operator>(PackedPixelRowIterator const & rhs) const
    {
        return rhs.less(*this);
    }

    bool operator>=(PackedPixelRowIterator const & rhs) const
    {
        return !rhs.less(*this);
    }

    difference_type operator-(PackedPixelRowIterator const & rhs) const
    {
        return (data_ - rhs.data_)*num_intraword_positions + (remainder_ - rhs.remainder_);
    }

    value_type get() const
    {
        // TODO(Q3): use traits to get unsigned type for data_type (if
        // not already)
        return static_cast<unsigned int>(*data_ & mask_) >>
            get_shift<num_intraword_positions, bits_per_pixel, MsbFirst>(remainder_);
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

template< typename Datatype,
          typename Valuetype,
          int      bits_per_pixel,
          bool     MsbFirst > class PackedPixelIterator
{
public:
    // no reference, no index_reference type here
    typedef Datatype                                    data_type;
    typedef Valuetype                                   value_type;
    typedef vigra::Diff2D                               difference_type;
    typedef image_traverser_tag                         iterator_category;
    typedef PackedPixelRowIterator<data_type,
                                   value_type,
                                   bits_per_pixel,
                                   MsbFirst>            row_iterator;
    typedef PackedPixelColumnIterator<data_type,
                                      value_type,
                                      bits_per_pixel,
                                      MsbFirst>         column_iterator;

    typedef data_type*                                  pointer;
    typedef int                                         MoveX;
    typedef StridedArrayIterator< data_type >           MoveY;

    enum {
        /** The number of pixel within a single data_type value
         */
        num_intraword_positions=sizeof(data_type)*8/bits_per_pixel,
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
        return row_iterator(current());
    }

    column_iterator columnIterator() const
    {
        return column_iterator(MoveY(y,
                                     x / num_intraword_positions),
                               x % num_intraword_positions);
    }

    value_type get() const
    {
        const int remainder( x() % num_intraword_positions );

        // TODO(Q3): use traits to get unsigned type for data_type (if
        // not already)
        return (static_cast<unsigned int>(*current() &
                                          get_mask<data_type, bits_per_pixel, MsbFirst>(remainder))
                >> (MsbFirst ?
                    (num_intraword_positions - remainder) :
                    remainder));
    }

    value_type get(difference_type const & d) const
    {
        const int remainder( x(d.x) % num_intraword_positions );

        // TODO(Q3): use traits to get unsigned type for data_type (if
        // not already)
        return (static_cast<unsigned int>(*current(d.x,d.y) &
                                          get_mask<data_type, bits_per_pixel, MsbFirst>(remainder))
                >> get_shift<num_intraword_positions, bits_per_pixel, MsbFirst>(remainder));
    }

    void set( value_type v ) const
    {
        const int remainder( x() % num_intraword_positions );
        const int mask( get_mask<data_type, bits_per_pixel, MsbFirst>(remainder) );
        const value_type pixel_value(
            (v <<
             get_shift<num_intraword_positions, bits_per_pixel, MsbFirst>(remainder))
            & mask );
        pointer p = current();
        *p = (*p & ~mask) | pixel_value;
    }

    void set( value_type v, difference_type const & d ) const
    {
        const int remainder( x(d.x) % num_intraword_positions );
        const int mask( get_mask<data_type, bits_per_pixel, MsbFirst>(remainder) );
        const value_type pixel_value(
            (v <<
             get_shift<num_intraword_positions, bits_per_pixel, MsbFirst>(remainder))
             & mask );
        pointer p = current(d.x,d.y);
        *p = (*p & ~mask) | pixel_value;
    }
};


/** Access (possibly packed-pixel) data via palette indirection
 */
template< typename Valuetype, typename Datatype > class PaletteImageAccessor
{
  public:
    typedef Valuetype                                   value_type;
    typedef Datatype                                    data_type;
    typedef typename remove_const<data_type>::type      count_type;


private:
    const BitmapColor* palette;
    count_type         num_entries;

    double norm( BitmapColor const& rLHS,
                 BitmapColor const& rRHS ) const
    {
        // convert RGBValue's linear space to a normed linear space
        return sqrt(
            vigra::sq(rLHS.GetRed()-rRHS.GetRed()) +
            vigra::sq(rLHS.GetGreen()-rRHS.GetGreen()) +
            vigra::sq(rLHS.GetBlue()-rRHS.GetBlue()) );
    }

    data_type find_best_match(value_type const& v) const
    {
        // TODO(F3): not generic!!!
        const BitmapColor aTmpCol(v.red(),
                                  v.green(),
                                  v.blue());

        // TODO(P3): use table-based/octree approach here!
        const BitmapColor* best_entry;
        const BitmapColor* palette_end( palette+num_entries );
        if( (best_entry=std::find( palette, palette_end, aTmpCol)) != palette_end )
            return best_entry-palette;

        // TODO(F3): HACK. Need palette traits, and an error function
        // here. We blatantly assume value_type is a normed linear
        // space.
        const BitmapColor* curr_entry( palette );
        best_entry = curr_entry;
        while( curr_entry != palette_end )
        {
            if( norm(*curr_entry,*best_entry) > norm(*curr_entry,aTmpCol) )
                best_entry = curr_entry;

            ++curr_entry;
        }

        return best_entry-palette;
    }

    value_type toCol( BitmapColor const& rCol ) const
    {
        return value_type(rCol.GetRed(),rCol.GetGreen(),rCol.GetBlue());
    }

public:
    PaletteImageAccessor() :
        palette(0),
        num_entries(0)
    {}

    PaletteImageAccessor( const BitmapColor* pPalette,
                          data_type          entries ) :
        palette(pPalette),
        num_entries(entries)
    {}

    template< class Iterator >
    value_type operator()(Iterator const& i) const { return toCol(palette[i.get()]); }
    value_type operator()(data_type const* i) const { return toCol(palette[*i]); }

    template< class Iterator, class Difference >
    value_type operator()(Iterator const& i, Difference const& diff) const
    {
        return toCol(palette[i.get(diff)]);
    }

    template< typename V, class Iterator >
    void set(V const& value, Iterator const& i) const
    {
        i.set(
            find_best_match(
                vigra::detail::RequiresExplicitCast<value_type>::cast(value) ));
    }

    template< typename V, class Iterator, class Difference >
    void set(V const& value, Iterator const& i, Difference const& diff) const
    {
        i.set(
            find_best_match(
                vigra::detail::RequiresExplicitCast<value_type>::cast(value)),
            diff );
    }
};

}


class TestApp : public Application
{
public:
    virtual void Main();
    virtual USHORT  Exception( USHORT nError );
};

class TestWindow : public Dialog
{
    public:
        TestWindow() : Dialog( (Window *) NULL )
        {
            SetText( OUString( "VIGRA test" ) );
            SetSizePixel( Size( 1024, 1024 ) );
            EnablePaint( true );
            Show();
        }
        virtual ~TestWindow() {}
        virtual void MouseButtonUp( const MouseEvent& /*rMEvt*/ )
        {
            //TODO: do something cool
            EndDialog();
        }
        virtual void Paint( const Rectangle& rRect );
};


static basegfx::B2IPoint project( const basegfx::B2IPoint& rPoint )
{
    const double angle_x = M_PI / 6.0;
    const double angle_z = M_PI / 6.0;

    // transform planar coordinates to 3d
    double x = rPoint.getX();
    double y = rPoint.getY();
    //double z = 0;

    // rotate around X axis
    double x1 = x;
    double y1 = y * cos( angle_x );
    double z1 = y * sin( angle_x );

    // rotate around Z axis
    double x2 = x1 * cos( angle_z ) + y1 * sin( angle_z );
    //double y2 = y1 * cos( angle_z ) - x1 * sin( angle_z );
    double z2 = z1;

    //return basegfx::B2IPoint( (sal_Int32)3*x2, (sal_Int32)3*z2 );
    return basegfx::B2IPoint( (sal_Int32)(6*x2), (sal_Int32)(6*z2) );
}

static basebmp::Color approachColor( const basebmp::Color& rFrom, const basebmp::Color& rTo )
{
    basebmp::Color aColor;
    UINT8 nDiff;
    // approach red
    if( rFrom.getRed() < rTo.getRed() )
    {
        nDiff = rTo.getRed() - rFrom.getRed();
        aColor.setRed( rFrom.getRed() + ( nDiff < 10 ? nDiff : 10 ) );
    }
    else if( rFrom.getRed() > rTo.getRed() )
    {
        nDiff = rFrom.getRed() - rTo.getRed();
        aColor.setRed( rFrom.getRed() - ( nDiff < 10 ? nDiff : 10 ) );
    }
    else
        aColor.setRed( rFrom.getRed() );

    // approach Green
    if( rFrom.getGreen() < rTo.getGreen() )
    {
        nDiff = rTo.getGreen() - rFrom.getGreen();
        aColor.setGreen( rFrom.getGreen() + ( nDiff < 10 ? nDiff : 10 ) );
    }
    else if( rFrom.getGreen() > rTo.getGreen() )
    {
        nDiff = rFrom.getGreen() - rTo.getGreen();
        aColor.setGreen( rFrom.getGreen() - ( nDiff < 10 ? nDiff : 10 ) );
    }
    else
        aColor.setGreen( rFrom.getGreen() );

    // approach blue
    if( rFrom.getBlue() < rTo.getBlue() )
    {
        nDiff = rTo.getBlue() - rFrom.getBlue();
        aColor.setBlue( rFrom.getBlue() + ( nDiff < 10 ? nDiff : 10 ) );
    }
    else if( rFrom.getBlue() > rTo.getBlue() )
    {
        nDiff = rFrom.getBlue() - rTo.getBlue();
        aColor.setBlue( rFrom.getBlue() - ( nDiff < 10 ? nDiff : 10 ) );
    }
    else
        aColor.setBlue( rFrom.getBlue() );

    return aColor;
}

#define DELTA 5.0



void TestWindow::Paint( const Rectangle& /*rRect*/ )
{
    basegfx::B2ISize aTestSize(1000,1000);
    basebmp::BitmapDeviceSharedPtr pDevice( basebmp::createBitmapDevice( aTestSize,
                                                                         false,
                                                                         basebmp::Format::THIRTYTWO_BIT_TC_MASK ));

    {
        OUString aSvg;

        basegfx::B2DPolyPolygon aPoly;

        basegfx::tools::importFromSvgD( aPoly, OUString( "m0 0 h7 v7 h-7 z" ) );

        basegfx::tools::importFromSvgD( aPoly, OUString( "m2 2 h3 v3 h-3 z" ) );

        pDevice->fillPolyPolygon( aPoly,
            basebmp::Color(0xFFFFFFFF),
            basebmp::DrawMode_PAINT );
    }

    {
        basebmp::BitmapDeviceSharedPtr pMask( basebmp::createBitmapDevice( aTestSize,
                                                                           false,
                                                                           basebmp::Format::ONE_BIT_MSB_GREY ));

        const basegfx::B2IPoint aPt111(10,10);
        const basegfx::B2IPoint aPt222(0,10);
        const basebmp::Color aCol333(0xFFFFFFFF);
        pMask->drawLine( aPt111, aPt222, aCol333, basebmp::DrawMode_PAINT );


        OUString aSvg( "m 0 0 h5 l5 5 v5 h-5 l-5-5 z" );
        basegfx::B2DPolyPolygon aPoly;
        basegfx::tools::importFromSvgD( aPoly, aSvg );
        pMask->clear(basebmp::Color(0xFFFFFFFF));
        pMask->drawPolygon(
            aPoly.getB2DPolygon(0),
            basebmp::Color(0),
            basebmp::DrawMode_PAINT );

        basebmp::BitmapDeviceSharedPtr pSubsetDevice =
            basebmp::subsetBitmapDevice( pDevice,
                                         basegfx::B2IBox(3,3,7,7) );

        const basegfx::B2IPoint aPt1(0,0);
        const basegfx::B2IPoint aPt2(1,9);
        const basebmp::Color aCol(0xFFFFFFFF);
        pDevice->drawLine( aPt1, aPt2, aCol, basebmp::DrawMode_PAINT, pMask );
    }

    {
        const basebmp::Color aCol(0xFFFFFFFF);
        basegfx::B2DPolygon aRect = basegfx::tools::createPolygonFromRect(
            basegfx::B2DRange( 0,0,1001,1001 ));
        pDevice->drawPolygon( aRect, aCol, basebmp::DrawMode_PAINT );

        const basegfx::B2IPoint aPt1(0,0);
        const basegfx::B2IPoint aPt2(0,800);
        pDevice->drawLine( aPt1, aPt2, aCol, basebmp::DrawMode_PAINT );

        const basegfx::B2IPoint aPt3(0,1001);
        pDevice->drawLine( aPt1, aPt3, aCol, basebmp::DrawMode_PAINT );
    }

    {
        pDevice->clear(basebmp::Color(0));

        basegfx::B2IPoint aCenter( aTestSize.getX()/2,
                                   aTestSize.getY()/2 );
        //basegfx::B2IPoint aP1( aTestSize.getX()/48, 0), aP2( aTestSize.getX()/40, 0 ), aPoint;
        //basegfx::B2IPoint aP1( aTestSize.getX()/7, 0), aP2( aTestSize.getX()/6, 0 ), aPoint;
        //basegfx::B2IPoint aP1( aTestSize.getX()/5, 0), aP2( aTestSize.getX()/4, 0 ), aPoint;
        basegfx::B2IPoint aP1( aTestSize.getX()/12, 0), aP2( aTestSize.getX()/11, 0 ), aPoint;

        double sind = sin( DELTA*M_PI/180.0 );
        double cosd = cos( DELTA*M_PI/180.0 );
        double factor = 1 + (DELTA/1000.0);
        int n=0;
        basebmp::Color aLineColor( 0, 0, 0 );
        basebmp::Color aApproachColor( 0, 0, 200 );
        while ( aP2.getX() < aCenter.getX() && n++ < 680 )
        {
            aLineColor = approachColor( aLineColor, aApproachColor );

            // switch aproach color
            if( aApproachColor == aLineColor )
            {
                if( aApproachColor.getRed() )
                    aApproachColor = basebmp::Color( 0, 0, 200 );
                else if( aApproachColor.getGreen() )
                    aApproachColor = basebmp::Color( 200, 0, 0 );
                else
                    aApproachColor = basebmp::Color( 0, 200, 0 );
            }

            basegfx::B2DPolygon aPoly;
            aPoly.append( basegfx::B2DPoint(project( aP1 ) + aCenter) );
            aPoly.append( basegfx::B2DPoint(project( aP2 ) + aCenter) );
            pDevice->fillPolyPolygon(
                basegfx::tools::createAreaGeometry(
                    aPoly,
//                    std::max(1,n/30),
//                    std::max(1,n/60),
                    std::max(1,n/30),
                    basegfx::B2DLINEJOIN_NONE),
                aLineColor,
                basebmp::DrawMode_PAINT);

            aPoint.setX( (int)((((double)aP1.getX())*cosd - ((double)aP1.getY())*sind)*factor) );
            aPoint.setY( (int)((((double)aP1.getY())*cosd + ((double)aP1.getX())*sind)*factor) );
            aP1 = aPoint;
            aPoint.setX( (int)((((double)aP2.getX())*cosd - ((double)aP2.getY())*sind)*factor) );
            aPoint.setY( (int)((((double)aP2.getY())*cosd + ((double)aP2.getX())*sind)*factor) );
            aP2 = aPoint;
        }
    }

    Bitmap aBitmap( Size(aTestSize.getX(),
                         aTestSize.getY()), 24 );

    // Fill bitmap with generated content
    {
        Bitmap::ScopedWriteAccess pWriteAccess( aBitmap );
        for( int y=0; y<aTestSize.getY(); ++y )
            for( int x=0; x<aTestSize.getX(); ++x )
                pWriteAccess->SetPixel(y,x,
                                       Color(pDevice->getPixelData(basegfx::B2IPoint(x,y))) );
    }

    DrawBitmap( Point(), aBitmap );
}

USHORT TestApp::Exception( USHORT nError )
{
    switch( nError & EXC_MAJORTYPE )
    {
        case EXC_RSCNOTLOADED:
            Abort( String::CreateFromAscii( "Error: could not load language resources.\nPlease check your installation.\n" ) );
            break;
    }
    return 0;
}

void TestApp::Main()
{
    //-------------------------------------------------
    // create the global service-manager
    //-------------------------------------------------
    uno::Reference< lang::XMultiServiceFactory > xFactory;
    try
    {
        uno::Reference< uno::XComponentContext > xCtx = ::cppu::defaultBootstrap_InitialComponentContext();
        xFactory = uno::Reference< lang::XMultiServiceFactory >(  xCtx->getServiceManager(),
                                                                  uno::UNO_QUERY );
        if( xFactory.is() )
            ::comphelper::setProcessServiceFactory( xFactory );
    }
    catch(const uno::Exception& )
    {
    }

    if( !xFactory.is() )
    {
        OSL_TRACE( "Could not bootstrap UNO, installation must be in disorder. Exiting." );
        exit( 1 );
    }

    // Create UCB (for backwards compatibility, in case some code still uses
    // plain createInstance w/o args directly to obtain an instance):
    ::ucb::UniversalContentBroker::create(
        comphelper::getProcessComponentContext() );

    TestWindow pWindow;
    pWindow.Execute();
}

TestApp aDemoApp;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
