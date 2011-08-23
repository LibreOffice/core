/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef _XMLOFF_FUNCTIONAL_HXX
#define _XMLOFF_FUNCTIONAL_HXX

#include <rtl/ustring.hxx>

/* THIS HEADER IS DEPRECATED. USE comphelper/stl_types.hxx INSTEAD!!! */

/** @#file
 *
 * re-implement STL functors as needed
 *
 * The standard comparison operators from the STL cause warnings with
 * several compilers about our sal_Bool (=unsigned char) being
 * converted to bool (C++ bool). We wish to avoid that.
 */

struct less_functor
{
    bool operator()(const ::rtl::OUString& x, 
                    const ::rtl::OUString& y) const
    {
        return 0 != (x<y);
    }
};

namespace binfilter {
}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
