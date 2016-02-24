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


//  sp_collector.cpp

//  Copyright (c) 2002, 2003 Peter Dimov

//  Permission to copy, use, modify, sell and distribute this software
//  is granted provided this copyright notice appears in all copies.
//  This software is provided "as is" without express or implied
//  warranty, and with no claim as to its suitability for any purpose.


#if defined(BOOST_SP_ENABLE_DEBUG_HOOKS)

#include <boost/assert.hpp>
#include <boost/detail/lightweight_mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <canvas/debug.hxx>
#include <cstdlib>
#include <map>
#include <memory>
#include <deque>
#include <iostream>

typedef std::map< void const *, std::pair<void *, size_t> > map_type;

static map_type & get_map()
{
    static map_type m;
    return m;
}

typedef boost::detail::lightweight_mutex mutex_type;

static mutex_type & get_mutex()
{
    static mutex_type m;
    return m;
}

static void * init_mutex_before_main = &get_mutex();


// debug hooks

namespace boost
{

void sp_scalar_constructor_hook(void * px, std::size_t size, void * pn)
{
#ifdef BOOST_HAS_THREADS

    mutex_type::scoped_lock lock(get_mutex());

#endif

    get_map()[pn] = std::make_pair(px, size);
}

void sp_scalar_destructor_hook(void *, std::size_t, void * pn)
{
#ifdef BOOST_HAS_THREADS

    mutex_type::scoped_lock lock(get_mutex());

#endif

    get_map().erase(pn);
}

} // namespace boost

#endif // defined(BOOST_SP_ENABLE_DEBUG_HOOKS)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
