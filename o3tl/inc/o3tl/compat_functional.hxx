/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *
 * Copyright (c) 1994
 * Hewlett-Packard Company
 *
 * Copyright (c) 1996-1998
 * Silicon Graphics Computer Systems, Inc.
 *
 * Copyright (c) 1997
 * Moscow Center for SPARC Technology
 *
 * Copyright (c) 1999
 * Boris Fomitchev
 *
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use or copy this software for any purpose is hereby granted
 * without fee, provided the above notices are retained on all copies.
 * Permission to modify the code and to distribute modified code is granted,
 * provided the above notices are retained, and a notice that the code was
 * modified is included with the above copyright notice.
 *
 */

/*
 * Lifted and paraphrased from STLport
 */

#ifndef INCLUDED_O3TL_COMPAT_FUNCTIONAL_HXX
#define INCLUDED_O3TL_COMPAT_FUNCTIONAL_HXX

#include <functional>

namespace o3tl
{

template<class T>
struct identity : public std::unary_function<T, T>
{
     T operator()(const T& y) const
     {
          return (y);
     }
};

template<class T1,class T2>
struct project1st : public std::binary_function<T1, T2, T1>
{
    T1 operator()(const T1& y, const T2&) const
    {
        return (y);
    }
};

template<class T1,class T2>
struct project2nd : public std::binary_function<T1, T2, T2>
{
    T2 operator()(const T1&, const T2& x) const
    {
        return (x);
    }
};

template<class P>
struct select1st : public std::unary_function<P, typename P::first_type>
{
    const typename P::first_type& operator()(const P& y) const
    {
        return (y.first);
    }
};

template<class P>
struct select2nd : public std::unary_function<P, typename P::second_type>
{
    const typename P::second_type& operator()(const P& y) const
    {
        return (y.second);
    }
};

template<class F1, class F2>
class unary_compose : public std::unary_function<typename F2::argument_type, typename F1::result_type>
{
    public:
        unary_compose(const F1& fnction1, const F2& fnction2) : ftor1(fnction1), ftor2(fnction2) {}

        typename F1::result_type operator()(const typename F2::argument_type& y) const
        {
            return (ftor1(ftor2(y)));
        }

    protected:
        F1 ftor1;
        F2 ftor2;
};

template<class F1, class F2>
inline unary_compose<F1, F2> compose1(const F1& fnction1, const F2& fnction2)
{
    return (unary_compose<F1, F2>(fnction1, fnction2));
}

template<class F1, class F2, class F3>
class binary_compose : public std::unary_function<typename F2::argument_type,typename F1::result_type>
{
    public:
        binary_compose(const F1& fnction1, const F2& fnction2, const F3& fnction3) : ftor1(fnction1), ftor2(fnction2), ftor3(fnction3) {}

        typename F1::result_type operator()(const typename F2::argument_type& y) const
        {
            return (ftor1(ftor2(y), ftor3(y)));
        }

    protected:
        F1 ftor1;
        F2 ftor2;
        F3 ftor3;
};

template<class F1, class F2, class F3>
inline binary_compose<F1, F2, F3> compose2(const F1& fnction1, const F2& fnction2, const F3& fnction3)
{
    return (binary_compose<F1, F2, F3>(fnction1, fnction2, fnction3));
}

}   // namespace o3tl

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
