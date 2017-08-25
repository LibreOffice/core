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

#include <com/sun/star/uno/Exception.hpp>

namespace o3tl {

/** To markup std::unique_ptr cases where coverity warns might throw exceptions
    which won't throw in practice, or where std::terminate is
    an acceptable response if they do
*/
template<typename T> struct default_delete
{
    void operator() (T* p) noexcept
    {
#if defined(__COVERITY__)
        try
        {
            delete p;
        }
        catch (const css::uno::Exception& e)
        {
            SAL_WARN("vcl.app", "Fatal exception: " << e.Message);
            std::terminate();
        }
        catch (const std::exception& e)
        {
            SAL_WARN("vcl.app", "Fatal exception: " << e.what());
            std::terminate();
        }
#else
        delete p;
#endif
    }
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
