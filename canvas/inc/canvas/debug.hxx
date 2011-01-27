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

#ifndef INCLUDED_CANVAS_DEBUG_HXX
#define INCLUDED_CANVAS_DEBUG_HXX

// shared_ptr debugging
// --------------------

#ifdef BOOST_SP_ENABLE_DEBUG_HOOKS

# include <sal/config.h>
# include <boost/shared_ptr.hpp>

::std::size_t find_unreachable_objects( bool );

# ifdef VERBOSE
#  include <osl/diagnose.h>
#  define SHARED_PTR_LEFTOVERS(a) OSL_TRACE("%s\n%s: Unreachable objects still use %d bytes\n", \
                                            BOOST_CURRENT_FUNCTION, a, \
                                            find_unreachable_objects(true) )
# else
/** This macro shows how much memory is still used by shared_ptrs

    Use this macro at places in the code where normally all shared_ptr
    objects should have been deleted. You'll get the number of bytes
    still contained in those objects, which quite possibly are prevented
    from deletion by circular references.
 */
#  define SHARED_PTR_LEFTOVERS(a) OSL_TRACE("%s\n%s: Unreachable objects still use %d bytes\n", \
                                            BOOST_CURRENT_FUNCTION, a, \
                                            find_unreachable_objects(false) )
# endif

#else

# define SHARED_PTR_LEFTOVERS(a) ((void)0)

#endif

#endif // ! defined(INCLUDED_CANVAS_DEBUG_HXX)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
