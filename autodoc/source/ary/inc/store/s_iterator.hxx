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

#ifndef ARY_STORE_S_ITERATOR_HXX
#define ARY_STORE_S_ITERATOR_HXX

// USED SERVICES
#include <ary/getncast.hxx>
#include "s_base.hxx"




namespace ary
{
namespace stg
{


template <class>        class const_iterator;
template <class, class> class const_filter_iterator;


/** A non-const iterator that runs on a ->Storage<>.

    @collab Storage<>
*/
template <class ENTITY>
class iterator : public std::iterator<std::forward_iterator_tag, ENTITY>
{
  public:
    typedef iterator<ENTITY>                            self;
    typedef typename Base<ENTITY>::impl_type            impl_container;
    typedef typename impl_container::const_iterator     impl_type;

    // OPERATORS
                        iterator()
                            :   itImpl()        {}
    explicit            iterator(
                            impl_type           i_impl)
                            :   itImpl(i_impl)  {}
                        ~iterator()             {}

    bool                operator==(
                            self                i_other ) const
                                                { return itImpl == i_other.itImpl; }
    bool                operator!=(
                            self                i_other ) const
                                                { return itImpl != i_other.itImpl; }
    ENTITY &            operator*() const       { csv_assert(*itImpl != 0);
                                                  return *(*itImpl); }
    self &              operator++()            { ++itImpl; return *this; }
    self                operator++(int)         { return self(itImpl++); }

  private:
    friend class const_iterator<ENTITY>;        // For const_iterator(iterator);
    impl_type           ImplIterator() const    { return itImpl; }

    // DATA
    impl_type           itImpl;
};


/** A const iterator that runs on a ->Storage<>.

    @collab Storage<>
*/
template <class ENTITY>
class const_iterator :
    public std::iterator<std::forward_iterator_tag, const ENTITY>
{
  public:
    typedef const_iterator<ENTITY>                      self;
    typedef typename Base<ENTITY>::impl_type            impl_container;
    typedef typename impl_container::const_iterator     impl_type;

    // OPERATORS
                        const_iterator()
                            :   itImpl()        {}
    explicit            const_iterator(
                            impl_type           i_impl)
                            :   itImpl(i_impl)  {}
                        const_iterator(         // implicit conversions allowed
                            ::ary::stg::iterator<ENTITY>    i_it )
                            :   itImpl(i_it.ImplIterator()) {}
                        ~const_iterator()       {}

    bool                operator==(
                            self                i_other ) const
                                                { return itImpl == i_other.itImpl; }
    bool                operator!=(
                            self                i_other ) const
                                                { return itImpl != i_other.itImpl; }
    const ENTITY &      operator*() const       { csv_assert(*itImpl != 0);
                                                  return *(*itImpl); }
    self &              operator++()            { ++itImpl; return *this; }
    self                operator++(int)         { return self(itImpl++); }

  private:
    // DATA
    impl_type           itImpl;
};





/** A non const iterator that runs on a ->Storage<> and returns only
    the elements of a specific type.

    @tpl ENTITY
    The element type of the ->Storage<>

    @tpl FILTER
    The actual type of the returned items. FILTER needs to be derived from
    ENTITY.

    @collab Storage<>
*/
template <class ENTITY, class FILTER>
class filter_iterator :
    public std::iterator<std::forward_iterator_tag, FILTER>
{
  public:
    typedef filter_iterator<ENTITY,FILTER>              self;
    typedef ::ary::stg::iterator<ENTITY>                impl_type;

    // OPERATORS
                        filter_iterator()
                            :   itCur()         {}
    explicit            filter_iterator(
                            impl_type           i_cur )
                            :   itCur(i_cur)    {}
                        ~filter_iterator()      {}

    bool                operator==(
                            self                i_other ) const
                                                { return itCur == i_other.itCur; }
    bool                operator!=(
                            self                i_other ) const
                                                { return itCur != i_other.itCur; }
    FILTER &            operator*() const       { csv_assert(IsValid());
                                                  return static_cast< FILTER& >(*itCur); }
    self &              operator++()            { ++itCur;
                                                  return *this; }
    self                operator++(int)         { return self(itCur++); }
    bool                IsValid() const         { return ary::is_type<FILTER>(*itCur); }

  private:
    friend class const_filter_iterator<ENTITY,FILTER>;  // For const_filter_iterator(filter_iterator);
    impl_type           ImplCur() const         { return itCur; }

    // DATA
    impl_type           itCur;
};


/** A const iterator that runs on a ->Storage<> and returns only
    the elements of a specific type.

    @tpl ENTITY
    The element type of the ->Storage<>

    @tpl FILTER
    The actual type of the returned items. FILTER needs to be derived from
    ENTITY.

    @collab Storage<>
*/
template <class ENTITY, class FILTER>
class const_filter_iterator :
    public std::iterator<std::forward_iterator_tag, const FILTER>
{
  public:
    typedef const_filter_iterator<ENTITY,FILTER>        self;
    typedef ::ary::stg::const_iterator<ENTITY>          impl_type;

    // OPERATORS
                        const_filter_iterator()
                            :   itCur()         {}
    explicit            const_filter_iterator(
                            impl_type           i_cur )
                            :   itCur(i_cur)    {}
    explicit            const_filter_iterator(  // implicit conversions allowed
                            filter_iterator<ENTITY,FILTER>
                                                i_it )
                            :   itCur(i_it.ImplCur())   {}
                        ~const_filter_iterator()
                                                {}
    bool                operator==(
                            self                i_other ) const
                                                { return itCur == i_other.itCur; }
    bool                operator!=(
                            self                i_other ) const
                                                { return itCur != i_other.itCur; }
    const FILTER &      operator*() const       { csv_assert(IsValid());
                                                  return static_cast< const FILTER& >(*itCur); }
    self &              operator++()            { ++itCur;
                                                  return *this; }
    self                operator++(int)         { return self(itCur++); }
    bool                IsValid() const         { return ary::is_type<FILTER>(*itCur); }

  private:
    // DATA
    impl_type           itCur;
};




}   // namespace stg
}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
