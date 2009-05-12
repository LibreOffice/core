/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: compositeiterator.hxx,v $
 * $Revision: 1.5 $
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

#ifndef INCLUDED_BASEBMP_COMPOSITEITERATOR_HXX
#define INCLUDED_BASEBMP_COMPOSITEITERATOR_HXX

#include <sal/types.h>
#include <osl/diagnose.h>

#include <basebmp/nonstandarditerator.hxx>
#include <vigra/tuple.hxx>
#include <vigra/iteratortraits.hxx>


namespace basebmp
{

namespace detail
{
    template< typename T1, typename T2 > class ArithmeticProxy
    {
    public:
        ArithmeticProxy(T1& val1, T2& val2) :
            mpVal1( &val1 ),
            mpVal2( &val2 )
        {}

        void operator++() { ++(*mpVal1); ++(*mpVal2); }
        void operator++(int) { (*mpVal1)++; (*mpVal2)++; }
        void operator--() { --(*mpVal1); --(*mpVal2); }
        void operator--(int) { (*mpVal1)--; (*mpVal2)--; }
        void operator+=(int d) {*mpVal1+=d; *mpVal2+=d; }
        void operator-=(int d) {*mpVal1-=d; *mpVal2-=d; }

        bool operator==(ArithmeticProxy const & rhs) const
        { return *mpVal1==*rhs.mpVal1 && *mpVal2==*rhs.mpVal2; }

        bool operator!=(ArithmeticProxy const & rhs) const
        { return *mpVal1!=*rhs.mpVal1 || *mpVal2!=*rhs.mpVal2; }

        bool operator<(ArithmeticProxy const & rhs) const
        { return *mpVal1<*rhs.mpVal1 && *mpVal2<*rhs.mpVal2; }

        bool operator<=(ArithmeticProxy const & rhs) const
        { return *mpVal1<=*rhs.mpVal1 && *mpVal2<=*rhs.mpVal2; }

        bool operator>(ArithmeticProxy const & rhs) const
        { return *mpVal1>*rhs.mpVal1 && *mpVal2>*rhs.mpVal2; }

        bool operator>=(ArithmeticProxy const & rhs) const
        { return *mpVal1>=*rhs.mpVal1 && *mpVal2>=*rhs.mpVal2; }

        int operator-(ArithmeticProxy const & rhs) const
        { return *mpVal1 - *rhs.mpVal1; }

    private:
        T1* mpVal1;
        T2* mpVal2;
    };

    template< typename Iterator1,
              typename Iterator2,
              typename ValueType,
              typename DifferenceType,
              typename IteratorCategory,
              class Derived >
    class CompositeIteratorBase : public NonStandardIterator
    {
    public:
        typedef Iterator1        iterator1_type;
        typedef Iterator2        iterator2_type;
        typedef ValueType        value_type;
        typedef DifferenceType   difference_type;
        typedef IteratorCategory iterator_category;

    protected:
        iterator1_type maIter1;
        iterator2_type maIter2;

    private:
        bool equal(CompositeIteratorBase const & rhs) const
        {
            return (maIter1 == rhs.maIter1) && (maIter2 == rhs.maIter2);
        }

    public:
        CompositeIteratorBase() :
            maIter1(),
            maIter2()
        {}

        CompositeIteratorBase( const iterator1_type& rIter1, const iterator2_type& rIter2 ) :
            maIter1( rIter1 ),
            maIter2( rIter2 )
        {}

        bool operator==(Derived const & rhs) const
        {
            return equal(rhs);
        }

        bool operator!=(Derived const & rhs) const
        {
            return !equal(rhs);
        }

        difference_type operator-(Derived const & rhs) const
        {
            OSL_ASSERT( maIter1 - rhs.maIter1 == maIter2 - rhs.maIter2 );
            return maIter1 - rhs.maIter1;
        }

        Derived & operator+=(difference_type const & s)
        {
            maIter1 += s;
            maIter2 += s;
            return static_cast<Derived&>(*this);
        }

        Derived & operator-=(difference_type const & s)
        {
            maIter1 -= s;
            maIter2 -= s;
            return static_cast<Derived&>(*this);
        }

        Derived operator+(difference_type const & s) const
        {
            Derived ret(static_cast<Derived const&>(*this));
            ret += s;
            return ret;
        }

        Derived operator-(difference_type const & s) const
        {
            Derived ret(static_cast<Derived const&>(*this));
            ret -= s;
            return ret;
        }

        Derived& operator++()
        {
            ++maIter1;
            ++maIter2;
            return static_cast<Derived&>(*this);
        }

        Derived& operator--()
        {
            --maIter1;
            --maIter2;
            return static_cast<Derived&>(*this);
        }

        Derived operator++(int)
        {
            Derived ret(static_cast<Derived const&>(*this));
            ++maIter1;
            ++maIter2;
            return ret;
        }

        Derived operator--(int)
        {
            Derived ret(static_cast<Derived const&>(*this));
            --maIter1;
            --maIter2;
            return ret;
        }

        value_type get() const
        {
            return value_type(maIter1.get(),
                              maIter2.get());
        }

        value_type get(difference_type const & d) const
        {
            return value_type(maIter1.get(d),
                              maIter2.get(d));
        }

        void set( value_type v ) const
        {
            maIter1.set(v);
            maIter2.set(v);
        }

        void set( value_type v, difference_type const & d ) const
        {
            maIter1.set(v,d);
            maIter2.set(v,d);
        }

        const iterator1_type& first() const { return maIter1; }
        iterator1_type& first() { return maIter1; }

        const iterator2_type& second() const { return maIter2; }
        iterator2_type& second() { return maIter2; }
    };
}

/** Provide the composition of two 1D image iterators

    Use this template to compose two iterators into one (e.g. image
    and mask). Operations are transitive, e.g. operator== only returns
    true, if both wrapped iterator operator== have yielded true.

    Note that both iterators must have compatible difference types. To
    avoid funny effects, iterator ranges given by a CompositeIterator
    should consist of wrapped iterators of similar range
 */
template< typename Iterator1,
          typename Iterator2,
          typename ValueType,
          typename DifferenceType,
          typename IteratorCategory >
class CompositeIterator1D :
    public detail::CompositeIteratorBase< Iterator1,
                                          Iterator2,
                                          ValueType,
                                          DifferenceType,
                                          IteratorCategory,
                                          CompositeIterator1D<Iterator1,
                                                              Iterator2,
                                                              ValueType,
                                                              DifferenceType,
                                                              IteratorCategory> >
{
    typedef detail::CompositeIteratorBase< Iterator1,
                                           Iterator2,
                                           ValueType,
                                           DifferenceType,
                                           IteratorCategory,
                                           CompositeIterator1D<Iterator1,
                                                               Iterator2,
                                                               ValueType,
                                                               DifferenceType,
                                                               IteratorCategory> > base_type;
public:
    CompositeIterator1D() :
        base_type()
    {}

    CompositeIterator1D( const Iterator1& rIter1,
                         const Iterator2& rIter2 ) :
        base_type( rIter1, rIter2 )
    {}
};

/** Provide the composition of two 2D image iterators

    Use this template to compose two iterators into one (e.g. image
    and mask). Operations are transitive, e.g. operator== only returns
    true, if both wrapped iterator operator== have yielded true.

    Note that both iterators must have compatible difference types. To
    avoid funny effects, iterator ranges given by a CompositeIterator
    should consist of wrapped iterators of similar range
 */
template< typename Iterator1, typename Iterator2 > class CompositeIterator2D :
    public detail::CompositeIteratorBase< Iterator1,
                                          Iterator2,
                                          std::pair<
                                                typename vigra::IteratorTraits<Iterator1>::value_type,
                                                typename vigra::IteratorTraits<Iterator2>::value_type >,
                                          typename vigra::IteratorTraits<Iterator1>::difference_type,
                                          typename vigra::IteratorTraits<Iterator1>::iterator_category,
                                          CompositeIterator2D<Iterator1, Iterator2> >
{
    typedef detail::CompositeIteratorBase< Iterator1,
                                           Iterator2,
                                           std::pair<
                                                typename vigra::IteratorTraits<Iterator1>::value_type,
                                                typename vigra::IteratorTraits<Iterator2>::value_type >,
                                           typename vigra::IteratorTraits<Iterator1>::difference_type,
                                           typename vigra::IteratorTraits<Iterator1>::iterator_category,
                                           CompositeIterator2D<Iterator1, Iterator2> > base_type;
public:
    typedef CompositeIterator1D< typename Iterator1::row_iterator,
                                 typename Iterator2::row_iterator,
                                 typename base_type::value_type,
                                 int,
                                 typename base_type::iterator_category >  row_iterator;
    typedef CompositeIterator1D< typename Iterator1::column_iterator,
                                 typename Iterator2::column_iterator,
                                 typename base_type::value_type,
                                 int,
                                 typename base_type::iterator_category >  column_iterator;

    typedef detail::ArithmeticProxy< typename Iterator1::MoveX,
                                     typename Iterator2::MoveX >            MoveX;
    typedef detail::ArithmeticProxy< typename Iterator1::MoveY,
                                     typename Iterator2::MoveY >            MoveY;

    MoveX x;
    MoveY y;

    CompositeIterator2D() :
        base_type(),
        x(this->maIter1.x,this->maIter2.x),
        y(this->maIter1.y,this->maIter2.y)
    {}

    CompositeIterator2D( const Iterator1& rIter1, const Iterator2& rIter2 ) :
        base_type( rIter1, rIter2 ),
        x(this->maIter1.x,this->maIter2.x),
        y(this->maIter1.y,this->maIter2.y)
    {}

    CompositeIterator2D( const CompositeIterator2D& rOld ) :
        base_type(rOld),
        x(this->maIter1.x,this->maIter2.x),
        y(this->maIter1.y,this->maIter2.y)
    {}

    CompositeIterator2D& operator=( const CompositeIterator2D& rNew )
    {
        this->maIter1 = rNew.maIter1;
        this->maIter2 = rNew.maIter2;

        x = MoveX(this->maIter1.x,
                  this->maIter2.x);
        y = MoveY(this->maIter1.y,
                  this->maIter2.y);
    }

    row_iterator rowIterator() const
    {
        return row_iterator(this->maIter1.rowIterator(),
                            this->maIter2.rowIterator());
    }

    column_iterator columnIterator() const
    {
        return column_iterator(this->maIter1.columnIterator(),
                               this->maIter2.columnIterator());
    }
};

} // namespace basebmp

#endif /* INCLUDED_BASEBMP_COMPOSITEITERATOR_HXX */
