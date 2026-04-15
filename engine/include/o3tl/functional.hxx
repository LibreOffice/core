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
 * Lifted and paraphrased from STLport - with additions from Fridrich
 * Strba and Thorsten Behrens
 */

#ifndef INCLUDED_O3TL_FUNCTIONAL_HXX
#define INCLUDED_O3TL_FUNCTIONAL_HXX

namespace o3tl
{
/// Select first value of a pair
template<typename P>
struct select1st
{
    typedef P argument_type;
    typedef typename P::first_type result_type;
    const result_type& operator()( const argument_type& cp ) const {
        return cp.first;
    }
};

/// Select second value of a pair
template<typename P>
struct select2nd
{
    typedef P argument_type;
    typedef typename P::second_type result_type;
    const result_type& operator()( const argument_type& cp ) const {
        return cp.second;
    }
};

}   // namespace o3tl

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
