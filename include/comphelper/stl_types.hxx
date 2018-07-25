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
#ifndef INCLUDED_COMPHELPER_STL_TYPES_HXX
#define INCLUDED_COMPHELPER_STL_TYPES_HXX

#include <sal/config.h>

#include <math.h>
#include <memory>

#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>

namespace comphelper
{

// comparison functors

struct UStringMixLess
{
    bool m_bCaseSensitive;
public:
    UStringMixLess(bool bCaseSensitive = true):m_bCaseSensitive(bCaseSensitive){}
    bool operator() (const OUString& x, const OUString& y) const
    {
        if (m_bCaseSensitive)
            return x.compareTo(y) < 0;
        else
            return x.compareToIgnoreAsciiCase(y) < 0;
    }

    bool isCaseSensitive() const {return m_bCaseSensitive;}
};

class UStringMixEqual
{
    bool const m_bCaseSensitive;

public:
    UStringMixEqual(bool bCaseSensitive = true):m_bCaseSensitive(bCaseSensitive){}
    bool operator() (const OUString& lhs, const OUString& rhs) const
    {
        return m_bCaseSensitive ? lhs == rhs : lhs.equalsIgnoreAsciiCase( rhs );
    }
    bool isCaseSensitive() const {return m_bCaseSensitive;}
};

/// by-value less functor for std::set<std::unique_ptr<T>>
template<class T> struct UniquePtrValueLess
{
        bool operator()(std::unique_ptr<T> const& lhs,
                        std::unique_ptr<T> const& rhs) const
        {
            assert(lhs.get());
            assert(rhs.get());
            return (*lhs) < (*rhs);
        }
};

/// by-value implementation of std::foo<std::unique_ptr<T>>::operator==
template<template<typename, typename...> class C, typename T, typename... Etc>
bool ContainerUniquePtrEquals(
        C<std::unique_ptr<T>, Etc...> const& lhs,
        C<std::unique_ptr<T>, Etc...> const& rhs)
{
    if (lhs.size() != rhs.size())
    {
        return false;
    }
    for (auto iter1 = lhs.begin(), iter2 = rhs.begin();
         iter1 != lhs.end();
         ++iter1, ++iter2)
    {
        if (!(**iter1 == **iter2))
        {
            return false;
        }
    }
    return true;
};


/** STL-compliant structure for comparing Reference&lt; &lt;iface&gt; &gt; instances
*/
template < class IAFCE >
struct OInterfaceCompare
{
    bool operator() (const css::uno::Reference< IAFCE >& lhs, const css::uno::Reference< IAFCE >& rhs) const
    {
        return lhs.get() < rhs.get();
            // this does not make any sense if you see the semantics of the pointer returned by get:
            // It's a pointer to a point in memory where an interface implementation lies.
            // But for our purpose (provide a reliable less-operator which can be used with the STL), this is
            // sufficient ....
    }
};

template <class Tp, class Arg>
class mem_fun1_t
{
    typedef void (Tp::*_fun_type)(Arg);
public:
    explicit mem_fun1_t(_fun_type pf) : M_f(pf) {}
    void operator()(Tp* p, Arg x) const { (p->*M_f)(x); }
private:
    _fun_type const M_f;
};

template <class Tp, class Arg>
inline mem_fun1_t<Tp,Arg> mem_fun(void (Tp::*f)(Arg))
{
    return mem_fun1_t<Tp,Arg>(f);
}

/** output iterator that appends OUStrings into an OUStringBuffer.
 */
class OUStringBufferAppender
{
public:
    typedef OUStringBufferAppender Self;
    typedef ::std::output_iterator_tag iterator_category;
    typedef void value_type;
    typedef void reference;
    typedef void pointer;
    typedef size_t difference_type;

    OUStringBufferAppender(OUStringBuffer & i_rBuffer)
        : m_rBuffer(i_rBuffer) { }
    Self & operator=(Self const &)
    {   // MSVC 2013 with non-debug runtime requires this in xutility.hpp:289
        return *this;
    }
    Self & operator=(OUString const & i_rStr)
    {
        m_rBuffer.append( i_rStr );
        return *this;
    }
    Self & operator*() { return *this; } // so operator= works
    Self & operator++() { return *this; }

private:
    OUStringBuffer & m_rBuffer;
};

/** algorithm similar to std::copy, but inserts a separator between elements.
 */
template< typename ForwardIter, typename OutputIter, typename T >
OutputIter intersperse(
    ForwardIter start, ForwardIter end, OutputIter out, T const & separator)
{
    if (start != end) {
        *out = *start;
        ++start;
        ++out;
    }

    while (start != end) {
        *out = separator;
        ++out;
        *out = *start;
        ++start;
        ++out;
    }

    return out;
}

}

#endif // INCLUDED_COMPHELPER_STL_TYPES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
