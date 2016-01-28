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

#ifndef INCLUDED_CANVAS_DEBUG_HXX
#define INCLUDED_CANVAS_DEBUG_HXX

// shared_ptr debugging


#ifdef BOOST_SP_ENABLE_DEBUG_HOOKS

# include <sal/config.h>
# include <memory>

::std::size_t find_unreachable_objects( bool );

# if OSL_DEBUG_LEVEL > 2
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
