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
#include <functional>
#include <memory>

#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/NamedValue.hpp>

namespace comphelper
{

// comparison functors

struct UStringMixLess : public ::std::binary_function< OUString, OUString, bool>
{
    bool m_bCaseSensitive;
public:
    UStringMixLess(bool bCaseSensitive = true):m_bCaseSensitive(bCaseSensitive){}
    bool operator() (const OUString& x, const OUString& y) const
    {
        if (m_bCaseSensitive)
            return rtl_ustr_compare(x.getStr(), y.getStr()) < 0;
        else
            return rtl_ustr_compareIgnoreAsciiCase(x.getStr(), y.getStr()) < 0;
    }

    bool isCaseSensitive() const {return m_bCaseSensitive;}
};

class UStringMixEqual: public std::binary_function<OUString, OUString, bool>
{
    bool m_bCaseSensitive;

public:
    UStringMixEqual(bool bCaseSensitive = true):m_bCaseSensitive(bCaseSensitive){}
    bool operator() (const OUString& lhs, const OUString& rhs) const
    {
        return m_bCaseSensitive ? lhs.equals( rhs ) : lhs.equalsIgnoreAsciiCase( rhs );
    }
    bool isCaseSensitive() const {return m_bCaseSensitive;}
};

class TPropertyValueEqualFunctor : public ::std::binary_function< css::beans::PropertyValue,OUString,bool>
{
public:
    TPropertyValueEqualFunctor()
    {}
    bool operator() (const css::beans::PropertyValue& lhs, const OUString& rhs) const
    {
        return !!(lhs.Name == rhs);
    }
};

class TNamedValueEqualFunctor : public ::std::binary_function< css::beans::NamedValue,OUString,bool>
{
public:
    TNamedValueEqualFunctor()
    {}
    bool operator() (const css::beans::NamedValue& lhs, const OUString& rhs) const
    {
        return !!(lhs.Name == rhs);
    }
};

/// by-value less functor for std::set<std::unique_ptr<T>>
template<class T> struct UniquePtrValueLess
    : public ::std::binary_function<std::unique_ptr<T>, std::unique_ptr<T>, bool>
{
        bool operator()(std::unique_ptr<T> const& lhs,
                        std::unique_ptr<T> const& rhs) const
        {
            assert(lhs.get());
            assert(rhs.get());
            return (*lhs) < (*rhs);
        }
};

/** STL-compliant structure for comparing Reference&lt; &lt;iface&gt; &gt; instances
*/
template < class IAFCE >
struct OInterfaceCompare
    :public ::std::binary_function  <   css::uno::Reference< IAFCE >
                                    ,   css::uno::Reference< IAFCE >
                                    ,   bool
                                    >
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

template <class _Tp, class _Arg>
class mem_fun1_t : public ::std::binary_function<_Tp*,_Arg,void>
{
    typedef void (_Tp::*_fun_type)(_Arg);
public:
    explicit mem_fun1_t(_fun_type __pf) : _M_f(__pf) {}
    void operator()(_Tp* __p, _Arg __x) const { (__p->*_M_f)(__x); }
private:
    _fun_type _M_f;
};

template <class _Tp, class _Arg>
inline mem_fun1_t<_Tp,_Arg> mem_fun(void (_Tp::*__f)(_Arg))
{
    return mem_fun1_t<_Tp,_Arg>(__f);
}

/** output iterator that appends OUStrings into an OUStringBuffer.
 */
class OUStringBufferAppender :
    public ::std::iterator< ::std::output_iterator_tag, void, void, void, void>
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
    Self & operator=(OUString const & i_rStr)
    {
        m_rBuffer.append( i_rStr );
        return *this;
    }
    Self & operator*() { return *this; } // so operator= works
    Self & operator++() { return *this; }
    Self & operator++(int) { return *this; }

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
