#ifndef _XMLOFF_FUNCTIONAL_HXX
#define _XMLOFF_FUNCTIONAL_HXX

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


#endif
