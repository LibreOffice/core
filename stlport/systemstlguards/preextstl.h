/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
//1. Force inclusion of a std:: using header to ensure the stlport define
//of std as "stlport"
#include <algorithm>
#if defined(ADAPT_EXT_STL)
//2. Force inclusion of stlport headers to get their stlport:: definitions
#   include <ostream>
#   include <istream>
#   include <fstream>
#   include <iostream>
#   include <sstream>
#   include <vector>
#   include <deque>
#   include <set>
#   include <list>
#   include <map>
//3. Now force inclusion of native headers to get their std:: definitions
#   if defined(std)
#       define std_was_redefined_as_stlport std
#       undef std
#       define _STLP_OUTERMOST_HEADER_ID 0xdeadbeaf
#       if defined(_GNUC__)
#           pragma GCC visibility push(default)
#       endif
//Do we still need to include exception_defines.h here, I'd prefer
//to drop it if possible
#       if (__GNUC__ >= 5 || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)))
#           include _STLP_NATIVE_HEADER(bits/exception_defines.h)
#       else
#           include _STLP_NATIVE_HEADER(exception_defines.h)
#       endif
#       include _STLP_NATIVE_HEADER(limits)
#       if !defined(__GXX_EXPERIMENTAL_CXX0X__)
#           include _STLP_NATIVE_HEADER(memory)
#       endif
#       include _STLP_NATIVE_HEADER(exception)
#       include _STLP_NATIVE_HEADER(iosfwd)
#       if !defined(__GXX_EXPERIMENTAL_CXX0X__)
#           include _STLP_NATIVE_HEADER(algorithm)
#           include _STLP_NATIVE_HEADER(string)
#       else
#           include _STLP_NATIVE_HEADER(vector)
#           include _STLP_NATIVE_HEADER(string)
#           include _STLP_NATIVE_HEADER(numeric)
#           include _STLP_NATIVE_HEADER(functional)
#           include _STLP_NATIVE_HEADER(algorithm)
#       endif
#       include _STLP_NATIVE_HEADER(streambuf)
#       include _STLP_NATIVE_HEADER(ios)
#       include _STLP_NATIVE_HEADER(locale)
#       include _STLP_NATIVE_HEADER(stdexcept)
#       include _STLP_NATIVE_HEADER(ostream)
#       include _STLP_NATIVE_HEADER(istream)
#       include _STLP_NATIVE_HEADER(iostream)
#       include _STLP_NATIVE_HEADER(sstream)
#       include _STLP_NATIVE_HEADER(vector)
#       include _STLP_NATIVE_HEADER(deque)
#       include _STLP_NATIVE_HEADER(set)
#       include _STLP_NATIVE_HEADER(list)
#       include _STLP_NATIVE_HEADER(map)
#       if defined(_GNUC__)
#           pragma GCC visibility pop
#       endif
#   endif
#endif
//ext_std resolves to the std that external c++ libs, e.g. Graphite were built
//against regardless of whether that is stlport or system stl
namespace ext_std = std;
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
