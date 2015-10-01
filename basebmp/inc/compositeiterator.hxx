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

#ifndef INCLUDED_BASEBMP_INC_COMPOSITEITERATOR_HXX
#define INCLUDED_BASEBMP_INC_COMPOSITEITERATOR_HXX

#include <sal/types.h>
#include <osl/diagnose.h>

#include <nonstandarditerator.hxx>
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

        struct Impl
        {
            iterator1_type maIter1;
            iterator2_type maIter2;
            Impl()
                : maIter1()
                , maIter2()
            {
            }
            Impl(const iterator1_type& rIter1, const iterator2_type& rIter2)
                : maIter1(rIter1)
                , maIter2(rIter2)
            {
            }
        };

    protected:
        Impl* pImpl;

    private:
        bool equal(CompositeIteratorBase const & rhs) const
        {
            return (pImpl->maIter1 == rhs.pImpl->maIter1) && (pImpl->maIter2 == rhs.pImpl->maIter2);
        }

    public:
        CompositeIteratorBase()
        {
            pImpl = new Impl();
        }

        CompositeIteratorBase(const iterator1_type& rIter1, const iterator2_type& rIter2)
        {
            pImpl = new Impl(rIter1, rIter2);
        }

        CompositeIteratorBase(const CompositeIteratorBase& rOther)
        {
            pImpl = new Impl(rOther.pImpl->maIter1, rOther.pImpl->maIter2);
        }

        ~CompositeIteratorBase()
        {
            delete pImpl;
        }

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
            OSL_ASSERT(pImpl->maIter1 - rhs.pImpl->maIter1 == pImpl->maIter2 - rhs.pImpl->maIter2);
            return pImpl->maIter1 - rhs.pImpl->maIter1;
        }

        Derived & operator+=(difference_type const & s)
        {
            pImpl->maIter1 += s;
            pImpl->maIter2 += s;
            return static_cast<Derived&>(*this);
        }

        Derived & operator-=(difference_type const & s)
        {
            pImpl->maIter1 -= s;
            pImpl->maIter2 -= s;
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
            ++pImpl->maIter1;
            ++pImpl->maIter2;
            return static_cast<Derived&>(*this);
        }

        Derived& operator--()
        {
            --pImpl->maIter1;
            --pImpl->maIter2;
            return static_cast<Derived&>(*this);
        }

        Derived operator++(int)
        {
            Derived ret(static_cast<Derived const&>(*this));
            ++pImpl->maIter1;
            ++pImpl->maIter2;
            return ret;
        }

        Derived operator--(int)
        {
            Derived ret(static_cast<Derived const&>(*this));
            --pImpl->maIter1;
            --pImpl->maIter2;
            return ret;
        }

        value_type get() const
        {
            return value_type(pImpl->maIter1.get(),
                              pImpl->maIter2.get());
        }

        value_type get(difference_type const & d) const
        {
            return value_type(pImpl->maIter1.get(d),
                              pImpl->maIter2.get(d));
        }

        void set( value_type v ) const
        {
            pImpl->maIter1.set(v);
            pImpl->maIter2.set(v);
        }

        void set( value_type v, difference_type const & d ) const
        {
            pImpl->maIter1.set(v,d);
            pImpl->maIter2.set(v,d);
        }

        CompositeIteratorBase& operator=(const CompositeIteratorBase& rNew)
        {
            this->pImpl->maIter1 = rNew.pImpl->maIter1;
            this->pImpl->maIter2 = rNew.pImpl->maIter2;
            return *this;
        }

        const iterator1_type& first() const { return pImpl->maIter1; }
        iterator1_type& first() { return pImpl->maIter1; }

        const iterator2_type& second() const { return pImpl->maIter2; }
        iterator2_type& second() { return pImpl->maIter2; }
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
        x(this->pImpl->maIter1.x,this->pImpl->maIter2.x),
        y(this->pImpl->maIter1.y,this->pImpl->maIter2.y)
    {}

    CompositeIterator2D( const Iterator1& rIter1, const Iterator2& rIter2 ) :
        base_type( rIter1, rIter2 ),
        x(this->pImpl->maIter1.x,this->pImpl->maIter2.x),
        y(this->pImpl->maIter1.y,this->pImpl->maIter2.y)
    {}

    CompositeIterator2D( const CompositeIterator2D& rOld ) :
        base_type(rOld),
        x(this->pImpl->maIter1.x,this->pImpl->maIter2.x),
        y(this->pImpl->maIter1.y,this->pImpl->maIter2.y)
    {}

    CompositeIterator2D& operator=( const CompositeIterator2D& rNew )
    {
        this->pImpl->maIter1 = rNew.pImpl->maIter1;
        this->pImpl->maIter2 = rNew.pImpl->maIter2;

        x = MoveX(this->pImpl->maIter1.x,
                  this->pImpl->maIter2.x);
        y = MoveY(this->pImpl->maIter1.y,
                  this->pImpl->maIter2.y);

        return *this;
    }

    row_iterator rowIterator() const
    {
        return row_iterator(this->pImpl->maIter1.rowIterator(),
                            this->pImpl->maIter2.rowIterator());
    }

    column_iterator columnIterator() const
    {
        return column_iterator(this->pImpl->maIter1.columnIterator(),
                               this->pImpl->maIter2.columnIterator());
    }
};

} // namespace basebmp

#endif /* INCLUDED_BASEBMP_INC_COMPOSITEITERATOR_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
