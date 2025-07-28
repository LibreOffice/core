/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_O3TL_DELETER_HXX
#define INCLUDED_O3TL_DELETER_HXX

#include <sal/config.h>

#include <cstdlib>

#if defined(__COVERITY__) && __COVERITY_MAJOR__ <= 2024
#define suppress_fun_call_w_exception(expr)                                                        \
    do                                                                                             \
    {                                                                                              \
        try                                                                                        \
        {                                                                                          \
            expr;                                                                                  \
        }                                                                                          \
        catch (...)                                                                                \
        {                                                                                          \
            std::terminate();                                                                      \
        }                                                                                          \
    } while (false)
#else
#define suppress_fun_call_w_exception(expr)                                                        \
    do                                                                                             \
    {                                                                                              \
        expr;                                                                                      \
    } while (false)
#endif

namespace o3tl
{
/** To markup std::unique_ptr that coverity warns might throw exceptions
    which won't throw in practice, or where std::terminate is
    an acceptable response if they do
*/
template <typename T> struct default_delete
{
    void operator()(T* p) noexcept { suppress_fun_call_w_exception(delete p); }
};

struct free_delete
{
    void operator()(void* p) { std::free(p); }
};

template <typename uniqueptr> void reset_preserve_ptr_during(uniqueptr& ptr)
{
    // HACK: for the case where the dtor of the obj held by ptr will trigger
    // functions which expect ptr to still be set during the dtor.
    // e.g. SdrObject::GetBroadcaster() is called during the destructor
    // in SdrEdgeObj::Notify(). So delete first, then clear the pointer
    delete ptr.get();
    // coverity[leaked_storage] - not a leak, deleted on line above
    (void)ptr.release();
}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
