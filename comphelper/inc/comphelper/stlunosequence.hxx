/*************************************************************************
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: stlunosequence.hxx,v $
 *
 * $Revision: 1.3 $
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
 ************************************************************************/

#ifndef _COMPHELPER_STLUNOITERATOR_HXX
#define _COMPHELPER_STLUNOITERATOR_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <iterator>
#include <sal/types.h>

namespace comphelper { namespace stlunosequence {
    template<typename S, typename V>
    class StlSequence;

    template<typename S, typename V>
    class StlSequenceIterator : public std::iterator<std::random_access_iterator_tag, V, sal_Int32>
    {
        public:
            typedef ::std::random_access_iterator_tag iterator_category;
            typedef V& reference;
            typedef V value_type;
            typedef sal_Int32 difference_type;
            typedef V* pointer;
            typedef const V& const_reference;

            StlSequenceIterator();
            StlSequenceIterator(S * uno_sequence, typename StlSequenceIterator<S,V>::difference_type index);

            reference operator*() const;
            StlSequenceIterator<S,V>& operator++();
            StlSequenceIterator<S,V> operator++(int);
            StlSequenceIterator<S,V>& operator--();
            StlSequenceIterator<S,V> operator--(int);
            StlSequenceIterator<S,V>& operator+=(const typename StlSequenceIterator<S,V>::difference_type by);
            StlSequenceIterator<S,V>& operator-=(const typename StlSequenceIterator<S,V>::difference_type by);
            StlSequenceIterator<S,V> operator+(const typename StlSequenceIterator<S,V>::difference_type by) const;
            StlSequenceIterator<S,V> operator-(const typename StlSequenceIterator<S,V>::difference_type by) const;
            typename StlSequenceIterator<S,V>::difference_type operator-(const StlSequenceIterator<S,V>& to) const;

            bool operator==(const StlSequenceIterator<S,V>& other) const;
            bool operator!=(const StlSequenceIterator<S,V>& other) const;
            bool operator<(const StlSequenceIterator<S,V>& other) const;
            reference operator[](const typename StlSequenceIterator<S,V>::difference_type by) const;


        protected:
            friend class StlSequence<S,V>;
            S * m_UnoSequence;
            typename StlSequenceIterator<S,V>::difference_type m_Index;
    };

    template<typename S, typename V>
    class StlSequenceConstIterator : public StlSequenceIterator<S,V>
    {
        public:
            typedef const V& reference;
            StlSequenceConstIterator();
            StlSequenceConstIterator(S * uno_sequence, typename StlSequenceIterator<S,V>::difference_type by);
            typename StlSequenceIterator<S,V>::const_reference operator*() const
            {
                return (*(this->m_UnoSequence))[this->m_Index];
            }

            typename StlSequenceIterator<S,V>::const_reference operator[](const typename StlSequenceIterator<S,V>::difference_type by) const
            {
                return (*(this->m_UnoSequence))[this->m_Index+by];
            }
    };

    template<typename S, typename V>
    class StlSequence
    {
        public:
            typedef sal_Int32 size_type;
            typedef V& reference;
            typedef const V& const_reference;
            typedef sal_Int32 difference_type;
            typedef V value_type;
            typedef StlSequenceIterator<S,V> iterator;
            typedef StlSequenceConstIterator<S,V> const_iterator;

            StlSequence(S& uno_sequence);

            typename StlSequence<S,V>::size_type size() const;
            typename StlSequence<S,V>::size_type max_size() const;
            bool empty() const;
            void swap(StlSequence<S,V>& other);
            typename StlSequence<S,V>::iterator begin();
            typename StlSequence<S,V>::iterator end();
            typename StlSequence<S,V>::const_iterator begin() const;
            typename StlSequence<S,V>::const_iterator end() const;

            bool operator==(const StlSequence<S,V>& other) const;
            bool operator!=(const StlSequence<S,V>& other) const;
            bool operator<(const StlSequence<S,V>& other) const;
            bool operator>(const StlSequence<S,V>& other) const;
            bool operator<=(const StlSequence<S,V>& other) const;
            bool operator>=(const StlSequence<S,V>& other) const;


        private:
            friend class StlSequenceIterator<S,V>;
            S * m_UnoSequence;
            static const typename StlSequence<S,V>::size_type begin_of_sequence;
            static const typename StlSequence<S,V>::size_type end_of_sequence;
    };

    //StlSequenceIterator
    template<typename S, typename V>
    StlSequenceIterator<S,V>::StlSequenceIterator()
        : m_UnoSequence(0), m_Index(StlSequence<S,V>::end_of_sequence)
    {}

    template<typename S, typename V>
    StlSequenceConstIterator<S,V>::StlSequenceConstIterator()
    {}

    template<typename S, typename V>
    StlSequenceIterator<S, V>::StlSequenceIterator(S * uno_sequence, typename StlSequenceIterator<S,V>::difference_type index)
        : m_UnoSequence(uno_sequence), m_Index(index)
    {}

    template<typename S, typename V>
    StlSequenceConstIterator<S, V>::StlSequenceConstIterator(S * uno_sequence, typename StlSequenceIterator<S,V>::difference_type index)
        : StlSequenceIterator<S,V>(uno_sequence, index)
    {}

    template<typename S, typename V>
    inline typename StlSequenceIterator<S,V>::reference StlSequenceIterator<S, V>::operator*() const
    {
        return (*m_UnoSequence)[m_Index];
    }

    template<typename S, typename V>
    inline StlSequenceIterator<S,V>& StlSequenceIterator<S, V>::operator++()
    {
        ++m_Index;
        if(m_Index>=m_UnoSequence->getLength())
            m_Index = StlSequence<S,V>::end_of_sequence;
        return *this;
    }

    template<typename S, typename V>
    inline StlSequenceIterator<S,V> StlSequenceIterator<S, V>::operator++(int)
    {
        StlSequenceIterator<S,V> temp = *this;
        ++*this;
        return temp;
    }

    template<typename S, typename V>
    inline StlSequenceIterator<S,V>& StlSequenceIterator<S, V>::operator--()
    {
        if(m_Index==StlSequence<S,V>::end_of_sequence)
            m_Index = m_UnoSequence->getLength();
        --m_Index;
        return *this;
    }

    template<typename S, typename V>
    inline StlSequenceIterator<S,V> StlSequenceIterator<S, V>::operator--(int)
    {
        StlSequenceIterator<S,V> temp = *this;
        --*this;
        return temp;
    }

    template<typename S, typename V>
    inline StlSequenceIterator<S,V>& StlSequenceIterator<S, V>::operator+=(const typename StlSequenceIterator<S,V>::difference_type by)
    {
        if(by<0)
        {
            this->operator-=(-by);
            return *this;
        }
        if(by==0)
            return *this;
        m_Index+=by;
        if(m_Index>=m_UnoSequence->getLength())
            m_Index=StlSequence<S,V>::end_of_sequence;
        return *this;
    }

    template<typename S, typename V>
    inline StlSequenceIterator<S,V>& StlSequenceIterator<S, V>::operator-=(const typename StlSequenceIterator<S,V>::difference_type by)
    {
        if(by<0)
        {
            if(-by==by) return *this; // breaking infinitive loop on MIN_INT
            this->operator+=(-by);
            return *this;
        }
        if(by==0)
            return *this;
        if(m_Index==StlSequence<S,V>::end_of_sequence)
            m_Index=m_UnoSequence->getLength();
        m_Index-=by;
        return *this;
    }

    template<typename S, typename V>
    inline StlSequenceIterator<S,V> StlSequenceIterator<S, V>::operator+(const typename StlSequenceIterator<S,V>::difference_type by) const
    {
        StlSequenceIterator<S,V> result = StlSequenceIterator<S,V>(*this);
        result+=by;
        return result;
    }

    template<typename S, typename V>
    inline StlSequenceIterator<S,V> StlSequenceIterator<S, V>::operator-(const typename StlSequenceIterator<S,V>::difference_type by) const
    {
        StlSequenceIterator<S,V> result = StlSequenceIterator<S,V>(*this);
        result-=by;
        return result;
    }

    template<typename S, typename V>
    inline typename StlSequenceIterator<S,V>::difference_type StlSequenceIterator<S, V>::operator-(const StlSequenceIterator<S,V>& to) const
    {
        if(m_Index==StlSequence<S,V>::end_of_sequence ^ to.m_Index==StlSequence<S,V>::end_of_sequence)
        {
            if(m_Index==StlSequence<S,V>::end_of_sequence)
                return m_UnoSequence->getLength()-to.m_Index;
            else
                return m_Index-m_UnoSequence->getLength();
        }
        return m_Index-to.m_Index;
    }

    template<typename S, typename V>
    inline bool StlSequenceIterator<S, V>::operator==(const StlSequenceIterator<S,V>& other) const
    {
        return (m_Index == other.m_Index);
    }

    template<typename S, typename V>
    inline bool StlSequenceIterator<S, V>::operator!=(const StlSequenceIterator<S,V>& other) const
    {
        return !this->operator==(other);
    }

    template<typename S, typename V>
    inline bool StlSequenceIterator<S, V>::operator<(const StlSequenceIterator<S,V>& other) const
    {
        if(m_Index==StlSequence<S,V>::end_of_sequence ^ other.m_Index==StlSequence<S,V>::end_of_sequence)
            return other.m_Index==StlSequence<S,V>::end_of_sequence;
        return m_Index<other.m_Index;
    }

    template<typename S, typename V>
    inline typename StlSequenceIterator<S,V>::reference StlSequenceIterator<S, V>::operator[](const typename StlSequenceIterator<S,V>::difference_type by) const
    {
        return (*(this->m_UnoSequence))[this->m_Index+by];
    }

    // StlSequence
    template<typename S, typename V>
    StlSequence<S,V>::StlSequence(S& uno_sequence)
        : m_UnoSequence(&uno_sequence)
    {}

    template<typename S, typename V>
    inline typename StlSequence<S,V>::size_type StlSequence<S,V>::size() const
    {
        return m_UnoSequence->getLength();
    }

    template<typename S, typename V>
    inline typename StlSequence<S,V>::size_type StlSequence<S,V>::max_size() const
    {
        return this->size();
    }

    template<typename S, typename V>
    inline bool StlSequence<S,V>::empty() const
    {
        return this->size() == 0;
    }

    template<typename S, typename V>
    inline void StlSequence<S,V>::swap(StlSequence<S,V>& other)
    {
        ::std::swap(*this, other);
    }

    template<typename S, typename V>
    inline typename StlSequence<S,V>::iterator StlSequence<S,V>::begin()
    {
        return typename StlSequence<S,V>::iterator(m_UnoSequence,
            size() ? begin_of_sequence : end_of_sequence);
    }

    template<typename S, typename V>
    inline typename StlSequence<S,V>::iterator StlSequence<S,V>::end()
    {
        return typename StlSequence<S,V>::iterator(m_UnoSequence, end_of_sequence);
    }

    template<typename S, typename V>
    inline typename StlSequence<S,V>::const_iterator StlSequence<S,V>::begin() const
    {
        return typename StlSequence<S,V>::const_iterator(m_UnoSequence,
            size() ? begin_of_sequence : end_of_sequence);
    }

    template<typename S, typename V>
    inline typename StlSequence<S,V>::const_iterator StlSequence<S,V>::end() const
    {
        return typename StlSequence<S,V>::const_iterator(m_UnoSequence, end_of_sequence);
    }

    template<typename S, typename V>
    inline bool StlSequence<S,V>::operator==(const StlSequence<S,V>& other) const
    {
        if(this->size() != other.size()) return false;
        return ::std::equal(this->begin(), this->end(), other.begin());
    }

    template<typename S, typename V>
    inline bool StlSequence<S,V>::operator<(const StlSequence<S,V>& other) const
    {
        return ::std::lexicographical_compare(this->begin(), this->end(), other.begin(), other.end());
    }

    template<typename S, typename V>
    inline bool StlSequence<S,V>::operator!=(const StlSequence<S,V>& other) const
    { return !(*this == other); }

    template<typename S, typename V>
    inline bool StlSequence<S,V>::operator>(const StlSequence<S,V>& other) const
    { return (other < *this); }

    template<typename S, typename V>
    inline bool StlSequence<S,V>::operator<=(const StlSequence<S,V>& other) const
    { return !(other < *this); }

    template<typename S, typename V>
    inline bool StlSequence<S,V>::operator>=(const StlSequence<S,V>& other) const
    { return !(*this < other); }

    template<typename S, typename V>
    const typename StlSequence<S,V>::size_type StlSequence<S,V>::begin_of_sequence = 0;

    template<typename S, typename V>
    const typename StlSequence<S,V>::size_type StlSequence<S,V>::end_of_sequence = -1;
}}


namespace comphelper
{
    /**
      @short a wrapper-class that provides stl-container-like access to an existing
      ::com::sun::star::uno::Sequence
      @descr This template class allows using an existing ::com::sun::star::uno::Sequence
      like any other stl container. It provides standard-compliant mutable random access
      iterators. Because random access iterators are the most generic iterators defined
      by the stl, any stl algorithm can be applied to the Sequence.
      <p>
      This is just a basic stl forward container, but _not_ an stl sequence (the size of the
      ::com::sun::star::uno::Sequence cannot be changed using the StlUnoSequence interface).
      Inserts are expensive operations on ::com::sun::star::uno::Sequence - use
      ::std::copy() to a efficient stl container implementing the stl sequence interface
      and the available insert iterator adapters you need those.
      <p>
      @attention There is one requirement of an stl container that this class does not meet:
      It is _not_ owner of its contents and thus it does not destroy its contents when destructed.
      @attention No additional boundschecking over the requirements of the STL are performed.
      E.g. sequence.begin()-- results in undefined behavior.
      @attention StlUnoSequence is not threadsafe.
      <p>
      Example: (creating a ::std::list from a ::com::sun::star::uno::Sequence)
      <code>
      ::com::sun::star::uno::Sequence<sal_Int32> uno_seq(10);
      ::comphelper::StlUnoSequence<sal_Int32> stl_seq(&uno_seq);
      ::std::list stl_list(stl_seq.begin(), stl_seq.end());
      </code>
      <p>
      Example: (sorting ::com::sun::star::uno::Sequence inplace)
      <code>
      ::com::sun::star::uno::Sequence<sal_Int32> uno_seq(10);
      ::comphelper::StlUnoSequence<sal_Int32> stl_seq(&uno_seq);
      ::std::sort(stl_seq.begin(), stl_seq.end());
      </code>
      <p>
      Example: (counting occurrences of 4711 in a ::com::sun::star::uno::Sequence)
      <code>
      ::com::sun::star::uno::Sequence<sal_Int32> uno_seq(10);
      ::comphelper::StlUnoSequence<sal_Int32> stl_seq(&uno_seq);
      sal_Int32 count = 0;
      ::std::count(stl_seq.begin(), stl_seq.end(), 4711, count);
      </code>
      <p>
      Example: (inserting the second half of one Sequence after the first element of another)
      <code>
      ::com::sun::star::uno::Sequence<sal_Int32> uno_seq1(10);
      ::com::sun::star::uno::Sequence<sal_Int32> uno_seq2(10);
      ::com::sun::star::uno::Sequence<sal_Int32> result(15);
      ::comphelper::StlUnoSequence<sal_Int32> stl_seq1(&uno_seq1);
      ::comphelper::StlUnoSequence<sal_Int32> stl_seq2(&uno_seq1);
      ::comphelper::StlUnoSequence<sal_Int32> stl_result(&result);
      ::std::list<sal_Int32> temp(uno_seq1.begin(), uno_seq1.end());
      ::std::copy(uno_seq2.begin()+5, uno_seq2.end(), ::std::insert_iterator< ::std::list<sal_Int32> >(temp, ++temp.begin()));
      ::std::copy(temp.begin(), temp.end(), result.begin());
      </code>

      @see http://www.sgi.com/tech/stl/Container.html
      @see http://www.sgi.com/tech/stl/Sequence.html
      @see http://www.sgi.com/tech/stl/RandomAccessIterator.html
    */

    template <typename V>
    class StlUnoSequence : public stlunosequence::StlSequence< ::com::sun::star::uno::Sequence<V>, V>
    {
        public:
            /*
               @short named constructor (these are needed to keep const-correctness)
             */
            static StlUnoSequence<V> createInstance(::com::sun::star::uno::Sequence<V>& uno_sequence)
            { return StlUnoSequence<V>(uno_sequence); }
            /*
               @short named constructor (these are needed to keep const-correctness)
             */
            static const StlUnoSequence<V> createInstance(const ::com::sun::star::uno::Sequence<V>& uno_sequence)
            { return StlUnoSequence<V>(const_cast< ::com::sun::star::uno::Sequence<V>& >(uno_sequence)); }
        private:
            StlUnoSequence(::com::sun::star::uno::Sequence<V>& uno_sequence)
                : stlunosequence::StlSequence< ::com::sun::star::uno::Sequence<V>, V>(uno_sequence)
            {}
    };

    /*
       @short shortcut for StlUnoSequence<V>::createInstance(uno_sequence).begin()
     */
    template <typename V>
    typename StlUnoSequence<V>::iterator stl_begin(::com::sun::star::uno::Sequence<V>& uno_sequence)
    { return StlUnoSequence<V>::createInstance(uno_sequence).begin(); }

    /*
       @short shortcut for StlUnoSequence<V>::createInstance(uno_sequence).end()
     */
    template <typename V>
    typename StlUnoSequence<V>::iterator stl_end(::com::sun::star::uno::Sequence<V>& uno_sequence)
    { return StlUnoSequence<V>::createInstance(uno_sequence).end(); }

    /*
       @short shortcut for StlUnoSequence<V>::createInstance(uno_sequence).begin()
     */
    template <typename V>
    typename StlUnoSequence<V>::const_iterator stl_begin(const ::com::sun::star::uno::Sequence<V>& uno_sequence)
    {
        return StlUnoSequence<V>::createInstance(uno_sequence).begin();
    }

    /*
       @short shortcut for StlUnoSequence<V>::createInstance(uno_sequence).end()
     */
    template <typename V>
    typename StlUnoSequence<V>::const_iterator stl_end(const ::com::sun::star::uno::Sequence<V>& uno_sequence)
    {
        return StlUnoSequence<V>::createInstance(uno_sequence).end();
    }
}
#endif
