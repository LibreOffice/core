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
#ifndef _COMPHELPER_STLTYPES_HXX_
#define _COMPHELPER_STLTYPES_HXX_

#include "sal/config.h"

#include <vector>
#include <map>

#include <stack>
#include <set>

#include <math.h> // prevent conflict between exception and std::exception
#include <functional>


#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/NamedValue.hpp>

//... namespace comphelper ................................................
namespace comphelper
{
//.........................................................................

//========================================================================
// comparisation functions

//------------------------------------------------------------------------
    struct UStringLess : public ::std::binary_function< ::rtl::OUString, ::rtl::OUString, bool>
{
    bool operator() (const ::rtl::OUString& x, const ::rtl::OUString& y) const { return x < y ? true : false;}      // construct prevents a MSVC6 warning
};
//------------------------------------------------------------------------
struct UStringMixLess : public ::std::binary_function< ::rtl::OUString, ::rtl::OUString, bool>
{
    bool m_bCaseSensitive;
public:
    UStringMixLess(bool bCaseSensitive = true):m_bCaseSensitive(bCaseSensitive){}
    bool operator() (const ::rtl::OUString& x, const ::rtl::OUString& y) const
    {
        if (m_bCaseSensitive)
            return rtl_ustr_compare(x.getStr(), y.getStr()) < 0 ? true : false;
        else
            return rtl_ustr_compareIgnoreAsciiCase(x.getStr(), y.getStr()) < 0 ? true : false;
    }

    bool isCaseSensitive() const {return m_bCaseSensitive;}
};
//------------------------------------------------------------------------
struct UStringEqual
{
    sal_Bool operator() (const ::rtl::OUString& lhs, const ::rtl::OUString& rhs) const { return lhs.equals( rhs );}
};

//------------------------------------------------------------------------
struct UStringIEqual
{
    sal_Bool operator() (const ::rtl::OUString& lhs, const ::rtl::OUString& rhs) const { return lhs.equalsIgnoreAsciiCase( rhs );}
};

//------------------------------------------------------------------------
class UStringMixEqual
{
    sal_Bool m_bCaseSensitive;

public:
    UStringMixEqual(sal_Bool bCaseSensitive = sal_True):m_bCaseSensitive(bCaseSensitive){}
    sal_Bool operator() (const ::rtl::OUString& lhs, const ::rtl::OUString& rhs) const
    {
        return m_bCaseSensitive ? lhs.equals( rhs ) : lhs.equalsIgnoreAsciiCase( rhs );
    }
    sal_Bool isCaseSensitive() const {return m_bCaseSensitive;}
};
//------------------------------------------------------------------------
class TStringMixEqualFunctor : public ::std::binary_function< ::rtl::OUString,::rtl::OUString,bool>
{
    sal_Bool m_bCaseSensitive;

public:
    TStringMixEqualFunctor(sal_Bool bCaseSensitive = sal_True)
        :m_bCaseSensitive(bCaseSensitive)
    {}
    bool operator() (const ::rtl::OUString& lhs, const ::rtl::OUString& rhs) const
    {
        return !!(m_bCaseSensitive ? lhs.equals( rhs ) : lhs.equalsIgnoreAsciiCase( rhs ));
    }
    sal_Bool isCaseSensitive() const {return m_bCaseSensitive;}
};
//------------------------------------------------------------------------
class TPropertyValueEqualFunctor : public ::std::binary_function< ::com::sun::star::beans::PropertyValue,::rtl::OUString,bool>
{
public:
    TPropertyValueEqualFunctor()
    {}
    bool operator() (const ::com::sun::star::beans::PropertyValue& lhs, const ::rtl::OUString& rhs) const
    {
        return !!(lhs.Name == rhs);
    }
};
//------------------------------------------------------------------------
class TNamedValueEqualFunctor : public ::std::binary_function< ::com::sun::star::beans::NamedValue,::rtl::OUString,bool>
{
public:
    TNamedValueEqualFunctor()
    {}
    bool operator() (const ::com::sun::star::beans::NamedValue& lhs, const ::rtl::OUString& rhs) const
    {
        return !!(lhs.Name == rhs);
    }
};
//------------------------------------------------------------------------
class UStringMixHash
{
    sal_Bool m_bCaseSensitive;

public:
    UStringMixHash(sal_Bool bCaseSensitive = sal_True):m_bCaseSensitive(bCaseSensitive){}
    size_t operator() (const ::rtl::OUString& rStr) const
    {
        return m_bCaseSensitive ? rStr.hashCode() : rStr.toAsciiUpperCase().hashCode();
    }
    sal_Bool isCaseSensitive() const {return m_bCaseSensitive;}
};

//=====================================================================
//= OInterfaceCompare
//=====================================================================
/** is stl-compliant structure for comparing Reference&lt; &lt;iface&gt; &gt; instances
*/
template < class IAFCE >
struct OInterfaceCompare
    :public ::std::binary_function  <   ::com::sun::star::uno::Reference< IAFCE >
                                    ,   ::com::sun::star::uno::Reference< IAFCE >
                                    ,   bool
                                    >
{
    bool operator() (const ::com::sun::star::uno::Reference< IAFCE >& lhs, const ::com::sun::star::uno::Reference< IAFCE >& rhs) const
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

//.........................................................................
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

    OUStringBufferAppender(::rtl::OUStringBuffer & i_rBuffer)
        : m_rBuffer(i_rBuffer) { }
    Self & operator=(::rtl::OUString const & i_rStr)
    {
        m_rBuffer.append( i_rStr );
        return *this;
    }
    Self & operator*() { return *this; } // so operator= works
    Self & operator++() { return *this; }
    Self & operator++(int) { return *this; }

private:
    ::rtl::OUStringBuffer & m_rBuffer;
};

//.........................................................................
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

//.........................................................................
}
//... namespace comphelper ................................................

//==================================================================
// consistently defining stl-types
//==================================================================

#define DECLARE_STL_ITERATORS(classname)                            \
    typedef classname::iterator         classname##Iterator;        \
    typedef classname::const_iterator   Const##classname##Iterator  \

#define DECLARE_STL_MAP(keytype, valuetype, comparefct, classname)  \
    typedef std::map< keytype, valuetype, comparefct >  classname;  \
    DECLARE_STL_ITERATORS(classname)                                \

#define DECLARE_STL_STDKEY_MAP(keytype, valuetype, classname)               \
    DECLARE_STL_MAP(keytype, valuetype, std::less< keytype >, classname)    \

#define DECLARE_STL_VECTOR(valuetyp, classname)     \
    typedef std::vector< valuetyp >     classname;  \
    DECLARE_STL_ITERATORS(classname)                \

#define DECLARE_STL_USTRINGACCESS_MAP(valuetype, classname)                 \
    DECLARE_STL_MAP(::rtl::OUString, valuetype, ::comphelper::UStringLess, classname)   \

#define DECLARE_STL_STDKEY_SET(valuetype, classname)    \
    typedef ::std::set< valuetype > classname;          \
    DECLARE_STL_ITERATORS(classname)                    \

#define DECLARE_STL_SET(valuetype, comparefct, classname)               \
    typedef ::std::set< valuetype, comparefct > classname;  \
    DECLARE_STL_ITERATORS(classname)                        \

#endif  // _COMPHELPER_STLTYPES_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
