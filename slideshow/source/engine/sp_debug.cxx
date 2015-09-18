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
#include <boost/shared_ptr.hpp>
#include <boost/detail/lightweight_mutex.hpp>
#include <canvas/debug.hxx>
#include <cstdlib>
#include <map>
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

namespace
{
    class X;

    struct count_layout
    {
        boost::detail::sp_counted_base * pi;
        int id;
    };

    struct shared_ptr_layout
    {
        X * px;
        count_layout pn;
    };
}

// assume 4 byte alignment for pointers when scanning
size_t const pointer_align = 4;

typedef std::map<void const *, long> map2_type;

static void scan_and_count(void const * area, size_t size, map_type const & m, map2_type & m2)
{
    unsigned char const * p = static_cast<unsigned char const *>(area);

    for(size_t n = 0; n + sizeof(shared_ptr_layout) <= size; p += pointer_align, n += pointer_align)
    {
        shared_ptr_layout const * q = reinterpret_cast<shared_ptr_layout const *>(p);

        if(q->pn.id == boost::detail::shared_count_id && q->pn.pi != 0 && m.count(q->pn.pi) != 0)
        {
            ++m2[q->pn.pi];
        }
    }
}

typedef std::deque<void const *> open_type;

static void scan_and_mark(void const * area, size_t size, map2_type & m2, open_type & open)
{
    unsigned char const * p = static_cast<unsigned char const *>(area);

    for(size_t n = 0; n + sizeof(shared_ptr_layout) <= size; p += pointer_align, n += pointer_align)
    {
        shared_ptr_layout const * q = reinterpret_cast<shared_ptr_layout const *>(p);

        if(q->pn.id == boost::detail::shared_count_id && q->pn.pi != 0 && m2.count(q->pn.pi) != 0)
        {
            open.push_back(q->pn.pi);
            m2.erase(q->pn.pi);
        }
    }
}

static void find_unreachable_objects_impl(map_type const & m, map2_type & m2)
{
    // scan objects for shared_ptr members, compute internal counts

    {
        std::cout << "... " << m.size() << " objects in m.\n";

        for(map_type::const_iterator i = m.begin(); i != m.end(); ++i)
        {
            BOOST_ASSERT(static_cast<boost::detail::sp_counted_base const *>(i->first)->use_count() != 0); // there should be no inactive counts in the map

            scan_and_count(i->second.first, i->second.second, m, m2);
        }

        std::cout << "... " << m2.size() << " objects in m2.\n";
    }

    // mark reachable objects

    {
        open_type open;

        for(map2_type::iterator i = m2.begin(); i != m2.end(); ++i)
        {
            boost::detail::sp_counted_base const * p = static_cast<boost::detail::sp_counted_base const *>(i->first);
            if(p->use_count() != i->second) open.push_back(p);
        }

        std::cout << "... " << m2.size() << " objects in open.\n";

        for(open_type::iterator j = open.begin(); j != open.end(); ++j)
        {
            m2.erase(*j);
        }

        while(!open.empty())
        {
            void const * p = open.front();
            open.pop_front();

            map_type::const_iterator i = m.find(p);
            BOOST_ASSERT(i != m.end());

            scan_and_mark(i->second.first, i->second.second, m2, open);
        }
    }

    // m2 now contains the unreachable objects
}

std::size_t find_unreachable_objects(bool report)
{
    map2_type m2;

#ifdef BOOST_HAS_THREADS

    // This will work without the #ifdef, but some compilers warn
    // that lock is not referenced

    mutex_type::scoped_lock lock(get_mutex());

#endif

    map_type const & m = get_map();

    find_unreachable_objects_impl(m, m2);

    if(report)
    {
        for(map2_type::iterator j = m2.begin(); j != m2.end(); ++j)
        {
            map_type::const_iterator i = m.find(j->first);
            BOOST_ASSERT(i != m.end());
            std::cout << "Unreachable object at " << i->second.first << ", " << i->second.second << " bytes long.\n";
        }
    }

    return m2.size();
}

// debug hooks

namespace boost
{

void sp_scalar_constructor_hook(void *)
{
}

void sp_scalar_constructor_hook(void * px, std::size_t size, void * pn)
{
#ifdef BOOST_HAS_THREADS

    mutex_type::scoped_lock lock(get_mutex());

#endif

    get_map()[pn] = std::make_pair(px, size);
}

void sp_scalar_destructor_hook(void *)
{
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
