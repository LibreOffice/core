/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef COMPHELPER_NEW_ARRAY_HXX
#define COMPHELPER_NEW_ARRAY_HXX

#include <limits>
#include <new>
#include <stddef.h>

namespace comphelper {

template<typename T> T *
newArray_null(size_t const n) throw()
{
    if ((::std::numeric_limits<size_t>::max() / sizeof(T)) <= n) {
        return 0;
    }
    return new (::std::nothrow) T[n];
}

template<typename T> T *
newArray_ex(size_t const n)
{
    if ((::std::numeric_limits<size_t>::max() / sizeof(T)) <= n) {
        throw ::std::bad_alloc();
    }
    return new T[n];
}

} // namespace comphelper

#endif // COMPHELPER_NEW_ARRAY_HXX
